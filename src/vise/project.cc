#include "project.h"

// preset_conf_1 : Indexing process is fast and disk space usage is low but visual search is less accurate.
// preset_conf_2 : More accurate visual search but indexing process uses more disk space and takes longer to complete.
// preset_conf_auto: Select configuration automatically (before selecting this option, add all the images to this project
// preset_conf_manual: Manually set all options (only for advanced users)
const std::vector<std::string> vise::project::d_preset_name_list = {
                                                                    "preset_conf_1",
                                                                    "preset_conf_2",
//                                                                    "preset_conf_auto",
                                                                    "preset_conf_manual" };

// used by vise-cli and others to initialize a new project using the
// provided configuration filename
vise::project::project(std::string pname,
                       std::string pconf_fn)
  : d_pname(pname),
    d_pconf_fn(pconf_fn),
    d_state(project_state::UNKNOWN),
    d_is_index_load_ongoing(false),
    d_is_metadata_ready(false),
    d_app_dir_exists(false),
    d_vgroup_task_progress_table("vgroup_task_progress"),
    d_vgroup_match_table("vgroup_match"),
    d_vgroup_non_match_table("vgroup_non_match"),
    d_vgroup_metadata_table("vgroup_metadata"),
    d_vgroup_region_table("vgroup_region"),
    d_vgroup_table("vgroup"),
    d_vgroup_inv_table("vgroup_inv")
{
  d_project_dir = boost::filesystem::path(d_pconf_fn).parent_path().parent_path();
  if(d_pconf_fn.filename() != "conf.txt") {
    std::cout << "PRECONDITION FAILED: configuration filename must be conf.txt" << std::endl;
    d_state = project_state::INIT_FAILED;
    return;
  }
  if(!boost::filesystem::exists(d_pconf_fn)) {
    std::cout << "configuration file " << d_pconf_fn.string() << " not found"
	      << std::endl;
    d_state = project_state::INIT_FAILED;
    return;
  }

  bool success = false;
  success = vise::configuration_load(d_pconf_fn.string(), d_pconf);
  if(success) {
    bool create_data_dir_if_missing = true;
    bool data_dir_created = init_project_data_dir(create_data_dir_if_missing);
    if(!data_dir_created) {
      d_state = project_state::INIT_FAILED;
      std::cout << "project(): failed to initialize project's data directories" << std::endl;
      return;
    }
  } else {
    d_state = project_state::INIT_FAILED;
    std::cout << "project(): failed to load project configuration file from "
              << d_pconf_fn.string() << std::endl;
    return;
  }

  std::cout << "Assuming that the VISE project folder is organized as follows: " << std::endl
            << "  - project base directory     : " << d_project_dir.string() << std::endl
            << "  - project configuration file : " << d_pconf_fn.string() << std::endl
            << "  - project images directory   : " << d_image_dir.string() << std::endl;

  success = false;
  std::string message;
  search_engine_init(d_pconf.at("search_engine"), success, message);
  if(!success) {
    d_state = project_state::INIT_FAILED;
    std::cout << "project(): failed to initialize search engine" << std::endl;
    return;
  }
  state_update();

  if(d_state == vise::project_state::SEARCH_READY) {
    d_metadata = std::unique_ptr<vise::metadata>(new vise::metadata(d_pname, d_data_dir));
    if(d_metadata->is_metadata_available()) {
      d_is_metadata_ready = true;
    }

    // load the name of groups allowed to be queried
    init_vgroup_id_list();
  }
}

// used by the web-ui to create a new project based only on name of the
// project supplied by the user
vise::project::project(std::string pname,
                       std::map<std::string, std::string> const &vise_conf)
  : d_pname(pname),
    d_conf(vise_conf),
    d_state(project_state::UNKNOWN),
    d_is_index_load_ongoing(false),
    d_is_metadata_ready(false),
    d_app_dir_exists(false),
    d_vgroup_task_progress_table("vgroup_task_progress"),
    d_vgroup_match_table("vgroup_match"),
    d_vgroup_metadata_table("vgroup_metadata"),
    d_vgroup_region_table("vgroup_region"),
    d_vgroup_table("vgroup"),
    d_vgroup_inv_table("vgroup_inv")
{
  std::cout << "project(): constructing " << pname << " ..."
            << std::endl;
  d_project_dir = boost::filesystem::path(d_conf.at("vise-project-dir")) / pname;
  d_pconf_fn = d_project_dir / "data";
  d_pconf_fn = d_pconf_fn / "conf.txt";
  bool save_conf = false;
  if( boost::filesystem::exists(d_project_dir) &&
      boost::filesystem::exists(d_pconf_fn) ) {
    // this is an existing project, load configuration
    vise::configuration_load(d_pconf_fn.string(), d_pconf);
  } else {
    if( boost::filesystem::exists(d_project_dir) ) {
      // this is a new project
      // - create standard folder structure (e.g. $PROJECT_DIR/data, $PROJECT_DIR/image, ...)
      // - create default configuration file
      // -  save configuration
      init_default_conf();
      save_conf = true;
    } else {
      d_state = project_state::INIT_FAILED;
      std::cout << "project(): project_dir=" << d_project_dir
                << "does not exist." << std::endl;
      return;
    }
  }

  // define and create default data directories if the configuration file
  // does not contain definition of d_data_dir, d_image_dir, d_image_src_dir, etc.
  bool create_data_dir_if_missing = true;
  bool data_dir_created = init_project_data_dir(create_data_dir_if_missing);
  if(!data_dir_created) {
    d_state = project_state::INIT_FAILED;
    std::cout << "project(): failed to initialize project's data directories" << std::endl;
    return;
  }

  if(save_conf) {
    vise::configuration_save(d_pconf, d_pconf_fn.string());
  }

  bool success;
  std::string message;
  search_engine_init(d_pconf.at("search_engine"), success, message);
  if(!success) {
    d_state = project_state::INIT_FAILED;
    std::cerr << "project(): search_engine_init failed: " << message << std::endl;
    return;
  }

  state_update();
  if(d_state == vise::project_state::SET_CONFIG) {
    if(d_pconf.count("preset_conf_id")) {
      use_preset_conf(d_pconf.at("preset_conf_id"));
    } else {
      use_preset_conf_1(); // default configuration is preset_conf_1
    }
    if( ! vise::configuration_save(d_pconf, d_pconf_fn.string()) ) {
      std::cout << "error: failed to save configuration " << d_pconf_fn << std::endl;
      d_state = project_state::INIT_FAILED;
    }
  } else {
    if(d_state == vise::project_state::SEARCH_READY) {
      d_metadata = std::unique_ptr<vise::metadata>(new vise::metadata(d_pname, d_data_dir));
      if(d_metadata->is_metadata_available()) {
        d_is_metadata_ready = true;
      }

      // load the name of groups allowed to be queried
      init_vgroup_id_list();
    }
  }
}

vise::project::~project() {
  if(index_is_loaded()) {
    std::cout << "~project(): unloading index for ["
              << d_pname << "] ..." << std::endl;
    bool success;
    std::string message;
    index_unload(success, message);
  }
}

vise::project_state vise::project::state() const {
  return d_state;
}

void vise::project::state(vise::project_state new_state) {
  std::cout << "project::state() : switching state from "
            << state_id_to_name(d_state) << " to "
            << state_id_to_name(new_state) << std::endl;
  d_state = new_state;
}

std::string vise::project::state_name() const {
  return state_id_to_name(d_state);
}

std::string vise::project::state_id_to_name(vise::project_state state) const {
  std::string name;
  switch(state) {
  case vise::project_state::UNKNOWN:
    name = "UNKNOWN";
    break;
  case vise::project_state::INIT_FAILED:
    name = "INIT_FAILED";
    break;
  case vise::project_state::SET_CONFIG:
    name = "SET_CONFIG";
    break;
  case vise::project_state::INDEX_ONGOING:
    name = "INDEX_ONGOING";
    break;
  case vise::project_state::BROKEN_INDEX:
    name = "BROKEN_INDEX";
    break;
  case vise::project_state::SEARCH_READY:
    name = "SEARCH_READY";
    break;
  default:
    name = "unrecognized state name";
    break;
  }
  return name;
}

void vise::project::state_update() {
  // check everything and ascertain the current state of a project
  bool success;
  std::string message;
  if(index_is_done()) {
    index_load(success, message);
    if(success) {
      state(project_state::SEARCH_READY);
    } else {
      std::cout << "index_load(): failed with message=" << message << std::endl;
      state(project_state::BROKEN_INDEX);
    }
  } else {
    if(index_is_ongoing()) {
      state(project_state::INDEX_ONGOING);
    } else {
      if(index_is_incomplete()) {
        state(project_state::BROKEN_INDEX);
      } else {
        state(project_state::SET_CONFIG);
      }
    }
  }
}

//
// conf
//
void vise::project::init_default_conf() {
  d_pconf.clear();
  d_pconf["project_name"] = d_pname;
  d_pconf["search_engine"] = "relja_retrival";
  d_pconf["use_root_sift"] = "true";
  d_pconf["sift_scale_3"] = "true";
  d_pconf["bow_descriptor_count"] = "-1";
  d_pconf["cluster_num_iteration"] = "30";
  d_pconf["bow_cluster_count"] = "100000";
  d_pconf["hamm_embedding_bits"] = "64";
  d_pconf["resize_dimension"] = "-1";
}

