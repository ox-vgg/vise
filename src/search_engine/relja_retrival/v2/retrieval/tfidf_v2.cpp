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

#include "tfidf_v2.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "argsort.h"
#include "tfidf_data.pb.h"
#include "timing.h"
#include "weighter_v2.h"


tfidfV2::tfidfV2(
                 protoIndex const *iidx,
                 protoIndex const *fidx,
                 std::string tfidfFn,
                 featGetter const *featGetter_obj,
                 VlKDForest *kd_forest,
                 softAssigner const *SA_obj )
  : retrieverFromIter(iidx, fidx, false, false),
    iidx_(iidx),
    featGetter_obj_(featGetter_obj),
    kd_forest_(kd_forest),
    SA_obj_(SA_obj),
    numDims_(featGetter_obj==NULL ? 0 : featGetter_obj->numDims()) {

  if ( tfidfFn.length()>0 && boost::filesystem::exists( tfidfFn ) ){

    load(tfidfFn, idf_, docL2_);
    numDocs_= docL2_.size();

  } else {

    computeIdf();
    computeDocL2();
    numDocs_= docL2_.size();

    if (tfidfFn.length()>0)
      save(tfidfFn, idf_, docL2_);

  }

}



void
tfidfV2::load(std::string tfidfFn, std::vector<double> &idf, std::vector<double> &docL2){

  rr::tfidfData data;

  std::ifstream in(tfidfFn.c_str(), std::ios::binary);
  ASSERT(data.ParseFromIstream(&in));
  in.close();

  idf.clear();
  idf.reserve(data.idf_size());
  for (int i= 0; i<data.idf_size(); ++i)
    idf.push_back( data.idf(i) );

  docL2.clear();
  docL2.reserve(data.docl2_size());
  for (int i= 0; i<data.docl2_size(); ++i)
    docL2.push_back( data.docl2(i) );

}



void
tfidfV2::save(std::string tfidfFn, std::vector<double> const &idf, std::vector<double> const &docL2){

  rr::tfidfData data;
  data.mutable_idf()->Reserve(idf.size());
  for (uint32_t i= 0; i<idf.size(); ++i)
    data.add_idf(idf[i]);
  data.mutable_docl2()->Reserve(docL2.size());
  for (uint32_t i= 0; i<docL2.size(); ++i)
    data.add_docl2(docL2[i]);

  std::ofstream of(tfidfFn.c_str(), std::ios::binary);
  data.SerializeToOstream(&of);
  of.close();

}



void
tfidfV2::queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
  std::vector<double> scores;
  queryExecute(queryRep, ueIter, scores);
  retriever::sortResults( scores, queryRes, toReturn );
}



void
tfidfV2::queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<double> &scores ) const {

  // weight query BoW with idf
  weight(queryRep);

  // query
  weighterV2::queryExecute(queryRep, ueIter, idf_, docL2_, scores);

}



void
tfidfV2::weightStatic(rr::indexEntry &entry, double *weight, std::vector<double> const *idf) {

  ASSERT(weight!=NULL || idf!=NULL);
  bool hasWeigth= (entry.weight_size()!=0);
  bool hasCount= (entry.count_size()!=0);

  if (!hasWeigth)
    entry.mutable_weight()->Reserve( entry.id_size() );
  else
    ASSERT( entry.id_size() == entry.weight_size() );

  if (hasCount)
    ASSERT( entry.id_size() == entry.count_size() );

  double w= (weight==NULL ? 0.0 : *weight);

  for (int i= 0; i < entry.id_size(); ++i) {
    if (weight==NULL)
      w= idf->at( entry.id(i) );

    if (hasWeigth)
      entry.set_weight( i, w * entry.weight(i) );
    else
      entry.add_weight( hasCount ? w * entry.count(i) : w );
  }

}



void
tfidfV2::computeIdf(protoIndex const &iidx, std::vector<double> &idf, protoIndex const *fidx){

  uint32_t numWords= iidx.numIDs();
  ASSERT(fidx!=NULL); // if needed, this could be replaced by computing numDocs as max(all ids)
  uint32_t numDocs= fidx->numIDs();

  idf.clear();
  idf.resize( numWords, 0.0 );

  uint32_t numWords_printStep= std::max(static_cast<uint32_t>(1),numWords/20);

  std::cout<<"tfidfV2::computeIdf\n";

  double time= timing::tic();

  // compute idf (pretend a word appears in 1 document if it appears in 0 to prevent division by 0)

  for (uint32_t wordID= 0; wordID < numWords; ++wordID){

    if (wordID % numWords_printStep == 0)
      std::cout<<"tfidfV2::computeIdf: wordID= "<<wordID<<" / "<<numWords<<" "<<timing::toc(time)<<" ms\n";

    idf[ wordID ]=
      log(
          static_cast<double>(numDocs) /
          std::max( static_cast<uint32_t>(1), iidx.getUniqNumWithID( wordID ) )
          );

  }

  std::cout<<"tfidfV2::computeIdf: DONE ("<<timing::toc(time)<<" ms)\n";

}



