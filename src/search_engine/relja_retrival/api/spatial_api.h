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

#ifndef _SPATIAL_API_H_
#define _SPATIAL_API_H_

#include <string>
#include <stdint.h>

#include "abs_api.h"
#include "dataset_abs.h"
#include "query.h"
#include "retriever.h"
#include "spatial_retriever.h"
#include "macros.h"
#include "multi_query.h"



class API : public absAPI {
    
    public:
        
        API(spatialRetriever const &aSpatialRetriever_obj,
            multiQuery const  *aMultiQuery_obj,
            datasetAbs const &datasetObj ) :
            absAPI(datasetObj),
            spatialRetriever_obj(&aSpatialRetriever_obj),
            multiQuery_obj(aMultiQuery_obj),
            dataset_(&datasetObj)
                {}
        
        std::string
            getReply( boost::property_tree::ptree &pt, std::string const &request ) const;
        
    private:
            
        void
            queryExecute( query &query_obj, uint32_t startFrom, uint32_t numberToReturn, std::string &output ) const;
        
        void
            multipleQueries( std::vector<query> const &query_objs, uint32_t startFrom, uint32_t numberToReturn, std::string &output ) const;
        
        void
            processImage( std::string imageFn, std::string compDataFn, std::string &output ) const;
        
        static void
            returnResults( std::vector<indScorePair> const &queryRes, std::map<uint32_t,homography> const *Hs, uint32_t startFrom, uint32_t numberToReturn, std::string &output );
        
        void
            getMatches( query &query_obj, uint32_t docID2, std::string &output ) const;
        
        void
            getPutativeMatches( query &query_obj, uint32_t docID2, std::string &output ) const;
        
        static void
            returnMatches( std::vector< std::pair<ellipse,ellipse> > &matches, std::string &output );
        
        
        spatialRetriever const *spatialRetriever_obj;
        multiQuery const *multiQuery_obj;
        datasetAbs const *dataset_;
        
        DISALLOW_COPY_AND_ASSIGN(API)
    
};

#endif
