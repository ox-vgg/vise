/** @file util.h
 *  @brief Various utility functions used by VISE
 *  @author Abhishek Dutta
 *  @date 12 Nov. 2019
 */

#ifndef VISE_UTIL_H
#define VISE_UTIL_H

#include <string>
#include <map>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <omp.h>
#include <cmath>

#include <boost/filesystem.hpp>
#include <Magick++.h>

/*
#ifndef ASSERT
#define ASSERT(expression) if (!(expression)) { std::cerr << "Precondition failed: " #expression " in "  << __FUNCTION__ << " (" __FILE__ ":" << __LINE__ << ")\n"; exit(1); }
#endif
*/

namespace vise {
  // command line interface (CLI)
  bool parse_cli_args(int argc, char **argv,
                      std::unordered_map<std::string, std::string> &cli_args,
                      std::unordered_map<std::string, std::string> &pname_pconf_list);

  // VISE settings
  void init_vise_settings_comments(std::map<std::string, std::string> &vise_settings);
  void init_vise_settings(std::map<std::string, std::string> &vise_settings);
  bool does_vise_home_and_store_exist(std::map<std::string, std::string> &vise_settings);
  bool create_vise_home_and_store(std::map<std::string, std::string> &vise_settings);
  void init_default_vise_settings(std::map<std::string, std::string> &vise_settings);

  // VISE configuration
  bool configuration_load(std::string filename,
                          std::map<std::string, std::string> &conf );
  bool configuration_save(std::map<std::string, std::string> &conf,
                          std::string filename);
  void configuration_show(std::map<std::string, std::string> const &conf);
  std::string configuration_get(std::string key);
  uint32_t configuration_get_nthread();
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
  void escape_string(std::string &in, char match_char, std::string replace_match_with);

  // file
  bool file_load(const boost::filesystem::path fn,
                 std::string& file_content);
  bool file_save(const boost::filesystem::path fn,
                 std::string& file_content);
  bool file_save_binary(const boost::filesystem::path fn,
                        std::string& file_content);
  // URI decoding
  // e.g. "http%3A%2F%2Ffoo%20bar%2F" -> "http://foo bar/"
  bool url_decode(const std::string& in, std::string& out);
  bool decode_uri_query_param(const std::string& in, std::string& out); // '+' is decoded as space ' '
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

  // check if input image is valid
  bool is_valid_image(std::string img_fn, std::string &message);
  bool if_valid_get_image_size(std::string img_fn, std::string &message, uint32_t &width, uint32_t &height);

  // parse string
  void csv_string_to_float_array(std::string csv_string,
                                 std::vector<float> &float_array);

  double iou(std::vector<float> &a, std::vector<float> &b);

}
#endif
