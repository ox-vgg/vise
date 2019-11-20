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

#include "index_entry.pb.h"
#include "proto_db_file.h"
#include "proto_index.h"



int main(){
    
    std::string idxFn1= "/home/relja/Relja/Data/tmp/indexing_v2/d/80.v2bin";
    std::string idxFn2= "/home/relja/Relja/Data/tmp/indexing_v2/d/60.v2bin";
//     std::string idxFn2= "/home/relja/Relja/Data/tmp/indexing_v2/d/40.v2bin";
    
    protoDbFile dbIdx_file1(idxFn1);
    protoIndex idx1(dbIdx_file1, false);
    
    protoDbFile dbIdx_file2(idxFn2);
    protoIndex idx2(dbIdx_file2, false);
    
    std::cout<<"numIDs: "<<idx1.numIDs()<<"\n";
    ASSERT( idx1.numIDs() == idx2.numIDs() );
    
    std::vector<rr::indexEntry> entries1, entries2;
    
    uint32_t total1= 0, total2= 0;
    
    for (uint32_t ID= 0; ID<idx1.numIDs(); ++ID){
        
        if (ID%100000==0)
            std::cout<<"checking ID= "<<ID<<"\n";
        
        idx1.getEntries(ID, entries1);
        idx2.getEntries(ID, entries2);
        ASSERT(entries1.size() == entries2.size());
        
        for (uint32_t iEntry= 0; iEntry<entries1.size(); ++iEntry){
            rr::indexEntry const &entry1= entries1[iEntry];
            rr::indexEntry const &entry2= entries2[iEntry];
            total1+= entry1.id_size();
            total2+= entry2.id_size();
            
            if (entry1.id_size()!=entry2.id_size()){
                std::cout<<"ID= "<<ID<< ", num: "<<entry1.id_size() <<" "<< entry2.id_size() <<"\n";
                int sz= std::min( entry1.id_size(), entry2.id_size() );
                /*
                for (int i= 0; i<sz; ++i){
                    std::cout<<entry1.id(i)<<" "<<entry2.id(i)<<"\n";
                }
                */
                int i= 0;
                for (i= 0; i<sz; ++i){
                    if (entry1.id(i)!=entry2.id(i)){
                        std::cout<<"diff ("<<i<<") "<<entry1.id(i)<<" "<<entry2.id(i)<<"\n";
                        break;
                    }
                }
                if (i==sz){
                    if (sz==entry1.id_size())
                        std::cout<< "docID "<<entry2.id(sz)<<" missing from 1\n";
                    else
                        std::cout<< "docID "<<entry1.id(sz)<<" missing from 2\n";
                }
                
//                 ASSERT(0);
                std::cout<<"\n";
            }
        }
        
        
    }
    
    std::cout<<"\n";
    std::cout<< total1 <<" "<< total2<<"\n";
    
    return 0;
}
