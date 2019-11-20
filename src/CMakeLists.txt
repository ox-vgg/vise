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

## OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

message("Boost_LIBRARY_DIRS: " ${Boost_LIBRARY_DIRS})
message("Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})

add_subdirectory("vise" )