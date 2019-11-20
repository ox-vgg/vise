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

#ifndef _HAMMING_H_
#define _HAMMING_H_

#include <string>

#include "hamming_embedder.h"
#include "macros.h"
#include "retriever_v2.h"
#include "tfidf_v2.h"

// for kd-tree based nearest neighbour search
#include <vl/generic.h>
#include <vl/kdtree.h>

#define HAMM_DO_WEIGHTED 1

class hamming : public retrieverFromIter {

public:

  hamming( tfidfV2 const &tfidf,
           protoIndex const *iidx,
           hammingEmbedderFactory const &embFactory,
           protoIndex const *fidx= NULL,
           featGetter const *featGetterObj= NULL,
           VlKDForest* kd_forest = NULL,
           clstCentres const *clstCentresObj= NULL);

  ~hamming();

  void
  queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;

  void
  queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<double> &scores ) const;

  inline bool
  changesEntryWeights() const { return true; }

  inline uint32_t
  numDocs() const {
    return numDocs_;
  }

private:

  std::vector<double> const &idf_, &docL2_;
  protoIndex const *iidx_;
  hammingEmbedderFactory const *embFactory_;
  uint32_t const numDocs_;

  // const after constructor finishes
  int distThr_, distThrSpatial_;
#if HAMM_DO_WEIGHTED
  float *wDist_;
#endif

private:
  DISALLOW_COPY_AND_ASSIGN(hamming)
};

#endif
