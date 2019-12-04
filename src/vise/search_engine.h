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

namespace vise {
  class search_engine {
  public:
    search_engine(std::string se_name);
    virtual ~search_engine();
    virtual void index_create(bool &success, std::string &message) = 0;
    virtual void index_load(bool &success, std::string &message) = 0;
    virtual void index_unload(bool &success, std::string &message) = 0;
    virtual bool index_is_loaded() = 0;
    virtual bool index_is_done() = 0;
    virtual void index_search(vise::search_query const &q,
                              std::vector<vise::search_result> &r) = 0;

  private:
    const std::string d_se_name;
  };
}
#endif
