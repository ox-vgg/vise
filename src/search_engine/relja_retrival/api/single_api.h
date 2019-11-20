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

#ifndef _SINGLE_API_H_
#define _SINGLE_API_H_

#include <string>
#include <stdint.h>

#ifdef MPI
#include <boost/mpi.hpp>
#endif
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>


#include "abs_api.h"
#include "retriever.h"
#include "multi_query.h"



class singleAPI : public absAPI {
    
    public:
        
        singleAPI( datasetAbs const &datasetObj, retriever const &aRetriever_obj, multiQuery const *aMq= NULL ) :
            absAPI(datasetObj),
            retriever_obj(&aRetriever_obj), mq(aMq)
                {}
        
        std::string
            getReply( boost::property_tree::ptree &pt, std::string const &request ) const;
        
    private:
            
        void
            queryExecute( query &query_obj, uint32_t startFrom, uint32_t numberToReturn, std::string &output ) const;
        
        void
            multipleQueries( std::vector<query> const &queries, uint32_t startFrom, uint32_t numberToReturn, std::string &output ) const;
        
        static void
            returnResults( std::vector<indScorePair> const &queryRes, uint32_t startFrom, uint32_t numberToReturn, std::string &output );
        
        
        retriever const *retriever_obj;
        multiQuery const *mq;
    
};

#endif
