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

#include <fstream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "common_data.pb.h"
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
#include "uniq_retriever.h"
#include "util.h"



// Quantize UTM
void getPittsID(std::string utmFn, std::vector<uint32_t> &docIDtoObjID ){
    
    std::ifstream fin(utmFn.c_str());
    
    std::vector<double> pos;
    pos.reserve(10586 * 2);
    
    double coord, minX= 0, minY= 0, maxX= 0, maxY= 0;
    bool isFirst= true;
    
    while (fin>>coord){
        pos.push_back(coord);
        if (isFirst){
            minX= coord;
            maxX= coord;
        } else {
            minX= std::min(minX, coord);
            maxX= std::max(maxX, coord);
        }
        
        fin>>coord;
        pos.push_back(coord);
        if (isFirst){
            minY= coord;
            maxY= coord;
        } else {
            minY= std::min(minY, coord);
            maxY= std::max(maxY, coord);
        }
    }
    ASSERT( pos.size()==10586 * 2 );
    
    fin.close();
    
    minX-=0.1;
    minY-=0.1;
    double const d= 25;
    double w= maxX-minX, h= maxY-minY;
    ASSERT(w/d<200 && h/d<200 );
    uint32_t const stride= w/d + 5;
    
    docIDtoObjID.reserve( pos.size()/2 * 24 );
    uint32_t x, y;
    for (uint32_t i= 0; i<pos.size(); i+=2){
        x= (pos[i]-minX)/d;
        y= (pos[i+1]-minY)/d;
        docIDtoObjID.resize( docIDtoObjID.size()+24, x*stride+y );
    }
}



int main(int argc, char* argv[]){
    MPI_INIT_ENV
    
    std::string gtPath= util::expandUser("~/Relja/Data/gt");
    
    std::string dsetFn= util::expandUser("~/Relja/Data/Pittsburgh/dset_pitts.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/Pittsburgh/iidx_pitts_hesrootsift3up_200k.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/Pittsburgh/fidx_pitts_hesrootsift3up_200k.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/Pittsburgh/wght_pitts_hesrootsift3up_200k.v2bin");
//     std::string uniqObjFn= util::expandUser("~/Relja/Data/Pittsburgh/uniqobj_sf.v2bin");
    std::string utmFn= util::expandUser("~/Relja/Databases/Pittsburgh/groundtruth/utm_export.txt");
    
    std::string hammFn= util::expandUser("~/Relja/Data/Pittsburgh/train/train_pitts_hesrootsift3up_200k_hamm64.v2bin");
    
    datasetV2 dset(dsetFn, util::expandUser("~/Relja/Databases/Pittsburgh/") );
    evaluatorV2 evalObj( gtPath,
        "Pittsburgh_2013_03",
        &dset );
    
    std::vector<uint32_t> docIDtoObjID;
    getPittsID(utmFn, docIDtoObjID);
    
    protoDbFile dbFidx_file(fidxFn);
    protoDbInRam dbFidx(dbFidx_file);
    protoIndex fidx(dbFidx, false);
    
    protoDbFile dbIidx_file(iidxFn);
    protoDbInRam dbIidx(dbIidx_file);
    protoIndex iidx(dbIidx, false);
    
    tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);
    
    hammingEmbedderFactory embFactory(hammFn, 64);
    hamming hammingObj(tfidfObj, &iidx, embFactory, &fidx);
    uniqRetriever uqHammingObj(hammingObj, docIDtoObjID);
    
//     spatialVerifV2 spatVerifTfidf(tfidfObj, &iidx, &fidx, true);
    spatialVerifV2 spatVerifHamm(hammingObj, &iidx, &fidx, true);
    
    if (true){
        double rec_hamm= evalObj.computeAvgRecallAtN( hammingObj, 100, 10, NULL, true, true );
        printf("rec_hamm= %.4f\n", rec_hamm);
    }
    
    if (false){
        double rec_uqhamm= evalObj.computeAvgRecallAtN( uqHammingObj, 100, 10, NULL, true, true );
        printf("rec_uqhamm= %.4f\n", rec_uqhamm);
    }
    
    if (false){
        double rec_hammsp= evalObj.computeAvgRecallAtN( spatVerifHamm, 100, 10, NULL, true, true );
        printf("rec_hamm+sp= %.4f\n", rec_hammsp);
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
