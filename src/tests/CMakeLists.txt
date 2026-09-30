if(WIN32)
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/util )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/dataset )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/embedding )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/evaluation )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/indexing )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/retrieval )

	add_executable( test_oxford_buildings test_oxford_buildings.cc )
	target_link_libraries( test_oxford_buildings
		vise_util
		dataset_v2
		evaluator_oxford
		hamming
		proto_db
		proto_db_file
		proto_index
		spatial_verif_v2
		feat_standard
		tfidf_v2
		image_util
		${ImageMagick_LIBRARIES}
		OpenMP::OpenMP_CXX
    project_manager http_request http_response project vise_util
	)
endif(WIN32)

if(UNIX)
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/util )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/dataset )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/embedding )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/evaluation )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/indexing )
  include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/retrieval )

	add_executable( test_oxford_buildings test_oxford_buildings.cc )
	target_link_libraries( test_oxford_buildings
		vise_util
		dataset_v2
		evaluator_oxford
		hamming
		proto_db
		proto_db_file
		proto_index
		spatial_verif_v2
		feat_standard
		tfidf_v2
		image_util
		${ImageMagick_LIBRARIES}
		${Boost_LIBRARIES}
		OpenMP::OpenMP_CXX
    project_manager http_request http_response project vise_util
	)

  if(APPLE)
    # fix required to allow loader resolve the path of vlfeat library in MacOS
    add_custom_command(TARGET test_oxford_buildings
      POST_BUILD COMMAND
      ${CMAKE_INSTALL_NAME_TOOL} -change "@loader_path/libvl.dylib" "@rpath/libvl.dylib"
      $<TARGET_FILE:test_oxford_buildings>)
  endif(APPLE)
endif(UNIX)
option(VISE_BUILD_FEATURE_STATUS_TEST "Build the forward-index feature-status regression" OFF)
if(VISE_BUILD_FEATURE_STATUS_TEST)
  add_executable(test_file_feature_status test_file_feature_status.cc)
  target_link_libraries(test_file_feature_status project_manager ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} OpenMP::OpenMP_CXX)
  set(VISE_FEATURE_STATUS_TEST_PROJECT "" CACHE PATH "Disposable completed index with a featureless document")
  set(VISE_FEATURE_STATUS_EMPTY_FILENAME "" CACHE STRING "Featureless filename in the test index")
  if(VISE_FEATURE_STATUS_TEST_PROJECT AND VISE_FEATURE_STATUS_EMPTY_FILENAME)
    add_test(NAME file_feature_status COMMAND test_file_feature_status ${VISE_FEATURE_STATUS_TEST_PROJECT} ${VISE_FEATURE_STATUS_EMPTY_FILENAME})
  endif()
endif()
