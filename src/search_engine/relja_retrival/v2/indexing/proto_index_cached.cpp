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

#include "proto_index_cached.h"



protoIndexCached::protoIndexCached(protoDb const &db, bool precompQuantEl, uint32_t maxCacheSize) :
        protoIndex(db, precompQuantEl),
        numIDs_(db.numIDs()),
        maxCacheSize_(maxCacheSize),
        size_(0) {
    
    keyToCachePosValid_.resize(numIDs_, false);
    keyToCachePos_.resize(numIDs_);
    keyToValue_.resize(numIDs_);
    keyToN_.resize(numIDs_);
}



protoIndexCached::~protoIndexCached(){
}



uint32_t
protoIndexCached::getEntries( uint32_t ID, std::vector<rr::indexEntry> &entries ) const {
    
    if (ID > numIDs_){
        entries.clear();
        return 0;
    }
    
    const uint32_t &key= ID;
    
    boost::mutex::scoped_lock lock(cacheLock_);
    
    if (!keyToCachePosValid_[key]){
        // cache miss
        
        // get the values
        
        lock.unlock(); // as this stuff is expensive and doesn't require cache access
        uint32_t N= protoIndex::getEntries(ID, entries);
        
        // do caching
        
        boost::mutex::scoped_lock lock2(cacheLock_);
        
        // could be that in mean time another process loaded it now, so don't double write stuff
        
        if (!keyToCachePosValid_[key]){
            // TODO do in another thread?
            
            // check if full
            if (size_ == maxCacheSize_){
                // delete last
                uint32_t delKey= cache_.back();
                cache_.pop_back();
                keyToCachePosValid_[delKey]= false;
                keyToValue_[delKey].clear();
            } else {
                ASSERT(size_ < maxCacheSize_);
                ++size_;
            }
            
            // add to cache
            cache_.push_front(key);
            keyToCachePos_[key]= cache_.begin();
            keyToCachePosValid_[key]= true;
            keyToValue_[key]= entries;
            keyToN_[key]= N;
        }
        
        return N;
        
    } else {
        // cache hit
        
        // read from cache and update it
        
        cacheListType::iterator &it= keyToCachePos_[key];
        if ( it != cache_.begin() ){
            cacheListType::iterator itNext= it;
            std::advance(itNext, 1);
            cache_.splice( cache_.begin(), cache_, it, itNext );
        }
        entries= keyToValue_[key];
        return keyToN_[key];
    }
}
