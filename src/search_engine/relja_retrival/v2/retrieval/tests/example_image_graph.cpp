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
#include "hamming.h"
#include "hamming_embedder.h"
#include "mpi_queue.h"
#include "image_graph.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "spatial_verif_v2.h"
#include "tfidf_v2.h"
#include "util.h"



void printUsageAndExit(uint32_t rank){
    if (rank!=0) exit(0);
    std::cout<<"Usage: example_image_graph choice\n";
    std::cout<<"Where choice takes one of following values:\n";
    std::cout<<"e : Print some nodes and their edges\n";
    std::cout<<"s : Compute image graph by querying sequentially\n";
    std::cout<<"p : Compute image graph in parallel\n";
    exit(1);
}



int main(int argc, char* argv[]){
    
    // MPI initialization
    MPI_INIT_ENV
    MPI_GLOBAL_ALL
    
    if (argc!=2)
        printUsageAndExit(rank);
    
    char choice= argv[1][0];
    if (!(choice=='e' || choice=='s' || choice=='p') ||
        ((choice=='e' || choice=='s') && numProc>1) )
        printUsageAndExit(rank);
    
    // file name
    std::string imageGraphFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/image_graph/ox5k_hesrootsift3_100k_hamm64.v2bin");
    
    // create the image graph
    
    if (choice=='s' || choice=='p') {
        
        // file names
        
        std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/iidx_ox5k_hesrootsift3_dParis_100k.v2bin");
        std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/fidx_ox5k_hesrootsift3_dParis_100k.v2bin");
        std::string wghtFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/wght_ox5k_hesrootsift3_dParis_100k.v2bin");
        std::string hammFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/train_paris_hesrootsift3_100k_hamm64.v2bin");
        
        // load fidx
        
        protoDbFile dbFidx_file(fidxFn);
        protoDbInRam dbFidx(dbFidx_file, rank==0);
        protoIndex fidx(dbFidx, false);
        
        // load iidx
        
        protoDbFile dbIidx_file(iidxFn);
        protoDbInRam dbIidx(dbIidx_file, rank==0);
        protoIndex iidx(dbIidx, false);
        
        // create retriever
        
        tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);
        hammingEmbedderFactory embFactory(hammFn, 64);
        hamming hammingObj(tfidfObj, &iidx, embFactory, &fidx);
        spatialVerifV2 spatVerifHamm(hammingObj, &iidx, &fidx, true);
        
        imageGraph imGraph;
        if (choice=='s')
            // compute image graph sequential querying
            imGraph.computeSingle(
                imageGraphFn, fidx.numIDs(), spatVerifHamm, 100, 10 );
        else {
            // compute image graph in parallel
            imGraph.computeParallel(
                imageGraphFn, fidx.numIDs(), spatVerifHamm, 100, 10 );
        }
    }
    
    // load the image graph and print some edges
    
    if (choice=='e') {
        
        std::string dsetFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/dset_ox5k.v2bin");
        
        datasetV2 dset(dsetFn);
        imageGraph imGraph( imageGraphFn );
        
        uint32_t maxNode= imGraph.graph_.rbegin()->first;
        
        std::ofstream html( util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/image_graph/ox5k_hesrootsift3_100k_hamm64.html").c_str() );
        html<<"<html><head><title>Image Graph example</title></head><body>\n";
        
        for (uint32_t docID= 0; docID<=maxNode; docID+= maxNode/25){
            
            std::cout<< "\tnode: "<<docID<<" "<<dset.getFn(docID)<<"\n";
            html<<"<h2>Node:</h2><br>\n";
            html<<"<img height=\"200\" src=\"" <<dset.getFn(docID)<< "\"><br><h2>Results:</h2><br>\n";
            
            if (imGraph.graph_.count(docID)==0){
                
                std::cout<<"no neighbours\n";
                html<<"None\n";
                
            } else {
                
                std::vector<indScorePair> &neighs= imGraph.graph_[docID];
                for (uint32_t iNeigh= 0; iNeigh<neighs.size() && iNeigh<10; ++iNeigh){
                    uint32_t docIDres= neighs[iNeigh].first;
                    double score= neighs[iNeigh].second;
                    std::cout<< "docID: "<<docIDres<< " score: "<<score<<" "<<dset.getFn(docIDres)<<"\n";
                    html<<"<img height=\"200\" src=\"" <<dset.getFn(docIDres)<< "\">\n";
                }
            }
            
            std::cout<<"\n";
            html<<"<br><hr><br>\n";
        }
        
        html<<"</body></html>\n";
        html.close();
    }
    
    // required clean up for protocol buffers
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
