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

#ifndef _TFIDF_V2_H_
#define _TFIDF_V2_H_

#include <string>

#include "feat_getter.h"
#include "macros.h"
#include "soft_assigner.h"
#include "retriever_v2.h"

// for kd-tree based nearest neighbour search
#include <vl/generic.h>
#include <vl/kdtree.h>

class tfidfV2 : public retrieverFromIter {

public:

  tfidfV2( protoIndex const *iidx,
           protoIndex const *fidx= NULL,
           std::string tfidfFn= "",
           featGetter const *featGetter_obj= NULL,
           VlKDForest* kd_forest = NULL,
           softAssigner const *SA_obj= NULL );

  void
  externalQuery_computeData( std::string imageFn, query const &queryObj ) const;

  void
  queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;

  void
  queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<double> &scores ) const;

  inline uint32_t
  numDocs() const {
    return numDocs_;
  }

  static void
  computeIdf(protoIndex const &iidx, std::vector<double> &idf, protoIndex const *fidx= NULL);

  static void
  load(std::string tfidfFn, std::vector<double> &idf, std::vector<double> &docL2);

  static void
  save(std::string tfidfFn, std::vector<double> const &idf, std::vector<double> const &docL2);

  // set/add weights according to count/weight/empty and multiply by:
  // if weight==NULL: idf( id(i) ),
  // if weight!=NULL: *weight
  static void
  weightStatic(rr::indexEntry &entry, double *weight= NULL, std::vector<double> const *idf= NULL);

  inline std::vector<double> const &
  getIdf() const { return idf_; }

  inline std::vector<double> const &
  getDocL2() const { return docL2_; }

private:

  inline void
  computeIdf(){
    ASSERT(iidx_!=NULL);
    computeIdf(*iidx_, idf_, fidx_);
  }

  void
  computeDocL2();

  inline void
  weight(rr::indexEntry &entry, double *weight= NULL) const {
    weightStatic(entry, weight, &idf_);
  }

  protoIndex const *iidx_;
  std::vector<double> idf_, docL2_;
  uint32_t numDocs_;

  featGetter const *featGetter_obj_;
  VlKDForest* kd_forest_;
  softAssigner const *SA_obj_;
  uint32_t const numDims_;

private:
  DISALLOW_COPY_AND_ASSIGN(tfidfV2)
};

#endif
