/** @file project_manager.h
 *  @brief Manages the creation, indexing, query, update and deletion of VISE projects
 *  @author Abhishek Dutta <adutta _at_ robots.ox.ac.uk>
 *  @date 13 Nov. 2019
 */
#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include "vise_util.h"
#include "http_request.h"
#include "http_response.h"
#include "project.h"
#include "search_result.h"
#include "search_query.h"
#include "project_html_resources.h"

#include <unordered_map>
#include <string>
#include <iostream>
#include <utility>
#include <memory>
#include <mutex>
#include <cmath>

namespace vise {
  class project_manager {
  public:
    project_manager(std::map<std::string, std::string> const &conf);
    ~project_manager();

    // http request handlers
    void process_http_request(http_request const &request,
                              http_response &response);
    void handle_get(http_request const &request,
                    std::vector<std::string> const &uri,
                    std::map<std::string, std::string> const &param,
                    http_response &response) const;
    void handle_post(http_request const &request,
                     std::vector<std::string> const &uri,
                     std::map<std::string, std::string> const &param,
                     http_response &response);
    void handle_put(http_request const &request,
                    std::vector<std::string> const &uri,
                    std::map<std::string, std::string> const &param,
                    http_response &response);
    void handle_delete(http_request const &request,
                       std::vector<std::string> const &uri,
                       std::map<std::string, std::string> const &param,
                       http_response &response);

    void file_send(boost::filesystem::path fn,
                   http_response& response) const;
    void payload_save(http_request const &request,
                      boost::filesystem::path fn,
                      http_response& response);

    // project's HTML based minimal UI
    void project_home(std::string pname,
                      http_response &response) const;
    void project_filelist(std::string pname,
                          http_response &response,
                          std::map<std::string, std::string> const &param) const;
    void project_file(std::string pname,
                      http_response &response,
                      std::map<std::string, std::string> const &param) const;
    void project_show_match(std::string pname,
                            std::map<std::string, std::string> const &param,
                            http_response &response) const;
    void project_index_search(std::string pname,
                              std::map<std::string, std::string> const &param,
                              http_response &response) const;
    void project_register_image(std::string pname,
                                std::map<std::string, std::string> const &param,
                                http_response &response) const;

    bool project_exists(std::string pname) const;
    void project_create(std::string pname);
    bool project_load(std::string pname);
    bool project_is_loaded(std::string pname) const;
    void project_delete(std::string pname);
    void project_index_create(std::string pname,
                       http_response &response);
    void project_index_load(std::string pname,
                            http_response &response);
    void project_index_unload(std::string pname,
                            http_response &response);
    bool project_index_is_done(std::string pname);
    bool project_index_is_loaded(std::string pname);

    void project_file_add(std::string pname,
                          std::map<std::string, std::string> const &param,
                          http_response &response);

    void project_conf_set(std::string pname,
                          std::string &conf_str,
                          http_response &response);

    void project_list(std::map<std::string, std::string> const &param,
                      http_response &response) const;
    void project_pname_list(std::vector<std::string> &pname_list) const;

    void debug();
  private:
    std::mutex d_project_load_mutex;
    const std::map<std::string, std::string> d_conf;
    std::unordered_map<std::string, std::unique_ptr<vise::project> > d_projects;
  };
}
#endif
