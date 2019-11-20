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

#include "build_index.h"

#include <fstream>
#include <queue>
#include <vector>

#include <boost/filesystem.hpp>

#ifdef RR_MPI
#include <boost/mpi/collectives.hpp>
#include <boost/serialization/utility.hpp> // for std::pair
#include <boost/serialization/string.hpp>
#endif

#include <boost/thread.hpp>

#include "build_index_status.pb.h"
#include "clst_centres.h"
#include "dataset_v2.h"
#include "image_util.h"
#include "index_entry_util.h"
#include "mpi_queue.h"
#include "par_queue.h"
#include "protobuf_util.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"
#include "util.h"

// for kd-tree based nearest neighbour search
#include <vl/generic.h>
#include <vl/kdtree.h>

namespace buildIndex {

  static const int semiSortedProtoByteSizeLim= 50000000; // 50 MB
  static const uint64_t semiSortedPartByteSizeLim= 1000000000; // 1 GB

  static const int sortedProtoByteSizeLimMax= 50000000; // 50 MB
  static const uint32_t mergingMemoryLim= 1500000000; // 1.5 GB

  static const int mergedProtoByteSizeLim= 50000000; // 50 MB



  bool
  loadStatus(std::string const indexingStatusFn, rr::buildIndexStatus &status){
    if (boost::filesystem::exists( indexingStatusFn )){
      std::ifstream in(indexingStatusFn.c_str(), std::ios::binary);
      ASSERT(in.is_open());
      ASSERT(status.ParseFromIstream(&in));
      in.close();
      return true;
    }
    return false;
  }



  void
  saveStatus(std::string const indexingStatusFn, rr::buildIndexStatus const &status){
    std::ofstream of(indexingStatusFn.c_str(), std::ios::binary);
    ASSERT(of.is_open());
    status.SerializeToOstream(&of);
    of.close();
  }



  class buildManagerFiles : public managerWithTiming<std::string> {
  public:

    buildManagerFiles(uint32_t nJobs, std::string const desc) : managerWithTiming<std::string>(nJobs, desc) {}

    void
    compute( uint32_t jobID, std::string &result ){
      fns_.push_back(result);
    }

    std::vector<std::string> fns_;
  };




  class orderIDs {
  public:

#define IES_COMPARE(fieldName)                                          \
    if (entries_->at(l.first).fieldName(l.second) > entries_->at(r.first).fieldName(r.second)) \
      return true;                                                      \
    if (entries_->at(l.first).fieldName(l.second) < entries_->at(r.first).fieldName(r.second)) \
      return false;

#define IES_COMPARESTR(fieldName)                                       \
    if (entries_->at(l.first).fieldName()[l.second] > entries_->at(r.first).fieldName()[r.second]) \
      return true;                                                      \
    if (entries_->at(l.first).fieldName()[l.second] < entries_->at(r.first).fieldName()[r.second]) \
      return false;

#define IES_COMPARE_IF_EXISTS(fieldName)                \
    if (entries_->at(l.first).fieldName ## _size()){    \
      IES_COMPARE(fieldName)                            \
        }

#define IES_COMPARESTR_IF_EXISTS(fieldName)             \
    if (entries_->at(l.first).fieldName().length()>0){  \
      IES_COMPARESTR(fieldName)                         \
        }
    orderIDs(std::vector<rr::indexEntry> const &entries) : entries_(&entries) {}

    bool operator()(std::pair<uint32_t,int> const &l, std::pair<uint32_t,int> const &r) const {

      IES_COMPARE(id)

        IES_COMPARE_IF_EXISTS(docid)
        IES_COMPARE_IF_EXISTS(x)
        IES_COMPARE_IF_EXISTS(qx)
        IES_COMPARE_IF_EXISTS(y)
        IES_COMPARE_IF_EXISTS(qy)
        IES_COMPARE_IF_EXISTS(a)
        IES_COMPARE_IF_EXISTS(b)
        IES_COMPARE_IF_EXISTS(c)
        IES_COMPARESTR_IF_EXISTS(qel_scale)
        IES_COMPARESTR_IF_EXISTS(qel_ratio)
        IES_COMPARESTR_IF_EXISTS(qel_angle)

        return true;
    }

  private:
    std::vector<rr::indexEntry> const *entries_;
  };



  class orderFidxIDs {

  public:
    orderFidxIDs(std::vector<uint32_t> const &IDs) : IDs_(&IDs) {}

    bool operator() (uint32_t l, uint32_t r) const {
      return IDs_->at(l) > IDs_->at(r);
    }

  private:
    std::vector<uint32_t> const *IDs_;
  };



  // ------------------------------------
  // ------------------------------------ SemiSorted
  // ------------------------------------



  typedef std::pair< std::string, std::pair<uint32_t, uint32_t> > buildResultSemiSorted;



  class buildManagerSemiSorted : public managerWithTiming<buildResultSemiSorted> {
  public:

    buildManagerSemiSorted(uint32_t numDocs, std::string const dsetFn)
      : managerWithTiming<buildResultSemiSorted>(numDocs, "buildManagerSemiSorted"),
        dsetBuilder_(dsetFn),
        nextID_(0)
    {}

    void
    compute( uint32_t jobID, buildResultSemiSorted &result );

  private:
    datasetBuilder dsetBuilder_;
    uint32_t nextID_;
    std::map<uint32_t, buildResultSemiSorted> results_;

    DISALLOW_COPY_AND_ASSIGN(buildManagerSemiSorted)
  };



