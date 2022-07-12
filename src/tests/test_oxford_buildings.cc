//
// Evaluate search engine on oxford buildings dataset
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 12 June 2021
//

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cassert>
#include <cmath>

#include "dataset_v2.h"
#include "evaluator_oxford.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "spatial_verif_v2.h"
#include "tfidf_v2.h"
#include "timing.h"
#include "util.h"
#include "vise/vise_util.h"

#include "project_manager.h"
#include "http_response.h"
#include "vise_util.h"

#include <boost/filesystem.hpp>

vise::http_request create_http_request(const std::string method,
                                       const std::string pname,
                                       const std::string uri) {
    std::ostringstream req_stream;
    req_stream << method << " /" << pname << "/" << uri << " HTTP/1\r\n\r\n";
    vise::http_request req;
    req.parse(req_stream.str());
    return req;
}

int main(int argc, char **argv) {
  if(argc != 6) {
    std::cout << "Usage: " << argv[0] << " TEST_ID IMG_DIR GND_TRUTH_DIR TOLERANCE TMP_DIR" << std::endl;
    return 1;
  }
  std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

  std::string test_id(argv[1]);
  if(test_id != "oxford-buildings-100" &&
      test_id != "oxford-buildings-5k") {
    std::cout << "TEST_ID must be in "
              << "{oxford-buildings-100,oxford-buildings-5k}"
              << std::endl;
    return 1;
  }

  boost::filesystem::path imdir(argv[2]);
  boost::filesystem::path gtdir(argv[3]);
  std::string tolerance_str(argv[4]);
  boost::filesystem::path tmpdir(argv[5]);
  Magick::InitializeMagick(*argv);

  double tolerance = std::stod(tolerance_str);
  std::cout << "mAP tolerance = " << tolerance << std::endl;

  const boost::filesystem::path visehome = vise::vise_home();
  std::map<std::string, std::string> vise_conf;
  vise::init_default_vise_settings(vise_conf);
  ASSERT(!vise_conf.empty());
  vise_conf["vise-project-dir"] = tmpdir.string();
  vise_conf["nthread-indexing"] = "-1"; // use all available threads

  std::unique_ptr<vise::project_manager> pmanager(new vise::project_manager(vise_conf));
  std::string pname(test_id);
  vise::http_response last_http_response;

  // check if this project already exists
  if(pmanager->project_exists(pname)) {
    std::unordered_map<std::string, std::string> del_pname_list;
    del_pname_list[pname] = "1";
    pmanager->vise_project_delete(del_pname_list, last_http_response);
    std::cout << last_http_response.d_status_code << " " << last_http_response.d_status
              << " : " << last_http_response.d_payload << std::endl;
    ASSERT(last_http_response.d_status_code == 303); // redirect to settings page
  }

  // create a new project
  vise::http_response project_create_response;
  std::unordered_map<std::string, std::string> create_pname_list;
  create_pname_list["pname"] = pname;
  pmanager->vise_project_create(create_pname_list, last_http_response);
  std::cout << "pmanager->vise_project_create(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 303);
  bool load_success = pmanager->project_load(pname);
  ASSERT(load_success);

  // count number of image files in image folder
  // required to verify that all files were added
  uint32_t img_count = 0;
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator it(imdir); it!=end_itr; ++it) {
    if(boost::filesystem::is_regular_file(it->path())) {
      img_count++;
    }
  }
  std::cout << imdir << " has " << img_count << " files" << std::endl;

  // add images to project
  std::unordered_map<std::string, std::string> param
    {
     {"source_type", "local_folder"},
     {"source_loc", imdir.string()}
    };
  vise::http_response file_add_response;
  pmanager->project_file_add(pname, param, last_http_response);
  std::cout << "pmanager->project_file_add(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 303);
  ASSERT(pmanager->project_image_src_count(pname) == img_count);

  // set configuration
  pmanager->project_config_use_preset(pname, "preset_conf_manual", last_http_response);
  std::cout << "pmanager->project_config_use_preset(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 200);

  std::ostringstream config;
  config << "search_engine=relja_retrival\n";
  config << "use_root_sift=true\n";
  config << "sift_scale_3=true\n";
  config << "resize_dimension=-1\n";
  //config << "nthread-indexing=16\n";

  if(test_id == "oxford-buildings-100") {
    config << "bow_descriptor_count=-1\n";
    config << "bow_cluster_count=1000\n";
    config << "cluster_num_iteration=3\n";
    config << "hamm_embedding_bits=64\n";
  } else {
    if(test_id == "oxford-buildings-5k") {
      config << "bow_descriptor_count=1000000\n";
      config << "bow_cluster_count=10000\n";
      config << "cluster_num_iteration=5\n";
      config << "hamm_embedding_bits=32\n";
    } else {
      std::cout << "WARNING: Unknown test_id=" << test_id << std::endl;
    }
  }
  std::string config_str(config.str());
  pmanager->project_config_save(pname, config_str, last_http_response);
  std::cout << "pmanager->project_config_save(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 200);

  // start the indexing process
  vise::http_response index_create_response;
  bool block_until_done = true;
  std::cout << "Creating index ... (blocking until done)" << std::endl;
  pmanager->project_index_create(pname, last_http_response, block_until_done);
  std::cout << "pmanager->project_index_create(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(pmanager->project_index_is_done(pname));

  vise::http_response index_unload_response;
  pmanager->project_index_unload(pname, last_http_response);
  std::cout << "pmanager->project_index_inload(): "
            << last_http_response.d_status_code << " " << last_http_response.d_status << std::endl;
  ASSERT(last_http_response.d_status_code == 200);
  ASSERT(!pmanager->project_index_is_loaded(pname));

  // tests for HTTP GET request handler
  std::cout << "testing get request handler" << std::endl;
  pmanager->process_http_request(create_http_request("GET", pname, "image"),
                                 last_http_response);
  ASSERT(last_http_response.d_status_code == 404);

  pmanager->process_http_request(create_http_request("GET", pname, "image/"),
                                 last_http_response);
  ASSERT(last_http_response.d_status_code == 404);

  pmanager->process_http_request(create_http_request("GET", pname, "image/all_souls_000002.jpg"),
                                 last_http_response);
  ASSERT(last_http_response.d_status_code == 200);

  // pmanager is no longer required, free resources
  std::cout << "freeing resources held by project_manager"
            << std::endl;
  pmanager.reset(nullptr);

  // evaluate performance
  std::cout << "evaluating performance ..."
            << std::endl;
  boost::filesystem::path project_dir(tmpdir);
  project_dir = project_dir / test_id;

  boost::filesystem::path pdata_dir = project_dir / "data";
  boost::filesystem::path img_dir = project_dir / "image";
  boost::filesystem::path pconf_fn = pdata_dir / "conf.txt";
  std::map<std::string, std::string> pconf;
  vise::configuration_load(pconf_fn.string(), pconf);
  ASSERT(!pconf.empty());

  boost::filesystem::path dset_fn = pdata_dir / "index_dset.bin";
  boost::filesystem::path iidx_fn = pdata_dir / "index_iidx.bin";
  boost::filesystem::path fidx_fn = pdata_dir / "index_fidx.bin";
  boost::filesystem::path weight_fn = pdata_dir / "weight.bin";
  boost::filesystem::path hamm_fn = pdata_dir / "trainhamm.bin";

  datasetV2 dset(dset_fn.string(), img_dir.string() );
  evaluator_oxford evalObj( gtdir.string(), &dset );

  protoDbFile dbFidx_file(fidx_fn.string());
  protoDbInRam dbFidx(dbFidx_file);
  protoIndex fidx(dbFidx, false);

  protoDbFile dbIidx_file(iidx_fn.string());
  protoDbInRam dbIidx(dbIidx_file);
  protoIndex iidx(dbIidx, false);

  tfidfV2 tfidfObj(&iidx, &fidx, weight_fn.string());

  uint32_t hamm_embedding_bits = 0;
  std::istringstream ss(pconf.at("hamm_embedding_bits"));
  ss >> hamm_embedding_bits;

  hammingEmbedderFactory embFactory(hamm_fn.string(), hamm_embedding_bits);
  hamming hammingObj(tfidfObj, &iidx, embFactory, &fidx);
  spatialVerifV2 spatVerifHamm(hammingObj, &iidx, &fidx, true);
  double mAP_hammsp= evalObj.computeMAP( spatVerifHamm, NULL, true, true, 1 );
  printf("mAP_hamm+sp= %.4f\n", mAP_hammsp);

  std::chrono::steady_clock::time_point tend = std::chrono::steady_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart);
  std::cout << "Test completed in " << ms.count() << "ms ("
            << (ms.count() / 1000) << " sec.)" << std::endl;

  double error = abs(mAP_hammsp - 0.8272);
  ASSERT(error < tolerance);
}
