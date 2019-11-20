add_library( colour_sift colour_sift.cpp )
target_link_libraries( colour_sift
    image_util
    ${Boost_LIBRARIES} )

add_library( feat_getter feat_getter.cpp )
target_link_libraries( feat_getter ellipse ${ImageMagick_LIBRARIES} )

add_library( feat_standard feat_standard.cpp )
target_link_libraries( feat_standard
    colour_sift
    compute_descriptors
    detect_points
    feat_getter
    holidays_public
    image_util
    ${Boost_LIBRARIES} )

add_library( holidays_public holidays_public.cpp )
target_link_libraries( holidays_public
    image_util
    ${Boost_LIBRARIES} )
