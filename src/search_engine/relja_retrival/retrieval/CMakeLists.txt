add_library( retriever STATIC retriever.cpp )
target_link_libraries( retriever util)

add_library( spatial_retriever STATIC spatial_retriever.cpp )
target_link_libraries( spatial_retriever same_random )

add_library( multi_query STATIC multi_query.cpp )
target_link_libraries( multi_query retriever )
