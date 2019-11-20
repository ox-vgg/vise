if (cREGISTER)
    set(REGISTER_LIB "register_images")
endif (cREGISTER)

add_library( abs_api abs_api.cpp )
target_link_libraries( abs_api ${Boost_LIBRARIES} ${MPI_LIBRARIES} )

add_library( spatial_api spatial_api.cpp )
target_link_libraries( spatial_api
    abs_api
    multi_query
    spatial_retriever
    ${Boost_LIBRARIES}
    ${MPI_LIBRARIES}
    ${REGISTER_LIB} )

# TODO: can be trivially ported to v2
# add_executable( single_api single_api.cpp )
# target_link_libraries( single_api
#     abs_api
#     dataset_v2
#     retriever
#     multi_query
#     nn_single_retriever
#     nn_raw_single_retriever
#     product_quant
#     coarse_residual
#     index_with_data_file
#     index_with_data_file_fixed1
#     iidx_with_data_builder
#     ${Boost_LIBRARIES} )
