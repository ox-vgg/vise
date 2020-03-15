add_library( clst_centres STATIC clst_centres.cpp )
target_link_libraries( clst_centres )

add_library( index_with_data STATIC index_with_data.cpp )
target_link_libraries( index_with_data slow_construction ${Boost_LIBRARIES} )