  void
  buildManagerSemiSorted::compute( uint32_t jobID, buildResultSemiSorted &result ){
    // make sure results are saved sorted by job/docID!
    results_[jobID]= result;
    if (jobID==nextID_){
      // save the buffered results and remove them from the map
      for (std::map<uint32_t, buildResultSemiSorted>::iterator it= results_.begin();
           it!=results_.end() && it->first==nextID_;
           ++nextID_){
        buildResultSemiSorted const &res= it->second;
        dsetBuilder_.add( res.first,
                          res.second.first,
                          res.second.second );
        results_.erase(it++);
      }
    }
  }



  class buildWorkerSemiSorted : public queueWorker<buildResultSemiSorted> {
  public:

    buildWorkerSemiSorted(std::string const outDir,
                          std::string const imagelistFn, std::string const databasePath,
                          featGetter const &featGetter_obj,
                          VlKDForest* const kd_forest,
                          clstCentres const *clstCentres_obj= NULL,
                          embedderFactory const *embFactory= NULL);

    ~buildWorkerSemiSorted() {
      finish();
      if (delEmbF_) delete embFactory_;
      delete emb_;
    }

    void
    finish(){
      if (indexEntry_.id_size()>0)
        save();
      if (dbBuilder_!=NULL){
        ASSERT( indexBuilder_!= NULL );
        closeFile();
      }
      findexBuilder_.close();
      fImagelist_.close();
    }

    void
    operator() ( uint32_t jobID, buildResultSemiSorted &result ) const;

    mutable std::vector<std::string> fns_;
    std::string const fidx_fn_;
    mutable uint64_t totalFeats_;

  private:

    void
    closeFile() const;

    void
    save() const;

    mutable std::ifstream fImagelist_;
    std::string const databasePath_;

    featGetter const *featGetter_;
    uint32_t const numDims_;
    VlKDForest* const kd_forest_;
    clstCentres const *clstCentres_;
    embedderFactory const *embFactory_;
    bool delEmbF_;
    embedder *emb_;
    mutable rr::indexEntry indexEntry_;

    std::string const outDir_;
    mutable protoDbFileBuilder *dbBuilder_;
    mutable indexBuilder *indexBuilder_;
    mutable protoDbFileBuilder fdbBuilder_;
    mutable indexBuilder findexBuilder_;

    mutable uint32_t nextPossibleID_;

    DISALLOW_COPY_AND_ASSIGN(buildWorkerSemiSorted)
  };



  buildWorkerSemiSorted::buildWorkerSemiSorted(
                                               std::string const outDir,
                                               std::string const imagelistFn, std::string const databasePath,
                                               featGetter const &featGetter_obj,
                                               VlKDForest* const kd_forest,
                                               clstCentres const *clstCentres_obj,
                                               embedderFactory const *embFactory)
  : fidx_fn_( util::getTempFileName( outDir, "fidxpart_", ".bin" ) ),
    fImagelist_(imagelistFn.c_str()),
    databasePath_(databasePath),
    featGetter_(&featGetter_obj),
    numDims_(featGetter_obj.numDims()),
    kd_forest_(kd_forest),
    clstCentres_(clstCentres_obj),
    outDir_(outDir),
    dbBuilder_(NULL),
    indexBuilder_(NULL),
    fdbBuilder_(fidx_fn_, "indexing fidx"),
    findexBuilder_(fdbBuilder_, true, true, true) {
    ASSERT(featGetter_obj.numDims() == clstCentres_obj->numDims);
    nextPossibleID_= 0;
    totalFeats_= 0;
    if (embFactory==NULL){
      embFactory_= new noEmbedderFactory;
      delEmbF_= true;
    } else {
      embFactory_= embFactory;
      delEmbF_= false;
    }
    emb_= embFactory_->getEmbedder();
  }



