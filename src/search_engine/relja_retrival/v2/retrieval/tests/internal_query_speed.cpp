/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

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
#include "spatial_verif_v2.h"
#include "tfidf_v2.h"
#include "timing.h"
#include "util.h"



int main(int argc, char* argv[]){
    MPI_INIT_ENV
    
    std::string gtPath= util::expandUser("~/Relja/Data/gt");
    
    #if 1
    #if 1
    std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/dset_oxc1_5k.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/iidx_oxc1_5k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/fidx_oxc1_5k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox5k/wght_oxc1_5k_hesaff_rootsift_scale3_1000000_43.v2bin");
    #else
    std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/dset_ox105k.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/iidx_ox105k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/fidx_ox105k_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_ox105k/wght_ox105k_hesaff_rootsift_scale3_1000000_43.v2bin");
    #endif
    #else
    std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_BBCb/dset_BBCb.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_BBCb/iidx_BBCb_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_BBCb/fidx_BBCb_hesaff_rootsift_scale3_1000000_43.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/indexing_BBCb/wght_BBCb_hesaff_rootsift_scale3_1000000_43.v2bin");
    #endif
    
    datasetV2 dset(dsetFn, util::expandUser("~/Relja/Databases/OxfordBuildings/") );
    
    protoDbFile dbFidx_file(fidxFn);
    protoDbInRam dbFidx(dbFidx_file);
    protoIndex fidx(dbFidx, false);
    
    protoDbFile dbIidx_file(iidxFn);
    protoDbInRam dbIidx(dbIidx_file);
    protoIndex iidx(dbIidx, false);
    
    tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);
    spatialVerifV2 spatVerifObj(tfidfObj, &iidx, &fidx, true);
    
    uint32_t numTests= std::min(300u, dset.getNumDoc());
    
    {
        timing::progressPrint progressPrint(numTests, "InternalQuerySpeedTest tfidf");
        std::vector<indScorePair> queryRes;
        
        for (uint32_t docID= 0; docID<numTests; ++docID, progressPrint.inc())
            tfidfObj.internalQuery(docID, queryRes, 1000);
    }
    
    {
        timing::progressPrint progressPrint(numTests, "InternalQuerySpeedTest spat ");
        std::vector<indScorePair> queryRes;
        
        for (uint32_t docID= 0; docID<numTests; ++docID, progressPrint.inc())
            spatVerifObj.internalQuery(docID, queryRes, 1000);
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
