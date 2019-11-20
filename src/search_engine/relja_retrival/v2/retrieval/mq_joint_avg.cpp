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

#include "mq_joint_avg.h"

#include "hamming_embedder.h"
#include "protobuf_util.h"



void
multiQueryJointAvg::queryExecute(
        std::vector<query> const &queries,
        std::vector<indScorePair> &queryRes,
        uint32_t toReturn ) const {
    
    rr::indexEntry queryRepTemp;
    
    embedderFactory const *eF= (retriever_->embFactory_==NULL) ?
        NULL :
        retriever_->embFactory_;
    
    embedder *embTemp= NULL;
    if (eF!=NULL)
        embTemp= eF->getEmbedder();
    
    std::vector<rr::indexEntry> queryReps( queries.size() );
    
    for (uint32_t iQuery= 0; iQuery<queries.size(); ++iQuery){
        rr::indexEntry &thisQueryRep= queryReps[iQuery];
        if (spat_!=NULL)
            spat_->getQueryRep(queries[iQuery], thisQueryRep);
        else
            retriever_->getQueryRep(queries[iQuery], thisQueryRep);
        
        ASSERT( thisQueryRep.id_size()==0 || thisQueryRep.has_data() );
        
        if (thisQueryRep.id_size()>0){
            
            indexEntryUtil::quantXY(thisQueryRep);
            ASSERT(thisQueryRep.qx_size()>0);
            
            PROTOBUFUTIL_ADD_ALL(thisQueryRep, queryRepTemp, id);
            PROTOBUFUTIL_ADD_ALL(thisQueryRep, queryRepTemp, qx);
            PROTOBUFUTIL_ADD_ALL(thisQueryRep, queryRepTemp, qy);
            
            if (eF!=NULL){
                embedder *embThis= eF->getEmbedder();
                embThis->setDataCopy(thisQueryRep.data());
                
                embTemp->copyRangeFrom(*embThis, 0, thisQueryRep.id_size());
                
                delete embThis;
            }
        }
    }
    
    // sort according to clusterID
    std::vector<int> inds;
    indexEntryUtil::argSort::sort(queryRepTemp, inds);
    
    // apply the sort
    rr::indexEntry queryRep= queryRepTemp; // to get all repeated field sizes correctly
    uint32_t *wordID= queryRep.mutable_id()->mutable_data();
    uint32_t *docID= queryRep.mutable_docid()->mutable_data();
    uint32_t *qx= queryRep.mutable_qx()->mutable_data();
    uint32_t *qy= queryRep.mutable_qy()->mutable_data();
    uint32_t size= queryRepTemp.id_size();
    embedder *emb= NULL;
    if (eF!=NULL)
        emb= eF->getEmbedder();
    
    for (uint32_t i= 0; i<size; ++i, ++wordID, ++docID, ++qx, ++qy){
        int ind= inds[i];
        *wordID= queryRepTemp.id(ind);
        *qx= queryRepTemp.qx(ind);
        *qy= queryRepTemp.qy(ind);
        if (eF!=NULL)
            emb->copyFrom(*embTemp, ind);
    }
    
    queryRep.set_data(emb->getEncoding());
    
    if (eF!=NULL){
        delete emb;
        delete embTemp;
    }
    
    // do the query
    retriever_->queryExecute(queryRep, queryRes, toReturn);
    
    if (spat_!=NULL){
        for (uint32_t iQuery= 0; iQuery<queries.size(); ++iQuery){
            spat_->spatialQueryExecute(
                queryReps[iQuery],
                queryRes,
                NULL, NULL,
                toReturn,
                true,
                true);
        }
    }
    
}
