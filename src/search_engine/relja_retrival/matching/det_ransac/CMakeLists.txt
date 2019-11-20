add_library( putative putative.cpp )
target_link_libraries( putative )

add_library( ellipse ellipse.cpp )
target_link_libraries( ellipse )

add_library( homography homography.cpp )
target_link_libraries( homography ellipse )

add_library( det_ransac det_ransac.cpp )
target_link_libraries( det_ransac ellipse homography putative same_random ${Boost_LIBRARIES} )