bool vise::project::conf_reload() {
  if(!boost::filesystem::exists(d_pconf_fn)) {
    return false;
  }
  if(!vise::configuration_load(d_pconf_fn.string(), d_pconf)) {
    return false;
  }
  bool create_data_dir_if_missing = false;
  if(!init_project_data_dir(create_data_dir_if_missing)) {
    return false;
  }

  return true;
}

void vise::project::conf_to_json(std::ostringstream &json) {
  if(d_pconf.size()) {
    json << "{";
    std::map<std::string, std::string>::const_iterator itr = d_pconf.begin();
    //std::cout << itr->first << ":" << itr->second << std::endl;
    json << "\"" << itr->first << "\":\"" << json_escape_str(itr->second) << "\"";
    ++itr;
    for(; itr != d_pconf.end(); ++itr) {
      //std::cout << itr->first << ":" << itr->second << std::endl;
      json << ",\"" << itr->first << "\":\"" << json_escape_str(itr->second) << "\"";
    }
    json << "}";
  } else {
    json << "{}";
  }
}

bool vise::project::conf_from_plaintext(std::string plaintext) {
  try {
    d_pconf.clear();
    std::istringstream ss(plaintext);
    std::string line;
    while(std::getline(ss, line)) {
      std::size_t eqpos = line.find('=');
      if(eqpos != std::string::npos) {
        std::string key = line.substr(0, eqpos);
        std::string val = line.substr(eqpos+1);
        d_pconf[key] = val;
      }
    }
    bool create_dir_if_missing = true;
    if(!init_project_data_dir(create_dir_if_missing)) {
      return false;
    }
    bool result = vise::configuration_save(d_pconf, d_pconf_fn.string());
    return result;
  } catch(std::exception &ex) {
    return false;
  }
}

void vise::project::preset_conf_to_json(std::ostringstream &json) {
  json << "[";
  for(std::size_t i=0; i < d_preset_name_list.size(); ++i) {
    if(i!=0) {
      json << ",";
    }
    json << "\"" << d_preset_name_list.at(i) << "\"";
  }
  json << "]";
}

bool vise::project::use_preset_conf(std::string preset_conf_id) {
  std::cout << "using config preset: " << preset_conf_id << std::endl;
  if(preset_conf_id == "preset_conf_1") {
    return use_preset_conf_1();
  }
  if(preset_conf_id == "preset_conf_2") {
    use_preset_conf_2();
    return true;
  }
  if(preset_conf_id == "preset_conf_auto") {
    use_preset_conf_auto();
    return true;
  }
  if(preset_conf_id == "preset_conf_manual") {
    use_preset_conf_manual();
    return true;
  }

  return false;
}

