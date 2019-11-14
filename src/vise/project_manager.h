/** @file project_manager.h
 *  @brief Manages the creation, query, update and deletion of VISE projects
 *  @author Abhishek Dutta
 *  @date 13 Nov. 2019
 */
#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include "vise_util.h"
#include "http_request.h"
#include "http_response.h"

#include <map>
#include <string>
#include <iostream>

namespace vise {
  class project_manager {
  public:
    project_manager(std::map<std::string, std::string> const &conf);
    void process_http_request(http_request const &request, http_response &response);
    ~project_manager() {
      std::cout << "**********destroying project manager" << std::endl;
    }

  private:
    const std::map<std::string, std::string> _conf;
  };
}
#endif
