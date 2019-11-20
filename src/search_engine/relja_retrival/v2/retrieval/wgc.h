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

#ifndef _WGC_H_
#define _WGC_H_



#include <string>

#include "macros.h"
#include "retriever_v2.h"


class wgc : public retrieverFromIter {
    
    public:
        
        wgc( protoIndex const &iidx, protoIndex const *fidx= NULL, std::string wgcFn= "" );
        
        void
            queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        void
            queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<double> &scores ) const;
        
        inline uint32_t
            numDocs() const {
                return numDocs_;
            }
    
    private:
        
        // same as tfidfV2
        void
            computeIdf();
        
        // different from tfidfV2
        void
            computeDocL2();
        
        protoIndex const *iidx_;
        std::vector<double> idf_, docL2_;
        uint32_t numDocs_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(wgc)
};

#endif
