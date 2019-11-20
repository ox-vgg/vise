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

#ifndef _FLAT_DESC_FILE_H_
#define _FLAT_DESC_FILE_H_

#include <stdint.h>
#include <stdio.h>
#include <string>

#include "macros.h"




namespace buildIndex {



class flatDescsFile {
    public:
        flatDescsFile(std::string const descsFn, bool const doHellinger);
        
        uint32_t
            numDescs() const { return numDescs_; }
        
        uint32_t
            numDims() const { return numDims_; }
        
        void
            getDescs(uint32_t start, uint32_t end, float *&descs) const;
        
    private:
        FILE *f_;
        int fd_;
        uint8_t dtypeCode_;
        uint32_t numDims_, numDescs_;
        bool const doHellinger_;
        
        DISALLOW_COPY_AND_ASSIGN(flatDescsFile);
};
};

#endif
