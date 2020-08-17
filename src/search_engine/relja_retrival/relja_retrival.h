/** @file relja_retrival.h
 *  @brief Interface to the visual search engine developed by
 *         Relja Arandjelovic during his DPhil / Postdoc at
 *         the Visual Geometry Group, Department of Engineering
 *         Science, University of Oxford in 2014.
 *  @author Abhishek Dutta
 *  @date 20 Nov. 2019
 */
#ifndef RELJA_RETRIVAL_H
#define RELJA_RETRIVAL_H

#include "vise/search_engine.h"
#include "vise/search_result.h"
#include "vise/vise_util.h"
#include "vise/task_progress.h"

// for building an index
#include "build_index.h"
#include "embedder.h"
#include "feat_standard.h"
#include "hamming_embedder.h"
#include "mpi_queue.h"
#include "train_assign.h"
#include "train_descs.h"
#include "train_cluster.h"
#include "train_hamming.h"
#include "util.h"

// for loading an index
#include "dataset_v2.h"        // for datasetAbs
#include "slow_construction.h" // for sequentialConstructions
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "feat_getter.h"
#include "feat_standard.h"
#include "clst_centres.h"
#include "soft_assigner.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "tfidf_data.pb.h"
#include "tfidf_v2.h"
#include "spatial_verif_v2.h"
#include "mq_filter_outliers.h"

// for register_image()
#include "ellipse.h"
#include "feat_getter.h"
#include "feat_standard.h"
#include "putative.h"
#include "det_ransac.h"
#include "macros.h"
#include "register_images.h"

#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <exception>
#include <fstream>
#include <unordered_map>
#include <functional>
#include <stdexcept>

#include <boost/filesystem.hpp>

#include <Magick++.h>

namespace vise {
  class relja_retrival:public search_engine {
  public:
    relja_retrival(boost::filesystem::path pconf_fn,
                   boost::filesystem::path project_dir);
    ~relja_retrival();
    void index_create(bool &success,
                      std::string &message,
                      std::function<void(void)> callback,
                      bool block_until_done=false) override;
    void index_load(bool &success, std::string &message) override;
    void index_unload(bool &success, std::string &message) override;
    bool index_is_loaded() const override;
    bool index_is_done() const override;
    bool index_is_incomplete() const override;
    bool index_is_ongoing() const override;
    std::string index_status() const override;

    void index_search(vise::search_query const &q,
                      std::vector<vise::search_result> &r) const override;
    void index_internal_match(vise::search_query const &q,
                              uint32_t match_file_id,
                              std::ostringstream &json) const override;
    void register_image(uint32_t file1_id, uint32_t file2_id,
                        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
                        std::array<double, 9> &H) const;

    uint32_t fid_count() const override;
    uint32_t fid(std::string filename) const override;
    std::string filename(uint32_t fid) const override;

    void conf(std::map<std::string, std::string> conf_data) override;
    std::map<std::string, std::string> conf() const override;
  private:
    void index_run_all_stages(std::function<void(void)> callback);
    void preprocess_image(std::string srcfn,
                          std::string dstfn,
                          bool &result,
                          std::string &message);
    uint32_t image_src_count() const;
    void create_file_list();
    void extract_train_descriptors();
    void cluster_train_descriptors();
    void assign_train_descriptors();
    void hamm_train_descriptors();
    void create_index();

    void index_read_status(std::vector<std::string> &status_tokens) const;
    int32_t get_traindesc_count(std::string train_desc_fn);

    void findBBox2( double xl, double xu, double yl, double yu, homography const &H, double &xl2, double &xu2, double &yl2, double &yu2, uint32_t w2, uint32_t h2 ) const;

    boost::filesystem::path d_data_dir;
    boost::filesystem::path d_image_dir;
    boost::filesystem::path d_image_src_dir;
    boost::filesystem::path d_tmp_dir;
    boost::filesystem::path d_pconf_fn;
    boost::filesystem::path d_project_dir;
    std::map<std::string, std::string> d_pconf;
    bool pconf_validate_data_dir();

    boost::filesystem::path d_filelist_fn;
    boost::filesystem::path d_filestat_fn;
    boost::filesystem::path d_traindesc_fn;
    boost::filesystem::path d_bowcluster_fn;
    boost::filesystem::path d_trainassign_fn;
    boost::filesystem::path d_trainhamm_fn;
    boost::filesystem::path d_index_dset_fn;
    boost::filesystem::path d_index_iidx_fn;
    boost::filesystem::path d_index_fidx_fn;
    boost::filesystem::path d_index_tmp_dir;
    boost::filesystem::path d_weight_fn;
    boost::filesystem::path d_index_status_fn;
    boost::filesystem::path d_index_log_fn;

    std::thread d_thread_index;
    std::mutex d_search_engine_load_mutex;
    std::mutex d_search_engine_index_mutex;
    std::mutex d_search_engine_unload_mutex;

    bool d_is_search_engine_loaded;
    bool d_is_indexing_ongoing;
    bool d_is_indexing_done;

    // search engine data structures
    datasetV2 *d_dataset = nullptr;
    sequentialConstructions *d_cons_queue = nullptr;
    protoDbFile *d_db_fidx_file = nullptr;
    protoDbFile *d_db_iidx_file = nullptr;
    protoDbInRamStartDisk *d_db_fidx = nullptr;
    protoDbInRamStartDisk *d_db_iidx = nullptr;
    protoIndex *d_fidx = nullptr;
    protoIndex *d_iidx = nullptr;
    featGetter *d_feat_getter = nullptr;
    clstCentres *d_clst_centres = nullptr;
    VlKDForest *d_kd_forest = nullptr;
    softAssigner *d_soft_assigner = nullptr;
    hamming *d_hamming_emb = nullptr;
    embedderFactory *d_emb_factory = nullptr;
    tfidfV2 *d_tfidf = nullptr;
    retrieverFromIter *d_base_retriever = nullptr;
    spatialVerifV2 *d_spatial_verif_v2 = nullptr;
    spatialRetriever *d_spatial_retriever = nullptr;
    multiQuery *d_multi_query = nullptr;
    multiQueryMax *d_multi_query_max = nullptr;

    std::ofstream d_log;
    static const std::vector<std::string> task_name_list;
    std::unordered_map<std::string, vise::task_progress> d_task_progress_list;
  };
}
#endif
