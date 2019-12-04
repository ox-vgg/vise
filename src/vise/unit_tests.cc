#include "vise_util.h"

#include <array>

#define BOOST_TEST_MODULE vise_util
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( config_load ) {
  // Assumption: test is being run from root of source tree
  std::string fn("src/conf/viseconf.txt");
  std::map<std::string, std::string> conf;
  vise::configuration_load(fn, conf);

  BOOST_TEST(conf.size() != 0);
  BOOST_TEST(conf.count("address") == 1);
  BOOST_TEST(conf.count("port") == 1);
  BOOST_TEST(conf.count("nthread") == 1);
}

BOOST_AUTO_TEST_CASE( split ) {
  std::map< std::string, std::array<std::size_t,2> > test_data
    {
     {"", {0,0}},
     {"/", {2,0}},
     {"/index.html", {2,0}},
     {"/ox5k", {2,0}},
     {"/ox5k/", {3,0}},
     {"/ox5k/file1.jpg", {3,0}},
     {"/ox5k/_file?fid=232", {3,1}},
     {"/ox5k/_file_add?source_type=local&source_loc=/data/dataset/imagenet", {3,2}},
     {"/ox5k/_search?fid1=481&fid2=829&region1=[2,2,10,10]", {3,3}},
     {"/ox5k/folder1/folder2/file2.jpg", {5,0}}
    };

  std::vector<std::string> uri_components;
  std::map<std::string, std::string> uri_param;
  std::map<std::string, std::array<std::size_t,2> >::const_iterator it;
  for (it=test_data.begin(); it!=test_data.end(); ++it) {
    vise::decompose_uri(it->first, uri_components, uri_param);
    //vise::print_vector<std::string>(it->first, uri_components);
    //vise::print_map(it->first, uri_param);
    BOOST_TEST( uri_components.size()==it->second[0] );
    BOOST_TEST( uri_param.size()==it->second[1] );
  }
}
