#include "task_progress.h"

#include <iostream>

vise::task_progress::task_progress(std::string name) {
    d_name = name;
    d_value = 0;
    d_max = 0;
    d_message = "";
    d_is_complete = false;
    d_has_started = false;
}

void vise::task_progress::start(int64_t value, int64_t max) {
    d_value = value;
    d_max = max;
    d_is_complete = false;
    d_has_started = true;
    d_tstart = getmillisecs();
    d_elapsed_ms = 0;
}
void vise::task_progress::finish_success() {
    d_value = d_max;
    d_is_complete = true;
    d_has_started = true;
    d_elapsed_ms = getmillisecs() - d_tstart;
}

void vise::task_progress::finish_error() {
    d_is_complete = false;
    d_has_started = false;
    d_elapsed_ms = getmillisecs() - d_tstart;
}

void vise::task_progress::update(int64_t value, std::string message) {
    d_value = value;
    d_message = message;
    d_elapsed_ms = getmillisecs() - d_tstart;
}

void vise::task_progress::update(int64_t value) {
    d_value = value;
    d_message = "";
    d_elapsed_ms = getmillisecs() - d_tstart;
}

void vise::task_progress::add(int64_t value) {
    d_value = d_value + value;
    d_message = "";
    d_elapsed_ms = getmillisecs() - d_tstart;
}

uint32_t vise::task_progress::get_time_remaining() {
  double elapsed_ms = (double) (getmillisecs() - d_tstart);
  double avg_time_per_step = elapsed_ms / ((double) d_value);
  uint32_t remaining_ms = (uint32_t) ( (avg_time_per_step * ((double) (d_max-1))) - elapsed_ms );
  return remaining_ms;
}

std::string vise::task_progress::to_json() const {
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
uint32_t vise::task_progress::getmillisecs() {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}
