#ifndef VISE_HTTP_REQUEST_HEAD_H
#define VISE_HTTP_REQUEST_HEAD_H

#include <cstddef>
#include <string>
#include <algorithm>
#include <cctype>

namespace vise {
inline bool http_token_char(unsigned char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z') ||
         std::string("!#$%&'*+-.^_`|~").find(char(c)) != std::string::npos;
}

struct http_request_head {
  unsigned status;
  std::size_t content_length;
  std::string message;
  std::string canonical_header;
  http_request_head() : status(0), content_length(0) {}
};

// The server does not implement chunked bodies or request pipelining. Check
// framing before the existing request parser allocates an uploaded body.
inline http_request_head validate_http_request_head(const std::string& header,
                                                   std::size_t max_body) {
  http_request_head result;
  const std::size_t line_end = header.find("\r\n");
  const std::size_t method_end = header.find(' ');
  if(line_end==std::string::npos || method_end==std::string::npos || method_end>=line_end) {
    result.status=400; result.message="Malformed HTTP request line"; return result;
  }
  const std::string method = header.substr(0, method_end);
  if(method.empty() || !std::all_of(method.begin(), method.end(), http_token_char)) {
    result.status=400; result.message="Malformed HTTP method"; return result;
  }
  const std::size_t target_end=header.find(' ', method_end+1);
  if(target_end==std::string::npos || target_end==method_end+1 || target_end>=line_end ||
     (header.substr(target_end+1, line_end-target_end-1)!="HTTP/1.1" &&
      header.substr(target_end+1, line_end-target_end-1)!="HTTP/1.0")) {
    result.status=400; result.message="Malformed HTTP request line"; return result;
  }
  result.canonical_header=header.substr(0, line_end+2);
  for(std::size_t i=method_end+1; i<target_end; ++i) {
    if(static_cast<unsigned char>(header[i])<=32 || header[i]==127) {
      result.status=400; result.message="Malformed HTTP request target"; return result;
    }
  }
  bool saw_length=false;
  for(std::size_t start=line_end+2; start+2<header.size();) {
    const std::size_t end=header.find("\r\n", start);
    if(end==std::string::npos) { result.status=400; break; }
    if(end==start) break;
    const std::size_t colon=header.find(':', start);
    if(colon==std::string::npos || colon==start || colon>=end) { result.status=400; break; }
    std::string name=header.substr(start, colon-start);
    for(unsigned char c : name) {
      if(!http_token_char(c)) {
        result.status=400; result.message="Malformed HTTP header name"; return result;
      }
    }
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    std::size_t value_start=colon+1, value_end=end;
    while(value_start<value_end && (header[value_start]==' ' || header[value_start]=='\t')) ++value_start;
    while(value_end>value_start && (header[value_end-1]==' ' || header[value_end-1]=='\t')) --value_end;
    for(std::size_t i=value_start; i<value_end; ++i) {
      const unsigned char c=header[i];
      if((c<32 && c!='\t') || c==127) {
        result.status=400; result.message="Malformed HTTP header value"; return result;
      }
    }
    if(name=="transfer-encoding") { result.status=400; result.message="Transfer-Encoding is not supported"; return result; }
    if(name=="content-length") {
      if(saw_length || value_start==value_end) { result.status=400; break; }
      saw_length=true;
      for(std::size_t i=value_start; i<value_end; ++i) {
        const unsigned char c=header[i];
        if(c<'0' || c>'9') { result.status=400; break; }
        const unsigned digit=c-'0';
        if(result.content_length>max_body/10 ||
           (result.content_length==max_body/10 && digit>max_body%10)) {
          result.status=413; result.message="HTTP request body exceeds the size limit"; return result;
        }
        result.content_length=result.content_length*10+digit;
      }
      if(result.status) break;
    }
    const std::string canonical_name=name=="content-length" ? "Content-Length" :
                                     name=="content-type" ? "Content-Type" :
                                     name=="expect" ? "Expect" : header.substr(start, colon-start);
    std::string value=header.substr(value_start, value_end-value_start);
    if(name=="expect") {
      std::string lower=value;
      std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return char(std::tolower(c)); });
      if(lower=="100-continue") value=lower;
    }
    result.canonical_header+=canonical_name+": "+value+"\r\n";
    start=end+2;
  }
  if(result.status) result.message="Malformed or duplicate Content-Length";
  else if(result.content_length && method!="POST" && method!="PUT") {
    result.status=400; result.message="Request body is not supported for this method";
  }
  if(!saw_length && (method=="POST" || method=="PUT"))
    result.canonical_header+="Content-Length: 0\r\n";
  result.canonical_header+="\r\n";
  return result;
}
}
#endif
