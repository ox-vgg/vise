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

#ifndef _TRAIN_ASSIGN_H_
#define _TRAIN_ASSIGN_H_

#include <string>
#include <omp.h>
#include <fstream>
#include "vise/task_progress.h"

namespace buildIndex {

  void
  computeTrainAssigns(std::string const clstFn,
                      bool const RootSIFT,
                      std::string const trainDescsFn,
                      std::string const trainAssignsFn,
                      std::ofstream& logf,
                      const unsigned int nthread,
                      vise::task_progress *progress = nullptr);
}

#endif
