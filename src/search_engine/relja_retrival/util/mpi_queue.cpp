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

#include "mpi_queue.h"

#include <math.h>
#include <iostream>

void isPrime( uint32_t x, bool &result ){
    if (x==0 || x==1){
        result= false;
        return;
    }
    uint32_t lim= sqrt(x)+2;
    if (lim>=x) lim= x-1;
    for (uint32_t i=2; i <= lim; ++i)
        if (x%i==0){
            result= false;
            return;
        }
    result= true;
}

class worker : public queueWorker<bool> {
    public:
        inline void operator() ( uint32_t jobID, bool &result ) const {
            isPrime(jobID, result);
        }
};

class manager : public queueManager<bool> {
    public:
        manager(uint32_t nJobs) : rec_(nJobs,false), count_(0), numc_(0) {}
        inline void operator() ( uint32_t jobID, bool &result ){
            ++numc_;
            count_+= result;
            rec_[jobID]= result;
        }
        std::vector<bool> rec_;
        uint32_t count_, numc_;
};

void mpiQueue_test(){
    
    MPI_GLOBAL_RANK;
    
    uint32_t nJobs= 100000;
    worker worker_obj;
    manager *manager_obj= (rank==0) ?
        new manager(nJobs) :
        NULL;
    
    mpiQueue<bool>::start(
        nJobs,
        worker_obj, manager_obj
    );
    
    if (manager_obj->count_>0){
        // for (uint32_t i= 0; i < manager_obj.numc_; ++i)
        //     if (manager_obj->rec_[i])
        //         std::cout<<i<<"\n";
        
        // std::cout<<"\n";
        std::cout<<manager_obj->count_<<" "<<manager_obj->numc_<<"\n";
    }
    
    if (rank==0) delete manager_obj;
    
}
