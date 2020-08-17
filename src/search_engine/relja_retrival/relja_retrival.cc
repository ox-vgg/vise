#include "relja_retrival.h"

const std::vector<std::string> vise::relja_retrival::task_name_list = {
                                                                       "preprocess",
                                                                       "traindesc",
                                                                       "cluster",
                                                                       "assign",
                                                                       "hamm",
                                                                       "index"
};

vise::relja_retrival::relja_retrival(boost::filesystem::path pconf_fn,
                                     boost::filesystem::path project_dir)
  : search_engine("relja_retrival"), d_pconf_fn(pconf_fn), d_project_dir(project_dir)
{
  std::cout << "relja_retrival()" << std::endl;
  if( !boost::filesystem::exists(d_pconf_fn) ) {
    std::cout << "vise::relja_retrival::relja_retrival(): project configuration file not found: "
              << d_pconf_fn << std::endl;
  }
  bool success = false;
  success = vise::configuration_load(d_pconf_fn.string(), d_pconf);
  if(success) {
    if(!pconf_validate_data_dir()) {
      std::cout << "relja_retrival(): failed to validate conf. data dir" << std::endl;
      return;
    }
  } else {
    std::cout << "relja_retrival(): failed to load configuration from: "
              << d_pconf_fn << std::endl;
  }

  d_filelist_fn    = d_data_dir / "filelist.txt";
  d_filestat_fn    = d_data_dir / "filestat.txt";
  d_traindesc_fn   = d_data_dir / "traindesc.bin";
  d_bowcluster_fn  = d_data_dir / "bowcluster.bin";
  d_trainassign_fn = d_data_dir / "trainassign.bin";
  d_trainhamm_fn   = d_data_dir / "trainhamm.bin";
  d_index_dset_fn  = d_data_dir / "index_dset.bin";
  d_index_iidx_fn  = d_data_dir / "index_iidx.bin";
  d_index_fidx_fn  = d_data_dir / "index_fidx.bin";
  d_index_tmp_dir  = d_data_dir / "_tmp/";
  d_index_tmp_dir.make_preferred(); // convert the trailing path-separator to platform specific character
  d_index_status_fn= d_data_dir / "index_status.txt";
  d_weight_fn      = d_data_dir / "weight.bin";

  d_is_search_engine_loaded = false;
  d_is_indexing_ongoing = false;
  d_is_indexing_done = false;

  d_index_log_fn   = d_data_dir / "index.log";
  d_log.open(d_index_log_fn.string(), std::fstream::app);
  d_log << "===================[ "
        << "logging started on " << vise::now_timestamp()
        << " ]==================="<< std::endl;

  d_task_progress_list.clear();
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

  if(d_log) {
    d_log << "logging ended on " << vise::now_timestamp() << std::endl;
    d_log.close();
  }
}

void vise::relja_retrival::index_create(bool &success,
                                        std::string &message,
                                        std::function<void(void)> callback,
                                        bool block_until_done) {
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
    d_thread_index = std::thread(&relja_retrival::index_run_all_stages, this, callback);
    if(block_until_done) {
      std::cout << "relja_retrival::index_create(): blocking until done" << std::endl;
      d_thread_index.join();
      if (d_is_indexing_done) {
        message = "index created";
        success = true;
      } else {
        message = "failed to create index";
        success = false;
      }
    } else {
      message = "indexing started";
      success = true;
    }
  } catch(std::exception &e) {
    success = false;
    std::ostringstream ss;
    ss << "failed to start indexing. "
       << "[" << e.what() << "]";
    message = ss.str();
  }
}

void vise::relja_retrival::preprocess_image(std::string srcfn,
                                            std::string dstfn,
                                            bool &result,
                                            std::string &message) {
  try {
    std::ostringstream resize_stat;
    std::ostringstream geom_spec;

    bool resize_image = true;
    if(d_pconf.count("resize_dimension")) {
      if (d_pconf.at("resize_dimension") == "-1") {
          resize_image = false;
      } else {
        geom_spec << d_pconf.at("resize_dimension");
      }
    } else {
      geom_spec << "800x800";
    }
    geom_spec << "+0+0>";

    Magick::Image src(srcfn);
    src.quiet(true); // to supress warnings
    resize_stat << src.rows() << "," << src.columns();
    src.magick("JPEG");
    src.colorSpace(Magick::RGBColorspace);
    if (resize_image) {
      Magick::Geometry dst_img_size(geom_spec.str());
      src.resize(dst_img_size);
    }

    src.write(dstfn);
    src.quiet(false);
    resize_stat << "," << src.rows() << "," << src.columns();
    result = true;
    message = resize_stat.str();
  } catch(std::exception &ex) {
    result = false;
    message = ex.what();
  }
}
uint32_t vise::relja_retrival::image_src_count() const {
  uint32_t count = 0;
  const boost::filesystem::path image_src_dir(d_image_src_dir);
  boost::filesystem::recursive_directory_iterator end_itr;
  for (boost::filesystem::recursive_directory_iterator it(image_src_dir); it!=end_itr; ++it) {
    if (boost::filesystem::is_regular_file(it->path())) {
      count++;
    }
  }
  return count;
}

