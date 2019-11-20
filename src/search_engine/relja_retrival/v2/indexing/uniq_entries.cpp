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

#include "uniq_entries.h"



void
uniqEntries::getUniqIndToInd(std::vector<int> &uniqIndToInd) const {
    int size= index_.size();
    for (int i= 0; i < size; ++i)
        if (i==0 || index_[i-1]!=index_[i])
            uniqIndToInd.push_back(i);
    uniqIndToInd.push_back( size ); // for convenience
}



std::vector<rr::indexEntry>*
precompUEIterator::getEntriesConst() const {
    return &( ue_->allEntries_[ ue_->index_[ind_] ] );
}



void
precompUEIterator::incrementToDifferent() {
    uint32_t prevIndex= ue_->index_[ind_];
    for (++ind_; ind_ < ue_->index_.size() && prevIndex == ue_->index_[ind_]; ++ind_);
}




std::vector<rr::indexEntry>*
onlineUEIterator::getEntries() {
    uint32_t currID= queryRep_->id(ind_);
    if (firstLoad_ || loadedID_!=currID){
        loadedID_= currID;
        idx_->getEntries(loadedID_, entries_);
        firstLoad_= false;
    }
    return &entries_;
}



void
onlineUEIterator::incrementToDifferent() {
    uint32_t prevID= queryRep_->id(ind_);
    uint32_t size= static_cast<uint32_t>(queryRep_->id_size());
    for (++ind_; ind_ < size && prevID == queryRep_->id(ind_); ++ind_);
}
