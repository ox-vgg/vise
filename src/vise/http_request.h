/** @file   http_request.h
 *  @brief  Data structure and algorithm to parse and store http request
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   15 Nov 2017
 */

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>

namespace vise {
  enum class parser_state {WAITING_FOR_FIRST_REQUEST_CHUNK,
                           WAITING_FOR_HTTP_METHOD,
                           WAITING_FOR_REQUEST_URI,
                           WAITING_FOR_HTTP_VERSION,
                           VERSION_SEEN_WAITING_FOR_CR,
                           VERSION_SEEN_WAITING_FOR_LF,
                           WAITING_FOR_HTTP_HEADER_FIELD,
                           WAITING_FOR_HTTP_FIELD_VALUE,
                           WAITING_FOR_END_OF_HEADER_LF,
                           HEADER_SEEN_WAITING_FOR_PAYLOAD,
                           REQUEST_COMPLETE
  };

  class http_request {
  private:
    parser_state d_parser_state;
    bool d_header_has_expect_100_continue;
    std::size_t d_content_length;
    std::string d_unset_field_name;
    std::size_t d_payload_size;

  public:
    http_request() {
      d_parser_state = vise::parser_state::WAITING_FOR_FIRST_REQUEST_CHUNK;
      d_header_has_expect_100_continue = false;
      d_content_length = 0;
      d_payload_size = 0;
    };

    std::string d_method;
    std::string d_uri;
    std::string d_version;

    std::map<std::string, std::string> d_fields;
    std::map<std::string, std::string> d_multipart_data;
    std::map<std::string, std::string> uri_arg_;

    std::ostringstream d_payload;

    void reset_expect_100_continue_header() {
      d_header_has_expect_100_continue = false;
    }
    bool get_expect_100_continue_header() const {
      return d_header_has_expect_100_continue;
    }

    size_t payload_size() const {
      return d_payload_size;
    }

    void parse(std::string request_chunk);
    std::string print(bool include_payload=false) const;
    std::string current_state_name() const;
    void set_state_request_complete();
    bool is_request_complete() const;
    bool is_header_field_present(std::string field_name) const;
    std::string header_field_value(std::string field_name) const;

    bool parse_urlencoded_form_data();
    bool parse_multipart_form_data();
    size_t multipart_data_size() const {
      return d_multipart_data.size();
    }
    bool exists_multipart_data_name(std::string name) const;
    std::string multipart_data_value(std::string name) const;

    bool parse_uri(std::map<std::string,std::string>& uri_arg) const;
  };
}
#endif