void vise::relja_retrival::create_file_list() {
  std::cout << "preprocess: resize_dimension=" << d_pconf.at("resize_dimension") << std::endl;

  d_task_progress_list.at("preprocess").start(0, image_src_count());

  const boost::filesystem::path image_src_dir(d_image_src_dir);
  const boost::filesystem::path image_dir(d_image_dir);
  d_log << "preprocess: preprocessing images in : " << std::endl
        << "preprocess:   source: " << image_src_dir << std::endl
        << "preprocess:   target: " << image_dir << std::endl;

  d_log << "preprocess: resize_dimension=" << d_pconf.at("resize_dimension") << std::endl;

  uint32_t success_count = 0;
  uint32_t fail_count = 0;
  std::ofstream filelist(d_filelist_fn.string());
  std::ofstream filestat(d_filestat_fn.string());
  boost::filesystem::recursive_directory_iterator end_itr;
  uint32_t count = 0;
  for (boost::filesystem::recursive_directory_iterator it(image_src_dir); it!=end_itr; ++it) {
    if (boost::filesystem::is_regular_file(it->path())) {
      boost::filesystem::path fn = boost::filesystem::relative(it->path(), image_src_dir).string();
      boost::filesystem::path dst_dir = image_dir / boost::filesystem::relative(it->path(), image_src_dir).parent_path();
      if (!boost::filesystem::exists(dst_dir)) {
          boost::filesystem::create_directories(dst_dir);
      }

      bool preprocess_result = true;
      std::string preprocess_message("");
      std::string dst_str;
      if (d_pconf.at("resize_dimension") == "-1") {
        // simply copy images (no preprocessing required)
        boost::system::error_code ec;
        boost::filesystem::path dst = dst_dir / it->path().filename();
        dst_str = dst.string();
        boost::filesystem::copy_file(it->path(), dst, boost::filesystem::copy_option::overwrite_if_exists, ec);
        if(ec) {
            preprocess_result = false;
            preprocess_message = ec.message();
        }
      } else {
        boost::filesystem::path dst = dst_dir / it->path().stem();
        dst_str = dst.string() + ".jpg";
        preprocess_image(it->path().string(), dst_str, preprocess_result, preprocess_message);
      }
      if(preprocess_result) {
        boost::filesystem::path dst_path(dst_str);
        std::string dst_relpath(boost::filesystem::relative(dst_path, image_dir).string());
#ifdef _WIN32
        // convert relative paths to Unix format so that it is compatible with HTTP requests
        // e.g. GET /pname/subdir1/image1.jpg
        std::replace(dst_relpath.begin(), dst_relpath.end(), '\\', '/');
#endif
        filelist << dst_relpath << std::endl;
        filestat << dst_relpath << "," << preprocess_message << std::endl;
        success_count++;
      } else {
        d_log << "preprocess: discarded " << fn.string()
              << " [reason: " << preprocess_message << "]" << std::endl;
        fail_count++;
      }
      count++;
      d_task_progress_list.at("preprocess").update(count);
    }
  }
  filelist.close();
  filestat.close();
  d_task_progress_list.at("preprocess").finish_success();
  d_log << "preprocess: written " << success_count
        << " (discarded " << fail_count << ")"
        << " filenames to " << d_filelist_fn << std::endl;
  d_log << "preprocess: completed in "
        << d_task_progress_list.at("preprocess").d_elapsed_ms
        << "ms" << std::endl;
  d_log << std::flush;
}

