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

#include "retriever_v2.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "argsort.h"
#include "image_util.h"
#include "index_entry_util.h"

void
retrieverV2::getQueryRep( query const &queryObj, rr::indexEntry &queryRep ) const {

  bool queryWholeImage= queryObj.allInf();
  bool needXYForThis= needXY_ || !queryWholeImage;
  bool alreadyFiltered= false;

  if (queryObj.isInternal){
    ASSERT(fidx_!=NULL);

    std::vector<rr::indexEntry> entries;
    fidx_->getEntries(queryObj.docID, entries);

    if (!entries.empty()){

      ASSERT(entries.size()==1);
      rr::indexEntry const &entry= entries[0];

      if (
          ( (needXYForThis && (entry.x_size()>0 || entry.qx_size()>0)) ||
            (!needXYForThis && (entry.count_size()>0 || entry.weight_size()>0)) )
          &&
          ( embFactory_==NULL || (entry.has_data() && entry.data().size()>0) )
          )
        // if fidx contains all needed info, we're done
        queryRep= entry;
      else {
        // if fidx doesn't contain needed info, iidx does
        ASSERT(iidx_!=NULL);

        std::vector<rr::indexEntry> invEntries;
        std::vector< std::pair<uint32_t,uint32_t> > inds;

        uint32_t total= 0;

        for (int i= 0; i < entry.id_size(); ++i){
          uint32_t wordID= entry.id(i);
          ASSERT(i==0 || wordID>entry.id(i-1));

          inds.clear();
          invEntries.clear();
          iidx_->getEntries(wordID, invEntries);
          uint32_t N= iidx_->getInverseEntryInds(queryObj.docID, inds, invEntries);
          total+= N;

          if (N>0){

            if (!queryWholeImage){
              // filter ROI
              alreadyFiltered= true;
              if (indexEntryUtil::markInside(invEntries, queryObj, inds)==0)
                continue;
            }

            indexEntryUtil::copyRanges(invEntries, inds, queryRep, &wordID, needXYForThis, needEllipse_, embFactory_);
          }
        }
      }
    }

  } else {

    // load already created file
    try {
      std::ifstream in(queryObj.compDataFn.c_str(), std::ios::binary);
      if (!queryRep.ParseFromIstream(&in)){
        std::cout<<"failed to parse protobuf in "<<queryObj.compDataFn<<"\n";
        queryRep.Clear();
      }
      in.close();
    } catch (std::exception &e) {
      std::cout<<"file probably doesn't exist: "<<queryObj.compDataFn<<"\n";
    }

  }

  if (!queryWholeImage && !alreadyFiltered){
    // need to filter based on ROI

    rr::indexEntry queryRep_nofilt;
    queryRep_nofilt= queryRep;
    queryRep.Clear();

    std::vector< std::pair<uint32_t,uint32_t> > inds;
    if (indexEntryUtil::markInside(queryRep_nofilt, queryObj, inds)>0){
      for (uint32_t i= 0; i<inds.size(); ++i){
        indexEntryUtil::copyRange(queryRep_nofilt, inds[i].first, inds[i].second, queryRep, NULL, needXYForThis, needEllipse_, embFactory_);
      }
    }
  }

}



