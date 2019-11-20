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

#include "median_computer.h"

#include <iostream>
#include <math.h>

#include "macros.h"

void expected(float val, float exp){
    std::cout<<val<<" (exp: "<<exp<<")\n";
    ASSERT(fabs(val-exp)<0.1);
}

int main(){
    
    {
        medianComputer mc;
        mc.add(1);
        mc.add(2);
        mc.add(3);
        expected( mc.getMedian(), 2 );
        mc.add(4);
        expected( mc.getMedian(), 2.5 );
        mc.add(5);
        expected( mc.getMedian(), 3 );
        mc.add(6);
        expected( mc.getMedian(), 3.5 );
    }
    
    {
        medianComputer mc;
        float vs[4]={10, 11, 20, 19};
        for (int v= 0; v<4; ++v){
            for (int i= 0; i<100; ++i)
                mc.add(vs[v]);
        }
        expected( mc.getMedian(), 19 );
        mc.add(13);
        expected( mc.getMedian(), 13 );
    }
    
    {
        medianComputer mc;
        for (int i= 0; i<300; ++i)
            mc.add(10);
        expected( mc.getMedian(), 10 );
    }
    
    std::cout<<"\nAll OK\n";
    
    return 0;
}
