add_library( register_images STATIC register_images.cpp )
target_link_libraries( register_images
    feat_standard
    ellipse
    putative
    det_ransac
    ${ImageMagick_LIBRARIES}
    ${Boost_LIBRARIES} )
