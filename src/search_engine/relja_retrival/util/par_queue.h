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

#ifndef _PAR_QUEUE_H_
#define _PAR_QUEUE_H_



#ifdef RR_MPI
#include <boost/mpi.hpp>
#include "mpi_queue.h"
#else
#define MPI_INIT_ENV
#endif

#include "macros.h"
#include "timing.h"
#include "thread_queue.h"


#ifdef RR_MPI
template <class Result>
class mpiQueue;
#endif

template <class Result>
class threadQueue;



template <class Result>
class queueWorker {
    public:
        queueWorker(){}
        virtual void operator() ( uint32_t jobID, Result &result ) const =0;
        virtual ~queueWorker() {}
    private: DISALLOW_COPY_AND_ASSIGN(queueWorker)
};



template <class Result>
class queueManager {
    public:
        queueManager() : stopJobs_(false) {}
        virtual void operator() ( uint32_t jobID, Result &result ) {}
        virtual void finalize() {};
        virtual ~queueManager() {}
        inline bool stopJobs() const { return stopJobs_; }
    protected:
        bool stopJobs_;
    private: DISALLOW_COPY_AND_ASSIGN(queueManager)
};



template <class Result>
class managerWithTiming : public queueManager<Result> {
    
    public:
        
        managerWithTiming(uint32_t nJobs, std::string prefix) :
            progressPrint_(nJobs, prefix) {}
        
        virtual ~managerWithTiming() {}
        
        void
            operator() ( uint32_t jobID, Result &result ){
                compute(jobID, result);
                progressPrint_.inc();
            }
        
        virtual void
            compute( uint32_t jobID, Result &result ) {}
        
    private:
        timing::progressPrint progressPrint_;
        DISALLOW_COPY_AND_ASSIGN(managerWithTiming)
};



template <typename Result>
void serialQueue( uint32_t nJobs, queueWorker<Result> const &worker, queueManager<Result> &manager ){
    for (uint32_t jobID=0; jobID < nJobs && !manager.stopJobs(); ++jobID){
        Result result;
        worker( jobID, result );
        manager( jobID, result );
    }
    manager.finalize();
}



inline bool
detectUseThreads() {
    #ifndef RR_MPI
        return true;
    #else
        boost::mpi::communicator comm;
        return (comm.size()<=1);
    #endif
}

template <class Result>
class parQueue {
    
    public:
        
        static void
            startStatic( uint32_t nJobs,
                   queueWorker<Result> const &worker,
                   queueManager<Result> *manager,
                   bool useThreads= false,
                   uint32_t numWorkerThreads= 4 ){
                
            if (useThreads){
                
                if (numWorkerThreads<=0)
                    numWorkerThreads= 1;
                
                ASSERT(manager!=NULL);
                threadQueue<Result>::start( nJobs, worker, *manager, numWorkerThreads );
                
            } else {
                
                #ifdef RR_MPI
                mpiQueue<Result>::start( nJobs, worker, manager );
                #else
                throw std::runtime_error("Compiled without MPI support!");
                #endif
                
            }
            
        }
        
        
        static void
            startAutodetectType( uint32_t nJobs,
                             queueWorker<Result> const &worker,
                             queueManager<Result> *manager,
                             uint32_t numWorkerThreads= 4 ){
                parQueue queue(numWorkerThreads);
                queue.start(nJobs, worker, manager);
            }
        
        
        // autodetect if in MPI environement and MPI is enabled
        parQueue( uint32_t aNumWorkerThreads= 4 ) : numWorkerThreads(aNumWorkerThreads) {
            useThreads= detectUseThreads();
            if (numWorkerThreads<=0)
                numWorkerThreads= 1;
        }
        
        
        
        parQueue( bool aUseThreads= false, uint32_t aNumWorkerThreads= 4 ) : useThreads(aUseThreads), numWorkerThreads(aNumWorkerThreads) {
            
            if (!useThreads){
                #ifndef RR_MPI
                    rank= 0;
                #else
                    boost::mpi::communicator comm;
                    rank= comm.rank();
                #endif
            } else {
                rank= 0;
                if (numWorkerThreads<=0)
                    numWorkerThreads= 1;
            }
            
        }
        
        
        
        void
            start( uint32_t nJobs,
                   queueWorker<Result> const &worker,
                   queueManager<Result> *manager ) const {
                parQueue<Result>::startStatic( nJobs, worker, manager, useThreads, numWorkerThreads );
            }
        
        
        
        inline uint32_t
            getRank() const { return rank; }
        
        
    private:
        
        bool useThreads;
        uint32_t numWorkerThreads;
        uint32_t rank;
        DISALLOW_COPY_AND_ASSIGN(parQueue)
    
};

#endif
