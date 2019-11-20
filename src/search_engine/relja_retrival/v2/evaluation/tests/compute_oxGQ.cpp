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
    
    std::string clstFn= "~/Relja/Data/tmp/indexing_v2/ox5k/clst_paris_hesrootsift3_100k.e3bin";
    std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/train_paris_hesrootsift3_100k_hamm64.v2bin");
    
    // feature getter
    featGetter_standard featGetter_obj( "hesaff-rootsift-scale3" );
    
    // clusters
    std::cout<<"oxGQ::main: Loading cluster centres\n";
    double t0= timing::tic();
    clstCentres clstCentres_obj( util::expandUser(clstFn).c_str(), true );
    std::cout<<"oxGQ::main: Loading cluster centres - DONE ("<< timing::toc(t0) <<" ms)\n";
    
    std::cout<<"oxGQ::main: Constructing NN search object\n";
    t0= timing::tic();
    
    fastann::nn_obj<float> *nn= fastann::nn_obj_build_kdtree(
            clstCentres_obj.clstC_flat,
            clstCentres_obj.numClst,
            clstCentres_obj.numDims, 8, 1024);
    
    std::cout<<"oxGQ::main: Constructing NN search object - DONE ("<< timing::toc(t0) << " ms)\n";
    
    
    // embedder
    
    uint32_t const hammEmbBits= 64;
    uint32_t const vocSize= 100000;
    std::string const trainFilesPrefix= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/train_paris_hesrootsift3_" );
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
    
    for (uint32_t queryID= 0; queryID<11; ++queryID){
        std::cout<< queryID <<"\n";
        for (uint32_t imageID= 0; imageID<8; ++imageID){
            std::string queryImage= (boost::format("/home/relja/Relja/Databases/MultiQuery/%02d/%02d.jpg") % queryID % imageID).str();
            std::string outData= (boost::format("/home/relja/Relja/Data/Temp/MultiQuery/hamm_ox5k_HA/%02d/%02d.v2bin") % queryID % imageID).str();
            query queryObj(0, false, outData );
            hammingObj.externalQuery_computeData( queryImage, queryObj );
        }
    }
}
