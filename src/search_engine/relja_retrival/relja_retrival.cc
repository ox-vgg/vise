#include "relja_retrival.h"

vise::relja_retrival::relja_retrival(std::map<std::string, std::string> const &pconf)
  : search_engine("relja_retrival"),
    d_pconf(pconf),
    d_storedir(pconf.at("storedir")),
    d_datadir(pconf.at("datadir"))
{
    std::cout << "relja_retrival()" << std::endl;

    d_filelist_fn  = d_datadir / "filelist.txt";
    d_traindesc_fn = d_datadir / "traindesc.bin";
}

vise::relja_retrival::~relja_retrival() {
    std::cout << "~relja_retrival()" << std::endl;
}

void vise::relja_retrival::index() {
  std::cout << "relja_retrival::index() start" << std::endl;
  create_file_list();
  extract_train_descriptors();
  std::cout << "relja_retrival::index() done" << std::endl;
}

void vise::relja_retrival::search() {
  std::cout << "relja_retrival::search()" << std::endl;
}

void vise::relja_retrival::create_file_list() {
  std::cout << "reading filelist from " << d_storedir.string() << std::endl;
  std::cout << "saving filelist to " << d_filelist_fn.string() << std::endl;
  std::ofstream filelist(d_filelist_fn.string());

  boost::filesystem::path store(d_pconf.at("storedir"));
  boost::filesystem::directory_iterator end_itr;
  for (boost::filesystem::directory_iterator it(d_storedir); it!=end_itr; ++it) {
    if (boost::filesystem::is_regular_file(it->path())) {
      filelist << boost::filesystem::relative(it->path(), d_storedir).string() << std::endl;
    }
  }
  filelist.close();
}

void vise::relja_retrival::extract_train_descriptors() {
  std::cout << "relja_retrival::extract_train_descriptors()" << std::endl;

  std::ostringstream feat_getter_param;
  feat_getter_param << "hesaff-sift";
  if (d_pconf.count("sift_scale_3")) {
    feat_getter_param << "-scale3";
  }

  featGetter_standard const feat_getter_obj(feat_getter_param.str().c_str());

  int32_t bow_descriptor_count = -1;
  //std::istringstream ss(d_pconf.at("bow_descriptor_count"));
  //ss >> bow_descriptor_count;

  buildIndex::computeTrainDescs(d_filelist_fn.string(),
                                d_storedir.string(),
                                d_traindesc_fn.string(),
                                bow_descriptor_count,
                                feat_getter_obj);
}
