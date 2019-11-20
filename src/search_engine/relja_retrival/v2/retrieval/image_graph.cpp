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

#include "image_graph.h"

#include <iostream>
#include <fstream>

#include <boost/format.hpp>

#ifdef RR_MPI
#include <boost/mpi/collectives.hpp>
#include <boost/serialization/utility.hpp> // for std::pair
#include <boost/serialization/vector.hpp>
#endif

#include "mpi_queue.h"
#include "par_queue.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"



void
imageGraph::computeSingle(
        std::string filename,
        uint32_t numDocs,
        retriever const &retrieverObj,
        uint32_t maxNeighs,
        double scoreThr ) {
    
    graph_.clear();
    
    // prepare files
    
    protoDbFileBuilder dbBuilder(filename, "image graph");
    indexBuilder idxBuilder(dbBuilder, false, false, false); // no "doDiff" compression as I'm not sorting returned docIDs, but one could
    
    // do it all
    
    double score;
    uint32_t numNeighsTotal= 0, docIDres;
    
    std::vector<indScorePair> queryRes;
    
    timing::progressPrint graphBuildProgress(numDocs, "imageGraph");
    
    for (uint32_t docID=0; docID < numDocs; ++docID){
        
        // print debug info
        
        graphBuildProgress.inc( docID==0 ?
            "" :
            (boost::format("neigh/image= %.2f") % (static_cast<double>(numNeighsTotal)/docID)).str()
            );
        
        // query with docID
        
        queryRes.clear();
        retrieverObj.internalQuery( docID, queryRes, maxNeighs );
        ASSERT( maxNeighs==0 || queryRes.size()<=maxNeighs );
        
        // save results into the graph
        
        std::vector<indScorePair> neighs;
        rr::indexEntry entry; // format for saving
        
        for (std::vector<indScorePair>::const_iterator itRes= queryRes.begin();
             itRes!=queryRes.end();
             ++itRes){
                
                docIDres= itRes->first;
                score= itRes->second;
                
                // skip self
                if (docIDres == docID)
                    continue;
                
                // if it passes the score threshold, save it
                if (score >= scoreThr){
                    
                    entry.add_id(docIDres);
                    entry.add_weight(score);
                    
                    neighs.push_back( *itRes );
                    
                } else
                    // otherwise stop execution as no future scores will be large enough since they are sorted in non-increasing order
                    break;
                
            }
        
        if (neighs.size() > 0){
            
            // insert into the graph
            graph_[docID]= neighs;
            
            // save to file
            idxBuilder.addEntry(docID, entry);
        }
        
        numNeighsTotal+= neighs.size();
        
    }
    
    // close the file explicitly, though its destructor would do it anyway
    idxBuilder.close();
    
}



// ------- imageGraph::computeParallel and helper functions



typedef std::vector<indScorePair> imageGraphResult;

class imageGraphManager : public managerWithTiming<imageGraphResult> {
    public:
        
        imageGraphManager(std::string filename,
                          uint32_t numDocs,
                          double scoreThr)
            : managerWithTiming<imageGraphResult>(numDocs, "imageGraph"),
              dbBuilder_(filename, "image graph"),
              idxBuilder_(dbBuilder_, false, false, false),
              scoreThr_(scoreThr),
              currentDocID(0)
                {}
        
        void
            compute(uint32_t docID, imageGraphResult &queryRes);
        
        void
            finalize();
        
        imageGraph::imageGraphType graph_;
        
    private:
        protoDbFileBuilder dbBuilder_;
        indexBuilder idxBuilder_;
        double const scoreThr_;
        std::map<uint32_t, rr::indexEntry> buffer_;
        uint32_t currentDocID;
};



