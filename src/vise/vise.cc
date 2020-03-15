//
// Entry point for VISE application
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 12 Nov. 2019
//

#include "vise_version.h"
#include "vise_util.h"
#include "http_server.h"

#include <boost/filesystem.hpp>
#include <Magick++.h>

#include <iostream>
#include <memory>
#include <cstdlib>

int main(int argc, char **argv) {
  std::cout << VISE_FULLNAME << " (" << VISE_NAME << ") "
            << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH
            << std::endl;
  const boost::filesystem::path visehome = vise::vise_home();
  boost::filesystem::path vise_settings = visehome / "vise_settings.txt";
  std::map<std::string, std::string> conf;
  std::cout << "VISE_HOME=" << visehome << std::flush << std::endl;
  if(!boost::filesystem::exists(vise_settings)) {
    // use default configuration for VISE
    boost::filesystem::path vise_store = visehome / "store";
    boost::filesystem::path www_store = visehome / "www";

    if(!boost::filesystem::exists(visehome)) {
      boost::filesystem::create_directories(visehome);
      boost::filesystem::create_directory(vise_store);
      boost::filesystem::create_directory(www_store);
    }

    conf["vise_store"] = vise_store.string();
    conf["www_store"] = www_store.string();
    conf["address"] = "0.0.0.0";
    conf["port"] = "9670";
    conf["nthread"] = "4";
    vise::configuration_save(conf, vise_settings.string());
  }
  // load VISE configuration
  vise::configuration_load(vise_settings.string(), conf);
  Magick::InitializeMagick(*argv);

  // create temp. dir (if not exists)
  boost::filesystem::path tmpdir = boost::filesystem::temp_directory_path();
  tmpdir = tmpdir / "vise";
  if (!boost::filesystem::exists(tmpdir)) {
      boost::filesystem::create_directories(tmpdir);
  }

  // start http server to serve contents in a web browser
  std::cout << "Initializing http_server ..." << std::endl;
  vise::http_server server(conf);
  server.start();
  return 0;
}
