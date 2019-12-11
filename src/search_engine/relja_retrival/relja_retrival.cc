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
  d_weight_fn      = d_datadir / "weight.bin";

  d_is_search_engine_loaded = false;
  d_is_indexing_ongoing = false;
  d_is_indexing_done = false;
}

vise::relja_retrival::~relja_retrival() {
  std::cout << "~relja_retrival()" << std::endl;

  if (d_thread_index.joinable()) {
    std::cout << "~relja_retrival(): waiting for d_thread_index ..."
              << std::endl;
    d_thread_index.join();
  }

  if (d_is_search_engine_loaded) {
    bool success;
    std::string message;
    index_unload(success, message);
    if (!success) {
      std::cout << "~relja_retrival(): index_unload failed. "
                << "[" << message << "]"
                << std::endl;
    }
  }
}

void vise::relja_retrival::index_create(bool &success, std::string &message) {
  std::lock_guard<std::mutex> lock(d_search_engine_index_mutex);
  if (d_is_indexing_ongoing || d_is_indexing_done) {
    success = false;
    if (d_is_indexing_ongoing) {
      message = "an existing indexing process is already ongoing";
    }
    if (d_is_indexing_done) {
      message = "index has already been created";
    }
    return;
  }

  try {
    d_thread_index = std::thread(&relja_retrival::index_run_all_stages, this);
    success = true;
    message = "indexing started";
  } catch(std::exception &e) {
    success = false;
    std::ostringstream ss;
    ss << "failed to start indexing. "
       << "[" << e.what() << "]";
    message = ss.str();
  }
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
  d_is_indexing_ongoing = true;
  d_is_indexing_done = false;
  try {
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
    d_is_indexing_ongoing = false;
    d_is_indexing_done = true;
  } catch(std::exception &e) {
    d_is_indexing_ongoing = false;
    d_is_indexing_done = false;
  }
}

bool vise::relja_retrival::index_is_done() {
  if (boost::filesystem::exists(d_filelist_fn) &&
      boost::filesystem::exists(d_traindesc_fn) &&
      boost::filesystem::exists(d_bowcluster_fn) &&
      boost::filesystem::exists(d_trainassign_fn) &&
      boost::filesystem::exists(d_trainhamm_fn) &&
      boost::filesystem::exists(d_index_dset_fn) &&
      boost::filesystem::exists(d_index_iidx_fn) &&
      boost::filesystem::exists(d_index_fidx_fn)) {

    if (boost::filesystem::file_size(d_filelist_fn) &&
        boost::filesystem::file_size(d_traindesc_fn) &&
        boost::filesystem::file_size(d_bowcluster_fn) &&
        boost::filesystem::file_size(d_trainassign_fn) &&
        boost::filesystem::file_size(d_trainhamm_fn) &&
        boost::filesystem::file_size(d_index_dset_fn) &&
        boost::filesystem::file_size(d_index_iidx_fn) &&
        boost::filesystem::file_size(d_index_iidx_fn)) {
      if (d_is_indexing_done) {
        return true;
      } else {
        return false;
      }
    }
  }
  return false;
}

