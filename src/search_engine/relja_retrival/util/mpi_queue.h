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

#ifndef _MPI_QUEUE_H_
#define _MPI_QUEUE_H_

#include <stdint.h>
#include <vector>
#include <map>
#include <iostream>

#ifdef RR_MPI
#include <boost/mpi.hpp>
#endif

#include "util.h"
#include "macros.h"
#include "par_queue.h"


#ifndef RR_MPI

#define MPI_INIT_ENV
#define MPI_GLOBAL_COMM
#define MPI_GLOBAL_ALL  uint32_t rank= 0, numProc= 1;
#define MPI_GLOBAL_RANK uint32_t rank= 0;
#define MPI_GLOBAL_NPRC uint32_t numProc= 0;

#else

#define MPI_INIT_ENV boost::mpi::environment env(argc, argv);
#define MPI_GLOBAL_COMM boost::mpi::communicator comm; uint32_t rank= comm.rank();
#define MPI_GLOBAL_ALL  boost::mpi::communicator comm; uint32_t rank= comm.rank(), numProc= comm.size();
#define MPI_GLOBAL_RANK boost::mpi::communicator comm; uint32_t rank= comm.rank();
#define MPI_GLOBAL_NPRC boost::mpi::communicator comm; uint32_t numProc= comm.size();

#endif



template <class Result>
class queueWorker;
template <class Result>
class queueManager;
template <typename Result>
void serialQueue( uint32_t nJobs, queueWorker<Result> const &worker, queueManager<Result> &manager );


template <class Result>
class mpiQueue {
    
    public:
        
        static void
            start( uint32_t nJobs,
                   queueWorker<Result> const &worker,
                   queueManager<Result> *manager,
                   uint32_t dynamicThresh= 4,
                   uint32_t rootRank= 0
                   );
    
    private:
        mpiQueue(){}
        DISALLOW_COPY_AND_ASSIGN(mpiQueue);
};

void mpiQueue_test();



template <class Result>
void
mpiQueue<Result>::start(
      uint32_t nJobs,
      queueWorker<Result> const &worker,
      queueManager<Result> *manager,
      uint32_t dynamicThresh,
      uint32_t rootRank
      ){
    
#ifndef RR_MPI
    std::cout<<"\n\n\tWarning: trying to execute MPI queue, using serial queue instead\n\n";
    ASSERT(manager!=NULL);
    serialQueue<Result>(nJobs, worker, *manager);
    return;
#else
    
    MPI_GLOBAL_ALL
    
    if (numProc == 1) {
        // actually don't work in parallel as we only have one guy working
        ASSERT(manager!=NULL);
        serialQueue<Result>(nJobs, worker, *manager);
        return;
    }
    
    uint32_t jobID= 0;
        
    
    if ( rank != rootRank ){
        
        // worker
        ASSERT(manager==NULL);
        
        bool stop;
        
        while (true) {
            
            comm.recv( rootRank, tagQueueStopGo, stop );
            
            if (stop)
                break;
            
            comm.recv( rootRank, tagQueueJob, jobID );
            Result result;
            worker( jobID, result );
            comm.send( rootRank, tagQueueResult, result );
            
        }
        
    } else {
        
        // queue manager
        ASSERT(manager!=NULL);
        uint32_t iProc;
    
        if ( numProc < dynamicThresh) {
            
            // static queue - manager does work too
            
            uint32_t numSent= 0;
            uint32_t *procJobIDs= new uint32_t[numProc];
            
            // go through jobs
            
            while ( jobID < nJobs && !manager->stopJobs() ) {
                
                // send jobs
                numSent= 0;
                for (iProc= 0; (iProc < numProc) && (jobID < nJobs); ++iProc)
                    if (iProc!=rootRank){
                        comm.send( iProc, tagQueueStopGo, false );
                        comm.send( iProc, tagQueueJob, jobID );
                        procJobIDs[ iProc ]= jobID;
                        ++jobID;
                        ++numSent;
                    }
                
                // do job on root
                if (jobID < nJobs){
                    Result result;
                    if (!manager->stopJobs()){
                        worker( jobID, result );
                        (*manager)( jobID, result );
                    }
                    ++jobID;
                }
                
                // get results and process them
                for (iProc= 0; (iProc < numProc) && (numSent > 0); ++iProc)
                    if (iProc!=rootRank){
                        Result result;
                        comm.recv( iProc, tagQueueResult, result );
                        if (!manager->stopJobs())
                            (*manager)( procJobIDs[iProc], result );
                        --numSent;
                    }
                
            }
            
            delete []procJobIDs;
            
            // tell everyone to stop
            
            for (iProc= 0; iProc < numProc; ++iProc)
                if (iProc!=rootRank)
                    comm.send( iProc, tagQueueStopGo, true );
            
        } else {
            
            // dynamic queue - just control stuff
            
            jobID= 0;
            uint32_t recJobID= 0;
            std::map<uint32_t,uint32_t> procJobIDs;
            
            // send a first batch of jobs
            for (iProc= 0; jobID < nJobs && iProc < numProc; ++iProc)
                if (iProc!=rootRank){
                    comm.send( iProc, tagQueueStopGo, false );
                    comm.send( iProc, tagQueueJob, jobID );
                    procJobIDs[ iProc ]= jobID;
                    ++jobID;
                }
            
            // if less jobs than processors - tell the others to stop
            for (; iProc < numProc; ++iProc)
                if (iProc!=rootRank)
                    comm.send( iProc, tagQueueStopGo, true );
            
            // now wait for results
            boost::mpi::status stat;
            while ( procJobIDs.size() ){
                Result result;
                stat= comm.recv( boost::mpi::any_source, tagQueueResult, result );
                iProc= stat.source();
                recJobID= procJobIDs[ iProc ];
                if (jobID < nJobs && !manager->stopJobs()){
                    // send a new job
                    comm.send( iProc, tagQueueStopGo, false );
                    comm.send( iProc, tagQueueJob, jobID );
                    procJobIDs[ iProc ]= jobID;
                    ++jobID;
                } else {
                    // out of jobs - tell him to stop
                    comm.send( iProc, tagQueueStopGo, true );
                    procJobIDs.erase( iProc );
                }
                if (!manager->stopJobs())
                    (*manager)( recJobID, result );
            }
            
        }
        
        manager->finalize();
        
    }
    
    comm.barrier();
    
#endif
    
}



#endif