void
retrieverV2::externalQuery_computeData( std::string imageFn, query const &queryObj ) const {

  ASSERT( featGetter_!=NULL );

  // make sure the image exists and is readable
  std::pair<uint32_t, uint32_t> wh= std::make_pair(0,0);
  if (boost::filesystem::exists(imageFn) && boost::filesystem::is_regular_file(imageFn)){
    wh= imageUtil::getWidthHeight(imageFn);
    if (wh.first==0 && wh.second==0) {
      std::cerr<<"retrieverV2::externalQuery_computeData: "<<imageFn<<" is corrupt or 0x0\n";
      return;
    }
  } else {
    std::cerr<<"retrieverV2::externalQuery_computeData: "<<imageFn<<" doesn't exist\n";
    return;
  }

  // compute features

  uint32_t numFeats;
  std::vector<ellipse> regions;
  float *descs;

  std::cout<<"retrieverV2::externalQuery_computeData: Extracting features\n";
  featGetter_->getFeats(imageFn.c_str(), numFeats, regions, descs);
  std::cout<<"retrieverV2::externalQuery_computeData: Extracting features - DONE\n";
  if (numFeats==0){
    delete []descs;
    return;
  }

  // get visual words

  static const uint KNN= 3;

  // prepare memory
  rr::indexEntry queryRep0;
  google::protobuf::RepeatedField<uint32_t> *wordIDs0= queryRep0.mutable_id();
  google::protobuf::RepeatedField<float> *x0= queryRep0.mutable_x();
  google::protobuf::RepeatedField<float> *y0= queryRep0.mutable_y();
  google::protobuf::RepeatedField<float> *a0= queryRep0.mutable_a();
  google::protobuf::RepeatedField<float> *b0= queryRep0.mutable_b();
  google::protobuf::RepeatedField<float> *c0= queryRep0.mutable_c();
  wordIDs0->Reserve( numFeats * KNN );
  x0->Reserve( numFeats * KNN );
  y0->Reserve( numFeats * KNN );
  a0->Reserve( numFeats * KNN );
  b0->Reserve( numFeats * KNN );
  c0->Reserve( numFeats * KNN );
  embedder *emb0= embFactory_->getEmbedder();
  emb0->reserve( numFeats * KNN );

  VlKDForestSearcher* kd_forest_searcher = vl_kdforest_new_searcher(kd_forest_);
  std::vector<VlKDForestNeighbor> cluster(KNN);
  uint32_t const numDims= featGetter_->numDims();

  float *residual= new float[numDims];

  std::cout<<"retrieverV2::externalQuery_computeData: assigning to clusters\n";

  for (uint32_t iFeat=0; iFeat<numFeats; ++iFeat){

    // assign to clusters
    vl_kdforestsearcher_query(kd_forest_searcher, cluster.data(), KNN, (descs + iFeat*numDims));

    ellipse const &region= regions[iFeat];

    for (uint32_t i= 0; i<KNN; ++i){
      wordIDs0->AddAlreadyReserved( cluster[i].index );
      x0->AddAlreadyReserved( region.x );
      y0->AddAlreadyReserved( region.y );
      a0->AddAlreadyReserved( region.a );
      b0->AddAlreadyReserved( region.b );
      c0->AddAlreadyReserved( region.c );

      if (emb0->doesSomething()){
        // due to KNN need to make a copy
        float const *itD= descs+iFeat*numDims;
        float const *itC= clstCentres_->clstC_flat + cluster[i].index * numDims;
        float const *endC= itC + numDims;
        float *itResidual= residual;
        for (; itC!=endC; ++itC, ++itD, ++itResidual)
          *itResidual= *itD - *itC;
        emb0->add(residual, cluster[i].index);
      }
    }

  }
  std::cout<<"retrieverV2::externalQuery_computeData: assigning to clusters - DONE\n";

  // cleanup
  cluster.clear();
  delete []descs;
  delete []residual;

  // sort IDs
  std::vector<uint32_t> inds;
  argSortArray<uint32_t>::sort( queryRep0.id().data(), numFeats * KNN, inds );

  // apply the sort
  rr::indexEntry queryRep= queryRep0; // to get all repeated field sizes correctly
  uint32_t *wordIDs= queryRep.mutable_id()->mutable_data();
  float *x= queryRep.mutable_x()->mutable_data();
  float *y= queryRep.mutable_y()->mutable_data();
  float *a= queryRep.mutable_a()->mutable_data();
  float *b= queryRep.mutable_b()->mutable_data();
  float *c= queryRep.mutable_c()->mutable_data();
  uint32_t size= wordIDs0->size();
  embedder *emb= embFactory_->getEmbedder();
  emb->reserve(numFeats * KNN);

  for (uint32_t i= 0; i<size; ++i, ++wordIDs, ++x, ++y, ++a, ++b, ++c){
    uint32_t ind= inds[i];
    *wordIDs= wordIDs0->Get(ind);
    *x= x0->Get(ind);
    *y= y0->Get(ind);
    *a= a0->Get(ind);
    *b= b0->Get(ind);
    *c= c0->Get(ind);
    emb->copyFrom(*emb0, ind);
  }
  delete emb0;

  // dump extra data into indexEntry_
  if (emb->getByteSize()>0)
    queryRep.set_data( emb->getEncoding() );
  delete emb;

  // save to file
  std::ofstream of(queryObj.compDataFn.c_str(), std::ios::binary);
  queryRep.SerializeToOstream(&of);
  of.close();

}
