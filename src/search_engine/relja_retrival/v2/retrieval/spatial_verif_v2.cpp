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

#include "spatial_verif_v2.h"

#include <algorithm>
#include <string>

#include "uniq_entries.h"
#include "util.h"



spatialVerifV2::spatialVerifV2(
                               retrieverFromIter const &firstRetriever,
                               protoIndex const *iidx,
                               protoIndex const *fidx,
                               bool verifyFromIidx,
                               featGetter const *featGetterObj,
                               VlKDForest* kd_forest,
                               clstCentres const *clstCentresObj,
                               spatParams spatParamsObj )
  : retrieverV2(fidx, iidx, true, true, firstRetriever.embFactory_, featGetterObj, kd_forest, clstCentresObj),
    firstRetriever_(&firstRetriever),
    iidx_(iidx),
    verifyFromIidx_(verifyFromIidx),
    spatParams_(spatParamsObj) {

  ASSERT( (verifyFromIidx_ && iidx_!=NULL) || (!verifyFromIidx_ && fidx!=NULL) );

}



void
spatialVerifV2::queryExecute( rr::indexEntry &queryRep, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
  spatialQueryExecute(queryRep, queryRes, NULL, NULL, toReturn, true);
}



void
spatialVerifV2::spatialQueryExecute(
                                    rr::indexEntry &queryRep,
                                    std::vector<indScorePair> &queryRes,
                                    std::map<uint32_t, homography> *Hs,
                                    std::set<uint32_t> *ignoreDocs,
                                    uint32_t toReturn,
                                    bool queryFirst,
                                    bool forgetFirst) const {

  assert( !forgetFirst || queryFirst );
  ASSERT(queryRep.id_size()==queryRep.x_size() || queryRep.id_size()==queryRep.qx_size());
  ASSERT(queryRep.id_size()==queryRep.y_size() || queryRep.id_size()==queryRep.qy_size());
  ASSERT( ignoreDocs==NULL ); // TODO

  uint32_t spatialDepthEff= spatParams_.spatialDepth;

  if (ignoreDocs!=NULL)
    spatialDepthEff+= ignoreDocs->size();

  ASSERT(verifyFromIidx_);
  uniqEntries ue;
  iidx_->getUniqEntries(queryRep, ue);
  precompUEIterator ueIter(ue);

  if (queryFirst){
    uint32_t toReturnFirst= toReturn;
    if (toReturn!=0 && toReturnFirst < spatialDepthEff )
      toReturnFirst= spatialDepthEff;
    std::vector<indScorePair> queryResDummy;
    firstRetriever_->queryExecute( queryRep, &ueIter, forgetFirst ? queryResDummy : queryRes, toReturnFirst );
    // queryExecute could change queryRep, so check it hasn't changed id_size
    ASSERT(ueIter.getNum()==static_cast<uint32_t>(queryRep.id_size()));
  }

  if (spatialDepthEff>queryRes.size())
    spatialDepthEff= queryRes.size();

  // create ellipses of the query
  std::vector<ellipse> ellipses1;
  createEllipses(queryRep, ellipses1);

  // which docIDs to verify?
  std::vector<uint32_t> docIDtoVerify(spatialDepthEff);
  for (uint32_t i= 0; i<spatialDepthEff; ++i)
    docIDtoVerify[i]= queryRes[i].first;
  std::sort(docIDtoVerify.begin(), docIDtoVerify.end());

  // prepare for DAAT output (returns ind into unique queryRep.id's)
  std::vector<int> uniqIndToInd;
  ue.getUniqIndToInd(uniqIndToInd);

  // create DAAT iterator
  ueIter.reset();
  daat daatIter(&ueIter, &docIDtoVerify);

  // prepare parallel

  boost::mutex daatLock;

  // synchronous DAAT call is relatively expensive so the gain of using
  // multiple threads is not large (maybe re-evalaute this?)
  uint32_t const numWorkerThreads= std::min(
                                            static_cast<uint32_t>(detectUseThreads() ? 10 : 1),
                                            spatialDepthEff);

  std::vector<queueWorker<Result> const *> workers;
  for (uint32_t iThread= 0; iThread < numWorkerThreads; ++iThread)
    workers.push_back( new spatWorker(ellipses1, ue, daatIter, daatLock, uniqIndToInd, spatParams_, elUnquant_, sameRandomObj_) );

  spatManager manager( queryRes, spatParams_, spatialDepthEff, Hs );

  // start the threads

  threadQueue<Result>::start(
                             spatialDepthEff, workers, manager
                             );

  // cleanup
  util::delPointerVector(workers);

  retriever::sortResults( queryRes, spatialDepthEff, toReturn );

}



