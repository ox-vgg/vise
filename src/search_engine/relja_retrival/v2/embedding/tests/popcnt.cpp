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
#include <stdint.h>


int main(){
    
    uint32_t lim= ~static_cast<uint32_t>(0);
    uint64_t sum= 0;
    for (uint32_t j= 0; j<1; ++j)
        for (uint32_t i= 0; i != lim; ++i)
            sum+= __builtin_popcountl(i);
    std::cout<<sum<<"\n";
    
    return 0;
}
