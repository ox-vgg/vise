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

#ifndef _SPATiAL_DEFS_H_
#define _SPATiAL_DEFS_H_

#include <stdint.h>
#include "retriever.h"


struct spatParams {
    
    uint32_t spatialDepth, minInliers, maxReest;
    float errorThr, lowAreaChange, highAreaChange;
    
    spatParams( uint32_t aSpatialDepth= 200, uint32_t aMinInliers= 4,
                float aErrorThr= 40.0,
                float aLowAreaChange= 0, float aHighAreaChange= 31.63 /* =sqrt(1000) */,
                uint32_t aMaxReest= 4
                ) : 
                spatialDepth(aSpatialDepth), minInliers(aMinInliers), maxReest(aMaxReest), errorThr(aErrorThr), lowAreaChange(aLowAreaChange), highAreaChange(aHighAreaChange) {}
};

static const spatParams spatParams_def;

typedef std::pair<indScorePair,homography> spatResType;

#endif
