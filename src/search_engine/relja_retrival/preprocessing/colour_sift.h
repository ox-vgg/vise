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

#ifndef _COLOUR_SIFT_H_
#define _COLOUR_SIFT_H_

#include <stdexcept>

#include <boost/algorithm/string/predicate.hpp>

#include "feat_getter.h"
#include "ellipse.h"



// (Colour)SIFT by van de Sande: harrislaplace + descriptor
// NOTE: don't use when memory consumption is large - all those should be compiled as libraries into the code, as there is a massive overhead for calling system
class vanDeSande : public featGetter {
    
    public:
        
        // defaults are harrisK==0.06, laplaceThreshold==0.03, but those give too many descriptors
        vanDeSande(std::string descriptor, double harrisK= 0.1, double laplaceThreshold= 0.04);
        
        void
            getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const;
        
        uint32_t
            numDims() const { return numDims_; }
        
        std::string
            getRawDescs(float const *descs, uint32_t numFeats) const;
        
        inline uint8_t getDtypeCode() const { return 0; /* uint8 */ }
        
    private:
        uint32_t numDims_;
        std::string const descriptor_;
        double const harrisK_, laplaceThreshold_;
};

#endif
