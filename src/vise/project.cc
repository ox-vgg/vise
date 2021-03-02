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
    d_app_dir_exists(false)
{
  d_project_dir = boost::filesystem::path(d_pconf_fn).parent_path().parent_path();
  std::cout << "PRECONDITION: " << std::endl
            << "- the project configuration file is saved as $PROJECT_DIR/data/conf.txt" << std::endl
            << "- $PROJECT_DIR=" << d_project_dir.string() << std::endl
            << "- $PROJECT_DIR is the folder in which VISE will store all the project's data" << std::endl;
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
    init_group_id_list();
  }
}

// used by the web-ui to create a new project based only on name of the
// project supplied by the user
vise::project::project(std::string pname,
                       std::map<std::string, std::string> const &vise_conf)
  : d_pname(pname), d_conf(vise_conf), d_state(project_state::UNKNOWN),
    d_is_index_load_ongoing(false),
    d_app_dir_exists(false)
{
  std::cout << "project(): constructing " << pname << " ..."
            << std::endl;
  d_project_dir = boost::filesystem::path(d_conf.at("vise_store")) / pname;
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
      // load the name of groups allowed to be queried
      init_group_id_list();
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
  boost::filesystem::path generic_vvoc_dir(d_conf.at("generic_visual_vocabulary"));
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
  }
}

void vise::project::region_metadata_as_json(const uint32_t file_id,
                                            std::ostringstream &json) const {
  if(d_is_metadata_ready) {
    d_metadata->region_metadata_as_json(file_id, json);
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
void vise::project::create_visual_group(const std::unordered_map<std::string, std::string> &params,
                                        bool &success, std::string &message,
                                        bool &block_until_done) const {
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

  d_search_engine->create_visual_group(params, success, message, block_until_done);
}

void vise::project::get_image_graph(std::map<std::string, std::string> const &param,
                                     std::ostringstream &json) const {
  if(d_state != vise::project_state::SEARCH_READY) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"project state must be SEARCH_READY for querying a visual group\"}";
    return;
  }

  if(!d_search_engine) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"search engine not initialized yet\"}";
    return;
  }

  if(param.count("group_id")) {
    std::string group_id(param.at("group_id"));
    if(d_group_id_list.count(group_id)) {
      d_search_engine->get_image_graph(group_id, param, json);
    } else {
      json << "{\"STATUS\":\"error\",\"MESSAGE\":"
           << "\"group " << group_id << " does not exist\"}";
    }
  } else {
    if(d_group_id_list.size()) {
      std::ostringstream ss;
      std::set<std::string>::const_iterator itr = d_group_id_list.begin();
      ss << "\"" << *itr << "\"";
      for(++itr; itr != d_group_id_list.end(); ++itr) {
        ss << ",\"" << *itr << "\"";
      }
      json << "{\"STATUS\":\"group_index\",\"MESSAGE\":"
           << "\"The following image graphs are available:\""
           << ",\"group_id_list\":[" << ss.str() << "]}";
    } else {
      json << "{\"STATUS\":\"error\",\"MESSAGE\":"
           << "\"image graphs are not available\"}";
      return;
    }
  }
}

void vise::project::get_image_group(std::map<std::string, std::string> const &param,
                                     std::ostringstream &json) const {
  if(d_state != vise::project_state::SEARCH_READY) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"project state must be SEARCH_READY for querying a visual group\"}";
    return;
  }

  if(!d_search_engine) {
    json << "{\"STATUS\":\"error\",\"MESSAGE\":"
         << "\"search engine not initialized yet\"}";
    return;
  }

  if(param.count("group_id")) {
    std::string group_id(param.at("group_id"));
    if(d_group_id_list.count(group_id)) {
      d_search_engine->get_image_group(group_id, param, json);
    } else {
      json << "{\"STATUS\":\"error\",\"MESSAGE\":"
           << "\"group " << group_id << " does not exist\"}";
    }
  } else {
    if(d_group_id_list.size()) {
      std::ostringstream ss;
      std::set<std::string>::const_iterator itr = d_group_id_list.begin();
      ss << "\"" << *itr << "\"";
      for(++itr; itr != d_group_id_list.end(); ++itr) {
        ss << ",\"" << *itr << "\"";
      }
      json << "{\"STATUS\":\"group_index\",\"MESSAGE\":"
           << "\"The following image groups are available:\""
           << ",\"group_id_list\":[" << ss.str() << "]}";
    } else {
      json << "{\"STATUS\":\"error\",\"MESSAGE\":"
           << "\"image graphs are not available\"}";
      return;
    }
  }
}

void vise::project::init_group_id_list() {
  d_group_id_list.clear();

  if(d_pconf.count("group_id_list")) {
    std::vector<std::string> group_id_list = vise::split(d_pconf.at("group_id_list"), ',');
    std::ostringstream ss;
    for(std::size_t i=0; i<group_id_list.size(); ++i) {
      std::string group_id(group_id_list.at(i));
      std::string message;
      bool success;
      d_search_engine->is_visual_group_valid(group_id, success, message);
      if(success) {
        d_group_id_list.insert(group_id);
        ss << group_id << ",";
      } else {
        std::cout << "vise::project : DISCARD group " << group_id << ", REASON="
                  << message << std::endl;
      }
    }
    std::cout << "vise::project : initialized following " << d_group_id_list.size()
              << " groups: " << ss.str() << std::endl;
  }
}
