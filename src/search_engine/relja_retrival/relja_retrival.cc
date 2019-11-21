#include "relja_retrival.h"

vise::relja_retrival::relja_retrival(std::map<std::string, std::string> const &pconf)
  : search_engine("relja_retrival"),
    d_pconf(pconf),
    d_storedir(pconf.at("storedir")),
    d_datadir(pconf.at("datadir"))
{
  std::cout << "relja_retrival()" << std::endl;

  d_filelist_fn    = d_datadir / "filelist.txt";
  d_traindesc_fn   = d_datadir / "traindesc.bin";
  d_bowcluster_fn  = d_datadir / "bowcluster.bin";
  d_trainassign_fn = d_datadir / "trainassign.bin";
  d_trainhamm_fn   = d_datadir / "trainhamm.bin";
  d_index_dset_fn  = d_datadir / "index_dset.bin";
  d_index_iidx_fn  = d_datadir / "index_iidx.bin";
  d_index_fidx_fn  = d_datadir / "index_fidx.bin";
  d_index_tmp_dir  = d_datadir / "_tmp";
}

vise::relja_retrival::~relja_retrival() {
  std::cout << "~relja_retrival()" << std::endl;

  std::cout << "~relja_retrival(): waiting for d_thread_index ..."
            << std::endl;
  d_thread_index.join();
}

void vise::relja_retrival::index() {
  std::cout << "relja_retrival::index() start" << std::endl;
  d_thread_index = std::thread(&relja_retrival::index_run_all_stages, this);
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
  std::istringstream ss(d_pconf.at("bow_descriptor_count"));
  ss >> bow_descriptor_count;

  buildIndex::computeTrainDescs(d_filelist_fn.string(),
                                d_storedir.string(),
                                d_traindesc_fn.string(),
                                bow_descriptor_count,
                                feat_getter_obj);
}

void vise::relja_retrival::cluster_train_descriptors() {
  bool use_root_sift = true;
  if (d_pconf.at("use_root_sift") == "false" ||
      d_pconf.at("use_root_sift") == "False") {
    use_root_sift = false;
  }
  uint32_t bow_cluster_count = 0;
  std::istringstream ss1(d_pconf.at("bow_cluster_count"));
  ss1 >> bow_cluster_count;

  uint32_t cluster_num_iteration = 0;
  std::istringstream ss2(d_pconf.at("cluster_num_iteration"));
  ss2 >> cluster_num_iteration;

  buildIndex::compute_train_cluster(d_traindesc_fn.string(),
                                    use_root_sift,
                                    d_bowcluster_fn.string(),
                                    bow_cluster_count,
                                    cluster_num_iteration);
}

void vise::relja_retrival::assign_train_descriptors() {
  bool use_root_sift = true;
  if (d_pconf.at("use_root_sift") == "false" ||
      d_pconf.at("use_root_sift") == "False") {
    use_root_sift = false;
  }

  buildIndex::computeTrainAssigns(d_bowcluster_fn.string(),
                                  use_root_sift,
                                  d_traindesc_fn.string(),
                                  d_trainassign_fn.string());
}

void vise::relja_retrival::hamm_train_descriptors() {
  bool use_root_sift = true;
  if (d_pconf.at("use_root_sift") == "false" ||
      d_pconf.at("use_root_sift") == "False") {
    use_root_sift = false;
  }

  uint32_t hamm_embedding_bits = 0;
  std::istringstream ss(d_pconf.at("hamm_embedding_bits"));
  ss >> hamm_embedding_bits;

  buildIndex::computeHamming(d_bowcluster_fn.string(),
                             use_root_sift,
                             d_traindesc_fn.string(),
                             d_trainassign_fn.string(),
                             d_trainhamm_fn.string(),
                             hamm_embedding_bits);
}

void vise::relja_retrival::create_index() {
  uint32_t hamm_embedding_bits = 0;
  std::istringstream ss(d_pconf.at("hamm_embedding_bits"));
  ss >> hamm_embedding_bits;

  std::ostringstream feat_getter_param;
  feat_getter_param << "hesaff-sift";
  if (d_pconf.count("sift_scale_3")) {
    feat_getter_param << "-scale3";
  }
  featGetter_standard const feat_getter_obj(feat_getter_param.str().c_str());

  // @todo: for safety, use shared_ptr instead of a raw pointer.
  embedderFactory *embed_factory = NULL;
  if (d_pconf.count("hamm_embedding_bits")) {
    embed_factory = new hammingEmbedderFactory(d_trainhamm_fn.string(),
                                               hamm_embedding_bits);
  } else {
    embed_factory = new noEmbedderFactory;
  }

  boost::filesystem::remove_all(d_index_tmp_dir);
  boost::filesystem::create_directory(d_index_tmp_dir);
  std::string tmp_dir = d_index_tmp_dir.string() + boost::filesystem::path::preferred_separator;

  buildIndex::build(d_filelist_fn.string(),
                    d_storedir.string(),
                    d_index_dset_fn.string(),
                    d_index_iidx_fn.string(),
                    d_index_fidx_fn.string(),
                    tmp_dir,
                    feat_getter_obj,
                    d_bowcluster_fn.string(),
                    embed_factory );
  delete embed_factory; // @todo: for safety, use shared_ptr
}

void vise::relja_retrival::index_run_all_stages() {
  std::cout << "relja_retrival::index_run_all_stages(): start, "
            << "thread_id=" << std::this_thread::get_id()
            << std::endl;
  if (!boost::filesystem::exists(d_filelist_fn)) {
    create_file_list();
  }

  if (!boost::filesystem::exists(d_traindesc_fn)) {
    extract_train_descriptors();
  }

  if (!boost::filesystem::exists(d_bowcluster_fn)) {
    cluster_train_descriptors();
  }

  if (!boost::filesystem::exists(d_trainassign_fn)) {
    assign_train_descriptors();
  }

  if (!boost::filesystem::exists(d_trainhamm_fn)) {
    hamm_train_descriptors();
  }

  if (!boost::filesystem::exists(d_index_dset_fn) ||
      !boost::filesystem::exists(d_index_iidx_fn) ||
      !boost::filesystem::exists(d_index_fidx_fn)) {
    create_index();
  }

  std::cout << "relja_retrival::index_run_all_stages(): end, "
            << "thread_id=" << std::this_thread::get_id()
            << std::endl;
}
