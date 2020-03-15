add_library( putative STATIC putative.cpp )
target_link_libraries( putative )

add_library( ellipse STATIC ellipse.cpp )
target_link_libraries( ellipse )

add_library( homography STATIC homography.cpp )
target_link_libraries( homography ellipse )

add_library( det_ransac STATIC det_ransac.cpp )
target_link_libraries( det_ransac ellipse homography putative same_random ${Boost_LIBRARIES} )
