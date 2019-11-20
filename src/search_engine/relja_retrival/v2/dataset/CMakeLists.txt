#add_subdirectory( tests )

PROTOBUF_GENERATE_CPP(dataset_entry.pb.cpp dataset_entry.pb.h dataset_entry.proto)
add_library( dataset_entry.pb ${dataset_entry.pb.cpp} )
target_link_libraries( dataset_entry.pb ${PROTOBUF_LIBRARIES} )


add_library( dataset_v2 dataset_v2.cpp )
target_link_libraries( dataset_v2
    dataset_entry.pb
    proto_db_file
    ${Boost_LIBRARIES} )
