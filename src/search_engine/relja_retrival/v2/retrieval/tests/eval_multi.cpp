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
#include "mq_joint_avg.h"
#include "multi_query.h"
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
    
    if (true){
        // single
        if (true){
            std::vector<double> APs;
            double mAP_hamm= evalObj.computeMAP( hammingObj, &APs, false, true );
            printf("hamm= %.4f\n", mAP_hamm);
            
            // single best cheating
            double mAP_hamm_cheat= 0;
            uint8_t const nImPerQ= APs.size()/11;
            for (uint32_t iMultiQuery= 0; iMultiQuery<11; ++iMultiQuery)
                mAP_hamm_cheat+= *std::max_element(&APs[iMultiQuery*nImPerQ], &APs[(iMultiQuery+1)*nImPerQ]);
            mAP_hamm_cheat/=11;
            printf("hamm_cheat= %.4f\n", mAP_hamm_cheat);
        }
        
        if (true){
            std::vector<double> APs;
            double mAP_hammsp= evalObj.computeMAP( spatVerifHamm, &APs, false, true );
            printf("hamm+sp= %.4f\n", mAP_hammsp);
            
            // single best cheating
            double mAP_hammsp_cheat= 0;
            uint8_t const nImPerQ= APs.size()/11;
            for (uint32_t iMultiQuery= 0; iMultiQuery<11; ++iMultiQuery)
                mAP_hammsp_cheat+= *std::max_element(&APs[iMultiQuery*nImPerQ], &APs[(iMultiQuery+1)*nImPerQ]);
            mAP_hammsp_cheat/=11;
            printf("hamm+sp_cheat= %.4f\n", mAP_hammsp_cheat);
        }
    }
    
    if (true){
        multiQueryMax mqMax( hammingObj );
        multiQueryMax mqMaxSpat( spatVerifHamm );
        
        if (true){
            double mAP_max= evalObj.computeMultiMAP( mqMax, NULL, true );
            printf("mq max= %.4f\n", mAP_max);
        }
        
        if (true){
            double mAP_maxsp= evalObj.computeMultiMAP( mqMaxSpat, NULL, true );
            printf("mq max+sp= %.4f\n", mAP_maxsp);
        }
    }
    
    if (true){
        multiQueryAvg mqAvg( hammingObj );
        multiQueryAvg mqAvgSpat( spatVerifHamm );
        
        if (true){
            double mAP_avg= evalObj.computeMultiMAP( mqAvg, NULL, true );
            printf("mq avg= %.4f\n", mAP_avg);
        }
        
        if (true){
            double mAP_avgsp= evalObj.computeMultiMAP( mqAvgSpat, NULL, true );
            printf("mq avg+sp= %.4f\n", mAP_avgsp);
        }
    }
    
    if (true){
        multiQueryJointAvg jtAvg( hammingObj );
        multiQueryJointAvg jtAvgSpat( hammingObj, &spatVerifHamm );
        
        if (true){
            double mAP_jtavg= evalObj.computeMultiMAP( jtAvg, NULL, true );
            printf("jt avg= %.4f\n", mAP_jtavg);
        }
        
        if (true){
            double mAP_jtavgsp= evalObj.computeMultiMAP( jtAvgSpat, NULL, true );
            printf("jt avg+sp= %.4f\n", mAP_jtavgsp);
        }
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