  void
  buildWorkerSemiSorted::operator() ( uint32_t jobID, buildResultSemiSorted &result ) const {

    uint32_t docID= jobID;

    // get filename
    ASSERT(nextPossibleID_<=docID);
    std::string imageFn;
    for(; nextPossibleID_<=docID; ++nextPossibleID_)
      ASSERT( std::getline(fImagelist_, imageFn) );
    result.first= imageFn;
    imageFn= databasePath_ + imageFn;

    // make sure the image exists and is readable
    result.second= std::make_pair(0,0);
    if (boost::filesystem::exists(imageFn) && boost::filesystem::is_regular_file(imageFn)){
      result.second= imageUtil::getWidthHeight(imageFn);
    } else {
      std::cerr<<"buildWorkerSemiSorted::operator(): "<<imageFn<<" doesn't exist\n";
      return;
    }
    if (result.second.first==0 && result.second.second==0){
      std::cerr<<"buildWorkerSemiSorted::operator(): "<<imageFn<<" is corrupt or 0x0\n";
      return;
    }

    uint32_t numFeats;
    std::vector<ellipse> regions;
    float *descs;

    // extract features
    featGetter_->getFeats(imageFn.c_str(), numFeats, regions, descs);
    if (numFeats==0){
      delete []descs;
      return;
    }
    totalFeats_+= numFeats;

    // prepare memory
    uint32_t reserveCount= static_cast<uint32_t>(indexEntry_.id_size()) + numFeats;
    google::protobuf::RepeatedField<uint32_t> *wordIDs= indexEntry_.mutable_id();
    google::protobuf::RepeatedField<uint32_t> *qx= indexEntry_.mutable_qx();
    google::protobuf::RepeatedField<uint32_t> *qy= indexEntry_.mutable_qy();
    google::protobuf::RepeatedField<float> *a= indexEntry_.mutable_a();
    google::protobuf::RepeatedField<float> *b= indexEntry_.mutable_b();
    google::protobuf::RepeatedField<float> *c= indexEntry_.mutable_c();
    wordIDs->Reserve(reserveCount);
    qx->Reserve(reserveCount);
    qy->Reserve(reserveCount);
    a->Reserve(reserveCount);
    b->Reserve(reserveCount);
    c->Reserve(reserveCount);
    std::vector<uint32_t> wordIDsUnique;
    wordIDsUnique.reserve(numFeats);
    emb_->reserve(numFeats);

    // add docID
    protobufUtil::addManyToEnd<uint32_t>( docID, numFeats, *(indexEntry_.mutable_docid()) );

    float *itD= descs;

    // assign each feature to a clusters
    VlKDForestSearcher* kd_forest_searcher = vl_kdforest_new_searcher(kd_forest_);

    VlKDForestNeighbor cluster;
    for (uint32_t iFeat=0; iFeat<numFeats; ++iFeat){
      ellipse const &region= regions[iFeat];

      vl_kdforestsearcher_query(kd_forest_searcher, &cluster, 1, (descs + iFeat*numDims_));
      wordIDsUnique.push_back( cluster.index );
      wordIDs->AddAlreadyReserved( cluster.index );
      qx->AddAlreadyReserved( round(region.x) );
      qy->AddAlreadyReserved( round(region.y) );
      a->AddAlreadyReserved( region.a );
      b->AddAlreadyReserved( region.b );
      c->AddAlreadyReserved( region.c );

      if (emb_->doesSomething()){
        float *thisDesc= itD;
        float const *itC= clstCentres_->clstC_flat + cluster.index * numDims_;
        float const *endC= itC + numDims_;
        for (; itC!=endC; ++itC, ++itD)
          *itD -= *itC;
        emb_->add(thisDesc, cluster.index);
      }

    }

    // cleanup
    delete []descs;

    // protobufs are not designed for more
    if (indexEntry_.ByteSize() + static_cast<int>(emb_->getByteSize()) > semiSortedProtoByteSizeLim)
      save();

    // save fidx
    std::sort(wordIDsUnique.begin(), wordIDsUnique.end());
    std::vector<uint32_t>::const_iterator newEnd= std::unique(wordIDsUnique.begin(), wordIDsUnique.end());
    rr::indexEntry fidxEntry;
    google::protobuf::RepeatedField<uint32_t> *fidxWordID= fidxEntry.mutable_id();
    fidxWordID->Reserve(newEnd - wordIDsUnique.begin());

    for (std::vector<uint32_t>::const_iterator it= wordIDsUnique.begin();
         it!=newEnd;
         ++it){
      fidxWordID->AddAlreadyReserved(*it);
    }
    findexBuilder_.addEntry(docID, fidxEntry);
  }



  void
  buildWorkerSemiSorted::save() const {

    // sort according to clusterID
    std::vector<int> inds;
    indexEntryUtil::argSort::sort(indexEntry_, inds);

    // apply the sort
    rr::indexEntry indexEntry= indexEntry_; // to get all repeated field sizes correctly
    uint32_t *wordID= indexEntry.mutable_id()->mutable_data();
    uint32_t *docID= indexEntry.mutable_docid()->mutable_data();
    uint32_t *qx= indexEntry.mutable_qx()->mutable_data();
    uint32_t *qy= indexEntry.mutable_qy()->mutable_data();
    float *a= indexEntry.mutable_a()->mutable_data();
    float *b= indexEntry.mutable_b()->mutable_data();
    float *c= indexEntry.mutable_c()->mutable_data();
    uint32_t size= indexEntry_.id_size();
    embedder *emb= embFactory_->getEmbedder();

    for (uint32_t i= 0; i<size; ++i, ++wordID, ++docID, ++qx, ++qy, ++a, ++b, ++c){
      int ind= inds[i];
      *wordID= indexEntry_.id(ind);
      *docID= indexEntry_.docid(ind);
      *qx= indexEntry_.qx(ind);
      *qy= indexEntry_.qy(ind);
      *a= indexEntry_.a(ind);
      *b= indexEntry_.b(ind);
      *c= indexEntry_.c(ind);
      emb->copyFrom(*emb_, ind);
    }

    // clear for future usage (will clear indexEntry_ later)
    emb_->clear();

    // dump extra data into indexEntry_
    if (emb->getByteSize()>0)
      indexEntry.set_data( emb->getEncoding() );
    delete emb;

    // do we need to make a new file?
    if (dbBuilder_==NULL){
      ASSERT( indexBuilder_==NULL );
      fns_.push_back( util::getTempFileName( outDir_, "semisortedpart_", ".bin" ) );
      dbBuilder_= new protoDbFileBuilder( fns_.back(), "indexing" );
      indexBuilder_= new indexBuilder(*dbBuilder_, true, true, true);
    }

    // save to disk
    indexBuilder_->addEntry( 0, indexEntry );

    // clear for future usage (cleared emb_ already)
    indexEntry_.Clear();

    // is the current file too big?
    if (dbBuilder_!=NULL){
      ASSERT( indexBuilder_!= NULL );
      if ( util::fileSize(dbBuilder_->getFn()) > semiSortedPartByteSizeLim )
        closeFile();
    }
  }



