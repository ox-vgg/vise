/** @file project_manager.h
 *  @brief Manages the creation, indexing, query, update and deletion of VISE projects
 *  @author Abhishek Dutta
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

#include <map>
#include <string>
#include <iostream>
#include <utility>
#include <memory>
#include <mutex>

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
                    http_response &response);
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
                   http_response& response);
    void payload_save(http_request const &request,
                      boost::filesystem::path fn,
                      http_response& response);

    // project
    bool project_exists(std::string pname);
    void project_create(std::string pname);
    bool project_load(std::string pname);
    void project_delete(std::string pname);
    void project_index_create(std::string pname,
                       http_response &response);
    void project_index_load(std::string pname,
                            http_response &response);
    void project_index_unload(std::string pname,
                            http_response &response);
    void project_index_search(std::string pname,
                              std::map<std::string, std::string> const &param,
                              http_response &response);
    bool project_index_is_done(std::string pname);
    bool project_index_is_loaded(std::string pname);

    void project_file_add(std::string pname,
                          std::map<std::string, std::string> const &param,
                          http_response &response);

    void project_conf_set(std::string pname,
                          std::string &conf_str,
                          http_response &response);

    void debug();

    static std::string PROJECT_HTML_PAGE_PREFIX;
    static std::string PROJECT_HTML_PAGE_SUFFIX;
  private:
    std::mutex d_project_load_mutex;
    const std::map<std::string, std::string> d_conf;
    std::map<std::string, std::unique_ptr<vise::project> > d_projects;
  };
}
#endif
