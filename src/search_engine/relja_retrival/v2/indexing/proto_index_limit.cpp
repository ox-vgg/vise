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

#include "proto_index_limit.h"

#include <algorithm>

#include "protobuf_util.h"



protoIndexLimit::protoIndexLimit(uint64_t limit, protoDb const &db, bool precompQuantEl) : protoIndex(db, precompQuantEl), limit_(limit) {
    
    loading_= true;
    
    N_.reserve(numIDs_);
    for (uint32_t ID= 0; ID < numIDs_; ++ID)
        N_.push_back( static_cast<protoIndex const *>(this)->getNumWithID(ID) );
    
    loading_= false;
}



bool IDCountComp(std::pair<int, uint64_t> const &l, std::pair<int, uint64_t> const &r){
    return l.second > r.second;
}



void
protoIndexLimit::getUniqEntries(
        rr::indexEntry &queryRep,
        uniqEntries &entries ) const {
    
    uint32_t prevID= 0, currID;
    
    // get list of IDs and their counts
    
    std::vector< std::pair<int, uint64_t> > IDcount;
    IDcount.reserve( queryRep.id_size() );
    uint64_t totalCount= 0;
    uint32_t N;
    
    for (int i= 0; i<queryRep.id_size(); ++i){
        currID= queryRep.id(i);
        if (i==0 || currID!=prevID)
            IDcount.push_back( std::make_pair(i, 0) );
        N= (queryRep.keep_size()==0 || queryRep.keep(i)) ? N_[currID] : 0;
        totalCount+= N;
        IDcount.back().second+= N;
        prevID= currID;
    }
    
    // if only one ID: keep it
    if (IDcount.size()>1) {
        
        // order in decreasing number of entries
        std::sort(IDcount.begin(), IDcount.end(), IDCountComp);
        
        if (queryRep.keep_size()==0)
            protobufUtil::addManyToEnd(true, queryRep.id_size(), *(queryRep.mutable_keep()));
        
        // pick which ones to ignore
        for (uint32_t iIDcount= 0;
             iIDcount < IDcount.size()-1 && // ensure we don't remove everything by keeping the final ID
             totalCount>limit_;
             ++iIDcount
             ){
            
            int i= IDcount[iIDcount].first;
            currID= queryRep.id(i);
            N= N_[currID];
            for (int j= i; j<queryRep.id_size() && currID==queryRep.id(j); ++j){
                totalCount-= queryRep.keep(j) ? N : 0;
                queryRep.set_keep(j, false);
            }
            
        }
        
//         {
//             uint64_t totalCount2= 0;
//             for (int i= 0; i<queryRep.id_size(); ++i){
//                 currID= queryRep.id(i);
//                 N= (queryRep.keep_size()==0 || queryRep.keep(i)) ? N_[currID] : 0;
//                 totalCount2+= N;
//             }
//             ASSERT( totalCount2==totalCount );
//         }
    }
    
    
    // load remaining entries
    protoIndex::getUniqEntries(queryRep, entries);
}
