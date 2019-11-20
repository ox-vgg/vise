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

#ifndef _MEDIAN_COMPUTER_H_
#define _MEDIAN_COMPUTER_H_

#include <stdint.h>
#include <vector>

#include "macros.h"

class medianComputer {
    
    public:
        medianComputer();
        
        void
            add(float value);
        
        float
            getMedian();
    
    private:
        
        void
            addQuantized(float value);
        
        std::vector<float> values_;
        std::vector<uint32_t> hist_;
        uint32_t totalNum_;
        float min_, max_;
};

#endif
