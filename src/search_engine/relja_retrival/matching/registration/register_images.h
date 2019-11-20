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

#ifndef _REGISTER_IMAGES_H_
#define _REGISTER_IMAGES_H_

#include "dataset_abs.h"
#include "query.h"
#include "homography.h"
#include "spatial_retriever.h"
#include "same_random.h"


class registerImages {
    
    public:
        
        static void
            registerFromGuess( sameRandomUint32 const &sameRandomObj,
                               const char image_fn1[], const char image_fn2[],
                               double xl, double xu, double yl, double yu,
                               homography &Hinit,
                               const char outFn1[], const char outFn2[], const char outFn2t[],
                               const char *fullSizeFn1= NULL, const char *fullSizeFn2= NULL );
        
        static void
            registerFromQuery( query const &query_obj,
                               const char inFn1[], uint32_t docID2,
                               datasetAbs const &datasetObj,
                               spatialRetriever const &spatialRetriever_obj,
                               const char outFn1[], const char outFn2[], const char outFn2t[],
                               const char *fullSizeFn1= NULL, const char *fullSizeFn2= NULL );
        
    private:
        
        static void
            findBBox2( double xl, double xu, double yl, double yu, homography const &H, double &xl2, double &xu2, double &yl2, double &yu2, uint32_t w2, uint32_t h2 );
        
};


#endif
