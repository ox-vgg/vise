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
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "tfidf_v2.h"
#include "util.h"
#include "wgc.h"



int main(int argc, char* argv[]){
    MPI_INIT_ENV
    
    std::string gtPath= util::expandUser("~/Relja/Data/gt");
    
    #if 1
    std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/dset_oxc1_5k.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/iidx_oxc1_5k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/fidx_oxc1_5k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/wght_oxc1_5k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string wgcFn = util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/wgc_oxc1_5k_hesaff_rootsift_scale3_1000000_43.v2bin");
    #else
    std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/dset_ox105k.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/iidx_ox105k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/fidx_ox105k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/wght_ox105k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string wgcFn = util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/wgc_ox105k_hesaff_rootsift_scale3_1000000_43.v2bin");
    #endif
    
    datasetV2 dset(dsetFn, util::expandUser("~/Relja/Databases/OxfordBuildings/") );
    
    protoDbFile dbFidx_file(fidxFn);
    protoDbInRam dbFidx(dbFidx_file);
    protoIndex fidx(dbFidx, false);
    
    protoDbFile dbIidx_file(iidxFn);
    protoDbInRam dbIidx(dbIidx_file);
    protoIndex iidx(dbIidx, false);
    
    tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);
    wgc wgcObj(iidx, &fidx, wgcFn);
    
    evaluatorV2 evalObj( gtPath, "Oxford", &dset );
    
    double mAP_tfidf= evalObj.computeMAP( tfidfObj, NULL, true, true );
    double mAP_wgc= evalObj.computeMAP( wgcObj, NULL, true, true );
    
    printf("mAP_tfidf= %.4f\n", mAP_tfidf);
    printf("mAP_wgc  = %.4f\n", mAP_wgc);
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