bool vise::project::use_preset_conf_1() {
  std::cout << "use_preset_conf_1()" << std::endl;

  // load generic visual vocabulary configuration
  boost::filesystem::path generic_vvoc_dir(d_conf.at("vise-asset-dir"));
  generic_vvoc_dir = generic_vvoc_dir / "relja_retrival";
  generic_vvoc_dir = generic_vvoc_dir / "visual_vocabulary";
  generic_vvoc_dir = generic_vvoc_dir / "latest";
  boost::filesystem::path generic_vvoc_conf_fn = generic_vvoc_dir / "generic_visual_vocab_conf.txt";
  std::cout << "generic_vvoc_conf_fn=" << generic_vvoc_conf_fn << std::endl;
  std::map<std::string, std::string> generic_vvoc_conf;
  bool success = vise::configuration_load(generic_vvoc_conf_fn.string(), generic_vvoc_conf);
  if(!success) {
    return false;
  }

  init_default_conf();
  d_pconf["bow_descriptor_count"] = generic_vvoc_conf["bow_descriptor_count"];
  d_pconf["bow_cluster_count"] = generic_vvoc_conf["bow_cluster_count"];
  d_pconf["hamm_embedding_bits"] = generic_vvoc_conf["hamm_embedding_bits"];
  d_pconf["cluster_num_iteration"] = generic_vvoc_conf["cluster_num_iteration"];
  d_pconf["resize_dimension"] = generic_vvoc_conf["resize_dimension"];
  d_pconf["preset_conf_id"] = "preset_conf_1";

  // copy files corresponding to generic visual vocabulary
  if(!boost::filesystem::exists(generic_vvoc_dir / "bowcluster.bin") ||
     !boost::filesystem::exists(generic_vvoc_dir / "trainhamm.bin") ) {
    return false;
  }
  boost::filesystem::copy_file(generic_vvoc_dir / "bowcluster.bin",
                               d_data_dir / "bowcluster.bin",
                               boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(generic_vvoc_dir / "trainhamm.bin",
                               d_data_dir / "trainhamm.bin",
                               boost::filesystem::copy_option::overwrite_if_exists);

  success = vise::configuration_save(d_pconf, d_pconf_fn.string());
  return success;
}

void vise::project::remove_existing_visual_vocabulary() {
  if(boost::filesystem::exists(d_data_dir / "bowcluster.bin")) {
    boost::filesystem::remove(d_data_dir / "bowcluster.bin");
  }
  if(boost::filesystem::exists(d_data_dir / "trainhamm.bin")) {
    boost::filesystem::remove(d_data_dir / "trainhamm.bin");
  }
}

void vise::project::use_preset_conf_2() {
  std::cout << "use_preset_conf_2()" << std::endl;
  remove_existing_visual_vocabulary();

  init_default_conf();
  d_pconf["bow_descriptor_count"] = "18000000"; // max 18 million descriptors
  d_pconf["bow_cluster_count"] = "0";           // select automatically
  d_pconf["hamm_embedding_bits"] = "64";
  d_pconf["cluster_num_iteration"] = "10";       // select automatically
  d_pconf["preset_conf_id"] = "preset_conf_2";

  init_default_conf();
  uint32_t img_count = image_src_count();
  // assumption: each image contains 3000 descriptors
  d_pconf["bow_descriptor_count"] = "-1";
  if(img_count >= 1 && img_count < 100) {
    d_pconf["bow_cluster_count"] = "1000";
	d_pconf["cluster_num_iteration"] = "3";
  } else if(img_count > 100 && img_count < 300) {
	d_pconf["bow_cluster_count"] = "1500";
	d_pconf["cluster_num_iteration"] = "3";
  } else if(img_count > 300 && img_count < 1000) {
	d_pconf["bow_cluster_count"] = "5000";
	d_pconf["cluster_num_iteration"] = "4";
  } else if(img_count > 1000 && img_count < 3000) {
	d_pconf["bow_cluster_count"] = "10000";
    d_pconf["bow_descriptor_count"] = "5000000";
	d_pconf["cluster_num_iteration"] = "5";
  } else if(img_count > 3000 && img_count < 10000) {
	d_pconf["bow_cluster_count"] = "50000";
    d_pconf["bow_descriptor_count"] = "10000000";
	d_pconf["cluster_num_iteration"] = "7";
  } else {
	d_pconf["bow_cluster_count"] = "100000";
    d_pconf["bow_descriptor_count"] = "18000000";
	d_pconf["cluster_num_iteration"] = "10";
  }
  d_pconf["resize_dimension"] = "800x800";

  vise::configuration_save(d_pconf, d_pconf_fn.string());
}

void vise::project::use_preset_conf_auto() {
  remove_existing_visual_vocabulary();

  init_default_conf();
  uint32_t img_count = image_src_count();
  if(img_count < 500) {
    d_pconf["bow_descriptor_count"] = "-1";
  } else if(img_count < 4000) {
    d_pconf["bow_descriptor_count"] = "4000000";
  } else {
    d_pconf["bow_descriptor_count"] = "18000000";
  }
  if(img_count < 10000) {
    d_pconf["hamm_embedding_bits"] = "64";
  } else {
    d_pconf["hamm_embedding_bits"] = "32";
  }
  d_pconf["cluster_num_iteration"] = "0";
  d_pconf["bow_cluster_count"] = "0";
  d_pconf["resize_dimension"] = "800x800";
  d_pconf["preset_conf_id"] = "preset_conf_auto";

  vise::configuration_save(d_pconf, d_pconf_fn.string());
}

void vise::project::use_preset_conf_manual() {
  remove_existing_visual_vocabulary();
  init_default_conf();
  d_pconf["preset_conf_id"] = "preset_conf_manual";
  vise::configuration_save(d_pconf, d_pconf_fn.string());
}

bool vise::project::init_project_data_dir(bool create_data_dir_if_missing) {
  d_data_dir      = d_project_dir / "data/";
  d_image_dir     = d_project_dir / "image/";
  d_image_src_dir = d_project_dir / "image_src/";
  d_tmp_dir       = d_project_dir / "tmp/";
  d_app_dir       = d_project_dir / "app/";
  d_image_small_dir = d_project_dir / "image_small/";

  if(d_pconf.count("data_dir") == 1) {
    d_data_dir = boost::filesystem::path(d_pconf.at("data_dir"));
  }
  if(d_pconf.count("image_dir") == 1) {
    d_image_dir = boost::filesystem::path(d_pconf.at("image_dir"));
  }
  if(d_pconf.count("image_src_dir") == 1) {
    d_image_src_dir = boost::filesystem::path(d_pconf.at("image_src_dir"));
  }
  if(d_pconf.count("tmp_dir") == 1) {
    d_tmp_dir = boost::filesystem::path(d_pconf.at("tmp_dir"));
  }
  if(d_pconf.count("app_dir") == 1) {
    d_app_dir = boost::filesystem::path(d_pconf.at("app_dir"));
  }
  if(d_pconf.count("image_small_dir") == 1) {
    d_image_small_dir = boost::filesystem::path(d_pconf.at("image_small_dir"));
  }

  // convert the trailing path-separator to platform specific character
  d_data_dir.make_preferred();
  d_image_dir.make_preferred();
  d_image_src_dir.make_preferred();
  d_tmp_dir.make_preferred();

  // handle missing dirs
  if(!boost::filesystem::exists(d_image_small_dir)) {
    d_image_small_dir = d_image_dir;
  }
  if(boost::filesystem::exists(d_app_dir)) {
    d_app_dir_exists = true;
  }
  d_image_small_dir.make_preferred();
  d_app_dir.make_preferred();

  if(create_data_dir_if_missing) {
    try {
      boost::filesystem::create_directories(d_data_dir);
      boost::filesystem::create_directories(d_image_dir);
      boost::filesystem::create_directories(d_image_src_dir);
      boost::filesystem::create_directories(d_tmp_dir);
      return true;
    } catch(std::exception &ex) {
      std::cout << "project(): failed to create project data directories: "
                << ex.what() << std::endl;
      return false;
    }
  } else {
    if(!boost::filesystem::exists(d_data_dir) ||
       !boost::filesystem::exists(d_image_dir) ||
       !boost::filesystem::exists(d_image_src_dir) ||
       !boost::filesystem::exists(d_tmp_dir) ) {
      return false;
    } else {
      return true;
    }
  }
}

//
// search engine
//
void vise::project::search_engine_init(std::string search_engine_name,
                                       bool &success,
                                       std::string &message) {
  if(!d_search_engine) {
    if (d_pconf.at("search_engine") == "relja_retrival") {
      d_search_engine = std::unique_ptr<vise::relja_retrival>(new relja_retrival(d_pconf_fn, d_project_dir));
      success = true;
      message = "initialized relja_retrival";
    } else {
      success = false;
      message = "unknown search_engine";
    }
  } else {
    success = true;
    message = "search_engine already initialized";
  }
}

void vise::project::index_create(bool &success,
                                 std::string &message,
                                 bool block_until_done) {
  std::lock_guard<std::mutex> lock(d_index_mutex);

  try {
    search_engine_init(d_pconf.at("search_engine"), success, message);
    if (success) {
      d_search_engine->conf(d_pconf); // required as the settings may have changed
      d_search_engine->index_create(success,
                                    message,
                                    std::bind( &vise::project::state_update, this),
                                    block_until_done);
    }
  } catch(std::exception &e) {
    success = false;
    message = e.what();
  }
}

void vise::project::index_load(bool &success, std::string &message) {
  std::cout << "loading index of " << d_pname << std::endl;
  std::lock_guard<std::mutex> lock(d_index_load_mutex);
  if (!d_search_engine) {
    if(!conf_reload()) {
      success = false;
      message = "Failed to reload configuration";
    }
    search_engine_init(d_pconf.at("search_engine"), success, message);
    if(!success) {
      return;
    }
  }
  d_is_index_load_ongoing = true;
  d_search_engine->index_load(success, message);
  d_is_index_load_ongoing = false;
}

bool vise::project::index_load_is_ongoing() const {
  return d_is_index_load_ongoing;
}

void vise::project::index_unload(bool &success, std::string &message) {
  std::lock_guard<std::mutex> lock(d_index_load_mutex);
  if (d_search_engine) {
    std::cout << "d_search_engine->index_unload() ..." << std::endl;
    d_search_engine->index_unload(success, message);
  } else {
    success = false;
    message = "index not loaded yet.";
  }
}

std::string vise::project::index_status_to_json() {
  if (d_search_engine) {
    return d_search_engine->index_status();
  } else {
    return "{}";
  }
}

void vise::project::index_search(vise::search_query const &q,
                                 std::vector<vise::search_result> &r) const {
  if (d_search_engine) {
    d_search_engine->index_search(q, r);
  }
}

void vise::project::index_internal_match(vise::search_query const &q,
                                      uint32_t match_file_id,
                                      std::ostringstream &json) const {
  if (d_search_engine) {
    d_search_engine->index_internal_match(q, match_file_id, json);
  }
}

void vise::project::register_image(uint32_t file1_id, uint32_t file2_id,
                                   uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                   std::array<double, 9> &H) const {
  if (d_search_engine) {
    d_search_engine->register_image(file1_id, file2_id, x, y, width, height, H);
  }
}

bool vise::project::index_is_loaded() const {
  if (d_search_engine) {
    return d_search_engine->index_is_loaded();
  } else {
    std::cout << "project::index_is_loaded(): cannot query index state as search engine has not been initialized yet."
	      << std::endl;
    return false;
  }
}

bool vise::project::index_is_done() const {
  if (d_search_engine) {
    return d_search_engine->index_is_done();
  } else {
    std::cout << "project::index_is_done(): cannot query index state as search engine has not been initialized yet."
	      << std::endl;
    return false;
  }
}

bool vise::project::index_is_incomplete() const {
  if (d_search_engine) {
    return d_search_engine->index_is_incomplete();
  } else {
    std::cout << "project::index_is_incomplete(): cannot query index state as search engine has not been initialized yet."
	      << std::endl;
    return false;
  }
}

bool vise::project::index_is_ongoing() const {
  if (d_search_engine) {
    return d_search_engine->index_is_ongoing();
  } else {
    std::cout << "project::index_is_ongoing(): cannot query index state as search engine has not been initialized yet."
	      << std::endl;
    return false;
  }
}

//
// filelist
//

uint32_t vise::project::fid_count() const {
  if(d_search_engine) {
    return d_search_engine->fid_count();
  } else {
    return 0;
  }
}

uint32_t vise::project::fid(const std::string filename) const {
  if(d_search_engine) {
    return d_search_engine->fid(filename);
  } else {
    return 4294967295; //@todo: improve
  }
}

std::string vise::project::filename(const uint32_t fid) const {
  if(d_search_engine) {
    return d_search_engine->filename(fid);
  } else {
    return "";
  }
}

uint32_t vise::project::image_src_count() const {
  uint32_t count = 0;
  boost::filesystem::recursive_directory_iterator end_itr;
  for (boost::filesystem::recursive_directory_iterator it(d_image_src_dir); it!=end_itr; ++it) {
    if (boost::filesystem::is_regular_file(it->path())) {
      count++;
    }
  }
  return count;
}

bool vise::project::pconf_is_set(std::string key) {
  if(d_pconf.count(key)) {
    return true;
  } else {
    return false;
  }
}

std::string vise::project::pconf(std::string key) {
  if(d_pconf.count(key)) {
    return d_pconf.at(key);
  } else {
    if(key == "data_dir") {
      return d_data_dir.string();
    }
    if(key == "image_dir") {
      return d_image_dir.string();
    }
    if(key == "image_src_dir") {
      return d_image_src_dir.string();
    }
    if(key == "tmp_dir") {
      return d_tmp_dir.string();
    }
    if(key == "image_small_dir") {
      return d_image_small_dir.string();
    }
    if(key == "app_dir") {
      return d_app_dir.string();
    }

    std::ostringstream ss;
    ss << "_UNKNOWN_" << key;
    return ss.str();
  }
}

//
// filelist search
//
void vise::project::file_metadata_full_text_search(const std::string query, std::vector<uint32_t> &flist) const {
  if(d_is_metadata_ready) {
    d_metadata->file_metadata_full_text_search(query, flist);
  }
}

void vise::project::file_metadata_full_text_search_group_stat(const std::string query,
                                                              const std::string groupby,
                                                              std::map<std::string, uint32_t> &group_stat) const {
  if(d_is_metadata_ready) {
    d_metadata->file_metadata_full_text_search_group_stat(query, groupby, group_stat);
  }
}

void vise::project::file_metadata_as_json(const uint32_t file_id,
                                          std::ostringstream &json) const {
  if(d_is_metadata_ready) {
    d_metadata->file_metadata_as_json(file_id, json);
  } else {
    json << "{}";
  }
}

void vise::project::region_metadata_as_json(const uint32_t file_id,
                                            std::ostringstream &json) const {
  if(d_is_metadata_ready) {
    d_metadata->region_metadata_as_json(file_id, json);
  } else {
    json << "[]";
  }
}

void vise::project::metadata_conf_as_json(std::ostringstream &json) const {
  if(d_is_metadata_ready) {
    d_metadata->metadata_conf_as_json(json);
  } else {
    json << "{}";
  }
}

void vise::project::error_metadata_not_available() const {
  std::cout << "vise::metadata is not available" << std::endl;
}

void vise::project::file_attribute_name_list(std::vector<std::string> &file_attribute_name_list) const {
  if(d_is_metadata_ready) {
    d_metadata->file_attribute_name_list(file_attribute_name_list);
  }
}

void vise::project::metadata_group_stat(const std::string groupby,
                                        std::map<std::string, uint32_t> &group_stat) const {
  if(d_is_metadata_ready) {
    d_metadata->metadata_group_stat(groupby, group_stat);
  }
}

void vise::project::metadata_groupby(const std::string groupby,
                                     const std::string group,
                                     std::vector<uint32_t> &flist) const {
  if(d_is_metadata_ready) {
    d_metadata->metadata_groupby(groupby, group, flist);
  }
}

//
// search using image features (e.g. using external image)
//
void vise::project::extract_image_features(const std::string &image_data,
                                           std::string &image_features) const {
  if(index_is_loaded()) {
    return d_search_engine->extract_image_features(image_data, image_features);
  }
}
void vise::project::index_search_using_features(const std::string &image_features,
                                                      std::vector<vise::search_result> &r) const {
  if (d_search_engine) {
    d_search_engine->index_search_using_features(image_features, r);
  }
}


void vise::project::index_get_feature_match_details(const std::string &image_features,
                                                    const uint32_t match_file_id,
                                                    std::ostringstream &json) const {
  if (d_search_engine) {
    d_search_engine->index_feature_match(image_features, match_file_id, json);
  }
}

void vise::project::register_external_image(const std::string &image_data,
                                            uint32_t file2_id,
                                            std::array<double, 9> &H) const {
  if (d_search_engine) {
    d_search_engine->register_external_image(image_data, file2_id, H);
  }
}

//
// visual group
//
void vise::project::create_vgroup(const std::unordered_map<std::string, std::string> &params,
                                  const bool block_until_done,
                                  bool &success, std::string &message) const {
  if(d_state != vise::project_state::SEARCH_READY) {
    success = false;
    message = "project state must be SEARCH_READY for creating a visual group";
    return;
  }

  if (!d_search_engine) {
    success = false;
    message = "search engine not initialized yet";
    return;
  }

  if(params.count("vgroup-id") == 0) {
    success = false;
    message = "visual group id missing, define using --vgroup-id=VISUAL-GROUP-NAME";
    return;
  }

  std::string vgroup_id = params.at("vgroup-id");

  // select files that will be used a query
  std::vector<std::size_t> all_qid_list;
  std::string filename_like = "%";
  if(params.count("filename-like")) {
    filename_like = params.at("filename-like");
  }

  if(params.count("query-type") == 1 &&
     params.at("query-type") == "region") {
    d_metadata->select_rid_with_filename_like(filename_like,
                                              all_qid_list);
  } else {
    // default: query using full image
    d_metadata->select_fid_with_filename_like(filename_like,
                                              all_qid_list);
  }
  if(all_qid_list.size() == 0) {
    message = "no matching files or regions";
    success = false;
    return;
  }
  std::cout << "vise::project::" << d_pname << " : pattern "
            << filename_like << " matched " << all_qid_list.size()
            << " files or regions" << std::endl;

  // initialize group db tables and list of file_id that needs to be processed
  std::vector<std::size_t> todo_qid_list;
  std::unordered_map<std::string, std::string> vgroup_metadata(params);

  // check progress to see if we need to resume from previous state
  std::set<std::size_t> done_qid_list;
  get_vgroup_task_progress(vgroup_id, done_qid_list, success, message);
  if(!success) {
    return;
  }
  if(done_qid_list.size()) {
    for(std::size_t i=0; i<all_qid_list.size(); ++i) {
      if(done_qid_list.count(all_qid_list[i]) == 0) {
        todo_qid_list.push_back(all_qid_list[i]);
      }
    }
    if(todo_qid_list.size()) {
      std::cout << "vise::project::" << d_pname << " : resuming computations "
                << "from file_id=" << todo_qid_list.at(0) << "(" << done_qid_list.size()
                << " already processed, " << todo_qid_list.size()
                << " remaining)" << std::endl;
    }
  } else {
    // start from beginning
    std::cout << "vise::project::" << d_pname << " : create_vgroup() "
              << "is starting from the beginning" << std::endl;
    todo_qid_list = all_qid_list;
    init_vgroup_db(vgroup_id, vgroup_metadata, success, message);
    if(!success) {
      return;
    }
  }

  // create match graph
  if(todo_qid_list.size()) {
    vgroup_match_graph(vgroup_id, vgroup_metadata, todo_qid_list, success, message);
    if(!success) {
      return;
    }
  } else {
    std::cout << "vise::project:: full visual group match graph already exists"
              << std::endl;
  }

  // find connected components of match graph
  vgroup_connected_components(vgroup_id, vgroup_metadata, success, message);
}


void vise::project::init_vgroup_id_list() {
  d_vgroup_id_list.clear();

  if(d_pconf.count("visual-group-id-list")) {
    std::vector<std::string> vgroup_id_list = vise::split(d_pconf.at("visual-group-id-list"), ',');
    std::ostringstream ss;
    for(std::size_t i=0; i<vgroup_id_list.size(); ++i) {
      std::string vgroup_id(vgroup_id_list.at(i));
      std::string message;
      bool success;
      is_visual_group_valid(vgroup_id, success, message);
      if(success) {
        d_vgroup_id_list.insert(vgroup_id);
        ss << vgroup_id << ",";

        // pre-load visual group metadata
        std::unordered_map<std::string, std::string> vgroup_metadata;
        load_vgroup_metadata(vgroup_id, vgroup_metadata);
        d_vgroup_metadata_list[vgroup_id] = vgroup_metadata;
      } else {
        std::cout << "vise::project : DISCARD group " << vgroup_id << ", REASON="
                  << message << std::endl;
      }
    }
    std::cout << "vise::project : initialized following " << d_vgroup_id_list.size()
              << " visual groups: " << ss.str() << std::endl;
  }
}

void vise::project::load_vgroup_metadata(const std::string vgroup_id,
                                         std::unordered_map<std::string, std::string> &vgroup_metadata) const {
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;

  sqlite3 *db = nullptr;
  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READONLY,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    std::cout << "failed to load metadata for visual group ["
              << vgroup_id << "] database" << std::endl;
    sqlite3_close_v2(db);
    return;
  }

  std::string sql = "SELECT * FROM `" + d_vgroup_metadata_table + "`";
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    std::cout << "failed to query metadata table" << std::endl;
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  if(rc != SQLITE_ROW) {
    std::cout << "failed to load visual group metadata" << std::endl;
    sqlite3_close_v2(db);
    return;
  }
  int ncols = sqlite3_column_count(stmt);
  vgroup_metadata.clear();
  for(std::size_t i=0; i<ncols; ++i) {
    std::ostringstream ss;
    ss << sqlite3_column_text(stmt, i);
    std::string value = ss.str();
    ss.str("");
    ss << sqlite3_column_name(stmt, i);
    std::string key = ss.str();
    vgroup_metadata[key] = value;
  }
  sqlite3_finalize(stmt);
  sqlite3_close_v2(db);
}

