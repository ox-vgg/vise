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

  vise::configuration_show(d_pconf);
  /*
  if (d_pconf.at("search_engine") == "relja_retrival") {
    d_search_engine = std::make_unique<vise::search_engine>();
  } else {
    std::cout << "Unknown search engine ["
              << d_pconf.at("search_engine") << "]"
              << std::endl;
  }
  */
}
