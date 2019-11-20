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

#include "dataset_v2.h"
#include "embedder.h"
#include "hamming_embedder.h"
#include "index_entry_util.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"
#include "util.h"



void reduceIidxScale(std::string iidxFn, std::string iidxFn_new, float keepPercent, embedderFactory const &embFactory){
    
    protoDbFile dbIidx(iidxFn);
    protoIndex iidx(dbIidx, false);
    protoDbFileBuilder dbIidxBuilder(iidxFn_new, "reduced");
    indexBuilder iidxBuilder(dbIidxBuilder, true, false, false); // XY/ellipse we will copy quantizedscale
    
    uint32_t const numIDs= iidx.numIDs();
    
    std::vector<rr::indexEntry> entries;
    
    // Sample the scales to get the threshold
    
    uint32_t const maxNum= 10000000;
    std::vector<float> invScales;
    float invScale;
    invScales.reserve(maxNum);
    
    timing::progressPrint progSample(maxNum, "reduceIidxScale sample");
    
    for (uint32_t ID= 0; ID<numIDs && invScales.size()<maxNum; ++ID){
        entries.clear();
        iidx.getEntries(ID, entries);
        
        for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
            rr::indexEntry &entry= entries[iEntry];
            indexEntryUtil::unquantEllipse(entry);
            
            int n= entry.id_size();
            
            ASSERT( n == entry.a_size() );
            ASSERT( n == entry.b_size() );
            ASSERT( n == entry.c_size() );
            
            for (int i= 0; i<entry.id_size(); ++i, progSample.inc()){
                invScale= entry.a(i)*entry.c(i)-entry.b(i)*entry.b(i);
                invScales.push_back(invScale);
            }
        }
    }
    
    uint32_t ind= std::min(static_cast<size_t>(keepPercent * invScales.size()), invScales.size()-1);
    std::nth_element(invScales.begin(),
                     invScales.begin() + ind,
                     invScales.end());
    
    float const invScaleThr=  invScales[ind] - 1e-9;
    
    
    
    // Keep only featres with larger scales (i.e. remove invScale>scaleThr)
    
    rr::indexEntry entryNew;
    
    timing::progressPrint progReduce(numIDs, "reduceIidxScale reduce");
    
    for (uint32_t ID= 0; ID<numIDs; ++ID, progReduce.inc()){
        entries.clear();
        iidx.getEntries(ID, entries);
        
        for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
            rr::indexEntry &entry= entries[iEntry];
            embedder *emb= embFactory.getEmbedder();
            if ( emb->doesSomething() ){
                ASSERT(entry.has_data());
                emb->setDataCopy(entry.data());
            }
            
            int n= entry.id_size();
            
            rr::indexEntry entryEl= entry; // do copy
            indexEntryUtil::unquantEllipse(entryEl);
            
            ASSERT( n == entry.qx_size() );
            ASSERT( n == entry.qy_size() );
            ASSERT( n == static_cast<int>(entry.qel_scale().length()) );
            ASSERT( n == static_cast<int>(entry.qel_ratio().length()) );
            ASSERT( n == static_cast<int>(entry.qel_angle().length()) );
            ASSERT( entry.a_size()==0 );
            ASSERT( entry.b_size()==0 );
            ASSERT( entry.c_size()==0 );
            if ( emb->doesSomething() )
                ASSERT( n == static_cast<int>(emb->getNum()) );
            
            entryNew.mutable_id()->Reserve(n);
            entryNew.mutable_qx()->Reserve(n);
            entryNew.mutable_qy()->Reserve(n);
            entryNew.mutable_qel_scale()->reserve(n);
            entryNew.mutable_qel_ratio()->reserve(n);
            entryNew.mutable_qel_angle()->reserve(n);
            embedder *embNew= embFactory.getEmbedder();
            embNew->reserve(n);
            
            std::string const &scaleStr= entry.qel_scale();
            std::string const &ratioStr= entry.qel_ratio();
            std::string const &angleStr= entry.qel_angle();
            unsigned char const *scale= reinterpret_cast<unsigned char const*>(scaleStr.c_str());
            unsigned char const *ratio= reinterpret_cast<unsigned char const*>(ratioStr.c_str());
            unsigned char const *angle= reinterpret_cast<unsigned char const*>(angleStr.c_str());
            
            for (int i= 0; i<entry.id_size(); ++i, ++scale, ++ratio, ++angle){
                invScale= entryEl.a(i)*entryEl.c(i)-entryEl.b(i)*entryEl.b(i);
                if (invScale < invScaleThr){
                    entryNew.add_id(entry.id(i));
                    entryNew.add_qx(entry.qx(i));
                    entryNew.add_qy(entry.qy(i));
                    entryNew.mutable_qel_scale()->append( &(entry.qel_scale()[i]), 1 );
                    entryNew.mutable_qel_ratio()->append( &(entry.qel_ratio()[i]), 1 );
                    entryNew.mutable_qel_angle()->append( &(entry.qel_angle()[i]), 1 );
                    if (emb->doesSomething())
                        embNew->copyFrom(*emb, i);
                }
            }
            
            entryNew.set_data(embNew->getEncoding());
            
            delete emb;
            delete embNew;
            
            if (entryNew.id_size()>0)
                iidxBuilder.addEntry(ID, entryNew);
            
            entryNew.Clear();
        }
    }
    
}



