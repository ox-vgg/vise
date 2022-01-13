//
// Compute visual vocabulary from SIFT descriptors extracted from
// a set of images (based on Bag of Visual Words concept described in:
//
// Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
// Date:   16 July 2019
//

// Updates:
// 16-Jul-2019 : initial version based on kmeans and vlfeat::kd_tree
// 28-May-2021 : fixed bug with openmp code, improved computation speed using openmp parallel for
#include "train_cluster.h"

#include <vl/generic.h>
#include <vl/kdtree.h>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <omp.h>
#include <sstream>
#include "vise/vise_util.h"
#include "timing.h"

namespace buildIndex {
  void compute_train_cluster(std::string const train_desc_fn,
                             bool const use_root_sift,
                             std::string const cluster_fn,
                             uint32_t bow_cluster_count,
                             uint32_t cluster_num_iteration,
                             std::ofstream &logf,
                             const unsigned int nthread,
                             vise::task_progress *progress) {
    vl_set_num_threads(nthread);
    omp_set_num_threads(nthread);

    logf << "cluster:: using " << nthread << " threads" << std::endl;

    FILE *f = fopen(train_desc_fn.c_str(), "rb");
    if ( f == NULL ) {
      logf << "cluster:: failed to open training descriptors file: "
           << train_desc_fn << std::endl;
      return;
    }

    // file structure
    // SIFT-dimension (uint32_t, 4 bytes)
    // data-type-code (uint8_t , 1 byte )
    // continuous-stream-of-data ...
    // size_t fwrite( const void *buffer, size_t size, size_t count, FILE *stream );
    // size_t fread( void *buffer, size_t size, size_t count, FILE *stream );
    const std::size_t HEADER_BYTES = 4 + 1;
    const std::vector<std::string> DATA_TYPE_STR = {"uint8", "uint16", "uint32", "uint64", "float32", "float64"};
    const std::size_t DATA_TYPE_SIZE[] = {1, 2, 4, 8, 4, 8};

    uint32_t descriptor_dimension;
    uint8_t data_type_code;

    std::size_t read_count;
    read_count = fread(&descriptor_dimension, sizeof(descriptor_dimension), 1, f);
    if ( read_count != 1 ) {
      std::cerr << "Error reading value of descriptor_dimension! "
                << "stored in train descs file: " << train_desc_fn
                << std::endl;
      fclose(f);
      return;
    }

    read_count = fread(&data_type_code, sizeof(data_type_code), 1, f);
    if ( read_count != 1 ) {
      logf << "cluster:: error reading value of data_type_code stored in train descs file: " << train_desc_fn << std::endl;
      fclose(f);
      return;
    }
    logf << "cluster:: data_type_code=" << DATA_TYPE_STR.at(data_type_code) << std::endl;

    uint32_t element_size = DATA_TYPE_SIZE[data_type_code];
    std::size_t file_size;
#ifdef _WIN32
    _fseeki64(f, 0, SEEK_END);
    file_size = _ftelli64(f);
#elif __APPLE__
    fseeko(f, 0, SEEK_END);
    file_size = ftello(f);
#else
    fseeko64(f, 0, SEEK_END);
    file_size = ftello64(f);
#endif
    std::size_t descriptor_data_length = (file_size - HEADER_BYTES) / (element_size);
    uint32_t descriptor_count = descriptor_data_length / descriptor_dimension;

    logf << "cluster:: descriptor_dimension = " << descriptor_dimension << std::endl;
    logf << "cluster:: data_type_code = " << (int) data_type_code << std::endl;
    logf << "cluster:: file_size = " << file_size << std::endl;
    logf << "cluster:: element_size = " << element_size << std::endl;
    logf << "cluster:: descriptor_data_length = " << descriptor_data_length << std::endl;
    logf << "cluster:: descriptor_count = " << descriptor_count << std::endl;
    logf << "cluster:: bow_cluster_count = " << bow_cluster_count << std::endl;
    logf << "cluster:: cluster_num_iteration = " << cluster_num_iteration << std::endl;

    std::vector<float> cluster_centers( bow_cluster_count * descriptor_dimension );
    std::vector<uint32_t> cluster_descriptor_count(bow_cluster_count);
    std::vector<uint8_t> descriptors_sift( descriptor_data_length );
    std::vector<float> descriptors_rootsift( descriptor_data_length );

    // read descriptors
#ifdef _WIN32
    read_count = _fseeki64(f, HEADER_BYTES, SEEK_SET); // move file pointer to start of descriptors
#elif __APPLE__
    read_count = fseeko(f, HEADER_BYTES, SEEK_SET); // move file pointer to start of descriptors
#else
    read_count = fseeko64(f, HEADER_BYTES, SEEK_SET); // move file pointer to start of descriptors
#endif
    if(read_count) {
      logf << "cluster:: fseek() failed for file " << train_desc_fn << std::endl;
      fclose(f);
      return;
    }
    logf << "cluster:: reading SIFT descriptors ... ";
    read_count = fread(descriptors_sift.data(), element_size, descriptor_data_length, f);
    logf << "cluster:: " << read_count << " elements read" << std::endl;
    fclose(f);

    // convert SIFT descriptors to RootSIFT
    logf << "cluster:: converting descriptors to RootSIFT ... " << std::endl;
    convert_sift_to_rootsift(descriptors_sift.data(),
                             descriptor_dimension,
                             descriptor_count,
                             descriptors_rootsift.data());
    descriptors_sift.clear(); // no longer needed as we operate on descriptors_rootsift

    // assign cluster centers to randomly choosen descripts
    logf << "cluster:: assigning initial clusters to random descriptors ... " << std::endl;
    std::vector<uint32_t> descriptors_index_list( descriptor_count );
#pragma omp parallel for
    // use index variable of type "long long int" instead of "std::size_t"
    // because Windows MSVC compiler required OpenMP index variables to be signed integer
    for ( long long int i = 0; i < descriptor_count; ++i ) {
      descriptors_index_list[i] = i;
    }

    std::uint32_t random_seed = 9971;
    std::srand(random_seed);
    std::random_shuffle( descriptors_index_list.begin(), descriptors_index_list.end() );

#pragma omp parallel for
    for ( long long int ci = 0; ci<bow_cluster_count; ++ci ) {
      float *descriptor = descriptors_rootsift.data() + descriptors_index_list[ci] * descriptor_dimension;
      float *cluster = cluster_centers.data() + ci * descriptor_dimension;
      for ( std::size_t j = 0; j < descriptor_dimension; ++j ) {
        cluster[j] = descriptor[j];
      }
    }

    // create kd tree of the descriptors
    // see vlfeat-0.9.21/doc/api/kdtree_8c.html#a52564e86ef0d9294a9bc9b13c5d44427
    std::size_t num_trees = 8;
    std::size_t max_num_checks = 512;
    std::cout.precision(7);

    logf << "cluster:: starting " << cluster_num_iteration << " iterations of kmeans ..."
         << std::endl;
    for ( uint32_t iter = 0; iter < cluster_num_iteration; ++iter ) {
      std::chrono::steady_clock::time_point iter_start = std::chrono::steady_clock::now();

      VlKDForest* kd_forest = vl_kdforest_new( VL_TYPE_FLOAT, descriptor_dimension, num_trees, VlDistanceL2 );
      vl_kdforest_set_max_num_comparisons(kd_forest, max_num_checks);

      //std::cout << "Building Kd forest from cluster centers using " << num_trees << " trees ..." << std::endl;
      vl_kdforest_build(kd_forest, bow_cluster_count, cluster_centers.data());

      // find nearest cluster assignment for each descriptor
      // see vlfeat-0.9.21/doc/api/kdtree_8c.html#a2b44b23d4ea1b3296a76c6a020831ac6
      //std::cout << "Querying Kd forest to obtain cluster assignment for each descriptor" << std::endl;
      std::vector<uint32_t> descriptor_cluster_assignment(descriptor_count);
      std::vector<float> descriptor_cluster_distance(descriptor_count);
      vl_kdforest_query_with_array(kd_forest,
                                   descriptor_cluster_assignment.data(),
                                   1, // number of nearest neighbour to be found for each data point
                                   descriptor_count,
                                   descriptor_cluster_distance.data(),
                                   descriptors_rootsift.data() );
      vl_kdforest_delete(kd_forest);
      float cluster_distance_sum = 0.0;
      for ( std::size_t i = 0; i < descriptor_count; ++i ) {
        cluster_distance_sum += descriptor_cluster_distance[i];
      }

      compute_centroids(descriptors_rootsift.data(),
                        descriptor_cluster_assignment,
                        descriptor_count,
                        descriptor_dimension,
                        bow_cluster_count,
                        cluster_centers.data(),
                        cluster_descriptor_count.data(),
                        logf);

      std::chrono::steady_clock::time_point iter_end = std::chrono::steady_clock::now();

      uint32_t elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(iter_end - iter_start).count();
      uint32_t remaining_sec = (cluster_num_iteration-iter-1) * elapsed_sec;
      std::ostringstream ss;
      ss << "cluster:: iteration " << iter << "/"
         << cluster_num_iteration << ", error="
         << std::setprecision(10) << cluster_distance_sum << ", elapsed="
         << timing::hrminsec(elapsed_sec) << ", "
         << "remaining=" << timing::hrminsec(remaining_sec);

      if(progress != nullptr) {
        progress->update(iter, ss.str());
      }
      logf << ss.str() << std::endl;

      std::string cluster_data_fn;
      if(iter == (cluster_num_iteration - 1)) {
        // last iteration
        //boost::filesystem::remove(boost::filesystem::path(cluster_cp_fn));
        cluster_data_fn = cluster_fn;
      } else {
        cluster_data_fn = cluster_fn + ".temp";
      }

      save_clusters(cluster_data_fn,
                    bow_cluster_count,
                    descriptor_dimension,
                    iter,
                    cluster_num_iteration,
                    descriptor_count,
                    cluster_distance_sum,
                    random_seed,
                    cluster_centers,
                    logf);
    }
    // delete temporary cluster file
    std::string tmp_cluster_fn = cluster_fn + ".temp";
    boost::filesystem::remove(boost::filesystem::path(tmp_cluster_fn));
    logf << "cluster:: deleted temp. clusters file " << tmp_cluster_fn << std::endl;
    logf << "cluster:: written clusters to " << cluster_fn << std::endl;
  }

