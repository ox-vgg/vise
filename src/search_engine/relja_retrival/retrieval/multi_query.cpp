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

#include "multi_query.h"



void
multiQueryIndpt::queryExecute( std::vector<query> const &queries, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    queryRes.clear();
    
    std::vector<indScorePair> thisRes;
    thisRes.reserve( toReturn );
    
    std::vector<double> scores(numDocs_);
    
    mqIndpt_worker worker( *retriever_obj, queries, workerReturnOnlyTop() ? toReturn : 0 );
    queueManager<Result> *manager= getManager( scores );
    
    // TODO: how many threads?
    threadQueue<Result>::start( queries.size(), worker, *manager, 8 );
    
    // NOTE: important to call this before sorting the results
    delete manager;
    
    retriever::sortResults( scores, queryRes, toReturn );
    
}



void
multiQueryIndpt::mqIndpt_worker::operator() ( uint32_t jobID, Result &result ) const {
    
    // manager should delete this
    result= new std::vector<indScorePair>;
    result->reserve( toReturn );
    
    // do the query
    retriever_obj->queryExecute(
        queries->at(jobID),
        *result,
        toReturn);
    
}



void
multiQueryMax::mqMax_manager::operator() ( uint32_t jobID, Result &result ) {
    
    // TODO keep a sorted list of results and track the top toReturn, instead of tracking everything and then re-sorting
    for (std::vector<indScorePair>::const_iterator itR= result->begin(); itR!=result->end(); ++itR){
        if (first || itR->second > scores->at( itR->first ) )
            scores->at( itR->first )= itR->second;
    }
    
    first= false;
    
    delete result;
    
}



void
multiQueryAvg::mqAvg_manager::operator() ( uint32_t jobID, Result &result ) {
    
    for (std::vector<indScorePair>::const_iterator itR= result->begin(); itR!=result->end(); ++itR){
        if (first)
            scores->at( itR->first ) = itR->second;
        else
            scores->at( itR->first )+= itR->second;
    }
    
    first= false;
    
    delete result;
    
}



void
multiQueryMaxK::mqMaxK_manager::operator() ( uint32_t jobID, Result &result ) {
    
    for (std::vector<indScorePair>::const_iterator itR= result->begin(); itR!=result->end(); ++itR){
        uint32_t start= itR->first * K_;
        uint32_t ind= start + std::min(doneK_, K_-1);
        
        if (doneK_<K_ || itR->second > scoresK_[ind])
            scoresK_[ind]= itR->second;
        
        for (; start != ind && scoresK_[ind] > scoresK_[ind-1]; --ind)
            std::swap(scoresK_[ind-1], scoresK_[ind]);
    }
    
    ++doneK_;
    
    delete result;
    
}



multiQueryMaxK::mqMaxK_manager::~mqMaxK_manager() {
    
    for (uint32_t docID= 0; docID < numDocs_; ++docID){
        scores->at(docID)= scoresK_[docID * K_ + K_-1];
    }
    
}