void vise::relja_retrival::extract_train_descriptors() {
  std::cout << "relja_retrival::extract_train_descriptors()" << std::endl;

  std::ostringstream feat_getter_param;
  feat_getter_param << "hesaff-sift";
  if (d_pconf.count("sift_scale_3")) {
    feat_getter_param << "-scale3";
  }

  d_log << "traindesc: feat_getter_param : "
        << feat_getter_param.str() << std::endl;

  featGetter_standard const feat_getter_obj(feat_getter_param.str().c_str());

  int32_t bow_descriptor_count = -1;
  std::istringstream ss(d_pconf.at("bow_descriptor_count"));
  ss >> bow_descriptor_count;
  if(bow_descriptor_count == -1) {
    uint32_t max_feature_count = image_src_count() * 3000; // assumption: max 3000 descriptors per image
    d_task_progress_list.at("traindesc").start(0, max_feature_count);
  } else {
    d_task_progress_list.at("traindesc").start(0, bow_descriptor_count);
  }
  d_log << "traindesc: bow_descriptor_count : "
        << bow_descriptor_count << std::endl;

  buildIndex::computeTrainDescs(d_filelist_fn.string(),
                                d_image_dir.string(),
                                d_traindesc_fn.string(),
                                bow_descriptor_count,
                                feat_getter_obj,
                                d_log,
                                &d_task_progress_list.at("traindesc"));
  int32_t actual_bow_descriptor_count = get_traindesc_count(d_traindesc_fn.string());
  std::cout << "relja_retrival::extract_train_descriptors(): actual_bow_descriptor_count=" << actual_bow_descriptor_count << std::endl;
  if(bow_descriptor_count != actual_bow_descriptor_count) {
    d_pconf["bow_descriptor_count"] = std::to_string(actual_bow_descriptor_count);
    vise::configuration_save(d_pconf, d_pconf_fn.string());
  }
  d_task_progress_list.at("traindesc").finish_success();

  d_log << "traindesc: completed in "
        << d_task_progress_list.at("traindesc").d_elapsed_ms
        << "ms" << std::endl;
  d_log << std::flush;
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

  if(bow_cluster_count == 0) {
    // automatically compute number of clusters based on training descriptor count
    // 1. read number of training descriptors (save if different from conf)
    // 2. infer cluster
    int32_t bow_descriptor_count = 0;
    std::istringstream ss1(d_pconf.at("bow_descriptor_count"));
    ss1 >> bow_descriptor_count;

    if(bow_descriptor_count >= 18000000) {
      // max descriptors entails max clusters
      bow_cluster_count = 100000;
      cluster_num_iteration = 30;
    } else {
      if(bow_descriptor_count < 100000) {
        bow_cluster_count = 1000;
      } else {
        bow_cluster_count = 1000 + uint32_t(bow_descriptor_count / 3000);
      }
      if( bow_cluster_count<10000 ) {
        cluster_num_iteration = 10;
      } else {
        cluster_num_iteration = 20;
      }
    }
    d_pconf["bow_cluster_count"] = std::to_string(bow_cluster_count);
    d_pconf["cluster_num_iteration"] = std::to_string(cluster_num_iteration);
    vise::configuration_save(d_pconf, d_pconf_fn.string());
  }

  d_log << "cluster: use_root_sift=" << use_root_sift
        << ", bow_cluster_count=" << bow_cluster_count
        << ", cluster_num_iteration=" << cluster_num_iteration
        << std::endl;

  d_task_progress_list.at("cluster").start(0, cluster_num_iteration);
  buildIndex::compute_train_cluster(d_traindesc_fn.string(),
                                    use_root_sift,
                                    d_bowcluster_fn.string(),
                                    bow_cluster_count,
                                    cluster_num_iteration,
                                    &d_task_progress_list.at("cluster"));
  d_task_progress_list.at("cluster").finish_success();

  d_log << "cluster: completed in "
        << d_task_progress_list.at("cluster").d_elapsed_ms
        << "ms" << std::endl;
  d_log << std::flush;
}

