#include "project.h"

vise::project::project(std::string pname,
                       std::map<std::string, std::string> const &conf)
  : d_pname(pname), d_conf(conf), d_state(project_state::UNKNOWN)
{

  std::cout << "project(): constructing " << pname << " ..."
            << std::endl;
  d_storedir = boost::filesystem::path(d_conf.at("project_store")) / pname;
  d_datadir  = boost::filesystem::path(d_conf.at("vise_store")) / pname;
  d_pconf_fn = d_datadir / "conf.txt";

  conf_reload();
  bool success;
  std::string message;
  search_engine_init(d_pconf.at("search_engine"), success, message);
  if(!success) {
    std::cerr << message << std::endl;
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
    if(!success) {
      std::cout << "~project(): unloading index for ["
                << d_pname << "] failed:" << message
                << std::endl;
    }
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

void vise::project::conf_reload() {
  vise::configuration_load(d_pconf_fn.string(), d_pconf);
  d_pconf["storedir"] = d_storedir.string() + boost::filesystem::path::preferred_separator;
  d_pconf["datadir"] = d_datadir.string() + boost::filesystem::path::preferred_separator;
}

void vise::project::search_engine_init(std::string search_engine_name,
                                       bool &success,
                                       std::string &message) {
  if (d_pconf.at("search_engine") == "relja_retrival") {
    d_search_engine = std::unique_ptr<vise::relja_retrival>(new relja_retrival(d_pconf));
    success = true;
    message = "initialized relja_retrival search engine";
  } else {
    success = false;
    message = "unknown search_engine";
  }
}

void vise::project::index_create(bool &success, std::string &message) {
  std::lock_guard<std::mutex> lock(d_index_mutex);

  try {
    // load configuration
    conf_reload();
    search_engine_init(d_pconf.at("search_engine"), success, message);
    if (success) {
      d_search_engine->index_create(success, message);
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
