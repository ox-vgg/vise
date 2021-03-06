/** @file metadata.h
 *  @brief manages metadata associated with all images in a VISE project
 *  @author Abhishek Dutta
 *  @date 17 Nov. 2020
 */
#ifndef METADATA_H
#define METADATA_H

#include "vise_util.h"

#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <algorithm>

#include <boost/filesystem.hpp>

#include <sqlite3.h>

namespace vise {
  class metadata {
  public:
    metadata(std::string pname,
             boost::filesystem::path project_data_dir);
    ~metadata();
    bool is_metadata_available() const {
      return d_is_metadata_available;
    };
    bool is_region_metadata_available() const;
    void init_metadata_db();
    void create_metadata_db();
    bool sqlite_table_exists(const std::string table_name) const;

    // full text search (fts) of metadata
    void init_metadata_fts_vtable();
    void init_file_attribute_name_list();
    void init_region_attribute_name_list();

    void file_metadata_full_text_search(const std::string query,
                                        std::vector<uint32_t> &flist);
    void file_metadata_full_text_search_group_stat(const std::string query,
                                                   const std::string groupby,
                                                   std::map<std::string, uint32_t> &group_stat) const;
    bool full_text_search_query_to_sql(const std::string query,
                                       std::string &sql) const ;

    // file and region metadata
    void init_metadata_conf();
    void file_metadata_as_json(const uint32_t file_id,
                               std::ostringstream &json) const;
    void region_metadata_as_json(const uint32_t file_id,
                               std::ostringstream &json) const;
    void get_region_shape(const uint32_t region_id,
                          std::size_t &file_id,
                          std::size_t &region_index,
                          std::string &region_shape,
                          std::string &region_points) const;
    void get_region_file_info(const uint32_t region_id,
                              std::size_t &file_id,
                              std::size_t &region_index) const;

    void metadata_conf_as_json(std::ostringstream &json) const;

    void file_attribute_name_list(std::vector<std::string> &file_attribute_list) const;
    void write_metadata_conf(std::ostringstream &ss) const;
    void select_fid_with_filename_like(const std::string filename_pattern,
                                       std::vector<std::size_t> &file_id_list) const;
    void select_rid_with_filename_like(const std::string filename_pattern,
                                       std::vector<std::size_t> &region_id_list) const;

    void select_all_regions(const std::vector<std::size_t> &fid_list,
                            std::vector<std::size_t> &region_id_list) const;
    // group by
    void metadata_group_stat(const std::string groupby,
                             std::map<std::string, uint32_t> &group_stat) const;
    void metadata_groupby(const std::string groupby,
                          const std::string group,
                          std::vector<uint32_t> &flist) const;

    // util
    std::string get_metadata_db_fn() const {
      return d_metadata_db_fn.string();
    };
    void get_copy_of_metadata(const std::string source_table_name,
                              const std::string destination_db_filename,
                              const std::string destination_table_name,
                              bool &success,
                              std::string &message) const;
  private:
    std::string d_pname;
    boost::filesystem::path d_project_data_dir;
    boost::filesystem::path d_metadata_db_fn;
    boost::filesystem::path d_metadata_conf_fn;

    std::map<std::string, std::string> d_pconf;       // project configuration

    // metadata
    const std::string d_file_metadata_table_name;
    const std::string d_region_metadata_table_name;
    const std::string d_metadata_fts_vtable_name;
    const std::string d_concatenated_file_metadata_col_name;
    bool d_is_metadata_available;
    std::string d_metadata_conf;

    // sqlite3
    sqlite3 *d_db = nullptr;
    int d_sqlite_db_status;

    std::vector<std::string> d_file_attribute_name_list;
    std::vector<std::string> d_region_attribute_name_list;

    std::string table_name_from_attribute_name(const std::string aname) const;
  };
}
#endif
