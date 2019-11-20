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

#include "hamming.h"

#include "argsort.h"
#include "bitcount.h"

// for kd-tree based nearest neighbour search
#include <vl/generic.h>
#include <vl/kdtree.h>

hamming::hamming(
                 tfidfV2 const &tfidf,
                 protoIndex const *iidx,
                 hammingEmbedderFactory const &embFactory,
                 protoIndex const *fidx,
                 featGetter const *featGetterObj,
                 VlKDForest* kd_forest,
                 clstCentres const *clstCentresObj)
  : retrieverFromIter(iidx, fidx, false, false, &embFactory, featGetterObj, kd_forest, clstCentresObj),
    idf_(tfidf.getIdf()),
    docL2_(tfidf.getDocL2()),
    iidx_(iidx),
    embFactory_(&embFactory),
    numDocs_(tfidf.numDocs()) {
  uint32_t const numBits= embFactory.numBits();

  // threshold for 64 is 24
  distThr_= static_cast<int>( round( static_cast<float>(numBits) * 24.0 / 64.0 ) );
  // found that for a slightly looser threshold is better for spatial reranking, for 64 it's 32
  distThrSpatial_= static_cast<int>( round( static_cast<float>(numBits) * 28.0 / 64.0 ) );
  ASSERT( distThrSpatial_ >= distThr_ ); // assumed in scoring

#if HAMM_DO_WEIGHTED
  // sigma=16 for 64
  float const sigma= static_cast<float>(numBits) / 4;
  wDist_= new float[distThrSpatial_+1];
  for (int hammDist= 0; hammDist<=distThrSpatial_; ++hammDist)
    wDist_[hammDist]= exp( - static_cast<float>(hammDist*hammDist)/(sigma*sigma) );
#endif
}



hamming::~hamming(){
#if HAMM_DO_WEIGHTED
  delete []wDist_;
#endif
}



void
hamming::queryExecute(
                      rr::indexEntry &queryRep,
                      ueIterator *ueIter,
                      std::vector<indScorePair> &queryRes,
                      uint32_t toReturn ) const {
  std::vector<double> scores;
  queryExecute(queryRep, ueIter, scores);
  retriever::sortResults( scores, queryRes, toReturn );
}



void
hamming::queryExecute(
                      rr::indexEntry &queryRep,
                      ueIterator *ueIter,
                      std::vector<double> &scores ) const {

  scores.clear();
  scores.resize( numDocs_, 0.0 );
  if (ueIter->isEnd())
    return;

  hammingEmbedder *heQ= embFactory_->getEmbedder();
  hammingEmbedder *heDb= embFactory_->getEmbedder();

  heQ->setDataCopy(queryRep.data());
  charStream* csQ= heQ->getCharStream();
  ASSERT(csQ->getNum() == static_cast<uint32_t>(queryRep.id_size()));
  ASSERT(ueIter->getNum() == static_cast<uint32_t>(queryRep.id_size()));

  // query

  double queryL2= 0.0;
  uint32_t wordID, prevDocID, thisNum;
  double thisIncScore, thisOneScore;
  int hammDist;

  // just ensure that weight and count don't exist (for first entry as don't want to check everything..)
  std::vector<rr::indexEntry> *entries= ueIter->getEntries();
  if (entries->size()>0){
    rr::indexEntry const &entry= entries->at(0);
    ASSERT(entry.weight_size()==0 && entry.count_size()==0);
  }

  for (int iQueryWord= 0; iQueryWord < queryRep.id_size();){

    wordID= queryRep.id(iQueryWord);

    // get the boundaries of the current query visual word
    int queryWordEnd= iQueryWord;
    for (; queryWordEnd < queryRep.id_size() && queryRep.id(queryWordEnd)==wordID;
         ++queryWordEnd);
    // this is not in the burstiness paper by Jegou, but it must be right!? works a bit better on Oxford 5k/105k..
    float const numQueryWordSqrt= sqrt(queryWordEnd-iQueryWord);

    double const w= idf_[wordID] * idf_[wordID];

    // for every query descriptor (within this wordID)

    for (; iQueryWord < queryWordEnd; ++iQueryWord) {
      ASSERT( static_cast<uint32_t>(iQueryWord) == ueIter->getInd() );

      uint64_t const querySig= csQ->getNextUnsafe();
      queryL2+= w;

      std::vector<rr::indexEntry> *entries= ueIter->getEntries();

      if (entries->size()==0){
        // advance ueIter enough
        for (; iQueryWord < queryWordEnd; ++iQueryWord, ueIter->increment() );
        break;
      }
      ueIter->increment();
      prevDocID= entries->at(0).id(0);
      thisIncScore= 0.0;
      thisNum= 0;

      for (uint32_t iEntry= 0; iEntry<entries->size(); ++iEntry){
        rr::indexEntry &entry= entries->at(iEntry);
        google::protobuf::RepeatedField<float> *entryweight= entry.mutable_weight();
        entryweight->Reserve( entryweight->size() + entry.id_size() );

        uint32_t const *itID= entry.id().data();
        uint32_t const *endID= itID + entry.id_size();

        heDb->setDataCopy(entry.data());
        charStream* csDb= heDb->getCharStream();
        ASSERT(csDb->getNum() == static_cast<uint32_t>(entry.id_size()));

        while (itID!=endID){
          for (; itID!=endID && *itID==prevDocID; ++itID, ++thisNum){
            hammDist= bitcount64(querySig ^ csDb->getNextUnsafe());
            if (hammDist <= distThrSpatial_){
              thisOneScore=
#if HAMM_DO_WEIGHTED
                w*wDist_[hammDist];
#else
              w;
#endif
              if (hammDist <= distThr_)
                thisIncScore+= thisOneScore;
              entryweight->AddAlreadyReserved( thisOneScore );
            } else
              entryweight->AddAlreadyReserved( -1.0 );
          }

          // if docID changed add the accumulated score for this image
          // it is a bit long-winded but it's needed for burstiness
          // TODO: precompute sqrt as all are integers
          if (itID!=endID){
            scores[prevDocID]+= thisIncScore / sqrt(thisNum) / numQueryWordSqrt;
            prevDocID= *itID;
            thisIncScore= 0.0;
            thisNum= 0;
          }
        }

      }
      // add the final score
      scores[prevDocID]+= thisIncScore / sqrt(thisNum) / numQueryWordSqrt;
    }
  }

  if (queryL2 <= 1e-7)
    queryL2= 1.0;

  std::vector<double>::const_iterator docL2Iter= docL2_.begin();
  for (std::vector<double>::iterator itS= scores.begin(); itS!=scores.end(); ++itS, ++docL2Iter)
    (*itS)= (*itS) / ( queryL2 * (*docL2Iter) );

  delete heQ;
  delete heDb;

}
