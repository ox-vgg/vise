add_executable( daat_test daat_test.cpp )
target_link_libraries( daat_test daat proto_db proto_db_file proto_index )

add_executable( idx_diff idx_diff.cpp )
target_link_libraries( idx_diff proto_db_file proto_index )

add_executable( imbalance_factor imbalance_factor.cpp )
target_link_libraries(
    imbalance_factor
    proto_db
    proto_db_file
    proto_index )

add_executable( invert_test invert_test.cpp )
target_link_libraries( invert_test proto_db proto_db_file proto_index )

add_executable( reduce_idxs reduce_idxs.cpp )
target_link_libraries( reduce_idxs
    dataset_v2
    embedder
    hamming_embedder
    proto_db
    proto_db_file
    proto_index )

add_executable( reduce_idxs_scale reduce_idxs_scale.cpp )
target_link_libraries( reduce_idxs_scale
    dataset_v2
    embedder
    hamming_embedder
    index_entry_util
    proto_db
    proto_db_file
    proto_index )

add_executable( stupid_create_iidx_test stupid_create_iidx_test.cpp )
target_link_libraries( stupid_create_iidx_test proto_db proto_db_file proto_index )

add_executable( test_quant_ellipse test_quant_ellipse.cpp )
target_link_libraries( test_quant_ellipse proto_index )
