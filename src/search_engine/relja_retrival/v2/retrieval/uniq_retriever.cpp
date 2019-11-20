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

#include "uniq_retriever.h"

#include <algorithm>



uniqRetriever::uniqRetriever(
        retriever const &firstRetriever,
        std::vector<uint32_t> const &docIDtoObjID)
        : firstRetriever_(&firstRetriever),
          docIDtoObjID_(&docIDtoObjID) {
    
    maxObjID_= *std::max_element(docIDtoObjID_->begin(), docIDtoObjID_->end());
}



void
uniqRetriever::filterSameObject(std::vector<indScorePair> &queryRes, uint32_t toReturn) const {
    
    uint32_t currI= 0;
    uint32_t last= queryRes.size();
    if (toReturn!=0 && toReturn<last)
        last= toReturn;
    
    std::vector<bool> used(maxObjID_, false);
    
    for (uint32_t i= 0; currI<last && i<queryRes.size(); ++i){
        indScorePair const &qr= queryRes[i];
        uint32_t const objID= (*docIDtoObjID_)[qr.first];
        if (!used[objID]){
            used[objID]= true;
            queryRes[currI]= qr;
            ++currI;
        }
    }
    
    queryRes.resize(currI);
}
