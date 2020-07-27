set(CMAKE_INCLUDE_CURRENT_DIR ON)

# search_engine::relja_retrival
# for protobufs
# protobuf compiles *.pb.cc and *.pb.h headers in the cmake build tree
# therefore, we need to include the corresponding folders in the build tree
# should point to a folder inside "cmake_build/"
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/util )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/dataset )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/embedding )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/evaluation )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/indexing )
include_directories( ${CMAKE_BINARY_DIR}/search_engine/relja_retrival/v2/retrieval )


add_library(vise_util STATIC vise_util.cc)
add_library(task_progress STATIC task_progress.cc)

add_library(search_engine STATIC search_engine.cc)
add_library(project STATIC project.cc)
target_link_libraries(project search_engine vise_util relja_retrival)

add_library(http_request STATIC http_request.cc)
add_library(http_response STATIC http_response.cc)

add_library(project_manager STATIC project_manager.cc)
target_link_libraries(project_manager http_request http_response project)

add_library( connection STATIC connection.cc)
target_link_libraries(connection project_manager)
add_library(http_server STATIC http_server.cc)
target_link_libraries(http_server vise_util project_manager http_request http_response connection ${Boost_LIBRARIES})

if (WIN32)
	#add_executable(VISE WIN32 vise.cc)
	#target_link_libraries(VISE vise_util http_server ${ImageMagick_LIBRARIES})

	add_executable(VISE vise.cc)
	target_link_libraries(VISE vise_util http_server ${ImageMagick_LIBRARIES})

	install(TARGETS VISE DESTINATION bin)
else(WIN32)
	add_executable(vise vise.cc)
	target_link_libraries(vise vise_util http_server ${ImageMagick_LIBRARIES})
endif (WIN32)

#add_executable(unit_tests unit_tests.cc)
#target_link_libraries(unit_tests vise_util ${Boost_LIBRARIES})

#add_executable(project_manager_test project_manager_test.cc)
#target_link_libraries(project_manager_test project_manager http_request http_response project vise_util)

#add_test(NAME unit_tests COMMAND unit_tests WORKING_DIRECTORY ${VISE_SOURCE_ROOT_DIR})
#add_test(NAME project_manager_test COMMAND project_manager_test WORKING_DIRECTORY ${VISE_SOURCE_ROOT_DIR})
