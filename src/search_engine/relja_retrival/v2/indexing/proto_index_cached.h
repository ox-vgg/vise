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

#ifndef _PROTO_INDEX_CACHED_H_
#define _PROTO_INDEX_CACHED_H_

#include <list>

#include <boost/thread/mutex.hpp>

#include "proto_index.h"
#include "macros.h"



class protoIndexCached : public protoIndex {
    
    public:
        
        protoIndexCached(protoDb const &db, bool precompQuantEl= true, uint32_t maxCacheSize= 10000);
        
        ~protoIndexCached();
        
        uint32_t
            getEntries( uint32_t ID, std::vector<rr::indexEntry> &entries ) const;
    
    private:
        
        typedef std::list<uint32_t> cacheListType;
        
        const uint32_t numIDs_;
        const uint32_t maxCacheSize_;
        
        mutable boost::mutex cacheLock_;
        mutable std::vector< cacheListType::iterator > keyToCachePos_;
        mutable std::vector<bool> keyToCachePosValid_;
        mutable std::vector< std::vector<rr::indexEntry> > keyToValue_;
        mutable std::vector<uint32_t> keyToN_;
        mutable cacheListType cache_;
        mutable uint32_t size_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoIndexCached)
};

#endif
