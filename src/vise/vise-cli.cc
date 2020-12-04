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
  std::cout << "\nUsage::\n"
            << visecli_exec << " --run-mode=[create-project | server-project | web-ui | create-visual-vocabulary | ...]\n"
            << "         --vise-home=DIR --port=PORT --nthread=NTHREAD --http_uri_namespace=NS ...\n"
            << "         PROJECT1_NAME:CONF_FILENAME PROJECT2_NAME:CONF_FILENAME ...\n"
            << "\nExample::\n"
            << "a) to access all features of VISE using the web address http://localhost:10011/my_vise/\n"
            << "$ " << visecli_exec << " --run-mode=web-ui --vise-home=/home/xyz/.vise/ --port=10011 --nthread=8 --http_uri_namespace=/my_vise/\n"
            << "b) to create a project whose configuration file is stored in /data/Oxford-Buildings/data/conf.txt\n"
            << "$ " << visecli_exec << " --run-mode=create-project --nthread=32 Oxford-Buildings /data/Oxford-Buildings/data/conf.txt" << std::endl
            << "c) to allow users to search two projects (e.g. P1, P2) using the web address http://localhost:80/demo/\n"
            << "$ " << visecli_exec << " --run-mode=serve-project --port=80 --http_uri_namespace=/demo/ P1:/p1/data/conf.txt P2:/p2/data/conf.txt" << std::endl;
}

int main(int argc, char **argv) {
  std::cout << VISE_FULLNAME << " (" << VISE_NAME << ") "
            << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH
            << std::endl;
  std::string visecli_exec_name = boost::filesystem::path(argv[0]).filename().string();
  std::unordered_map<std::string, std::string> cli_args;
  std::unordered_map<std::string, std::string> pname_pconf_list;
  bool cli_arg_ok = vise::parse_cli_args(argc, argv, cli_args, pname_pconf_list);

  // sanity check of command line arguments
  if(argc == 1 ||
     cli_args.count("help") == 1 ||
     !cli_arg_ok ||
     cli_args.count("run-mode") == 0) {
    print_usage(visecli_exec_name);
    return 0;
  }
  if(cli_args.at("run-mode") == "create-project" ||
     cli_args.at("run-mode") == "create-visual-vocabularly" ) {
    if(pname_pconf_list.size() == 1) {
      std::cout << "--run-mode={create-project, create-visual-vocabulary} accepts only a single PROJECT_NAME:CONF_FILENAME parameter."
                << std::endl;
      return 1;
    }
  }

  if(cli_args.at("run-mode") == "create-project") {
    std::unordered_map<std::string, std::string>::const_iterator itr;
    for( itr=pname_pconf_list.begin(); itr!=pname_pconf_list.end(); ++itr) {
      std::string pname = itr->first;
      boost::filesystem::path project_conf_fn(itr->second);
      if( !boost::filesystem::exists(project_conf_fn) ) {
        std::cout << "Error: configuration file ["
                  << project_conf_fn << "] for project [" << pname << "] "
                  << "was not found" << std::endl;
        return 1;
      }
      vise::project new_project(pname, project_conf_fn.string());
      bool success;
      std::string message;
      bool block_until_done = true;
      new_project.index_create(success, message, block_until_done);
      std::cout << message << std::endl;
      return 0; // as we know there is only one project to be processed
    }
  }

  if(cli_args.at("run-mode") == "create-visual-vocabulary") {
    std::unordered_map<std::string, std::string>::const_iterator itr;
    for( itr=pname_pconf_list.begin(); itr!=pname_pconf_list.end(); ++itr) {
      std::string pname = itr->first;
      boost::filesystem::path project_conf_fn(itr->second);
      if( !boost::filesystem::exists(project_conf_fn) ) {
        std::cout << "Error: configuration file ["
                  << project_conf_fn << "] for project [" << pname << "] "
                  << "was not found" << std::endl;
        return 1;
      }
      boost::filesystem::path data_dir = project_conf_fn.parent_path();
      std::string placeholder("VISE has created this dummy file to only perform traindesc, cluster, trainassign, trainhamm and avoid indexing stage. You must remove the 0 sized files (i.e. index_dset.bin, index_fidx.bin, index_iidx.bin if you want to continue through the indexing stage.");
      vise::file_save(data_dir / "index_dset.bin", placeholder);
      vise::file_save(data_dir / "index_fidx.bin", placeholder);
      vise::file_save(data_dir / "index_iidx.bin", placeholder);

      vise::project new_project(pname, project_conf_fn.string());
      bool success;
      std::string message;
      bool block_until_done = true;
      new_project.index_create(success, message, block_until_done);
      std::cout << message << std::endl;
      return 0; // as we know there is only one project to be processed
    }
  }

  if(cli_args.at("run-mode") == "serve-project") {
    if(pname_pconf_list.size() == 0) {
      std::cout << "--run-mode=serve-project requires at least one PROJECT_NAME:CONF_FILENAME parameter."
                << std::endl;
      return 1;
    }
    std::unordered_map<std::string, std::string>::const_iterator itr;
    for( itr=pname_pconf_list.begin(); itr!=pname_pconf_list.end(); ++itr) {
      std::string pname = itr->first;
      boost::filesystem::path project_conf_fn(itr->second);
      if( !boost::filesystem::exists(project_conf_fn) ) {
        std::cout << "Error: configuration file ["
                  << project_conf_fn << "] for project [" << pname << "] "
                  << "was not found" << std::endl;
        return 1;
      }
    }
    std::map<std::string, std::string> vise_settings;
    vise::init_default_vise_settings(vise_settings);
    for(itr=cli_args.begin(); itr!=cli_args.end(); ++itr) {
      vise_settings[itr->first] = itr->second;
    }
    if(!vise::does_vise_home_and_store_exist(vise_settings)) {
      std::cout << "missing folders should be defined as the following command line arguments: "
                << "--vise_home, --vise_store, --www_store, --asset_store"
                << std::endl;
      return 1;
    }

    vise::project_manager manager(vise_settings);
    manager.serve_only(pname_pconf_list);
    vise::http_server server(vise_settings, manager);
    server.start();
    return 0;
  }

  std::cout << "Unknown --run-mode=" << cli_args.at("run-mode") << std::endl;
  print_usage(visecli_exec_name);
  return 1;
}