void vise::relja_retrival::assign_train_descriptors() {
  bool use_root_sift = true;
  if (d_pconf.at("use_root_sift") == "false" ||
      d_pconf.at("use_root_sift") == "False") {
    use_root_sift = false;
  }

  //d_task_progress_list.at("assign").start() is invoked by computeTrainAssigns();
  buildIndex::computeTrainAssigns(d_bowcluster_fn.string(),
                                  use_root_sift,
                                  d_traindesc_fn.string(),
                                  d_trainassign_fn.string(),
                                  d_log,
                                  &d_task_progress_list.at("assign"));
  d_task_progress_list.at("assign").finish_success();

  d_log << "assign: completed in "
        << d_task_progress_list.at("assign").d_elapsed_ms
        << "ms" << std::endl;
  d_log << std::flush;
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

  //d_task_progress_list.at("hamm").start() is invoked by computeHamming();
  buildIndex::computeHamming(d_bowcluster_fn.string(),
                             use_root_sift,
                             d_traindesc_fn.string(),
                             d_trainassign_fn.string(),
                             d_trainhamm_fn.string(),
                             hamm_embedding_bits,
                             &d_task_progress_list.at("hamm"));
  d_task_progress_list.at("hamm").finish_success();

  d_log << "hamm: completed in "
        << d_task_progress_list.at("hamm").d_elapsed_ms
        << "ms" << std::endl;
  d_log << std::flush;
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

  //d_task_progress_list.at("index").start() is invoked by buildIndex::build();
  buildIndex::build(d_filelist_fn.string(),
                    d_image_dir.string(),
                    d_index_dset_fn.string(),
                    d_index_iidx_fn.string(),
                    d_index_fidx_fn.string(),
                    d_index_tmp_dir.string(),
                    feat_getter_obj,
                    d_bowcluster_fn.string(),
                    embed_factory,
                    &d_task_progress_list.at("index"));
  delete embed_factory; // @todo: for safety, use shared_ptr
  d_task_progress_list.at("index").finish_success();

  d_log << "index: completed in "
        << d_task_progress_list.at("index").d_elapsed_ms
        << "ms" << std::endl;
  d_log << std::flush;
}

void vise::relja_retrival::index_run_all_stages(std::function<void(void)> callback) {
  d_is_indexing_ongoing = true;
  d_is_indexing_done = false;
  try {
    callback(); // to update the state to INDEX_ONGOING
    std::ofstream index_status_f(d_index_status_fn.string());
    index_status_f << "start" << std::flush;

    // reload configuration
    bool success = false;
    success = vise::configuration_load(d_pconf_fn.string(), d_pconf);
    if(!success) {
      throw std::runtime_error("Failed to load configuration file");
      return;
    }
    if(!pconf_validate_data_dir()) {
      throw std::runtime_error("Failed to validate configuration file");
      return;
    }

    d_task_progress_list.clear();
    bool visual_vocabulary_exists = false;
    if(boost::filesystem::exists(d_bowcluster_fn) &&
       boost::filesystem::exists(d_trainhamm_fn)) {
      visual_vocabulary_exists = true;
    }
    for(uint32_t i=0; i<vise::relja_retrival::task_name_list.size(); ++i) {
      std::string task_name(vise::relja_retrival::task_name_list.at(i));
      if(visual_vocabulary_exists) {
        // since we use generic visual vocabulary, we discard some tasks
        if(task_name != "traindesc" &&
           task_name != "cluster" &&
           task_name != "assign" &&
           task_name != "hamm") {
          d_task_progress_list.insert( std::pair<std::string, vise::task_progress>(task_name, vise::task_progress(task_name)));
        }
      } else {
        d_task_progress_list.insert( std::pair<std::string, vise::task_progress>(task_name, vise::task_progress(task_name)));
      }
    }

    std::cout << "relja_retrival::index_run_all_stages(): start, "
              << "thread_id=" << std::this_thread::get_id()
              << std::endl;
    d_log << "indexing started on " << vise::now_timestamp() << std::endl;

    if (!boost::filesystem::exists(d_filelist_fn)) {
      create_file_list();
      index_status_f << ",filelist" << std::flush;
    }

    if(boost::filesystem::exists(d_bowcluster_fn) &&
       boost::filesystem::exists(d_trainhamm_fn)) {
      // use existing visual vocabulary
      d_log << "using existing visual vocabulary defined by:\n"
            << "  - visual vocabulary=" << d_bowcluster_fn << "\n"
            << "  - hamming embedding=" << d_trainhamm_fn
            << std::endl;
      index_status_f << ",traindesc,cluster,assign,hamm" << std::flush;
    } else {
      if (!boost::filesystem::exists(d_traindesc_fn)) {
        extract_train_descriptors();
        index_status_f << ",traindesc" << std::flush;
      }

      if (!boost::filesystem::exists(d_bowcluster_fn)) {
        cluster_train_descriptors();
        index_status_f << ",cluster" << std::flush;
      }

      if (!boost::filesystem::exists(d_trainassign_fn)) {
        assign_train_descriptors();
        index_status_f << ",assign" << std::flush;
      }

      if (!boost::filesystem::exists(d_trainhamm_fn)) {
        hamm_train_descriptors();
        index_status_f << ",hamm" << std::flush;
      }
    }

    if (!boost::filesystem::exists(d_index_dset_fn) ||
        !boost::filesystem::exists(d_index_iidx_fn) ||
        !boost::filesystem::exists(d_index_fidx_fn)) {
      create_index();
      index_status_f << ",index" << std::flush;
    }

    std::cout << "relja_retrival::index_run_all_stages(): end, "
              << "thread_id=" << std::this_thread::get_id()
              << std::endl;
    d_is_indexing_ongoing = false;
    d_is_indexing_done = true;
    index_status_f << ",end";
    index_status_f.close();

    // delete d_traindesc_fn as it is no longer needed, @todo: review this action in future
    boost::filesystem::remove(d_traindesc_fn);

    callback();
  } catch(std::exception &e) {
    d_is_indexing_ongoing = false;
    d_is_indexing_done = false;
    d_log << "Exception: " << e.what() << std::endl;
    callback();
  }
}