void reduceFidx(std::string iidxFn_new, std::string fidxFn, std::string fidxFn_new){
    
    protoDbFile dbFidx(fidxFn);
    protoIndex fidx(dbFidx, false);
    
    protoDbFileBuilder dbFidxBuilder(fidxFn_new, "reduced");
    indexBuilder fidxBuilder(dbFidxBuilder, true, false, false); // not storing xy/ellipse anyway
    
    uint32_t const numDocs= fidx.numIDs();
    
    std::vector<rr::indexEntry> entries;
    std::vector<uint32_t> wordIDsNew;
    
    timing::progressPrint progReduce(numDocs, "reduceFidxScale reduce");
    
    #if 1
    // stupid, in memory, but fast
    
    
    protoDbFile dbIidx(iidxFn_new);
    protoIndex iidx(dbIidx, false);
    uint32_t const numWords= iidx.numIDs();
    
    std::vector<rr::indexEntry> newFidx(numDocs);
    timing::progressPrint progLoad(numWords, "reduceFidxScale make fidx");
    
    for (uint32_t wordID= 0; wordID<numWords; ++wordID, progLoad.inc()){
        iidx.getEntries(wordID, entries);
        uint32_t prevDocID= 0, docID;
        
        for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
            rr::indexEntry &entry= entries[iEntry];
            
            for (int i= 0; i<entry.id_size(); ++i){
                docID= entry.id(i);
                if ((iEntry==0 && i==0) || docID!=prevDocID){
                    prevDocID= docID;
                    newFidx[ docID ].add_id(wordID);
                }
            }
        }
    }
    
    for (uint32_t docID= 0; docID<numDocs; ++docID, progReduce.inc()){
        if (newFidx[docID].id_size()>0)
            fidxBuilder.addEntry(docID, newFidx[docID]);
    }
    
    #else
    // slow but low memory
    
    protoDbFile dbIidx(iidxFn_new);
    protoIndex iidx(dbIidx, false);
    
    std::vector< std::pair<uint32_t,uint32_t> > entryInd;
    rr::indexEntry entryFidx;
    
    for (uint32_t docID= 0; docID<numDocs; ++docID, progReduce.inc()){
        
        // old wordIDs for this docID
        
        uint32_t N= fidx.getEntries(docID, entries);
        
        std::vector<uint32_t> lookInInds;
        lookInInds.reserve(N);
        {
            uint32_t prevID= 0;
            for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
                rr::indexEntry const &entry= entries[iEntry];
                for (int i= 0; i<entry.id_size(); ++i)
                    if ( (iEntry==0 && i==0) || entry.id(i)!=prevID){
                        prevID= entry.id(i);
                        lookInInds.push_back(prevID);
                    }
            }
        }
        
        // get new wordIDs for this docID
        wordIDsNew.clear();
        iidx.getInverseEntryInds(docID, wordIDsNew, entryInd, &lookInInds);
        std::sort(wordIDsNew.begin(), wordIDsNew.end());
        
        // check wordIDsNew is subset of lookInInds
        uint32_t j= 0;
        for (uint32_t i= 0; i<wordIDsNew.size(); ++i){
            for (; j<lookInInds.size() && lookInInds[j]<wordIDsNew[i]; ++j);
            ASSERT(j<lookInInds.size() && lookInInds[j]==wordIDsNew[i]);
        }
        
        // add to index
        
        for (uint32_t i= 0; i<wordIDsNew.size(); ++i){
            if (i==0 || (wordIDsNew[i]!=wordIDsNew[i-1]))
                entryFidx.add_id(wordIDsNew[i]);
        }
        
        fidxBuilder.addEntry(docID, entryFidx);
        
        entryFidx.Clear();
    }
    #endif
    
}



int main(){
    
//     std::string iidxFn_new= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/redscale_iidx_ox5k_hesrootsift3_dParis_100k.v2bin");
    std::string iidxFn_new= util::expandUser("~/Relja/Data/axes/hamm/axes/redscale_iidx_axes_hesrootsift3_200k.v2bin");
    
    if (true){
//         std::string iidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/fidx_ox5k_hesrootsift3_dParis_100k.v2bin");
//         std::string hammFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/train_paris_hesrootsift3_100k_hamm64.v2bin");
//         hammingEmbedderFactory embFactory(hammFn, 64);
        std::string iidxFn= util::expandUser("~/Relja/Data/axes/hamm/axes/iidx_axes_hesrootsift3_200k.v2bin");
        std::string hammFn= util::expandUser("~/Relja/Data/axes/hamm/train/train_nisv_hesrootsift3_200k_hamm32.v2bin");
        hammingEmbedderFactory embFactory(hammFn, 32);
        
        double t0= timing::tic();
        reduceIidxScale(iidxFn, iidxFn_new, 0.2, embFactory);
        std::cout<< "iidx: "<<timing::toc(t0) <<"\n";
    }
    
    if (true){
//         std::string fidxFn= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/fidx_ox5k_hesrootsift3_dParis_100k.v2bin");
//         std::string fidxFn_new= util::expandUser("~/Relja/Data/tmp/indexing_v2/ox5k/hamm/redscale_fidx_ox5k_hesrootsift3_dParis_100k.v2bin");
        std::string fidxFn= util::expandUser("~/Relja/Data/axes/hamm/axes/fidx_axes_hesrootsift3_200k.v2bin");
        std::string fidxFn_new= util::expandUser("~/Relja/Data/axes/hamm/axes/redscale_fidx_axes_hesrootsift3_200k.v2bin");
        
        double t0= timing::tic();
        reduceFidx(iidxFn_new, fidxFn, fidxFn_new);
        std::cout<< "fidx: "<<timing::toc(t0) <<"\n";
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
