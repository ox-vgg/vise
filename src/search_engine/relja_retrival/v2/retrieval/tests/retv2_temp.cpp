/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

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



int main(int argc, char* argv[]){
    MPI_INIT_ENV
    
    std::string gtPath= util::expandUser("~/Relja/Data/gt");
    
#if 1
    #if 1
    std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/dset_ox5k.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/iidx_ox5k_hesrootsift3_dParis_100k.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/fidx_ox5k_hesrootsift3_dParis_100k.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/wght_ox5k_hesrootsift3_dParis_100k.v2bin");
    std::string hammFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/train_paris_hesrootsift3_100k_hamm64.v2bin");
    #else
    std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox105k/hamm/dset_ox105k.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox105k/hamm/iidx_ox105k_hesrootsift3_dParis_100k.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox105k/hamm/fidx_ox105k_hesrootsift3_dParis_100k.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox105k/hamm/wght_ox105k_hesrootsift3_dParis_100k.v2bin");
    std::string hammFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/train_paris_hesrootsift3_100k_hamm64.v2bin");
    #endif
    
    datasetV2 dset(dsetFn, util::expandUser("~/Relja/Databases/OxfordBuildings/") );
    evaluatorV2 evalObj( gtPath, "Oxford", &dset );
#else

    std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/holidays/dset_holidays.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/holidays/iidx_holidays_public_rootsift_100k.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/holidays/fidx_holidays_public_rootsift_100k.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/holidays/wght_holidays_public_rootsift_100k.v2bin");
    std::string hammFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/holidays/train_flickr60k_public_rootsift_100k_hamm64.v2bin");
    
    datasetV2 dset(dsetFn, util::expandUser("~/Relja/Databases/Holidays/jpg_pp/") );
    evaluatorV2 evalObj( gtPath, "Holidays", &dset );
#endif
    
    protoDbFile dbFidx_file(fidxFn);
    protoDbInRam dbFidx(dbFidx_file);
    protoIndex fidx(dbFidx, false);
    
    protoDbFile dbIidx_file(iidxFn);
    protoDbInRam dbIidx(dbIidx_file);
    protoIndex iidx(dbIidx, false);
    
    tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);
    
    hammingEmbedderFactory embFactory(hammFn, 64);
    hamming hammingObj(tfidfObj, &iidx, embFactory, &fidx);
    
    spatialVerifV2 spatVerifTfidf(tfidfObj, &iidx, &fidx, true);
    spatialVerifV2 spatVerifHamm(hammingObj, &iidx, &fidx, true);
    
    uint32_t numTests= std::min(300u, dset.getNumDoc());
    
    if (false) {
        timing::progressPrint progressPrint(numTests, "InternalQuerySpeedTest tfidf");
        std::vector<indScorePair> queryRes;
        
        for (uint32_t docID= 0; docID<numTests; ++docID, progressPrint.inc())
            tfidfObj.internalQuery(docID, queryRes, 1000);
    }
    
    if (false) {
        timing::progressPrint progressPrint(numTests, "InternalQuerySpeedTest hamming");
        std::vector<indScorePair> queryRes;
        
        for (uint32_t docID= 0; docID<numTests; ++docID, progressPrint.inc())
            hammingObj.internalQuery(docID, queryRes, 1000);
    }
    
    if (false) {
        timing::progressPrint progressPrint(numTests, "InternalQuerySpeedTest spat ");
        std::vector<indScorePair> queryRes;
        
        for (uint32_t docID= 0; docID<numTests; ++docID, progressPrint.inc())
            spatVerifTfidf.internalQuery(docID, queryRes, 1000);
    }
    
    if (false){
        double mAP_tfidf= evalObj.computeMAP( tfidfObj, NULL, false, true );
        printf("mAP_tfidf= %.4f\n", mAP_tfidf);
    }
    
    if (false){
        double mAP_tfidfsp= evalObj.computeMAP( spatVerifTfidf, NULL, false, true );
        printf("mAP_tfidf+sp= %.4f\n", mAP_tfidfsp);
    }
    
    if (true){
        double mAP_hamm= evalObj.computeMAP( hammingObj, NULL, false, true );
        printf("mAP_hamm= %.4f\n", mAP_hamm);
    }
    
    if (true){
        double mAP_hammsp= evalObj.computeMAP( spatVerifHamm, NULL, false, true );
        printf("mAP_hamm+sp= %.4f\n", mAP_hammsp);
    }
    
    if (false){
        double rec_hamm= evalObj.computeAvgRecallAtN( hammingObj, 50, 10, NULL, true, true );
        printf("rec_hamm= %.4f\n", rec_hamm);
    }
    
    if (false){
        double rec_hammsp= evalObj.computeAvgRecallAtN( spatVerifHamm, 50, 10, NULL, true, true );
        printf("rec_hamm+sp= %.4f\n", rec_hammsp);
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
