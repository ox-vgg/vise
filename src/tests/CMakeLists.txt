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

if(UNIX AND NOT APPLE)
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
endif(UNIX AND NOT APPLE)
