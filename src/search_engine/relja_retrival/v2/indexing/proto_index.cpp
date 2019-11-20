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

#include "proto_index.h"

#include <algorithm>

#include "thread_queue.h"



protoIndex::protoIndex(protoDb const &db, bool precompQuantEl) : db_(&db), numIDs_(db.numIDs()), precompQuantEl_(precompQuantEl) {
    if (precompQuantEl_){
        std::cout<<"protoIndex::protoIndex: precomputing quantized ellipses\n";
        indexEntryUtil::preUnquantEllipse(aQuant_, bQuant_, cQuant_);
        std::cout<<"protoIndex::protoIndex: precomputing quantized ellipses - DONE\n";
    } else {
        aQuant_= NULL;
        bQuant_= NULL;
        cQuant_= NULL;
    }
}



protoIndex::~protoIndex(){
    if (precompQuantEl_){
        delete []aQuant_;
        delete []bQuant_;
        delete []cQuant_;
    }
}



uint32_t
protoIndex::getEntries( uint32_t ID, std::vector<rr::indexEntry> &entries ) const {
    
    db_->getProtos<rr::indexEntry>(ID, entries);
    
    uint32_t N= 0;
    
    for (std::vector<rr::indexEntry>::iterator it= entries.begin();
         it!=entries.end();
         ++it) {
        indexEntryUtil::fromDiff(*it);
        N+= indexEntryUtil::getNum(*it);
    }
    
    return N;
}



class uniqLoaderWorker : public queueWorker<bool> {
    public:
        uniqLoaderWorker(protoIndex const &idx,
                         std::vector<uint32_t> const &uniqIDs,
                         std::vector<bool> const &keep,
                         std::vector< std::vector<rr::indexEntry> > &allEntries)
            : idx_(&idx), uniqIDs_(&uniqIDs), keep_(&keep), allEntries_(&allEntries) {}
        
        void operator() ( uint32_t jobID, bool &result ) const {
            if (keep_->at(jobID))
                idx_->getEntries( uniqIDs_->at(jobID), allEntries_->at(jobID) );
            else
                allEntries_->at(jobID).clear();
        }
    private:
        protoIndex const *idx_;
        std::vector<uint32_t> const *uniqIDs_;
        std::vector<bool> const *keep_;
        std::vector< std::vector<rr::indexEntry> > *allEntries_;
        
        DISALLOW_COPY_AND_ASSIGN(uniqLoaderWorker)
};



void
protoIndex::getUniqEntries(
        rr::indexEntry &queryRep,
        uniqEntries &entries ) const {
    
    std::vector<uint32_t> &index= entries.index_;
    std::vector< std::vector<rr::indexEntry> > &allEntries= entries.allEntries_;
    
    index.clear();
    index.reserve(queryRep.id_size());
    allEntries.clear();
    
    uint32_t prevID= 0, currID;
    
    #if 0
    
    for (int i= 0; i<queryRep.id_size(); ++i){
        currID= queryRep.id(i);
        if (i==0 || currID!=prevID) {
            ASSERT(i==0 || prevID<currID);
            allEntries.resize( allEntries.size()+1 );
            getEntries(currID, allEntries.back());
        }
        index.push_back( allEntries.size()-1 );
        prevID= currID;
    }
    
    #else
    
    // get list of unique IDs and index
    
    std::vector<uint32_t> uniqIDs;
    std::vector<bool> keep;
    uniqIDs.reserve(queryRep.id_size());
    keep.reserve(queryRep.id_size());
    
    for (int i= 0; i<queryRep.id_size(); ++i){
        currID= queryRep.id(i);
        keep.push_back( queryRep.keep_size()==0 || queryRep.keep(i) );
        if (i==0 || currID!=prevID) {
            ASSERT(i==0 || prevID<currID);
            uniqIDs.push_back(currID);
        }
        index.push_back( uniqIDs.size()-1 );
        prevID= currID;
    }
    
    // load entries in parallel (basically for parallel protobuf decoding)
    
    allEntries.resize(uniqIDs.size());
    queueManager<bool> manager; // does nothing
    uniqLoaderWorker worker(*this, uniqIDs, keep, allEntries);
    threadQueue<bool>::start( allEntries.size(), worker, manager, 4);
    
    #endif
}



uint32_t
protoIndex::getInverseEntryInds(
        uint32_t invID,
        std::vector<uint32_t> &ID,
        std::vector< std::pair<uint32_t,uint32_t> > &entryInd,
        std::vector<uint32_t> const *lookInIDs ) const {
    
    ID.clear();
    entryInd.clear();
    
    uint32_t N= 0;
    
    if (lookInIDs==NULL){
        for (uint32_t i= 0; i<db_->numIDs(); ++i)
            N+= getInverseEntryInds( invID, ID, entryInd, i );
    } else {
        for (uint32_t ii= 0; ii<lookInIDs->size(); ++ii)
            N+= getInverseEntryInds( invID, ID, entryInd, (*lookInIDs)[ii] );
    }
    
    return N;
}



uint32_t
protoIndex::getInverseEntryInds(
        uint32_t invID,
        std::vector< std::pair<uint32_t,uint32_t> > &entryInd,
        std::vector<rr::indexEntry> const &entries ) const {
    
    uint32_t N= 0;
    
    std::pair<const unsigned int*, const unsigned int*> range;
    
    uint32_t offset= 0;
    
    for (std::vector<rr::indexEntry>::const_iterator it= entries.begin();
         it!=entries.end();
         offset+= it->id_size(), ++it) {
        
        range= std::equal_range(it->id().begin(), it->id().end(), invID);
        if (range.second-range.first != 0){
            // found
            entryInd.push_back(std::make_pair(
                offset + range.first - it->id().begin(), offset + range.second - it->id().begin() ));
            N+= entryInd.back().second - entryInd.back().first;
        }
    }
    
    return N;
    
}



uint32_t
protoIndex::getInverseEntryInds(
        uint32_t invID,
        std::vector<uint32_t> &ID,
        std::vector< std::pair<uint32_t,uint32_t> > &entryInd,
        uint32_t lookID ) const {
    
    std::vector<rr::indexEntry> entries;
    getEntries(lookID, entries);
    
    uint32_t N= getInverseEntryInds(invID, entryInd, entries);
    
    if (N>0)
        ID.push_back(lookID);
    
    return N;
}



void
protoIndex::unquantEllipse(rr::indexEntry &entry) const {
    if (precompQuantEl_)
        indexEntryUtil::predUnquantEllipse(aQuant_, bQuant_, cQuant_, entry);
    else
        indexEntryUtil::unquantEllipse(entry);
}



void
indexBuilder::addEntry( uint32_t ID, rr::indexEntry &entry ){
    
    ASSERT(!hasBeenClosed());
    
    ASSERT( entry.id_size()==0 || entry.diffid_size()==0 );
    if (doDiff_)
        indexEntryUtil::toDiff(entry);
    if (quantXY_)
        indexEntryUtil::quantXY(entry);
    if (quantEl_)
        indexEntryUtil::quantEllipse(entry);
    
    dbBuilder_->addProto(ID, entry);
    
}
