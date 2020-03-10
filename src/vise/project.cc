#include "project.h"

vise::project::project(std::string pname,
                       std::map<std::string, std::string> const &conf)
  : d_pname(pname), d_conf(conf), d_state(project_state::UNKNOWN)
{

  std::cout << "project(): constructing " << pname << " ..."
            << std::endl;
  d_project_dir = boost::filesystem::path(d_conf.at("vise_store")) / pname;
  d_data_dir = d_project_dir / "data";
  if(conf_reload()) {
    if(d_pconf.count("data_dir") == 0 ||
       d_pconf.count("image_dir") == 0 ||
       d_pconf.count("image_src_dir") == 0 ||
       d_pconf.count("search_engine") == 0) {
      std::clog << "project(): malformed config file" << std::endl;
      return;
    }
    bool success;
    std::string message;
    search_engine_init(d_pconf.at("search_engine"), success, message);
    if(!success) {
      std::cerr << message << std::endl;
    }
  } else {
    std::clog << "failed to load configuration for " << pname << std::endl;
  }
  state_update();
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
      state(project_state::BROKEN_INDEX);
    }
  } else {
    if(index_is_incomplete()) {
      state(project_state::BROKEN_INDEX);
    } else {
      if(index_is_ongoing()) {
        state(project_state::INDEX_ONGOING);
      } else {
        state(project_state::SET_CONFIG);
      }
    }
  }
}

bool vise::project::conf_reload() {
  bool success = false;
  boost::filesystem::path pconf_fn = d_data_dir / "conf.txt";
  if(boost::filesystem::exists(pconf_fn)) {
    std::cout << "reading config: " << pconf_fn << std::endl;
    success = vise::configuration_load(pconf_fn.string(), d_pconf);
    if(success) {
      d_data_dir = boost::filesystem::path(d_pconf.at("data_dir"));
      d_image_dir = boost::filesystem::path(d_pconf.at("image_dir"));
      d_image_src_dir = boost::filesystem::path(d_pconf.at("image_src_dir"));
    }
  } else {
    std::cout << "writing config: " << pconf_fn << std::endl;
    conf_load_default();
    success = vise::configuration_save(d_pconf, pconf_fn.string());
  }
  return success;
}

void vise::project::conf_load_default() {
  d_pconf.clear();
  d_pconf["search_engine"] = "relja_retrival";
  d_pconf["use_root_sift"] = "true";
  d_pconf["sift_scale_3"] = "true";
  d_pconf["bow_descriptor_count"] = "-1";
  d_pconf["cluster_num_iteration"] = "3";
  d_pconf["bow_cluster_count"] = "1000";
  d_pconf["hamm_embedding_bits"] = "32";
  d_pconf["resize_dimension"] = "512x512";

  d_data_dir = d_project_dir / "data";
  d_image_dir = d_project_dir / "image";
  d_image_src_dir = d_project_dir / "image_src";
  boost::filesystem::create_directory(d_data_dir);
  boost::filesystem::create_directory(d_image_dir);
  boost::filesystem::create_directory(d_image_src_dir);

  d_pconf["data_dir"] = d_data_dir.string() + boost::filesystem::path::preferred_separator;
  d_pconf["image_dir"] = d_image_dir.string() + boost::filesystem::path::preferred_separator;
  d_pconf["image_src_dir"] = d_image_src_dir.string() + boost::filesystem::path::preferred_separator;
}

void vise::project::conf_to_json(std::ostringstream &json) {
  if(d_pconf.size()) {
    json << "{";
    std::map<std::string, std::string>::const_iterator itr = d_pconf.begin();
    json << "\"" << itr->first << "\":\"" << itr->second << "\"";
    ++itr;
    for(; itr != d_pconf.end(); ++itr) {
      json << ",\"" << itr->first << "\":\"" << itr->second << "\"";
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
    boost::filesystem::path pconf_fn = d_data_dir / "conf.txt";
    bool result = vise::configuration_save(d_pconf, pconf_fn.string());
    return result;
  } catch(std::exception &ex) {
    return false;
  }
}

void vise::project::search_engine_init(std::string search_engine_name,
                                       bool &success,
                                       std::string &message) {
  if(!d_search_engine) {
    if (d_pconf.at("search_engine") == "relja_retrival") {
      d_search_engine = std::unique_ptr<vise::relja_retrival>(new relja_retrival(d_pconf));
      success = true;
      message = "initialized relja_retrival search engine";
    } else {
      success = false;
      message = "unknown search_engine";
    }
  } else {
    success = true;
    message = "search_engine already initialized";
  }
}

void vise::project::index_create(bool &success, std::string &message) {
  std::lock_guard<std::mutex> lock(d_index_mutex);

  try {
    // load configuration
    conf_reload();
    search_engine_init(d_pconf.at("search_engine"), success, message);
    if (success) {
      d_search_engine->index_create(success,
                                    message,
                                    std::bind( &vise::project::state_update, this));
    }
  } catch(std::exception &e) {
    success = false;
    message = e.what();
  }
}

void vise::project::index_load(bool &success, std::string &message) {
  std::lock_guard<std::mutex> lock(d_index_load_mutex);
  if (!d_search_engine) {
    conf_reload();
    search_engine_init(d_pconf.at("search_engine"), success, message);
    if(!success) {
      return;
    }
  }
  d_search_engine->index_load(success, message);
}

void vise::project::index_unload(bool &success, std::string &message) {
  std::lock_guard<std::mutex> lock(d_index_load_mutex);
  if (d_search_engine) {
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

bool vise::project::index_is_loaded() {
  if (d_search_engine) {
    return d_search_engine->index_is_loaded();
  } else {
    return false;
  }
}

bool vise::project::index_is_done() {
  if (d_search_engine) {
    return d_search_engine->index_is_done();
  } else {
    return false;
  }
}

bool vise::project::index_is_incomplete() {
  if (d_search_engine) {
    return d_search_engine->index_is_incomplete();
  } else {
    return false;
  }
}

bool vise::project::index_is_ongoing() {
  if (d_search_engine) {
    return d_search_engine->index_is_ongoing();
  } else {
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

uint32_t vise::project::fid(std::string filename) const {
  if(d_search_engine) {
    return d_search_engine->fid(filename);
  } else {
    return 4294967295; //@todo: improve
  }
}

std::string vise::project::filename(uint32_t fid) const {
  if(d_search_engine) {
    return d_search_engine->filename(fid);
  } else {
    return "";
  }
}

uint32_t vise::project::image_src_count() const {
  uint32_t count = 0;
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator it(d_image_src_dir); it!=end_itr; ++it) {
    if (boost::filesystem::is_regular_file(it->path())) {
      count++;
    }
  }
  return count;
}

std::string vise::project::pconf(std::string key) {
  if(d_pconf.count(key)) {
    return d_pconf.at(key);
  } else {
    return "";
  }
}
