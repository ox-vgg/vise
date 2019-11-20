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

#include <algorithm>
#include <stdint.h>
#include <string>
#include <vector>

#include "index_entry.pb.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"



inline double sqr(double x){ return x*x; }



int main(int argc, char* argv[]){
    
    std::string iidxFn= "/home/relja/Relja/Data/tmp/indexing_v2/ox5k/hamm/iidx_ox5k_hesrootsift3_dParis_100k.v2bin";
    if (argc>1)
        iidxFn= argv[1];
    
    protoDbFile dbIidx_file(iidxFn);
    protoIndex iidx(dbIidx_file, false);
    
    if (true) {
        double imbalanceFactor= 0.0;
        uint32_t N;
        uint64_t totalFeats= 0;
        uint32_t const numWords= iidx.numIDs();
        uint32_t const printStep= std::max(static_cast<uint32_t>(1),numWords/10);
        double time= timing::tic();
        std::vector<uint32_t> Ns; Ns.reserve(numWords);
        for (uint32_t wordID= 0; wordID < numWords; ++wordID){
            if (wordID % printStep==0)
                std::cout<<"imbalanceFactor computing ID= "<<wordID<<" / "<<numWords<<" "<<timing::toc(time)<<" ms\n";
            N= iidx.getNumWithID(wordID);
            Ns.push_back(N);
            imbalanceFactor+= sqr( static_cast<double>(N) );
            totalFeats+= N;
        }
        imbalanceFactor*= numWords;
        imbalanceFactor/=totalFeats;
        imbalanceFactor/=totalFeats;
        
        std::cout<< "imbalanceFactor= "<<imbalanceFactor<<"\n";
        std::sort(Ns.begin(), Ns.end());
        uint32_t const median= Ns[Ns.size()/2];
        for (uint32_t wordID= 0; wordID<numWords + printStep -1; wordID+= printStep){
            uint32_t wordIDthis= std::min(wordID, numWords-1);
            std::cout<< static_cast<double>(Ns[wordIDthis])/median <<" ";
        }
        std::cout<<"\n";
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
