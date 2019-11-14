//
// Entry point for VISE application
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 12 Nov. 2019
//

#include "vise_util.h"
#include "http_server.h"

#include <iostream>
#include <memory>

int main(int argc, char **argv) {
  std::cout << "VGG Image Search Engine (VISE)"
            << std::endl;

  std::string conf_fn("/home/tlm/code/vise2/data/test/viseconf.txt");

  // load VISE configuration
  std::map<std::string, std::string> conf;
  vise::configuration_load(conf_fn, conf);

  // start http server to serve contents in a web browser
  vise::http_server server(conf);
  server.start();
}
