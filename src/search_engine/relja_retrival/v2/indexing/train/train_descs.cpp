/*
  ==== Author:

  Relja Arandjelovic (relja@robots.ox.ac.uk)
  Visual Geometry Group,
  Department of Engineering Science
  University of Oxford

  ==== Copyright:

  The library belongs to Relja Arandjelovic and the University of Oxford.
  No usage or redistribution is allowed without explicit permission.
*/

#include "train_descs.h"

#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>

#ifdef RR_MPI
#include <boost/mpi/collectives.hpp>
#include <boost/serialization/utility.hpp> // for std::pair
#include <boost/serialization/string.hpp>
#endif

#include "image_util.h"
#include "mpi_queue.h"
#include "par_queue.h"
#include "same_random.h"
#include "timing.h"

namespace buildIndex {



  typedef std::pair<uint32_t, std::string> trainDescsResult;

  class trainDescsManager : public queueManager<trainDescsResult> {
  public:

    trainDescsManager(uint32_t numDocs,
                      uint32_t numDims,
                      uint8_t dtypeCode,
                      int64_t trainNumDescs,
                      std::string const trainDescsFn)
      : allDescs_(trainNumDescs<0),
        remainNumDescs_(trainNumDescs<0 ? 0 : trainNumDescs),
        nextID_(0),
        progressPrint_(
                       trainNumDescs<0 ? numDocs : trainNumDescs,
                       std::string("trainDescsManager") + (trainNumDescs<0 ? "(images)" : "(descs)") ) {

      f_= fopen(trainDescsFn.c_str(), "wb");
      ASSERT(f_!=NULL);
      fwrite( &numDims, sizeof(numDims), 1, f_ );
      fwrite( &dtypeCode, sizeof(dtypeCode), 1, f_ );
    }

    ~trainDescsManager(){ fclose(f_); }

    void
    operator()( uint32_t jobID, trainDescsResult &result );

  private:
    bool const allDescs_;
    int64_t remainNumDescs_;
    uint32_t nextID_;
    std::map<uint32_t, trainDescsResult> results_;
    FILE *f_;
    timing::progressPrint progressPrint_;

    DISALLOW_COPY_AND_ASSIGN(trainDescsManager)
  };



  void
  trainDescsManager::operator()( uint32_t jobID, trainDescsResult &result ){
    if (stopJobs_){
      results_.clear();
      return;
    }
    // make sure results are saved sorted by job/docID!
    results_[jobID]= result;
    if (jobID==nextID_){

      // save the buffered results and remove them from the map
      for (std::map<uint32_t, trainDescsResult>::iterator it= results_.begin();
           (allDescs_ || remainNumDescs_!=0) && it!=results_.end() && it->first==nextID_;
           ++nextID_){

        trainDescsResult const &res= it->second;
        if (allDescs_)
          progressPrint_.inc();

        if (res.first>0) {
          uint32_t numToCopy= (allDescs_ || res.first < remainNumDescs_) ?
            res.first :
            static_cast<uint32_t>(remainNumDescs_);
          ASSERT( res.second.size() % res.first == 0 );
          // the numToCopy*res.second.size()/res.first is because we don't know size per scalar (could be uint8, float32..)
          fwrite( res.second.c_str(),
                  sizeof(char),
                  numToCopy*res.second.size()/res.first,
                  f_ );
          if (!allDescs_)
            progressPrint_.inc(numToCopy);
          remainNumDescs_-= numToCopy;
        }
        results_.erase(it++);
      }
    }
    stopJobs_= (!allDescs_ && remainNumDescs_==0);
  }



  class trainDescsWorker : public queueWorker<trainDescsResult> {
  public:

    trainDescsWorker(std::vector<std::string> const &imageFns,
                     std::string const trainDatabasePath,
                     featGetter const &featGetter_obj);

    void
    operator() ( uint32_t jobID, trainDescsResult &result ) const;

  private:

    std::vector<std::string> const *imageFns_;
    std::string const databasePath_;

