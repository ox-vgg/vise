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
    uint32_t d_value;
    uint32_t d_max;
    std::string d_name;
    std::string d_message;
    bool d_has_started;
    bool d_is_complete;
    uint32_t d_tstart;
    uint32_t d_elapsed_ms;

    task_progress(std::string name) {
      d_name = name;
      d_value = 0;
      d_max = 0;
      d_message = "";
      d_is_complete = false;
      d_has_started = false;
    }
    inline void start(uint32_t value, uint32_t max) {
      d_value = value;
      d_max = max;
      d_is_complete = false;
      d_has_started = true;
      d_tstart = getmillisecs();
      d_elapsed_ms = 0;
    }
    inline void finish_success() {
      d_value = d_max;
      d_is_complete = true;
      d_has_started = true;
      d_elapsed_ms = getmillisecs() - d_tstart;
    }

    inline void finish_error() {
      d_is_complete = false;
      d_has_started = false;
      d_elapsed_ms = getmillisecs() - d_tstart;
    }

    inline void update(uint32_t value, std::string message) {
      d_value = value;
      d_message = message;
      d_elapsed_ms = getmillisecs() - d_tstart;
    }

    inline void update(uint32_t value) {
      d_value = value;
      d_message = "";
      d_elapsed_ms = getmillisecs() - d_tstart;
    }

    inline void add(uint32_t value) {
      d_value = d_value + value;
      d_message = "";
      d_elapsed_ms = getmillisecs() - d_tstart;
    }

    std::string to_json() const {
      std::ostringstream ss;
      ss << "{\"name\":\"" << d_name << "\""
         << ",\"value\":" << d_value
         << ",\"max\":" << d_max
         << ",\"message\":\"" << d_message << "\""
         << ",\"is_complete\":" << d_is_complete
         << ",\"has_started\":" << d_has_started
         << ",\"elapsed_ms\":" << d_elapsed_ms
         << "}";
      return ss.str();
    }
    uint32_t getmillisecs() {
      std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
      return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
    }

  };
}
#endif
