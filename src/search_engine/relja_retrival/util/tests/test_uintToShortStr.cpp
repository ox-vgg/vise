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

#include <iostream>
#include <math.h>

#include "util.h"



int main(){
    
    std::vector<uint32_t> values;
    values.push_back(0);
    values.push_back(50);
    values.push_back(103);
    values.push_back(999);
    values.push_back(1000);
    values.push_back(1001);
    values.push_back(1024);
    values.push_back(1099);
    values.push_back(99999);
    values.push_back(100000);
    values.push_back(200000);
    values.push_back(220000);
    values.push_back(999999);
    values.push_back(1000000);
    values.push_back(3000000);
    values.push_back(3490000);
    
    for (uint32_t i= 0; i<values.size(); ++i)
        std::cout<< values[i] <<" "<< util::uintToShortStr(values[i]) <<"\n";;
    
    return 0;
}
