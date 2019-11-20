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

#include "flat_desc_file.h"

#include "desc_to_hell.h"


namespace buildIndex {



flatDescsFile::flatDescsFile(std::string const descsFn, bool const doHellinger)
        : doHellinger_(doHellinger) {
    f_= fopen(descsFn.c_str(), "rb");
    ASSERT(f_!=NULL);
    uint64_t fileSize= util::fileSize(descsFn);
    
    int temp_;
    temp_= fread(&numDims_, sizeof(numDims_), 1, f_);
    temp_= fread(&dtypeCode_, sizeof(dtypeCode_), 1, f_);
    REMOVE_UNUSED_WARNING(temp_);
    
    ASSERT( dtypeCode_==0 || dtypeCode_==4 );
    
    uint8_t size= dtypeCode_==0 ? 1 : sizeof(float);
    ASSERT( (fileSize-5) % (numDims_*size) == 0 );
    numDescs_= static_cast<uint32_t>( (fileSize-5) / (numDims_*size) );
    
    fd_= fileno(f_);
}



void
flatDescsFile::getDescs(uint32_t start, uint32_t end, float *&descs) const {
    ASSERT(end>=start);
    descs= new float[(end-start)*numDims_];
    
    int temp_;
    
    if (dtypeCode_==0){
        
        uint8_t *descsRaw= new uint8_t[(end-start)*numDims_];
        temp_= pread64(fd_, descsRaw,
                       (end-start)*numDims_,
                       static_cast<uint64_t>(start)*numDims_ + 5 );
        
        uint8_t const *inIter= descsRaw;
        float *outIter= descs;
        float *outIterEnd= descs + (end-start)*numDims_;
        for (; outIter!=outIterEnd; ++inIter, ++outIter)
            *outIter= static_cast<float>(*inIter);
        
        delete []descsRaw;
        
    } else if (dtypeCode_==4) {
        
        temp_= pread64(fd_, descs,
                       (end-start)*numDims_*sizeof(float),
                       5 + start*numDims_*sizeof(float) );
        
    } else ASSERT(0);
    
    if (doHellinger_){
        float *thisDesc= descs;
        for (; start!=end; ++start, thisDesc+=numDims_)
            descToHell::convertToHell(numDims_, thisDesc);
    }
    
    REMOVE_UNUSED_WARNING(temp_);
}


};
