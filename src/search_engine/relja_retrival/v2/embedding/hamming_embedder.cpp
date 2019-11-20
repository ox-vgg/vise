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

#include "hamming_embedder.h"

#include <cstring>
#include <fstream>

#include "macros.h"



hammingEmbedder::hammingEmbedder(
        std::vector<float> const &median,
        std::vector<float> const &rot,
        uint32_t k,
        uint32_t numBits,
        uint32_t numDims)
        : k_(k), numBits_(numBits), numDims_(numDims), median_(&median), rot_(&rot) {
    charStream_= charStream::charStreamCreate(numBits_);
}



void
hammingEmbedder::add(float vector[], uint32_t clusterID){
    
    // TODO: if this is slow, do SSE or whatever (Eigen?) to speedup dot product
    float *proj= new float[numBits_];
    {
        float *projIt= proj;
        float const *rotIt= &((*rot_)[0]);
        float const *vecIt;
        float const *vecEnd= vector+numDims_;
        
        for (uint32_t iBit= 0; iBit<numBits_; ++iBit, ++projIt){
            // compute the projection
            *projIt= 0.0f;
            for (vecIt= vector; vecIt!=vecEnd; ++rotIt, ++vecIt)
                *projIt+= *rotIt * (*vecIt);
        }
    }
    
    // threshold based on median and compute the signature
    float const *median= &((*median_)[clusterID*numBits_]);
    float const *medianEnd= median + numBits_;
    float *projItC= proj;
    uint64_t value= (*projItC > *median);
    for (++projItC, ++median; median!=medianEnd; ++projItC, ++median){
        value <<= 1;
        value |= (*projItC > *median);
    }
    delete []proj;
    
    charStream_->add(value);
}



void
hammingEmbedder::copyFrom(embedder &emb, uint32_t index){
    hammingEmbedder* thisEmb= dynamic_cast<hammingEmbedder*>( &emb );
    thisEmb->charStream_->setIter(index);
    charStream_->add( thisEmb->charStream_->getNextUnsafe() );
}



void
hammingEmbedder::copyRangeFrom(embedder &emb, uint32_t start, uint32_t end){
    reserveAdditional(end-start);
    hammingEmbedder* thisEmb= dynamic_cast<hammingEmbedder*>( &emb );
    thisEmb->charStream_->setIter(start);
    for (; start<end; ++start)
        charStream_->add( thisEmb->charStream_->getNextUnsafe() );
}



void
hammingEmbedder::reserve(uint32_t n){
    charStream_->reserve(n);
}



void
hammingEmbedder::setDataCopy(std::string const &data) {
    charStream_->setDataCopy(data);
}



void
hammingEmbedder::clear(){
    charStream_->clear();
}



hammingEmbedderFactory::hammingEmbedderFactory(std::string const trainHammFn, uint32_t numBits) {
    ASSERT(numBits<=64);
    
    std::ifstream in(trainHammFn.c_str(), std::ios::binary);
    if (!in.is_open()){
        std::cout<<"hammingEmbedderFactory::hammingEmbedderFactory error opening hamming file: "<<trainHammFn<<"\n";
        ASSERT(in.is_open());
    }
    
    
    FILE *f= fopen(trainHammFn.c_str(), "rb");
    
    uint32_t temp_;
    uint32_t magicNumber;
    temp_= fread( &(magicNumber), sizeof(uint32_t), 1, f );
    
    if (magicNumber==0xF1234987){
        // new format
        uint32_t headerSize;
        temp_= fread( &(headerSize), sizeof(uint32_t), 1, f );
        
        std::string headerStr(headerSize, '\0');
        temp_= fread( &(headerStr[0]), sizeof(char), headerSize, f );
        ASSERT( hamm_.ParseFromString(headerStr) );
        
        rot_.resize(hamm_.numbits() * hamm_.numdims());
        temp_= fread( &rot_[0], sizeof(float), rot_.size(), f );
        
        median_.resize(hamm_.k() * hamm_.numbits());
        temp_= fread( &median_[0], sizeof(float), median_.size(), f );
        
    } else {
        // old format
        
        std::ifstream in(trainHammFn.c_str(), std::ios::binary);
        ASSERT(in.is_open());
        ASSERT(hamm_.ParseFromIstream(&in));
        in.close();
        
        // copy medians and rotation
        median_.resize(hamm_.median_size());
        std::memcpy( &median_[0], hamm_.median().data(), hamm_.median_size() * sizeof(float) );
        rot_.resize(hamm_.rotation_size());
        std::memcpy( &rot_[0], hamm_.rotation().data(), hamm_.rotation_size() * sizeof(float) );
    }
    
    REMOVE_UNUSED_WARNING(temp_);
    
    fclose(f);
    
    ASSERT(hamm_.numbits()==numBits);
    
}



void
hammingEmbedderFactory::convertFormats(std::string inFn, std::string outFn){
    
    // load old format
    rr::hammingData hamm;
    std::ifstream in(inFn.c_str(), std::ios::binary);
    if (!in.is_open()){
        std::cout<<"hammingEmbedderFactory::convertFormats error opening hamming file: "<<inFn<<"\n";
        ASSERT(in.is_open());
    }
    ASSERT(hamm.ParseFromIstream(&in));
    in.close();
    
    // copy medians and rotation
    std::vector<float> median, rot;
    median.resize(hamm.median_size());
    std::memcpy( &median[0], hamm.median().data(), hamm.median_size() * sizeof(float) );
    rot.resize(hamm.rotation_size());
    std::memcpy( &rot[0], hamm.rotation().data(), hamm.rotation_size() * sizeof(float) );
    
    // remove rotation and median from protobuf
    hamm.mutable_rotation()->Clear();
    hamm.mutable_median()->Clear();
    
    // save new format
    // magicNumber, sizeof(header), header, rotation, medians
    std::string headerStr;
    hamm.SerializeToString(&headerStr);
    uint32_t const magicNumber= 0xF1234987;
    uint32_t const headerSize= headerStr.length();
    
    FILE *f= fopen(outFn.c_str(), "wb");
    
    fwrite( &(magicNumber), sizeof(uint32_t), 1, f );
    fwrite( &(headerSize), sizeof(uint32_t), 1, f );
    fwrite( &(headerStr[0]), sizeof(char), headerSize, f );
    fwrite( &rot[0], sizeof(float), rot.size(), f );
    fwrite( &median[0], sizeof(float), median.size(), f );
    
    fclose(f);
}
