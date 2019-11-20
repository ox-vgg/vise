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

#ifndef _PROTO_INDEX_H_
#define _PROTO_INDEX_H_


#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#include <boost/function.hpp>

#include "index_entry.pb.h"
#include "index_entry_util.h"
#include "macros.h"
#include "proto_db.h"
#include "slow_construction.h"
#include "timing.h"
#include "uniq_entries.h"



class uniqEntries;

class protoIndex {
    
    public:
        
        protoIndex(protoDb const &db, bool precompQuantEl= true);
        
        virtual
            ~protoIndex();
        
        inline uint32_t
            numIDs() const { return db_->numIDs(); }
        
        inline bool
            contains( uint32_t ID ) const {
                return db_->contains(ID);
            }
        
        virtual uint32_t
            getNumWithID( uint32_t ID ) const {
                std::vector<rr::indexEntry> entries;
                return getEntries(ID, entries);
            }
        
        inline uint32_t
            getUniqNumWithID( uint32_t ID ) const {
                std::vector<rr::indexEntry> entries;
                getEntries(ID, entries);
                return indexEntryUtil::getUniqNum(entries);
            }
        
        // returns the same value as getNumWithID(ID)
        virtual uint32_t
            getEntries( uint32_t ID, std::vector<rr::indexEntry> &entries ) const;
        
        // getEntries for all unique query.id in order not to redo getEntries for equal ones
        // output: allEntries[ index[i] ]= getEntries( query.id(i) )
        // index.size==query.id_size, but allEntries.size <= query.id_size
        // assumes query.id is sorted
        virtual void
            getUniqEntries( rr::indexEntry &queryRep,
                            uniqEntries &entries ) const;
        
        uint32_t
            getInverseEntryInds( uint32_t invID,
                                 std::vector<uint32_t> &ID,
                                 std::vector< std::pair<uint32_t,uint32_t> > &entryInd,
                                 std::vector<uint32_t> const *lookInIDs= NULL) const;
        
        uint32_t
            getInverseEntryInds( uint32_t invID,
                                 std::vector< std::pair<uint32_t,uint32_t> > &entryInd,
                                 std::vector<rr::indexEntry> const &entries ) const;
        
        void
            unquantEllipse(rr::indexEntry &entry) const;
        
    
    protected:
        
        uint32_t
            getInverseEntryInds( uint32_t invID,
                                 std::vector<uint32_t> &ID,
                                 std::vector< std::pair<uint32_t,uint32_t> > &entryInd,
                                 uint32_t lookID ) const;
        
        protoDb const *db_;
        uint32_t const numIDs_;
        bool precompQuantEl_;
        float *aQuant_, *bQuant_, *cQuant_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoIndex)
};



class indexBuilder {
    
    public:
        
        indexBuilder(protoDbBuilder &dbBuilder, bool doDiff= true, bool quantXY= true, bool quantEl= true) : dbBuilder_(&dbBuilder), doDiff_(doDiff), quantXY_(quantXY), quantEl_(quantEl) {}
        
        virtual ~indexBuilder() { close(); }
        
        inline void
            close() { dbBuilder_->close(); }
        
        inline bool
            hasBeenClosed() const { return dbBuilder_->hasBeenClosed(); }
        
        void
            addEntry( uint32_t ID, rr::indexEntry &entry );
    
    private:
        protoDbBuilder *dbBuilder_;
        bool doDiff_, quantXY_, quantEl_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(indexBuilder)
};



// NOTE careful:
// 1) because protobuf's are loaded into RAM, it means that all the compression is gone (e.g. small uint32 values are not stored in 1 byte but 4).
// 2) don't use it with protoDbInRam as this is a waste of RAM, protoDb doesn't have to be in RAM as it is only read once, in the constructor
class protoIndexInRam : public protoIndex {
    
    public:
        
        // don't give protoDbInRam to waste RAM
        protoIndexInRam(protoDb const &db, bool precompQuantEl= true, bool verbose= true)
                : protoIndex(db, precompQuantEl),
                  numIDs_(numIDs()) {
            
            if (verbose)
                std::cout<<"protoIndexInRam::protoIndexInRam: loading idx\n";
            double const time= timing::tic();
            uint32_t const IDs_printStep= std::max(static_cast<uint32_t>(1),numIDs_/10);
            
            entriess_.resize(numIDs_);
            N_.resize(numIDs_);
            
            for (uint32_t ID= 0; ID<numIDs_; ++ID){
                if (verbose && ID % IDs_printStep == 0)
                    std::cout<<"protoIndexInRam::protoIndexInRam: loading ID= "<<ID<<" / "<<numIDs_<<" "<<timing::toc(time)<<" ms\n";
                N_[ID]= protoIndex::getEntries(ID, entriess_[ID]);
            }
            
            if (verbose)
                std::cout<<"protoIndexInRam::protoIndexInRam: loading idx - DONE ("<<timing::toc(time)<<" ms)\n";
        }
        
        inline uint32_t
            getEntries( uint32_t ID, std::vector<rr::indexEntry> &entries ) const {
                if (ID > numIDs_){
                    entries.clear();
                    return 0;
                }
                entries= entriess_[ID];
                return N_[ID];
            }
        
    private:
        uint32_t const numIDs_;
        std::vector< std::vector<rr::indexEntry> > entriess_;
        std::vector<uint32_t> N_;
        
        DISALLOW_COPY_AND_ASSIGN(protoIndexInRam)
};


#endif