  void save_clusters(const std::string cluster_data_fn,
                     const uint32_t bow_cluster_count,
                     const uint32_t descriptor_dimension,
                     const uint32_t cluster_num_iteration,
                     const uint32_t iter,
                     const uint32_t descriptor_count,
                     const float cluster_distance_sum,
                     const uint32_t random_seed,
                     const std::vector<float> &cluster_centers,
                     std::ofstream &logf) {
    // save clusters
    // file structure
    // data-type-code                           (uint8_t , 1 byte )
    // cluster-count, cluster-dim               (uint32_t, 8 bytes)
    // current-iteration, cluster_num_iteration (uint32_t, 8 bytes)
    // descriptor_count, descriptor_dimension   (uint32_t, 8 bytes)
    // seed                                     (uint32_t, 4 bytes)
    // distortion                               (float32, 4 bytes)
    // continuous-stream-of-data ...
    // size_t fwrite( const void *buffer, size_t size, size_t count, FILE *stream );
    // size_t fread( void *buffer, size_t size, size_t count, FILE *stream );
    FILE *cf = fopen(cluster_data_fn.c_str(), "wb");
    if ( cf == NULL ) {
      logf << "cluster:: failed to open cluster file for writing: "
           << cluster_data_fn << std::endl;
      logf << "cluster:: writing results to temp_clusters.bin file";
      cf = fopen("temp_clusters.bin", "wb");
    }

    uint8_t cluster_dtype_code = 4; // float32
    fwrite( &cluster_dtype_code, sizeof(cluster_dtype_code), 1, cf);

    // shape of clusters
    fwrite( &bow_cluster_count, sizeof(bow_cluster_count), 1, cf);
    fwrite( &descriptor_dimension, sizeof(descriptor_dimension), 1, cf);

    // k-means iteration specifications
    fwrite( &iter, sizeof(iter), 1, cf);
    fwrite( &cluster_num_iteration, sizeof(cluster_num_iteration), 1, cf);

    // shape of features
    fwrite( &descriptor_count, sizeof(descriptor_count), 1, cf);
    fwrite( &descriptor_dimension, sizeof(descriptor_dimension), 1, cf);

    fwrite( &random_seed, sizeof(random_seed), 1, cf);

    fwrite( &cluster_distance_sum, sizeof(cluster_distance_sum), 1, cf);

    std::size_t data_count = bow_cluster_count * descriptor_dimension;
    fwrite( cluster_centers.data(), sizeof(cluster_centers[0]), data_count, cf);
    fclose(cf);
  }

