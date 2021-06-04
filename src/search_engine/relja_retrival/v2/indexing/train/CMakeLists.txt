add_library( flat_desc_file STATIC flat_desc_file.cpp )
target_link_libraries( flat_desc_file )


add_library( train_cluster STATIC train_cluster.cpp )
target_link_libraries( train_cluster ${VLFEAT_LIB} OpenMP::OpenMP_CXX)

add_library( train_assign STATIC train_assign.cpp )
target_link_libraries( train_assign
    feat_getter
    flat_desc_file
    image_util
    par_queue
    same_random
    ${VLFEAT_LIB}
    ${Boost_LIBRARIES} )

add_library( train_descs STATIC train_descs.cpp )
target_link_libraries( train_descs
    clst_centres
    par_queue
    ${Boost_LIBRARIES} )

add_library( train_hamming STATIC train_hamming.cpp )
target_link_libraries( train_hamming
    clst_centres
    flat_desc_file
    median_computer
    par_queue
    protobuf_util
    same_random
    hamming_data.pb
    ${Boost_LIBRARIES} )

add_library( train_resize STATIC train_resize.cpp )
target_link_libraries( train_resize
    par_queue
    vise_util
    task_progress
    ${Boost_LIBRARIES} ${ImageMagick_LIBRARIES} )

#add_executable( train_resize_test train_resize_test.cc )
#target_link_libraries( train_resize_test train_resize par_queue ${Boost_LIBRARIES} ${ImageMagick_LIBRARIES} )
