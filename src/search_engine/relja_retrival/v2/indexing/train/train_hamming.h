/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

*/

#ifndef _TRAIN_HAMMING_H_
#define _TRAIN_HAMMING_H_

#include <stdint.h>
#include <string>
#include <omp.h>
#include "vise/task_progress.h"

namespace buildIndex {

    void
        computeHamming(std::string const clstFn,
                       bool const RootSIFT,
                       std::string const trainDescsFn,
                       std::string const trainAssignsFn,
                       std::string const trainHammFn,
                       uint32_t const hammEmbBits,
                       std::ofstream &logf,
                       const unsigned int nthread,
                       vise::task_progress *progress = nullptr);
}

#endif
