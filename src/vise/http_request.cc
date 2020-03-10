#include "http_request.h"

std::string vise::http_request::print(bool include_payload) const {
  std::ostringstream s;
  s << "{" << d_method << " [" << d_uri << "]" << " " << d_version << "; {";
  for ( auto it = d_fields.begin(); it != d_fields.end(); it++ ) {
    s << "[" << it->first << ":" << it->second << "]";
  }
  s << "}";
  s << "{payload=" << payload_size() << " bytes}";
  s << "{d_parser_state=" << current_state_name() << "}";

  if(include_payload) {
    s << "{payload=" << d_payload.str() << "}";
  } else {
    s << "{payload=" << d_payload_size << " bytes}";
  }
  s << "}";
  return s.str();
}

void vise::http_request::parse(std::string request_chunk) {
  if ( d_parser_state == parser_state::WAITING_FOR_FIRST_REQUEST_CHUNK ) {
    if ( request_chunk.length() ) {
      d_parser_state = parser_state::WAITING_FOR_HTTP_METHOD;
    }
  }

  // @todo: for uploaded images as payload, it may be more efficient to
  // leave them in an array
  if ( d_parser_state == parser_state::HEADER_SEEN_WAITING_FOR_PAYLOAD ) {
    d_payload << request_chunk;
    if ( ((size_t) d_payload.tellp()) == d_content_length ) {
      set_state_request_complete();
    }
    return;
  }

  std::size_t extract = 0;
  for ( std::size_t i = 0; i != request_chunk.length(); i++ ) {
    switch( request_chunk[i] ) {
    case ' ':
      if ( d_parser_state == parser_state::WAITING_FOR_HTTP_METHOD ) {
        d_method = request_chunk.substr(extract, i - extract);
        extract = i + 1;
        d_parser_state = parser_state::WAITING_FOR_REQUEST_URI;
        continue;
      }
      if ( d_parser_state == parser_state::WAITING_FOR_REQUEST_URI ) {
        d_uri = request_chunk.substr(extract, i - extract);
        extract = i + 1;
        d_parser_state = parser_state::WAITING_FOR_HTTP_VERSION;
        continue;
      }
      break;

    case '\r':
      if ( d_parser_state == parser_state::WAITING_FOR_HTTP_VERSION ) {
        d_version = request_chunk.substr(extract, i - extract);
        d_parser_state = parser_state::VERSION_SEEN_WAITING_FOR_LF;
        continue;
      }
      if ( d_parser_state == parser_state::WAITING_FOR_HTTP_FIELD_VALUE ) {
        std::string field_value = request_chunk.substr(extract, i - extract);
        if ( field_value[0] == ' ' ) {
          field_value = field_value.substr(1); // remove prefix space
        }
        auto it = d_fields.find(d_unset_field_name);
        if( it != d_fields.end() ) {
          it->second = field_value;
          d_unset_field_name = "";
        } else {
          std::cerr << "\nmalformed http header value: "
                    << field_value << std::flush;
        }
        extract = i + 1;
        // to prepare for next (if any) http header field
        d_parser_state = parser_state::VERSION_SEEN_WAITING_FOR_LF;
        continue;
      }
      if ( d_parser_state == parser_state::WAITING_FOR_HTTP_HEADER_FIELD ) {
        d_parser_state = parser_state::WAITING_FOR_END_OF_HEADER_LF;
        continue;
      }

      break;
    case '\n':
      if ( d_parser_state == parser_state::VERSION_SEEN_WAITING_FOR_LF ) {
        extract = i + 1;
        d_parser_state = parser_state::WAITING_FOR_HTTP_HEADER_FIELD;
        continue;
      }
      if ( d_parser_state == parser_state::WAITING_FOR_END_OF_HEADER_LF ) {
        // browsers expect 100-continue response before sending large files
        if ( is_header_field_present("Expect") ) {
          if ( header_field_value("Expect") == "100-continue" ) {
            d_header_has_expect_100_continue = true;
          }
        }

        if ( (d_method == "POST" || d_method == "PUT") &&
             is_header_field_present("Content-Length") ) {
          std::istringstream s(header_field_value("Content-Length"));
          s >> d_content_length;
          d_payload << request_chunk.substr(i+1);

          if ( ((size_t) d_payload.tellp()) == d_content_length ) {
            set_state_request_complete();
            return;
          } else {
            d_parser_state = parser_state::HEADER_SEEN_WAITING_FOR_PAYLOAD;
            continue;
          }
        } else {
          d_parser_state = parser_state::REQUEST_COMPLETE;
        }
        continue;
      }

      break;
    case ':':
      if ( d_parser_state == parser_state::WAITING_FOR_HTTP_HEADER_FIELD ) {
        d_unset_field_name = request_chunk.substr(extract, i - extract);
        d_fields.insert( make_pair(d_unset_field_name, "_value_not_set_") );
        extract = i + 1;
        d_parser_state = parser_state::WAITING_FOR_HTTP_FIELD_VALUE;
        continue;
      }
      break;
    default:
      continue;
    }
  }
}

