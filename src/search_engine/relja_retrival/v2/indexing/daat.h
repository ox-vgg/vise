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

#ifndef _DAAT_H_
#define _DAAT_H_



#include <queue>
#include <stdint.h>
#include <vector>

#include "index_entry_util.h"
#include "index_entry.pb.h"
#include "macros.h"
#include "uniq_entries.h"
#include "util.h"



// for efficiency iterating is done only over unique IDs (i.e. using ueIter->incrementToDifferent)

class daat {
    
    public:
        
        daat(precompUEIterator *ueIter,
             std::vector<uint32_t> const *docIDs= NULL,
             uint32_t *docID= NULL);
        
        ~daat(){
            util::delPointerVector(entryVecs_);
            if (delDocIDs_)
                delete docIDs_;
        }
        
        void
            advance();
        
        // NOTE: only entryInd[ nonEmptyEntryInd ] are valid, others can be junk
        inline bool
            getMatches(std::vector< std::pair<uint32_t,uint32_t> > const *&entryInd, std::vector<uint32_t> const *&nonEmptyEntryInd) const {
                if (nonEmptyEntryInd_.size()>0){
                    nonEmptyEntryInd= &nonEmptyEntryInd_;
                    entryInd= &entryInd_;
                    return true;
                } else
                    return false;
            }
        
        inline uint32_t
            getDocID() const
                { return docID_; }
        
        inline bool
            isEnd() const
                { return isEnd_; }
    
    private:
        
        void
            advanceOne(bool doMatching= true);
        
        struct orderIDs {
            bool operator()(std::pair<uint32_t,uint32_t> const &l, std::pair<uint32_t,uint32_t> const &r){
                return l.second > r.second;
            }
        };
        
        bool isEnd_, delDocIDs_;
        std::vector<uint32_t> const *docIDs_;
        uint32_t docIDInd_, docID_;
        std::vector< std::vector<rr::indexEntry>* > entries_; // don't delete
        std::vector<indexEntryVector*> entryVecs_;
        
        // current matching start-end pairs for every query word
        std::vector< std::pair<uint32_t,uint32_t> > entryInd_;
        std::vector<uint32_t> nonEmptyEntryInd_;
        
        // heap with indexInd-docID pair; note indexInd != wordID but unique index of given ueIter
        // i.e. if ueIter contains ID's [0,0,1,4,4,4,4,6] then indexInd can only be [0,1,2,3] where the correspondence is 0->0, 1->1, 2->4, 3->6
        std::priority_queue< std::pair<uint32_t,uint32_t>,
                             std::vector< std::pair<uint32_t,uint32_t> >,
                             orderIDs > queue_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(daat)
};

#endif
