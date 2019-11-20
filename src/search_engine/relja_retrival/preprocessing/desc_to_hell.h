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

#ifndef _DESC_TO_HELL_H_
#define _DESC_TO_HELL_H_

#include <math.h>

#include "feat_getter.h"



class descToHell : public descGetter {
    
    public:
        
        descToHell( descGetter *aDescGetter, bool aDelChild ) : desc(aDescGetter), delChild(aDelChild)
            {}
        
        ~descToHell(){
            if (delChild)
                delete desc;
        }
        
        void
            getDescs( const char fileName[], std::vector<ellipse> &regions, uint32_t &numDescs, float *&descs ) const {
                desc->getDescs(fileName, regions, numDescs, descs);
                convertToHell( numDims(), numDescs, descs );
            }
        
        static void
            convertToHell( uint32_t numDims, float *desc ){
                uint32_t iDim;
                float l1norm= 0.0;
                for (iDim= 0; iDim<numDims; ++iDim)
                    l1norm+= desc[iDim];
                l1norm= (l1norm>1e-6)?l1norm:1;
                for (iDim= 0; iDim<numDims; ++iDim)
                    desc[iDim]= sqrt( std::max(desc[iDim] / l1norm, 0.0f) );
            }
        
        static void
            convertToHell( uint32_t numDims, uint32_t numDescs, float *descs ){
                for (uint32_t iD= 0; iD<numDescs; ++iD)
                    convertToHell(numDims, descs+iD*numDims);
            }
        
        uint32_t
            numDims() const {
                return desc->numDims();
            }
    
    private:
        
        descGetter *desc;
        bool delChild;
    
};

#endif