  void
  buildWorkerSemiSorted::closeFile() const {
    delete indexBuilder_;
    delete dbBuilder_;
    dbBuilder_= NULL;
    indexBuilder_= NULL;
  }



  // ------------------------------------
  // ------------------------------------ Sorted
  // ------------------------------------



  class buildWorkerSorted : public queueWorker<std::string> {
  public:

    buildWorkerSorted(std::string const outDir,
                      std::vector<std::string> const &inputFns,
                      uint32_t protoByteSizeLim,
                      embedderFactory const *embFactory= NULL);

    ~buildWorkerSorted(){
      if (delEmbF_) delete embFactory_;
    }

    void
    operator() ( uint32_t jobID, std::string &result ) const;

    mutable std::vector<std::string> fns_;

  private:

    std::string const outDir_;
    std::vector<std::string> const *inputFns_;
    int const sortedProtoByteSizeLim_;
    embedderFactory const *embFactory_;
    bool delEmbF_;

    DISALLOW_COPY_AND_ASSIGN(buildWorkerSorted)
  };



  buildWorkerSorted::buildWorkerSorted(
                                       std::string const outDir,
                                       std::vector<std::string> const &inputFns,
                                       uint32_t protoByteSizeLim,
                                       embedderFactory const *embFactory) :
    outDir_(outDir),
    inputFns_(&inputFns),
    sortedProtoByteSizeLim_(static_cast<int>(std::min(
                                                      static_cast<uint32_t>(sortedProtoByteSizeLimMax),
                                                      protoByteSizeLim) )) {
    if (embFactory==NULL){
      embFactory_= new noEmbedderFactory;
      delEmbF_= true;
    } else {
      embFactory_= embFactory;
      delEmbF_= false;
    }
  }



