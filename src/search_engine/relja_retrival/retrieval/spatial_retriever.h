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

#ifndef _SPATIAL_RETRIEVER_H_
#define _SPATIAL_RETRIEVER_H_

#include <map>
#include <stdint.h>
#include <vector>

#include "homography.h"
#include "macros.h"
#include "query.h"
#include "retriever.h"
#include "same_random.h"



class spatialRetriever {
    
    public:
        
        spatialRetriever() : sameRandomObj_(10000) {}
        
        virtual void
            spatialQuery( query const &queryObj,
                          std::vector<indScorePair> &queryRes,
                          std::map<uint32_t, homography> &Hs,
                          uint32_t toReturn= 0 ) const =0;
        
        virtual void
            getMatches( query const &queryObj,
                        uint32_t docID2,
                        homography &H,
                        std::vector< std::pair<ellipse,ellipse> > &matches ) const =0;
        
        virtual void
            getPutativeMatches( query const &queryObj,
                                uint32_t docID2,
                                std::vector< std::pair<ellipse,ellipse> > &matches ) const
                { throw std::runtime_error("Not implemented"); }
        
        virtual void
            externalQuery_computeData( std::string imageFn, query const &queryObj ) const
                { throw std::runtime_error("Not implemented"); }
        
        // TODO: this should really be abstracted away, but here for simplicity with register_images (otherwise make a registerImages or even better ransac class which would store sameRandomObj_)
        inline sameRandomUint32 const *
            getSameRandom() const
                { return &sameRandomObj_; }
    
    protected:
        sameRandomUint32 const sameRandomObj_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(spatialRetriever)
    
};



// for debug
class fakeSpatialRetriever : public spatialRetriever {
    
    public:
        
        fakeSpatialRetriever(retriever const &trueRetriever) : trueRetriever_(&trueRetriever) {}
        
        virtual void
            spatialQuery( query const &queryObj,
                          std::vector<indScorePair> &queryRes,
                          std::map<uint32_t, homography> &Hs,
                          uint32_t toReturn= 0 ) const {
                Hs.clear();
                trueRetriever_->queryExecute(queryObj, queryRes, toReturn);
            }
        
        virtual void
            getMatches( query const &queryObj,
                        uint32_t docID2,
                        homography &H,
                        std::vector< std::pair<ellipse,ellipse> > &matches ) const
                { throw std::runtime_error("Not implemented"); }
        
        virtual void
            externalQuery_computeData( std::string imageFn, query const &queryObj ) const {
                trueRetriever_->externalQuery_computeData(imageFn, queryObj);
            }
    
    private:
        retriever const *trueRetriever_;
        DISALLOW_COPY_AND_ASSIGN(fakeSpatialRetriever)
};

#endif