void vise::relja_retrival::index_load(bool &success,
                                      std::string &message) {
  std::lock_guard<std::mutex> lock(d_search_engine_load_mutex);
  if (d_is_search_engine_loaded) {
    success = true;
    message = "index is already loaded";
  }

  try {
    // @todo use std::shared_ptr instead of raw pointers
    // for d_dataset, d_cons_queue, ....
    std::cout << "loading dataset from " << d_index_dset_fn
              << std::endl;
    d_dataset = new datasetV2( d_index_dset_fn.string(),
                               d_storedir.string() );
    std::cout << "done loading dataset"<< std::endl;

    // needed to setup forward and inverted index
    std::cout << "setting up forward and inverted index "<< std::endl;
    d_cons_queue = new sequentialConstructions();
    // setup forward index
    d_db_fidx_file = new protoDbFile( d_index_fidx_fn.string() );
    boost::function<protoDb*()> fidxInRamConstructor = boost::lambda::bind(boost::lambda::new_ptr<protoDbInRam>(),
                                                                           boost::cref(*d_db_fidx_file) );
    d_db_fidx = new protoDbInRamStartDisk( *d_db_fidx_file, fidxInRamConstructor, true, d_cons_queue );
    d_fidx = new protoIndex(*d_db_fidx, false);
    std::cout << "forward index done"<< std::endl;

    // setup inverted index
    d_db_iidx_file = new protoDbFile( d_index_iidx_fn.string() );
    boost::function<protoDb*()> iidxInRamConstructor= boost::lambda::bind(boost::lambda::new_ptr<protoDbInRam>(),
                                                                          boost::cref(*d_db_iidx_file) );
    d_db_iidx = new protoDbInRamStartDisk( *d_db_iidx_file, iidxInRamConstructor, true, d_cons_queue );
    d_iidx = new protoIndex(*d_db_iidx, false);
    std::cout << "inverted index done"<< std::endl;
    d_cons_queue->start(); // start the construction of in-RAM stuff

    // feature getter and assigner
    std::cout << "loading feature getter "<< std::endl;

    bool use_root_sift = true;
    if (d_pconf.at("use_root_sift") == "false" ||
        d_pconf.at("use_root_sift") == "False") {
      use_root_sift = false;
    }
    std::ostringstream feat_getter_param;
    feat_getter_param << "hesaff-sift";
    if (d_pconf.count("sift_scale_3")) {
      feat_getter_param << "-scale3";
    }
    d_feat_getter = new featGetter_standard(feat_getter_param.str().c_str());

    // clusters
    std::cout << "loading clusters"<< std::endl;
    d_clst_centres = new clstCentres( d_bowcluster_fn.string().c_str(), true );

    // build kd-tree for nearest neighbour search
    // to assign cluster-id for each descriptor
    std::size_t num_trees = 8;
    std::size_t max_num_checks = 1024;
    d_kd_forest = vl_kdforest_new(VL_TYPE_FLOAT,
                                  d_clst_centres->numDims,
                                  num_trees,
                                  VlDistanceL2 );
    vl_kdforest_set_max_num_comparisons(d_kd_forest, max_num_checks);
    vl_kdforest_build(d_kd_forest,
                      d_clst_centres->numClst,
                      d_clst_centres->clstC_flat);

    // soft assigner
    std::cout << "loading assigner"<< std::endl;
    uint32_t hamm_embedding_bits = 0;
    std::istringstream ss(d_pconf.at("hamm_embedding_bits"));
    ss >> hamm_embedding_bits;

    if (hamm_embedding_bits == 0) {
      if (use_root_sift) {
        d_soft_assigner = new SA_exp( 0.02 );
      } else {
        d_soft_assigner = new SA_exp( 6250 );
      }
    }
    if (hamm_embedding_bits != 0) {
      d_emb_factory = new hammingEmbedderFactory(d_trainhamm_fn.string(), hamm_embedding_bits);
    }
    else {
      d_emb_factory = new noEmbedderFactory;
    }

    // create retriever
    std::cout << "loading retriever"<< std::endl;
    d_tfidf = new tfidfV2(d_iidx,
                          d_fidx,
                          d_weight_fn.string(),
                          d_feat_getter,
                          d_kd_forest,
                          d_soft_assigner);

    if (hamm_embedding_bits != 0) {
      d_hamming_emb = new hamming(*d_tfidf,
                                  d_iidx,
                                  *dynamic_cast<hammingEmbedderFactory const *>(d_emb_factory),
                                  d_fidx,
                                  d_feat_getter, d_kd_forest, d_clst_centres);
      d_base_retriever = d_hamming_emb;
    } else {
      d_base_retriever = d_tfidf;
    }

    // spatial verifier
    std::cout << "loading spatial verifier"<< std::endl;
    d_spatial_verif_v2 = new spatialVerifV2(*d_base_retriever,
                                            d_iidx,
                                            d_fidx,
                                            true,
                                            d_feat_getter,
                                            d_kd_forest,
                                            d_clst_centres);
    d_spatial_retriever = d_spatial_verif_v2;

    // multiple queries
    std::cout << "loading multiple queries"<< std::endl;
    d_multi_query_max = new multiQueryMax( *d_spatial_verif_v2 );
    if (d_hamming_emb != NULL){
      d_multi_query = new mqFilterOutliers(*d_multi_query_max,
                                           *d_spatial_verif_v2,
                                           *dynamic_cast<hammingEmbedderFactory const *>(d_emb_factory) );
    } else {
      d_multi_query = d_multi_query_max;
    }
    d_is_search_engine_loaded = true;
    success = true;
    message = "index loaded";
    std::cout << "relja_retrival:load() done" << std::endl;
  } catch(std::exception &e) {
    d_is_search_engine_loaded = false;
    success = false;
    std::ostringstream ss;
    ss << "failed to load index. "
       << "[" << e.what() << "]";
    message = ss.str();
  }
}

