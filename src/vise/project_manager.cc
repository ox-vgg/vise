#include "project_manager.h"

vise::project_manager::project_manager(std::map<std::string, std::string> const &conf)
  : _conf(conf)
{
  vise::configuration_show(_conf);
  std::cout << "project_manager: Initializing project manager ..."
            << std::endl;
  std::cout << "::store=" << _conf.at("store") << std::endl;
}

// WARNING: this method may be invoked by multiple threads simultaneously
// be careful of avoid race conditions
void vise::project_manager::process_http_request(http_request const &request,
                                            http_response &response)
{
  std::cout << "project_manager: Processing http request ..."
            << std::endl;
  std::cout << "::" << request.print() << std::endl;
  response.set_status(200);
  response.set_payload("request received");
}