void vise::project::is_visual_group_valid(const std::string vgroup_id,
                                          bool &success,
                                          std::string &message) const {
  // initialize sqlite db
  sqlite3 *db = nullptr;
  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READONLY,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    success = false;
    message = "failed to load visual group database";
    sqlite3_close_v2(db);
    return;
  }

  std::ostringstream ss;
  ss << "SELECT COUNT(type) from sqlite_master where type='table' and name IN ("
     << "'" << d_vgroup_task_progress_table << "',"
     << "'" << d_vgroup_match_table << "',"
     << "'" << d_vgroup_metadata_table << "',"
     << "'" << d_vgroup_table << "',"
     << "'" << d_vgroup_inv_table << "');";
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;
  std::string sql(ss.str());
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    success = false;
    message = "failed to retrieve details about tables from visual group database";
    return;
  }
  rc = sqlite3_step(stmt);
  int ncols = sqlite3_column_count(stmt);
  if(ncols != 1) {
    sqlite3_close_v2(db);
    success = false;
    message = "malformed group database";
    return;
  }

  unsigned int table_count = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  if(table_count == 5) {
    success = true;
    message = "found all required tables in visual group database";
  } else {
    success = false;
    message = "missing required tables in visual group database";
  }
  sqlite3_close_v2(db);
}

std::string vise::project::get_vgroup_db_filename(const std::string vgroup_id) const {
  boost::filesystem::path vgroup_db_fn;
  if(vgroup_id.size()) {
    vgroup_db_fn = d_data_dir / (vgroup_id + ".sqlite");
  } else {
    vgroup_db_fn = d_data_dir / "unknown_group.sqlite";
    std::cout << "relja_retrival:: get_vgroup_db_filename() got empty group_id, "
              << "so using vgroup_db_filename = " << vgroup_db_fn
              << std::endl;
  }
  return vgroup_db_fn.string();
}

void vise::project::get_vgroup_task_progress(const std::string vgroup_id,
                                             std::set<std::size_t> &query_id_list,
                                             bool &success,
                                             std::string &message) const {
  query_id_list.clear();

  sqlite3 *db = nullptr;
  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  if(!boost::filesystem::exists( boost::filesystem::path(vgroup_db_fn))) {
    success = true;
    message = "visual group database does not yet exist";
    return;
  }

  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READONLY,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    success = false;
    message = "failed to open existing visual group database file";
    return;
  }

  int rc;
  std::string sql;
  sqlite3_stmt *stmt;
  const char *tail;

  // check if table exists
  sql = "SELECT COUNT(type) from sqlite_master where type='table' and name='" + d_vgroup_task_progress_table + "';";
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    success = false;
    message = "failed to query database";
    return;
  }
  rc = sqlite3_step(stmt);
  unsigned int table_count = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  if(table_count == 0) {
    success = true;
    message = "progress from previous session does not exist";
    return;
  }

  // fetch match progress
  sql = "SELECT `query_id` FROM " + d_vgroup_task_progress_table;
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    success = false;
    message = "failed to read existing " + d_vgroup_task_progress_table;
    return;
  }
  rc = sqlite3_step(stmt);
  while(rc == SQLITE_ROW) {
    query_id_list.insert(sqlite3_column_int(stmt, 0));
    rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);
  if(sqlite3_close_v2(db) == SQLITE_OK) {
    success = true;
    message = "retrieved visual group task progress data";
  } else {
    success = false;
    message = "failed to retrive visual group task progress data";
  }
}

