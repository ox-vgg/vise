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

#ifndef _DESC_FROM_FVECS_FILE_H_
#define _DESC_FROM_FVECS_FILE_H_

#include <stdio.h>
#include <stdint.h>

#include "desc_getter_from_file.h"
#include "util.h"
#include "macros.h"



class descFromFvecsFile : public descGetterFromFile {
    
    public:
        
        descFromFvecsFile( const char fn[] ){
            
            uint64_t fileSize= util::fileSize(fn);
            
            f= fopen(fn,"rb");
            f_d= fileno(f);
            
            int d;
            size_t temp_= fread(&d, sizeof(int), 1, f);
            if (false && temp_) {} // to avoid the warning about not checking temp_
            numDims_= static_cast<uint32_t>(d);
            ASSERT( fileSize % ((d+1)*4) == 0 );
            numDocs_= fileSize/((d+1)*4);
            
        }
        
        ~descFromFvecsFile(){
            fclose(f);
        }
        
        void
            getDescs( uint32_t docID, uint32_t &numDescs, float *&descs ) const {
                
                int d_= 0;
                
                numDescs= 1;
                descs= new float[numDims_];
                
                size_t temp_;
                temp_= pread64(f_d, &d_, sizeof(int), (numDims_+1)*docID*sizeof(float));
                ASSERT( static_cast<uint32_t>(d_)==numDims_ ); // check dimension is ok
                temp_= pread64(f_d, descs, numDims_*sizeof(float), ((numDims_+1)*docID+1)*sizeof(float));
                
                if (false && temp_) {} // to avoid the warning about not checking temp_
                
            }
        
        uint32_t
            numDims() const { return numDims_; }
        
        virtual uint32_t
            numDocs() const { return numDocs_; }
    
    private:
        
        FILE *f;
        int f_d;
        uint32_t numDims_, numDocs_;
    
};

#endif
