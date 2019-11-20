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

#ifndef _SAME_RANDOM_H_
#define _SAME_RANDOM_H_

#include <iostream>
#include <limits>
#include <stdint.h>
#include <vector>

#include "macros.h"



class sameRandomUint32;



// use the precomputed stream of numbers
// useful as std::random stuff is not thread safe
// also, generating random numbers is computationally expensive
class sameRandomStreamUint32 {
    
    public:
        
        sameRandomStreamUint32(sameRandomUint32 const &rand);
        
        inline uint32_t getNextUint32() {
            uint32_t res= *itNumber_;
            ++itNumber_;
            return res;
        }
        
        // [0,n)
        inline uint32_t getNext0ToN(uint32_t n) {
            uint32_t res= *itNumber_ % n;
            ++itNumber_;
            return res;
        }
        
        // [n,m)
        inline uint32_t getNextNtoM(uint32_t n, uint32_t m){
            return n + getNext0ToN(m-n);
        }
        
        inline float getNextFloat(){
            float res= static_cast<float>(*itNumber_) / std::numeric_limits<uint32_t>::max();
            ++itNumber_;
            return res;
        }
    
    private:
        uint32_t *itNumber_; // don't delete as it is owned by sameRandomUint32
    
    private:
        DISALLOW_COPY_AND_ASSIGN(sameRandomStreamUint32)
};



// pregenerate random numbers
class sameRandomUint32 {
    
    public:
        
        sameRandomUint32(uint32_t num, uint32_t seed= 42);
        
        ~sameRandomUint32() {
            delete []numbers_;
        }
        
        template <class T>
            void
                shuffle( typename std::vector<T>::iterator itBegin,
                         typename std::vector<T>::iterator itEnd ) const {
                    sameRandomStreamUint32 stream(*this);
                    
                    uint32_t n= itEnd-itBegin;
                    if (n > num_){
                        n= num_;
                        itEnd= itBegin + n;
                    }
                    uint32_t i= n-1;
                    for (--itEnd; itBegin!=itEnd; --itEnd, --i )
                        std::swap( *itEnd, *(itBegin + stream.getNext0ToN(i+1)) );
            }
        
        uint32_t const num_;
        uint32_t *numbers_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(sameRandomUint32)
};

#endif
