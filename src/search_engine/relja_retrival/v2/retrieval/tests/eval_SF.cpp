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



// cartoID is 64 bit, I conver it to a new ID which is 32 bit
void getSFCartoID(datasetV2 const &dset, std::string uniqObjFn, std::vector<uint32_t> &docIDtoObjID ){
    
    if (!boost::filesystem::exists( uniqObjFn ) ){
        
        std::cout<<"getSFCartoID:: Generating docIDtoObjID\n";
        std::map<uint64_t, uint32_t> cartoIDtoObjID;
        docIDtoObjID.clear();
        docIDtoObjID.reserve(dset.getNumDoc());
        uint32_t numObjIDs= 0;
        
        // from evaluatorV2::convertSF
        std::string gtLine, imageFn;
        
        for (uint32_t docID= 0; docID<dset.getNumDoc(); ++docID){
            if (docID%50000==0)
                std::cout<<docID<<"\n";
            imageFn= dset.getInternalFn(docID);
            size_t pos= imageFn.find("PCI_sp_");
            ASSERT(pos!=std::string::npos);
            imageFn= imageFn.substr(pos);
            std::vector<std::string> strList;
            boost::split(strList, imageFn, boost::is_any_of("_"));
            if (strList.size()!=10){
                std::cout<<imageFn<<"\n"<<strList.size()<<"\n";
            }
            ASSERT(strList.size()==10);
            
            uint64_t cartoID= boost::lexical_cast<uint32_t>(strList[7]);
            
            // cartoID -> objID
            uint32_t thisObjID= numObjIDs;
            if (cartoIDtoObjID.count(cartoID)==0){
                cartoIDtoObjID[cartoID]= numObjIDs;
                ++numObjIDs;
            } else {
                thisObjID= cartoIDtoObjID[cartoID];
            }
            
            docIDtoObjID.push_back(thisObjID);
        }
        
        // save
        std::cout<<"getSFCartoID:: saving docIDtoObjID\n";
        
        rr::uint32vec data;
        data.mutable_v()->Reserve(docIDtoObjID.size());
        for (uint32_t i= 0; i<docIDtoObjID.size(); ++i)
            data.add_v(docIDtoObjID[i]);
        
        std::ofstream of(uniqObjFn.c_str(), std::ios::binary);
        data.SerializeToOstream(&of);
        of.close();
        
    } else {
        
        // load
        std::cout<<"getSFCartoID:: loading docIDtoObjID\n";
        
        rr::uint32vec data;
        std::ifstream in(uniqObjFn.c_str(), std::ios::binary);
        ASSERT(data.ParseFromIstream(&in));
        in.close();
        
        docIDtoObjID.clear();
        docIDtoObjID.reserve(data.v_size());
        for (int i= 0; i<data.v_size(); ++i)
            docIDtoObjID.push_back( data.v(i) );
    }
}



void getSF_UTM_ID(std::string utmFn, std::vector<uint32_t> &docIDtoObjID ){
    
    std::ifstream fin(utmFn.c_str());
    
    std::vector<double> pos;
    pos.reserve(1070000 * 2);
    
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
    ASSERT( pos.size()==159792 * 2 || pos.size()==1062468 * 2 );
    
    fin.close();
    
    minX-=0.1;
    minY-=0.1;
    double const d= 25;
    double w= maxX-minX, h= maxY-minY;
    ASSERT(w/d < 200 && h/d<200 );
    uint32_t const stride= w/d + 5;
    
    docIDtoObjID.reserve( pos.size()/2 );
    uint32_t x, y;
    for (uint32_t i= 0; i<pos.size(); i+=2){
        x= (pos[i]-minX)/d;
        y= (pos[i+1]-minY)/d;
        docIDtoObjID.push_back( x*stride+y );
    }
}



int main(int argc, char* argv[]){
    MPI_INIT_ENV
    
    std::string gtPath= util::expandUser("~/Relja/Data/gt");
    
    #if 1
    std::string dsetFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/dset_sf.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/iidx_sf_hesrootsift3up_200k.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/fidx_sf_hesrootsift3up_200k.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/wght_sf_hesrootsift3up_200k.v2bin");
    std::string uniqObjFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/uniqobj_sf.v2bin");
    std::string utmFn= util::expandUser("~/Relja/Databases/SF_landmarks/utm_export.txt");
    #else
    std::string dsetFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/dset_sfmini.v2bin");
    std::string iidxFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/iidx_sfmini_hesrootsift3up_200k.v2bin");
    std::string fidxFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/fidx_sfmini_hesrootsift3up_200k.v2bin");
    std::string wghtFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/wght_sfmini_hesrootsift3up_200k.v2bin");
    std::string uniqObjFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/uniqobj_sfmini.v2bin");
    std::string utmFn= util::expandUser("~/Relja/Databases/SF_landmarks/minimal/utm_export.txt");
    #endif
    
    std::string hammFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/train/train_sf_hesrootsift3up_200k_hamm64.v2bin");
    
    datasetV2 dset(dsetFn, util::expandUser("~/Relja/Databases/SF_landmarks/") );
    evaluatorV2 evalObj( gtPath,
        //"SF_landmarks_2011_04",
        "SF_landmarks_2014_04",
        &dset );
    
    std::vector<uint32_t> docIDtoObjID;
    getSFCartoID(dset, uniqObjFn, docIDtoObjID);
//     getSF_UTM_ID(utmFn, docIDtoObjID_utm );
    
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
