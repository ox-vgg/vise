/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

*/

#ifndef _BUILD_INDEX_H_
#define _BUILD_INDEX_H_

#include <string>
#include <omp.h>
#include <chrono>

#include "embedder.h"
#include "feat_getter.h"
#include "util.h"
#include "vise/task_progress.h"

namespace buildIndex {

    void
        build(std::string const imagelistFn, std::string const databasePath,
              std::string const dsetFn,
              std::string const iidxFn,
              std::string const fidxFn,
              std::string const tmpDir,
              featGetter const &featGetter_obj,
              std::string const clstFn,
              std::ofstream &logf,
              const unsigned int nthread,
              embedderFactory const *embFactory= NULL,
              vise::task_progress *progress = nullptr);
};

#endif
