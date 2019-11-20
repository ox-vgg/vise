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

#include <iostream>
#include <string.h>

#include <boost/format.hpp>

#include <fastann/fastann.hpp>

#include "feat_standard.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "tfidf_v2.h"
#include "util.h"



int main(){
    
    std::string clstFn= "~/Relja/Data/SF_landmarks/hamm/clst_sf_hesrootsift3up_200k.e3bin";
    std::string wghtFn= util::expandUser("~/Relja/Data/SF_landmarks/hamm/wght_sf_hesrootsift3up_200k.v2bin");
    
    // feature getter
    featGetter_standard featGetter_obj( "hesaff-rootsift-scale3-up" );
    
    // clusters
    std::cout<<"SFqueryfeats::main: Loading cluster centres\n";
    double t0= timing::tic();
    clstCentres clstCentres_obj( util::expandUser(clstFn).c_str(), true );
    std::cout<<"SFqueryfeats::main: Loading cluster centres - DONE ("<< timing::toc(t0) <<" ms)\n";
    
    std::cout<<"SFqueryfeats::main: Constructing NN search object\n";
    t0= timing::tic();
    
    fastann::nn_obj<float> *nn= fastann::nn_obj_build_kdtree(
            clstCentres_obj.clstC_flat,
            clstCentres_obj.numClst,
            clstCentres_obj.numDims, 8, 1024);
    
    std::cout<<"SFqueryfeats::main: Constructing NN search object - DONE ("<< timing::toc(t0) << " ms)\n";
    
    
    // embedder
    
    uint32_t const hammEmbBits= 64;
    uint32_t const vocSize= 200000;
    std::string const trainFilesPrefix= util::expandUser("~/Relja/Data/SF_landmarks/hamm/train/train_sf_hesrootsift3up_" );
    std::string const trainHammFn= trainFilesPrefix + util::uintToShortStr(vocSize) + "_hamm" + boost::lexical_cast<std::string>(hammEmbBits) + ".v2bin";
    
    hammingEmbedderFactory embFactory(trainHammFn, hammEmbBits);
    
    
    // create retrievers
    
    tfidfV2 tfidfObj(
        NULL, NULL, wghtFn);
    
    hamming hammingObj(
            tfidfObj,
            NULL,
            embFactory,
            NULL,
            &featGetter_obj, nn, &clstCentres_obj);
    
    // extract
    
    for (uint32_t queryID= 0; queryID<803; ++queryID){
        std::cout<< queryID <<"\n";
        std::string queryImage= (boost::format("/home/relja/Relja/Databases/SF_landmarks/BuildingQueryImagesCartoIDCorrected-Upright/%04d.jpg") % queryID).str();
        std::string outData= (boost::format("/home/relja/Relja/Data/SF_landmarks/hamm/query_hesrootsift3up_200k/%04d.v2bin") % queryID).str();
        query queryObj(0, false, outData );
        hammingObj.externalQuery_computeData( queryImage, queryObj );
    }
}