void vise::http_request::set_state_request_complete() {
  d_payload_size = d_payload.tellp();
  d_parser_state = parser_state::REQUEST_COMPLETE;
}

bool vise::http_request::is_request_complete() const {
  if ( d_parser_state == parser_state::REQUEST_COMPLETE ) {
    return true;
  } else {
    return false;
  }
}

bool vise::http_request::is_header_field_present(std::string field_name) const {
  const std::map<std::string, std::string>::const_iterator it = d_fields.find(field_name);
  if ( it != d_fields.end() ) {
    return true;
  } else {
    return false;
  }
}

std::string vise::http_request::header_field_value(std::string field_name) const {
  std::map<std::string, std::string>::const_iterator it = d_fields.find(field_name);
  if ( it != d_fields.end() ) {
    return it->second;
  } else {
    return "";
  }
}

std::string vise::http_request::current_state_name() const {
  switch(d_parser_state) {
  case vise::parser_state::WAITING_FOR_FIRST_REQUEST_CHUNK:
    return "WAITING_FOR_FIRST_REQUEST_CHUNK";
    break;
  case vise::parser_state::WAITING_FOR_HTTP_METHOD:
    return "WAITING_FOR_HTTP_METHOD";
    break;
  case vise::parser_state::WAITING_FOR_REQUEST_URI:
    return "WAITING_FOR_REQUEST_URI";
    break;
  case vise::parser_state::WAITING_FOR_HTTP_VERSION:
    return "WAITING_FOR_HTTP_VERSION";
    break;
  case vise::parser_state::VERSION_SEEN_WAITING_FOR_CR:
    return "VERSION_SEEN_WAITING_FOR_CR";
    break;
  case vise::parser_state::VERSION_SEEN_WAITING_FOR_LF:
    return "VERSION_SEEN_WAITING_FOR_LF";
    break;
  case vise::parser_state::WAITING_FOR_HTTP_HEADER_FIELD:
    return "WAITING_FOR_HTTP_HEADER_FIELD";
    break;
  case vise::parser_state::WAITING_FOR_HTTP_FIELD_VALUE:
    return "WAITING_FOR_HTTP_FIELD_VALUE";
    break;
  case vise::parser_state::WAITING_FOR_END_OF_HEADER_LF:
    return "WAITING_FOR_END_OF_HEADER_LF";
    break;
  case vise::parser_state::HEADER_SEEN_WAITING_FOR_PAYLOAD:
    return "HEADER_SEEN_WAITING_FOR_PAYLOAD";
    break;
  case vise::parser_state::REQUEST_COMPLETE:
    return "REQUEST_COMPLETE";
    break;
  default:
    return "UNKNOWN STATE";
  }
}

///
/// Multipart Form Data
///


