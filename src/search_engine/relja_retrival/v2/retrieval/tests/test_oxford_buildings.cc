//
// Evaluate search engine on standard datasets
//
// Author: Abhishek Dutta <adutta _AT_ robots.ox.ac.uk>
// Date: 04 Aug. 2020
//
// source: relja_retrieval/src/v2/retrieval/tests/retv2_temp.cpp

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <iomanip>

#include "dataset_v2.h"
#include "evaluator_v2.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "spatial_verif_v2.h"
#include "tfidf_v2.h"
#include "timing.h"
#include "util.h"
#include "vise/vise_util.h"

#include <boost/filesystem.hpp>

int main(int argc, char **argv) {
  if(argc != 4) {
    std::cout << "Usage: " << argv[0] << " OX5K_GND_TRUTH_DIR PROJECT_CONFIG_FILENAME RESULT_FILENAME" << std::endl;
    return 1;
  }
  std::string gtPath(argv[1]);
  std::string conf_fn(argv[2]);
  std::string out_fn(argv[3]);
  std::map<std::string, std::string> conf;
  vise::configuration_load(conf_fn, conf);

  boost::filesystem::path data_dir = boost::filesystem::path(conf_fn).parent_path();
  if(conf.count("data_dir")) {
    boost::filesystem::path conf_data_dir(conf.at("data_dir"));
    if(boost::filesystem::exists(conf_data_dir)) {
      data_dir = conf_data_dir;
    }
  }
  data_dir = data_dir / "/";
  data_dir.make_preferred();

  std::string dsetFn= data_dir.string() + "index_dset.bin";
  std::string iidxFn= data_dir.string() + "index_iidx.bin";
  std::string fidxFn= data_dir.string() + "index_fidx.bin";
  std::string wghtFn= data_dir.string() + "weight.bin";
  std::string hammFn= data_dir.string() + "trainhamm.bin";

  datasetV2 dset(dsetFn, conf["image_src_dir"] );
  evaluatorV2 evalObj( gtPath, "Oxford", &dset );

  protoDbFile dbFidx_file(fidxFn);
  protoDbInRam dbFidx(dbFidx_file);
  protoIndex fidx(dbFidx, false);

  protoDbFile dbIidx_file(iidxFn);
  protoDbInRam dbIidx(dbIidx_file);
  protoIndex iidx(dbIidx, false);

  tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);

  uint32_t hamm_embedding_bits = 0;
  std::istringstream ss(conf.at("hamm_embedding_bits"));
  ss >> hamm_embedding_bits;

  hammingEmbedderFactory embFactory(hammFn, hamm_embedding_bits);
  hamming hammingObj(tfidfObj, &iidx, embFactory, &fidx);

  spatialVerifV2 spatVerifTfidf(tfidfObj, &iidx, &fidx, true);
  spatialVerifV2 spatVerifHamm(hammingObj, &iidx, &fidx, true);

  double mAP_hamm= evalObj.computeMAP( hammingObj, NULL, false, true );
  printf("mAP_hamm= %.4f\n", mAP_hamm);

  double mAP_hammsp= evalObj.computeMAP( spatVerifHamm, NULL, false, true );
  printf("mAP_hamm+sp= %.4f\n", mAP_hammsp);

  std::ofstream outf(out_fn);
  outf << "mAP_hamm=" << std::setprecision(5) << mAP_hamm << std::endl;
  outf << "mAP_hamm+sp=" << std::setprecision(5) << mAP_hammsp << std::endl;
  outf.close();

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
