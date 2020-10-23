//
// VISE Command Line Interface (vise-cli) is used to create and serve vise projects from command line
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 15 Oct. 2020
//

#include "vise_version.h"
#include "vise_util.h"
#include "http_server.h"

#include <string>
#include <map>

#include <boost/filesystem.hpp>
#include <Magick++.h>

#include "vise_version.h"
#include "vise_util.h"
#include "http_server.h"

#include <boost/filesystem.hpp>
#include <Magick++.h>

#include <iostream>
#include <memory>
#include <cstdlib>

void print_usage(std::string visecli_exec="vise-cli") {
  std::cout << "Usage:" << std::endl
            << "- " << visecli_exec << " create-project PROJECT_NAME CONFIG_FILENAME" << std::endl
            << "- " << visecli_exec << " serve-project --port=9669 --nthread=4 --http_uri_namespace=/vise/demo/ PNAME1:PCONFIG1 PNAME2:PCONFIG2 ..." << std::endl;
}

int main(int argc, char **argv) {
  std::cout << VISE_FULLNAME << " (" << VISE_NAME << ") "
            << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH
            << std::endl;
  std::string visecli_exec_name = boost::filesystem::path(argv[0]).filename().string();

  if(argc == 1) {
    print_usage(visecli_exec_name);
    return 0;
  }

  std::string cmd(argv[1]);
  if(cmd == "create-project") {
    if(argc == 4) {
      std::string pname(argv[2]);
      boost::filesystem::path conf_fn(argv[3]);
      if( !boost::filesystem::exists(conf_fn) ) {
        std::cout << "project configuration file not found: "
                  << conf_fn << std::endl;
        return 1;
      }
      vise::project new_project(pname, conf_fn.string());
      bool success;
      std::string message;
      bool block_until_done = true;
      new_project.index_create(success, message, block_until_done);
      std::cout << message << std::endl;
      return 0;
    } else {
      print_usage(visecli_exec_name);
      return 1;
    }
  }

  if(cmd == "create-visual-vocabulary") {
    if(argc == 4) {
      std::string pname(argv[2]);
      boost::filesystem::path conf_fn(argv[3]);
      if( !boost::filesystem::exists(conf_fn) ) {
        std::cout << "project configuration file not found: "
                  << conf_fn << std::endl;
        return 1;
      }
      boost::filesystem::path data_dir = conf_fn.parent_path();
      std::string placeholder("dummy file to only perform traindesc, cluster, trainassign, trainhamm and avoid indexing stage");
      vise::file_save(data_dir / "index_dset.bin", placeholder);
      vise::file_save(data_dir / "index_fidx.bin", placeholder);
      vise::file_save(data_dir / "index_iidx.bin", placeholder);

      vise::project new_project(pname, conf_fn.string());
      bool success;
      std::string message;
      bool block_until_done = true;
      new_project.index_create(success, message, block_until_done);
      std::cout << message << std::endl;
      return 0;
    } else {
      print_usage(visecli_exec_name);
      return 1;
    }
  }

  if(cmd == "serve-project") {
    if(argc > 2) {
      std::map<std::string, std::string> vise_settings;
      std::map<std::string, std::string> pname_pconf_fn_map;

      // EXPECTED: ./vise serve-project --port=9103 PNAME1:PCONF1 PNAME2:PCONF2 ...
      for(std::size_t i=2; i<argc; ++i) {
        std::string arg(argv[i]);
        if(arg.size() < 3) {
          continue;
        }
        if(arg[0] == '-' && arg[1] == '-') {
          std::size_t eq_pos = arg.find('=');
          if(eq_pos == std::string::npos) {
            continue;
          }
          std::string key(arg.substr(2, eq_pos-2));
          std::string val(arg.substr(eq_pos+1));
          vise_settings[key] = val;
        } else {
          std::size_t colon_pos = arg.find(':');
          if(colon_pos == std::string::npos) {
            continue;
          }
          std::string pname(arg.substr(0, colon_pos));
          boost::filesystem::path project_conf_fn(arg.substr(colon_pos+1));
          if( !boost::filesystem::exists(project_conf_fn) ) {
            std::cout << "not found configuration file [" << project_conf_fn << "] for project [" << pname << "]" << std::endl;
            return 1;
          }
          pname_pconf_fn_map[pname] = project_conf_fn.string();
        }
      }
      if(pname_pconf_fn_map.size() == 0) {
        std::cout << "Projects details must be provided in PNAME:PCONF format" << std::endl;
        print_usage(visecli_exec_name);
        return 1;
      }
      vise::project_manager manager(vise_settings);
      manager.serve_only(pname_pconf_fn_map);
      vise::http_server server(vise_settings, manager);
      server.start();
      return 0;
    } else {
      print_usage(visecli_exec_name);
      return 1;
    }
  }

  std::cout << "Unknown command: " << argv[1] << std::endl;
  print_usage(visecli_exec_name);
  return 1;
}
