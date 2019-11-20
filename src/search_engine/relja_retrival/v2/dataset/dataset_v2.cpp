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

#include "dataset_v2.h"

#include <stdio.h>
#include <vector>

#include <boost/functional/hash.hpp>

#include "util.h"



uint32_t const datasetV2::emptyEntry_= std::numeric_limits<uint32_t>::max();

datasetV2::datasetV2(
        std::string fileName,
        std::string addPrefix,
        std::string removePrefix)
        : addPrefixFullLen_( util::expandUser(addPrefix).length() ),
          removePrefixLen_(removePrefix.length()),
          addPrefix_(addPrefix),
          removePrefix_(removePrefix) {
    
    std::cout<<"datasetV2::datasetV2: Loading dataset from file= "<<fileName<<"\n";
    protoDbFile dbFile(fileName);
    db_= new protoDbInRam(dbFile, false);
    
    numIDs_= db_->numIDs();
    ASSERT(numIDs_>0);
    
    std::vector<rr::datasetEntry> entries;
    db_->getProtos(0, entries);
    ASSERT(entries.size()==1);
    numPerID_= static_cast<uint32_t>( entries[0].filename_size() );
    
    db_->getProtos(numIDs_-1, entries);
    ASSERT(entries.size()==1);
    rr::datasetEntry const &entry= entries[0];
    numDocs_= numPerID_ * (numIDs_-1) + static_cast<uint32_t>(entry.filename_size());
    
    std::string fn= entry.filename(entry.filename_size()-1);
    std::cout<<"datasetV2::datasetV2: Loaded info about "<<numDocs_<<" images\n";
    std::cout<<"datasetV2::datasetV2: last image:\n"<< fn <<"\n";
    
    if (removePrefixLen_>0){
        ASSERT(fn.length()>removePrefixLen_);
        ASSERT(std::string(fn.begin(), fn.begin() + removePrefixLen_) == removePrefix);
    }
    
    memoryEffLookup_= numDocs_>5000000;
    if (memoryEffLookup_)
        hashTable_.resize( numDocs_*3, emptyEntry_ );
    
    // make map fn->ID
    uint32_t docID= 0;
    std::string insertFn;
    uint32_t hashKey;
    
    for (uint32_t ID= 0; ID<numIDs_; ++ID){
        db_->getProtos(ID, entries);
        ASSERT(entries.size()==1);
        rr::datasetEntry const &entry= entries[0];
        for (int i= 0; i<entry.filename_size(); ++i, ++docID){
            std::string const &fn= entry.filename(i);
            insertFn= (removePrefixLen_>0) ?
                    std::string( fn.begin()+removePrefixLen_, fn.end() ) :
                    fn;
            if (memoryEffLookup_){
                for (hashKey= hashFunc_(insertFn) % hashTable_.size();
                     hashTable_[hashKey]!=emptyEntry_;
                     hashKey= (hashKey+1) % hashTable_.size());
                hashTable_[hashKey]= docID;
            } else
                fn2ID_[ insertFn ]= docID;
        }
    }
    std::cout<<"datasetV2::datasetV2: Generated fn2ID\n";
}



datasetV2::~datasetV2(){
    delete db_;
}



std::string
datasetV2::getInternalFn( uint32_t docID ) const {
    
    ASSERT(docID<=numDocs_);
    
    uint32_t entriesID= docID / numPerID_;
    ASSERT(entriesID<numIDs_);
    
    std::vector<rr::datasetEntry> entries;
    db_->getProtos(entriesID, entries);
    ASSERT(entries.size()==1);
    return entries[0].filename(docID % numPerID_);
}



std::string
datasetV2::getFn( uint32_t docID ) const {
    std::string const fn= getInternalFn(docID);
    if (removePrefixLen_>0)
        return util::expandUser( addPrefix_ + std::string(fn.begin() + removePrefixLen_, fn.end()) );
    else
        return util::expandUser( addPrefix_ + fn );
}




std::pair<uint32_t, uint32_t>
datasetV2::getWidthHeight( uint32_t docID ) const {
    
    ASSERT(docID<=numDocs_);
    
    uint32_t entriesID= docID / numPerID_;
    ASSERT(entriesID<numIDs_);
    
    std::vector<rr::datasetEntry> entries;
    db_->getProtos(entriesID, entries);
    ASSERT(entries.size()==1);
    rr::datasetEntry const &entry= entries[0];
    
    return std::make_pair(entry.width(docID % numPerID_), entry.height(docID % numPerID_));
}



uint32_t
datasetV2::getDocID(std::string fn) const {
    uint32_t const *docID= lookupFn( fn );
    if (docID==NULL)
        throw std::runtime_error("Unknown filename");
    else
        return *docID;
}



uint32_t
datasetV2::getDocIDFromAbsFn(std::string fn) const {
    uint32_t const *docID= lookupFn( util::expandUser(removePrefix_) + fn.substr( addPrefixFullLen_ ) );
    if (docID==NULL)
        throw std::runtime_error("Unknown filename");
    else
        return *docID;
}



bool
datasetV2::containsFn( std::string fn ) const {
    return lookupFn(fn)!=NULL ||
           (fn.length()>addPrefixFullLen_ && lookupFn( util::expandUser(removePrefix_) + fn.substr( addPrefixFullLen_ ) ) != NULL);
}



uint32_t const *
datasetV2::lookupFn(std::string const &fn) const {
    
    if (memoryEffLookup_){
        
        uint32_t hashKey= hashFunc_(fn) % hashTable_.size();
        for (; hashTable_[hashKey]!=emptyEntry_ && fn!=getInternalFn(hashTable_[hashKey]);
               hashKey= (hashKey+1) % hashTable_.size());
        if (hashTable_[hashKey]==emptyEntry_)
            return NULL;
        return &(hashTable_[hashKey]);
        
    } else {
        
        std::map<std::string, uint32_t>::const_iterator it= fn2ID_.find(fn);
        if (it==fn2ID_.end())
            return NULL;
        return &(it->second);
        
    }
}



datasetBuilder::datasetBuilder(std::string fileName)
    : hasBeenClosed_(false),
      dbBuilder_(fileName, "dataset"),
      entriesID_(0) {
    
    buffer_.mutable_filename()->Reserve(numPerID_);
    buffer_.mutable_width()->Reserve(numPerID_);
    buffer_.mutable_height()->Reserve(numPerID_);
}



void
datasetBuilder::close(){
    if (!dbBuilder_.hasBeenClosed()){
        if (buffer_.filename_size()>0)
            save(true);
        dbBuilder_.close();
        hasBeenClosed_= true;
    }
}



void
datasetBuilder::save(bool isLast){
    
    ASSERT(isLast || static_cast<uint32_t>(buffer_.filename_size())==numPerID_);
    
    dbBuilder_.addProto(entriesID_, buffer_);
    ++entriesID_;
    
    buffer_.Clear();
}



void
datasetBuilder::add(std::string fn, uint32_t width, uint32_t height){
    
    ASSERT(!dbBuilder_.hasBeenClosed());
    
    ASSERT(static_cast<uint32_t>(buffer_.filename_size()) < numPerID_);
    buffer_.add_filename(fn);
    buffer_.add_width(width);
    buffer_.add_height(height);
    
    if (static_cast<uint32_t>(buffer_.filename_size()) == numPerID_)
        save();
    
}