bool vise::relja_retrival::index_is_done() const {
  if(d_is_indexing_done) {
    return true;
  }

  if(d_is_indexing_ongoing) {
    return false;
  }

  std::vector<std::string> status_tokens;
  index_read_status(status_tokens);
  if(status_tokens.size()==0) {
    return false;
  }

  // status tokens in a complete index is as follows:
  // start, filelist, traindesc, cluster, assign, hamm, index, end
  if( status_tokens.size() != 8 ) {
    return false;
  }

  if( status_tokens.at(0) == "start" &&
	  status_tokens.at(1) == "filelist" &&
	  status_tokens.at(2) == "traindesc" &&
	  status_tokens.at(3) == "cluster" &&
	  status_tokens.at(4) == "assign" &&
	  status_tokens.at(5) == "hamm" &&
	  status_tokens.at(6) == "index" &&
	  status_tokens.at(7) == "end" ) {
	  return true;
  }
  else {
    return false;
  }
}

void vise::relja_retrival::index_read_status(std::vector<std::string> &status_tokens) const {
  status_tokens.clear();
  std::ifstream index_status_f(d_index_status_fn.string());
  if(index_status_f) {
    //std::cerr << "reading index status: " << d_index_status_fn << std::endl;
    while(index_status_f.good()) {
      std::string token;
      std::getline(index_status_f, token, ',');
      //std::cout << token << "|";
      status_tokens.push_back(token);
    }
    std::cout << std::endl;
  }
}

bool vise::relja_retrival::index_is_ongoing() const {
  return d_is_indexing_ongoing;
}

bool vise::relja_retrival::index_is_incomplete() const {
  std::vector<std::string> status_tokens;
  index_read_status(status_tokens);
  if(status_tokens.size()==0) {
    return false;
  }

  if(status_tokens[status_tokens.size()-1] != "end") {
    return true;
  }

  return false;
}

std::string vise::relja_retrival::index_status() const {
  std::ostringstream ss;
  ss << "{\"index_is_ongoing\":" << index_is_ongoing()
     << ",\"index_is_done\":" << index_is_done()
     << ",\"task_progress\":[";
  if(d_task_progress_list.size()) {
    for(uint32_t i=0; i<vise::relja_retrival::task_name_list.size(); ++i) {
      std::string task_name = vise::relja_retrival::task_name_list.at(i);
      if(d_task_progress_list.count(task_name)) {
        if( i!=0 ) {
          ss << ",";
        }
        ss << d_task_progress_list.at(task_name).to_json();
      }
    }
  }
  ss << "]}";
  return ss.str();
}

void vise::relja_retrival::index_load(bool &success,
                                      std::string &message) {
  std::lock_guard<std::mutex> lock(d_search_engine_load_mutex);
  if (d_is_search_engine_loaded) {
    success = true;
    message = "index is already loaded";
    return;
  }

  try {
    // @todo use std::shared_ptr instead of raw pointers
    // for d_dataset, d_cons_queue, ....
    std::cout << "loading dataset from " << d_index_dset_fn
              << std::endl;
    d_dataset = new datasetV2( d_index_dset_fn.string(),
                               d_data_dir.string() );
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
    if(d_cons_queue != nullptr) {
      delete d_cons_queue;
    }

    if (d_hamming_emb != nullptr) {
      delete d_hamming_emb;
    }

    if(d_multi_query_max != nullptr) {
      delete d_multi_query_max;
    }

    if (d_multi_query != nullptr) {
      delete d_multi_query;
    }

    if (d_emb_factory != nullptr) {
      delete d_emb_factory;
    }

    if (d_soft_assigner != nullptr) {
      delete d_soft_assigner;
    }

    if(d_kd_forest != nullptr) {
      vl_kdforest_delete(d_kd_forest);
    }

    if(d_clst_centres != nullptr) {
      delete d_clst_centres;
    }

    if(d_feat_getter != nullptr) {
      delete d_feat_getter;
    }

    if(d_fidx != nullptr) {
      delete d_fidx;
    }

    if(d_iidx != nullptr) {
      delete d_iidx;
    }

    if(d_tfidf != nullptr) {
      delete d_tfidf;
    }

    if(d_dataset != nullptr) {
      delete d_dataset;
    }

    if(d_db_fidx != nullptr) {
      delete d_db_fidx;
    }

    if(d_db_iidx != nullptr) {
      delete d_db_iidx;
    }

    d_is_search_engine_loaded = false;
    success = true;
    message = "index unloaded";
    std::cout << "search_engine unloaded" << std::endl;
  } catch(std::exception &e) {
    success = false;
    message = "failed to unload index";
  }
}

