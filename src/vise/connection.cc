#include "connection.h"
#include "http_request_head.h"

const std::string vise::connection::crlf = "\r\n";
const std::string vise::connection::crlf2 = "\r\n\r\n";

const std::string vise::connection::http_100 = "HTTP/1.1 100 Continue\r\n\r\n";
const std::string vise::connection::http_200 = "HTTP/1.1 200 OK\r\n";
const std::string vise::connection::http_301 = "HTTP/1.1 301 Moved Permanently\r\n";
const std::string vise::connection::http_400 = "HTTP/1.1 400 Bad Request\r\n";

vise::connection::connection(boost::asio::io_service &io_service,
                             vise::project_manager *manager,
                             std::size_t max_header_size,
                             std::size_t max_body_size)
  :d_strand(io_service),
   d_socket(io_service),
   d_max_header_size(max_header_size),
   d_max_body_size(max_body_size),
   d_header_complete(false),
   d_expected_body_size(0),
   d_received_body_size(0),
   d_continue_pending(false),
   d_final_response_waiting(false),
   d_manager(manager)
{
}

boost::asio::ip::tcp::socket& vise::connection::socket() {
  return d_socket;
}

void vise::connection::process_connection() {
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint ep = d_socket.remote_endpoint(ec);
  if ( ec ) {
    close_connection();
    return;
  }

  d_socket.async_read_some(boost::asio::buffer( d_buffer ),
                          d_strand.wrap( boost::bind(&connection::on_request_data, shared_from_this(),
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred
                                                    )
                                        )
                          );

  // CRITICAL
  // @todo: what happens if the remote connection does not send any data?
  // this may lead to an orphan connection
  // include a timer which kills this connection automatically after TIMEOUT_SEC
  // ref: https://stackoverflow.com/questions/42487847/boost-asio-async-read-some-timeout
}

void vise::connection::on_request_data(const boost::system::error_code& e, std::size_t bytes_read) {
  boost::system::error_code ec;
  if ( bytes_read == 0 ) {
    close_connection();
    return;
  }
  std::string request_chunk( d_buffer.data(), d_buffer.data() + bytes_read );
  if(!d_header_complete) {
    d_header_buffer.append(request_chunk);
    const std::size_t end=d_header_buffer.find(crlf2);
    if((end==std::string::npos && d_header_buffer.size()>d_max_header_size) ||
       (end!=std::string::npos && end+crlf2.size()>d_max_header_size)) {
      d_response.set_status(413);
      d_response.set_payload("HTTP request header exceeds the size limit");
      send_response(); return;
    }
    if(end==std::string::npos) {
      d_socket.async_read_some(boost::asio::buffer(d_buffer),
        d_strand.wrap(boost::bind(&connection::on_request_data, shared_from_this(),
                                 boost::asio::placeholders::error,
                                 boost::asio::placeholders::bytes_transferred)));
      return;
    }
    const std::size_t head_size=end+crlf2.size();
    const std::string head=d_header_buffer.substr(0, head_size);
    const http_request_head framing=validate_http_request_head(head, d_max_body_size);
    if(framing.status) {
      d_response.set_status(framing.status);
      d_response.set_payload(framing.message);
      send_response(); return;
    }
    d_header_complete=true;
    d_expected_body_size=framing.content_length;
    d_received_body_size=d_header_buffer.size()-head_size;
    request_chunk=framing.canonical_header+d_header_buffer.substr(head_size);
    d_header_buffer.clear();
  } else {
    if(request_chunk.size()>d_expected_body_size-d_received_body_size) {
      d_response.set_status(400); d_response.set_payload("HTTP body exceeds Content-Length"); send_response(); return;
    }
    d_received_body_size+=request_chunk.size();
  }
  if(d_received_body_size>d_expected_body_size) {
    d_response.set_status(400); d_response.set_payload("HTTP body exceeds Content-Length"); send_response(); return;
  }
  d_request.parse(request_chunk);

  if ( d_request.get_expect_100_continue_header() ) {
    if(!d_request.is_request_complete() && d_expected_body_size)
      response_http_100();
    d_request.reset_expect_100_continue_header();
  }

  if ( d_request.is_request_complete() ) {
    // process http request here
    boost::asio::ip::tcp::endpoint ep = d_socket.remote_endpoint(ec);
    if ( ec ) {
      return;
    }

    /*
    if(d_request.d_method == "POST") {
      d_request.parse_urlencoded_form_data();
      d_request.parse_multipart_form_data();
    }
    */
    d_manager->process_http_request(d_request, d_response);
    send_response();
    return;
  } else {
    // fetch more chunks of http request to get the complete http request
    d_socket.async_read_some(boost::asio::buffer( d_buffer ),
                            d_strand.wrap( boost::bind(&connection::on_request_data, shared_from_this(),
                                                      boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred
                                                      )
                                          )
                            );
    return;
  }
}

void vise::connection::send_response() {
  if(d_continue_pending) {
    d_final_response_waiting=true;
    return;
  }
  std::ostream http_response( &d_response_buffer );
  http_response << d_response.d_status << connection::crlf;

  for( auto it = d_response.d_fields.begin(); it != d_response.d_fields.end(); it++ ) {
    http_response << it->first << ": " << it->second << connection::crlf;
  }
  http_response << connection::crlf << d_response.d_payload;

  boost::asio::async_write(d_socket, d_response_buffer.data(),
                           d_strand.wrap(boost::bind(&connection::on_response_write,
                                                    shared_from_this(),
                                                    boost::asio::placeholders::error
                                                    )
                                        )
                           );
}

void vise::connection::response_http_100() {
  d_continue_pending=true;
  std::ostream http_response( &d_continue_response_buffer );
  http_response << connection::http_100;

  boost::asio::async_write(d_socket, d_continue_response_buffer.data(),
                           d_strand.wrap(boost::bind(&connection::on_http_100_response_write,
                                                    shared_from_this(),
                                                    boost::asio::placeholders::error
                                                    )
                                        )
                           );
}

void vise::connection::on_http_100_response_write(const boost::system::error_code& e) {
  d_continue_pending=false;
  d_continue_response_buffer.consume(d_continue_response_buffer.size());
  if ( e ) {
    std::cerr << "\nfailed to send 100 continue response" << std::endl;
    close_connection(); return;
  }
  if(d_final_response_waiting)
    send_response();
}

void vise::connection::on_response_write(const boost::system::error_code& e) {
  if ( e ) {
    std::cerr << "\nfailed to send http response: " << e.message() << std::endl;
  }

  close_connection();
}

void vise::connection::close_connection() {
  boost::system::error_code ec;
  if ( d_socket.is_open() ) {
    d_socket.shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec );
    d_socket.close(ec);
  }
}
