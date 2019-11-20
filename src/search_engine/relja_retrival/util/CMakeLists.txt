#add_subdirectory(tests)

PROTOBUF_GENERATE_CPP(common_data.pb.cpp common_data.pb.h common_data.proto)
add_library( common_data.pb ${common_data.pb.cpp} )
target_link_libraries( common_data.pb ${PROTOBUF_LIBRARIES} )

add_library( mpi_queue mpi_queue.cpp )
target_link_libraries( mpi_queue ${Boost_LIBRARIES} ${MPI_LIBRARIES} )

add_library( thread_queue thread_queue.cpp )
target_link_libraries( thread_queue mpi_queue ${Boost_LIBRARIES} )

add_library( median_computer median_computer.cpp )
target_link_libraries( median_computer )

add_library( par_queue par_queue.cpp )
target_link_libraries( par_queue mpi_queue thread_queue )

add_library( slow_construction slow_construction.cpp )
target_link_libraries( slow_construction ${Boost_LIBRARIES} )

add_library( image_util image_util.cpp )
target_link_libraries( image_util util ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} )

add_library( same_random same_random.cpp )
target_link_libraries( same_random ${Boost_LIBRARIES} )

add_library( protobuf_util protobuf_util.cpp )
target_link_libraries( protobuf_util ${PROTOBUF_LIBRARIES} )
