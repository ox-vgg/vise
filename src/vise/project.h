/** @file project.h
 *  @brief denotes a VISE project
 *  @author Abhishek Dutta
 *  @date 15 Nov. 2019
 */
#ifndef PROJECT_H
#define PROJECT_H

#include "search_engine.h"
#include "search_query.h"
#include "search_result.h"
#include "vise_util.h"
#include "relja_retrival/relja_retrival.h"

#include <memory>
#include <thread>
#include <exception>

#include <boost/filesystem.hpp>

namespace vise {
  enum class SEARCH_ENGINE_ID { RELJA_RETRIVAL, _RESERVED_FOR_FUTURE };
  enum class PROJECT_STATE { UNKNOWN, INIT, INDEX, SEARCH };

  class project {
  public:
    project(std::string pname,
            std::map<std::string, std::string> const &conf);
    project(project const &p);
    ~project();
    void index_create(bool &success, std::string &message);
    void index_load(bool &success, std::string &message);
    void index_unload(bool &success, std::string &message);
    bool index_is_loaded();
    bool index_is_done();
    void index_search(vise::search_query const &query,
                      std::vector<vise::search_result> &result);

  private:
    std::string d_pname;
    boost::filesystem::path d_storedir;
    boost::filesystem::path d_datadir;
    boost::filesystem::path d_pconf_fn;

    const std::map<std::string, std::string> d_conf;  // VISE application configuration
    std::map<std::string, std::string> d_pconf;       // project configuration
    std::unique_ptr<vise::search_engine> d_search_engine;

    std::thread d_index_thread;
    std::mutex d_index_mutex;
    std::mutex d_index_load_mutex;

    void search_engine_init(std::string search_engine_name,
                            bool &success,
                            std::string &message);
    void conf_reload();
  };
}
#endif
