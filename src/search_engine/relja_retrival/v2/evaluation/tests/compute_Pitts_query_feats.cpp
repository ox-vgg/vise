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
#include <fstream>
#include <string.h>

#include <boost/format.hpp>

#include <fastann/fastann.hpp>

#include "feat_standard.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "tfidf_v2.h"
#include "util.h"



int main(){
    
    std::string clstFn= "~/Relja/Data/Pittsburgh/clst_pitts_hesrootsift3up_200k.e3bin";
    std::string wghtFn= util::expandUser("~/Relja/Data/Pittsburgh/wght_pitts_hesrootsift3up_200k.v2bin");
    
    // feature getter
    featGetter_standard featGetter_obj( "hesaff-rootsift-scale3-up" );
    
    // clusters
    std::cout<<"PittsQueryfeats::main: Loading cluster centres\n";
    double t0= timing::tic();
    clstCentres clstCentres_obj( util::expandUser(clstFn).c_str(), true );
    std::cout<<"PittsQueryfeats::main: Loading cluster centres - DONE ("<< timing::toc(t0) <<" ms)\n";
    
    std::cout<<"PittsQueryfeats::main: Constructing NN search object\n";
    t0= timing::tic();
    
    fastann::nn_obj<float> *nn= fastann::nn_obj_build_kdtree(
            clstCentres_obj.clstC_flat,
            clstCentres_obj.numClst,
            clstCentres_obj.numDims, 8, 1024);
    
    std::cout<<"PittsQueryfeats::main: Constructing NN search object - DONE ("<< timing::toc(t0) << " ms)\n";
    
    
    // embedder
    
    uint32_t const hammEmbBits= 64;
    uint32_t const vocSize= 200000;
    std::string const trainFilesPrefix= util::expandUser("~/Relja/Data/Pittsburgh/train/train_pitts_hesrootsift3up_" );
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
    
    std::ifstream fin("/home/relja/Relja/Databases/Pittsburgh/groundtruth/querylist.txt");
    std::string fn;
    uint32_t queryID= 0;
    
    while (fin>>fn){
        if (queryID % 100==0)
            std::cout<< "\n\t\t\t"<< queryID<<" "<< fn <<"\n\n";
        
        std::string queryImage= (boost::format("/home/relja/Relja/Databases/Pittsburgh/queries_real/%s") % fn).str();
        std::string outData= (boost::format("/home/relja/Relja/Data/Pittsburgh/query_hesrootsift3up_200k/%05d.v2bin") % queryID).str();
        query queryObj(0, false, outData );
        hammingObj.externalQuery_computeData( queryImage, queryObj );
        
        ++queryID;
    }
    
}
