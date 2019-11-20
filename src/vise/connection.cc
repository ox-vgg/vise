#include "connection.h"

const std::string vise::connection::crlf = "\r\n";
const std::string vise::connection::crlf2 = "\r\n\r\n";

const std::string vise::connection::http_100 = "HTTP/1.1 100 Continue\r\n";
const std::string vise::connection::http_200 = "HTTP/1.1 200 OK\r\n";
const std::string vise::connection::http_301 = "HTTP/1.1 301 Moved Permanently\r\n";
const std::string vise::connection::http_400 = "HTTP/1.1 400 Bad Request\r\n";

vise::connection::connection(boost::asio::io_service &io_service,
                             vise::project_manager *manager)
  :d_strand(io_service),
   d_socket(io_service),
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
  d_request.parse(request_chunk);

  if ( d_request.get_expect_100_continue_header() ) {
    response_http_100();
    d_request.reset_expect_100_continue_header();
  }

  if ( d_request.is_request_complete() ) {
    // process http request here
    boost::asio::ip::tcp::endpoint ep = d_socket.remote_endpoint(ec);
    if ( ec ) {
      return;
    }

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
  std::ostream http_response( &d_response_buffer );
  http_response << connection::http_100;

  boost::asio::async_write(d_socket, d_response_buffer.data(),
                           d_strand.wrap(boost::bind(&connection::on_http_100_response_write,
                                                    shared_from_this(),
                                                    boost::asio::placeholders::error
                                                    )
                                        )
                           );
}

void vise::connection::on_http_100_response_write(const boost::system::error_code& e) {
  if ( e ) {
    std::cerr << "\nfailed to send 100 continue response" << std::endl;
  }
}

void vise::connection::on_response_write(const boost::system::error_code& e) {
  if ( e ) {
    std::cerr << "\nfailed to send http response: " << e.message() << std::endl;
  }

  close_connection();
}

void vise::connection::close_connection() {
  boost::system::error_code ec;
  boost::asio::ip::tcp::endpoint ep = d_socket.remote_endpoint(ec);
  if ( ec ) {
    return;
  }

  if ( d_socket.is_open() ) {
    d_socket.shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec );
    d_socket.close(ec);
  }
}
