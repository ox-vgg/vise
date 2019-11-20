//
// Compute visual vocabulary from SIFT descriptors extracted from
// a set of images (based on Bag of Visual Words concept described in:
//
// Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
// Date:   16 July 2019
//
#ifndef _TRAIN_CLUSTER_H_
#define _TRAIN_CLUSTER_H_

#include <iostream>
#include <string>
#include <cstdio>

#include <stdint.h>
#include "util.h"

namespace buildIndex {
  void compute_train_cluster(std::string const train_desc_fn,
                             bool const use_root_sift,
                             std::string const cluster_fn,
                             uint32_t const bow_cluster_count,
                             uint32_t const cluster_num_iteration);

}

#endif
