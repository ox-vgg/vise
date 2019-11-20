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

#ifndef _INDEX_ENTRY_UTIL_H_
#define _INDEX_ENTRY_UTIL_H_

#include <stdint.h>
#include <vector>

#include <boost/iterator/iterator_facade.hpp>

#include "embedder.h"
#include "index_entry.pb.h"
#include "macros.h"
#include "query.h"



namespace indexEntryUtil {
    
    // number of ids / diffids
    uint32_t
        getNum(rr::indexEntry const &entry);
    
    uint32_t
        getNum(std::vector<rr::indexEntry> const &entries);
    
    bool
        isEmpty(std::vector<rr::indexEntry> const &entries);
    
    // number of unique ids, assumes sorted ids or having diffids
    uint32_t
        getUniqNum(std::vector<rr::indexEntry> const &entries);
    
    void
        toDiff(rr::indexEntry &entry);
    
    void
        fromDiff(rr::indexEntry &entry);
    
    void
        quantXY(rr::indexEntry &entry);
    
    void
        unquantXY(rr::indexEntry &entry);
    
    void
        quantEllipse(rr::indexEntry &entry);
    
    void
        unquantEllipse(rr::indexEntry &entry);
    
    void
        preUnquantEllipse(float *&aQuant,
                          float *&bQuant,
                          float *&cQuant);
    
    inline void
        predUnquantEllipse(float const *aQuant,
                           float const *bQuant,
                           float const *cQuant,
                           unsigned char scale,
                           unsigned char ratio,
                           unsigned char angle,
                           float &a, float &b, float &c){
            uint32_t ind=
                static_cast<uint32_t>(scale)*256*256 +
                static_cast<uint32_t>(ratio)*256 +
                static_cast<uint32_t>(angle);
            a= aQuant[ind];
            b= bQuant[ind];
            c= cQuant[ind];
        }
    
    void
        predUnquantEllipse(float const *aQuant,
                           float const *bQuant,
                           float const *cQuant,
                           rr::indexEntry &entry);
    
    inline unsigned char
        quantizeTo256(double val, double min, double max){
            return static_cast<unsigned char>( round( ( std::min(std::max(val, min), max)-min)*255.0/(max-min) ) );
        }
    
    inline double
        unquantizeFrom256(unsigned char qval, double min, double max){
            return static_cast<double>(qval)*(max-min)/255 + min;
        }
    
    static const double scale_min= -35, scale_max= -8;
    static const double ratio_min= 0, ratio_max= 6;
    static const double angle_min= -1.5708, angle_max= +1.5708;
    static const double pi= 3.141592653589793, piHalf= 1.570796326794897;
    
    // find ranges of entry that fall inside a query ROI
    uint32_t
        markInside(rr::indexEntry const &entry,
                   query const &queryObj,
                   std::vector< std::pair<uint32_t, uint32_t> > &inds,
                   int *begin= NULL, int *end= NULL, // if NULL do entire entry, otherwise in [*begin, *end)
                   uint32_t offset= 0);
    
    // find ranges of entries that fall inside a query ROI
    // if input inds is not empty, then we search only items that fall into range specified by the input (input ranges are assumed to be sorted)
    uint32_t
        markInside(std::vector<rr::indexEntry> const &entries,
                   query const &queryObj,
                   std::vector< std::pair<uint32_t, uint32_t> > &inds);
    
    void
        copyRange(rr::indexEntry const &from,
                  int begin, int end,
                  rr::indexEntry &to,
                  uint32_t const *ID= NULL, // if NULL keep ID from 'from', otherwise put *ID
                  bool copyXY= false,
                  bool copyEllipse= false,
                  embedderFactory const *embFactory= NULL);
    
    void
        copyRanges(std::vector<rr::indexEntry> const &from,
                   std::vector< std::pair<uint32_t, uint32_t> > const &inds,
                   rr::indexEntry &to,
                   uint32_t const *ID= NULL, // if NULL keep ID from 'from', otherwise put *ID
                   bool copyXY= false,
                   bool copyEllipse= false,
                   embedderFactory const *embFactory= NULL);
    
    
    class argSort {
    