    featGetter const *featGetter_;

    DISALLOW_COPY_AND_ASSIGN(trainDescsWorker)
  };



  trainDescsWorker::trainDescsWorker(
                                     std::vector<std::string> const &imageFns,
                                     std::string const trainDatabasePath,
                                     featGetter const &featGetter_obj)
    : imageFns_(&imageFns),
      databasePath_(trainDatabasePath),
      featGetter_(&featGetter_obj) {
  }



  void
  trainDescsWorker::operator() ( uint32_t jobID, trainDescsResult &result ) const {

    uint32_t docID= jobID;
    result= std::make_pair(0, "");

    // get filename
    std::string imageFn= databasePath_ + imageFns_->at(docID);

    // make sure the image exists and is readable
    std::pair<uint32_t, uint32_t> wh= std::make_pair(0,0);
    if (boost::filesystem::exists(imageFn) && boost::filesystem::is_regular_file(imageFn)){
      wh= imageUtil::getWidthHeight(imageFn);
    } else {
      std::cerr<<"buildWorkerSemiSorted::operator(): "<<imageFn<<" doesn't exist\n";
      return;
    }
    if (wh.first==0 && wh.second==0){
      std::cerr<<"buildWorkerSemiSorted::operator(): "<<imageFn<<" is corrupt or 0x0\n";
      return;
    }

    uint32_t numFeats;
    std::vector<ellipse> regions;
    float *descs;

    // extract features
    featGetter_->getFeats(imageFn.c_str(), numFeats, regions, descs);
    result.first= numFeats;
    if (numFeats==0){
      delete []descs;
      return;
    }

    result.second= featGetter_->getRawDescs(descs, numFeats);
    delete []descs;

  }



  void
  computeTrainDescs(
                    std::string const trainImagelistFn,
                    std::string const trainDatabasePath,
                    std::string const trainDescsFn,
                    int32_t const trainNumDescs,
                    featGetter const &featGetter_obj){

    MPI_GLOBAL_RANK;

    if (boost::filesystem::exists(trainDescsFn)){
      if (rank==0)
        std::cout<<"buildIndex::computeTrainDescs: trainDescs already exist ("<<trainDescsFn<<")\n";
      return;
    }

    bool useThreads= detectUseThreads();
    uint32_t numWorkerThreads= omp_get_max_threads();

    // read the list of training images and shuffle it

    std::vector<std::string> imageFns;

    if (rank==0){
      std::cout << "trainImagelistFn=" << trainImagelistFn << std::endl;
      std::ifstream fImagelist(trainImagelistFn.c_str());
      ASSERT(fImagelist.is_open());

      imageFns.reserve(100000);
      std::string imageFn;
      while( std::getline(fImagelist, imageFn) )
        imageFns.push_back(imageFn);

      sameRandomUint32 sr(100000, 43);
      sr.shuffle<std::string>(imageFns.begin(), imageFns.end());

    }

#ifdef RR_MPI
    if (!useThreads)
      boost::mpi::broadcast(comm, imageFns, 0);
#endif
    uint32_t nJobs= imageFns.size();

    // compute training descriptors

#ifdef RR_MPI
    if (!useThreads) comm.barrier();
#endif

    trainDescsManager *manager= (rank==0) ?
      new trainDescsManager(nJobs,
                            featGetter_obj.numDims(),
                            featGetter_obj.getDtypeCode(),
                            trainNumDescs,
                            trainDescsFn) :
      NULL;

    trainDescsWorker worker(imageFns, trainDatabasePath, featGetter_obj);

    if (useThreads) {
      std::cout << "OpenMPI: number of threads = " << numWorkerThreads << std::endl;
      threadQueue<trainDescsResult>::start( nJobs, worker, *manager, numWorkerThreads );
    }
    else {
      mpiQueue<trainDescsResult>::start( nJobs, worker, manager );
    }

    if (rank==0) delete manager;
  }

};
