/** @file project.h
 *  @brief denotes a VISE project
 *  @author Abhishek Dutta
 *  @date 15 Nov. 2019
 */
#ifndef PROJECT_H
#define PROJECT_H

#include "metadata.h"
#include "search_engine.h"
#include "search_query.h"
#include "search_result.h"
#include "vise_util.h"
#include "relja_retrival/relja_retrival.h"

#include <memory>
#include <thread>
#include <exception>
#include <functional>
#include <unordered_map>
#include <set>

#include <boost/filesystem.hpp>

namespace vise {
  enum class project_state { UNKNOWN, INIT_FAILED, SET_CONFIG, INDEX_ONGOING, BROKEN_INDEX, SEARCH_READY };

  class project {
  public:
    project(std::string pname,
            std::string pconf_fn);
    project(std::string pname,
            std::map<std::string, std::string> const &vise_conf);
    project(project const &p);
    ~project();

    void index_create(bool &success, std::string &message, bool block_until_done=false);
    void index_load(bool &success, std::string &message);
    void index_unload(bool &success, std::string &message);
    bool index_load_is_ongoing() const;
    bool index_is_loaded() const;
    bool index_is_done() const;
    bool index_is_incomplete() const;
    bool index_is_ongoing() const;
    std::string index_status_to_json();

    void index_search(vise::search_query const &query,
                      std::vector<vise::search_result> &result) const;

    void index_internal_match(vise::search_query const &q,
                              uint32_t match_file_id,
                              std::ostringstream &json) const;

    void register_image(uint32_t file1_id, uint32_t file2_id,
                        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                        std::array<double, 9> &H) const;
    void register_external_image(const std::string &image_data,
                                 const uint32_t file2_id,
                                 std::array<double, 9> &H) const;

    project_state state() const;
    std::string state_name() const;

    uint32_t fid_count() const;
    uint32_t fid(const std::string filename) const;
    std::string filename(const uint32_t fid) const;

    // metadata
    bool is_metadata_available() const;
    void file_metadata_full_text_search(const std::string query,
                                        std::vector<uint32_t> &fid_list) const;
    void file_metadata_full_text_search_group_stat(const std::string query,
                                                   const std::string groupby,
                                                   std::map<std::string, uint32_t> &group_stat) const;

    void file_attribute_name_list(std::vector<std::string> &file_attribute_name_list) const;
    void metadata_group_stat(const std::string groupby,
                             std::map<std::string, uint32_t> &group_stat) const;
    void metadata_groupby(const std::string groupby,
                          const std::string group,
                          std::vector<uint32_t> &flist) const;

    void file_metadata_as_json(const uint32_t file_id,
                               std::ostringstream &json) const;
    void region_metadata_as_json(const uint32_t file_id,
                                 std::ostringstream &json) const;
    void metadata_conf_as_json(std::ostringstream &json) const;
    void error_metadata_not_available() const;

    uint32_t image_src_count() const;

    bool conf_reload();
    void conf_to_json(std::ostringstream &json);
    bool conf_from_plaintext(std::string plaintext);
    std::string pconf(std::string key);
    bool pconf_is_set(std::string key);
    bool use_preset_conf(std::string preset_conf_id);
    bool use_preset_conf_1();
    void use_preset_conf_2();
    void use_preset_conf_auto();
    void use_preset_conf_manual();
    void remove_existing_visual_vocabulary();
    void preset_conf_to_json(std::ostringstream &json);
    inline bool app_dir_exists() {
      return d_app_dir_exists;
    }