void
imageGraphManager::compute(uint32_t docID, imageGraphResult &queryRes){
    
    // save results returned by the worker into the graph
    
    std::vector<indScorePair> neighs;
    rr::indexEntry entry; // format for saving
    
    // if this part takes long (it doesn't here) then it should be handled by the worker, e.g. the worker would create neighs and entry and send it as the result
    
    uint32_t docIDres;
    double score;
    
    for (std::vector<indScorePair>::const_iterator itRes= queryRes.begin();
        itRes!=queryRes.end();
        ++itRes){
            
            docIDres= itRes->first;
            score= itRes->second;
            
            // skip self
            if (docIDres == docID)
                continue;
            
            // if it passes the score threshold, save it
            if (score >= scoreThr_){
                
                entry.add_id(docIDres);
                entry.add_weight(score);
                
                neighs.push_back( *itRes );
                
            } else
                // otherwise stop execution as no future scores will be large enough since they are sorted in non-increasing order
                break;
            
        }
    
    if (neighs.size() > 0){
        
        // insert into the graph
        graph_[docID]= neighs;
        
        /*
        // NOTE: Don't do this (naively copied from computeSingle
        // because docID's need to be added in ascending sequence to indexBuilder
        // and this is not guaranteed to happen (actually almost definitely
        // doesn't) if we do it now. Instead, save results into a buffer
        // and save them in increasing order
        
        // save to file
        idxBuilder_.addEntry(docID, entry);
        */
    }
    
    //--- the buffer stuff from comment above
    
    // add to buffer
    buffer_[docID]= entry;
    
    // check if we can save something (i.e. we have all results from before this docID
    for (; buffer_.count(currentDocID)!=0; ++currentDocID){
        entry= buffer_[currentDocID];
        if (entry.id_size()>0)
            idxBuilder_.addEntry(currentDocID, entry);
        buffer_.erase(currentDocID);
    }
}



void
imageGraphManager::finalize(){
    
    ASSERT(buffer_.size()==0);
    
    // close the file explicitly, though its destructor would do it anyway
    idxBuilder_.close();
}



class imageGraphWorker : public queueWorker<imageGraphResult> {
    
    public:
        
        imageGraphWorker(
            retriever const &retrieverObj,
            uint32_t maxNeighs)
                : retriever_(retrieverObj),
                maxNeighs_(maxNeighs) {}
        
        void
            operator() ( uint32_t docID, imageGraphResult &queryRes ) const {
                queryRes.clear();
                retriever_.internalQuery(docID, queryRes, maxNeighs_);
                ASSERT( maxNeighs_==0 || queryRes.size()<=maxNeighs_ );
            }
        
    private:
        retriever const &retriever_;
        uint32_t const maxNeighs_;
};



void
imageGraph::computeParallel(
        std::string filename,
        uint32_t numDocs,
        retriever const &retrieverObj,
        uint32_t maxNeighs,
        double scoreThr ) {
    
    MPI_GLOBAL_RANK
    bool useThreads= detectUseThreads();
    uint32_t numWorkerThreads= 4;
    
    graph_.clear();
    
    if (rank==0)
        std::cout<<"imageGraph::computeParallel\n";
    
    // make the manager if we are on the master node, otherwise NULL
    imageGraphManager *manager= (rank==0) ?
            new imageGraphManager(filename, numDocs, scoreThr) :
            NULL;
    
    // make the worker
    imageGraphWorker worker(retrieverObj, maxNeighs);
    
    parQueue<imageGraphResult>::startStatic(
        numDocs,
        worker,
        manager,
        useThreads,
        numWorkerThreads);
    
    // copy the result
    if (rank==0) graph_= manager->graph_;
    
    // free memory
    if (rank==0) delete manager;
    
}



// -------



void
imageGraph::loadFromFile( std::string filename ){
    
    std::cout<<"imageGraph::loadFromFile\n";
    
    graph_.clear();
    
    // open files
    protoDbFile db(filename);
    protoIndex idx(db, false);
    
    std::vector<rr::indexEntry> entries;
    
    for (uint32_t docID= 0; docID<idx.numIDs(); ++docID){
        
        // load
        idx.getEntries(docID, entries);
        ASSERT(entries.size()<=1);
        
        // if there are edges to this node
        if (entries.size()==1){
            
            // copy from indexEntry to graph_
            rr::indexEntry const &entry= entries[0];
            std::vector<indScorePair> &neighs= graph_[docID];
            ASSERT( entry.id_size() == entry.weight_size() );
            neighs.reserve( entry.id_size() );
            
            for (int iEntry= 0; iEntry<entry.id_size(); ++iEntry)
                neighs.push_back(
                    std::make_pair(entry.id(iEntry),
                                   entry.weight(iEntry)) );
        }
        
    }
    
    std::cout<<"imageGraph::loadFromFile - DONE\n";
    
}
