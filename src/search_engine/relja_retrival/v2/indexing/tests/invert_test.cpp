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
#include <string>
#include <vector>

#include "index_entry.pb.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"



int main(){
   
    std::string iidxFn= "/home/relja/Relja/Data/tmp/indexing_v2/iidx_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
    std::string fidxFn= "/home/relja/Relja/Data/tmp/indexing_v2/fidx_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
    
    protoDbFile dbIidx_file(iidxFn);
    protoDbInRam dbIidx(dbIidx_file);
    protoIndex iidx(dbIidx, false);
    
    protoDbFile dbFidx_file(fidxFn);
    protoDbInRam dbFidx(dbFidx_file);
    protoIndex fidx(dbFidx, false);
    
    uint32_t numDocs= fidx.numIDs();
    
    std::vector<rr::indexEntry> entries;
    std::vector<uint32_t> ID;
    std::vector< std::pair<uint32_t,uint32_t> > entryInd;
    
    double t0= timing::tic();
    double invTime= 0;
    
    for (uint32_t docID= 0; docID<numDocs; ++docID){
        if (docID%500==0)
            std::cout<< "invertTest: testing "<< docID <<" / "<< numDocs <<" ("<< invTime/docID <<")\n";
        
        uint32_t N= fidx.getEntries(docID, entries);
        
        std::vector<uint32_t> lookInInds;
        lookInInds.reserve(N);
        ASSERT(entries.size()==1);
        rr::indexEntry const& entry= entries[0];
        uint32_t prevID;
        for (int iE= 0; iE<entry.id_size(); ++iE)
            if (iE==0 || entry.id(iE)!=prevID){
                prevID= entry.id(iE);
                lookInInds.push_back(prevID);
            }
        
        double t0inv= timing::tic();
//         iidx.getInverseEntryInds(docID, ID, entryInd);
        iidx.getInverseEntryInds(docID, ID, entryInd, &lookInInds);
        invTime+= timing::toc(t0inv);
        
        uint32_t Ninv= 0;
        for (uint32_t i= 0; i<ID.size(); ++i){
            iidx.getEntries(ID[i], entries);
            ASSERT(entries.size()==1); // this test doesn't work for size>1 as offset needs to be used
            rr::indexEntry const& entry= entries[0];
            ASSERT(entry.count_size()==entry.id_size());
            for (uint32_t iElement= entryInd[i].first; iElement<entryInd[i].second; ++iElement)
                Ninv+= entry.count(iElement);
        }
        
        ASSERT(N==Ninv);
    }
    
    std::cout<< timing::toc(t0) <<"\n";
    std::cout<<"Test passed!\n";
    
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
