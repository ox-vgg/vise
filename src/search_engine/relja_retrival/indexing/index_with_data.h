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

#ifndef _INDEX_WITH_DATA_H_
#define _INDEX_WITH_DATA_H_


#include <iostream>
#include <stdint.h>
#include <vector>
#include <string>

#include <boost/function.hpp>

#include "slow_construction.h"
#include "timing.h"



class indexWithData {
    
    public:
        
        virtual
            ~indexWithData(){}
        
        virtual uint32_t
            numIDs() const =0;
        
        virtual uint32_t
            getNumWithID( uint32_t ID ) const =0;
        
        // returns the same value as getNumWithID(ID)
        virtual uint32_t
            getData( uint32_t ID, std::vector<uint32_t> &vecIDs, unsigned char *&data, uint32_t &size ) const =0;
    
};



class indexWithDataBuilder {
    
    public:
        
        indexWithDataBuilder() : maxVecID(0) {}
        
        virtual ~indexWithDataBuilder(){}
        
        virtual void
            close() =0;
                
        virtual void
            addData( uint32_t ID, std::vector<uint32_t> const &vecIDs, unsigned char const *data, uint32_t size ) =0;
    
    protected:
        
        uint32_t maxVecID;
    
};



class indexWithDataInRam : public indexWithData {
    
    public:
        
        indexWithDataInRam(indexWithData const &idx) : numIDs_(idx.numIDs()), Ns(numIDs_,0), sizes(numIDs_,0), vecIDss(numIDs_), datas(numIDs_,NULL) {
            
            std::cout<<"indexWithDataInRam::indexWithDataInRam: loading index\n";
            double time= timing::tic();
            uint32_t IDs_printStep= std::max(static_cast<uint32_t>(1),numIDs_/10);
            
            for (uint32_t ID= 0; ID<numIDs_; ++ID){
                if (ID % IDs_printStep == 0)
                    std::cout<<"indexWithDataInRam::indexWithDataInRam: loading ID= "<<ID<<" / "<<numIDs_<<" "<<timing::toc(time)<<" ms\n";
                Ns[ID]= idx.getData( ID, vecIDss[ID], datas[ID], sizes[ID] );
            }
            
            std::cout<<"indexWithDataInRam::indexWithDataInRam: loading index - DONE ("<<timing::toc(time)<<" ms)\n";
        }
        
        ~indexWithDataInRam(){
            for (uint32_t ID= 0; ID<numIDs_; ++ID)
                delete []datas[ID];
        }
        
        uint32_t
            numIDs() const { return numIDs_; }
        
        uint32_t
            getNumWithID( uint32_t ID ) const { return ID>numIDs_ ? 0 : Ns[ID]; }
        
        uint32_t
            getData( uint32_t ID, std::vector<uint32_t> &vecIDs, unsigned char *&data, uint32_t &size ) const {
                if (ID>numIDs_) {
                    vecIDs.clear();
                    data= new unsigned char[0];
                    size= 0;
                    return 0;
                }
                size= sizes[ID];
                data= new unsigned char[size];
                std::memcpy(data, datas[ID], size);
                vecIDs= vecIDss[ID];
                return Ns[ID];
            }
    
    private:
        
        uint32_t numIDs_;
        std::vector<uint32_t> Ns, sizes;
        std::vector< std::vector<uint32_t> > vecIDss;
        std::vector<unsigned char *> datas;
    
};




// See comments in iidx_in_ram for iidxInRamStartDisk
class indexWithDataInRamStartDisk : public indexWithData {
    
    public:
        
        indexWithDataInRamStartDisk( indexWithData const &idx, boost::function<indexWithData*()> idxInRamConstructor, bool deleteFirst= false, sequentialConstructions *consQueue= NULL ) : slowCons_(&idx, idxInRamConstructor, deleteFirst, consQueue) {
        }
        
        inline uint32_t
            numIDs() const { return slowCons_.getObject()->numIDs(); }
        
        inline uint32_t
            getNumWithID( uint32_t ID ) const { return slowCons_.getObject()->getNumWithID(ID); }
        
        inline uint32_t
            getData( uint32_t ID, std::vector<uint32_t> &vecIDs, unsigned char *&data, uint32_t &size ) const {
                return slowCons_.getObject()->getData(ID, vecIDs, data, size);
            }
    
    private:
        
        slowConstruction<indexWithData const> slowCons_;
};



class indexesWithData : public indexWithData {
    
    public:
        
        indexesWithData( std::vector<indexWithData const *> const &idxs ) : idxs_(&idxs) {
            numIDs_= 0;
            for (uint32_t iIdx= 0; iIdx < idxs_->size(); ++iIdx){
                offsets_.push_back(numIDs_);
                numIDs_+= idxs_->at(iIdx)->numIDs();
            }
        }
        
        virtual
            ~indexesWithData(){};
        
        uint32_t
            numIDs() const { return numIDs_; }
        
        uint32_t
            getNumWithID( uint32_t ID ) const {
                uint32_t ind= whichIdx(ID);
                return idxs_->at(ind)->getNumWithID(ID-offsets_.at(ind));
            }
        
        uint32_t
            getData( uint32_t ID, std::vector<uint32_t> &vecIDs, unsigned char *&data, uint32_t &size ) const {
                uint32_t ind= whichIdx(ID);
                return idxs_->at(ind)->getData(ID-offsets_.at(ind), vecIDs, data, size);
            }
    
    protected:
        
        uint32_t
            whichIdx(uint32_t ID) const {
                // can do binary search, but not worth it for few indexes
                uint32_t ind= 0;
                for (; ind < offsets_.size()-1 && ID >= offsets_[ind+1]; ++ind);
                return ind;
            }
        
        std::vector<indexWithData const *> const *idxs_;
        uint32_t numIDs_;
        std::vector<uint32_t> offsets_;
    
};

#endif
