#add_subdirectory( tests )

#add_executable( convert_hamming_formats convert_hamming_formats.cpp )
#target_link_libraries( convert_hamming_formats hamming_embedder )

add_library( embedder embedder.cpp )
target_link_libraries( embedder
    char_streams )

PROTOBUF_GENERATE_CPP(hamming_data.pb.cpp hamming_data.pb.h hamming_data.proto)
add_library( hamming_data.pb ${hamming_data.pb.cpp} )
target_link_libraries( hamming_data.pb ${PROTOBUF_LIBRARIES} )

add_library( hamming_embedder hamming_embedder.cpp )
target_link_libraries( hamming_embedder
    char_streams
    embedder
    hamming_data.pb )
