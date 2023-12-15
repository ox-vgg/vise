/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

*/

#ifndef _TRAIN_DESCS_H_
#define _TRAIN_DESCS_H_

#include <stdint.h>
#include <string>
#include <omp.h>
#include <fstream>

#include "feat_getter.h"
#include "vise/task_progress.h"

namespace buildIndex {

    void
        computeTrainDescs(std::string const trainImagelistFn, std::string const trainDatabasePath,
                          std::string const trainDescsFn,
                          int64_t const trainNumDescs,
                          featGetter const &featGetter_obj,
                          std::ofstream& logf,
                          const unsigned int nthread,
                          vise::task_progress *progress = nullptr);
}

#endif
