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

#include <algorithm>
#include <vector>



template <class T>
class argSort {
    
    public:
        
        static void
            sort( std::vector<T> const &aToSort, std::vector<uint32_t> &inds );
        
        static void
            partialSort( std::vector<T> const &aToSort, std::vector<uint32_t> &inds, uint32_t n );
        
        inline int
            operator()( uint32_t leftInd, uint32_t rightInd ) const {
                // careful here because has to be strictly < and not <=, otherwise sometimes causes segmentation fault!
                return (*toSort)[ leftInd ] < (*toSort)[ rightInd ];
            }
    
    private:
        
        argSort( std::vector<T> const &aToSort ) : toSort(&aToSort) {}
        std::vector<T> const *toSort;
        
};



template <class T>
class argSortArray {
    
    public:
        
        static void
            sort( T const *aToSort, uint32_t aN, std::vector<uint32_t> &inds );
        
        inline int
            operator()( uint32_t leftInd, uint32_t rightInd ) const {
                // careful here because has to be strictly < and not <=, otherwise sometimes causes segmentation fault!
                return toSort[ leftInd ] < toSort[ rightInd ];
            }
    
    private:
        
        argSortArray( T const *aToSort, uint32_t aN ) : toSort(aToSort), n(aN) {}
        T const * const toSort;
        uint32_t n;
};



template <class T>
void
argSort<T>::sort( std::vector<T> const &aToSort, std::vector<uint32_t> &inds ){
    
    inds.clear();
    inds.reserve( aToSort.size() );
    
    for (uint32_t i=0; i < aToSort.size(); ++i)
        inds.push_back( i );
    
    argSort<T> argSorter( aToSort );
    
    std::sort( inds.begin(), inds.end(), argSorter );
    
}



template <class T>
void
argSort<T>::partialSort( std::vector<T> const &aToSort, std::vector<uint32_t> &inds, uint32_t n ){
    
    inds.clear();
    inds.reserve( aToSort.size() );
    
    for (uint32_t i=0; i < aToSort.size(); ++i)
        inds.push_back( i );
    
    argSort<T> argSorter( aToSort );
    
    std::partial_sort( inds.begin(), inds.begin()+n, inds.end(), argSorter );
    inds.resize(n);
    
}



template <class T>
void
argSortArray<T>::sort( T const *aToSort, uint32_t aN, std::vector<uint32_t> &inds ){
    
    inds.clear();
    inds.reserve( aN );
    
    for (uint32_t i=0; i < aN; ++i)
        inds.push_back( i );
    
    argSortArray<T> argSorter( aToSort, aN );
    
    std::sort( inds.begin(), inds.end(), argSorter );
    
}
