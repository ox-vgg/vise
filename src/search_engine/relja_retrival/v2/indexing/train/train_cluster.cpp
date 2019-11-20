//
// Compute visual vocabulary from SIFT descriptors extracted from
// a set of images (based on Bag of Visual Words concept described in:
//
// Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
// Date:   16 July 2019
//

#include "train_cluster.h"

#include <vl/generic.h>
#include <vl/kdtree.h>

#include <algorithm>
#include <chrono>

namespace buildIndex {
  void compute_train_cluster(std::string const train_desc_fn,
                             bool const use_root_sift,
                             std::string const cluster_fn,
                             uint32_t const bow_cluster_count,
                             uint32_t const cluster_num_iteration) {
    std::cout << "buildIndex::compute_train_cluster()" << std::endl;
    std::cout << "train_desc_fn = " << train_desc_fn << std::endl;
    std::cout << "use_root_sift = " << use_root_sift << std::endl;
    std::cout << "cluster_fn = " << cluster_fn << std::endl;
    std::cout << "bow_cluster_count = " << bow_cluster_count << std::endl;

    FILE *f = fopen(train_desc_fn.c_str(), "rb");
    if ( f == NULL ) {
      std::cerr << "Failed to open training descriptors file: "
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
      return;
    }

    read_count = fread(&data_type_code, sizeof(data_type_code), 1, f);
    std::cout << "read_count=" << read_count << std::endl;
    if ( read_count != 1 ) {
      std::cerr << "Error reading value of data_type_code stored in train descs file: " << train_desc_fn << std::endl;
      return;
    }

    uint32_t element_size = DATA_TYPE_SIZE[data_type_code];

    fseek(f, 0, SEEK_END);
    uint32_t file_size = ftell(f);

    uint32_t descriptor_data_length = (file_size - HEADER_BYTES) / (element_size);
    uint32_t descriptor_count = descriptor_data_length / descriptor_dimension;

    std::cout << "descriptor_dimension = " << descriptor_dimension << std::endl;
    std::cout << "data_type_code = " << (int) data_type_code << std::endl;
    std::cout << "file_size = " << file_size << std::endl;
    std::cout << "element_size = " << element_size << std::endl;
    std::cout << "descriptor_data_length = " << descriptor_data_length << std::endl;
    std::cout << "descriptor_count = " << descriptor_count << std::endl;

    std::vector<float> cluster_centers( bow_cluster_count * descriptor_dimension );
    std::vector<uint8_t> descriptors_sift( descriptor_count * descriptor_dimension );
    std::vector<float> descriptors_rootsift( descriptor_count * descriptor_dimension );

    // read descriptors
    std::cout << "Reading SIFT descriptors ... ";
    fseek(f, HEADER_BYTES, SEEK_SET); // move file pointer to start of descriptors
    read_count = fread(descriptors_sift.data(), element_size, descriptor_data_length, f);
    std::cout << read_count << " elements read" << std::endl;
    fclose(f);

    // convert SIFT descriptors to RootSIFT
    std::cout << "Converting descriptors to RootSIFT ... " << std::endl;
    uint32_t descriptor_index = 0;
    std::vector<uint32_t> descriptors_sum( descriptor_count, 0 );
    for ( std::size_t i = 0; i < descriptor_data_length; i++ ) {
      descriptor_index = (uint32_t) (i / descriptor_dimension);
      descriptors_sum[descriptor_index] += descriptors_sift[i];
    }

    descriptor_index = 0;
    for ( std::size_t i = 0; i < descriptor_data_length; i++ ) {
      descriptor_index = (uint32_t) (i / descriptor_dimension);
      descriptors_rootsift[i] = sqrt( ((float) descriptors_sift[i]) / ((float) descriptors_sum[descriptor_index]) );
    }

    // assign cluster centers to randomly choosen descripts
    std::cout << "Assigning initial clusters to random descriptors ... " << std::endl;
    std::vector<uint32_t> descriptors_index_list( descriptor_count );
    for ( std::size_t i = 0; i < descriptor_count; ++i ) {
      descriptors_index_list[i] = i;
    }

    std::cout << "Starting " << cluster_num_iteration << " iterations of kmeans ..."
              << std::endl;
    std::uint32_t random_seed = 9971;
    std::srand(random_seed);
    std::random_shuffle( descriptors_index_list.begin(), descriptors_index_list.end() );
    for ( std::size_t i = 0; i < bow_cluster_count; ++i ) {
      // cluster_centers[i*descriptor_dimension : i*descriptor_dimension + descriptor_dimension]
      // di = descriptors_index_list[i]
      // descriptors[di*descriptor_dimension : di*descriptor_dimension + descriptor_dimension]
      std::size_t descriptors_offset = descriptors_index_list[i] * descriptor_dimension;
      std::size_t cluster_centers_offset = i * descriptor_dimension;
      for ( std::size_t j = 0; j < descriptor_dimension; ++j ) {
        cluster_centers[ cluster_centers_offset + j ] = descriptors_rootsift[descriptors_offset + j];
      }
    }

    // create kd tree of the descriptors
    // see vlfeat-0.9.21/doc/api/kdtree_8c.html#a52564e86ef0d9294a9bc9b13c5d44427
    std::size_t num_trees = 8;
    std::size_t max_num_checks = 512;
    float cluster_distance_sum = 0.0;
    std::cout.precision(7);

    for ( std::size_t iter = 0; iter < cluster_num_iteration; ++iter ) {
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

      std::vector<uint32_t> cluster_descriptor_count(bow_cluster_count, 0);
      cluster_distance_sum = 0.0;
      for ( std::size_t i = 0; i < descriptor_count; ++i ) {
        cluster_descriptor_count[ descriptor_cluster_assignment[i] ] += 1;
        cluster_distance_sum += descriptor_cluster_distance[i];
      }

      // ensure that no cluster is empty
      for ( std::size_t i = 0; i < bow_cluster_count; ++i ) {
        if ( cluster_descriptor_count[i] == 0 ) {
          std::cout << "Empty cluster found!" << std::endl;
          break;
        }
      }

      //std::cout << "Recomputing cluster centers ..." << std::endl;
      cluster_centers = std::vector<float>( bow_cluster_count * descriptor_dimension, 0.0 );
      uint32_t  cluster_id, descriptor_id, offset;
      for ( std::size_t i = 0; i < descriptor_data_length; ++i ) {
        descriptor_id = (uint32_t) ( i / descriptor_dimension );
        offset = i - (descriptor_id * descriptor_dimension);
        cluster_id = descriptor_cluster_assignment[descriptor_id];
        cluster_centers[cluster_id * descriptor_dimension + offset] += descriptors_rootsift[i];
      }

      for ( std::size_t i = 0; i < bow_cluster_count; ++i ) {
        for ( std::size_t j = 0; j < descriptor_dimension; ++j ) {
          cluster_centers[i*descriptor_dimension + j] = cluster_centers[i*descriptor_dimension + j] / cluster_descriptor_count[i];
        }
      }

      std::chrono::steady_clock::time_point iter_end = std::chrono::steady_clock::now();
      std::cout << "Iteration " << iter << " of "
                << cluster_num_iteration << " : "
                << "Sum of distance to cluster=" << cluster_distance_sum << ", "
                << "completed in "
                << std::chrono::duration_cast<std::chrono::milliseconds>(iter_end - iter_start).count()
                << " ms"
                << std::endl;

      // cleanup of kd-tree
      vl_kdforest_delete(kd_forest);
    }

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

    FILE *cf = fopen(cluster_fn.c_str(), "wb");
    if ( cf == NULL ) {
      std::cerr << "Failed to open cluster file for writing: "
                << cluster_fn << std::endl;
      std::cerr << "Writing results to temp_clusters.bin file";
      cf = fopen("temp_clusters.bin", "wb");
    }

    uint8_t cluster_dtype_code = 4; // float32
    fwrite( &cluster_dtype_code, sizeof(cluster_dtype_code), 1, cf);

    fwrite( &bow_cluster_count, sizeof(bow_cluster_count), 1, cf);
    fwrite( &descriptor_dimension, sizeof(descriptor_dimension), 1, cf);

    fwrite( &cluster_num_iteration, sizeof(cluster_num_iteration), 1, cf);
    fwrite( &cluster_num_iteration, sizeof(cluster_num_iteration), 1, cf);

    fwrite( &descriptor_count, sizeof(descriptor_count), 1, cf);
    fwrite( &descriptor_dimension, sizeof(descriptor_dimension), 1, cf);

    fwrite( &random_seed, sizeof(random_seed), 1, cf);

    fwrite( &cluster_distance_sum, sizeof(cluster_distance_sum), 1, cf);

    fwrite( cluster_centers.data(), sizeof(cluster_centers[0]), bow_cluster_count * descriptor_dimension, cf);
    fclose(cf);
  }
}; // end of namespace buildIndex
