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

#ifndef _DESC_FROM_FILE_TO_HELL_H_
#define _DESC_FROM_FILE_TO_HELL_H_

#include <math.h>

#include "desc_getter_from_file.h"
#include "desc_to_hell.h"



class descFromFileToHell : public descGetterFromFile {
    
    public:
        
        descFromFileToHell( descGetterFromFile const &aDescGetter ) : desc_obj(&aDescGetter)
            {}
        
        void
            getDescs( uint32_t docID, uint32_t &numDescs, float *&descs ) const {
                desc_obj->getDescs(docID, numDescs, descs);
                descToHell::convertToHell( numDims(), numDescs, descs );
            }
        
        uint32_t
            numDims() const {
                return desc_obj->numDims();
            }
        
        uint32_t
            numDocs() const {
                return desc_obj->numDocs();
            }
    
    private:
        
        descGetterFromFile const *desc_obj;
        
    
};

#endif
