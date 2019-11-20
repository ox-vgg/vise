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

#ifndef _MQ_FILTER_OUTLIERS_H_
#define _MQ_FILTER_OUTLIERS_H_

#include "hamming_embedder.h"
#include "macros.h"
#include "multi_query.h"
#include "retriever_v2.h"



class mqFilterOutliers : public multiQuery {
    
    public:
        
        mqFilterOutliers(multiQuery const &mq,
                         retrieverV2 const &baseRetriever,
                         hammingEmbedderFactory const &embFactory);
        
        ~mqFilterOutliers() {}
        
        inline void
            externalQuery_computeData( std::vector<std::string> const &imageFns, std::vector<query> const &queries ) const {
                mq_->externalQuery_computeData( imageFns, queries );
            }
        
        void
            queryExecute( std::vector<query> const &queries, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        uint32_t
            numDocs() const { return mq_->numDocs(); }
    
    private:
        
        multiQuery const *mq_;
        retrieverV2 const *baseRet_;
        hammingEmbedderFactory const *embFactory_;
        
        uint32_t const scoreThr_;
        float const failureProp_;
        // const after constructor finishes
        int distThr_;
        
        DISALLOW_COPY_AND_ASSIGN(mqFilterOutliers)
    
};


#endif
