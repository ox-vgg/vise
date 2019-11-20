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

#ifndef _MULTI_QUERY_H_
#define _MULTI_QUERY_H_

#include <stdexcept>
#include <string>
#include <vector>

#include "par_queue.h"
#include "thread_queue.h"
#include "retriever.h"



class multiQuery {
    
    public:
        
        multiQuery() {}
        
        virtual
            ~multiQuery() {};
        
        virtual void
            externalQuery_computeData( std::vector<std::string> const &imageFns, std::vector<query> const &queries ) const {
                throw std::runtime_error("Not implemented");
            }
        
        virtual void
            queryExecute( std::vector<query> const &queries, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const =0;
        
        virtual uint32_t
            numDocs() const =0;
    
};



class multiQueryIndpt : public multiQuery {
    
    protected:
        
        typedef std::vector<indScorePair>* Result;
    
    public:
        
        multiQueryIndpt( retriever const &aRetriever_obj ) : retriever_obj(&aRetriever_obj), numDocs_(retriever_obj->numDocs()) {}
        
        virtual
            ~multiQueryIndpt() {}
        
        void
            queryExecute( std::vector<query> const &queries, std::vector<indScorePair> &queryRes, uint32_t toReturn= 0 ) const;
        
        uint32_t
            numDocs() const { return numDocs_; }
        
        virtual bool
            workerReturnOnlyTop() const =0;
        
        virtual queueManager<Result>*
            getManager( std::vector<double> &aScores ) const =0;
        
    protected:
        
        retriever const *retriever_obj;
        uint32_t numDocs_;
        
    private:
        
        class mqIndpt_worker : public queueWorker<Result> {
            public:
                mqIndpt_worker( retriever const &aRetriever_obj, std::vector<query> const &aQueries, uint32_t aToReturn ) : retriever_obj(&aRetriever_obj), queries(&aQueries), toReturn(aToReturn) {}
                void operator() ( uint32_t jobID, Result &result ) const;
            private:
                retriever const *retriever_obj;
                std::vector<query> const *queries;
                const uint32_t toReturn;
        };
        
    
};



class multiQueryMax : public multiQueryIndpt {
    
    public:
        
        multiQueryMax( retriever const &aRetriever_obj ) : multiQueryIndpt( aRetriever_obj ) {}
        
        bool
            workerReturnOnlyTop() const { return true; }
        
        queueManager<Result>*
            getManager( std::vector<double> &aScores ) const {
                return new mqMax_manager( aScores );
            };
    
    private:
    
        class mqMax_manager : public queueManager<Result> {
            public:
                mqMax_manager( std::vector<double> &aScores ) : scores(&aScores), first(true) {}
                void operator() ( uint32_t jobID, Result &result );
                std::vector<double> *scores;
            private:
                bool first;
        };
    
};



class multiQueryAvg : public multiQueryIndpt {
    
    public:
        
        multiQueryAvg( retriever const &aRetriever_obj ) : multiQueryIndpt( aRetriever_obj ) {}
        
        bool
            workerReturnOnlyTop() const { return false; }
        
        queueManager<Result>*
            getManager( std::vector<double> &aScores ) const {
                return new mqAvg_manager( aScores );
            };
    
    private:
    
        class mqAvg_manager : public queueManager<Result> {
            public:
                mqAvg_manager( std::vector<double> &aScores ) : scores(&aScores), first(true) {}
                void operator() ( uint32_t jobID, Result &result );
                std::vector<double> *scores;
            private:
                bool first;
        };
    
};



class multiQueryMaxK : public multiQueryIndpt {
    
    public:
        
        multiQueryMaxK( retriever const &aRetriever_obj, uint32_t K= 2 ) : multiQueryIndpt( aRetriever_obj ), K_(K) {}
        
        bool
            workerReturnOnlyTop() const { return false; }
        
        queueManager<Result>*
            getManager( std::vector<double> &aScores ) const {
                return new mqMaxK_manager( aScores, numDocs(), K_ );
            };
    
    private:
    
        class mqMaxK_manager : public queueManager<Result> {
            public:
                mqMaxK_manager( std::vector<double> &aScores, uint32_t numDocs, uint32_t K ) : scores(&aScores), doneK_(0), numDocs_(numDocs), K_(K) {
                    scoresK_.resize(numDocs_*K_, 0.0);
                }
                ~mqMaxK_manager();
                void operator() ( uint32_t jobID, Result &result );
                std::vector<double> *scores;
            private:
                uint32_t doneK_;
                uint32_t numDocs_, K_;
                std::vector<double> scoresK_;
        };
        
        uint32_t K_;
    
};



#endif
