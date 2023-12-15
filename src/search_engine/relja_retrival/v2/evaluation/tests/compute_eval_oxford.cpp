/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

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
