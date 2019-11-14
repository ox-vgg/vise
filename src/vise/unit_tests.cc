#include "vise_util.h"

#define BOOST_TEST_MODULE unit_tests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( config_load ) {
  std::string fn("../data/test/viseconf.txt");
  std::map<std::string, std::string> conf;
  vise::configuration_load(fn, conf);

  BOOST_TEST(conf.size() == 4);
  BOOST_TEST(conf["address"] == "0.0.0.0");
  BOOST_TEST(conf["port"] == "9669");
  BOOST_TEST(conf["store"] == "/home/tlm/_tmp/vise/store/");
  BOOST_TEST(conf["nthread"] == "4");
}
