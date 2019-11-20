##
## VGG Image Search Engine (VISE)
## vise
##
## Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
## 12 Nov. 2019
##

cmake_minimum_required( VERSION 3.10 )

project(vise)

enable_testing()
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set( BUILD_SHARED_LIBS "ON" )

## Boost
set(Boost_DEBUG 0)
find_package(Boost 1.70.0 COMPONENTS filesystem system thread unit_test_framework REQUIRED)
link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

## Imagemagick
ADD_DEFINITIONS(-DMAGICKCORE_HDRI_ENABLE=0)
ADD_DEFINITIONS(-DMAGICKCORE_QUANTUM_DEPTH=8)
find_package( ImageMagick COMPONENTS Magick++ REQUIRED )
include_directories( ${ImageMagick_INCLUDE_DIRS} )

## OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

## Protocol Buffer
## required by "search_engine/relja_retrival"
include(FindProtobuf)
find_package( Protobuf REQUIRED )
include_directories( ${PROTOBUF_INCLUDE_DIR} )

## Eigen
## requires EIGEN_DIR to be set to the location where Eigen3 is built using cmake
find_package (Eigen3 3.3 REQUIRED NO_MODULE)
include_directories( ${EIGEN3_INCLUDE_DIR} )

## vlfeat
## -DVLFEAT_LIB=/data/mybin/vlfeat/vlfeat-0.9.20/bin/glnxa64/libvl.so -DVLFEAT_INCLUDE_DIR=/data/mybin/vlfeat/vlfeat-0.9.20/vl
# @todo: remove dependency on absolute path
include_directories( ${VLFEAT_INCLUDE_DIR})

message("Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
message("Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
message("ImageMagick_LIBRARIES: " ${ImageMagick_LIBRARIES})
message("ImageMagick_INCLUDE_DIRS: " ${ImageMagick_INCLUDE_DIRS})
message("PROTOBUF_INCLUDE_DIR: " ${PROTOBUF_INCLUDE_DIR} )
message("Protobuf_LIBRARIES: " ${Protobuf_LIBRARIES} )
message("EIGEN3_INCLUDE_DIR: " ${EIGEN3_INCLUDE_DIR} )
message("VLFEAT_LIB: " ${VLFEAT_LIB} )
message("VLFEAT_INCLUDE_DIR: " ${VLFEAT_INCLUDE_DIR} )

set(CMAKE_INCLUDE_CURRENT_DIR ON) # i.e. include_directories(.)
include_directories( . )

## include for relja_retrival
## @todo: remove all the include_directories() that are not required by
include_directories( search_engine )
include_directories( search_engine/relja_retrival )
include_directories( search_engine/relja_retrival/api )
include_directories( search_engine/relja_retrival/compression )
include_directories( search_engine/relja_retrival/external )
include_directories( search_engine/relja_retrival/external/KMCode_relja/exec/detect_points )
include_directories( search_engine/relja_retrival/external/KMCode_relja/exec/compute_descriptors )
include_directories( search_engine/relja_retrival/preprocessing )
include_directories( search_engine/relja_retrival/indexing )
include_directories( search_engine/relja_retrival/matching )
include_directories( search_engine/relja_retrival/matching/det_ransac )
include_directories( search_engine/relja_retrival/matching/registration )
include_directories( search_engine/relja_retrival/represent )
include_directories( search_engine/relja_retrival/retrieval )
include_directories( search_engine/relja_retrival/util )
include_directories( search_engine/relja_retrival/v2/dataset )
include_directories( search_engine/relja_retrival/v2/embedding )
include_directories( search_engine/relja_retrival/v2/evaluation )
include_directories( search_engine/relja_retrival/v2/indexing )
include_directories( search_engine/relja_retrival/v2/indexing/train )
include_directories( search_engine/relja_retrival/v2/retrieval )

add_subdirectory( "search_engine" )
add_subdirectory( "vise" )
