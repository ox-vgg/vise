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
    std::string iidxFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/iidx_gmy_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
    std::string fidxFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/fidx_gmn_oxc1_5k_hesaff_sift_hell_1000000_43.v2bin";
    
    protoDbFile dbFidx_file(fidxFn);
    protoDbInRam dbFidx(dbFidx_file);
    protoIndex fidx(dbFidx, false);
    
    std::vector<rr::indexEntry> entries;
    
    double t0= timing::tic();
    
    if (true){
        
        protoDbFile dbIidx_file(iidxFn);
        protoDbInRam dbIidx(dbIidx_file);
        protoIndex iidx(dbIidx, false);
        
        uint32_t numWords= iidx.numIDs();
        
        protoDbFileBuilder dbIidxBuilder(iidxFn_new, "testing");
        indexBuilder iidxBuilder(dbIidxBuilder, true, true, true);
        
        std::vector<uint32_t> ID;
        std::vector< std::pair<uint32_t,uint32_t> > entryInd;
        rr::indexEntry entryIidx;
        
        for (uint32_t wordID= 0; wordID<numWords; ++wordID){
            if (wordID%5000==0)
                std::cout<< "stupidCreateIidx: creating "<< wordID <<" / "<< numWords <<" ("<< timing::toc(t0)/wordID <<")\n";
            
            uint32_t N= iidx.getEntries(wordID, entries);
            
            std::vector<uint32_t> lookInInds;
            lookInInds.reserve(N);
            {
                ASSERT(entries.size()==1);
                rr::indexEntry const &entry= entries[0];
                uint32_t prevID;
                for (int iE= 0; iE<entry.id_size(); ++iE)
                    if (iE==0 || entry.id(iE)!=prevID){
                        prevID= entry.id(iE);
                        lookInInds.push_back(prevID);
                    }
            }
            
            uint32_t Ninv= fidx.getInverseEntryInds(wordID, ID, entryInd, &lookInInds);
            
            std::string qel_scale(Ninv, '\0');
            std::string qel_ratio(Ninv, '\0');
            std::string qel_angle(Ninv, '\0');
            uint32_t iNew= 0;
            
            for (uint32_t i= 0; i<ID.size(); ++i){
                
                fidx.getEntries(ID[i], entries);
                ASSERT(entries.size()==1);
                rr::indexEntry const &entry= entries[0];
                
                std::string const &old_scale= entry.qel_scale();
                std::string const &old_ratio= entry.qel_ratio();
                std::string const &old_angle= entry.qel_angle();
                ASSERT(old_scale.length()==old_ratio.length());
                ASSERT(old_scale.length()==static_cast<uint32_t>(entry.qx_size()));
                ASSERT(old_scale.length()==static_cast<uint32_t>(entry.qy_size()));
                
                for (uint32_t iElem= entryInd[i].first; iElem < entryInd[i].second; ++iElem){
                    ASSERT(iNew<Ninv);
                    entryIidx.add_id(ID[i]);
                    entryIidx.add_qx(entry.qx(iElem));
                    entryIidx.add_qy(entry.qy(iElem));
                    qel_scale[iNew]= old_scale[iElem];
                    qel_ratio[iNew]= old_ratio[iElem];
                    qel_angle[iNew]= old_angle[iElem];
                    ++iNew;
                }
                
            }
            ASSERT(iNew==Ninv);
            
            entryIidx.set_qel_scale(qel_scale);
            entryIidx.set_qel_ratio(qel_ratio);
            entryIidx.set_qel_angle(qel_angle);
            
            iidxBuilder.addEntry(wordID, entryIidx);
            
            entryIidx.Clear();
        }
        
    }
    
    if (true){
        
        uint32_t numDocs= fidx.numIDs();
        protoDbFileBuilder dbFidxBuilder(fidxFn_new, "testing");
        indexBuilder fidxBuilder(dbFidxBuilder, true);
        
        rr::indexEntry entryFidx;
        
        for (uint32_t docID= 0; docID<numDocs; ++docID){
            fidx.getEntries(docID, entries);
            ASSERT(entries.size()==1);
            rr::indexEntry const &entry= entries[0];
            // remove duplicate docIDs
            for (int i= 0; i < entry.id_size(); ++i){
                ASSERT(i==0 || entry.id(i-1)<=entry.id(i));
                if (i==0 || entry.id(i-1)!=entry.id(i))
                    entryFidx.add_id(entry.id(i));
            }
            fidxBuilder.addEntry(docID, entryFidx);
            entryFidx.Clear();
        }
        
    }
    
    std::cout<< timing::toc(t0) <<"\n";
    
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
