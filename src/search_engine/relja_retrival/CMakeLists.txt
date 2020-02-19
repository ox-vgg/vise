ADD_DEFINITIONS(-DRR_REGISTER)
ADD_DEFINITIONS(-DRR_MAGICK)
set( cREGISTER 1 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DEIGEN_NO_DEBUG -O2 -g3 -msse2 -mpopcnt")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories( api )
include_directories( compression )
include_directories( external )
include_directories( external/KMCode_relja/exec/detect_points )
include_directories( external/KMCode_relja/exec/compute_descriptors )
include_directories( preprocessing )
include_directories( indexing )
include_directories( matching )
include_directories( matching/det_ransac )
include_directories( matching/registration )
include_directories( represent )
include_directories( retrieval )
include_directories( util )
include_directories( v2/dataset )
include_directories( v2/embedding )
include_directories( v2/evaluation )
include_directories( v2/indexing )
include_directories( v2/indexing/train )
include_directories( v2/retrieval )

# for protobufs
# protobuf compiles *.pb.cc and *.pb.h headers in the cmake build tree
# therefore, we need to include the corresponding folders in the build tree
# should point to a folder inside "cmake_build/"
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/util )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/dataset )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/embedding )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/evaluation )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/indexing )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/retrieval )

add_subdirectory( api )
add_subdirectory( compression )
add_subdirectory( external )
add_subdirectory( indexing )
add_subdirectory( matching )
add_subdirectory( preprocessing )
add_subdirectory( represent )
add_subdirectory( retrieval )
add_subdirectory( util )
add_subdirectory( v2 )

add_library(relja_retrival relja_retrival.cc)
target_link_libraries(relja_retrival
  search_engine
  build_index
  dataset_v2
  embedder
  feat_standard
  hamming_embedder
  mpi_queue
  train_assign
  train_descs
  train_hamming
  train_cluster
  dataset_v2
  proto_db
  proto_db_file
  proto_index
  slow_construction
  feat_standard
  clst_centres
  hamming
  hamming_embedder
  spatial_api
  spatial_verif_v2
  tfidf_v2
  mq_filter_outliers
  ${Boost_LIBRARIES}
  ${ImageMagick_LIBRARIES}
)
