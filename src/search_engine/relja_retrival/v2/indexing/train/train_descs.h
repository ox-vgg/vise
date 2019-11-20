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

#ifndef _TRAIN_DESCS_H_
#define _TRAIN_DESCS_H_

#include <stdint.h>
#include <string>
#include <omp.h>

#include "feat_getter.h"

namespace buildIndex {

    void
        computeTrainDescs(std::string const trainImagelistFn, std::string const trainDatabasePath,
                          std::string const trainDescsFn,
                          int32_t const trainNumDescs,
                          featGetter const &featGetter_obj);
}

#endif
