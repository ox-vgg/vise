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
#include <stdexcept>

namespace vise {
  void configuration_load(std::string filename,
                          std::map<std::string, std::string> &conf );

  void configuration_show(std::map<std::string, std::string> const &conf);
}
#endif
