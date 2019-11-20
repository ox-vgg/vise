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

#ifndef _PUTATIVE_H_
#define _PUTATIVE_H_

#include <vector>
//#include <utility.h>
#include <algorithm>

#include <stdint.h>
#include <limits>

#include "quant_desc.h"
#include "jp_dist2.hpp"
#include "macros.h"



class putative_quantized {
    
    public:
        
        static void
            getPutativeMatches_Hard(
                    std::vector<uint32_t> const &ids1, std::vector<uint32_t> const &ids2,
                    std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches,
                    std::vector<uint32_t> const *presortedInds1= NULL );
        
        static void
            getPutativeMatches_Soft(
                    std::vector<wordWeightPair> const &ids1, std::vector<uint32_t> const &qDInds1,
                    std::vector<wordWeightPair> const &ids2, std::vector<uint32_t> const &qDInds2,
                    std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches,
                    std::vector<double> &weights,
                    std::vector<uint32_t> const *presortedInds1= NULL );
        
};



template<class DescType>
class putative_desc {
    
    public:
        
        static void
            getPutativeMatches(
                    DescType const* desc1, uint32_t size1,
                    DescType const* desc2, uint32_t size2,
                    uint32_t nDims,
                    std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches,
                    bool useLowe= true,
                    float deltaSq= 0.81f,
                    float epsilon= 100.0f );
};



class indSorter_Hard {
    
    public:
        
        static void
            sort( std::vector<uint32_t> const &aToSort, std::vector<uint32_t> &inds,
                  std::vector<uint32_t> const *presortedInds1= NULL );
        
    private:
        
        static bool
            isSorted( std::vector<uint32_t> const &ids, std::vector<uint32_t> const &inds );
            
};



class indSorter_Soft {
        
    private:
        
        std::vector<wordWeightPair> const *toSort;
    
    public:
        
        indSorter_Soft( std::vector<wordWeightPair> const &aToSort ) : toSort(&aToSort) {}
        
        static void
            sort( std::vector<wordWeightPair> const &aToSort, std::vector<uint32_t> &inds,
                  std::vector<uint32_t> const *presortedInds1= NULL );
        
        inline int
            operator()( uint32_t leftInd, uint32_t rightInd ){
                // careful here because has to be strictly < and not <=, otherwise sometimes causes segmentation fault!
                return (*toSort)[ leftInd ].first < (*toSort)[ rightInd ].first;
            }
        
    private:
        
        static bool
            isSorted( std::vector<wordWeightPair> const &ids, std::vector<uint32_t> const &inds );
            
};










template<class DescType>
void
putative_desc<DescType>::getPutativeMatches(
        DescType const* desc1, uint32_t size1,
        DescType const* desc2, uint32_t size2,
        uint32_t nDims,
        std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches,
        bool useLowe,
        float deltaSq,
        float epsilon ) {
    
    float dsq;
    
    if (!useLowe) { // Use the epsilon measure.
        
        for (uint32_t i=0; i<size1 ; ++i) {
            for (uint32_t j=0; j<size2 ; ++j) {
                dsq = jp_dist_l2(desc1+i*nDims, desc2+j*nDims, nDims);
                if (dsq < (epsilon*epsilon))
                    putativeMatches.push_back(std::make_pair(i, j));
            }
        }
      
    } else {
        
        static const int numnn = 2;
        std::pair<uint32_t,float> nns[numnn+1];
        int k;
        
        for (uint32_t i=0; i<size1; ++i) {
            
            nns[0] = nns[1] = nns[2] = std::make_pair(-1, std::numeric_limits<float>::max());
            for (uint32_t j=0; j<size2; ++j) {
                
                // One potential improvement is to copy desc1 into an aligned
                // buffer...
                dsq = jp_dist_l2(desc1+i*nDims, desc2+j*nDims, nDims);
                
                if (dsq < nns[numnn-1].second) {
                    for (k=numnn; k>0 && nns[k-1].second > dsq; --k)
                        nns[k] = nns[k-1];
                    nns[k] = std::make_pair(j, dsq);
                }
                
            }
            ASSERT(nns[0].second <= nns[1].second);
            if ((nns[0].second/nns[1].second) < deltaSq) {
                putativeMatches.push_back(std::make_pair(i, nns[0].first));
            }
            
        }
        
    }
    
}




#endif
