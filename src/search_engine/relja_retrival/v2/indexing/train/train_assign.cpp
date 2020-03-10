/*
  ==== Author:

  Relja Arandjelovic (relja@robots.ox.ac.uk)
  Visual Geometry Group,
  Department of Engineering Science
  University of Oxford

  ==== Copyright:

  The library belongs to Relja Arandjelovic and the University of Oxford.
  No usage or redistribution is allowed without explicit permission.

Updates:
 - 2 Aug. 2019: removed dependency on fastann for nearest neighbour search (Abhishek Dutta)

*/

#include "train_assign.h"

#include <stdint.h>
#include <vector>

#include <boost/filesystem.hpp>

#include "clst_centres.h"
#include "flat_desc_file.h"
#include "mpi_queue.h"
#include "par_queue.h"
#include "timing.h"
#include "util.h"

// for kd-tree based nearest neighbour search
#include <vl/generic.h>
#include <vl/kdtree.h>

namespace buildIndex {


  typedef std::vector<uint32_t> trainAssignsResult; // clusterIDs



  class trainAssignsManager : public managerWithTiming<trainAssignsResult> {
  public:

    trainAssignsManager(uint32_t numDocs, std::string const trainAssignsFn, vise::task_progress *progress)
      : managerWithTiming<trainAssignsResult>(numDocs, "trainAssignsManager"),
        nextID_(0),
        d_progress(progress)
    {
      f_= fopen(trainAssignsFn.c_str(), "wb");
      ASSERT(f_!=NULL);
    }

    ~trainAssignsManager(){ fclose(f_); }

    void
    compute( uint32_t jobID, trainAssignsResult &result );

  private:
    FILE *f_;
    uint32_t nextID_;
    std::map<uint32_t, trainAssignsResult> results_;
    vise::task_progress *d_progress;

    DISALLOW_COPY_AND_ASSIGN(trainAssignsManager)
  };



  void
  trainAssignsManager::compute( uint32_t jobID, trainAssignsResult &result ){
    // make sure results are saved sorted by job!
    results_[jobID]= result;
    if (jobID==nextID_){

      // save the buffered results and remove them from the map
      for (std::map<uint32_t, trainAssignsResult>::iterator it= results_.begin();
           it!=results_.end() && it->first==nextID_;
           ++nextID_){

        trainAssignsResult const &res= it->second;
        fwrite( &res[0],
                sizeof(uint32_t),
                res.size(),
                f_ );

        results_.erase(it++);
      }
      if(d_progress != nullptr) {
        d_progress->add(1);
      }
    }
  }



  class trainAssignsWorker : public queueWorker<trainAssignsResult> {
  public:

    trainAssignsWorker(VlKDForest* const kd_forest,
                       flatDescsFile const &descFile,
                       uint32_t chunkSize,
                       uint32_t numDims)
      : kd_forest_(kd_forest),
        descFile_(&descFile),
        chunkSize_(chunkSize),
        numDescs_(descFile.numDescs()),
        numDims_(numDims)
    {}

    void
    operator() ( uint32_t jobID, trainAssignsResult &result ) const;

  private:

    VlKDForest* const kd_forest_;
    flatDescsFile const *descFile_;
    uint32_t const chunkSize_, numDescs_, numDims_;

    DISALLOW_COPY_AND_ASSIGN(trainAssignsWorker)
  };



  void
  trainAssignsWorker::operator() ( uint32_t jobID, trainAssignsResult &result ) const {

    result.clear();

    uint32_t start= jobID*chunkSize_;
    uint32_t end= std::min( (jobID+1)*chunkSize_, numDescs_ );

    float *descs;
    descFile_->getDescs(start, end, descs);

    result.resize(end-start);

    VlKDForestSearcher* kd_forest_searcher = vl_kdforest_new_searcher(kd_forest_);
    VlKDForestNeighbor cluster;
    for ( uint32_t i = 0; i < (end - start); ++i ) {
      vl_kdforestsearcher_query(kd_forest_searcher, &cluster, 1, (descs + i*numDims_));
      result[i] = cluster.index;
    }
    delete []descs;
  }



  void
  computeTrainAssigns(
                      std::string const clstFn,
                      bool const RootSIFT,
                      std::string const trainDescsFn,
                      std::string const trainAssignsFn,
                      vise::task_progress *progress){

    MPI_GLOBAL_ALL;

    if (boost::filesystem::exists(trainAssignsFn)){
      if (rank==0)
        std::cout<<"buildIndex::computeTrainAssigns: trainAssignsFn already exist ("<<trainAssignsFn<<")\n";
      return;
    }
    ASSERT( boost::filesystem::exists(trainDescsFn) );

    bool useThreads= detectUseThreads();
    uint32_t numWorkerThreads= omp_get_max_threads();

    // clusters
    if (rank==0)
      std::cout<<"buildIndex::computeTrainAssigns: Loading cluster centres\n";
    double t0= timing::tic();
    clstCentres clstCentres_obj( clstFn.c_str(), true );
    if (rank==0)
      std::cout<<"buildIndex::computeTrainAssigns: Loading cluster centres - DONE ("<< timing::toc(t0) <<" ms)\n";

    if (rank==0)
      std::cout<<"buildIndex::computeTrainAssigns: Constructing NN search object\n";
    t0= timing::tic();

    // build kd-tree for nearest neighbour search
    // to assign cluster-id for each descriptor
    std::size_t num_trees = 8;
    std::size_t max_num_checks = 1024;
    VlKDForest* kd_forest = vl_kdforest_new( VL_TYPE_FLOAT, clstCentres_obj.numDims, num_trees, VlDistanceL2 );
    vl_kdforest_set_max_num_comparisons(kd_forest, max_num_checks);
    vl_kdforest_build(kd_forest, clstCentres_obj.numClst, clstCentres_obj.clstC_flat);

    if (rank==0)
      std::cout<<"buildIndex::computeTrainAssigns: Constructing NN search object - DONE ("<< timing::toc(t0) << " ms)\n";

    flatDescsFile const descFile(trainDescsFn, RootSIFT);
    uint32_t const numTrainDescs= descFile.numDescs();
    if (rank==0)
      std::cout<<"buildIndex::computeTrainAssigns: numTrainDescs= "<<numTrainDescs<<"\n";

    uint32_t const chunkSize=
      std::min( static_cast<uint32_t>(10000),
                static_cast<uint32_t>(
                                      std::ceil(static_cast<double>(numTrainDescs)/std::max(numWorkerThreads, numProc))) );
    uint32_t const nJobs= static_cast<uint32_t>( std::ceil(static_cast<double>(numTrainDescs)/chunkSize) );
    if(progress != nullptr) {
      progress->start(0, nJobs);
    }

    // assign training descriptors

#ifdef RR_MPI
    if (!useThreads) comm.barrier();
#endif

    trainAssignsManager *manager= (rank==0) ?
      new trainAssignsManager(nJobs, trainAssignsFn, progress) :
      NULL;

    trainAssignsWorker worker(kd_forest, descFile, chunkSize, clstCentres_obj.numDims);

    if (useThreads)
      threadQueue<trainAssignsResult>::start( nJobs, worker, *manager, numWorkerThreads );
    else
      mpiQueue<trainAssignsResult>::start( nJobs, worker, manager );

    if (rank==0) delete manager;
    vl_kdforest_delete(kd_forest);
  }

};
