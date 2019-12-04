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

namespace vise {
  class search_query {
  public:
    uint32_t d_file_id;
    uint32_t d_x;
    uint32_t d_y;
    uint32_t d_width;
    uint32_t d_height;

    search_query() {
      d_file_id = 0;
      d_x = 0;
      d_y = 0;
      d_width = 0;
      d_height = 0;
    }

    search_query(std::map<std::string, std::string> const &param) {
      if (param.count("file_id") &&
          param.count("x") &&
          param.count("y") &&
          param.count("width") &&
          param.count("height")) {
        std::stringstream ss;
        ss << param.at("file_id");
        ss >> d_file_id;
        ss.clear();

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
      }
    }
  };
}
#endif