void
spatialVerifV2::getMatchesCore(
                               query const &queryObj,
                               uint32_t docID2,
                               std::vector<ellipse> &ellipses1,
                               std::vector<ellipse> &ellipses2,
                               matchesType &putativeMatches) const{

  ellipses1.clear();
  ellipses2.clear();
  putativeMatches.clear();

  rr::indexEntry queryRep;
  getQueryRep(queryObj, queryRep);

  // following spatialQueryExecute and spatWorker::operator()

  ASSERT(queryRep.id_size()==queryRep.x_size() || queryRep.id_size()==queryRep.qx_size());
  ASSERT(queryRep.id_size()==queryRep.y_size() || queryRep.id_size()==queryRep.qy_size());

  ASSERT(verifyFromIidx_);
  uniqEntries ue;
  iidx_->getUniqEntries(queryRep, ue);
  precompUEIterator ueIter(ue);

  // create ellipses of the query
  createEllipses(queryRep, ellipses1);

  // prepare for DAAT output (returns ind into unique queryRep.id's)
  std::vector<int> uniqIndToInd;
  ue.getUniqIndToInd(uniqIndToInd);

#if 1
  // want to do it this way (i.e. like issuing an entire query) in order for the displayed matches to definitely be the same as the ones used for ranking. The reason there could be differences if the other option ('#else') is executed is that DAAT iteration can return word indexes in arbitrary order, so putative matches can be shuffled and thus ransac could get a different solution

  // which docIDs to verify + querying might change/set matching weights (e.g. hamming..)
  std::vector<uint32_t> docIDtoVerify;
  {
    uint32_t spatialDepthEff= spatParams_.spatialDepth;
    std::vector<indScorePair> queryRes;
    firstRetriever_->queryExecute( queryRep, &ueIter, queryRes, spatialDepthEff );
    ASSERT(ueIter.getNum()==static_cast<uint32_t>(queryRep.id_size()));

    docIDtoVerify.resize(spatialDepthEff);
    bool docID2InResults= false;
    for (uint32_t i= 0; i<spatialDepthEff; ++i){
      docID2InResults= docID2InResults || (queryRes[i].first == docID2);
      docIDtoVerify[i]= queryRes[i].first;
    }
    if (docID2InResults)
      std::sort(docIDtoVerify.begin(), docIDtoVerify.end());
    else {
      // Only possible if we're looking for a non-verified document, i.e. putative matches
      // Include this block to force DAAT to visit the image
      // Note that this can happen when manually tweeking the query ROI in the details web page.
      docIDtoVerify.clear();
      docIDtoVerify.push_back(docID2);
    }
  }

  // create DAAT iterator
  ueIter.reset();
  daat daatIter(&ueIter, &docIDtoVerify);

#else

  if (firstRetriever_->changesEntryWeights()){
    std::vector<indScorePair> queryRes;
    firstRetriever_->queryExecute( queryRep, &ueIter, queryRes, 1 );
    // queryExecute could change queryRep, so check it hasn't changed id_size
    ASSERT(ueIter.getNum()==static_cast<uint32_t>(queryRep.id_size()));
  }

  // create DAAT iterator
  ueIter.reset();
  daat daatIter(&ueIter, NULL, &docID2);

#endif

  // iterate DAAT once to get putative matches
  bool foundEntry= false;
  std::vector< std::pair<uint32_t,uint32_t> > const *entryInd= NULL;
  std::vector<uint32_t> const *nonEmptyEntryInd= NULL;

  while (!daatIter.isEnd()){
    daatIter.advance();
    if (!daatIter.getMatches(entryInd, nonEmptyEntryInd))
      continue;

    if (daatIter.getDocID()==docID2){
      foundEntry= true;
      break;
    }
  }

  if (!foundEntry)
    return;

  // form putative matches
  getPutativeMatches(ue, uniqIndToInd,
                     *nonEmptyEntryInd, *entryInd,
                     elUnquant_,
                     ellipses2, putativeMatches);

}