  void
  buildWorkerSorted::operator() ( uint32_t jobID, std::string &result ) const {

    std::vector<rr::indexEntry> entries;
    uint32_t ID_fake= 0;
    {
      // load the entire input file
      protoDbFile inDb( inputFns_->at(jobID) );
      protoIndex inIdx(inDb, false);
      ASSERT( inIdx.numIDs() == 1 );

      inIdx.getEntries(0, entries);
    }

    // make the file
    result= util::getTempFileName( outDir_, "sortedpart_", ".bin" );
    protoDbFileBuilder dbBuilder(result, "indexing");
    indexBuilder idxBuilder(dbBuilder, true, true, true);

    // make the heap
    // entries[first].id(second)
    std::priority_queue< std::pair<uint32_t,int>,
                         std::vector< std::pair<uint32_t,int> >,
                         orderIDs > queue( (orderIDs(entries)) );
    std::vector<embedder*> embedders(entries.size());

    for (uint32_t iEntry= 0; iEntry < entries.size(); ++iEntry)
      if (entries[iEntry].id_size()>0){

        queue.push(std::make_pair(iEntry, 0));
        rr::indexEntry const &entry= entries[iEntry];
        int n= entry.id_size();
        ASSERT( n == entry.docid_size() );
        ASSERT( n == entry.qx_size() );
        ASSERT( n == entry.qy_size() );
        ASSERT( n == static_cast<int>(entry.qel_scale().length()) );
        ASSERT( n == static_cast<int>(entry.qel_ratio().length()) );
        ASSERT( n == static_cast<int>(entry.qel_angle().length()) );
        ASSERT( entry.a_size()==0 );
        ASSERT( entry.b_size()==0 );
        ASSERT( entry.c_size()==0 );

        embedders[iEntry]= embFactory_->getEmbedder();
        if ( embedders[iEntry]->doesSomething() ){
          ASSERT(entry.has_data());
          embedders[iEntry]->setDataCopy(entry.data());
          ASSERT( n == static_cast<int>(embedders[iEntry]->getNum()) );
        }
        entries[iEntry].clear_data(); // to save RAM

      } else embedders[iEntry]= NULL;

    rr::indexEntry merged;
    merged.mutable_id()->Reserve(100000);
    merged.mutable_docid()->Reserve(100000);
    merged.mutable_qx()->Reserve(100000);
    merged.mutable_qy()->Reserve(100000);
    merged.mutable_qel_scale()->reserve(100000);
    merged.mutable_qel_ratio()->reserve(100000);
    merged.mutable_qel_angle()->reserve(100000);
    embedder *emb= embFactory_->getEmbedder();
    emb->reserve(100000);

    while (queue.size()){
      // get smallest ID
      std::pair<uint32_t,int> const &t= queue.top();
      uint32_t iEntry= t.first;
      int ind= t.second;
      queue.pop();

      // add data from the entry
      rr::indexEntry const &entry= entries[iEntry];
      merged.add_id( entry.id(ind) );
      merged.add_docid( entry.docid(ind) );
      merged.add_qx( entry.qx(ind) );
      merged.add_qy( entry.qy(ind) );
      merged.mutable_qel_scale()->append( &(entry.qel_scale()[ind]), 1 );
      merged.mutable_qel_ratio()->append( &(entry.qel_ratio()[ind]), 1 );
      merged.mutable_qel_angle()->append( &(entry.qel_angle()[ind]), 1 );
      emb->copyFrom(*embedders[iEntry], ind);

      // protobufs are not designed for more
      if (merged.id_size()%100==0 && // to avoid doing ByteSize() all the time
          merged.ByteSize() + static_cast<int>(emb->getByteSize()) > sortedProtoByteSizeLim_){
        if (emb->doesSomething())
          merged.set_data(emb->getEncoding());
        emb->clear();
        idxBuilder.addEntry(ID_fake, merged);
        ++ID_fake;
        merged.Clear();
      }

      // push next if we haven't reached the end
      ++ind;
      if (ind < entry.id_size())
        queue.push(std::make_pair(iEntry, ind));
      else {
        delete embedders[iEntry];
        embedders[iEntry]= NULL;
      }
    }

    if (merged.id_size()>0){
      // save last time
      if (emb->doesSomething())
        merged.set_data(emb->getEncoding());
      emb->clear();
      idxBuilder.addEntry(ID_fake, merged);
      merged.Clear();
    }

    idxBuilder.close();
    delete emb;

    for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry)
      ASSERT(embedders[iEntry]==NULL);
  }



  // ------------------------------------
  // ------------------------------------ Merged
  // ------------------------------------



  void
  mergeSortedFiles(
                   std::vector<std::string> const &fns,
                   std::string const iidxFn,
                   uint32_t const totalFeats,
                   embedderFactory const *embFactory= NULL){
    bool delEmbF= false;
    if (embFactory==NULL){
      embFactory= new noEmbedderFactory;
      delEmbF= true;
    }

    uint32_t const numFiles= fns.size();
    std::vector< protoDbFile* > inDbs;
    std::vector< protoIndex* > inIdxs;
    std::vector<uint32_t> ID_fake;
    std::vector<rr::indexEntry> entries, entries_temp;

    timing::progressPrint progressPrint(totalFeats, "buildIndex::mergeSortedFiles");

    // the heap
    // entries[first].id(second)
    std::priority_queue< std::pair<uint32_t,int>,
                         std::vector< std::pair<uint32_t,int> >,
                         orderIDs > queue( (orderIDs(entries)) );
    std::vector<embedder*> embedders(numFiles);

    // load first IDs of all files, add the first element to the heap
    for (uint32_t iEntry= 0; iEntry<numFiles; ++iEntry){
      inDbs.push_back( new protoDbFile(fns[iEntry]) );
      inIdxs.push_back( new protoIndex(*inDbs.back(), false) );
      ID_fake.push_back(0);

      if (inDbs.back()->numIDs()>0){

        inIdxs.back()->getEntries(0, entries_temp);
        ASSERT( entries_temp.size()==1 );
        rr::indexEntry const &entry= entries_temp[0];
        int n= entry.id_size();
        ASSERT( n == entry.docid_size() );
        ASSERT( n == entry.qx_size() );
        ASSERT( n == entry.qy_size() );
        ASSERT( n == static_cast<int>(entry.qel_scale().length()) );
        ASSERT( n == static_cast<int>(entry.qel_ratio().length()) );
        ASSERT( n == static_cast<int>(entry.qel_angle().length()) );
        ASSERT( entry.a_size()==0 );
        ASSERT( entry.b_size()==0 );
        ASSERT( entry.c_size()==0 );

        embedders[iEntry]= embFactory->getEmbedder();
        if ( embedders[iEntry]->doesSomething() ){
          ASSERT(entry.has_data());
          embedders[iEntry]->setDataCopy(entry.data());
          ASSERT( n == static_cast<int>(embedders[iEntry]->getNum()) );
        }
        entries_temp[0].clear_data(); // to save RAM

        entries.push_back( entry );
        queue.push(std::make_pair(iEntry, 0)); // make sure to do this AFTER setting entries so that sorting works

      } else embedders[iEntry]= NULL;
    }

    uint32_t ID= 0, prevID= 0;

    // do merging
    protoDbFileBuilder dbBuilder(iidxFn, "index");
    indexBuilder idxBuilder(dbBuilder, true, true, true);

    rr::indexEntry merged;
    merged.mutable_id()->Reserve(100000);
    merged.mutable_qx()->Reserve(100000);
    merged.mutable_qy()->Reserve(100000);
    merged.mutable_qel_scale()->reserve(100000);
    merged.mutable_qel_ratio()->reserve(100000);
    merged.mutable_qel_angle()->reserve(100000);
    embedder *emb= embFactory->getEmbedder();
    emb->reserve(100000);

    while (queue.size()){

      progressPrint.inc();

      // get smallest ID
      std::pair<uint32_t,int> const &t= queue.top();
      uint32_t iEntry= t.first;
      int ind= t.second;
      queue.pop();

      // add data from the entry
      rr::indexEntry const &entry= entries[iEntry];
      ID= entry.id(ind);

      ASSERT(ID>=prevID);
      if (ID>prevID && merged.id_size()>0){
        // save the current one as ID changed
        if (emb->doesSomething())
          merged.set_data(emb->getEncoding());
        emb->clear();
        idxBuilder.addEntry(prevID, merged);
        merged.Clear();
        prevID= ID;
      }

      merged.add_id( entry.docid(ind) );
      merged.add_qx( entry.qx(ind) );
      merged.add_qy( entry.qy(ind) );
      merged.mutable_qel_scale()->append( &(entry.qel_scale()[ind]), 1 );
      merged.mutable_qel_ratio()->append( &(entry.qel_ratio()[ind]), 1 );
      merged.mutable_qel_angle()->append( &(entry.qel_angle()[ind]), 1 );
      emb->copyFrom(*embedders[iEntry], ind);

      // protobufs are not designed for more
      if (merged.id_size()%100==0 && // to avoid doing ByteSize() all the time
          merged.ByteSize() > mergedProtoByteSizeLim){
        if (emb->doesSomething())
          merged.set_data(emb->getEncoding());
        emb->clear();
        idxBuilder.addEntry(ID, merged);
        merged.Clear();
      }

      // push next
      ++ind;
      if (ind < entry.id_size())
        // if we haven't reached the end
        queue.push(std::make_pair(iEntry, ind));
      else {
        // load the next entry
        ++ID_fake[iEntry];

        delete embedders[iEntry];
        embedders[iEntry]= NULL;

        if (ID_fake[iEntry] < inDbs[iEntry]->numIDs()){

          inIdxs[iEntry]->getEntries(ID_fake[iEntry], entries_temp);
          ASSERT( entries_temp.size()==1 );
          rr::indexEntry const &entry= entries_temp[0];
          int n= entry.id_size();
          ASSERT( n == entry.docid_size() );
          ASSERT( n == entry.qx_size() );
          ASSERT( n == entry.qy_size() );
          ASSERT( n == static_cast<int>(entry.qel_scale().length()) );
          ASSERT( n == static_cast<int>(entry.qel_ratio().length()) );
          ASSERT( n == static_cast<int>(entry.qel_angle().length()) );
          ASSERT( entry.a_size()==0 );
          ASSERT( entry.b_size()==0 );
          ASSERT( entry.c_size()==0 );

          embedders[iEntry]= embFactory->getEmbedder();
          if ( embedders[iEntry]->doesSomething() ){
            ASSERT(entry.has_data());
            embedders[iEntry]->setDataCopy(entry.data());
            ASSERT( n == static_cast<int>(embedders[iEntry]->getNum()) );
          }
          entries_temp[0].clear_data(); // to save RAM

          entries[iEntry]= entry;
          queue.push(std::make_pair(iEntry, 0)); // make sure to do this AFTER setting entries so that sorting works

        }
      }

    }

    if (merged.id_size()>0){
      // save last time
      if (emb->doesSomething())
        merged.set_data(emb->getEncoding());
      emb->clear();
      idxBuilder.addEntry(ID, merged);
      merged.Clear();
    }

    idxBuilder.close();

    util::delPointerVector(inIdxs);
    util::delPointerVector(inDbs);
    if (delEmbF) delete embFactory;
    delete emb;
    for (uint32_t iEntry= 0; iEntry<numFiles; ++iEntry)
      ASSERT(embedders[iEntry]==NULL);

    std::cout<<"buildIndex::mergeSortedFiles: done\n";

    ASSERT( progressPrint.totalDone() == totalFeats );

    for (uint32_t i= 0; i<fns.size(); ++i)
      boost::filesystem::remove(fns[i]);

  }



  void
  mergePartialFidx(std::vector<std::string> const &fns, std::string const fidxFn) {
    uint32_t const numFiles= fns.size();
    std::vector< protoDbFile* > inDbs;
    std::vector<uint32_t> IDs(numFiles);
    std::vector<rr::indexEntry> entries;

    // the heap
    // entries[first].id(second)
    std::priority_queue< uint32_t,
                         std::vector<uint32_t>,
                         orderFidxIDs > queue( (orderFidxIDs(IDs)) );

    // add available IDs from all files to the heap
    for (uint32_t iFile= 0; iFile<numFiles; ++iFile){
      inDbs.push_back( new protoDbFile(fns[iFile]) );
      protoDbFile const *db= inDbs.back();
      uint32_t ID= 0;
      for (ID= 0; ID<db->numIDs() && !(db->contains(ID)); ++ID);
      if (ID < db->numIDs()){
        IDs[iFile]= ID;
        queue.push(iFile);
      }
    }

    uint32_t ID= 0, nextID= 0;
    std::vector<std::string> data;
    uint32_t numNoFeatsWarnings= 0;

    // do merging
    protoDbFileBuilder dbBuilder(fidxFn, "index");

    while (queue.size()){

      // get smallest ID
      uint32_t iFile= queue.top();
      queue.pop();
      ID= IDs[iFile];
      ASSERT(ID>=nextID);
      for (; numNoFeatsWarnings<5 && nextID<ID; ++nextID, ++numNoFeatsWarnings)
        std::cout<<"buildIndex::mergePartialFidx: warning no features detected in imageID="<<nextID<<"\n";
      nextID= ID+1;

      // get/add data
      protoDbFile const *db= inDbs[iFile];
      db->getData(ID, data);
      bool hasFeatures= false;
      for (uint32_t i= 0; i<data.size(); ++i){
        if (!hasFeatures){
          rr::indexEntry entry;
          ASSERT(entry.ParseFromString(data[i]));
          hasFeatures= entry.id_size()>0 || entry.diffid_size()>0;
          if (!hasFeatures)
            continue;
        }
        dbBuilder.addData(ID, data[i]);
      }
      data.clear();
      if (!hasFeatures){
        if (numNoFeatsWarnings<5)
          std::cout<<"buildIndex::mergePartialFidx: warning no features detected in imageID="<<ID<<"\n";
        ++numNoFeatsWarnings;
      }

      // push next
      for (++ID; ID<db->numIDs() && !(db->contains(ID)); ++ID);
      if (ID < db->numIDs()){
        IDs[iFile]= ID;
        queue.push(iFile);
      }
    }

    dbBuilder.close();

    util::delPointerVector(inDbs);

    if (numNoFeatsWarnings>5) {
      std::cout<<"buildIndex::mergePartialFidx: warning no features detected "<<numNoFeatsWarnings<<" images\n";
    }

    std::cout<<"buildIndex::mergePartialFidx: done\n";

    for (uint32_t i= 0; i<fns.size(); ++i) {
      boost::filesystem::remove(fns[i]);
    }
  }



  // ------------------------------------
  // ------------------------------------ Build Index
  // ------------------------------------

