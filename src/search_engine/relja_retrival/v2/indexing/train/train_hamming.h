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

#ifndef _TRAIN_HAMMING_H_
#define _TRAIN_HAMMING_H_

#include <stdint.h>
#include <string>
#include <omp.h>

namespace buildIndex {

    void
        computeHamming(std::string const clstFn,
                       bool const RootSIFT,
                       std::string const trainDescsFn,
                       std::string const trainAssignsFn,
                       std::string const trainHammFn,
                       uint32_t const hammEmbBits);
}

#endif
