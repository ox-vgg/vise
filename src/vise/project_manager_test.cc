/** @file project_manager_test.h
 *  @brief tests for project_manager.cc
 *  @author Abhishek Dutta
 *  @date 04 Dec. 2019
 */
#include "project_manager.h"
#include "http_response.h"

#include <string>
#include <map>
#include <chrono>
#include <thread>

#define BOOST_TEST_MODULE project_manager
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( project_index ) {
  std::string conf_fn("src/conf/viseconf.txt");
  BOOST_TEST( boost::filesystem::exists(boost::filesystem::path(conf_fn)) );

  // load VISE configuration
  std::map<std::string, std::string> conf;
  vise::configuration_load(conf_fn, conf);
  BOOST_TEST(!conf.empty());

  vise::project_manager manager(conf);
  std::string pname("ox200test");
  if(manager.project_exists(pname)) {
    manager.project_delete(pname);
  }
  manager.project_create(pname);
  bool load_success = manager.project_load(pname);
  BOOST_TEST(load_success);

  boost::filesystem::path data_dir = boost::filesystem::current_path();
  data_dir = data_dir / "data";
  data_dir = data_dir / "test";
  data_dir = data_dir / "ox200test";

  boost::filesystem::path imdir(data_dir);
  imdir = imdir / "images";
  boost::filesystem::path proj_conf_fn(data_dir);
  proj_conf_fn = proj_conf_fn / "conf.txt";

  std::map<std::string, std::string> param
    {
     {"source_type", "local_folder"},
     {"source_loc", imdir.string()}
    };
  vise::http_response file_add_response;
  manager.project_file_add(pname, param, file_add_response);
  BOOST_TEST(file_add_response.d_status_code == 200);

  std::string proj_conf_str;
  bool ok = vise::file_load(proj_conf_fn, proj_conf_str);
  BOOST_TEST(ok);

  vise::http_response conf_set_response;
  manager.project_conf_set(pname, proj_conf_str, conf_set_response);
  BOOST_TEST(conf_set_response.d_status_code == 200);

  vise::http_response index_create_response;
  manager.project_index_create(pname, index_create_response);
  BOOST_TEST(index_create_response.d_status_code == 200);
  std::cout << "response=" << index_create_response.d_payload << std::endl;

  while(!manager.project_index_is_done(pname)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  vise::http_response index_load_response;
  manager.project_index_load(pname, index_load_response);
  BOOST_TEST(index_create_response.d_status_code == 200);

  BOOST_TEST(manager.project_index_is_loaded(pname));

  vise::http_response index_unload_response;
  manager.project_index_unload(pname, index_unload_response);
  BOOST_TEST(index_unload_response.d_status_code == 200);

  BOOST_TEST(!manager.project_index_is_loaded(pname));
}
