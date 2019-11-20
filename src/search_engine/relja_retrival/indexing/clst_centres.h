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

#ifndef _CLST_CENTRES_H_
#define _CLST_CENTRES_H_

#include <stdint.h>


class clstCentres {
    
    public:
        
        clstCentres( const char fileName[], bool flat= false );
        
        ~clstCentres();
        
        uint32_t numClst, numDims;
        float **clstC;
        float *clstC_flat;
    
};

#endif
