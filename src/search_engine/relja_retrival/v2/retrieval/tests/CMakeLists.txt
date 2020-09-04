if (WIN32)
	#add_executable(VISE WIN32 vise.cc)
	#target_link_libraries(VISE vise_util http_server ${ImageMagick_LIBRARIES})

	add_executable( test_oxford_buildings test_oxford_buildings.cc )
	target_link_libraries( test_oxford_buildings
		vise_util
		dataset_v2
		evaluator_v2
		hamming
		proto_db
		proto_db_file
		proto_index
		spatial_verif_v2
		feat_standard
		tfidf_v2
		image_util
		${ImageMagick_LIBRARIES}
	)
	install(TARGETS test_oxford_buildings DESTINATION bin)

else(WIN32)
	add_executable( test_oxford_buildings test_oxford_buildings.cc )
	target_link_libraries( test_oxford_buildings
		vise_util
		dataset_v2
		evaluator_v2
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
	)
endif (WIN32)