void vise::relja_retrival::index_search(vise::search_query const &q,
                                        std::vector<vise::search_result> &r) const {
  query qobj(q.d_file_id, true, "");
  if(q.is_region_query) {
    /*
    std::cout << "query: fid=" << q.d_file_id << ", "
              << "; region(x,y,width,height)="
              << q.d_x << "," << q.d_y << ","
              << q.d_width << "," << q.d_height << std::endl;
    */
    qobj = query(q.d_file_id, true, "", q.d_x, q.d_x + q.d_width, q.d_y, q.d_y + q.d_height);
  } else {
    std::cout << "query: fid=" << q.d_file_id << " (full image query)" << std::endl;
  }

  std::vector<indScorePair> all_result;
  std::map<uint32_t, homography> H_list;
  try {
    d_spatial_retriever->spatialQuery(qobj, all_result, H_list, q.d_max_result_count);
  } catch(std::exception &e) {
    std::cout << "exception: " << e.what() << std::endl;
  }

  for ( uint32_t i = 0; i < all_result.size(); ++i ) {
    uint32_t file_id = all_result[i].first;
    if ( H_list.count(file_id) == 1 ) {
      float score = (float) all_result[i].second;
      std::array<double, 9> H = {{H_list[file_id].H[0], H_list[file_id].H[1], H_list[file_id].H[2],
                                  H_list[file_id].H[3], H_list[file_id].H[4], H_list[file_id].H[5],
                                  H_list[file_id].H[6], H_list[file_id].H[7], H_list[file_id].H[8]}};
      r.push_back(vise::search_result(file_id,
                                      d_dataset->getInternalFn(file_id),
                                      score,
                                      H)
                  );
    }
  }
}

void vise::relja_retrival::index_internal_match(vise::search_query const &q,
                                                uint32_t match_file_id,
                                                std::ostringstream &json) const {
  homography H;
  std::vector< std::pair<ellipse, ellipse> > matches;

  query qobj(q.d_file_id, true, "");
  if(q.is_region_query) {
    qobj = query(q.d_file_id, true, "", q.d_x, q.d_x + q.d_width, q.d_y, q.d_y + q.d_height);
  }
  d_spatial_retriever->getMatches(qobj, match_file_id, H, matches);

  json.str("");
  json.clear();
  json << "{\"H\":[" << H.H[0] << "," << H.H[1] << "," << H.H[2] << ","
       << H.H[3] << "," << H.H[4] << "," << H.H[5] << ","
       << H.H[6] << "," << H.H[7] << "," << H.H[8] << "]";
  if(matches.size()) {
    json << ",\"matches\":[["
         << matches.at(0).first.x << ","
         << matches.at(0).first.y << ","
         << matches.at(0).first.a << ","
         << matches.at(0).first.b << ","
         << matches.at(0).first.c << ","
         << matches.at(0).second.x << ","
         << matches.at(0).second.y << ","
         << matches.at(0).second.a << ","
         << matches.at(0).second.b << ","
         << matches.at(0).second.c << "]";

    for(uint32_t i=1; i < matches.size(); ++i) {
      json << ",["
           << matches.at(i).first.x << ","
           << matches.at(i).first.y << ","
           << matches.at(i).first.a << ","
           << matches.at(i).first.b << ","
           << matches.at(i).first.c << ","
           << matches.at(i).second.x << ","
           << matches.at(i).second.y << ","
           << matches.at(i).second.a << ","
           << matches.at(i).second.b << ","
           << matches.at(i).second.c << "]";
    }
    json << "]";
  }
  json << "}";
}

