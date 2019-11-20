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

#ifndef _DESC_FILE_ROTATE_H_
#define _DESC_FILE_ROTATE_H_

#include <stdexcept>
#include <stdio.h>
#include <stdint.h>
#include <string>

#include "desc_getter_from_file.h"
#include "util.h"



class descFileRotate : public descGetterFromFile {
    
    public:
        
        descFileRotate( descGetterFromFile const &aF, std::string rotateFn, bool aDoNormalize= true ) : f(&aF), doNormalize(aDoNormalize) {
            
            // assumes that rotation was saved row-wise and in double precision (numpy.tofile default)
            uint64_t fileSize= util::fileSize(rotateFn);
            if (fileSize % sizeof(double)!=0)
                throw std::runtime_error("descFileRotate:: Corrupt file!");
            
            uint64_t Rsq= fileSize/sizeof(double);
            numDims_= sqrt(Rsq);
            if (fileSize != sizeof(double)*numDims_*numDims_)
                throw std::runtime_error("descFileRotate:: Corrupt file!");
            
            FILE *fRot= fopen(rotateFn.c_str(), "rb");
            R= new double*[numDims_];
            size_t temp_= 0;
            for (uint32_t iDim= 0; iDim<numDims_; ++iDim){
                R[iDim]= new double[numDims_];
                temp_= fread( R[iDim], sizeof(double), numDims_, fRot );
            }
            if (false && temp_) {} // to avoid the warning about not checking temp_
            fclose(fRot);
            
        }
        
        void
            getDescs( uint32_t docID, uint32_t &numDescs, float *&descs ) const {
                
                // load from original
                float *descsOrig;
                f->getDescs(docID, numDescs, descsOrig);
                
                // apply rotation
                descs= new float[numDescs*numDims_];
                for (uint32_t iDesc= 0; iDesc<numDescs; ++iDesc){
                    
                    float invnorm= 1.0;
                    if (doNormalize){
                        // compute norm
                        invnorm= 0;
                        for (uint32_t iDim= 0; iDim<numDims_; ++iDim)
                            invnorm+= descsOrig[iDesc*numDims_+iDim] * descsOrig[iDesc*numDims_+iDim];
                        invnorm= 1.0/sqrt(invnorm);
                    }
                    
                    // matrix multiplication should be possible to do faster (e.g. using some library which does: blocking, sse etc), but most likely not so important
                    for (uint32_t iDim= 0; iDim<numDims_; ++iDim){
                        descs[iDesc*numDims_+iDim]= 0;
                        for (uint32_t jDim= 0; jDim<numDims_; ++jDim)
                            descs[iDesc*numDims_+iDim]+= descsOrig[iDesc*numDims_+jDim] * R[jDim][iDim];
                        descs[iDesc*numDims_+iDim]*= invnorm;
                    }
                }
                
                // cleanup
                delete []descsOrig;
            }
        
        uint32_t
            numDims() const { return numDims_; }
        
        uint32_t
            numDocs() const { return f->numDocs(); }
        
        virtual ~descFileRotate() {
            util::del( numDims_, R );
        }
    
    private:
        
        descGetterFromFile const *f;
        bool const doNormalize;
        uint32_t numDims_;
        double **R;
    
};

#endif
