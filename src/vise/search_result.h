/** @file search_result.h
 *  @brief data structure to hold a result of search query
 *  @author Abhishek Dutta
 *  @date 22 Nov. 2019
 */
#ifndef SEARCH_RESULT_H
#define SEARCH_RESULT_H

#include <string>
#include <array>

namespace vise {
  class search_result {
  public:
    uint32_t d_file_id;
    std::string d_filename;
    float d_score;
    std::array<double, 9> d_H;

    search_result(uint32_t file_id,
                  std::string filename,
                  float score,
                  std::array<double, 9> H)
      : d_file_id(file_id), d_filename(filename), d_score(score), d_H(H) {
    }
  };
}
#endif
