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

#include "same_random.h"

#include <limits>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>



sameRandomStreamUint32::sameRandomStreamUint32(sameRandomUint32 const &rand) : itNumber_(rand.numbers_) {}



sameRandomUint32::sameRandomUint32(uint32_t num, uint32_t seed) : num_(num) {
    
    numbers_= new uint32_t[num_];
    uint32_t *itNumber= numbers_;
    
    boost::mt19937 gen(seed); // for thread safety
    
    boost::uniform_int<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());
    
    for (uint32_t i= 0; i < num_; ++i, ++itNumber)
        *itNumber= dist(gen);
    
}
