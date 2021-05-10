//
// VISE Command Line Interface (vise-cli) is used to create and serve vise projects from command line
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 15 Oct. 2020
//

#include "vise_version.h"
#include "vise_util.h"
#include "http_server.h"
#include "cli_resources.h"

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

namespace vise {
  void print_version() {
    std::cout << VISE_FULLNAME << " (" << VISE_NAME << ") "
              << VISE_VERSION_MAJOR << "." << VISE_VERSION_MINOR << "." << VISE_VERSION_PATCH
              << std::endl;
  }

  void print_help() {
    std::cout << vise::VISE_CLI_HELP_STR << std::endl;
  }
}

int main(int argc, char **argv) {
  if(argc == 1) { // no command line arguments
    vise::print_help();
    return 0;
  }

  std::unordered_map<std::string, std::string> cli_args;
  std::unordered_map<std::string, std::string> pname_pconf_list;
  bool cli_arg_ok = vise::parse_cli_args(argc, argv, cli_args, pname_pconf_list);

  if(cli_arg_ok == false) {
    std::cout << "failed to parse command line arguments"
              << std::endl;
    return 1;
  }

  if(cli_args.count("version") == 1) {
    vise::print_version();
    return 0;
  }

  if(cli_args.count("help") == 1 ||  // --help
     cli_args.count("cmd") == 0) {   // if --cmd is missing
    vise::print_help();
    return 0;
  }

  if(cli_args.at("cmd") == "create-project" ||
     cli_args.at("cmd") == "create-visual-vocabularly" ) {
    if(pname_pconf_list.size() != 1) {
      std::cout << "--cmd={create-project, create-visual-vocabulary} accepts "
                << "only a single PROJECT_NAME:CONF_FILENAME parameter."
                << std::endl;
      return 1;
    }
  }

  if(cli_args.at("cmd") == "web-ui") {
    // sanity check
    if(pname_pconf_list.size() != 0) {
      std::cout << "--cmd=web-ui does not accept PROJECT_NAME:CONF_FILENAME parameter."
                << std::endl;
      return 1;
    }

    std::map<std::string, std::string> vise_settings;
    std::unordered_map<std::string, std::string>::const_iterator itr;
    vise::init_default_vise_settings(vise_settings);
    for(itr=cli_args.begin(); itr!=cli_args.end(); ++itr) {
      if(vise_settings.count(itr->first)) {
        vise_settings[itr->first] = itr->second;
      }
    }

    vise::project_manager manager(vise_settings);
    vise::http_server server(vise_settings, manager);
    server.start();
    return 0;
  }

  if(cli_args.at("cmd") == "create-project") {
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

  if(cli_args.at("cmd") == "create-visual-vocabulary") {
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

  if(cli_args.at("cmd") == "serve-project") {
    if(pname_pconf_list.size() == 0) {
      std::cout << "--cmd=serve-project requires at least one PROJECT_NAME:CONF_FILENAME parameter."
                << std::endl;
      return 1;
    }
    if(cli_args.count("http-www-dir") == 0) {
      std::cout << "--http-www-dir must be provided and should point to a folder containing "
                << "all VISE web application files."
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

    vise::project_manager manager(vise_settings);
    manager.serve_only(pname_pconf_list);
    vise::http_server server(vise_settings, manager);
    server.start();
    return 0;
  }

  if(cli_args.at("cmd") == "create-visual-group") {
    if(pname_pconf_list.size() != 1) {
      std::cout << "--cmd=create-visual-group requires only one PROJECT_NAME:CONF_FILENAME parameter."
                << std::endl;
      return 1;
    }
    if(cli_args.count("vgroup-id") == 0) {
      std::cout << "you must specify a unique id for visual group using --vgroup-id parameter."
                << std::endl;
      return 1;
    }

    std::unordered_map<std::string, std::string>::const_iterator itr = pname_pconf_list.begin();
    std::string pname = itr->first;
    boost::filesystem::path project_conf_fn(itr->second);
    if( !boost::filesystem::exists(project_conf_fn) ) {
      std::cout << "Error: configuration file ["
                << project_conf_fn << "] for project [" << pname << "] "
                << "was not found" << std::endl;
      return 1;
    }
    vise::project existing_project(pname, project_conf_fn.string());
    bool success = false;
    std::string message;
    bool block_until_done = true;

    existing_project.create_vgroup(cli_args, block_until_done, success, message);
    if(!success) {
      std::cout << "failed: " << message << std::endl;
    }
    return 0; // as we know there is only one project to be processed
  }

  std::cout << "Unknown --cmd=" << cli_args.at("cmd")
            << std::endl << "use --help for more details."
            << std::endl;
  return 1;
}
