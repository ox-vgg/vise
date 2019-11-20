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

#ifndef _COMPRESSOR_H_
#define _COMPRESSOR_H_


#include <stdint.h>

#include "char_streams.h"
#include "macros.h"



class compressor {
    
    public:
        
        compressor(){}
        
        virtual uint32_t
            compress( float const vecs[], uint32_t const n, std::string &data ) const =0;
        
        virtual void
            decompress( std::string const &data, float *&vec ) const =0;
        
        virtual
            ~compressor(){}
        
        virtual uint32_t
            numDims() const =0;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(compressor);
    
};



class compressorIndep : public compressor {
    
    public:
        
        compressorIndep(){}
        
        virtual
            ~compressorIndep(){}
        
        virtual charStream*
            charStreamFactoryCreate() const =0;
        
        // number of bytes needed to encode a single vector, however n vectors don't necessarily need n* the amount of bytes, as a small constant size header is possible (e.g. for all charStreams apart from charStream8)
        virtual uint32_t
            numBytesPerVector() const =0;
        
        // if n vectors are encoded, then n* numCodePerVector "codes" are recoreded (e.g. having in mind product quantization, numCodePerVector= number of subquantizers)
        virtual uint32_t
            numCodePerVector() const =0;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(compressorIndep);
    
};



class compressorWithDistance : public compressor {
    
    public:
        
        compressorWithDistance(){}
        
        virtual
            ~compressorWithDistance(){}
        
        virtual uint32_t
            getDistsSq( float const vec[], std::string const &data, float *&distsSq  ) const =0;
        
        virtual uint32_t
            getDistsSq( float const vec[], std::string const &data, std::vector<float> &distsSq  ) const {
                float *distsSq_;
                uint32_t n= getDistsSq(vec, data, distsSq_);
                distsSq.assign( distsSq_, distsSq_+n );
                delete []distsSq_;
                return n;
            }
    
    private:
        DISALLOW_COPY_AND_ASSIGN(compressorWithDistance);
    
};

#endif
