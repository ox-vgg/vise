#add_subdirectory( tests )

add_library( hamming hamming.cpp )
target_link_libraries( hamming
    hamming_embedder
    tfidf_v2
    retriever_v2 )

add_library( image_graph image_graph.cpp )
target_link_libraries( image_graph
    index_entry.pb
    par_queue
    proto_db
    proto_db_file
    proto_index
    retriever)

add_library( mq_filter_outliers mq_filter_outliers.cpp )
target_link_libraries( mq_filter_outliers
    hamming_embedder
    multi_query
    retriever_v2)

add_library( mq_joint_avg mq_joint_avg.cpp )
target_link_libraries( mq_joint_avg
    hamming_embedder
    multi_query
    protobuf_util
    retriever_v2
    spatial_verif_v2)

add_library( retriever_v2 retriever_v2.cpp )
target_link_libraries( retriever_v2
    clst_centres
    embedder
    feat_getter
    image_util
    index_entry.pb
    index_entry_util
    proto_index
    retriever
    uniq_entries
    ${Boost_LIBRARIES} ${VLFEAT_LIB} )

add_executable( create_image_graph create_image_graph.cpp )
target_link_libraries( create_image_graph
    dataset_v2
    hamming
    image_graph
    mpi_queue
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    tfidf_v2 )

add_executable( export_image_graph export_image_graph.cpp )
target_link_libraries( export_image_graph
    dataset_v2
    hamming
    image_graph
    mpi_queue
    proto_db
    proto_db_file
    proto_index
    spatial_verif_v2
    tfidf_v2 )

add_library( spatial_verif_v2 spatial_verif_v2.cpp )
target_link_libraries( spatial_verif_v2 daat det_ransac ellipse homography index_entry_util par_queue retriever_v2 uniq_entries ${Boost_LIBRARIES} ${VLFEAT_LIB} )

PROTOBUF_GENERATE_CPP(tfidf_data.pb.cpp tfidf_data.pb.h ./tfidf_data.proto)
add_library( tfidf_data.pb ${tfidf_data.pb.cpp} )
target_link_libraries( tfidf_data.pb ${PROTOBUF_LIBRARIES} )

add_library( tfidf_v2 tfidf_v2.cpp )
target_link_libraries( tfidf_v2 feat_getter retriever_v2 tfidf_data.pb weighter_v2 ${Boost_LIBRARIES} ${VLFEAT_LIB} )

add_library( uniq_retriever uniq_retriever.cpp )
target_link_libraries( uniq_retriever )

add_library( weighter_v2 weighter_v2.cpp )
target_link_libraries( weighter_v2 index_entry.pb proto_index )

add_library( wgc wgc.cpp )
target_link_libraries( wgc retriever_v2 tfidf_v2 tfidf_data.pb weighter_v2 ${Boost_LIBRARIES} )
