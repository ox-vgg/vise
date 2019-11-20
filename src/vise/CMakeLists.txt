add_library(vise_util STATIC vise_util.cc)

add_library(search_engine STATIC search_engine.cc)
add_library(project STATIC project.cc)
target_link_libraries(project search_engine vise_util)

add_library(http_request STATIC http_request.cc)
add_library(http_response STATIC http_response.cc)

add_library(project_manager STATIC project_manager.cc)
target_link_libraries(project_manager http_request http_response project)


add_library( connection STATIC connection.cc)
target_link_libraries(connection project_manager)
add_library(http_server STATIC http_server.cc)
target_link_libraries(http_server vise_util project_manager http_request http_response connection ${Boost_LIBRARIES})

add_executable(vise vise.cc)
target_link_libraries(vise vise_util http_server)

add_executable(unit_tests unit_tests.cc)
target_link_libraries(unit_tests vise_util ${Boost_LIBRARIES})

add_test(unit_tests unit_tests)
