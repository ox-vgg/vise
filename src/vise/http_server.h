/** @file   http_server.h
 *  @brief  A generic http server inspired from the Boost asio HTTP Server
 *          http://www.boost.org/doc/libs/1_65_0/doc/html/boost_asio/examples/cpp11_examples.html
 *
 *
 *  @author Abhishek Dutta (adutta _AT_ robots.ox.ac.uk)
 *  @date   14 Nov. 2017
 */

#ifndef _HTTP_SERVER_H
#define _HTTP_SERVER_H

#include "project_manager.h"
#include "connection.h"

#include <signal.h>

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <memory>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/signal_set.hpp>

namespace vise {
  class http_server {
  public:
    http_server(std::map<std::string, std::string> const &conf,
                vise::project_manager &manager);
    void start();
    void stop();
  private:
    std::size_t d_thread_pool_size;
    boost::asio::io_service d_io_service;
    boost::asio::ip::tcp::acceptor d_acceptor;
    boost::asio::signal_set d_signals;
    boost::shared_ptr<vise::connection> d_new_connection;

    const std::map<std::string, std::string> d_conf;
    vise::project_manager *d_manager;

    void accept_new_connection();
    void handle_connection(const boost::system::error_code &e);
  };
}
#endif
