/** @file search_engine.h
 *  @brief An interface for visual search engine modules
 *  @author Abhishek Dutta
 *  @date 19 Nov. 2019
 */
#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include "search_query.h"
#include "search_result.h"

#include <iostream>
#include <vector>
#include <functional>

namespace vise {
  class search_engine {
  public:
    search_engine(std::string se_name);
    virtual ~search_engine();
    virtual void index_create(bool &success,
                              std::string &message,
                              std::function<void(void)> callback) = 0;
    virtual void index_load(bool &success, std::string &message) = 0;
    virtual void index_unload(bool &success, std::string &message) = 0;
    virtual bool index_is_loaded() const = 0;
    virtual bool index_is_done() const = 0;
    virtual bool index_is_incomplete() const = 0;
    virtual bool index_is_ongoing() const = 0;
    virtual std::string index_status() const = 0;

    virtual void index_search(vise::search_query const &q,
                              std::vector<vise::search_result> &r) const = 0;
    virtual void index_internal_match(vise::search_query const &q,
                                      uint32_t match_file_id,
                                      std::ostringstream &json) const = 0;

    virtual void register_image(uint32_t file1_id, uint32_t file2_id,
                                uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                std::array<double, 9> &H) const;

    virtual uint32_t fid_count() const = 0;
    virtual uint32_t fid(std::string filename) const = 0;
    virtual std::string filename(uint32_t fid) const = 0;

  private:
    const std::string d_se_name;
  };
}
#endif
