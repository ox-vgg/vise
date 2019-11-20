cmake_minimum_required(VERSION 2.8)
project(KMCode_relja)

set( BUILD_SHARED_LIBS "ON" )


# soooo many warnings, suppress..
set( KM_COMPILE_FLAGS "-Wno-unused-variable -Wno-sign-compare -Wno-deprecated -Wno-write-strings -Wno-unused-result -Wno-format-security -Wno-unused-but-set-variable -Wno-uninitialized -Wno-parentheses -Wno-strict-overflow -DQUIET")
set( KM_LINKER_FLAGS "-fPIC -O2 -g3 -fno-rtti -fno-exceptions")



file(GLOB matrix_src "matrix/*.cpp")

add_library( kmbase
descriptor/Corner.cpp
descriptor/CornerDescriptor.cpp
descriptor/JetLocal.cpp
descriptor/SiftDescriptor.cpp
descriptor/AffineRegion.cpp
descriptor/KoenDescriptor.cpp
descriptor/JetLocalAffine.cpp
descriptor/CornerDetector.cpp
descriptor/dog.cpp 
descriptor/FastDetector.cpp
descriptor/MomentDescriptor.cpp
descriptor/ComplexGauss.cpp
descriptor/SpinDescriptor.cpp
descriptor/AffineRegion.cpp
descriptor/SiftDescriptor.cpp
descriptor/PcaDescriptor.cpp
descriptor/ESiftDescriptor.cpp
descriptor/ShapeDescriptor.cpp
descriptor/EdgeDetector.cpp
descriptor/EdgeDescriptor.cpp
descriptor/CCDescriptor.cpp
${matrix_src}
ttime/ttime.cpp
stats/stats.cpp
ImageContent/ImageContent.cpp
gauss_iir/gauss_iir.cpp
homography/homography.cpp
)
target_link_libraries( kmbase "png" "jpeg" )
SET_TARGET_PROPERTIES(kmbase PROPERTIES COMPILE_FLAGS ${KM_COMPILE_FLAGS} LINK_FLAGS ${KM_LINKER_FLAGS})


# add_executable( detect_points.ln exec/detect_points/detect_points.cpp )
# target_link_libraries( detect_points.ln kmbase "png" "jpeg" )
# SET_TARGET_PROPERTIES(detect_points.ln PROPERTIES COMPILE_FLAGS ${KM_COMPILE_FLAGS} LINK_FLAGS ${KM_LINKER_FLAGS})

add_library( detect_points exec/detect_points/detect_points.cpp )
target_link_libraries( detect_points kmbase "png" "jpeg" )
SET_TARGET_PROPERTIES(detect_points PROPERTIES COMPILE_FLAGS ${KM_COMPILE_FLAGS} LINK_FLAGS ${KM_LINKER_FLAGS})

# add_executable( compute_descriptors.ln exec/compute_descriptors/compute_descriptors.cpp )
# target_link_libraries( compute_descriptors.ln kmbase "png" "jpeg" )
# SET_TARGET_PROPERTIES(compute_descriptors.ln PROPERTIES COMPILE_FLAGS ${KM_COMPILE_FLAGS} LINK_FLAGS ${KM_LINKER_FLAGS})

add_library( compute_descriptors exec/compute_descriptors/compute_descriptors.cpp )
target_link_libraries( compute_descriptors kmbase "png" "jpeg" )
SET_TARGET_PROPERTIES(compute_descriptors PROPERTIES COMPILE_FLAGS ${KM_COMPILE_FLAGS} LINK_FLAGS ${KM_LINKER_FLAGS})
