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

#ifndef _DET_RANSAC_H_
#define _DET_RANSAC_H_

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include "quant_desc.h"
#include "ellipse.h"
#include "homography.h"
#include "same_random.h"


typedef std::vector< std::pair<uint32_t, uint32_t> > matchesType;

class detRansac {
    
    private:
        class inlierFinder;
    
    public:
        
        static double
            matchWords_Hard(
                sameRandomUint32 const &sameRandomObj,
                
                uint32_t &nInliers,
                
                std::vector<quantDesc> const &ids1,
                std::vector<ellipse> const &ellipses1,
                std::vector<uint32_t> const *presortedInds1,
                std::vector<quantDesc> const &ids2,
                std::vector<ellipse> const &ellipses2,
                
                double errorThr,
                double lowAreaChange, double highAreaChange,
                uint32_t nReest= 4,
                
                homography *H= NULL,
                matchesType *inlierInds= NULL
                
                );
        
        static double
            matchWords_Soft(
                sameRandomUint32 const &sameRandomObj,
                
                uint32_t &nInliers,
                
                std::vector<quantDesc> const &ids1,
                std::vector<ellipse> const &ellipses1,
                std::vector<uint32_t> const *presortedInds1,
                std::vector<quantDesc> const &ids2,
                std::vector<ellipse> const &ellipses2,
                
                double errorThr,
                double lowAreaChange, double highAreaChange,
                uint32_t nReest= 4,
                
                homography *H= NULL,
                matchesType *inlierInds= NULL
                
                );
        
        static double
            matchDesc(
                sameRandomUint32 const &sameRandomObj,
                
                uint32_t &nInliers,
                
                float const* desc1,
                std::vector<ellipse> const &ellipses1,
                float const* desc2,
                std::vector<ellipse> const &ellipses2,
                uint32_t nDims,
                
                double errorThr,
                double lowAreaChange, double highAreaChange,
                uint32_t nReest= 4,
                
                bool useLowe= true,
                float deltaSq= 0.81f,
                float epsilon= 100.0f,
                
                homography *H= NULL,
                matchesType *inlierInds= NULL
                
                );
        
        static double
            match(
                sameRandomUint32 const &sameRandomObj,
                
                uint32_t &bestNInliers,
                
                std::vector<ellipse> const &ellipses1,
                std::vector<ellipse> const &ellipses2,
                matchesType const &putativeMatches,
                std::vector<double> const *PMweights,
                
                double errorThr,
                double lowAreaChange, double highAreaChange,
                uint32_t nReest= 4,
                
                homography *H= NULL,
                matchesType *inlierInds= NULL
                
                );
            
    private:
        
        inline static uint32_t
            getNStopping(double pFail, uint32_t nPutativeMatches, uint32_t bestNInliers){
                if (bestNInliers==nPutativeMatches) return 0;
                return std::min(
                    100000.0,
                    std::ceil(
                        std::log(pFail) /
                        std::log( 1.0 - static_cast<double>( std::max(static_cast<uint32_t>(4),bestNInliers) ) / nPutativeMatches )
                        )
                    );
            }
        
        static void
            getH( std::vector<ellipse> const &ellipses1,
                  std::vector<ellipse> const &ellipses2,
                  matchesType const &inliers, homography &H );
        
        static void
            normPoints( double *x, double *y, uint32_t n, homography &Hnorm );
        
        class sortH_helper{
            public:
                sortH_helper( std::vector<homography> *aHs, bool a ): Hs(aHs){
                    ind= (a?0:2);
                }
                std::vector<homography> *Hs;
                int ind;
                bool operator()(uint32_t i, uint32_t j){
                    return Hs->at(i).H[ind] < Hs->at(j).H[ind];
                }
        };
        
        
        
        class inlierFinder {
            
            public:
                
                inlierFinder(
                    std::vector<ellipse> const &aEllipses1,
                    std::vector<ellipse> const &aEllipses2,
                    matchesType const &aPutativeMatches,
                    std::vector<double> const &aPMweights,
                    double aErrorThr,
                    double aLowAreaChange, double aHighAreaChange);
                
                ~inlierFinder();
                
                double
                    getScore( homography const &H, uint32_t &nInliers, matchesType *inliers= NULL );
                
                inline double
                    getMaxScore(){ return static_cast<double>(nPutativeMatches); }
                
                inline uint32_t
                    getMaxInliers(){ return nPutativeMatches; }
                
            private:
                
                uint32_t nIter;
                
                matchesType const *putativeMatches;
                std::vector<double> const *PMweights;
                double errorThrSq, lowAreaChangeSq, highAreaChangeSq;
                
                double *x1, *y1, *x2, *y2, *areaDiffSq;
                
                std::vector<uint32_t> point1Used, point2Used;
                
                uint32_t nPutativeMatches;
        };
        
        
};

#endif
