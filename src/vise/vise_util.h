/** @file util.h
 *  @brief Various utility functions used by VISE
 *  @author Abhishek Dutta
 *  @date 12 Nov. 2019
 */

#ifndef VISE_UTIL_H
#define VISE_UTIL_H

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <ctime>
#include <chrono>
#include <cstdlib>

#include <boost/filesystem.hpp>

namespace vise {
  // VISE configuration
  bool configuration_load(std::string filename,
                          std::map<std::string, std::string> &conf );
  bool configuration_save(std::map<std::string, std::string> &conf,
                          std::string filename);
  void configuration_show(std::map<std::string, std::string> const &conf);

  boost::filesystem::path vise_home();

  // string
  bool starts_with(const std::string &s, const std::string prefix);
  bool ends_with(const std::string &s, const std::string suffix);
  bool contains(const std::string &s, const std::string substr);
  std::vector<std::string> split(const std::string &s, const char separator);
  void split(const std::string &s,
             const char separator,
             const std::size_t start,
             const std::size_t stop,
             std::vector<std::string> &chunks);
  void decompose_uri(const std::string &uri,
                     std::vector<std::string>& uri_components,
                     std::map<std::string, std::string>& uri_param);
  void parse_urlencoded_form(const std::string &formdata_str,
                             std::map<std::string, std::string>& formdata);

  // file
  bool file_load(const boost::filesystem::path fn,
                 std::string& file_content);
  bool file_save(const boost::filesystem::path fn,
                 std::string& file_content);

  // URI decoding
  // e.g. "http%3A%2F%2Ffoo%20bar%2F" -> "http://foo bar/"
  bool url_decode(const std::string& in, std::string& out);

  std::string json_escape_str(const std::string &in);

  // print
  template<typename T>
  void print_vector( std::string name, std::vector<T> const &v ) {
    if (v.size() == 0) {
      return;
    }
    std::ostringstream s;
    s << v[0];
    for ( std::size_t i = 1; i < v.size(); ++i ) {
      s << "," << v[i];
    }
    std::cout << name << " = [" << s.str() << "]" << std::endl;
  }

  void print_map(std::string name,
                 std::map<std::string, std::string> const &m );

  // timing and profiling
  std::string now_timestamp();
  uint32_t getmillisecs();
}
#endif
