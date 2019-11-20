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

#include "mq_filter_outliers.h"

#include "bitcount.h"




mqFilterOutliers::mqFilterOutliers(
        multiQuery const &mq,
        retrieverV2 const &baseRetriever,
        hammingEmbedderFactory const &embFactory)
        : mq_(&mq),
          baseRet_(&baseRetriever),
          embFactory_(&embFactory),
          scoreThr_(1), // nobody apart from itself
          failureProp_(0.25) // if only 1/4 kept, outlier detection failed most likely
          {
    uint32_t const numBits= embFactory.numBits();
    
    // threshold for 64 is 16
    distThr_= static_cast<int>( round( static_cast<float>(numBits) / 4 ) );
}



void
mqFilterOutliers::queryExecute( std::vector<query> const &queries, std::vector<indScorePair> &queryRes, uint32_t toReturn ) const {
    
    uint32_t nQ= queries.size();
    uint32_t const minKept= static_cast<uint32_t>( round(failureProp_ * nQ) );
    
    // collect query representations
    // TODO parallel as this could potentially be a bit slow for internal queries where queryRep needs to be collected by going through the inverted index
    // TODO reuse this in the mq_->queryExecute call
    
    std::vector<rr::indexEntry> queryReps(nQ);
    std::vector< std::vector<uint64_t> > hammingSigs(nQ);
    
    for (uint32_t iQ= 0; iQ<nQ; ++iQ){
        baseRet_->getQueryRep(queries[iQ], queryReps[iQ]);
        ASSERT( queryReps[iQ].id_size()==0 || queryReps[iQ].has_data() );
        
        hammingEmbedder *emb= embFactory_->getEmbedder();
        emb->setDataCopy(queryReps[iQ].data());
        charStream* cs= emb->getCharStream();
        ASSERT(cs->getNum() == static_cast<uint32_t>(queryReps[iQ].id_size()));
        
        std::vector<uint64_t> &sig= hammingSigs[iQ];
        sig.reserve(queryReps[iQ].id_size());
        for (int i= 0; i<queryReps[iQ].id_size(); ++i)
            sig.push_back( cs->getNextUnsafe() );
        
        delete emb;
    }
    
    // match all pairs
    uint32_t *score= new uint32_t[nQ*nQ];
    
    for (uint32_t iQ1= 0; iQ1<nQ-1; ++iQ1){
        
        rr::indexEntry const &q1= queryReps[iQ1];
        std::vector<uint64_t> const &sig1= hammingSigs[iQ1];
        
        score[ iQ1*nQ + iQ1 ]= q1.id_size();
        
        for (uint32_t iQ2= iQ1+1; iQ2<nQ; ++iQ2){
            
            rr::indexEntry const &q2= queryReps[iQ2];
            std::vector<uint64_t> const &sig2= hammingSigs[iQ2];
            
            uint32_t thisScore= 0;
            
            // match features assuming wordIDs are sorted
            
            int i1= 0, i1end;
            int i2= 0, i2end;
            
            while (i1 < q1.id_size() && i2 < q2.id_size()){
                
                // find equal wordIDs accross queryRep's
                while(i1 < q1.id_size() && i2 < q2.id_size() &&
                      q1.id(i1) != q2.id(i2)) {
                    
                    if (q1.id(i1) < q2.id(i2))
                        ++i1;
                    else
                        ++i2;
                }
                
                // found equal or end
                
                if (i1 < q1.id_size() && i2 < q2.id_size()){
                    ASSERT(q1.id(i1) == q2.id(i2));
                    // find all with same wordID within one queryRep
                    for (i1end= i1; i1end < q1.id_size() && q1.id(i1)==q1.id(i1end); ++i1end);
                    for (i2end= i2; i2end < q2.id_size() && q2.id(i2)==q2.id(i2end); ++i2end);
                    // measure all similarities
                    for (int j1= i1; j1<i1end; ++j1){
                        for (int j2= i2; j2<i2end; ++j2)
                            if ( bitcount64(sig1[j1]^sig2[j2]) <= distThr_ )
                                ++thisScore;
                    }
                    i1= i1end;
                    i2= i2end;
                }
                
            }
            
            score[ iQ1*nQ + iQ2 ]= thisScore;
            score[ iQ2*nQ + iQ1 ]= thisScore;
        }
    }
    
    // figure out which ones to keep
    
    std::vector<bool> keep(nQ, true);
    std::vector<uint32_t> totalScore(nQ);
    uint32_t numKept= nQ;
    bool keepUpdated= false;
    
    while (true){
        std::vector<bool> thisKeep= keep;
        keepUpdated= false;
        
        // compute the total scores counting only included images
        // and find the minimal score
        // and remove singleton ones (i.e. only vote from itself) along the way
        uint32_t minInd= nQ;
        
        for (uint32_t iQ1= 0; iQ1<nQ; ++iQ1){
            if (!keep[iQ1]) continue;
            totalScore[iQ1]= 0;
            for (uint32_t iQ2= 0; iQ2<nQ; ++iQ2)
                if (keep[iQ2])
                    totalScore[iQ1]+= (score[ iQ1*nQ + iQ2 ] >= 4);
            
            if (totalScore[iQ1] <=1 ){
                // nobody apart from itself
                thisKeep[iQ1]= false;
                keepUpdated= true;
                --numKept;
            } else  {
                // check if minimal score
                if (minInd==nQ || totalScore[iQ1] < totalScore[minInd])
                    minInd= iQ1;
            }
        }
        ASSERT( (minInd!=nQ) != (numKept==0) );
        
        // did we find and remove any singletons?
        if (keepUpdated){
            if (numKept < minKept){
                // outlier discovery failed, don't update keep
                break;
            }
            // update keep with thisKeep
            keep= thisKeep;
        }
        
        // remove the minimal score if it is smaller than the threshold
        if (totalScore[minInd] <= scoreThr_){
            if (numKept==minKept){
                // outlier discovery failed, don't update keep
                ASSERT(scoreThr_!=1); // if 1, the previous one should have removed it
                break;
            }
            keep[minInd]= false;
            --numKept;
        } else
            break;
        
        ASSERT( scoreThr_!=1 ); // if equal to 1, this line shouldn't have been reached
    }
    
    delete []score;
    
    // filter the queries
    std::vector<query> queriesFilt;
    queriesFilt.reserve(nQ);
    for (uint32_t iQ= 0; iQ<nQ; ++iQ)
        if (keep[iQ])
            queriesFilt.push_back(queries[iQ]);
    
    // execute the filtered ones
    mq_->queryExecute( queriesFilt, queryRes, toReturn );
    
}
