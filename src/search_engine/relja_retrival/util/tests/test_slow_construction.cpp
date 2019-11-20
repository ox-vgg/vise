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

#include "slow_construction.h"

#include <iostream>
#include <unistd.h>

#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>



class timedConstr {
    public:
        timedConstr(int val) : val_(val) {
            sleep(val);
        }
        int getVal(){ return val_; }
    private:
        int val_;
};



int main(){
    
    boost::function<timedConstr*()> xConstructor= boost::lambda::bind( boost::lambda::new_ptr<timedConstr>(), boost::lambda::make_const(2) );
    
    if (true) {
        std::cout<<"First\n";
        slowConstruction<timedConstr> slowCons(new timedConstr(0), xConstructor, true);
        double dt= 0.1;
        for (double t=0; t<3; t+=dt){
            std::cout<<t<<" "<<slowCons.getObject()->getVal()<<"\n";
            usleep(dt * 1000 * 1000);
        }
    }
    
    if (true) {
        std::cout<<"First\n";
        sequentialConstructions consQueue;
        slowConstruction<timedConstr> slowCons(new timedConstr(0), xConstructor, true, &consQueue);
        slowConstruction<timedConstr> slowCons2(new timedConstr(0), xConstructor, true, &consQueue);
        consQueue.start();
        double dt= 0.1;
        for (double t=0; t<5; t+=dt){
            std::cout<<t<<" "<<slowCons.getObject()->getVal()<<" "<<slowCons2.getObject()->getVal()<<"\n";
            usleep(dt * 1000 * 1000);
        }
    }
    
    if (true) {
        std::cout<<"Second\n";
        slowConstruction<timedConstr> slowCons(new timedConstr(0), xConstructor, true);
    }
    
    return 0;
}
