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

#ifndef _DESC_GETTER_FROM_FILE_H_
#define _DESC_GETTER_FROM_FILE_H_

#include <stdint.h>

#include "util.h"



class descGetterFromFile {
    
    public:
        
        virtual void
            getDescs( uint32_t docID, uint32_t &numDescs, float *&descs ) const =0;
        
        virtual uint32_t
            numDims() const =0;
        
        virtual uint32_t
            numDocs() const =0;
        
        virtual ~descGetterFromFile()
            {}
    
};

#endif