void vise::relja_retrival::register_image(uint32_t file1_id, uint32_t file2_id,
                                          uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                                          std::array<double, 9> &H) const {
  if(!d_is_search_engine_loaded) {
    return;
  }
  boost::filesystem::path fn1 = d_image_dir / d_dataset->getInternalFn(file1_id);
  boost::filesystem::path fn2 = d_image_dir / d_dataset->getInternalFn(file2_id);

  featGetter *featGetterObj= new featGetter_standard( "hesaff-rootsift" );

  Magick::Image im1; im1.read(fn1.string());
  Magick::Image im2; im2.read(fn2.string());
  Magick::Image im2t;
  homography Hi(H.data()); // initial estimate of homography matrix
  uint32_t numFeats1, numFeats2, bestNInliers;
  float* descs1;
  float* descs2;
  std::vector<ellipse> regions1, regions2;

  double xl = x;
  double yl = y;
  double xu = x + width;
  double yu = y + height;

  // compute RootSIFT: image 1
  featGetterObj->getFeats(fn1.string().c_str(),
                          static_cast<uint32_t>(xl), static_cast<uint32_t>(xu),
                          static_cast<uint32_t>(yl), static_cast<uint32_t>(yu),
                          numFeats1, regions1, descs1);
  bool firstGo = true;
  uint32_t loopNum_ = 0;

  boost::filesystem::path tmp_im2_fn = boost::filesystem::temp_directory_path();
  tmp_im2_fn = tmp_im2_fn / boost::filesystem::unique_path("vise_register_%%%%-%%%%-%%%%-%%%%.jpg");
  matchesType inlierInds;
  while (1) {
	  if (!firstGo) {
		  // compute RootSIFT: image 2
		  featGetterObj->getFeats(tmp_im2_fn.string().c_str(),
                              static_cast<uint32_t>(xl), static_cast<uint32_t>(xu),
                              static_cast<uint32_t>(yl), static_cast<uint32_t>(yu),
                              numFeats2, regions2, descs2);

		  // run RANSAC
		  homography Hnew;
		  detRansac::matchDesc(*(d_spatial_retriever->getSameRandom()),
			  bestNInliers,
			  descs1, regions1,
			  descs2, regions2,
			  featGetterObj->numDims(),
			  loopNum_ > 1 ? 1.0 : 5.0, 0.0, 1000.0, static_cast<uint32_t>(4),
			  true, 0.81f, 100.0f,
			  &Hnew, &inlierInds
			  );
		  if (! (bestNInliers > 9)) {
			  // good alignment cannot be obtained, so be happy with the initial H
			  break;
		  }

		  // apply new H to current H (i.e. H= H * Hnew)
		  {
			  double Happlied[9];
			  for (int i = 0; i < 3; ++i) {
				  for (int j = 0; j < 3; ++j) {
					  Happlied[i * 3 + j] = Hi.H[i * 3] * Hnew.H[j] +
						  Hi.H[i * 3 + 1] * Hnew.H[3 + j] +
						  Hi.H[i * 3 + 2] * Hnew.H[6 + j];
				  }
			  }
			  Hi.set(Happlied);
		  }
	  }

	  Hi.normLast();

	  // im2 -> im1 transformation
	  double Hinv[9];
	  Hi.getInverse(Hinv);
	  homography::normLast(Hinv);

	  firstGo = false;
	  ++loopNum_;
	  if (loopNum_ > 1) {
		  break;
	  }

	  // warp image 2 into image 1
	  im2t = im2;
	  // source: https://gitlab.com/vgg/imcomp/-/blob/master/src/imreg_sift/imreg_sift.cc#L850
	  Magick::DrawableAffine hinv_affine(Hinv[0], Hinv[4], Hinv[3], Hinv[1], 0, 0);
	  std::ostringstream offset;
	  offset << im2.columns() << "x" << im2.rows() << "-" << ((int)Hinv[2]) << "-" << ((int)Hinv[5]);
	  im2t.artifact("distort:viewport", offset.str());
	  im2t.affineTransform(hinv_affine);
	  im2t.quiet(true);
	  im2t.write(tmp_im2_fn.string().c_str());
	  im2t.quiet(false);
  }
  delete[] descs1;
  delete[] descs2;
  boost::filesystem::remove(tmp_im2_fn);
  delete featGetterObj;

  // return the final computed Homography matrix
  Hi.getInverse(H.data());
  homography::normLast(H.data());
}

bool vise::relja_retrival::index_is_loaded() const {
  return d_is_search_engine_loaded;
}

uint32_t vise::relja_retrival::fid_count() const {
  if(d_is_search_engine_loaded) {
    return d_dataset->getNumDoc();
  } else {
    return 0;
  }
}