void vise::project::init_vgroup_db(const std::string vgroup_id,
                                   std::unordered_map<std::string, std::string> &vgroup_metadata,
                                   bool &success,
                                   std::string &message) const {

  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  sqlite3 *db = nullptr;
  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    success = false;
    message = "failed to initialize visual group database";
    return;
  }

  int rc;
  char *err_msg;
  std::string sql;
  std::ostringstream ss;
  ss << "BEGIN TRANSACTION;"
     << "CREATE TABLE IF NOT EXISTS `" << d_vgroup_match_table << "`("
     << "`query_id` INTEGER NOT NULL, "
     << "`match_id` INTEGER NOT NULL, "
     << "`score` REAL NOT NULL, "
     << "`H` TEXT NOT NULL);"
     << "CREATE TABLE IF NOT EXISTS `" << d_vgroup_task_progress_table
     << "`(`query_id` INTEGER PRIMARY KEY, "
     << "`match_time_sec` REAL);";
  sql = ss.str();
  rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);

  if(vgroup_metadata.size()) {
    std::ostringstream metadata_table_sql;
    std::ostringstream metadata_row_sql;
    metadata_table_sql << "CREATE TABLE `" << d_vgroup_metadata_table << "`(";
    metadata_row_sql << "INSERT INTO `" << d_vgroup_metadata_table << "` VALUES(";
    std::unordered_map<std::string, std::string>::const_iterator itr = vgroup_metadata.begin();
    metadata_table_sql << "`" << itr->first << "` TEXT";
    metadata_row_sql << "'" << itr->second << "'";
    itr++;
    for(; itr!=vgroup_metadata.end(); ++itr) {
      metadata_table_sql << ",`" << itr->first << "` TEXT";
      metadata_row_sql << ",'" << itr->second << "'";
    }
    metadata_table_sql << ");";
    metadata_row_sql << ");";
    sql = metadata_table_sql.str();
    rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
    sql = metadata_row_sql.str();
    rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
  }

  // write everything
  sql = "END TRANSACTION";
  rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);

  if(rc != SQLITE_OK) {
    message = "failed to create tables in visual group database";
    success = false;
    if(err_msg != NULL) {
      sqlite3_free(err_msg);
    }
    sqlite3_close_v2(db);
    return;
  }

  if(vgroup_metadata.count("query-type") == 1 &&
     vgroup_metadata.at("query-type") == "region") {
    // insert existing regions from metadata_db
    std::string metadata_db_fn = d_metadata->get_metadata_db_fn();
    std::ostringstream ss;
    ss << "BEGIN TRANSACTION;"
       << "ATTACH DATABASE '" << metadata_db_fn << "' AS METADATA;"
       << "CREATE TABLE '" << d_vgroup_region_table << "' AS "
       << "SELECT region_id,file_id,region_index,region_shape,region_points "
       << "FROM METADATA.region_metadata" << ";"
       << "END TRANSACTION;";
    sql = ss.str();
    rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
    if(rc != SQLITE_OK) {
      message = "failed to import regions from metadata db";
      success = false;
      if(err_msg != NULL) {
        sqlite3_free(err_msg);
      }
    }
  }

  // create a table to store non-matches
  ss.str("");
  ss.clear();
  ss << "CREATE TABLE IF NOT EXISTS `" << d_vgroup_non_match_table << "`("
     << "`query_id` INTEGER NOT NULL, "
     << "`search_result_count` INTEGER NOT NULL, "
     << "`max_score` REAL NOT NULL);";
  sql = ss.str();
  rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);

  if(rc != SQLITE_OK) {
    message = "failed to create tables in visual group database";
    success = false;
    if(err_msg != NULL) {
      sqlite3_free(err_msg);
    }
    sqlite3_close_v2(db);
    return;
  }

  sqlite3_close_v2(db);
}

void vise::project::vgroup_match_graph(const std::string vgroup_id,
                                       const std::unordered_map<std::string, std::string> &vgroup_metadata,
                                       const std::vector<std::size_t> &query_id_list,
                                       bool &success,
                                       std::string &message) const {
  unsigned int nthread = 1;
  if(vgroup_metadata.count("nthread-search")) {
    nthread = std::stoi(vgroup_metadata.at("nthread-search"));
  } else {
    if(d_pconf.count("nthread-search")) {
      nthread = std::stoi(d_pconf.at("nthread-search"));
    }
  }

  std::size_t max_matches_count = 100;
  if(vgroup_metadata.count("max-matches")) {
    max_matches_count = std::stoi(vgroup_metadata.at("max-matches"));
  }
  float min_match_score = 10;
  if(vgroup_metadata.count("min-match-score")) {
    min_match_score = std::stof(vgroup_metadata.at("min-match-score"));
  }

  float match_iou_threshold = 0.9;
  if(vgroup_metadata.count("match-iou-threshold")) {
    match_iou_threshold = std::stof(vgroup_metadata.at("match-iou-threshold"));
  }

  std::cout << "vise::project:: create_vgroup_match_graph()"
            << " nthread-search=" << nthread
            << ", max-matches-count=" << max_matches_count
            << ", min-match-score=" << min_match_score
            << ", match-iou-threshold=" << match_iou_threshold
            << std::endl;

  sqlite3 *db = nullptr;
  std::string sql;
  int rc;
  char *err_msg;
  sqlite3_stmt *stmt;
  const char *tail;
  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    success = false;
    message = "failed to initialize visual group database";
    return;
  }

  // build a map of file_id and region_index
  std::unordered_map<std::size_t, int> file_id_region_index_map;
  std::unordered_map<std::size_t, std::set<std::size_t> > file_id_to_region_id_list;
  std::unordered_map<std::size_t, std::vector<float> > region_id_region_points_map;
  std::size_t next_region_id; // need for new regions added by vgroup

  if(vgroup_metadata.count("query-type") &&
     vgroup_metadata.at("query-type") == "region") {
    sql = "SELECT region_id, file_id, region_index, region_points FROM " + d_vgroup_region_table;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
    if(rc != SQLITE_OK) {
      success = false;
      message = "failed to query visual group region table";
      sqlite3_close_v2(db);
      return;
    }
    rc = sqlite3_step(stmt);
    std::size_t region_id = 0; // we need this to add new regions
    while(rc == SQLITE_ROW) {
      region_id = sqlite3_column_int(stmt, 0);
      std::size_t file_id = sqlite3_column_int(stmt, 1);
      std::size_t region_index = sqlite3_column_int(stmt, 2);
      file_id_region_index_map[file_id] = region_index;

      if(file_id_to_region_id_list.count(file_id) == 0) {
        file_id_to_region_id_list[file_id] = std::set<std::size_t>();
      }
      file_id_to_region_id_list[file_id].insert(region_id);

      std::ostringstream ss;
      ss << sqlite3_column_text(stmt, 3);
      std::vector<std::string> region_points = vise::split(ss.str(), ',');
      region_id_region_points_map[region_id] = std::vector<float>();
      for(std::size_t k=0; k<region_points.size(); ++k) {
        region_id_region_points_map[region_id].push_back( std::stof(region_points[k]) );
      }
      rc = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    next_region_id = region_id + 1;
  }

  unsigned int processed_query_count = 0;
  for(std::size_t qindex=0; qindex<query_id_list.size(); ++qindex) {
    std::size_t query_id = query_id_list.at(qindex);
    vise::search_query query;
    query.d_max_result_count = max_matches_count;
    if(vgroup_metadata.at("query-type") == "file") {
      // query using full file image
      query.d_file_id = query_id;
      query.is_region_query = false;
      std::cout << "file_id=" << query_id << " : ";
    } else {
      get_query_region(query_id, query);
      std::cout << "query (fid,rid)=(" << query.d_file_id << "," << query_id << ") : ";
    }

    sql = "BEGIN TRANSACTION";
    rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
    uint32_t tstart = vise::getmillisecs();
    std::vector<vise::search_result> search_result_list;
    d_search_engine->index_search(query, search_result_list);
    int match_count = 0;
    // first match corresponds to the query image (since, this is internal query)
    // so we ignore the first result
    for ( uint32_t i = 1; i < search_result_list.size(); ++i ) {
      float score = (float) search_result_list[i].d_score;
      if(score < min_match_score) {
        continue; // skip this match
      }
      std::size_t match_fid = search_result_list[i].d_file_id;;
      if(vgroup_metadata.at("query-type") == "file") {
        std::ostringstream ss;
        ss << "INSERT INTO `" << d_vgroup_match_table << "` VALUES("
           << query_id << "," << match_fid
           << "," << score << ","
           << "'" << search_result_list[i].H_to_csv() << "');";
        sql = ss.str();
        rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
        std::cout << "(" << match_fid << "," << score << ") ";
      } else {
        // create a new region entry in vgroup_region table
        // insert a reference to this new region in match table
        std::size_t new_region_index = 0;
        if(file_id_region_index_map.count(match_fid) == 1) {
          file_id_region_index_map[match_fid] = file_id_region_index_map[match_fid] + 1;
          new_region_index = file_id_region_index_map[match_fid];
        } else {
          file_id_region_index_map[match_fid] = new_region_index;
        }
        vise::search_query match_region;
        get_match_region(query, search_result_list[i], match_region);

        // check if match region overlaps with existing regions in that file
        bool is_match_region_new = true;
        std::size_t match_region_id = next_region_id;
        double match_region_iou = 0;
        std::set<std::size_t>::const_iterator itr = file_id_to_region_id_list[match_fid].begin();
        for(; itr!=file_id_to_region_id_list[match_fid].end(); ++itr) {
          std::size_t region_id = *itr;
          std::vector<float> region_points = region_id_region_points_map[region_id];
          std::vector<float> match_region_points;
          match_region.to_region_points(match_region_points);
          double match_iou = vise::iou(region_points, match_region_points);
          if(match_iou > match_iou_threshold) {
            is_match_region_new = false;
            match_region_id = region_id;
            match_region_iou = match_iou;
            break;
          }
        }
        std::ostringstream match_ss;
        match_ss << "INSERT INTO `" << d_vgroup_match_table << "` VALUES("
                 << query_id << "," << match_region_id
                 << "," << score << ","
                 << "'" << search_result_list[i].H_to_csv() << "');";
        sql = match_ss.str();
        rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
        if(is_match_region_new) {
          std::ostringstream region_ss;
          region_ss   << "INSERT INTO `" << d_vgroup_region_table << "`"
                      << "(region_id,file_id,region_index,region_shape,region_points) "
                      << "VALUES(" << next_region_id << "," << match_fid << ","
                      << new_region_index << ",'rect','"
                      << match_region.to_region_points_csv() << "')";
          sql = region_ss.str();
          rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
          file_id_to_region_id_list[match_fid].insert(next_region_id);
          region_id_region_points_map[next_region_id] = std::vector<float>();
          match_region.to_region_points(region_id_region_points_map[next_region_id]);
          std::cout << "(" << match_fid << "," << next_region_id << "*," << score << ") ";
          next_region_id = next_region_id + 1;
        } else {
          std::cout << "(" << match_fid << "," << next_region_id << "," << score << ") ";
        }
      }
      match_count = match_count + 1;
    }
    uint32_t tend = vise::getmillisecs();
    double telapsed = ((double) (tend - tstart)) / 1000.0;
    std::ostringstream progress;
    progress << "INSERT INTO `" << d_vgroup_task_progress_table
             << "` VALUES(" + std::to_string(query_id) << ","
             << std::to_string(telapsed) + ");";
    sql = progress.str();
    rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
    if(match_count == 0) {
      std::ostringstream non_match;
      non_match << "INSERT INTO `" << d_vgroup_non_match_table << "` "
                << "VALUES(" + std::to_string(query_id) << ","
                << (search_result_list.size() - 1) << ","; // discard first self match
      if(search_result_list.size() < 2) {
        non_match << "0";
      } else {
        float score = (float) search_result_list[1].d_score;
        non_match << score;
      }
      non_match << ");";
      sql = non_match.str();
      rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
    }
    sql = "END TRANSACTION";
    rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);

    if(rc != SQLITE_OK) {
      if(err_msg != NULL) {
        sqlite3_free(err_msg);
        message = "failed to insert data for query_id=" + std::to_string(query_id) + " into " + d_vgroup_match_table;
        success = false;
        sqlite3_close_v2(db);
        return;
      }
    }
    std::cout << std::endl;
    processed_query_count = processed_query_count + 1;
  }
  success = true;
  message = "done : processed " + std::to_string(processed_query_count) + " images";
  sqlite3_close_v2(db);
}

