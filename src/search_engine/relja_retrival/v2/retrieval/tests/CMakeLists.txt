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
    tfidf_v2 )