    // for search using image features
    void extract_image_features(const std::string &image_data,
                                std::string &image_features) const;
    void index_search_using_features(const std::string &image_features,
                                     std::vector<vise::search_result> &result) const;
    void index_get_feature_match_details(const std::string &image_features,
                                         const uint32_t match_file_id,
                                         std::ostringstream &json) const;
    // visual group
    void create_vgroup(const std::unordered_map<std::string, std::string> &params,
                       const bool block_until_done,
                       bool &success, std::string &message) const;
    std::string get_vgroup_db_filename(const std::string vgroup_id) const;
    void get_vgroup_task_progress(const std::string vgroup_id,
                                  std::set<std::size_t> &query_fid_list,
                                  bool &success,
                                  std::string &message) const;
    void init_vgroup_db(const std::string vgroup_id,
                        std::unordered_map<std::string, std::string> &vgroup_metadata,
                        bool &success, std::string &message) const;
    void vgroup_match_graph(const std::string vgroup_id,
                            const std::unordered_map<std::string, std::string> &vgroup_metadata,
                            const std::vector<std::size_t> &query_id_list,
                            bool &success, std::string &message) const;

    void vgroup_connected_components(const std::string vgroup_id,
                                     const std::unordered_map<std::string, std::string> &vgroup_metadata,
                                     bool &success,
                                     std::string &message) const;

    void depth_first_search(const std::unordered_map<std::size_t, std::set<std::size_t> > &match_graph,
                            std::unordered_map<std::size_t, uint8_t> &vertex_flag,
                            std::size_t vertex,
                            std::set<std::size_t> &visited_nodes) const;
    void get_match_region(const vise::search_query &query,
                          const vise::search_result &result,
                          vise::search_query &match_region) const;
    void get_query_region(const std::size_t query_id,
                          vise::search_query &query) const;
    void is_visual_group_valid(const std::string vgroup_id,
                               bool &success,
                               std::string &message) const;
    void get_vgroup(const std::string vgroup_id,
                    std::unordered_map<std::string, std::string> const &param,
                    std::ostringstream &json) const;
    void get_vgroup_set(const std::string vgroup_id,
                        const std::string set_id_str,
                        std::ostringstream &json) const;
    void get_vgroup_set_with_file_id(const std::string vgroup_id,
                                     const std::string file_id_str,
                                     std::ostringstream &json) const;
    bool is_vgroup_available(const std::string vgroup_id) const;
  private:
    std::string d_pname;
    boost::filesystem::path d_project_dir;
    boost::filesystem::path d_data_dir;
    boost::filesystem::path d_image_dir;
    boost::filesystem::path d_image_src_dir;
    boost::filesystem::path d_image_small_dir;
    boost::filesystem::path d_app_dir;
    boost::filesystem::path d_tmp_dir;

    bool d_is_index_load_ongoing;
    bool d_is_metadata_ready;
    bool d_app_dir_exists;

    boost::filesystem::path d_pconf_fn;
    const std::map<std::string, std::string> d_conf;  // VISE application configuration
    std::map<std::string, std::string> d_pconf;       // project configuration
    std::unique_ptr<vise::search_engine> d_search_engine;
    std::unique_ptr<vise::metadata> d_metadata;

    std::mutex d_index_mutex;
    std::mutex d_index_load_mutex;

    project_state d_state;
    void state(project_state new_state);
    void state_update();
    std::string state_id_to_name(project_state state) const;

    void search_engine_init(std::string search_engine_name,
                            bool &success,
                            std::string &message);

    static const std::vector<std::string> d_preset_name_list;
    void init_default_conf();
    void init_preset_conf();
    bool init_project_data_dir(bool create_data_dir_if_missing=false);

    // visual group
    std::set<std::string> d_vgroup_id_list;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string> > d_vgroup_metadata_list;
    const std::string d_vgroup_task_progress_table;
    const std::string d_vgroup_match_table;
    const std::string d_vgroup_metadata_table;
    const std::string d_vgroup_region_table;
    const std::string d_vgroup_table;
    const std::string d_vgroup_inv_table;
    void init_vgroup_id_list();
    void load_vgroup_metadata(const std::string vgroup_id,
                              std::unordered_map<std::string, std::string> &vgroup_metadata) const;
  };
}
#endif
