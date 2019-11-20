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

#ifndef _MQ_JOINT_AVG_H_
#define _MQ_JOINT_AVG_H_

#include "macros.h"
#include "multi_query.h"
#include "retriever_v2.h"
#include "spatial_verif_v2.h"



// Never really going to use this one so didn't parallelize
// Easy to do, but waste of time
// Included this code only because I needed it for a paper
// Also, the spatial reranking is quite hacky
// (an extra query is performed to filter hamming)
// Assumes BoW or HE
class multiQueryJointAvg : public multiQuery {
    
    public:
        
        multiQueryJointAvg( retrieverV2 const &retriever_obj, spatialVerifV2 const *spat_obj= NULL) :
            retriever_(&retriever_obj),
            spat_(spat_obj),
            numDocs_(retriever_obj.numDocs()) {}
        
        void
            queryExecute( std::vector<query> const &queries, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        uint32_t
            numDocs() const { return numDocs_; }
    
    private:
        
        retrieverV2 const *retriever_;
        spatialVerifV2 const *spat_;
        uint32_t const numDocs_;
};


#endif
