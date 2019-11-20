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

#ifndef _THREAD_QUEUE_H_
#define _THREAD_QUEUE_H_

#include <stdint.h>
#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

#include "macros.h"
#include "par_queue.h"
#include "util.h"


template <class Result>
class queueWorker;
template <class Result>
class queueManager;
template <typename Result>
void serialQueue( uint32_t nJobs, queueWorker<Result> const &worker, queueManager<Result> &manager );


template <class Result>
class threadQueue {
    
    public:
        
        inline static void
            start( uint32_t nJobs,
                   queueWorker<Result> const &worker,
                   queueManager<Result> &manager,
                   uint32_t numWorkerThreads){
                start(nJobs, &worker, NULL, manager, numWorkerThreads);
            }
        
        inline static void
            start( uint32_t nJobs,
                   std::vector< queueWorker<Result> const * > const &workers,
                   queueManager<Result> &manager) {
                start(nJobs, NULL, &workers, manager, workers.size());
            }
       
    private:
        
        static void
            start( uint32_t nJobs,
                   queueWorker<Result> const *worker,
                   std::vector< queueWorker<Result> const * > const *workers,
                   queueManager<Result> &manager,
                   uint32_t numWorkerThreads
                   );
        
        threadQueue(){}
        DISALLOW_COPY_AND_ASSIGN(threadQueue)
        
        class threadWorker_ {
            public:
                
                threadWorker_( uint32_t aNJobs,
                               queueWorker<Result> const &aThreadWorker,
                               uint32_t &aJobID,
                               boost::mutex &aJobIDLock,
                               std::vector< std::pair<uint32_t, Result> > &aResultQueue,
                               boost::mutex &aResultsLock,
                               boost::condition_variable &aResultsReady )
                               : stopJobs_(false),
                                 threadWorker_obj(&aThreadWorker),
                                 jobID(&aJobID),
                                 nJobs(aNJobs),
                                 resultQueue(&aResultQueue),
                                 jobIDLock(&aJobIDLock),
                                 resultsLock(&aResultsLock),
                                 resultsReady(&aResultsReady)
                {}
                
                void
                    operator()() {
                        
                        uint32_t thisJobID= 0;
                        while (!stopJobs_ && thisJobID < nJobs){
                            // get work
                            {
                                boost::mutex::scoped_lock lock(*jobIDLock);
                                thisJobID= (*jobID);
                                ++(*jobID);
                            }
                            // do work
                            if (!stopJobs_ && thisJobID < nJobs){
                                Result result;
                                (*threadWorker_obj)(thisJobID, result );
                                // send result
                                if (!stopJobs_){
                                    boost::mutex::scoped_lock lock(*resultsLock);
                                    resultQueue->push_back(
                                        std::make_pair(thisJobID, result)
                                        );
                                    lock.unlock();
                                    // report a result is ready
                                    resultsReady->notify_one();
                                }
                            }
                        }
                    }
                
                bool stopJobs_;
            
            private:
                queueWorker<Result> const *threadWorker_obj;
                uint32_t *jobID, nJobs;
                std::vector< std::pair<uint32_t, Result> > *resultQueue;
                boost::mutex *jobIDLock, *resultsLock;
                boost::condition_variable *resultsReady;
                
                DISALLOW_COPY_AND_ASSIGN(threadWorker_)
        };
    
};

void threadQueue_test();


template <class Result>
void
threadQueue<Result>::start(
        uint32_t nJobs,
        queueWorker<Result> const *worker,
        std::vector< queueWorker<Result> const * > const *workers,
        queueManager<Result> &manager,
        uint32_t numWorkerThreads ) {
    
    if (numWorkerThreads<=1){
        serialQueue<Result>(nJobs, worker==NULL ? *(workers->at(0)) : *worker, manager);
        return;
    }
    
    uint32_t numThreads= numWorkerThreads+1;
    uint32_t jobID= 0;
    
    boost::mutex jobIDLock, resultsLock;
    std::vector<boost::thread*> queue(0);
    
    boost::condition_variable resultsReady;
    std::vector< std::pair<uint32_t, Result> > resultQueue;
    std::vector< threadWorker_* > threadWorkers;
    
    // start queue
    for (uint32_t iThread=1; iThread < numThreads; iThread++){
        threadWorkers.push_back(new threadWorker_(nJobs, worker==NULL ? *(workers->at(iThread-1)) : *worker, jobID, jobIDLock, resultQueue, resultsLock, resultsReady));
        queue.push_back(
            new boost::thread( &threadWorker_::operator(), threadWorkers.back() )
            );
    }
    
    uint32_t thisJobID= 0;
    uint32_t completedJobs;
    
    for (completedJobs=0; completedJobs<nJobs && !manager.stopJobs(); completedJobs++){
        // wait for results
        Result result;
        {
            boost::mutex::scoped_lock lock(resultsLock);
            while (resultQueue.size()==0)
                resultsReady.wait(lock);
            
            // get result
            thisJobID= resultQueue.back().first;
            result= resultQueue.back().second;
            resultQueue.pop_back();
        }
        // process result
        manager( thisJobID, result );
    }
    
    // tell all to finish
    if (completedJobs<nJobs){
        for (uint32_t iThread=1; iThread < numThreads; iThread++)
            threadWorkers[iThread-1]->stopJobs_= true;
    }
    
    // wait to finish (they should all be done) and destroy
    for (uint32_t iThread=1; iThread < numThreads; iThread++){
        queue[iThread-1]->join();
        delete queue[iThread-1];
    }
    
    // cleanup the workers
    util::delPointerVector<threadWorker_*>(threadWorkers);
    
    // finalize manager
    manager.finalize();
}

#endif
