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

#include "weighter_v2.h"

#include <math.h>



void
weighterV2::queryExecute(
        rr::indexEntry const &queryRep,
        ueIterator *ueIter,
        std::vector<double> const &idf,
        std::vector<double> const &docL2,
        std::vector<double> &scores,
        double defaultScore ){
    
    ASSERT(queryRep.id_size()==queryRep.weight_size());
    
    scores.clear();
    scores.resize( docL2.size(), 0.0 );
    
    double queryL2= 0.0, queryW= 0.0, widf;
    uint32_t wordID;
    
    for (int iQueryWord= 0; iQueryWord < queryRep.id_size();){
        
        wordID= queryRep.id(iQueryWord);
        queryW= 0.0;
        
        // get sum of weights, e.g (wordID,weight): [(4, 1.0), (4, 0.5)] -> [4, 1.5]
        int prevIQueryWord= iQueryWord;
        for (; iQueryWord < queryRep.id_size() && queryRep.id(iQueryWord)==wordID;
               ++iQueryWord)
            queryW+= queryRep.weight(iQueryWord);
        ueIter->advance( iQueryWord - prevIQueryWord -1 );
        
        widf= idf[wordID] * queryW;
        queryL2+= queryW * queryW;
        
        // weight entries
        
        ASSERT( static_cast<uint32_t>(iQueryWord) == ueIter->getInd()+1 );
        std::vector<rr::indexEntry> *entries= ueIter->getEntries();
        ueIter->increment();
        
        for (uint32_t iEntry= 0; iEntry<entries->size(); ++iEntry){
            rr::indexEntry const &entry= entries->at(iEntry);
            uint32_t const *itID= entry.id().data();
            uint32_t const *endID= itID + entry.id_size();
            
            if (entry.weight_size()!=0) {
                
                ASSERT( entry.id_size() == entry.weight_size() );
                
                // - the following code is equivalent (but a bit faster) to:
                // for (int i= 0; i < entry.id_size(); ++i)
                //     scores[ entry.id(i) ]+= entry.weight(i) * widf;
                float const *itW= entry.weight().data();
                for (; itID!=endID; ++itW, ++itID)
                    scores[ *itID ]+= *itW * widf;
                
            } else if (entry.count_size()!=0) {
                
                ASSERT( entry.id_size() == entry.count_size() );
                
                // - the following code is equivalent (but a bit faster) to:
                // for (int i= 0; i < entry.id_size(); ++i)
                //     scores[ entry.id(i) ]+= static_cast<double>(entry.count(i)) * widf;
                unsigned const *itC= entry.count().data();
                for (; itID!=endID; ++itC, ++itID)
                    scores[ *itID ]+= static_cast<double>(*itC) * widf;
                
            } else {
                
                // - the following code is equivalent (but a bit faster) to:
                // for (int i= 0; i < entry.id_size(); ++i)
                //     scores[ entry.id(i) ]+= widf;
                for (; itID!=endID; ++itID)
                    scores[ *itID ]+= widf;
                
            }
        }
        
    }
    
    double queryL2sqrt= sqrt(queryL2);
    if (queryL2sqrt <= 1e-7)
        queryL2sqrt= 1.0;
    double defaultScoreByNorm= defaultScore / queryL2sqrt;
    
    std::vector<double>::const_iterator docL2Iter= docL2.begin();
    for (std::vector<double>::iterator itS= scores.begin(); itS!=scores.end(); ++itS, ++docL2Iter)
        (*itS)= (*itS) / ( queryL2sqrt * (*docL2Iter) ) + defaultScoreByNorm;
    
}



void
weighterV2::queryExecuteWGC(
        rr::indexEntry const &queryRep,
        ueIterator *ueIter,
        std::vector<double> const &idf,
        std::vector<double> const &docL2,
        std::vector<double> &scores,
        uint16_t numScales,
        double defaultScore ){
    
    ASSERT(queryRep.id_size()==queryRep.weight_size());
    ASSERT(queryRep.has_qel_scale()); // can't be bothered to implement for uncompressed ellipses - will never use it
    std::string const &queryScaleStr= queryRep.qel_scale();
    ASSERT(static_cast<uint32_t>(queryRep.id_size())==queryScaleStr.length());
    unsigned char const *itQueryScale= reinterpret_cast<unsigned char const*>(queryScaleStr.c_str());
    
    std::vector<float> scoresScale( docL2.size() * numScales, 0.0f );
    
    uint16_t scaleStep= std::ceil(static_cast<double>(255*2) / numScales);
    
    double queryL2= 0.0, queryW= 0.0, widf;
    uint32_t wordID;
    
    for (int iQueryWord= 0; iQueryWord < queryRep.id_size(); ++iQueryWord, ++itQueryScale, ueIter->increment()){
        
        wordID= queryRep.id(iQueryWord);
        queryW= queryRep.weight(iQueryWord);
        uint16_t queryScale= static_cast<uint16_t>(*itQueryScale) + 255;
        
        widf= idf[wordID] * queryW;
        queryL2+= queryW * queryW;
        
        // weight entries
        
        ASSERT( static_cast<uint32_t>(iQueryWord) == ueIter->getInd() );
        std::vector<rr::indexEntry> *entries= ueIter->getEntries();
        
        for (uint32_t iEntry= 0; iEntry<entries->size(); ++iEntry){
            rr::indexEntry const &entry= entries->at(iEntry);
            uint32_t const *itID= entry.id().data();
            uint32_t const *endID= itID + entry.id_size();
            
            ASSERT(entry.has_qel_scale());
            std::string const &entryScaleStr= entry.qel_scale();
            ASSERT(static_cast<uint32_t>(entry.id_size())==entryScaleStr.length());
            unsigned char const *itEntryScale= reinterpret_cast<unsigned char const*>(entryScaleStr.c_str());
            
            ASSERT(entry.weight_size()==0 && entry.count_size()==0); // TODO
            
            for (; itID!=endID; ++itID, ++itEntryScale)
                scoresScale[ *itID * numScales + (queryScale - *itEntryScale)/scaleStep  ]+= widf;
        }
        
    }
    
    double queryL2sqrt= sqrt(queryL2);
    if (queryL2sqrt <= 1e-7)
        queryL2sqrt= 1.0;
    double defaultScoreByNorm= defaultScore / queryL2sqrt;
    
    scores.clear();
    scores.resize( docL2.size(), 0.0 );
    
    std::vector<double>::const_iterator docL2Iter= docL2.begin();
    std::vector<float>::const_iterator ssIter= scoresScale.begin();
    uint8_t averageW= static_cast<uint8_t>( std::max(std::ceil(static_cast<double>(numScales)/16), 1.0) );
    
    
    for (std::vector<double>::iterator itS= scores.begin(); itS!=scores.end(); ++itS, ++docL2Iter){
        // moving average
        std::vector<float>::const_iterator ssFirst= ssIter;
        std::vector<float>::const_iterator ssEnd= ssIter + numScales;
        float sum= 0;
        
        // first sum
        std::vector<float>::const_iterator ssFirstEnd= ssIter + averageW;
        for (; ssIter!=ssFirstEnd; ++ssIter)
            sum+= *ssIter;
        *itS= sum;
        
        // other sums
        for(; ssIter!=ssEnd; ++ssIter, ++ssFirst){
            sum+= *ssIter - *ssFirst;
            if (sum > *itS)
                *itS= sum;
        }
        
        (*itS)= (*itS)/averageW / ( queryL2sqrt * (*docL2Iter) ) + defaultScoreByNorm;
    }
    
}