void
spatialVerifV2::convertMatchesToEllipses(
                                         std::vector<ellipse> const &ellipses1,
                                         std::vector<ellipse> const &ellipses2,
                                         matchesType const &matchesInds,
                                         std::vector< std::pair<ellipse,ellipse> > &matches ){

  // get ellipse matches
  matches.clear();
  matches.reserve( matchesInds.size() );

  uint32_t inInd1, inInd2;

  for (uint32_t iIn=0; iIn<matchesInds.size(); ++iIn){

    inInd1= matchesInds[iIn].first;
    inInd2= matchesInds[iIn].second;

    matches.push_back( std::make_pair(
                                      ellipses1[inInd1],
                                      ellipses2[inInd2] ) );

  }
}



void
spatialVerifV2::getPutativeMatches(
                                   query const &queryObj,
                                   uint32_t docID2,
                                   std::vector< std::pair<ellipse,ellipse> > &matches ) const {

  std::vector<ellipse> ellipses1, ellipses2;
  matchesType putativeMatches;
  getMatchesCore(queryObj, docID2, ellipses1, ellipses2, putativeMatches );

  convertMatchesToEllipses(ellipses1, ellipses2, putativeMatches, matches);
}



void
spatialVerifV2::getMatches(
                           query const &queryObj,
                           uint32_t docID2,
                           homography &H,
                           std::vector< std::pair<ellipse,ellipse> > &matches ) const {

  std::vector<ellipse> ellipses1, ellipses2;
  matchesType putativeMatches;
  getMatchesCore(queryObj, docID2, ellipses1, ellipses2, putativeMatches );

  std::vector< std::pair<uint32_t,uint32_t> > inlierInds;
  H.setIdentity();
  uint32_t numInliers= 0;

  // do matching
  numInliers= 0;
  detRansac::match(
                   sameRandomObj_,
                   numInliers,
                   ellipses1, ellipses2,
                   putativeMatches,
                   NULL,
                   spatParams_.errorThr,
                   spatParams_.lowAreaChange, spatParams_.highAreaChange,
                   spatParams_.maxReest,
                   &H, &inlierInds
                   );

  convertMatchesToEllipses(ellipses1, ellipses2, inlierInds, matches);
}



void
spatialVerifV2::createEllipses(rr::indexEntry &queryRep, std::vector<ellipse> &ellipses) const {

  elUnquant_.unquantize(queryRep);
  ellipses.reserve( queryRep.id_size() );
  {
    bool quantXY= queryRep.qx_size()>0;
    ASSERT( quantXY != (queryRep.qy_size()==0) );

    for (int i= 0; i<queryRep.id_size(); ++i)
      ellipses.push_back( ellipse(
                                  quantXY ? queryRep.qx(i) : queryRep.x(i),
                                  quantXY ? queryRep.qy(i) : queryRep.y(i),
                                  queryRep.a(i), queryRep.b(i), queryRep.c(i) ) );
  }

}