  // inspired from https://github.com/facebookresearch/faiss/blob/master/faiss/Clustering.cpp
  void compute_centroids(float *descriptors_rootsift,
                         const std::vector<uint32_t> &descriptor_cluster_assignment,
                         const uint32_t descriptor_count,
                         const uint32_t descriptor_dimension,
                         const uint32_t bow_cluster_count,
                         float *cluster_centers,
                         uint32_t *cluster_descriptor_count,
                         std::ofstream &logf) {
    memset(cluster_centers, 0, sizeof(*cluster_centers) * descriptor_dimension * bow_cluster_count);
    memset(cluster_descriptor_count, 0, sizeof(*cluster_descriptor_count) * bow_cluster_count);

#pragma omp parallel
    {
      int nt = omp_get_num_threads();
      int rank = omp_get_thread_num();

      // this thread is taking care of centroids c0:c1
      std::size_t c0 = (bow_cluster_count * rank) / nt;
      std::size_t c1 = (bow_cluster_count * (rank + 1)) / nt;

      for(std::size_t i=0; i<descriptor_count; ++i) {
        std::size_t ci = descriptor_cluster_assignment[i];
        if (ci >= c0 && ci < c1) {
          float *cluster = cluster_centers + ci * descriptor_dimension;
          float *descriptor = descriptors_rootsift + i * descriptor_dimension;
          cluster_descriptor_count[ci] += 1;
          for(std::size_t j=0; j<descriptor_dimension; ++j) {
            cluster[j] += descriptor[j];
          }
        }
      }
    } // end of #pragma omp parallel

#pragma omp parallel for
    for(long long int ci=0; ci<bow_cluster_count; ++ci) {
      if(cluster_descriptor_count[ci] == 0) {
        logf << "cluster:: encountered empty cluster, discarding them" << std::endl;
        continue; // discard empty clusters
      }
      float norm = 1.f / ((float) cluster_descriptor_count[ci]);
      float *cluster = cluster_centers + ci * descriptor_dimension;
      for(std::size_t j=0; j<descriptor_dimension; ++j) {
        cluster[j] *= norm;
      }
    }
  } // end of compute_centroids()

  void convert_sift_to_rootsift(uint8_t *descriptors_sift,
                                const uint32_t descriptor_dimension,
                                const uint32_t descriptor_count,
                                float *descriptors_rootsift) {
    std::vector<uint32_t> descriptors_sum(descriptor_count, 0);
#pragma omp parallel for
    for ( long long int i = 0; i < descriptor_count; ++i ) {
      uint8_t *descriptor = descriptors_sift + i * descriptor_dimension;
      for(std::size_t j=0; j<descriptor_dimension; ++j) {
        descriptors_sum[i] += descriptor[j];
      }
      if(descriptors_sum[i] < 1e-6) {
        descriptors_sum[i] = 1;
      }
    }

#pragma omp parallel for
    for ( long long int i = 0; i < descriptor_count; ++i ) {
      uint8_t *src = descriptors_sift + i * descriptor_dimension;
      float *dst = descriptors_rootsift + i * descriptor_dimension;
      float norm = 1.f / ((float) descriptors_sum[i]);
      for(std::size_t j=0; j<descriptor_dimension; ++j) {
        dst[j] = std::max(0.0f, sqrt( ((float) src[j]) * norm ));
      }
    }
  } // end of convert_sift_to_rootsift()

}; // end of namespace buildIndex
