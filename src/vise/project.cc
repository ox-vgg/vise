#include "project.h"

vise::project::project(std::string pname,
                       std::map<std::string, std::string> const &conf)
  : d_pname(pname), d_conf(conf)
{
  std::cout << "project(): constructing ..."
            << std::endl;
  d_storedir = boost::filesystem::path(d_conf.at("project_store")) / pname;
  d_datadir  = boost::filesystem::path(d_conf.at("vise_store")) / pname;
}

vise::project::~project() {
  std::cout << "Destroying project ..."
            << std::endl;
}

void vise::project::index() {
  d_state = vise::PROJECT_STATE::INDEX;
  std::cout << "project(): indexing ..."
            << std::endl;

  // load configuration
  boost::filesystem::path project_config_fn = d_datadir / "conf.txt";
  vise::configuration_load(project_config_fn.string(), d_pconf);
  d_pconf["storedir"] = d_storedir.string() + boost::filesystem::path::preferred_separator;
  d_pconf["datadir"] = d_datadir.string() + boost::filesystem::path::preferred_separator;

  if (d_pconf.at("search_engine") == "relja_retrival") {
    std::cout << "initializing instance of relja_retrival" << std::endl;
    d_search_engine = std::unique_ptr<vise::relja_retrival>(new relja_retrival(d_pconf));
    d_search_engine->index();
  } else {
    std::cout << "Unknown search engine ["
              << d_pconf.at("search_engine") << "]"
              << std::endl;
  }
}