void
spatialVerifV2::getPutativeMatches(
                                   uniqEntries const &ue,
                                   std::vector<int> const &uniqIndToInd,
                                   std::vector<uint32_t> const &nonEmptyEntryInd,
                                   std::vector< std::pair<uint32_t,uint32_t> > const &entryInd,
                                   ellipseUnquantizer const &elUnquant,
                                   std::vector<ellipse> &ellipses2,
                                   matchesType &putativeMatches) {

  // careful about Ind meaning (it is actually indexing directly into uniqEntries.allEntries_
  // semiTODO matching weight (didn't really improve things for hamming, so should distinguish somehow between it and bow (weight doesn't exist in db entries?). However seems unnecessary as hamming works well without it

  // for matching strenght - to limit number of putative matches per feature based on their strenght
  std::vector<float> thisWeights;
  float weightThr= 0.0;
  if (maxPutativePerDBFeature_!=0)
    thisWeights.reserve(1000);

  ellipses2.clear();
  putativeMatches.clear();
  // reserve memory with a rough guesstimate, 15% speedup for spatial query
  ellipses2.reserve( nonEmptyEntryInd.size() *5 );
  putativeMatches.reserve( nonEmptyEntryInd.size() *5 );

  for (uint32_t iInd= 0; iInd<nonEmptyEntryInd.size(); ++iInd){

    uint32_t uniqInd= nonEmptyEntryInd[iInd];
    ASSERT( uniqInd < ue.allEntries_.size() );
    std::vector<rr::indexEntry> const &entries= ue.allEntries_[uniqInd];
    ASSERT(entries.size()==1); // TODO with indexEntryVector
    rr::indexEntry const &entry= entries[0];
    bool quantXY= entry.qx_size()>0;

    std::string const &scaleStr= entry.qel_scale();
    std::string const &ratioStr= entry.qel_ratio();
    std::string const &angleStr= entry.qel_angle();
    unsigned char const *scale= reinterpret_cast<unsigned char const*>(scaleStr.c_str());
    unsigned char const *ratio= reinterpret_cast<unsigned char const*>(ratioStr.c_str());
    unsigned char const *angle= reinterpret_cast<unsigned char const*>(angleStr.c_str());
    float a, b, c;

    std::pair<uint32_t,uint32_t> const &matchInds= entryInd[uniqInd];

    // add ellipses for result and putative matches

    ellipses2.reserve( ellipses2.size() + matchInds.second - matchInds.first );
    putativeMatches.reserve( putativeMatches.size() +
                             (matchInds.second - matchInds.first) * ( uniqIndToInd[uniqInd+1] - uniqIndToInd[uniqInd] ) );

    // get match weights
    float const *weights= entry.weight_size()>0 ? entry.weight().data() : NULL;
    ASSERT( entry.weight_size()==0 ||
            static_cast<uint32_t>(entry.weight_size()) ==
            static_cast<uint32_t>(entry.id_size()) * ( uniqIndToInd[uniqInd+1] - uniqIndToInd[uniqInd] ) );

    for (uint32_t matchInd= matchInds.first; matchInd < matchInds.second; ++matchInd){
      elUnquant.unquantize( scale[matchInd], ratio[matchInd], angle[matchInd], a, b, c);
      ellipses2.push_back( ellipse(
                                   quantXY ? entry.qx(matchInd) : entry.x(matchInd),
                                   quantXY ? entry.qy(matchInd) : entry.y(matchInd),
                                   a, b, c ) );

      // for every repeated queryRep.id add this putative match
      if (weights==NULL){
        for (int ind= uniqIndToInd[uniqInd]; ind < uniqIndToInd[uniqInd+1]; ++ind)
          putativeMatches.push_back( std::make_pair(
                                                    static_cast<uint32_t>(ind), ellipses2.size()-1 ) );
      } else {

        if (maxPutativePerDBFeature_!=0 &&
            static_cast<uint32_t>(uniqIndToInd[uniqInd+1]-uniqIndToInd[uniqInd]) > maxPutativePerDBFeature_){
          // there is a maximum and there is a potential to breach it
          thisWeights.clear();
          thisWeights.reserve(uniqIndToInd[uniqInd+1]-uniqIndToInd[uniqInd]);

          // get all matching weights
          for (int ind= uniqIndToInd[uniqInd]; ind < uniqIndToInd[uniqInd+1]; ++ind)
            if (weights[ (ind - uniqIndToInd[uniqInd]) * entry.id_size() + matchInd ] > 0)
              thisWeights.push_back( weights[ (ind - uniqIndToInd[uniqInd]) * entry.id_size() + matchInd ] );

          if (thisWeights.size()>maxPutativePerDBFeature_){
            // find the threshold
            std::nth_element(thisWeights.begin(),
                             thisWeights.begin()+maxPutativePerDBFeature_,
                             thisWeights.end(),
                             std::greater<float>());
            weightThr= thisWeights[maxPutativePerDBFeature_-1]-1e-9;
          } else
            weightThr= 0.0;

        } else
          weightThr= 0.0;

        for (int ind= uniqIndToInd[uniqInd]; ind < uniqIndToInd[uniqInd+1]; ++ind)
          if (weights[ (ind - uniqIndToInd[uniqInd]) * entry.id_size() + matchInd ] >= weightThr)
            putativeMatches.push_back( std::make_pair(
                                                      static_cast<uint32_t>(ind), ellipses2.size()-1 ) );
      }
    }
  }

}



