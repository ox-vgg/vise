#include "http_request_head.h"
#include "http_request.h"
#include <iostream>
#include <stdexcept>

static void require(bool condition, const char* message) {
  if(!condition) throw std::runtime_error(message);
}

static void status(const std::string& fields, unsigned expected) {
  const vise::http_request_head head=vise::validate_http_request_head(
    "POST /_project_create HTTP/1.1\r\n"+fields+"\r\n", 256);
  require(head.status==expected, fields.c_str());
}

int main() {
  try {
    status("Content-Length: 0\r\n", 0);
    status("content-length: 256\r\n", 0);
    status("Content-Length: 257\r\n", 413);
    status("Content-Length: 184467440737095516160\r\n", 413);
    status("Content-Length: -1\r\n", 400);
    status("Content-Length: +1\r\n", 400);
    status("Content-Length: 1junk\r\n", 400);
    status("Content-Length: \r\n", 400);
    status("Content-Length: 1\r\ncontent-length: 1\r\n", 400);
    status("Transfer-Encoding: chunked\r\n", 400);
    status("transfer-encoding: identity\r\n", 400);
    status("Content-Length : 1\r\n", 400);
    status("X-Invalid: a\nb\r\n", 400);
    status(std::string("X-Invalid: a\0b\r\n", 17), 400);
    status("Content-Length:\t 0 \t\r\nexpect:\t100-Continue\t\r\n", 0);
    const vise::http_request_head expect=vise::validate_http_request_head(
      "POST /test HTTP/1.1\r\nexpect:\t100-Continue\t\r\n\r\n", 256);
    vise::http_request empty_post;
    empty_post.parse(expect.canonical_header);
    require(empty_post.is_request_complete() && empty_post.get_expect_100_continue_header(),
            "Empty POST or Expect normalization failed");
    const vise::http_request_head get=vise::validate_http_request_head(
      "GET /settings HTTP/1.1\r\nContent-Length: 1\r\n\r\n", 256);
    require(get.status==400, "GET request body accepted");
    const std::string payload("a\0b\r\n\r\nc", 8);
    const vise::http_request_head head=vise::validate_http_request_head(
      "POST /_project_create HTTP/1.1\r\ncontent-length: 8\r\ncontent-type: application/octet-stream\r\n\r\n", 256);
    require(head.status==0 && head.content_length==payload.size(), "Lowercase header rejected");
    for(std::size_t split=0; split<=payload.size(); ++split) {
      vise::http_request request;
      request.parse(head.canonical_header+payload.substr(0, split));
      if(split<payload.size()) {
        require(!request.is_request_complete(), "Body completed prematurely");
        request.parse(payload.substr(split));
      }
      require(request.is_request_complete() && request.payload_size()==payload.size() &&
              request.d_payload.str()==payload, "Binary body was not preserved across fragments");
      require(request.header_field_value("Content-Type")=="application/octet-stream", "Content-Type not normalized");
    }
    return 0;
  } catch(const std::exception& error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }
}
