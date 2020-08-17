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
  if(argc != 2) {
    std::cout << "Usage: " << argv[0] << " TEST_IMAGES_DIR" << std::endl;
    return 1;
  }

  std::cout << "running project_manager_test" << std::endl;
  std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

  boost::filesystem::path imdir(argv[1]);
  Magick::InitializeMagick(*argv);

  const boost::filesystem::path visehome = vise::vise_home();
  boost::filesystem::path vise_settings = visehome / "vise_settings.txt";
  ASSERT(boost::filesystem::exists(vise_settings));

  // load VISE configuration
  std::map<std::string, std::string> conf;
  vise::configuration_load(vise_settings.string(), conf);
  ASSERT(!conf.empty());

  vise::project_manager manager(conf);
  std::string pname("_test_project_manager");
  vise::http_response last_http_response;

  if(manager.project_exists(pname)) {
    std::map<std::string, std::string> del_pname_list;
    del_pname_list[pname] = "1";
    manager.vise_project_delete(del_pname_list, last_http_response);
    std::cout << last_http_response.d_status_code << " " << last_http_response.d_status
              << " : " << last_http_response.d_payload << std::endl;
    ASSERT(last_http_response.d_status_code == 303); // redirect to settings page
  }
  vise::http_response project_create_response;
  std::map<std::string, std::string> create_pname_list;
  create_pname_list["pname"] = pname;
  manager.vise_project_create(create_pname_list, last_http_response);
  std::cout << "manager.vise_project_create(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 303);
  bool load_success = manager.project_load(pname);
  ASSERT(load_success);

  // count number of image files in image folder
  uint32_t img_count = 0;
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator it(imdir); it!=end_itr; ++it) {
    if(boost::filesystem::is_regular_file(it->path())) {
      if(it->path().extension() == ".jpg" ||
         it->path().extension() == ".JPG" ||
         it->path().extension() == ".jpeg" ||
         it->path().extension() == ".JPEG" ||
         it->path().extension() == ".png" ||
         it->path().extension() == ".PNG" ||
         it->path().extension() == ".TIF" ||
         it->path().extension() == ".tif" ||
         it->path().extension() == ".TIFF" ||
         it->path().extension() == ".tiff"
         ) {
        img_count++;
      }
    }
  }

  std::map<std::string, std::string> param
    {
     {"source_type", "local_folder"},
     {"source_loc", imdir.string()}
    };
  vise::http_response file_add_response;
  manager.project_file_add(pname, param, last_http_response);
  std::cout << "manager.project_file_add(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 303);
  ASSERT(manager.project_image_src_count(pname) == img_count);

  // set configuration to auto
  std::string preset_id = "preset_conf_manual";
  manager.project_config_use_preset(pname, preset_id, last_http_response);
  std::cout << "manager.project_config_use_preset(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 200);
  std::ostringstream config;
  config << "search_engine=relja_retrival\n";
  config << "use_root_sift=true\n";
  config << "sift_scale_3=true\n";
  config << "bow_descriptor_count=-1\n";
  config << "bow_cluster_count=1000\n";
  config << "cluster_num_iteration=3\n";
  config << "hamm_embedding_bits=32\n";
  config << "resize_dimension=-1\n";
  std::string config_str(config.str());
  manager.project_config_save(pname, config_str, last_http_response);
  std::cout << "manager.project_config_save(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 200);

  vise::http_response index_create_response;
  bool block_until_done = true;
  std::cout << "Creating index ... (blocking until done)" << std::endl;
  manager.project_index_create(pname, last_http_response, block_until_done);
  std::cout << "manager.project_index_create(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(manager.project_index_is_done(pname));

  //uint32_t wait_time_ms = 3000;
  //std::cout << "waiting for " << wait_time_ms << " ms to allow loading of index ..." << std::endl;
  //std::this_thread::sleep_for(std::chrono::milliseconds(wait_time_ms));
  ASSERT(manager.project_index_is_loaded(pname));

  vise::http_response index_unload_response;
  manager.project_index_unload(pname, last_http_response);
  std::cout << "manager.project_index_inload(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 200);
  ASSERT(!manager.project_index_is_loaded(pname));

  // delete test project
  std::map<std::string, std::string> delete_pname_list;
  delete_pname_list[pname] = "1";
  manager.vise_project_delete(delete_pname_list, last_http_response);
  std::cout << "manager.vise_project_delete(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 303);
  ASSERT(!manager.project_exists(pname));

  std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);
  std::cout << "project_manager_test,ok," << ms.count() << std::endl;
  return 0;
}
