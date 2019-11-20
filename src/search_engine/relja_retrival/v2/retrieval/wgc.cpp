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

#include "wgc.h"

#include <fstream>

#include <boost/filesystem.hpp>

#include "tfidf_v2.h"
#include "tfidf_data.pb.h"
#include "timing.h"
#include "weighter_v2.h"



wgc::wgc( protoIndex const &iidx, protoIndex const *fidx, std::string wgcFn ) : retrieverFromIter(&iidx, fidx, false, true), iidx_(&iidx) {
    
    if ( wgcFn.length()>0 && boost::filesystem::exists( wgcFn ) ){
        
        tfidfV2::load(wgcFn, idf_, docL2_);
        numDocs_= docL2_.size();
        
    } else {
        
        tfidfV2::computeIdf(*iidx_, idf_, fidx_);
        computeDocL2();
        numDocs_= docL2_.size();
        
        if (wgcFn.length()>0)
            tfidfV2::save(wgcFn, idf_, docL2_);
        
    }
    
}



void
wgc::queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    std::vector<double> scores;
    queryExecute(queryRep, ueIter, scores);
    retriever::sortResults( scores, queryRes, toReturn );
}



void
wgc::queryExecute( rr::indexEntry &queryRep, ueIterator *ueIter, std::vector<double> &scores ) const {
    
    // weight query BoW with idf
    tfidfV2::weightStatic(queryRep, NULL, &idf_);
    
    // query
    weighterV2::queryExecuteWGC(queryRep, ueIter, idf_, docL2_, scores, 128);
    
}



void
wgc::computeDocL2() {
    
    uint32_t numWords= iidx_->numIDs();
    ASSERT(fidx_!=NULL); // if needed, this could be replaced by computing numDocs as max(all ids)
    uint32_t numDocs= fidx_->numIDs();
    
    docL2_.clear();
    docL2_.resize( numDocs, 0.0 );
    std::vector<rr::indexEntry> entries;
    
    uint32_t numWords_printStep= std::max(static_cast<uint32_t>(1),numWords/20);
    
    std::cout<<"wgc::computeDocL2\n";
    double time= timing::tic();
    
    for (uint32_t wordID= 0; wordID < numWords; ++wordID){
        if (wordID % numWords_printStep == 0)
            std::cout<<"wgc::computeDocL2: wordID= "<<wordID<<" / "<<numWords<<" "<<timing::toc(time)<<" ms\n";
        
        iidx_->getEntries( wordID, entries );
        
        for (uint32_t iEntry= 0; iEntry<entries.size(); ++iEntry){
            rr::indexEntry &entry= entries[iEntry];
            
            // set/add weights and multiply by idf
            tfidfV2::weightStatic(entry, &idf_[wordID], NULL);
            
            for (int i= 0; i<entry.id_size(); ++i)
                docL2_[ entry.id(i) ]+= entry.weight(i) * entry.weight(i);
        }
        
    }
    
    for (uint32_t docID= 0; docID < numDocs; ++docID){
        if ( docL2_[docID] <= 1e-7 )
            docL2_[docID]= 1.0;
        else
            docL2_[docID]= sqrt( docL2_[docID] );
    }
    
    std::cout<<"wgc::computeDocL2: DONE ("<<timing::toc(time)<<" ms)\n";
    
}
