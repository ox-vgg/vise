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

#include <boost/filesystem.hpp>

namespace vise {
  class relja_retrival:public search_engine {
  public:
    relja_retrival(std::map<std::string, std::string> const &pconf);
    ~relja_retrival();
    void index_create(bool &success, std::string &message) override;
    void index_load(bool &success, std::string &message) override;
    void index_unload(bool &success, std::string &message) override;
    bool index_is_loaded() override;
    bool index_is_done() override;
    void index_search(vise::search_query const &q,
                      std::vector<vise::search_result> &r) override;
  private:
    void index_run_all_stages();
    void create_file_list();
    void extract_train_descriptors();
    void cluster_train_descriptors();
    void assign_train_descriptors();
    void hamm_train_descriptors();
    void create_index();
    bool is_index_done();

    const std::map<std::string, std::string> d_pconf;
    const boost::filesystem::path d_storedir;
    const boost::filesystem::path d_datadir;
    boost::filesystem::path d_filelist_fn;
    boost::filesystem::path d_traindesc_fn;
    boost::filesystem::path d_bowcluster_fn;
    boost::filesystem::path d_trainassign_fn;
    boost::filesystem::path d_trainhamm_fn;
    boost::filesystem::path d_index_dset_fn;
    boost::filesystem::path d_index_iidx_fn;
    boost::filesystem::path d_index_fidx_fn;
    boost::filesystem::path d_index_tmp_dir;
    boost::filesystem::path d_weight_fn;

    std::thread d_thread_index;
    std::mutex d_search_engine_load_mutex;
    std::mutex d_search_engine_index_mutex;
    std::mutex d_search_engine_unload_mutex;

    bool d_is_search_engine_loaded;
    bool d_is_indexing_ongoing;
    bool d_is_indexing_done;

    // search engine data structures
    datasetV2 *d_dataset;
    sequentialConstructions *d_cons_queue;
    protoDbFile *d_db_fidx_file;
    protoDbFile *d_db_iidx_file;
    protoDbInRamStartDisk *d_db_fidx;
    protoDbInRamStartDisk *d_db_iidx;
    protoIndex *d_fidx;
    protoIndex *d_iidx;
    featGetter *d_feat_getter;
    clstCentres *d_clst_centres;
    VlKDForest *d_kd_forest;
    softAssigner *d_soft_assigner;
    hamming *d_hamming_emb;
    embedderFactory *d_emb_factory;
    tfidfV2 *d_tfidf;
    retrieverFromIter *d_base_retriever;
    spatialVerifV2 *d_spatial_verif_v2;
    spatialRetriever *d_spatial_retriever;
    multiQuery *d_multi_query;
    multiQueryMax *d_multi_query_max;
  };
}
#endif
