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

#ifndef _HOLIDAYS_PUBLIC_H_
#define _HOLIDAYS_PUBLIC_H_

#include <stdexcept>

#include <boost/algorithm/string/predicate.hpp>

#include "feat_getter.h"
#include "util.h"



// Publicly released SIFT descriptors by Jegou with the Holidays dataset
class holidaysPublic : public featGetter {
    
    public:
        
        holidaysPublic(bool useRootSIFT= true, std::string descDir= "~/Relja/Databases/Holidays/siftgeo/") :
            descDir_( util::expandUser(descDir) ),
            useRootSIFT_(useRootSIFT) {}
        
        void
            getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const;
        
        inline uint32_t
            numDims() const { return 128; }
        
        std::string
            getRawDescs(float const *descs, uint32_t numFeats) const;
        
        inline uint8_t getDtypeCode() const { return 0; /* uint8 */ }
        
    private:
        std::string const descDir_;
        bool const useRootSIFT_;
};

#endif
