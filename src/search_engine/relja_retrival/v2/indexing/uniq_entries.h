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

#ifndef _UNIQ_ENTRIES_H_
#define _UNIQ_ENTRIES_H_

#include <stdint.h>
#include <vector>

#include "index_entry.pb.h"
#include "macros.h"
#include "proto_index.h"



struct uniqEntries {
    uniqEntries(){}
    std::vector<uint32_t> index_;
    std::vector< std::vector<rr::indexEntry> > allEntries_;
    
    // ind into unique entries, i.e. for entries [0,0,5,6,6,8] index_ is [0,0,1,2,2,3] while uniqIndToInd is [0,2,3,5]
    // we also add index_.size() for convenience
    // output vector is int as it will be used for rr::indexEntry
    void
        getUniqIndToInd(std::vector<int> &uniqIndToInd) const;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(uniqEntries)
};



class ueIterator {
    
    public:
        
        ueIterator(uint32_t num, uint32_t ind= 0) : num_(num), ind_(ind) {}
        
        virtual std::vector<rr::indexEntry>*
            getEntries() =0;
        
        inline bool
            equal(ueIterator const &it) const
                { return it.ind_ == ind_; }
        
        inline bool
            isEnd()
                { return ind_==num_; }
        
        inline void
            increment()
                { ++ind_; }
        
        virtual void
            incrementToDifferent() =0;
        
        inline void
            advance(uint32_t n)
                { ind_+= n; }
        
        inline void
            reset()
                { ind_= 0; }
        
        inline uint32_t
            getInd() const { return ind_; }
        
        inline uint32_t
            getNum() const { return num_; }
        
    
    protected:
        uint32_t num_, ind_;
};



class precompUEIterator : public ueIterator {
    
    public:
        
        precompUEIterator(uniqEntries &ue, uint32_t ind= 0) : ueIterator(ue.index_.size(), ind), ue_(&ue) {}
        
        std::vector<rr::indexEntry>*
            getEntriesConst() const;
        
        inline std::vector<rr::indexEntry>*
            getEntries() { return getEntriesConst(); }
        
        void
            incrementToDifferent();
    
    private:
        uniqEntries *ue_;
};



class protoIndex;

class onlineUEIterator : public ueIterator {
    
    public:
        
        onlineUEIterator(rr::indexEntry const &queryRep, protoIndex const &idx, uint32_t ind= 0) : ueIterator(static_cast<uint32_t>(queryRep.id_size()), ind), queryRep_(&queryRep), idx_(&idx), firstLoad_(true), loadedID_(0) {}
        
        // careful, calling getEntries after changing the iterator potentially invalidates previously returned pointers, so don't store them (not the case for precompUEIterator)
        std::vector<rr::indexEntry>*
            getEntries();
        
        void
            incrementToDifferent();
    
    private:
        rr::indexEntry const *queryRep_;
        protoIndex const *idx_;
        bool firstLoad_;
        uint32_t loadedID_;
        std::vector<rr::indexEntry> entries_;
};

#endif
