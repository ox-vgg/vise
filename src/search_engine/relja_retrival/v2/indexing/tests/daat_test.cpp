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

#include "daat.h"
#include "index_entry.pb.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"



void check(std::vector<uint32_t> const &ID_inv, std::vector< std::pair<uint32_t,uint32_t> > const &entryInd_inv, std::vector<uint32_t> const *nonEmptyEntryInd, std::vector< std::pair<uint32_t,uint32_t> > const *entryInd, std::vector<uint32_t> const &wordIDs){
    
    ASSERT(entryInd_inv.size()==nonEmptyEntryInd->size());
    
    for (uint32_t i= 0; i<nonEmptyEntryInd->size(); ++i){
        // NOTE: this exploits the fact that input wordIDs are unique, otherwise a bit more complex (see DAAT)
        uint32_t ind= nonEmptyEntryInd->at(i);
        uint32_t ind2;
        for (ind2= 0; ind2<entryInd_inv.size() && wordIDs[ind]>ID_inv[ind2]; ++ind2);
        ASSERT( ind2<entryInd_inv.size() && wordIDs[ind]==ID_inv[ind2] );
        ASSERT( entryInd->at(ind).first == entryInd_inv[ind2].first );
        ASSERT( entryInd->at(ind).second == entryInd_inv[ind2].second );
    }
}



int main(){
   
    std::string iidxFn= "/home/relja/Relja/Data/tmp/indexing_v2/iidx_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
    
    protoDbFile dbIidx_file(iidxFn);
    #if 0
    protoDbInRam dbIidx(dbIidx_file);
    protoIndex iidx(dbIidx, false);
    #else
    protoIndex iidx(dbIidx_file, false);
    #endif
    
    std::vector<uint32_t> wordIDs;
    for (uint32_t i= 0; i<4000; ++i)
        wordIDs.push_back(i);
    
    std::vector< std::pair<uint32_t,uint32_t> > entryInd_inv;
    std::vector<uint32_t> ID_inv;
    std::vector< std::pair<uint32_t,uint32_t> > const *entryInd;
    std::vector<uint32_t> const *nonEmptyEntryInd;
    std::vector< std::vector<rr::indexEntry> > entries(wordIDs.size());
    for (uint32_t i= 0; i<wordIDs.size(); ++i){
        iidx.getEntries(wordIDs[i], entries[i]);
        ASSERT(entries[i].size()==1); // for this test..
    }
    
    double t0= timing::tic();
    double iterTime= 0;
    
    if (true) {
        // test all docs
        
        rr::indexEntry queryRep;
        queryRep.mutable_id()->Reserve(wordIDs.size());
        for (uint32_t i= 0; i<wordIDs.size(); ++i)
            queryRep.add_id(wordIDs[i]);
        uniqEntries ue;
        iidx.getUniqEntries(queryRep, ue);
        precompUEIterator ueIter(ue);
        
        daat daatIter(&ueIter);
        
        uint32_t docID= 0;
        
        while (!daatIter.isEnd()){
            
            if (docID%500==0)
                std::cout<< "daatTest: testing "<< docID <<" ("<< iterTime/docID <<")\n";
            
            double t0iter= timing::tic();
            daatIter.advance();
            daatIter.getMatches(entryInd, nonEmptyEntryInd);
            iterTime+= timing::toc(t0iter);
            uint32_t currDocID= entries[nonEmptyEntryInd->at(0)][0].id(entryInd->at(nonEmptyEntryInd->at(0)).first);
            
            for (; docID<currDocID; ++docID){
                iidx.getInverseEntryInds(docID, ID_inv, entryInd_inv, &wordIDs);
                ASSERT(entryInd_inv.empty());
            }
            
            iidx.getInverseEntryInds(docID, ID_inv, entryInd_inv, &wordIDs);
            check(ID_inv, entryInd_inv, nonEmptyEntryInd, entryInd, wordIDs);
            
            ++docID;
        }
        
        for (; docID<5062; ++docID){
            iidx.getInverseEntryInds(docID, ID_inv, entryInd_inv, &wordIDs);
            ASSERT(entryInd_inv.empty());
        }
        
    }
    
    if (true) {
        
        // test limited docIDs
        
        std::vector<uint32_t> docIDs;
        for (uint32_t i= 0; i<5000; i+=10)
            docIDs.push_back(i);
        
        rr::indexEntry queryRep;
        queryRep.mutable_id()->Reserve(wordIDs.size());
        for (uint32_t i= 0; i<wordIDs.size(); ++i)
            queryRep.add_id(wordIDs[i]);
        uniqEntries ue;
        iidx.getUniqEntries(queryRep, ue);
        precompUEIterator ueIter(ue);
        
        daat daatIter(&ueIter, &docIDs);
        
        uint32_t docIDInd= 0;
        
        while (!daatIter.isEnd() && docIDInd<docIDs.size()){
            
            if (docIDInd%100==0)
                std::cout<< "daatTest: testing "<< docIDInd <<" ("<< iterTime/docIDInd <<")\n";
            
            double t0iter= timing::tic();
            daatIter.advance();
            daatIter.getMatches(entryInd, nonEmptyEntryInd);
            iterTime+= timing::toc(t0iter);
            uint32_t currDocID= entries[nonEmptyEntryInd->at(0)][0].id(entryInd->at(nonEmptyEntryInd->at(0)).first);
            
            for (; docIDs[docIDInd]<currDocID; ++docIDInd){
                iidx.getInverseEntryInds(docIDs[docIDInd], ID_inv, entryInd_inv, &wordIDs);
                ASSERT(entryInd_inv.empty());
            }
            
            iidx.getInverseEntryInds(docIDs[docIDInd], ID_inv, entryInd_inv, &wordIDs);
            check(ID_inv, entryInd_inv, nonEmptyEntryInd, entryInd, wordIDs);
            
            ++docIDInd;
        }
        
        for (; docIDInd<docIDs.size(); ++docIDInd){
            iidx.getInverseEntryInds(docIDs[docIDInd], ID_inv, entryInd_inv, &wordIDs);
            ASSERT(entryInd_inv.empty());
        }
        
    }
    
    std::cout<< iterTime <<" "<< timing::toc(t0) <<"\n";
    
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