// create groups of visually similar images
// i.e. find all the connected components of the match graph
void vise::project::vgroup_connected_components(const std::string vgroup_id,
                                                const std::unordered_map<std::string, std::string> &vgroup_metadata,
                                                bool &success,
                                                std::string &message) const {
  std::cout << "vise::project:: computing connected components from match graph"
            << std::endl;
  uint32_t tstart = vise::getmillisecs();

  // initialize sqlite db
  std::string sql;
  int rc;
  char *err_msg;
  sqlite3_stmt *stmt;
  const char *tail;
  sqlite3 *db = nullptr;
  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READWRITE,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    success = false;
    message = "failed to load visual group database";
    sqlite3_close_v2(db);
    return;
  }

  // check if table exists
  sql = "SELECT COUNT(type) from sqlite_master where type='table' and name='" + d_vgroup_table + "';";
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    success = false;
    message = "failed to query visual group database";
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  unsigned int table_count = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);
  if(table_count == 1) {
    success = false;
    message = "connected components already computed";
    sqlite3_close_v2(db);
    return;
  }

  // create vgroup tables
  std::string query_type = "file";
  if(vgroup_metadata.count("query-type")) {
    query_type = vgroup_metadata.at("query-type");
  }

  std::ostringstream ss;
  ss << "BEGIN TRANSACTION;"
     << "CREATE TABLE `" << d_vgroup_table << "`("
     << "`set_id` INTEGER PRIMARY KEY, "
     << "`set_size` INTEGER NOT NULL, "
     << "`member_id_list` TEXT NOT NULL, "
     << "`query_id` INTEGER NOT NULL);"
     << "CREATE TABLE `" << d_vgroup_inv_table << "`("
     << "`member_id` INTEGER PRIMARY KEY, "
     << "`set_id` INTEGER NOT NULL);";
  ss << "END TRANSACTION;";
  sql = ss.str();
  rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);

  if(rc != SQLITE_OK) {
    success = false;
    message = "failed to create visual group tables";
    sqlite3_close_v2(db);
    return;
  }

  double vgroup_min_score = 0.0;
  if(vgroup_metadata.count("vgroup-min-score")) {
    vgroup_min_score = std::stof(vgroup_metadata.at("vgroup-min-score"));
  } else {
    // get default score threshold (if it exists)
    sql = "SELECT `vgroup-min-score` FROM " + d_vgroup_metadata_table + " LIMIT 1";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
    if(rc == SQLITE_OK) {
      rc = sqlite3_step(stmt);
      if(rc == SQLITE_ROW) {
        vgroup_min_score = sqlite3_column_double(stmt, 0);
      }
      sqlite3_finalize(stmt);
    }
  }
  std::unordered_map<std::size_t, std::set<std::size_t> > match_graph;
  std::unordered_map<std::size_t, uint8_t> vertex_flag;
  sql = "SELECT query_id,match_id,score from " + d_vgroup_match_table;
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    success = false;
    message = "failed to read table containing match graph edges";
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  while(rc == SQLITE_ROW) {
    std::size_t query_id = sqlite3_column_int(stmt, 0);
    std::size_t match_id = sqlite3_column_int(stmt, 1);
    double score = sqlite3_column_double(stmt, 2);
    if(score < vgroup_min_score) {
      rc = sqlite3_step(stmt);
      continue;
    }

    // insert undirected edge between query and match
    if(match_graph.find(query_id) == match_graph.end()) {
      match_graph[query_id] = std::set<std::size_t>();
    }
    if(match_graph.find(match_id) == match_graph.end()) {
      match_graph[match_id] = std::set<std::size_t>();
    }
    match_graph[query_id].insert(match_id);
    match_graph[match_id].insert(query_id);

    // maintain vertex list
    if(vertex_flag.count(query_id) == 0) {
      vertex_flag[query_id] = 0;
    }
    if(vertex_flag.count(match_id) == 0) {
      vertex_flag[match_id] = 0;
    }
    rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);

  std::cout << "vise::project:: vgroup-min-score=" << vgroup_min_score
            << ", match graph vertices=" << vertex_flag.size()
            << ", number of queries=" << match_graph.size()
            << std::endl;

  // perform depth first search to find all the connected components
  sql = "BEGIN TRANSACTION";
  rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
  std::unordered_map<std::size_t, std::set<std::size_t> >::const_iterator itr;
  std::size_t set_id = 0;
  std::unordered_map<std::size_t, std::set<std::size_t> > image_groups;
  for(itr=match_graph.begin(); itr!=match_graph.end(); ++itr) {
    std::size_t query_id = itr->first;

    if(vertex_flag[query_id] == 1) {
      continue; // discard already visited nodes
    }
    std::set<std::size_t> match_id_list(itr->second);
    std::set<std::size_t> visited_nodes;
    std::set<std::size_t>::const_iterator mitr = match_id_list.begin();
    for(; mitr != match_id_list.end(); ++mitr) {
      std::size_t match_id = *mitr;
      if(vertex_flag[match_id] == 1) {
        continue; // discard already visited nodes
      }
      vertex_flag[match_id] = 1;
      visited_nodes.insert(match_id);
      depth_first_search(match_graph, vertex_flag, match_id, visited_nodes);
    }
    if(visited_nodes.size() == 0) {
      continue; // discard empty components
    }
    //visited_nodes.push_back(query_id); // add query node
    image_groups[set_id] = visited_nodes;

    // save image group to database
    std::ostringstream image_group_row;
    std::ostringstream image_group_inv_rows;
    std::set<std::size_t>::const_iterator visited_nodes_itr = visited_nodes.begin();
    image_group_row << "INSERT INTO `" << d_vgroup_table << "` VALUES("
                    << set_id << "," << visited_nodes.size()
                    << ",'" << *visited_nodes_itr;
    image_group_inv_rows << "INSERT INTO `" << d_vgroup_inv_table << "` VALUES("
                         << *visited_nodes_itr << "," << set_id << ")";
    visited_nodes_itr++;
    for(; visited_nodes_itr != visited_nodes.end(); ++visited_nodes_itr) {
      image_group_row << "," << *visited_nodes_itr;
      image_group_inv_rows << ",(" << *visited_nodes_itr << "," << set_id << ")";
    }
    image_group_row << "'," << query_id << ");";
    image_group_inv_rows << ";";
    rc = sqlite3_exec(db, image_group_row.str().c_str(), NULL, NULL, &err_msg);
    rc = sqlite3_exec(db, image_group_inv_rows.str().c_str(), NULL, NULL, &err_msg);

    // move to next set
    set_id = set_id + 1;
  }
  sql = "END TRANSACTION";
  rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &err_msg);
  if(rc != SQLITE_OK) {
    std::cout << "vise::project:: visual group error (" << err_msg << ")" << std::endl;
    if(err_msg != NULL) {
      sqlite3_free(err_msg);
    }
  }
  sqlite3_close_v2(db);
  uint32_t tend = vise::getmillisecs();
  std::cout << "vise::project:: visual group contains " << set_id
            << " connected components (found in "
            << (tend - tstart) << " ms)"
            << std::endl;
}

