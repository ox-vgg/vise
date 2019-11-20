add_executable( test_median_computer test_median_computer.cpp )
target_link_libraries( test_median_computer median_computer )

add_executable( test_uintToShortStr test_uintToShortStr.cpp )
target_link_libraries( test_uintToShortStr )

add_executable( test_slow_construction test_slow_construction.cpp )
target_link_libraries( test_slow_construction slow_construction -lpthread)
