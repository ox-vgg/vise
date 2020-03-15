/** @file project_manager_test.h
 *  @brief tests for project_manager.cc
 *  @author Abhishek Dutta
 *  @date 04 Dec. 2019
 */
#include "project_manager.h"
#include "http_response.h"
#include "vise_util.h"

#include <boost/filesystem.hpp>
#include <Magick++.h>

#include <string>
#include <map>
#include <chrono>
#include <thread>
#include <cassert>

int main(int argc, char** argv) {
  std::cout << "running project_manager_test" << std::endl;
  Magick::InitializeMagick(*argv);
  
  const boost::filesystem::path visehome = vise::vise_home();
  boost::filesystem::path vise_settings = visehome / "vise_settings.txt";
  ASSERT(boost::filesystem::exists(vise_settings));

  // load VISE configuration
  std::map<std::string, std::string> conf;
  vise::configuration_load(vise_settings.string(), conf);
  ASSERT(!conf.empty());

  vise::project_manager manager(conf);
  std::string pname("ox200test");
  if(manager.project_exists(pname)) {
    bool delete_success = manager.project_delete(pname);
    ASSERT(delete_success);
  }
  bool create_success = manager.project_create(pname);
  ASSERT(create_success);
  bool load_success = manager.project_load(pname);
  ASSERT(load_success);

  // @todo: automatically download test images
  boost::filesystem::path imdir("C:\\Users\\tlm\\dep\\vise\\sample_images\\ox5k\\ox5k_debug_200");

  std::map<std::string, std::string> param
    {
     {"source_type", "local_folder"},
     {"source_loc", imdir.string()}
    };
  vise::http_response file_add_response;
  manager.project_file_add(pname, param, file_add_response);
  ASSERT(file_add_response.d_status_code == 200);

  vise::http_response index_create_response;
  manager.project_index_create(pname, index_create_response);
  std::cout << "response=" << index_create_response.d_status_code << ": " << index_create_response.d_payload << std::endl;
  ASSERT(index_create_response.d_status_code == 303);

  uint32_t wait_time_ms = 1000;
  vise::http_response index_load_response;
  while(!manager.project_index_is_done(pname)) {
    //std::cout << "waiting for " << wait_time_ms << " ms before checking ..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_ms));
  }

  ASSERT(manager.project_index_is_loaded(pname));

  vise::http_response index_unload_response;
  manager.project_index_unload(pname, index_unload_response);
  ASSERT(index_unload_response.d_status_code == 200);

  ASSERT(!manager.project_index_is_loaded(pname));
}