void vise::project::depth_first_search(const std::unordered_map<std::size_t, std::set<std::size_t> > &match_graph,
                                       std::unordered_map<std::size_t, uint8_t> &vertex_flag,
                                       std::size_t vertex,
                                       std::set<std::size_t> &visited_nodes) const {
  if(match_graph.count(vertex) == 0) {
    return; // this vertex is not connected to any other nodes
  }

  std::set<std::size_t> match_file_id_list( match_graph.at(vertex) );
  std::set<std::size_t>::const_iterator mitr = match_file_id_list.begin();
  for(; mitr != match_file_id_list.end(); ++mitr) {
    std::size_t visited_vertex = *mitr;
    if(vertex_flag[visited_vertex] == 1) {
      continue; // discard visited vertices
    }
    vertex_flag[visited_vertex] = 1;
    visited_nodes.insert(visited_vertex);
    depth_first_search(match_graph, vertex_flag, visited_vertex, visited_nodes);
  }
}

void vise::project::get_query_region(const std::size_t query_id,
                                     vise::search_query &query) const {
  // query using region
  std::size_t file_id;
  std::size_t region_index;
  std::string region_shape;
  std::string region_points_str;
  d_metadata->get_region_shape(query_id,
                               file_id,
                               region_index,
                               region_shape,
                               region_points_str);
  if(region_shape != "rect") {
    std::cout << "ignoring unknown region shape type "
              << region_shape << std::endl;
    return;
  }
  std::vector<float> region_points;
  vise::csv_string_to_float_array(region_points_str, region_points);
  if(region_points.size() != 4) {
    std::cout << "ignoring malformed region shape: "
              << region_points_str << std::endl;
    return;
  }
  query.d_file_id = file_id;
  query.d_x = region_points[0];
  query.d_y = region_points[1];
  query.d_width = region_points[2];
  query.d_height = region_points[3];
  query.is_region_query = true;
}

void vise::project::get_match_region(const vise::search_query &query,
                                     const vise::search_result &result,
                                     vise::search_query &match_region) const {
  int qx1 = query.d_x;
  int qy1 = query.d_y;
  int qx2 = qx1 + query.d_width;
  int qy2 = qy1 + query.d_height;

  match_region.d_filename = result.d_filename;
  match_region.d_file_id = result.d_file_id;
  match_region.is_region_query = true;

  match_region.d_x = result.d_H[0] * qx1 + result.d_H[1] * qy1 + result.d_H[2];
  match_region.d_y = result.d_H[3] * qx1 + result.d_H[4] * qy1 + result.d_H[5];
  int mx2 = result.d_H[0] * qx2 + result.d_H[1] * qy2 + result.d_H[2];
  int my2 = result.d_H[3] * qx2 + result.d_H[4] * qy2 + result.d_H[5];
  match_region.d_width  = mx2 - match_region.d_x;
  match_region.d_height = my2 - match_region.d_y;
}

void vise::project::get_vgroup(const std::string vgroup_id,
                               std::unordered_map<std::string, std::string> const &param,
                               std::ostringstream &json) const {
  if(!is_vgroup_available(vgroup_id)) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"visual group [" << vgroup_id << "] not available\"}";
    return;
  }

  std::ostringstream sqlss;
  std::string sql;
  int rc;
  sqlite3_stmt *stmt;
  const char *tail;

  sqlite3 *db = nullptr;
  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READONLY,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to load database\"}";
    return;
  }

  // generate set size statistics
  std::map<std::size_t, std::size_t> set_size_stat;
  sqlss.clear();
  sqlss.str("");
  sqlss << "SELECT set_size, count(set_size) AS set_size_count FROM `"
        << d_vgroup_table << "` GROUP BY set_size;";
  sql = sqlss.str();
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to read table containing match graph edges\"}";
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  while(rc == SQLITE_ROW) {
    std::size_t set_size = sqlite3_column_int(stmt, 0);
    std::size_t set_size_count = sqlite3_column_int(stmt, 1);
    set_size_stat[set_size] = set_size_count;
    rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);
  if(set_size_stat.size() == 0) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"group is empty\"}";
    sqlite3_close_v2(db);
    return;
  }

  // find min/max set id
  sql = "SELECT MIN(set_id), MAX(set_id) FROM `" + d_vgroup_table + "`";
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to query database\"}";
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  if(rc != SQLITE_ROW) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to find min/max set_id\"}";
    sqlite3_close_v2(db);
    return;
  }
  std::size_t min_set_id = sqlite3_column_int(stmt, 0);
  std::size_t max_set_id = sqlite3_column_int(stmt, 1);
  sqlite3_finalize(stmt);

  // prepare data for each set
  unsigned int default_set_size = set_size_stat.begin()->first;
  unsigned int set_size = default_set_size;
  if(param.count("set_size")) {
    set_size = std::stoi(param.at("set_size"));
  }

  // gather all query_file_id which has the given "set_size" number of matches
  sqlss.clear();
  sqlss.str("");
  sqlss << "SELECT set_id, member_id_list FROM `" << d_vgroup_table
        << "` WHERE set_size = " << set_size
        << " ORDER BY set_id ASC LIMIT 20000"; // limit to prevent abuse
  sql = sqlss.str();
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to gather member_id_list with size=" << set_size << "\"}";
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  std::vector<std::size_t> set_id_list;
  std::vector<std::string> image_group_member_id_list;;
  while(rc == SQLITE_ROW) {
    std::size_t set_id = sqlite3_column_int(stmt, 0);
    std::ostringstream ss;
    ss << sqlite3_column_text(stmt, 1);
    set_id_list.push_back(set_id);
    image_group_member_id_list.push_back(ss.str());
    rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);

  // NOTE:
  // if query-type = "file"
  //   member_id_list -> file_id_list
  // if query-type = "region"
  //   member_id_list -> region_id_list
  std::vector<std::string> image_group_file_id_list_str;
  std::vector<std::string> image_group_region_points_list_str;
  if(d_vgroup_metadata_list.at(vgroup_id).count("query-type") &&
     d_vgroup_metadata_list.at(vgroup_id).at("query-type") == "region") {
    for(std::size_t i=0; i<image_group_member_id_list.size(); ++i) {
      sqlss.str("");
      sqlss << "SELECT region_id,file_id,region_points "
            << "FROM `" << d_vgroup_region_table << "` WHERE region_id IN ("
            << image_group_member_id_list.at(i) << ");";
      rc = sqlite3_prepare_v2(db, sqlss.str().c_str(), -1, &stmt, &tail);
      if(rc != SQLITE_OK) {
        json << "{\"STATUS\":\"error\",\"MESSAGE\":"
             << "\"failed to query database\"}";
        sqlite3_close_v2(db);
        return;
      }
      rc = sqlite3_step(stmt);
      std::ostringstream set_file_id_list_ss;
      std::ostringstream set_region_points_list_ss;
      set_file_id_list_ss << sqlite3_column_int(stmt,1);
      set_region_points_list_ss << "[" << sqlite3_column_text(stmt,2) << "]";
      rc = sqlite3_step(stmt);
      while(rc == SQLITE_ROW) {
        set_file_id_list_ss << "," << sqlite3_column_int(stmt,1);
        set_region_points_list_ss << ",[" << sqlite3_column_text(stmt,2) << "]";
        rc = sqlite3_step(stmt);
      }
      sqlite3_finalize(stmt);
      image_group_file_id_list_str.push_back(set_file_id_list_ss.str());
      image_group_region_points_list_str.push_back(set_region_points_list_ss.str());
    }
  } else {
    image_group_file_id_list_str = image_group_member_id_list;
  }
  sqlite3_close_v2(db);

  // pagination of set list
  std::size_t set_index_from = 0;
  const std::size_t MAX_SET_PER_PAGE = 10;
  std::size_t set_index_to = MAX_SET_PER_PAGE;
  if(param.count("from")) {
    set_index_from = std::stoi(param.at("from"));
  }
  if(param.count("to")) {
    set_index_to = std::stoi(param.at("to"));
  }

  if(set_index_from > set_id_list.size() ||
     set_index_to > set_id_list.size()) {
    set_index_from = 0;
    if(set_id_list.size() < MAX_SET_PER_PAGE) {
      set_index_to = set_id_list.size();
    } else {
      set_index_to = MAX_SET_PER_PAGE;
    }
  }
  if(set_index_to < set_index_from) {
    set_index_to = set_index_from + MAX_SET_PER_PAGE;
    if(set_index_to > set_id_list.size()) {
      set_index_to = set_id_list.size();
    }
  }

  // prepare response json
  std::map<std::size_t, std::size_t>::const_iterator itr = set_size_stat.begin();
  json << "{\"group_id\":\"" << vgroup_id << "\",\"set_size_stat\":{"
       << "\"" << itr->first << "\":" << itr->second;
  for(++itr; itr!=set_size_stat.end(); ++itr) {
    json << ",\"" << itr->first << "\":" << itr->second;
  }
  json << "},\"set_size\":" << set_size
       << ",\"SET\":{";

  std::ostringstream set_index_list_subset;
  for(std::size_t i=set_index_from; i<set_index_to; ++i) {
    std::size_t set_index = i;
    std::size_t set_id = set_id_list.at(set_index);
    if(i!=set_index_from) {
      json << ",";
      set_index_list_subset << ",";
    }
    set_index_list_subset << set_index;
    std::string file_id_list_str = image_group_file_id_list_str.at(i);
    json << "\"" << set_index << "\":{\"set_id\":" << set_id
         << ",\"file_id_list\":[" << file_id_list_str << "]";
    if(d_vgroup_metadata_list.at(vgroup_id).count("query-type") &&
       d_vgroup_metadata_list.at(vgroup_id).at("query-type") == "region") {
      json << ",\"region_points_list\":[" << image_group_region_points_list_str.at(i) << "]";
    }
    json << ",\"filename_list\":[";
    std::vector<std::string> file_id_list = vise::split(file_id_list_str, ',');
    json << "\"" << filename(std::stoi(file_id_list.at(0))) << "\"";
    for(std::size_t j=1; j<file_id_list.size(); ++j) {
      json << ",\"" << filename(std::stoi(file_id_list.at(j))) << "\"";
    }
    json << "]}";
  }
  json << "},\"set_index_list\":[" << set_index_list_subset.str()
       << "],\"set_index_range\":[0," << (set_id_list.size()) << "]"
       << ",\"set_index_from\":" << set_index_from
       << ",\"set_index_to\":" << set_index_to
       << ",\"set_id_range\":[" << min_set_id << "," << max_set_id << "]}";
}

