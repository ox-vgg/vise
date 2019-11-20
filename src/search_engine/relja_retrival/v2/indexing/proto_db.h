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

#ifndef _PROTO_DB_H_
#define _PROTO_DB_H_


#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#include <boost/function.hpp>
#include <google/protobuf/stubs/common.h>

#include "macros.h"
#include "slow_construction.h"
#include "timing.h"



class protoDb {
    
    public:
        
        protoDb(){}
        
        virtual
            ~protoDb(){}
        
        virtual uint32_t
            numIDs() const =0;
        
        virtual void
            getData( uint32_t ID, std::vector<std::string> &data ) const =0;
        
        virtual bool
            contains( uint32_t ID ) const {
                std::vector<std::string> data;
                getData(ID, data);
                return data.size()>0;
            }
        
        // keeps ownership of the data
        virtual std::vector<std::string> const *
            getConstData( uint32_t ID ) const { ASSERT(0); return NULL; }
        
        virtual bool
            supportsGetConstData() const { return false; }
        
        // T should be a protocol buffer
        template <class T>
        void
            getProtos( uint32_t ID, std::vector<T> &protos ) const {
                bool isConstData= supportsGetConstData();
                std::vector<std::string> const *data= NULL;
                std::vector<std::string> *dataReal= NULL;
                
                if (isConstData){
                    data= getConstData(ID);
                }
                else {
                    dataReal= new std::vector<std::string>();
                    getData(ID, *dataReal);
                    data= dataReal;
                }
                
                protos.clear();
                protos.resize(data->size());
                for (uint32_t i= 0; i<data->size(); ++i)
                    GOOGLE_CHECK(protos[i].ParseFromString(data->at(i)));
                
                if (!isConstData)
                    delete dataReal;
            }
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoDb)
};



class protoDbBuilder {
    
    public:
        
        protoDbBuilder(){}
        
        virtual ~protoDbBuilder() {}
        
        virtual void
            close() =0;
        
        virtual bool
            hasBeenClosed() const =0;
        
        virtual void
            addData( uint32_t ID, std::string const &data ) =0;
        
        // T should be a protocol buffer
        template <class T>
        inline void
            addProto( uint32_t ID, T const &proto ){
                std::string data;
                proto.SerializeToString(&data);
                addData(ID, data);
            }
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoDbBuilder)
};



class protoDbInRam : public protoDb {
    
    public:
        
        protoDbInRam(protoDb const &db, bool verbose= true) : numIDs_(db.numIDs()), data_(numIDs_) {
            
            if (verbose)
                std::cout<<"protoDbInRam::protoDbInRam: loading db\n";
            double time= timing::tic();
            uint32_t IDs_printStep= std::max(static_cast<uint32_t>(1),numIDs_/10);
            
            for (uint32_t ID= 0; ID<numIDs_; ++ID){
                if (verbose && ID % IDs_printStep == 0)
                    std::cout<<"protoDbInRam::protoDbInRam: loading ID= "<<ID<<" / "<<numIDs_<<" "<<timing::toc(time)<<" ms\n";
                db.getData( ID, data_[ID] );
            }
            
            if (verbose)
                std::cout<<"protoDbInRam::protoDbInRam: loading db - DONE ("<<timing::toc(time)<<" ms)\n";
        }
        
        inline uint32_t
            numIDs() const { return numIDs_; }
        
        inline void
            getData( uint32_t ID, std::vector<std::string> &data ) const {
                if (ID > numIDs_)
                    data.clear();
                data= data_[ID];
            }
        
        std::vector<std::string> const *
            getConstData( uint32_t ID ) const { return &(data_[ID]); }
        
        virtual bool
            supportsGetConstData() const { return true; }
    
    private:
        
        uint32_t numIDs_;
        std::vector< std::vector<std::string> > data_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoDbInRam)
};



// See comments in iidx_in_ram for iidxInRamStartDisk
class protoDbInRamStartDisk : public protoDb {
    
    public:
        
        protoDbInRamStartDisk( protoDb const &db, boost::function<protoDb*()> protoDbInRamConstructor, bool deleteFirst= false, sequentialConstructions *consQueue= NULL ) : numIDs_(db.numIDs()), slowCons_(&db, protoDbInRamConstructor, deleteFirst, consQueue) {
        }
        
        inline uint32_t
            numIDs() const { return numIDs_; }
        
        inline void
            getData( uint32_t ID, std::vector<std::string> &data ) const {
                return slowCons_.getObject()->getData(ID, data);
            }
    
    private:
        
        uint32_t numIDs_;
        slowConstruction<protoDb const> slowCons_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoDbInRamStartDisk)
};



class protoDbs : public protoDb {
    
    public:
        
        protoDbs( std::vector<protoDbs const *> const &dbs ) : dbs_(&dbs) {
            numIDs_= 0;
            for (uint32_t iIdx= 0; iIdx < dbs_->size(); ++iIdx){
                offsets_.push_back(numIDs_);
                numIDs_+= dbs_->at(iIdx)->numIDs();
            }
        }
        
        virtual
            ~protoDbs(){};
        
        inline uint32_t
            numIDs() const { return numIDs_; }
        
        inline void
            getData( uint32_t ID, std::vector<std::string> &data ) const {
                uint32_t ind= whichDb(ID);
                return dbs_->at(ind)->getData(ID-offsets_.at(ind), data);
            }
    
    protected:
        
        inline uint32_t
            whichDb(uint32_t ID) const {
                // can do binary search, but not worth it for few protoDbs
                uint32_t ind= 0;
                for (; ind < offsets_.size()-1 && ID >= offsets_[ind+1]; ++ind);
                return ind;
            }
        
        std::vector<protoDbs const *> const *dbs_;
        uint32_t numIDs_;
        std::vector<uint32_t> offsets_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoDbs)
};



#endif