void vise::relja_retrival::index_unload(bool &success,
                                        std::string &message) {
  std::lock_guard<std::mutex> lock(d_search_engine_unload_mutex);
  if (!d_is_search_engine_loaded) {
    success = false;
    message = "index not loaded yet!";
    return;
  }

  try {
    delete d_cons_queue;

    if (d_hamming_emb != NULL) {
      delete d_hamming_emb;
      delete d_multi_query_max;
    }
    if (d_multi_query != NULL) {
      delete d_multi_query;
    }
    delete d_emb_factory;

    if (d_soft_assigner != NULL) {
      delete d_soft_assigner;
    }

    vl_kdforest_delete(d_kd_forest);
    delete d_clst_centres;
    delete d_feat_getter;

    delete d_fidx;
    delete d_iidx;

    delete d_tfidf;

    delete d_dataset;

    delete d_db_fidx;
    delete d_db_iidx;

    d_is_search_engine_loaded = false;
    success = true;
    message = "index unloaded";
  } catch(std::exception &e) {
    success = false;
    message = "failed to unload index";
  }
}

void vise::relja_retrival::index_search(vise::search_query const &q,
                                        std::vector<vise::search_result> &r) {
  std::cout << "relja_retrival::index_search(): searching "
            << d_dataset->getNumDoc() << " images ..."
            << std::endl;

  std::cout << "query: fid=" << q.d_file_id << ", "
            << "; region(x,y,width,height)="
            << q.d_x << "," << q.d_y << ","
            << q.d_width << "," << q.d_height << std::endl;
  query qobj(q.d_file_id, true, "", q.d_x, q.d_x + q.d_width, q.d_y, q.d_y + q.d_height);
  std::vector<indScorePair> all_result;
  std::map<uint32_t, homography> H;
  try {
    d_spatial_retriever->spatialQuery(qobj, all_result, H);
  } catch(std::exception &e) {
    std::cout << "exception: " << e.what() << std::endl;
  }

  unsigned int show_count = 200;
  std::cout << "Showing results " << show_count
            << "/" << all_result.size() << std::endl;
  for ( unsigned int i=0; i < show_count; ++i ) {
    std::cout << "[" << i << "] : "
              << "fid=" << all_result[i].first << "\t"
              << d_dataset->getInternalFn(all_result[i].first) << "\t"
              << "score=" << all_result[i].second
              << std::endl;
  }

  /*
  float score_threshold = 0.1;
  for ( unsigned int i = 0; i < all_result.size(); ++i ) {
    unsigned int file_id = all_result[i].first;
    // check if valid homography is available
    std::map<uint32_t, homography>::iterator it = H.find(file_id);
    float score = (float) all_result[i].second;

    std::cout << "[" << file_id << "] : score=" << score << std::endl;
    if ( (it != H.end()) && (score > score_threshold) ) {
      std::array<double, 9> H = {{it->second.H[0], it->second.H[1], it->second.H[2], it->second.H[3], it->second.H[4], it->second.H[5], it->second.H[6], it->second.H[7], it->second.H[8]}};

      r.push_back(vise::search_result(file_id,
                                      d_dataset->getInternalFn(file_id),
                                      score,
                                      H)
                  );
    }
  }
  */
}

bool vise::relja_retrival::index_is_loaded() {
  return d_is_search_engine_loaded;
}
