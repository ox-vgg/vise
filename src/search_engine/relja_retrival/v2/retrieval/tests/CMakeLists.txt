add_executable( eval_multi eval_multi.cpp )
target_link_libraries( eval_multi
    dataset_v2
    evaluator_v2
    hamming
    mq_joint_avg
    multi_query
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    tfidf_v2 )

add_executable( eval_Pitts eval_Pitts.cpp )
target_link_libraries( eval_Pitts
    common_data.pb
    dataset_v2
    evaluator_v2
    hamming
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    tfidf_v2
    uniq_retriever
    ${Boost_LIBRARIES} )

add_executable( eval_SF eval_SF.cpp )
target_link_libraries( eval_SF
    common_data.pb
    dataset_v2
    evaluator_v2
    hamming
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    tfidf_v2
    uniq_retriever
    ${Boost_LIBRARIES} )

add_executable( eval_spat_v2 eval_spat_v2.cpp )
target_link_libraries( eval_spat_v2
    dataset_v2
    evaluator_v2
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    tfidf_v2 )

add_executable( eval_tfidf_v2 eval_tfidf_v2.cpp )
target_link_libraries( eval_tfidf_v2
    dataset_v2
    evaluator_v2
    proto_db
    proto_db_file
    proto_index
    tfidf_v2 )

add_executable( eval_wgc eval_wgc.cpp )
target_link_libraries( eval_wgc
    dataset_v2
    evaluator_v2
    proto_db
    proto_db_file
    proto_index
    tfidf_v2
    wgc )

add_executable( example_image_graph example_image_graph.cpp )
target_link_libraries( example_image_graph
    dataset_v2
    hamming
    image_graph
    mpi_queue
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    tfidf_v2 )

add_executable( internal_query_speed internal_query_speed.cpp )
target_link_libraries( internal_query_speed
    dataset_v2
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    tfidf_v2 )

add_executable( retv2_temp retv2_temp.cpp )
target_link_libraries( retv2_temp
    dataset_v2
    evaluator_v2
    hamming
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    feat_standard
    tfidf_v2 )