uint32_t vise::relja_retrival::fid(std::string filename) const {
  if(d_is_search_engine_loaded) {
    return d_dataset->getDocID(filename);
  } else {
    return 4294967295; //@todo: improve
  }
}

std::string vise::relja_retrival::filename(uint32_t fid) const {
  if(d_is_search_engine_loaded) {
    return d_dataset->getInternalFn(fid);
  } else {
    return "";
  }
}

void vise::relja_retrival::conf(std::map<std::string, std::string> conf_data) {
    d_pconf = conf_data;
}

std::map<std::string, std::string> vise::relja_retrival::conf() const {
    return d_pconf;
}


int32_t vise::relja_retrival::get_traindesc_count(std::string train_desc_fn) {
  FILE *f = fopen(train_desc_fn.c_str(), "rb");
  if ( f == NULL ) {
    std::cerr << "Failed to open training descriptors file: "
              << train_desc_fn << std::endl;
    return -1;
  }

  // file structure
  // SIFT-dimension (uint32_t, 4 bytes)
  // data-type-code (uint8_t , 1 byte )
  // continuous-stream-of-data ...
  // size_t fwrite( const void *buffer, size_t size, size_t count, FILE *stream );
  // size_t fread( void *buffer, size_t size, size_t count, FILE *stream );
  const std::size_t HEADER_BYTES = 4 + 1;
  const std::vector<std::string> DATA_TYPE_STR = {"uint8", "uint16", "uint32", "uint64", "float32", "float64"};
  const std::size_t DATA_TYPE_SIZE[] = {1, 2, 4, 8, 4, 8};

  uint32_t descriptor_dimension;
  uint8_t data_type_code;

  std::size_t read_count;
  read_count = fread(&descriptor_dimension, sizeof(descriptor_dimension), 1, f);
  if ( read_count != 1 ) {
    std::cerr << "Error reading value of descriptor_dimension! "
              << "stored in train descs file: " << train_desc_fn
              << std::endl;
    return -1;
  }

  read_count = fread(&data_type_code, sizeof(data_type_code), 1, f);
  std::cout << "read_count=" << read_count << std::endl;
  if ( read_count != 1 ) {
    std::cerr << "Error reading value of data_type_code stored in train descs file: " << train_desc_fn << std::endl;
    return -1;
  }

  uint32_t element_size = DATA_TYPE_SIZE[data_type_code];
  int64_t file_size;
#ifdef _WIN32
  _fseeki64(f, 0, SEEK_END);
  file_size = _ftelli64(f);
#else
  fseeko64(f, 0, SEEK_END);
  file_size = ftello64(f);
#endif
  uint32_t descriptor_data_length = (file_size - HEADER_BYTES) / (element_size);
  return (descriptor_data_length / descriptor_dimension);

}

bool vise::relja_retrival::pconf_validate_data_dir() {
  if(!boost::filesystem::exists(d_project_dir)) {
    std::cout << "project::conf_validate_data_dir(): "
              << "d_project_dir=" << d_project_dir << " does not exist"
              << std::endl;
    return false;
  }

  if(d_pconf.empty()) {
    std::cout << "project::conf_validate_data_dir(): "
              << "d_pconf is empty"
              << std::endl;
    return false;
  }

  d_data_dir      = d_project_dir / "data/";
  d_image_dir     = d_project_dir / "image/";
  d_image_src_dir = d_project_dir / "image_src/";
  d_tmp_dir       = d_project_dir / "tmp/";

  if(d_pconf.count("data_dir") == 1) {
    d_data_dir = boost::filesystem::path(d_pconf.at("data_dir"));
  }
  if(d_pconf.count("image_dir") == 1) {
    d_image_dir = boost::filesystem::path(d_pconf.at("image_dir"));
  }
  if(d_pconf.count("image_src_dir") == 1) {
    d_image_src_dir = boost::filesystem::path(d_pconf.at("image_src_dir"));
  }
  if(d_pconf.count("tmp_dir") == 1) {
    d_tmp_dir = boost::filesystem::path(d_pconf.at("tmp_dir"));
  }

  // convert the trailing path-separator to platform specific character
  d_data_dir.make_preferred();
  d_image_dir.make_preferred();
  d_image_src_dir.make_preferred();
  d_tmp_dir.make_preferred();

  if(!boost::filesystem::exists(d_data_dir) ||
     !boost::filesystem::exists(d_image_dir) ||
     !boost::filesystem::exists(d_image_src_dir) ||
     !boost::filesystem::exists(d_tmp_dir) ) {
    return false;
  } else {
    return true;
  }
}
