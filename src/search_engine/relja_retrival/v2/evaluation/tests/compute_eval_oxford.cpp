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

#include <stdint.h>
#include <string>
#include <vector>

#include "evaluator_v2.h"
#include "util.h"



int main(){
    
    evaluatorV2( util::expandUser("~/Relja/Data/gt"), "Oxford", NULL );
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
