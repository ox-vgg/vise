/** @file   connection.h
 *  @brief  Denotes a connection corresponding to a user's http request
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   10 Nov 2017
 */

#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "project_manager.h"
#include "http_request.h"
#include "http_response.h"

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <boost/array.hpp>

namespace vise {
  class connection : public boost::enable_shared_from_this<connection>, private boost::noncopyable
  {
  public:
    connection(boost::asio::io_service &io_service,
               vise::project_manager *manager);

    boost::asio::ip::tcp::socket& socket();
    void process_connection();

    static const std::string crlf;
    static const std::string crlf2;
    static const std::string http_100;
    static const std::string http_200;
    static const std::string http_301;
    static const std::string http_400;
    static const std::string app_namespace;

  private:
    void on_request_data(const boost::system::error_code& e, std::size_t bytes_read);
    void on_response_write(const boost::system::error_code& e);
    void on_http_100_response_write(const boost::system::error_code& e);
    void close_connection();

    // responders
    void send_response();
    void response_not_found();
    void response_http_100();

    boost::asio::io_service::strand d_strand; // ensure that only a single thread invokes a handler
    boost::asio::ip::tcp::socket d_socket; // socket instance for this connection

    boost::array<char, 524288> d_buffer;
    boost::asio::streambuf d_response_buffer;
    boost::asio::streambuf d_continue_response_buffer;

    vise::http_request d_request;
    vise::http_response d_response;

    vise::project_manager *d_manager;
  };
}
#endif
