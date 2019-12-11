#include "project.h"

vise::project::project(std::string pname,
                       std::map<std::string, std::string> const &conf)
  : d_pname(pname), d_conf(conf)
{
  std::cout << "project(): constructing " << pname << " ..."
            << std::endl;
  d_storedir = boost::filesystem::path(d_conf.at("project_store")) / pname;
  d_datadir  = boost::filesystem::path(d_conf.at("vise_store")) / pname;
  d_pconf_fn = d_datadir / "conf.txt";
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
                                 std::vector<vise::search_result> &r) {
  if (d_search_engine) {
    d_search_engine->index_search(q, r);
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
