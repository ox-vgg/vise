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

#ifndef _DATASET_ABS_H_
#define _DATASET_ABS_H_

#include <stdint.h>
#include <string>

#include "macros.h"



class datasetAbs {
    
    public:
        datasetAbs(){}
        virtual ~datasetAbs(){}
        
        virtual uint32_t
            getNumDoc() const =0;
        
        virtual std::string
            getFn( uint32_t docID ) const =0;
        
        virtual std::pair<uint32_t, uint32_t>
            getWidthHeight( uint32_t docID ) const =0;
        
        virtual uint32_t
            getDocID( std::string fn ) const =0;
        
        virtual uint32_t
            getDocIDFromAbsFn( std::string fn ) const =0;
        
        virtual bool
            containsFn( std::string fn ) const =0;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(datasetAbs)
    
};

#endif
