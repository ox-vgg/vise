#include "http_server.h"

vise::http_server::http_server(std::map<std::string, std::string> const &conf,
                               vise::project_manager &manager)
  : d_io_service(),
    d_acceptor(d_io_service),
    d_signals(d_io_service),
    d_conf(conf),
    d_manager(&manager)
{
  if (d_conf.find("http-address") == d_conf.end() ||
      d_conf.find("http-port") == d_conf.end()
      ) {
    std::cerr << "http_server cannot start as http-address and http-port are missing in configuration!"
              << std::endl;
    throw std::runtime_error("address and port missing in configuration");
  }

  if (d_conf.find("http-worker") == d_conf.end()) {
    d_thread_pool_size = 1;
  } else {
    d_thread_pool_size = std::atoi(d_conf.at("http-worker").c_str());
    if(d_thread_pool_size == 0) {
      d_thread_pool_size = std::max(1, omp_get_max_threads()-2);
    }
  }

  // add hooks to stop http server when the program exits
  d_signals.add(SIGINT);
  d_signals.add(SIGTERM);
  d_signals.add(SIGSEGV);
  d_signals.add(SIGILL);
  d_signals.add(SIGFPE);
  d_signals.add(SIGABRT);
#if defined(SIGBREAK)
  d_signals.add(SIGBREAK);
#endif
#if defined(SIGQUIT)
  d_signals.add(SIGQUIT);
#endif

  d_signals.async_wait(boost::bind(&http_server::stop,
                                  this));

  boost::asio::ip::tcp::resolver resolver( d_io_service );
  boost::asio::ip::tcp::resolver::query query( d_conf.at("http-address"),
                                               d_conf.at("http-port") );
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve( query );

  d_acceptor.open( endpoint.protocol() );
  d_acceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address(true) );
  d_acceptor.set_option( boost::asio::socket_base::keep_alive(false) );

  d_acceptor.bind( endpoint );
  d_acceptor.listen();
  accept_new_connection();

  std::cout << "http_server: listening for connections at "
            << d_conf.at("http-address") << ":" << d_conf.at("http-port")
            << d_conf.at("http-namespace")
            << " (with " << d_thread_pool_size << " threads)"
            << std::endl;
}

void vise::http_server::stop() {
  std::cout << "http_server:: stopping ..."
            << std::endl;
  d_io_service.stop();
  //delete d_manager;
}

void vise::http_server::start() {
  std::vector< boost::shared_ptr<boost::thread> > threads;
  for ( std::size_t i = 0; i < d_thread_pool_size; ++i ) {
    // these threads will block until async operations are complete
    // the first async operation is created by a call to
    // http_server::acceptd_new_connection() method
    boost::shared_ptr<boost::thread> connection_thread( new boost::thread( boost::bind( &boost::asio::io_service::run, &d_io_service) ) );
    threads.push_back( connection_thread );
  }

  // wait until all threads return
  for ( std::size_t i = 0; i < threads.size(); ++i ) {
    threads[i]->join();
  }
}

void vise::http_server::accept_new_connection() {
  d_new_connection.reset( new vise::connection(d_io_service, d_manager) );
  d_acceptor.async_accept( d_new_connection->socket(),
                          boost::bind(&http_server::handle_connection,
                                      this,
                                      boost::asio::placeholders::error) );
}

void vise::http_server::handle_connection(const boost::system::error_code &e) {
  // check if server was stopped by a signal before handle_connection()
  // had a chance to run
  if (!d_acceptor.is_open()) {
    return;
  }

  if (!e) {
    d_new_connection->process_connection();
  } else {
    std::cerr << "\nhandle_connection: was passed error: " << e.message() << std::flush;
  }
  accept_new_connection();
}
