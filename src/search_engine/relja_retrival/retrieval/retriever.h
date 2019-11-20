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

#ifndef _RETRIEVER_H_
#define _RETRIEVER_H_

#include <stdint.h>
#include <vector>
#include <string>

#include "macros.h"
#include "query.h"


typedef std::pair<uint32_t,double> indScorePair;


class retriever {
    
    public:
        
        retriever(){}
        
        virtual
            ~retriever(){}
        
        void
            externalQuery( std::string imageFn, query const &query_obj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        virtual void
            externalQuery_computeData( std::string imageFn, query const &queryObj ) const
                { throw std::runtime_error("Not implemented"); }
        
        virtual void
            queryExecute( query const &queryObj, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const =0;
        
        virtual uint32_t
            numDocs() const =0;
        
        void
            internalQuery( uint32_t docID, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const {
                query query_obj(docID, true);
                queryExecute( query_obj, queryRes, toReturn );
            }
        
        static void
            sortResults( std::vector<indScorePair> &queryRes, uint32_t firstN= 0, uint32_t toReturn= 0 );
        
        static void
            sortResults( std::vector<double> &scores, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 );
        
    private:
        
        static bool compare( indScorePair const &x, indScorePair const &y ){ return x.second > y.second; }
        
        DISALLOW_COPY_AND_ASSIGN(retriever)
    
};

#endif
