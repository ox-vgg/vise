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

#ifndef _HAMMING_EMBEDDER_H_
#define _HAMMING_EMBEDDER_H_

#include "char_streams.h"
#include "embedder.h"
#include "hamming_data.pb.h"
#include "macros.h"



class hammingEmbedder : public embedder {
    
    public:
        hammingEmbedder(std::vector<float> const &median,
                        std::vector<float> const &rot,
                        uint32_t k,
                        uint32_t numBits,
                        uint32_t numDims);
        
        ~hammingEmbedder() {
            if (charStream_!=NULL) delete charStream_;
        }
        
        void
            add(float vector[], uint32_t clusterID);
        
        void
            copyFrom(embedder &emb, uint32_t index);
        
        void
            copyRangeFrom(embedder &emb, uint32_t start, uint32_t end);
        
        void
            reserve(uint32_t n);
        
        void
            setDataCopy(std::string const &data);
        
        void
            clear();
        
        inline uint32_t
            getByteSize() const { return charStream_->getByteSize(); }
        
        inline std::string
            getEncoding() const {
                return charStream_->getDataCopy();
            }
        
        inline uint32_t
            getNum() const {
                return charStream_->getNum();
            }
        
        // hamming specific stuff
        
        inline uint32_t
            numBits() const {
                return numBits_;
            }
        
        inline void
            hammingDist(uint64_t val, int *itResult){
                charStream_->resetIter();
                uint32_t const n= charStream_->getNum();
                for (uint32_t i= 0; i<n; ++i, ++itResult)
                    *itResult= __builtin_popcountll(val ^ charStream_->getNextUnsafe());
            }
        
        inline charStream *
            getCharStream() const { return charStream_; }
    
    private:
        uint32_t k_, numBits_, numDims_;
        std::vector<float> const *median_, *rot_;
        charStream *charStream_;
        DISALLOW_COPY_AND_ASSIGN(hammingEmbedder)
    
};



class hammingEmbedderFactory : public embedderFactory {
    public:
        hammingEmbedderFactory(std::string const trainHammFn, uint32_t numBits);
        
        // protobuf is too small when voc size >200k, so
        // storing rotation and medians in raw format;
        // see train_hamming.cpp
        // this provides conversion from protobuf->new format
        static void
            convertFormats(std::string inFn, std::string outFn);
        
        hammingEmbedder*
            getEmbedder() const { return new hammingEmbedder(median_, rot_, hamm_.k(), hamm_.numbits(), hamm_.numdims()); }
        
        inline uint32_t
            numBits() const {
                return hamm_.numbits();
            }
        
    private:
        // const after loaded
        rr::hammingData hamm_;
        std::vector<float> median_, rot_;
        
        DISALLOW_COPY_AND_ASSIGN(hammingEmbedderFactory)
};


#endif
