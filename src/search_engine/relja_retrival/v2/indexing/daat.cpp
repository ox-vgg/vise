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

#include "daat.h"

#include <algorithm>

#ifdef DAAT_USE_BINARY_SEARCH
#include "index_entry_util.h"
#endif



daat::daat(
        precompUEIterator *ueIter,
        std::vector<uint32_t> const *docIDs,
        uint32_t *docID)
        : isEnd_(true), delDocIDs_(docID!=NULL), docIDs_( docID==NULL ? docIDs : new std::vector<uint32_t> const (1,*docID) ), docIDInd_(0) {
    
    ASSERT(docIDs==NULL || docID==NULL);
    
    if (docIDs_==NULL)
        docID_= 0;
    else
        docID_= docIDs_->at(docIDInd_);
    
    for (uint32_t iQueryID= 0; !ueIter->isEnd(); ++iQueryID, ueIter->incrementToDifferent()){
        
        entries_.push_back( ueIter->getEntries() );
        bool isEmpty= indexEntryUtil::isEmpty( *entries_.back() );
        entryVecs_.push_back( new indexEntryVector(*entries_.back()) );
        
        isEnd_= isEnd_ && isEmpty;
        
        // set to no match
        entryInd_.push_back(std::make_pair(0,0));
        
        // add to the queue
        if (!isEmpty)
            queue_.push(std::make_pair(iQueryID, entryVecs_.back()->getID(0)));
    }
    
}



void
daat::advance() {
    
    if (isEnd())
        return;
    
    // clear matches
    nonEmptyEntryInd_.clear();
    
    uint32_t currID= queue_.top().second;
    
    // move current docID to the one with non-zero results
    while (docID_ != currID) {
        if (currID < docID_) {
            // currID catch up with docID by advancing the least advanced index
            advanceOne(false);
            if (isEnd())
                return;
            currID= queue_.top().second;
        } else {
            // docID catch up with currID
            if (docIDs_==NULL)
                docID_= currID;
            else {
                for (; docIDInd_ < docIDs_->size() && docIDs_->at(docIDInd_) < currID; ++docIDInd_);
                if (docIDInd_ < docIDs_->size())
                    docID_= docIDs_->at(docIDInd_);
                else {
                    isEnd_= true;
                    return;
                }
            }
        }
    }
    
    // advance the indexes which all point to currID and get matches
    do {
        advanceOne();
    } while (!isEnd() && queue_.top().second==currID);
    
}



void
daat::advanceOne(bool doMatching){
    
    ASSERT(!queue_.empty());
    
    // get least advanced inverted index
    std::pair<uint32_t, uint32_t> wordUniqIndDocID= queue_.top();
    queue_.pop();
    uint32_t wordUniqInd= wordUniqIndDocID.first;
    
    // find the match in this one
    std::pair<uint32_t, uint32_t> &entryInd= entryInd_[wordUniqInd];
    indexEntryVector const &entryVec= *(entryVecs_[wordUniqInd]);
    uint32_t num= entryVec.getNum();
    
    // advance the start marker
    #if DAAT_USE_BINARY_SEARCH
    // note: binary search seems to be slower
    ievIterator lb= std::lower_bound( entryVec.getIter(entryInd.second), entryVec.endIter(), docID_ );
    entryInd.first= lb.getInd();
    #else
    for (entryInd.first= entryInd.second;
         entryInd.first < num && entryVec.getID(entryInd.first) < docID_;
         ++entryInd.first);
    #endif
    
    if (doMatching){
        
        // advance the end marker
        for (entryInd.second= entryInd.first;
             entryInd.second < num && entryVec.getID(entryInd.second)==docID_;
             ++entryInd.second);
        
        if (entryInd.second - entryInd.first > 0)
            nonEmptyEntryInd_.push_back(wordUniqInd);
        
    } else {
        entryInd.second= entryInd.first;
    }
    
    // check if reached the end of the index
    if (entryInd.second==num){
        // end of posting list
        isEnd_= queue_.empty();
        return;
    }
    
    // re-add the advanced index into the queue
    queue_.push(std::make_pair(wordUniqInd, entryVec.getID(entryInd.second) ));
}
