/** @file search_query.h
 *  @brief defines a search query
 *  @author Abhishek Dutta
 *  @date 22 Nov. 2019
 */
#ifndef SEARCH_QUERY_H
#define SEARCH_QUERY_H

#include <sstream>
#include <map>
#include <string>
#include <sstream>

namespace vise {
  class search_query {
  public:
    uint32_t d_file_id;
    uint32_t d_x;
    uint32_t d_y;
    uint32_t d_width;
    uint32_t d_height;
    std::string d_filename;
    bool is_region_query;
    uint32_t d_max_result_count;

    search_query() {
      d_file_id = 0;
      d_x = 0;
      d_y = 0;
      d_width = 0;
      d_height = 0;
      is_region_query = false;
      d_max_result_count = 0;
    }

    search_query(std::map<std::string, std::string> const &param) {
      d_file_id = 0;
      d_x = 0;
      d_y = 0;
      d_width = 0;
      d_height = 0;
      is_region_query = false;
      d_max_result_count = 0;

      std::stringstream ss;
      if (param.count("file_id")) {
        ss << param.at("file_id");
        ss >> d_file_id;
        ss.clear();
      }

      if (param.count("x") &&
          param.count("y") &&
          param.count("width") &&
          param.count("height")) {
        is_region_query = true;
        ss << param.at("x");
        ss >> d_x;
        ss.clear();

        ss << param.at("y");
        ss >> d_y;
        ss.clear();

        ss << param.at("width");
        ss >> d_width;
        ss.clear();

        ss << param.at("height");
        ss >> d_height;
      } else {
        is_region_query = false;
      }

      if(param.count("max_result_count")) {
        ss << param.at("max_result_count");
        ss >> d_max_result_count;
      }
    }

    std::string to_json() {
      std::ostringstream ss;
      ss << "{\"file_id\":" << d_file_id
         << ",\"filename\":\"" << d_filename << "\"";
      if(is_region_query) {
        ss << ",\"x\":" << d_x
           << ",\"y\":" << d_y
           << ",\"width\":" << d_width
           << ",\"height\":" << d_height;
      }
      ss << "}";
      return ss.str();
    }
  };
}
#endif
