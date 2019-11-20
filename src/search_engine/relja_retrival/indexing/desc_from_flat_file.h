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

#ifndef _DESC_FROM_FLAT_FILE_H_
#define _DESC_FROM_FLAT_FILE_H_

#include <stdio.h>
#include <stdint.h>

#include "desc_getter_from_file.h"
#include "util.h"
#include "macros.h"



class descFromFlatFile : public descGetterFromFile {
    
    public:
        
        descFromFlatFile( const char fn[], uint32_t dim, uint32_t aNumDescPerDoc= 1, uint32_t aKeepFirstDim= 0, bool aDoNormalize= true ) : numDimsOrig_(dim), numDescPerDoc(aNumDescPerDoc), doNormalize(aDoNormalize) {
            
            if (aKeepFirstDim==0)
                numDims_= numDimsOrig_;
            else
                numDims_= aKeepFirstDim;
            ASSERT( numDims_ <= numDimsOrig_ );
            
            uint64_t fileSize= util::fileSize(fn);
            ASSERT( fileSize % (dim*sizeof(float)*numDescPerDoc) == 0 );
            numDocs_= fileSize/(dim*sizeof(float)*numDescPerDoc);
            
            f= fopen(fn,"rb");
            f_d= fileno(f);
            
        }
        
        ~descFromFlatFile(){
            fclose(f);
        }
        
        void
            getDescs( uint32_t docID, uint32_t &numDescs, float *&descs ) const {
                
                numDescs= numDescPerDoc;
                descs= new float[numDescs*numDims_];
                
                size_t temp_;
                uint64_t offset= static_cast<uint64_t>(numDimsOrig_)*docID*numDescPerDoc*sizeof(float);
                for (uint32_t iDesc= 0; iDesc<numDescs; ++iDesc){
                    temp_= pread64(f_d, descs+iDesc*numDims_, numDims_*sizeof(float), offset);
                    offset+= numDimsOrig_*sizeof(float);
                    if (doNormalize)
                        util::l2normalize(descs+iDesc*numDims_, numDims_);
                }
                
                if (false && temp_) {} // to avoid the warning about not checking temp_
                
            }
        
        uint32_t
            numDims() const { return numDims_; }
        
        virtual uint32_t
            numDocs() const { return numDocs_; }
    
    private:
        
        FILE *f;
        int f_d;
        uint32_t const numDimsOrig_, numDescPerDoc;
        bool doNormalize;
        uint32_t numDims_, numDocs_;
    
};

#endif
