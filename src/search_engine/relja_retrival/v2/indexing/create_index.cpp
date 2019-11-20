/*

  The task of creating a searchable index of images requires
  multiple stages (e.g. trainDescs, trainCluster, etc.). This
  module allows invocation of all those stages.

  Author: Abhishek Dutta (adutta@robots.ox.ac.uk)
  Original Author: Relja Arandjelovic (relja@robots.ox.ac.uk, 2014)
*/


#include <string>
#include <omp.h>

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <google/protobuf/stubs/common.h>

#include "build_index.h"
#include "embedder.h"
#include "feat_standard.h"
#include "hamming_embedder.h"
#include "mpi_queue.h"
#include "python_cfg_to_ini.h"
#include "train_assign.h"
#include "train_descs.h"
#include "train_cluster.h"
#include "train_hamming.h"
#include "util.h"



int main(int argc, char* argv[]) {

  MPI_INIT_ENV
  MPI_GLOBAL_ALL

  if (rank==0) {
    #ifdef RR_MPI
    std::cout<<"Number of MPI processes = "<<numProc<<"\n";
    #else
    std::cout<<"Not using MPI. Number of MPI processes = "<<numProc<<"\n";
    #endif
  }

  bool use_threads = detectUseThreads();
  if ( use_threads ) {
    std::cout << "OpenMP threads = " << omp_get_max_threads() << std::endl;
  }

  // ------------------------------------ setup config

  if ( argc != 4 ) {
    std::cout << "\nUsage: " << argv[0]
              << " [trainDescs|trainAssign|trainHamm|index] SEARCH_ENGINE_NAME CONFIG_FILENAME"
              << std::endl;
    return 0;
  }
  std::string stage    = argv[1];
  std::string dsetname = argv[2];
  std::string configFn = argv[3];

  configFn= util::expandUser(configFn);
  std::string tempConfigFn= util::getTempFileName();
  pythonCfgToIni( configFn, tempConfigFn );

  boost::property_tree::ptree pt;
  boost::property_tree::ini_parser::read_ini(tempConfigFn, pt);

  // ------------------------------------ read config
  bool const useRootSIFT= pt.get<bool>(dsetname+".RootSIFT", true);
  bool const SIFTscale3= pt.get<bool>( dsetname+".SIFTscale3", true);
  std::string const asset_dir = pt.get<std::string>( dsetname+".asset_dir", "./" );
  std::string const data_dir  = pt.get<std::string>( dsetname+".data_dir", "./" );
  std::string const temp_dir  = pt.get<std::string>( dsetname+".temp_dir", "./" );

  if (stage=="trainDescs"){
    // ------------------------------------ compute training descriptors (i.e. for clustering)

    // feature getter
    // NOTE: regardless of RootSIFT, extract SIFT as requires 4 times less storage, can convert later
    featGetter_standard const featGetter_obj( (
                                               std::string("hesaff-") +
                                               "sift" +
                                               std::string(SIFTscale3 ? "-scale3" : "")
                                               ).c_str() );

    std::string const imagelistFn        = data_dir + pt.get<std::string>( dsetname+".imagelistFn", "" );
    std::string const trainImagelistFn   = pt.get<std::string>( dsetname+".trainImagelistFn", imagelistFn);
    std::string const databasePath       = asset_dir + "image/";
    std::string const trainDatabasePath  = pt.get<std::string>( dsetname+".trainDatabasePath", databasePath);
    int32_t trainNumDescs                = pt.get<int32_t>( dsetname+".trainNumDescs", -1 );
    std::string const trainDescsFn       = data_dir + pt.get<std::string>( dsetname+".descsFn", "descs.e3bin" );

    buildIndex::computeTrainDescs(
                                  trainImagelistFn, trainDatabasePath,
                                  trainDescsFn,
                                  trainNumDescs,
                                  featGetter_obj);

  } else if (stage=="trainCluster"){
    // ------------------------------------ assign training descs to clusters
    std::string const train_desc_fn = data_dir + pt.get<std::string>( dsetname+".descsFn", "descs.e3bin" );
    std::string const cluster_fn    = data_dir + pt.get<std::string>( dsetname+".clstFn", "clst.e3bin" );
    uint32_t const bow_cluster_count     = pt.get<uint32_t>( dsetname+".bow_cluster_count" );
    uint32_t const cluster_num_iteration = pt.get<uint32_t>( dsetname+".cluster_num_iteration" );

    buildIndex::compute_train_cluster( train_desc_fn, useRootSIFT, cluster_fn, bow_cluster_count, cluster_num_iteration);

  } else if (stage=="trainAssign"){
    // ------------------------------------ assign training descs to clusters
    std::string const trainDescsFn       = data_dir + pt.get<std::string>( dsetname+".descsFn", "descs.e3bin" );
    std::string const trainAssignsFn     = data_dir + pt.get<std::string>( dsetname+".assignFn", "assign.bin" );
    std::string const clstFn             = data_dir + pt.get<std::string>( dsetname+".clstFn", "clst.e3bin" );

    buildIndex::computeTrainAssigns( clstFn, useRootSIFT, trainDescsFn, trainAssignsFn);

  } else if (stage=="trainHamm"){
    // ------------------------------------ compute hamming stuff
    // (i.e. rotation, medians)

    uint32_t const hammEmbBits= pt.get<uint32_t>( dsetname+".hammEmbBits" );
    std::string const trainDescsFn       = data_dir + pt.get<std::string>( dsetname+".descsFn", "descs.e3bin" );
    std::string const trainAssignsFn     = data_dir + pt.get<std::string>( dsetname+".assignFn", "assign.bin" );
    std::string const trainHammFn        = data_dir + pt.get<std::string>( dsetname+".hammFn", "hamm.v2bin" );
    std::string const clstFn             = data_dir + pt.get<std::string>( dsetname+".clstFn", "clst.e3bin" );

    buildIndex::computeHamming(clstFn, useRootSIFT, trainDescsFn, trainAssignsFn, trainHammFn, hammEmbBits);

  } else if (stage=="index"){
    // ------------------------------------ compute index

    std::string const imagelistFn  = data_dir + pt.get<std::string>( dsetname+".imagelistFn" );
    std::string const databasePath = asset_dir + "image/";
    std::string const dsetFn       = data_dir + pt.get<std::string>( dsetname+".dsetFn" );
    std::string const iidxFn       = data_dir + pt.get<std::string>( dsetname+".iidxFn" );
    std::string const fidxFn       = data_dir + pt.get<std::string>( dsetname+".fidxFn" );
    std::string const clstFn       = data_dir + pt.get<std::string>( dsetname+".clstFn", "clst.e3bin" );
    boost::optional<uint32_t> const hammEmbBits  = pt.get_optional<uint32_t>( dsetname+".hammEmbBits" );

    // feature getter
    featGetter_standard const featGetter_obj( (
                                               std::string("hesaff-") +
                                               std::string((useRootSIFT ? "rootsift" : "sift")) +
                                               std::string(SIFTscale3 ? "-scale3" : "")
                                               ).c_str() );

    // embedder
    embedderFactory *embFactory= NULL;
    if (hammEmbBits.is_initialized()){
      std::string const trainHammFn        = data_dir + pt.get<std::string>( dsetname+".hammFn", "hamm.v2bin" );

      embFactory= new hammingEmbedderFactory(trainHammFn, *hammEmbBits);
    }
    else
      embFactory= new noEmbedderFactory;
    //     rawEmbedderFactory embFactory(featGetter_obj.numDims());

    buildIndex::build(imagelistFn, databasePath,
                      dsetFn,
                      iidxFn,
                      fidxFn,
                      temp_dir,
                      featGetter_obj,
                      clstFn,
                      embFactory );

    delete embFactory;
  } else {
    throw std::runtime_error( std::string("Unrecognized stage: ") + stage);
  }

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
