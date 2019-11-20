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

#include "thread_queue.h"

#include <math.h>
#include <iostream>

void isPrime( uint32_t x, bool &result ){
    if (x==0 || x==1){
        result= false;
        return;
    }
    for (int i=2; i <= sqrt(x); ++i)
        if (x%i==0){
            result= false;
            return;
        }
    result= true;
}

static uint32_t count= 0, numc= 0;;

void printPrimes( uint32_t x, bool &result ){
    if (result){
//         std::cout<< x <<"\n";
        ++count;
    }
    ++numc;
}

class worker : public queueWorker<bool> {
    public:
        inline void operator() ( uint32_t jobID, bool &result ) const {
            isPrime(jobID, result);
        }
};

class manager : public queueManager<bool> {
    public:
        inline void operator() ( uint32_t jobID, bool &result ){
            printPrimes(jobID, result);
        }
};

void threadQueue_test(){
    
    worker worker_obj;
    manager manager_obj;
    
    threadQueue<bool>::start(
        100000, //3000000,
        worker_obj, manager_obj,
        4
    );
    
    std::cout<<count<<" "<<numc<<"\n";
    
}
