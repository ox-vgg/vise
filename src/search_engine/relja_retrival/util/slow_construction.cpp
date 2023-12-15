/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

*/

#include "slow_construction.h"

#include <iostream>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include <boost/lambda/bind.hpp>

#include "macros.h"



sequentialConstructions::~sequentialConstructions(){
    t_->join();
    delete t_;
    for (uint32_t i= 0; i<cleanups_.size(); ++i)
        cleanups_[i]();
}

void
sequentialConstructions::addFunction(boost::function<void()> f){
    ASSERT(!started);
    fs_.push_back(f);
}

void
sequentialConstructions::addCleanup(boost::function<void()> f){
    ASSERT(!started);
    cleanups_.push_back(f);
}

void
sequentialConstructions::start(){
    started= true;
    t_= new boost::thread(boost::bind(&sequentialConstructions::reallyStart, this));
}

void
sequentialConstructions::reallyStart(){
    for (uint32_t i= 0; i<fs_.size(); ++i)
        fs_[i]();
}
