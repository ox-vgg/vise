/** @file   http_response.h
 *  @brief  Data structure to store http response
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   17 Nov 2017
 */

#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include <iostream>
#include <sstream>
#include <map>
#include <algorithm> // for ::transform

namespace vise {
  class http_response {
  public:
    std::string d_status;
    unsigned int d_status_code;
    std::map<std::string,std::string> d_fields;
    std::string d_payload;

    http_response() : d_status("HTTP/1.1 200 OK") {
      d_status_code = 200;
    }

    void set_status(unsigned int response_code) {
      d_status = "HTTP/1.1 ";
      d_status_code = response_code;
      switch(response_code) {
      case 100:
        d_status += "100 Continue";
        break;
      case 200:
        d_status += "200 OK";
        break;
      case 303:
        d_status += "303 See Other";
        break;
      case 404:
        d_status += "404 Not Found";
        break;
      case 400:
        d_status += "400 Bad Request";
        break;
      case 412:
        d_status += "412 Precondition Failed";
        break;
      case 413:
        d_status += "413 Payload Too Large";
        break;
      default:
        d_status += "400 Bad Request";
        d_status_code = 400;
      }
    }
    void set_field(std::string name, std::string value) {
      d_fields.insert( make_pair(name, value) );
    }

    void set_content_type_from_filename(std::string filename) {
      std::string ctype = "application/octet-stream";
      size_t dot = filename.rfind(".");
      std::string ext = filename.substr(dot);
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

      if( ext == ".html" ) {
        ctype = "text/html";
      }
      else if( ext == ".jpg" || ext == ".jpeg" ) {
        ctype = "image/jpeg";
      }
      else if( ext == ".png" ) {
        ctype = "image/png";
      }
      else if( ext == ".css" ) {
        ctype = "text/css";
      }
      else if( ext == ".js" ) {
        ctype = "application/javascript";
      }
      else if( ext == ".txt" ) {
        ctype = "text/plain";
      }
      else if( ext == ".ico" ) {
        ctype = "image/x-icon";
      }
      else if( ext == ".json" ) {
        ctype = "application/json";
      }
      set_field("Content-Type", ctype);
    }

    void set_payload(std::string payload) {
      d_payload.assign(payload);
      d_fields["Content-Length"] = std::to_string(d_payload.length());
    }

    void set_html_payload(std::string payload) {
      d_payload.assign(payload);
      d_fields["Content-Length"] = std::to_string(d_payload.length());
      set_field("Content-Type", "text/html");
    }

    void set_json_payload(std::string payload) {
      d_payload.assign(payload);
      d_fields["Content-Length"] = std::to_string(d_payload.length());
      set_field("Content-Type", "application/json");
    }

    void set_text_payload(std::string payload) {
      d_payload.assign(payload);
      d_fields["Content-Length"] = std::to_string(d_payload.length());
      set_field("Content-Type", "text/plain");
    }

    void set_binary_payload(std::string payload) {
      d_payload.assign(payload);
      d_fields["Content-Length"] = std::to_string(d_payload.length());
      set_field("Content-Type", "application/octet-stream");
    }

    std::string get_response_str() {
      std::ostringstream s;
      s << d_status << "\r\n";
      for( auto it = d_fields.begin(); it != d_fields.end(); it++ ) {
        s << it->first << ": " << it->second << "\r\n";
      }
      s << "\r\n" << d_payload;
      return s.str();
    }
    void redirect_to(std::string location) {
      set_status(303);
      set_field("Location", location);
    }
  };
}
#endif
