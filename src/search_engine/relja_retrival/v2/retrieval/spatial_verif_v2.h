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

#ifndef _SPATIAL_VERIF_V2_H
#define _SPATIAL_VERIF_V2_H

#include <map>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "daat.h"
#include "det_ransac.h"
#include "ellipse.h"
#include "homography.h"
#include "index_entry_util.h"
#include "macros.h"
#include "par_queue.h"
#include "retriever_v2.h"
#include "spatial_defs.h"
#include "spatial_retriever.h"

// for kd-tree based nearest neighbour search
#include <vl/generic.h>
#include <vl/kdtree.h>

class spatialVerifV2 : public retrieverV2, public spatialRetriever {

public:

  typedef std::pair< std::pair< uint32_t, std::pair<double,uint32_t> >, homography> Result; // <docID, <score,#inliers>, H>

  spatialVerifV2( retrieverFromIter const &firstRetriever,
                  protoIndex const *iidx,
                  protoIndex const *fidx= NULL,
                  bool verifyFromIidx= true,
                  featGetter const *featGetterObj= NULL,
                  VlKDForest* kd_forest = NULL,
                  clstCentres const *clstCentresObj= NULL,
                  spatParams spatParamsObj= spatParams_def );

  inline void
  externalQuery_computeData( std::string imageFn, query const &queryObj ) const {
    firstRetriever_->externalQuery_computeData( imageFn, queryObj );
  }

  void
  queryExecute( rr::indexEntry &queryRep, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;

  void
  spatialQueryExecute( rr::indexEntry &queryRep,
                       std::vector<indScorePair> &queryRes,
                       std::map<uint32_t, homography> *Hs= NULL,
                       std::set<uint32_t> *ignoreDocs= NULL,
                       uint32_t toReturn= 0,
                       bool queryFirst= true,
                       bool forgetFirst= false) const;

  inline uint32_t
  numDocs() const {
    return firstRetriever_->numDocs();
  }

  // spatialRetriever

  inline void
  spatialQuery( query const &queryObj,
                std::vector<indScorePair> &queryRes,
                std::map<uint32_t, homography> &Hs,
                uint32_t toReturn= 0 ) const {
    rr::indexEntry queryRep;
    getQueryRep(queryObj, queryRep);
    spatialQueryExecute( queryRep, queryRes, &Hs, NULL, toReturn, true );
  }

  void
  getMatchesCore(query const &queryObj,
                 uint32_t docID2,
                 std::vector<ellipse> &ellipses1,
                 std::vector<ellipse> &ellipses2,
                 matchesType &putativeMatches) const;

  inline void
  getMatches( query const &queryObj,
              uint32_t docID2,
              homography &H,
              std::vector< std::pair<ellipse,ellipse> > &matches ) const;

  void
  getPutativeMatches( query const &queryObj,
                      uint32_t docID2,
                      std::vector< std::pair<ellipse,ellipse> > &matches ) const;


private:

  retrieverFromIter const *firstRetriever_;
  protoIndex const *iidx_;
  bool verifyFromIidx_;
  spatParams const spatParams_;
  ellipseUnquantizer const elUnquant_;

  static uint32_t const maxPutativePerDBFeature_= 1; // 0 if no maximum

  // create ellipses of the query
  void
  createEllipses(rr::indexEntry &queryRep, std::vector<ellipse> &ellipses) const;

  static void
  getPutativeMatches(uniqEntries const &ue,
                     std::vector<int> const &uniqIndToInd,
                     std::vector<uint32_t> const &nonEmptyEntryInd,
                     std::vector< std::pair<uint32_t,uint32_t> > const &entryInd,
                     ellipseUnquantizer const &elUnquant,
                     std::vector<ellipse> &ellipses2,
                     matchesType &putativeMatches);

  static void
  convertMatchesToEllipses(std::vector<ellipse> const &ellipses1,
                           std::vector<ellipse> const &ellipses2,
                           matchesType const &matchesInds,
                           std::vector< std::pair<ellipse,ellipse> > &matches );

  class spatManager : public queueManager<Result> {
  public:
    spatManager(std::vector<indScorePair> &queryRes,
                spatParams const &spatParamsObj,
                uint32_t spatialDepthEff,
                std::map<uint32_t, homography> *Hs= NULL);
    void operator() (uint32_t resInd, Result &result);
  private:
    std::vector<indScorePair> *queryRes_;
    spatParams const *spatParams_;
    uint32_t spatialDepthEff_;
    std::map<uint32_t, homography> *Hs_;
    DISALLOW_COPY_AND_ASSIGN(spatManager)
  };

  class spatWorker : public queueWorker<Result> {
  public:
    spatWorker(std::vector<ellipse> const &ellipses1,
               uniqEntries const &ue,
               daat &daatIter,
               boost::mutex &daatLock,
               std::vector<int> const &uniqIndToInd,
               spatParams const &spatParamsObj,
               ellipseUnquantizer const &elUnquant,
               sameRandomUint32 const &sameRandomObj);
    void operator() (uint32_t resInd, Result &result) const;
  private:
    std::vector<ellipse> const *ellipses1_;
    uniqEntries const *ue_;
    daat *daatIter_;
    boost::mutex *daatLock_;
    std::vector<int> const *uniqIndToInd_;
    spatParams const *spatParams_;
    ellipseUnquantizer const *elUnquant_;
    sameRandomUint32 const *sameRandomObj_;

    // to avoid reallocating RAM
    mutable std::vector<ellipse> ellipses2_;
    mutable matchesType putativeMatches_;
    mutable std::vector< std::pair<uint32_t,uint32_t> > entryIndC_;

    DISALLOW_COPY_AND_ASSIGN(spatWorker)
  };

private:
  DISALLOW_COPY_AND_ASSIGN(spatialVerifV2)
};

#endif
