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

#include "dataset_v2.h"
#include "embedder.h"
#include "hamming_embedder.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"



void reduceIidx(std::string iidxFn, std::string iidxFn_new, uint32_t step, embedderFactory const &embFactory){
    
    protoDbFile dbIidx(iidxFn);
    protoIndex iidx(dbIidx, false);
    protoDbFileBuilder dbIidxBuilder(iidxFn_new, "reduced");
    indexBuilder iidxBuilder(dbIidxBuilder, true, false, false); // XY/ellipse we will copy quantized
    
    uint32_t const numIDs= iidx.numIDs();
    
    std::vector<rr::indexEntry> entries;
    rr::indexEntry entryNew;
    
    for (uint32_t ID= 0; ID<numIDs; ++ID){
        if (ID%5000==0) std::cout<<ID<<" / "<<numIDs<<"\n";
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
                if (entry.id(i)%step==0){
                    entryNew.add_id(entry.id(i)/step);
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



void reduceFidx(std::string fidxFn, std::string fidxFn_new, uint32_t step){
    
    protoDbFile dbFidx(fidxFn);
    protoDbFileBuilder dbFidxBuilder(fidxFn_new, "reduced");
    uint32_t const numDocs= dbFidx.numIDs();
    uint32_t docIDnew= 0;
    
    std::vector<std::string> data;
    
    for (uint32_t docID= 0; docID<numDocs; docID+=step, ++docIDnew){
        dbFidx.getData(docID, data);
        for (uint32_t i= 0; i<data.size(); ++i)
            dbFidxBuilder.addData(docIDnew, data[i]);
    }
    
}



void reduceDset(std::string dsetFn, std::string dsetFn_new, uint32_t step){
    datasetV2 dset(dsetFn);
    datasetBuilder dsetBuilder(dsetFn_new);
    
    uint32_t const numDocs= dset.getNumDoc();
    uint32_t docIDnew= 0;
    std::pair<uint32_t, uint32_t> wh;
    std::string fn;
    
    for (uint32_t docID= 0; docID<numDocs; docID+=step, ++docIDnew){
        fn= dset.getFn(docID);
        wh= dset.getWidthHeight(docID);
        dsetBuilder.add(fn, wh.first, wh.second);
    }
}



int main(){
    
    uint32_t const step= 2;
    
    if (true){
        std::string dsetFn= "/home/relja/Relja/Data/tmp/indexing_v2/ox5k/hamm/dset_ox5k.v2bin";
        std::string dsetFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/ox5k/hamm/dset_ox5khalf.v2bin";
        
        double t0= timing::tic();
        reduceDset(dsetFn, dsetFn_new, step);
        std::cout<< "dset: "<<timing::toc(t0) <<"\n";
    }
    
    if (true){
        std::string iidxFn= "/home/relja/Relja/Data/tmp/indexing_v2/ox5k/hamm/iidx_ox5k_hesrootsift3_dParis_100k.v2bin";
        std::string iidxFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/ox5k/hamm/iidx_ox5khalf_hesrootsift3_dParis_100k.v2bin";
        std::string hammFn= "/home/relja/Relja/Data/tmp/indexing_v2/ox5k/hamm/train_paris_hesrootsift3_100k_hamm64.v2bin";
        hammingEmbedderFactory embFactory(hammFn, 64);
        
        double t0= timing::tic();
        reduceIidx(iidxFn, iidxFn_new, step, embFactory);
        std::cout<< "iidx: "<<timing::toc(t0) <<"\n";
    }
    
    if (true){
        std::string fidxFn= "/home/relja/Relja/Data/tmp/indexing_v2/ox5k/hamm/fidx_ox5k_hesrootsift3_dParis_100k.v2bin";
        std::string fidxFn_new= "/home/relja/Relja/Data/tmp/indexing_v2/ox5k/hamm/fidx_ox5khalf_hesrootsift3_dParis_100k.v2bin";
        
        double t0= timing::tic();
        reduceFidx(fidxFn, fidxFn_new, step);
        std::cout<< "fidx: "<<timing::toc(t0) <<"\n";
    }
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