spatialVerifV2::spatManager::spatManager(
                                         std::vector<indScorePair> &queryRes,
                                         spatParams const &spatParamsObj,
                                         uint32_t spatialDepthEff,
                                         std::map<uint32_t, homography> *Hs)
  : queryRes_(&queryRes), spatParams_(&spatParamsObj), spatialDepthEff_(spatialDepthEff), Hs_(Hs) {
}



void
spatialVerifV2::spatManager::operator() (uint32_t resInd, Result &result){

  if (result.first.second.second >= spatParams_->minInliers){
    // success
    uint32_t docID= result.first.first;
    uint32_t i;
    for (i= 0; i<spatialDepthEff_ && queryRes_->at(i).first!=docID; ++i);
    ASSERT(i<spatialDepthEff_);
    queryRes_->at(i).second+= result.first.second.first;
    if (Hs_!=NULL)
      (*Hs_)[docID]= result.second;
  }

}



spatialVerifV2::spatWorker::spatWorker(
                                       std::vector<ellipse> const &ellipses1,
                                       uniqEntries const &ue,
                                       daat &daatIter,
                                       boost::mutex &daatLock,
                                       std::vector<int> const &uniqIndToInd,
                                       spatParams const &spatParamsObj,
                                       ellipseUnquantizer const &elUnquant,
                                       sameRandomUint32 const &sameRandomObj) :
  ellipses1_(&ellipses1), ue_(&ue), daatIter_(&daatIter), daatLock_(&daatLock), uniqIndToInd_(&uniqIndToInd), spatParams_(&spatParamsObj), elUnquant_(&elUnquant), sameRandomObj_(&sameRandomObj){
}



void
spatialVerifV2::spatWorker::operator() (uint32_t resInd, Result &result) const {

  // iterate DAAT to get putative matches
  bool foundEntry= false;
  std::vector< std::pair<uint32_t,uint32_t> > const *entryInd= NULL;
  std::vector<uint32_t> const *nonEmptyEntryInd= NULL;

  boost::mutex::scoped_lock daatLock(*daatLock_);

  while (!daatIter_->isEnd()){

    daatIter_->advance();
    if (!daatIter_->getMatches(entryInd, nonEmptyEntryInd))
      continue;

    foundEntry= true;
    break;
  }

  if (!foundEntry){
    result.first.second.first= 0;
    return;
  }

  std::vector<uint32_t> const nonEmptyEntryIndC= *nonEmptyEntryInd;
  entryIndC_.resize( entryInd->size() );
  // copy only nonEmpty entryInd
  for (uint32_t iInd= 0; iInd<nonEmptyEntryInd->size(); ++iInd){
    uint32_t uniqInd= nonEmptyEntryInd->at(iInd);
    entryIndC_[uniqInd]= entryInd->at(uniqInd);
  }

  uint32_t docID= daatIter_->getDocID();

  daatLock.unlock();

  // form putative matches
  getPutativeMatches(*ue_, *uniqIndToInd_,
                     nonEmptyEntryIndC, entryIndC_,
                     *elUnquant_,
                     ellipses2_, putativeMatches_);

  // do matching
  uint32_t numInliers= 0;
  double score=
    detRansac::match(
                     *sameRandomObj_,
                     numInliers,
                     *ellipses1_, ellipses2_,
                     putativeMatches_,
                     NULL,
                     spatParams_->errorThr,
                     spatParams_->lowAreaChange, spatParams_->highAreaChange,
                     spatParams_->maxReest,
                     &result.second, NULL
                     );

  result.first.first= docID;
  result.first.second.first= score;
  result.first.second.second= numInliers;
}
