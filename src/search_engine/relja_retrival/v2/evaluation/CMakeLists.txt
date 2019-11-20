#add_subdirectory( tests )

PROTOBUF_GENERATE_CPP(eval_query.pb.cpp eval_query.pb.h eval_query.proto)
add_library( eval_query.pb ${eval_query.pb.cpp} )
target_link_libraries( eval_query.pb ${PROTOBUF_LIBRARIES} )

add_library( evaluator_v2 evaluator_v2.cpp )
target_link_libraries( evaluator_v2
    dataset_v2
    eval_query.pb
    multi_query
    proto_db_file
    retriever
    thread_queue
    ${Boost_LIBRARIES} )
