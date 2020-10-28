cmake_minimum_required(VERSION 2.8)
project(KMCode_relja)

set( BUILD_SHARED_LIBS "ON" )

if (UNIX)
  find_package(PNG REQUIRED)
	find_package(JPEG REQUIRED)
	# soooo many warnings, suppress..
	set( KM_COMPILE_FLAGS "-Wno-unused-variable -Wno-sign-compare -Wno-deprecated -Wno-write-strings -Wno-unused-result -Wno-format-security -Wno-unused-but-set-variable -Wno-uninitialized -Wno-parentheses -Wno-strict-overflow -DQUIET")
	set( KM_LINKER_FLAGS "-fPIC -O2 -g3 -fno-rtti -fno-exceptions")
endif (UNIX)

if (WIN32)
	# @todo improve the way libpng and libjpeg are found
	find_package(PNG REQUIRED)
	#find_package(JPEG REQUIRED)
	#find_package( libpng EQUIRED)
	#find_package(JPEG REQUIRED)
	#SET(PNG_LIBRARIES "C:\\Users\\tlm\\dep\\vise\\tmp_libsrc\\lpng1637\\projects\\vstudio\\x64\\Release\\libpng16.lib")
	#SET(PNG_INCLUDE_DIRS "C:\\Users\\tlm\\dep\\vise\\tmp_libsrc\\lpng1637")
	SET(JPEG_INCLUDE_DIR "C:\\Users\\tlm\\dep\\vise\\tmp_libsrc\\jpeg-9d")
	SET(JPEG_LIBRARIES "C:\\Users\\tlm\\dep\\vise\\tmp_libsrc\\jpeg-9d\\Release\\x64\\jpeg.lib")
	message("JPEG_INCLUDE_DIR: " ${JPEG_INCLUDE_DIR})
	message("JPEG_LIBRARIES: " ${JPEG_LIBRARIES})
	message("PNG_INCLUDE_DIRS: " ${PNG_INCLUDE_DIRS})
	message("PNG_LIBRARIES: " ${PNG_LIBRARIES})

        include_directories(${JPEG_INCLUDE_DIR})
	include_directories(${PNG_INCLUDE_DIRS})
endif (WIN32)

if (APPLE)
	# @todo improve the way libpng and libjpeg are found
	find_package(PNG REQUIRED)
	find_package(JPEG REQUIRED)
	message("JPEG_INCLUDE_DIR: " ${JPEG_INCLUDE_DIR})
	message("JPEG_LIBRARIES: " ${JPEG_LIBRARIES})
	message("PNG_INCLUDE_DIRS: " ${PNG_INCLUDE_DIRS})
	message("PNG_LIBRARIES: " ${PNG_LIBRARIES})

        include_directories(${JPEG_INCLUDE_DIR})
	include_directories(${PNG_INCLUDE_DIRS})
endif (APPLE)


file(GLOB matrix_src "matrix/*.cpp")

add_library( kmbase STATIC
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
target_link_libraries( kmbase ${JPEG_LIBRARIES} ${PNG_LIBRARIES})
SET_TARGET_PROPERTIES(kmbase PROPERTIES $COMPILE_FLAGS ${KM_COMPILE_FLAGS} $LINK_FLAGS ${KM_LINKER_FLAGS})


# add_executable( detect_points.ln exec/detect_points/detect_points.cpp )
# target_link_libraries( detect_points.ln kmbase "png" "jpeg" )
# SET_TARGET_PROPERTIES(detect_points.ln PROPERTIES COMPILE_FLAGS ${KM_COMPILE_FLAGS} LINK_FLAGS ${KM_LINKER_FLAGS})

add_library( detect_points STATIC exec/detect_points/detect_points.cpp )
target_link_libraries( detect_points kmbase ${JPEG_LIBRARIES} ${PNG_LIBRARIES} )
SET_TARGET_PROPERTIES(detect_points PROPERTIES $COMPILE_FLAGS ${KM_COMPILE_FLAGS} $LINK_FLAGS ${KM_LINKER_FLAGS})

# add_executable( compute_descriptors.ln exec/compute_descriptors/compute_descriptors.cpp )
# target_link_libraries( compute_descriptors.ln kmbase "png" "jpeg" )
# SET_TARGET_PROPERTIES(compute_descriptors.ln PROPERTIES COMPILE_FLAGS ${KM_COMPILE_FLAGS} LINK_FLAGS ${KM_LINKER_FLAGS})

add_library( compute_descriptors STATIC exec/compute_descriptors/compute_descriptors.cpp )
target_link_libraries( compute_descriptors kmbase ${JPEG_LIBRARIES} ${PNG_LIBRARIES} )
SET_TARGET_PROPERTIES(compute_descriptors PROPERTIES $COMPILE_FLAGS ${KM_COMPILE_FLAGS} $LINK_FLAGS ${KM_LINKER_FLAGS})