        public:
            static void
                sort( rr::indexEntry const &toSort, std::vector<int> &inds ){
                    inds.clear(); inds.reserve( toSort.id_size() );
                    for (int i=0; i < toSort.id_size(); ++i)
                        inds.push_back( i );
                    argSort argSorter( toSort );
                    
                    std::sort( inds.begin(), inds.end(), argSorter );
                }
            
            #define IEU_ARGSORT_COMPARE(fieldName) \
                if (toSort_->fieldName(leftInd) < toSort_->fieldName(rightInd)) \
                    return true; \
                if (toSort_->fieldName(leftInd) > toSort_->fieldName(rightInd)) \
                    return false;
            
            #define IEU_ARGSORT_COMPARE_IF_EXISTS(fieldName) \
                if (toSort_->fieldName ## _size()){ \
                    IEU_ARGSORT_COMPARE(fieldName) \
                }
            
            inline int
                operator()( int leftInd, int rightInd ) const {
                    
                    // sort by id
                    IEU_ARGSORT_COMPARE(id)
                    
                    IEU_ARGSORT_COMPARE_IF_EXISTS(docid)
                    IEU_ARGSORT_COMPARE_IF_EXISTS(x)
                    IEU_ARGSORT_COMPARE_IF_EXISTS(qx)
                    IEU_ARGSORT_COMPARE_IF_EXISTS(y)
                    IEU_ARGSORT_COMPARE_IF_EXISTS(qy)
                    IEU_ARGSORT_COMPARE_IF_EXISTS(a)
                    IEU_ARGSORT_COMPARE_IF_EXISTS(b)
                    IEU_ARGSORT_COMPARE_IF_EXISTS(c)
                    
                    return true;
                }

        private:
            
            argSort( rr::indexEntry const &toSort ) : toSort_(&toSort) {}
            rr::indexEntry const *toSort_;
            
        };
};



class ellipseUnquantizer {
    
    public:
        
        ellipseUnquantizer(){
            indexEntryUtil::preUnquantEllipse(aQuant_, bQuant_, cQuant_);
        }
        
        ~ellipseUnquantizer(){
            delete []aQuant_; delete []bQuant_; delete []cQuant_;
        }
        
        inline void
            unquantize(rr::indexEntry &entry) const {
                indexEntryUtil::predUnquantEllipse(aQuant_, bQuant_, cQuant_, entry);
            }
        
        inline void
            unquantize(unsigned char scale,
                       unsigned char ratio,
                       unsigned char angle,
                       float &a, float &b, float &c) const {
                indexEntryUtil::predUnquantEllipse(aQuant_, bQuant_, cQuant_, scale, ratio, angle, a, b, c);
            }
        
    private:
        float *aQuant_, *bQuant_, *cQuant_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(ellipseUnquantizer)
};



class indexEntryVector;

// shoud only be used as const iterator
class ievIterator : public boost::iterator_facade<ievIterator, uint32_t, boost::random_access_traversal_tag, uint32_t>  {
    
    public:
        
        ievIterator() : ind_(0), iev_(NULL) {} // not correct but std::lower_bound needs it
        ievIterator(uint32_t ind, indexEntryVector const *iev) : ind_(ind), iev_(iev) {}
        uint32_t dereference() const;
        bool equal(ievIterator const &it) const;
        void increment();
        void decrement();
        void advance(uint32_t n);
        size_t distance_to(ievIterator const &it) const;
        
        inline uint32_t getInd() const { return ind_; }
        
    
    private:
        uint32_t ind_;
        indexEntryVector const *iev_;
};



class indexEntryVector {
    
    public:
        
        indexEntryVector(std::vector<rr::indexEntry> const &entries);
        
        std::pair<uint32_t, int>
            getInds(uint32_t ind) const;
        
        uint32_t
            getID(uint32_t ind) const;
        
        inline uint32_t
            getNum() const { return num_; }
        
        inline ievIterator
            beginIter() const {
                return ievIterator(0, this);
            }
        
        inline ievIterator
            endIter() const {
                return ievIterator(num_, this);
            }
        
        inline ievIterator
            getIter(uint32_t ind) const {
                return ievIterator(ind, this);
            }
    
    private:
        uint32_t entriesSize_, num_;
        std::vector<rr::indexEntry> const *entries_;
        std::vector<int> nEntry_;
        std::vector<uint32_t> offset_;
        bool diffIDs_;
        
    private:
        DISALLOW_COPY_AND_ASSIGN(indexEntryVector)
};

#endif