#ifdef RR_MPI
#define IF_MPI_BARRIER if (!useThreads) comm.barrier();
#else
#define IF_MPI_BARRIER
#endif



  void
  build(
        std::string const imagelistFn,
        std::string const databasePath,
        std::string const dsetFn,
        std::string const iidxFn,
        std::string const fidxFn,
        std::string const tmpDir,
        featGetter const &featGetter_obj,
        std::string const clstFn,
        embedderFactory const *embFactory) {

    MPI_GLOBAL_ALL
      bool useThreads= detectUseThreads();
    uint32_t numWorkerThreads= omp_get_max_threads();

    ASSERT(tmpDir[tmpDir.length()-1]=='/');
    std::string indexingStatusFn= tmpDir+"indexingstatus.bin";

    // load status
    rr::buildIndexStatus status;
    IF_MPI_BARRIER

      if (rank==0 && !loadStatus(indexingStatusFn, status)){
        status.set_state( rr::buildIndexStatus::beginning );
        saveStatus(indexingStatusFn, status);
      }

    std::chrono::steady_clock::time_point build_start = std::chrono::steady_clock::now();

    // ------------------------------------ SemiSorted

    IF_MPI_BARRIER
      ASSERT(loadStatus(indexingStatusFn, status));
    if (status.state()==rr::buildIndexStatus::beginning){

      // extract features, assign to clusters, save to files sorted by clusterID within each indexEntry
      // also save the bare fidx (i.e. list of unique wordIDs)
      // also construct the dataset info (i.e. list of images, width/height)

      if (rank==0)
        std::cout<<"buildIndex::build: beginning\n";

      // clusters
      if (rank==0)
        std::cout<<"buildIndex::build: Loading cluster centres\n";
      double t0= timing::tic();
      clstCentres clstCentres_obj( clstFn.c_str(), true );
      if (rank==0)
        std::cout<<"buildIndex::build: Loading cluster centres - DONE ("<< timing::toc(t0) <<" ms)\n";

      if (rank==0)
        std::cout<<"buildIndex::build: Constructing NN search object\n";
      t0= timing::tic();

      // build kd-tree for nearest neighbour search
      // to assign cluster-id for each descriptor
      std::size_t num_trees = 8;
      std::size_t max_num_checks = 1024;
      VlKDForest* kd_forest = vl_kdforest_new( VL_TYPE_FLOAT, clstCentres_obj.numDims, num_trees, VlDistanceL2 );
      vl_kdforest_set_max_num_comparisons(kd_forest, max_num_checks);
      vl_kdforest_build(kd_forest, clstCentres_obj.numClst, clstCentres_obj.clstC_flat);

      if (rank==0)
        std::cout<<"buildIndex::build: Constructing NN search object - DONE ("<< timing::toc(t0) << " ms)\n";

      // get number of documents
      uint32_t numDocs= 0;
      if (rank==0){
        std::ifstream fImagelist(imagelistFn.c_str());
        std::string imageFn;
        bool emptyline= false;
        while (std::getline(fImagelist, imageFn)){
          if (imageFn.length()>1) {
            ++numDocs;
            ASSERT(!emptyline); // i.e. empty lines can only appear at end of file
          } else
            emptyline= true;
        }
        fImagelist.close();
      }

      // communicate numDocs to everyone
#ifdef RR_MPI
      if (!useThreads)
        boost::mpi::broadcast(comm, numDocs, 0);
#endif

      buildManagerSemiSorted *manager= (rank==0) ?
        new buildManagerSemiSorted(numDocs, dsetFn) :
        NULL;

      std::vector<std::string> fns;
      uint32_t totalFeats= 0;

      if (useThreads){

        std::vector<queueWorker<buildResultSemiSorted> const *> workers;
        for (uint32_t i= 0; i<numWorkerThreads; ++i)
          workers.push_back( new buildWorkerSemiSorted(
                                                       tmpDir, imagelistFn, databasePath,
                                                       featGetter_obj,
                                                       kd_forest,
                                                       &clstCentres_obj,
                                                       embFactory) );

        // start feature extraction + assignment
        threadQueue<buildResultSemiSorted>::start( numDocs, workers, *manager );

        // collect file names
        for (uint32_t i= 0; i<workers.size(); ++i){
          ((buildWorkerSemiSorted *)workers[i])->finish();
          fns.insert( fns.end(),
                      ((buildWorkerSemiSorted const *)workers[i])->fns_.begin(),
                      ((buildWorkerSemiSorted const *)workers[i])->fns_.end() );
          status.add_fidx_filename( ((buildWorkerSemiSorted const *)workers[i])->fidx_fn_ );
          totalFeats+= ((buildWorkerSemiSorted const *)workers[i])->totalFeats_;
        }

        util::delPointerVector(workers);

      } else {

#ifdef RR_MPI

        // start feature extraction + assignment
        buildWorkerSemiSorted worker(
                                     tmpDir, imagelistFn, databasePath,
                                     featGetter_obj,
                                     kd_forest,
                                     &clstCentres_obj,
                                     embFactory);
        mpiQueue<buildResultSemiSorted>::start( numDocs, worker, manager );
        worker.finish();

        // collect file names
        if (rank==0){
          for (uint32_t iProc= 0; iProc < numProc; ++iProc){
            std::vector<std::string> fnsI;
            if (iProc>0)
              comm.recv( iProc, 0, fnsI );
            else
              fnsI= worker.fns_;
            fns.insert( fns.end(), fnsI.begin(), fnsI.end() );
            std::string fidxFn;
            if (iProc>0)
              comm.recv( iProc, 1, fidxFn );
            else
              fidxFn= worker.fidx_fn_;
            status.add_fidx_filename(fidxFn);
            uint64_t workerTotalFeats;
            if (iProc>0)
              comm.recv( iProc, 2, workerTotalFeats );
            else
              workerTotalFeats= worker.totalFeats_;
            totalFeats+= workerTotalFeats;
          }
        } else {
          comm.send( 0, 0, worker.fns_ );
          comm.send( 0, 1, worker.fidx_fn_ );
          comm.send( 0, 2, worker.totalFeats_ );
        }

#else
        ASSERT(false); // need to have MPI for this
#endif

      }

      if (rank==0) delete manager;
      vl_kdforest_delete(kd_forest);

      // update status
      if (rank==0){
        status.set_state( rr::buildIndexStatus::semiSorted );
        status.clear_filename();
        for (uint32_t i= 0; i<fns.size(); ++i)
          status.add_filename( fns[i] );
        status.set_totalfeats(totalFeats);
        saveStatus(indexingStatusFn, status);
      }
    }



    // ------------------------------------ Sorted

    IF_MPI_BARRIER
      ASSERT(loadStatus(indexingStatusFn, status));
    if (status.state()==rr::buildIndexStatus::semiSorted){

      // sort the previously generated files (sorted within each indexEntry by clusterID) such that sorting is maintained accorss indexEntries as well (i.e. last element of first indexEntry is < than first element of second indexEntry)

      if (rank==0){
        std::cout<<"buildIndex::build: semiSorted\n";
        std::cout<< "\n" << status.DebugString() <<"\n";
      }

      std::vector<std::string> fns;
      fns.reserve( status.filename_size() );
      for (int i= 0; i < status.filename_size(); ++i)
        fns.push_back(status.filename(i));

      uint32_t nJobs= fns.size();

      buildManagerFiles *manager= (rank==0) ?
        new buildManagerFiles(nJobs, "buildManagerSorted") :
        NULL;
      buildWorkerSorted worker(tmpDir, fns, mergingMemoryLim / std::max(numProc, numWorkerThreads), embFactory );

      if (useThreads)
        threadQueue<std::string>::start( nJobs, worker, *manager, numWorkerThreads );
      else
        mpiQueue<std::string>::start( nJobs, worker, manager );

      // delete old files
      if (rank==0){
        for (uint32_t i= 0; i<fns.size(); ++i)
          boost::filesystem::remove(fns[i]);
      }

      // update status
      if (rank==0){
        status.set_state( rr::buildIndexStatus::merged );
        status.clear_filename();
        fns= manager->fns_;
        for (uint32_t i= 0; i<fns.size(); ++i)
          status.add_filename( fns[i] );

        saveStatus(indexingStatusFn, status);
      }

      if (rank==0) delete manager;
    }



    // ------------------------------------ Merged



    IF_MPI_BARRIER
      ASSERT(loadStatus(indexingStatusFn, status));
    if (status.state()==rr::buildIndexStatus::merged){

      if (rank==0){
        std::cout<<"buildIndex::build: merged\n";
        std::cout<< "\n" << status.DebugString() <<"\n";
      }

      std::vector<std::string> fns;
      fns.reserve( status.filename_size() );
      for (int i= 0; i < status.filename_size(); ++i)
        fns.push_back(status.filename(i));

      std::vector<std::string> fidxFns;
      fidxFns.reserve( status.fidx_filename_size() );
      for (int i= 0; i < status.fidx_filename_size(); ++i)
        fidxFns.push_back(status.fidx_filename(i));

      if (useThreads){

        // merge fidx
        boost::thread thread1( boost::bind(mergePartialFidx, fidxFns, fidxFn) );

        // merge iidx
        boost::thread thread2( boost::bind(mergeSortedFiles, fns, iidxFn, status.totalfeats(), embFactory) );

        thread1.join();
        thread2.join();

      } else {

#ifdef RR_MPI

        if (rank==0){
          // merge fidx
          mergePartialFidx(fidxFns, fidxFn);
        }

        if ((numProc==1 && rank==0) || rank==1){
          // merge iidx
          mergeSortedFiles(fns, iidxFn, status.totalfeats(), embFactory);
        }

        comm.barrier();

#else
        ASSERT(false); // need to have MPI for this
#endif

      }

      // update status
      if (rank==0){
        status.set_state( rr::buildIndexStatus::done );
        status.clear_filename();
        status.clear_fidx_filename();

        saveStatus(indexingStatusFn, status);
      }

    }

    if (rank==0){
      ASSERT(loadStatus(indexingStatusFn, status));
      ASSERT(status.state()==rr::buildIndexStatus::done);

      std::chrono::steady_clock::time_point build_end = std::chrono::steady_clock::now();
      std::cout<<"buildIndex::build: done in "<< std::chrono::duration_cast<std::chrono::minutes>(build_end - build_start).count() <<" minutes\n";
      std::cout<< "\n" << status.DebugString() <<"\n";
    }

  }

};
