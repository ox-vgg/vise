/** @file task_progress.h
 *  @brief data structure to hold and maintain progress of a long running task
 *  @author Abhishek Dutta
 *  @date 5 Mar. 2020
 */
#ifndef TASK_PROGRESS_H
#define TASK_PROGRESS_H

#include <sstream>
#include <string>
#include <chrono>

namespace vise {
  class task_progress {
  public:
    int64_t d_value;
    int64_t d_max;
    std::string d_name;
    std::string d_message;
    bool d_has_started;
    bool d_is_complete;
    uint32_t d_tstart;
    uint32_t d_elapsed_ms;

    task_progress(std::string name);
    void start(int64_t value, int64_t max);
    void finish_success();
    void finish_error();
    void update(int64_t value, std::string message);
    void update(int64_t value);
    void add(int64_t value);
    uint32_t get_time_remaining();

    std::string to_json() const;
    uint32_t getmillisecs();
  };
}
#endif