void
tfidfV2::computeDocL2() {

  uint32_t numWords= iidx_->numIDs();
  ASSERT(fidx_!=NULL); // if needed, this could be replaced by computing numDocs as max(all ids)
  uint32_t numDocs= fidx_->numIDs();

  docL2_.clear();
  docL2_.resize( numDocs, 0.0 );
  std::vector<rr::indexEntry> entries;

  uint32_t numWords_printStep= std::max(static_cast<uint32_t>(1),numWords/20);

  std::cout<<"tfidfV2::computeDocL2\n";
  double time= timing::tic();

  for (uint32_t wordID= 0; wordID < numWords; ++wordID){
    if (wordID % numWords_printStep == 0)
      std::cout<<"tfidfV2::computeDocL2: wordID= "<<wordID<<" / "<<numWords<<" "<<timing::toc(time)<<" ms\n";

    iidx_->getEntries( wordID, entries );

    // set/add weights and multiply by idf
    for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry)
      weight(entries[iEntry], &idf_[wordID]);

    indexEntryVector iev(entries);
    ievIterator it= iev.beginIter(), end= iev.endIter();
    uint32_t docID;
    double weight;

    while (it != end){
      docID= *it;
      weight= 0.0;
      for (; it != end && *it==docID; ++it){
        std::pair<uint32_t, int> p= iev.getInds(it.getInd());
        weight+= entries[p.first].weight(p.second);
      }
      docL2_[ docID ]+= weight * weight;
    }
  }

  for (uint32_t docID= 0; docID < numDocs; ++docID){
    if ( docL2_[docID] <= 1e-7 )
      docL2_[docID]= 1.0;
    else
      docL2_[docID]= sqrt( docL2_[docID] );
  }

  std::cout<<"tfidfV2::computeDocL2: DONE ("<<timing::toc(time)<<" ms)\n";

}



void
tfidfV2::externalQuery_computeData( std::string imageFn, query const &queryObj ) const {

  ASSERT( featGetter_obj_!=NULL );

  // compute features

  uint32_t numFeats;
  std::vector<ellipse> regions;
  float *descs;

  std::cout<<"tfidfV2::externalQuery_computeData: Extracting features\n";
  featGetter_obj_->getFeats(imageFn.c_str(), numFeats, regions, descs);
  std::cout<<"tfidfV2::externalQuery_computeData: Extracting features - DONE\n";

  // get visual words

  uint KNN= (SA_obj_==NULL) ? 1 : 3;

  rr::indexEntry queryRep0;
  google::protobuf::RepeatedField<uint32_t> *wordIDs0= queryRep0.mutable_id();
  google::protobuf::RepeatedField<float> *weights0= queryRep0.mutable_weight();
  google::protobuf::RepeatedField<float> *x0= queryRep0.mutable_x();
  google::protobuf::RepeatedField<float> *y0= queryRep0.mutable_y();
  google::protobuf::RepeatedField<float> *a0= queryRep0.mutable_a();
  google::protobuf::RepeatedField<float> *b0= queryRep0.mutable_b();
  google::protobuf::RepeatedField<float> *c0= queryRep0.mutable_c();
  wordIDs0->Reserve( numFeats * KNN );
  weights0->Reserve( numFeats * KNN );
  x0->Reserve( numFeats * KNN );
  y0->Reserve( numFeats * KNN );
  a0->Reserve( numFeats * KNN );
  b0->Reserve( numFeats * KNN );
  c0->Reserve( numFeats * KNN );

  std::cout<<"tfidfV2::externalQuery_computeData: assigning to clusters\n";

  VlKDForestSearcher* kd_forest_searcher = vl_kdforest_new_searcher(kd_forest_);
  for (uint32_t iFeat=0; iFeat<numFeats; ++iFeat){

    // assign to clusters
    std::vector<VlKDForestNeighbor> cluster(KNN);
    vl_kdforestsearcher_query(kd_forest_searcher, cluster.data(), KNN, (descs + iFeat*numDims_));

    quantDesc ww;

    for (uint iNN=0; iNN < KNN; ++iNN) {
      ww.rep.push_back( std::make_pair(cluster[iNN].index, cluster[iNN].distance) );
    }
    cluster.clear();

    ellipse const &region= regions[iFeat];

    // assign weights to visual words (soft or hard)

    if (SA_obj_!=NULL && KNN>1)
      SA_obj_->getWeights(ww);
    else
      ww.rep[0].second= 1.0f;

    for (uint32_t i= 0; i<ww.rep.size(); ++i){
      wordIDs0->AddAlreadyReserved( ww.rep[i].first );
      weights0->AddAlreadyReserved( ww.rep[i].second );
      x0->AddAlreadyReserved( region.x );
      y0->AddAlreadyReserved( region.y );
      a0->AddAlreadyReserved( region.a );
      b0->AddAlreadyReserved( region.b );
      c0->AddAlreadyReserved( region.c );
    }

  }
  std::cout<<"tfidfV2::externalQuery_computeData: assigning to clusters - DONE\n";

  // cleanup
  delete []descs;

  // sort IDs
  std::vector<uint32_t> inds;
  argSortArray<uint32_t>::sort( queryRep0.id().data(), numFeats * KNN, inds );

  // apply the sort
  rr::indexEntry queryRep= queryRep0; // to get all repeated field sizes correctly
  uint32_t *wordIDs= queryRep.mutable_id()->mutable_data();
  float *weights= queryRep.mutable_weight()->mutable_data();
  float *x= queryRep.mutable_x()->mutable_data();
  float *y= queryRep.mutable_y()->mutable_data();
  float *a= queryRep.mutable_a()->mutable_data();
  float *b= queryRep.mutable_b()->mutable_data();
  float *c= queryRep.mutable_c()->mutable_data();
  uint32_t size= wordIDs0->size();
  for (uint32_t i= 0; i<size; ++i, ++wordIDs, ++weights, ++x, ++y, ++a, ++b, ++c){
    uint32_t ind= inds[i];
    *wordIDs= wordIDs0->Get(ind);
    *weights= weights0->Get(ind);
    *x= x0->Get(ind);
    *y= y0->Get(ind);
    *a= a0->Get(ind);
    *b= b0->Get(ind);
    *c= c0->Get(ind);
  }

  // save to file
  std::ofstream of(queryObj.compDataFn.c_str(), std::ios::binary);
  queryRep.SerializeToOstream(&of);
  of.close();

}