bool vise::http_request::parse_urlencoded_form_data() {
  std::string ctype = header_field_value("Content-Type");
  if(ctype != "application/x-www-form-urlencoded") {
    return false;
  }
  std::string payload_str = d_payload.str();
  std::size_t start = 0;
  std::size_t amppos = payload_str.find('&', start);
  while(amppos != std::string::npos) {
    std::string param_i = payload_str.substr(start, amppos);
    std::size_t eqpos = param_i.find('=');
    if(eqpos != std::string::npos) {
      std::string key = param_i.substr(0, eqpos);
      std::string value = param_i.substr(eqpos+1);
      d_multipart_data.insert( std::pair<std::string, std::string>(key, value) );
    }
    start = amppos + 1;
    amppos = payload_str.find('&', start);
  }
  return true;
}

// see : https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Disposition
bool vise::http_request::parse_multipart_form_data() {
  std::string ctype = header_field_value("Content-Type");
  size_t bspos = ctype.find("boundary=");
  if( bspos == std::string::npos ) {
    return false;
  }

  std::string bs = ctype.substr(bspos + std::string("boundary=").length());
  size_t bslen = bs.length(); // length of boundary string

  std::string payload_str = d_payload.str();

  std::vector<size_t> bloc; // locations of boundary strings
  size_t bp = payload_str.find(bs);
  size_t old_bp = 0;
  do {
    bp = payload_str.find(bs, old_bp);
    if ( bp != std::string::npos ) {
      bloc.push_back(bp);
      old_bp = bp + bslen;
    }
  } while( bp != std::string::npos );

  if ( bloc.size() == 0 ) {
    return false;
  }

  size_t crlf_len = 2; // length of \r\n
  for(size_t i=0; i < bloc.size() - 1; i++) {
    size_t block_begin  = bloc.at(i) + bslen + crlf_len; // discard the trailing \r\n in first (n-1) boundaries
    size_t block_end    = bloc.at(i+1) - crlf_len;       // discard the preceding \r\n for n boundaries
    size_t block_length = block_end - block_begin;
    std::string block = payload_str.substr(block_begin, block_length);

    size_t name1 = block.find("name=");
    size_t name2 = block.find("\"", name1);
    size_t name3 = block.find("\"", name2 + 1);
    if ( name1 == std::string::npos || name2 == std::string::npos || name3 == std::string::npos ) {
      return false;
    }

    std::string block_name = block.substr(name2 + 1, name3 - name2 - 1);

    size_t value1 = block.find("\r\n\r\n");
    size_t value2 = block.rfind("\r\n");
    if ( value1 == std::string::npos || value2 == std::string::npos ) {
      return false;
    }
    std::string block_value = block.substr(value1 + 4, value2 - value1 - 4); // discard the trailing \r\n

    d_multipart_data.insert( make_pair(block_name, block_value) );
  }
  return true;
}

bool vise::http_request::exists_multipart_data_name(std::string name) const {
  const std::map<std::string, std::string>::const_iterator it = d_multipart_data.find(name);
  if ( it != d_multipart_data.end() ) {
    return true;
  } else {
    return false;
  }
}

std::string vise::http_request::multipart_data_value(std::string name) const {
  std::map<std::string, std::string>::const_iterator it = d_multipart_data.find(name);
  if ( it != d_multipart_data.end() ) {
    return it->second;
  } else {
    return "";
  }
}

bool vise::http_request::parse_uri(std::map<std::string,std::string>& uri_arg) const {
  uri_arg.clear();
  size_t start = d_uri.find("?");

  if( start != std::string::npos ) {
    bool parsing_key = true;
    start = start + 1;
    std::string key;
    std::string value;
    for( size_t i=start; i<d_uri.length(); i++ ) {
      if( d_uri.at(i) == '=' && parsing_key) {
        key = d_uri.substr(start, i - start);
        start = i + 1;
        parsing_key = false;
      }
      if( d_uri.at(i) == '&' && !parsing_key) {
        value = d_uri.substr(start, i - start);
        uri_arg.insert( std::pair<std::string,std::string>(key, value) );
        start = i + 1;
        parsing_key = true;
      }
    }
    value = d_uri.substr(start);
    uri_arg.insert( std::pair<std::string,std::string>(key, value) );
    return true;
  } else {
    return false;
  }
}
