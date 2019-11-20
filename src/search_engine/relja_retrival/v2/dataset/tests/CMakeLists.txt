add_executable( convert_text_to_dset convert_text_to_dset.cpp )
target_link_libraries( convert_text_to_dset dataset_v2 )

add_executable( dataset_v2_test dataset_v2_test.cpp )
target_link_libraries( dataset_v2_test dataset_v2 )
