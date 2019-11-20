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

#include "putative.h"
#include "argsort.h"
#include "macros.h"

#include <map>



void
indSorter_Hard::sort( std::vector<uint32_t> const &aToSort, std::vector<uint32_t> &inds,
                      std::vector<uint32_t> const *presortedInds1 ){
    
    if (presortedInds1==NULL) {
        
        argSort<uint32_t>::sort(aToSort, inds);
        
    } else {
        inds= *presortedInds1;
        ASSERT( isSorted(aToSort,inds) );
    }
    
}



void
indSorter_Soft::sort( std::vector<wordWeightPair> const &aToSort, std::vector<uint32_t> &inds,
                      std::vector<uint32_t> const *presortedInds1 ){
    
    if (presortedInds1==NULL) {
        
        inds.clear();
        inds.reserve( aToSort.size() );
        for (uint32_t i=0; i < aToSort.size(); ++i)
            inds.push_back( i );
        
        indSorter_Soft indSorterObj( aToSort );
        
        std::sort( inds.begin(), inds.end(), indSorterObj );
        
    } else {
        inds= *presortedInds1;
        ASSERT( isSorted(aToSort,inds) );
    }
    
}



void
putative_quantized::getPutativeMatches_Hard(
        std::vector<uint32_t> const &ids1, std::vector<uint32_t> const &ids2,
        std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches,
        std::vector<uint32_t> const *presortedInds1 ){
    
    std::vector<uint32_t> inds1, inds2;
    indSorter_Hard::sort( ids1, inds1, presortedInds1 );
    indSorter_Hard::sort( ids2, inds2 );
    
    putativeMatches.clear();
    
    uint32_t i1=0, i2=0, i2temp;
    
    while ( i1 < inds1.size() && i2 < inds2.size() ){
        if   (ids1[ inds1[i1] ] < ids2[ inds2[i2] ])
            ++i1;
        else if (ids1[ inds1[i1] ] > ids2[ inds2[i2] ])
            ++i2;
        else {
            // equal
            i2temp= i2;
            while ( i2temp < inds2.size() && ids1[ inds1[i1] ] == ids2[ inds2[i2temp] ] ){
                putativeMatches.push_back( std::make_pair(inds1[i1], inds2[i2temp]) );
                ++i2temp;
            }
            ++i1;
        }
    }
    
}



void
putative_quantized::getPutativeMatches_Soft(
        std::vector<wordWeightPair> const &ids1, std::vector<uint32_t> const &qDInds1,
        std::vector<wordWeightPair> const &ids2, std::vector<uint32_t> const &qDInds2,
        std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches,
        std::vector<double> &weights,
        std::vector<uint32_t> const *presortedInds1 ){
    
    std::vector<uint32_t> inds1, inds2;
    indSorter_Soft::sort( ids1, inds1, presortedInds1 );
    indSorter_Soft::sort( ids2, inds2 );
    
    putativeMatches.clear();
    weights.clear();
    std::map< std::pair<uint32_t, uint32_t>, double > PM_map;
    
    uint32_t i1=0, i2=0, i2temp;
    
    while ( i1 < inds1.size() && i2 < inds2.size() ){
        if   (ids1[ inds1[i1] ].first < ids2[ inds2[i2] ].first)
            ++i1;
        else if (ids1[ inds1[i1] ].first > ids2[ inds2[i2] ].first)
            ++i2;
        else {
            // equal
            i2temp= i2;
            while ( i2temp < inds2.size() && ids1[ inds1[i1] ].first == ids2[ inds2[i2temp] ].first ){
                PM_map[ std::make_pair( qDInds1[ inds1[i1] ], qDInds2[ inds2[i2temp] ] ) ]+=
                     ids1[ inds1[i1] ].second * ids2[ inds2[i2temp] ].second;
                ++i2temp;
            }
            ++i1;
        }
    }
    
    for (std::map< std::pair<uint32_t, uint32_t>, double >::iterator itPM= PM_map.begin();
         itPM!=PM_map.end();
         ++itPM){
        putativeMatches.push_back( itPM->first );
        weights.push_back( itPM->second );
    }
    
}



bool
indSorter_Hard::isSorted( std::vector<uint32_t> const &ids, std::vector<uint32_t> const &inds ){
    
    uint32_t prevID= 0;
    
    for (std::vector<uint32_t>::const_iterator itI=inds.begin(); itI!=inds.end(); ++itI){
        if (ids[*itI] < prevID)
            return false;
        prevID= ids[*itI];
    }
    
    return true;
    
}



bool
indSorter_Soft::isSorted( std::vector<wordWeightPair> const &ids, std::vector<uint32_t> const &inds ){
    
    uint32_t prevID= 0;
    
    for (std::vector<uint32_t>::const_iterator itI=inds.begin(); itI!=inds.end(); ++itI){
        if (ids[*itI].first < prevID)
            return false;
        prevID= ids[*itI].first;
    }
    
    return true;
    
}
