add_library( clst_centres clst_centres.cpp )
target_link_libraries( clst_centres )

add_library( index_with_data index_with_data.cpp )
target_link_libraries( index_with_data slow_construction ${Boost_LIBRARIES} )

add_library( index_with_data_file index_with_data_file.cpp )
target_link_libraries( index_with_data_file index_with_data )

add_library( index_with_data_file_fixed1 index_with_data_file_fixed1.cpp )
target_link_libraries( index_with_data_file_fixed1 index_with_data )
