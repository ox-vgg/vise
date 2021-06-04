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
#include "vise/task_progress.h"

namespace buildIndex {
  void compute_train_cluster(std::string const train_desc_fn,
                             bool const use_root_sift,
                             std::string const cluster_fn,
                             uint32_t const bow_cluster_count,
                             uint32_t const cluster_num_iteration,
                             std::ofstream &logf,
                             const unsigned int nthread,
                             vise::task_progress *progress = nullptr);

  void compute_centroids(float *descriptors_rootsift,
                         const std::vector<uint32_t> &descriptor_cluster_assignment,
                         const uint32_t descriptor_count,
                         const uint32_t descriptor_dimension,
                         const uint32_t bow_cluster_count,
                         float *cluster_centers,
                         uint32_t *cluster_descriptor_count,
                         std::ofstream &logf);

  void save_clusters(const std::string cluster_data_fn,
                     const uint32_t bow_cluster_count,
                     const uint32_t descriptor_dimension,
                     const uint32_t iter,
                     const uint32_t cluster_num_iteration,
                     const uint32_t descriptor_count,
                     const float cluster_distance_sum,
                     const uint32_t random_seed,
                     const std::vector<float> &cluster_centers,
                     std::ofstream &logf);

  void convert_sift_to_rootsift(uint8_t *descriptors_sift,
                                const uint32_t descriptor_dimension,
                                const uint32_t descriptor_count,
                                float *descriptors_rootsift);
}

#endif