void vise::project::get_vgroup_set(const std::string vgroup_id,
                                   const std::string set_id_str,
                                   std::ostringstream &json) const {
  if(!is_vgroup_available(vgroup_id)) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"visual group [" << vgroup_id << "] not available\"}";
    return;
  }

  int rc;
  sqlite3_stmt *stmt;
  const char *tail;

  sqlite3 *db = nullptr;
  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READONLY,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to load database\"}";
    return;
  }

  // find min/max set id
  std::string sql = "SELECT MIN(set_id), MAX(set_id) FROM `" + d_vgroup_table + "`";
  rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to query database\"}";
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  if(rc != SQLITE_ROW) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to find min/max set_id\"}";
    sqlite3_close_v2(db);
    return;
  }
  std::size_t min_set_id = sqlite3_column_int(stmt, 0);
  std::size_t max_set_id = sqlite3_column_int(stmt, 1);
  sqlite3_finalize(stmt);

  std::size_t set_id = std::stoi(set_id_str);
  if(set_id < min_set_id || set_id > max_set_id) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"set_id should be between " << min_set_id
         << " and " << max_set_id << "\"}";
    sqlite3_close_v2(db);
    return;
  }

  std::ostringstream sqlss;
  sqlss << "SELECT set_id, member_id_list FROM `"
        << d_vgroup_table << "` WHERE set_id=" << set_id
        << " LIMIT 1";
  rc = sqlite3_prepare_v2(db, sqlss.str().c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to query database\"}";
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  if(rc != SQLITE_ROW) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"set not found\"}";
    sqlite3_close_v2(db);
    return;
  }
  std::ostringstream member_id_list_ss;
  member_id_list_ss << sqlite3_column_text(stmt, 1);
  sqlite3_finalize(stmt);

  std::string file_id_list_str;
  std::string region_points_list_str;
  if(d_vgroup_metadata_list.at(vgroup_id).count("query-type") &&
     d_vgroup_metadata_list.at(vgroup_id).at("query-type") == "region") {
    sqlss.str("");
    sqlss << "SELECT region_id,file_id,region_points "
          << "FROM `" << d_vgroup_region_table << "` WHERE region_id IN ("
          << member_id_list_ss.str() << ");";
    rc = sqlite3_prepare_v2(db, sqlss.str().c_str(), -1, &stmt, &tail);
    if(rc != SQLITE_OK) {
      json << "{\"STATUS\":\"error\",\"MESSAGE\":"
           << "\"failed to query database\"}";
      sqlite3_close_v2(db);
      return;
    }
    rc = sqlite3_step(stmt);
    std::ostringstream file_id_list_ss;
    std::ostringstream region_points_list_ss;
    file_id_list_ss << sqlite3_column_int(stmt,1);
    region_points_list_ss << "[" << sqlite3_column_text(stmt,2) << "]";
    rc = sqlite3_step(stmt);
    while(rc == SQLITE_ROW) {
      file_id_list_ss << "," << sqlite3_column_int(stmt,1);
      region_points_list_ss << ",[" << sqlite3_column_text(stmt,2) << "]";
      rc = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    file_id_list_str = file_id_list_ss.str();
    region_points_list_str = region_points_list_ss.str();
  } else {
    file_id_list_str = member_id_list_ss.str();
  }
  sqlite3_close_v2(db);

  std::vector<std::string> file_id_list = vise::split(file_id_list_str, ',');
  if(file_id_list.size() == 0) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"set is empty\"}";
    return;
  }

  // prepare response
  json << "{\"group_id\":\"" << vgroup_id << "\""
       << ",\"set_id\":" << set_id
       << ",\"set_id_range\":[" << min_set_id << "," << max_set_id << "]"
       << ",\"file_id_list\":[" << file_id_list_str << "]"
       << ",\"region_points_list\":[" << region_points_list_str << "]"
       << ",\"filename_list\":[\""
       << filename(std::stoi(file_id_list.at(0))) << "\"";
  for(std::size_t i=1; i<file_id_list.size(); ++i) {
    json << ",\"" << filename(std::stoi(file_id_list.at(i))) << "\"";
  }
  json << "]}";
}

void vise::project::get_vgroup_set_with_file_id(const std::string vgroup_id,
                                                const std::string file_id_str,
                                                std::ostringstream &json) const {
  if(!is_vgroup_available(vgroup_id)) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"visual group [" << vgroup_id << "] not available\"}";
    return;
  }

  int rc;
  sqlite3_stmt *stmt;
  const char *tail;

  sqlite3 *db = nullptr;
  std::string vgroup_db_fn = get_vgroup_db_filename(vgroup_id);
  int sqlite_db_status = sqlite3_open_v2(vgroup_db_fn.c_str(),
                                         &db,
                                         SQLITE_OPEN_READONLY,
                                         NULL);
  if( sqlite_db_status != SQLITE_OK ) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to load database\"}";
    return;
  }

  // find all sets containing a file_id
  std::string query_type = "file";
  if(d_vgroup_metadata_list.at(vgroup_id).count("query-type") &&
     d_vgroup_metadata_list.at(vgroup_id).at("query-type") == "region") {
    query_type = "region";
  }

  std::ostringstream member_id_list_ss;
  std::ostringstream sqlss;
  if(query_type == "region") {
    // find all region-id belonging to the file-id
    std::vector<std::string> region_id_list;
    sqlss << "SELECT region_id FROM `"
          << d_vgroup_region_table << "` WHERE file_id =" << file_id_str
          << ";";
    rc = sqlite3_prepare_v2(db, sqlss.str().c_str(), -1, &stmt, &tail);
    if(rc != SQLITE_OK) {
      json << "{\"STATUS\":\"error\",\"MESSAGE\":"
           << "\"failed to query database\"}";
      sqlite3_close_v2(db);
      return;
    }
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW) {
      json << "{\"STATUS\":\"error\",\"MESSAGE\":"
           << "\"There are no sets that contains the file-id=" << file_id_str << "\"}";
      sqlite3_close_v2(db);
      return;
    }
    member_id_list_ss << sqlite3_column_int(stmt,0);
    rc = sqlite3_step(stmt);
    while(rc == SQLITE_ROW) {
      member_id_list_ss << "," << sqlite3_column_int(stmt,0);
      rc = sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
  } else {
    member_id_list_ss << file_id_str;
  }

  // find all the sets that contains this file-id
  std::vector<std::size_t> set_id_list;
  sqlss.str("");
  sqlss << "SELECT set_id FROM `"
        << d_vgroup_inv_table << "` WHERE member_id IN ("
        << member_id_list_ss.str() << ");";
  rc = sqlite3_prepare_v2(db, sqlss.str().c_str(), -1, &stmt, &tail);
  if(rc != SQLITE_OK) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"failed to query database\"}";
    sqlite3_close_v2(db);
    return;
  }
  rc = sqlite3_step(stmt);
  if(rc != SQLITE_ROW) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"There are no sets that contains the file-id=" << file_id_str << "\"}";
    sqlite3_close_v2(db);
    return;
  }
  while(rc == SQLITE_ROW) {
    set_id_list.push_back(sqlite3_column_int(stmt,0));
    rc = sqlite3_step(stmt);
  }
  sqlite3_finalize(stmt);
  sqlite3_close_v2(db);

  // prepare response
  json << "{\"group_id\":\"" << vgroup_id << "\""
       << ",\"file_id\":" << file_id_str
       << ",\"set_id_list\":[" << set_id_list.at(0);
  for(std::size_t i=1; i<set_id_list.size(); ++i) {
    json << "," << set_id_list.at(i);
  }
  json << "]}";
}

bool vise::project::is_vgroup_available(const std::string vgroup_id) const {
  if(d_vgroup_id_list.count(vgroup_id) == 1) {
    return true;
  } else {
    return false;
  }
}
