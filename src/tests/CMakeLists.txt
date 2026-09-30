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
option(VISE_BUILD_INDEX_ONLY_TEST "Build the external-gallery integration test" OFF)
if(VISE_BUILD_INDEX_ONLY_TEST)
  add_executable(test_index_only test_index_only.cc)
  target_link_libraries(test_index_only project_manager ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} OpenMP::OpenMP_CXX)
  set(VISE_INDEX_ONLY_TEST_PROJECT "" CACHE PATH "Completed disposable project used by the integration test")
  set(VISE_INDEX_ONLY_TEST_QUERY "" CACHE FILEPATH "Query image for the integration test")
  if(VISE_INDEX_ONLY_TEST_PROJECT AND VISE_INDEX_ONLY_TEST_QUERY)
    add_test(NAME index_only COMMAND test_index_only ${VISE_INDEX_ONLY_TEST_PROJECT} ${VISE_INDEX_ONLY_TEST_QUERY})
  endif()
endif()
