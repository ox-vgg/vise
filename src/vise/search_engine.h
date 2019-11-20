/** @file search_engine.h
 *  @brief An interface for visual search engine modules
 *  @author Abhishek Dutta
 *  @date 19 Nov. 2019
 */
#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include <iostream>

namespace vise {
  class search_engine {
  public:
    search_engine(std::string se_name);
    virtual ~search_engine();
    virtual void index() = 0;
    virtual void search() = 0;

  private:
    const std::string d_se_name;
  };
}
#endif
