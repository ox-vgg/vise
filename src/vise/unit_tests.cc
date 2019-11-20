#include "vise_util.h"

#define BOOST_TEST_MODULE vise_util
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( config_load ) {
  std::string fn("../data/test/viseconf.txt");
  std::map<std::string, std::string> conf;
  vise::configuration_load(fn, conf);

  BOOST_TEST(conf.size() == 5);
  BOOST_TEST(conf["address"] == "0.0.0.0");
  BOOST_TEST(conf["port"] == "9669");
  BOOST_TEST(conf["nthread"] == "4");
}

BOOST_AUTO_TEST_CASE( split ) {
  std::map<std::string, std::size_t> d
    {
     {"", 0},
     {"/", 2},
     {"/index.html", 2},
     {"/ox5k", 2},
     {"/ox5k/", 3},
     {"/ox5k/file1.jpg", 3},
     {"/ox5k/_file?fid=232", 3},
     {"/ox5k/_search?fid1=481&fid2=829&region1=[2,2,10,10]", 3},
     {"/ox5k/folder1/folder2/file2.jpg", 5}
    };

  std::vector<std::string> uri;
  std::map<std::string, std::size_t>::const_iterator it;
  for (it=d.begin(); it!=d.end(); ++it) {
    vise::split(it->first, '/', "?", uri);
    vise::print_vector<std::string>(it->first, uri);
    BOOST_TEST( uri.size()==it->second );
  }
}
