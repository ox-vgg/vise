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

#ifndef _UNIQ_RETRIEVER_H_
#define _UNIQ_RETRIEVER_H_

#include "macros.h"
#include "retriever.h"



class uniqRetriever : public retriever {
    
    public:
        
        uniqRetriever( retriever const &firstRetriever,
                       std::vector<uint32_t> const &docIDtoObjID);
        
        inline void
            queryExecute( query const &queryObj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const {
                firstRetriever_->queryExecute(queryObj, queryRes, 0);
                filterSameObject(queryRes, toReturn);
            }
        
        inline void
            externalQuery_computeData( std::string imageFn, query const &queryObj ) const {
                firstRetriever_->externalQuery_computeData(imageFn, queryObj);
            }
        
        inline uint32_t
            numDocs() const {
                return firstRetriever_->numDocs();
            }
        
        void
            filterSameObject(std::vector<indScorePair> &queryRes, uint32_t toReturn= 0) const;
    
    protected:
        
        retriever const *firstRetriever_;
        std::vector<uint32_t> const *docIDtoObjID_;
        uint32_t maxObjID_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(uniqRetriever)
};

#endif
