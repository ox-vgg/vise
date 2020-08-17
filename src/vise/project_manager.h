/** @file project_manager.h
 *  @brief Manages the creation, indexing, query, update and deletion of VISE projects
 *  @author Abhishek Dutta <adutta _at_ robots.ox.ac.uk>
 *  @date 13 Nov. 2019
 */
#ifndef PROJECT_MANAGER_H
#define PROJECT_MANAGER_H

#include "vise_version.h"
#include "vise_util.h"
#include "http_request.h"
#include "http_response.h"
#include "project.h"
#include "search_result.h"
#include "search_query.h"
#include "html_resources.h"

#include <map>
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

    // limit serving
    void serve_only(std::map<std::string, std::string> const pname_pconf_fn_map);
    void serve_only_4xx_response(http_response &response) const;

    // http request handlers
    void process_http_request(http_request const &request,
                              http_response &response);
    void handle_get(http_request const &request,
                    std::vector<std::string> const &uri,
                    std::map<std::string, std::string> const &param,
                    http_response &response); // non-const as it can trigger project_load()
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

    void serve_from_www_store(std::string res_uri,
                              http_response &response) const;
    void file_send(boost::filesystem::path fn,
                   http_response& response) const;
    void handle_project_get_request(std::string const pname,
                                    http_request const &request,
                                    std::vector<std::string> const &uri,
                                    std::map<std::string, std::string> const &param,
                                    http_response& response) const;

    void payload_save(http_request const &request,
                      boost::filesystem::path fn,
                      http_response& response);

    // VISE HTML based minimal UI
    void vise_home(std::map<std::string, std::string> const &param,
                   http_response &response) const;
    void vise_settings(std::map<std::string, std::string> const &param,
                       http_response &response) const;
    void vise_settings_save(std::string &settings_formdata,
                            http_response &response);

    void vise_about(std::map<std::string, std::string> const &param,
                    http_response &response) const;

    void vise_project_create(std::map<std::string, std::string> const &param,
                             http_response &response);
    void vise_project_delete(std::map<std::string, std::string> const &param,
                             http_response &response);
    void vise_project_unload(std::map<std::string, std::string> const& param,
                             http_response& response);
    void vise_error_page(const std::string message,
                         const std::string response_format,
                         http_response &response) const;
    void vise_wait_page(const std::string message,
                        const std::string response_format,
                        http_response &response) const;

    bool is_project_name_valid(const std::string pname) const;

    // project's HTML based minimal UI
    void project_home(std::string pname,
                      http_response &response) const;
    void project_filelist(std::string pname,
                          std::map<std::string, std::string> const &param,
                          http_response &response) const;
    void project_file(std::string pname,
                      std::map<std::string, std::string> const &param,
                      http_response &response) const;
    void project_show_match(std::string pname,
                            std::map<std::string, std::string> const &param,
                            http_response &response) const;
    void project_index_search(std::string pname,
                              std::map<std::string, std::string> const &param,
                              http_response &response) const;
    void project_register_image(std::string pname,
                                std::map<std::string, std::string> const &param,
                                http_response &response) const;
    void project_configure(std::string pname,
                           std::map<std::string, std::string> const &param,
                           http_response &response) const;
    void project_index_status(std::string pname,
                              std::map<std::string, std::string> const &param,
                              http_response &response) const;

    bool project_exists(std::string pname) const;
    bool project_create(std::string pname);
    bool project_load(std::string pname);
    bool project_is_loaded(std::string pname) const;
    void project_index_create(std::string pname,
                              http_response &response,
                              bool block_until_done=false);
    void project_index_load(std::string pname,
                            http_response &response);
    void project_index_unload(std::string pname,
                            http_response &response);
    bool project_index_is_done(std::string pname);
    bool project_index_is_loaded(std::string pname);

    void project_file_add(std::string pname,
                          std::map<std::string, std::string> const &param,
                          http_response &response);

    void project_config_save(std::string pname,
                             std::string &config_formdata,
                             http_response &response);
    void project_config_use_preset(std::string pname,
                                   std::string preset_id,
                                   http_response &response);

    void project_pname_list(std::vector<std::string> &pname_list) const;
    uint32_t project_image_src_count(std::string pname) const;
  private:
    bool is_serve_only_active;
    std::mutex d_project_load_mutex;
    const std::map<std::string, std::string> d_conf;
    std::map<std::string, std::unique_ptr<vise::project> > d_projects;
  };
}
#endif
