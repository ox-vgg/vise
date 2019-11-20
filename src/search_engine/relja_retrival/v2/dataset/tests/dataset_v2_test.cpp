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

#include "dataset_v2.h"


int main(){
    
    datasetBuilder dB("/home/relja/Relja/Temp/dset.v2bin");
    dB.add( "relja", 100, 20);
    dB.add( "marianna", 120, 50);
    dB.add( "relja2", 120, 50);
    dB.add( "relja3", 120, 50);
    dB.add( "relja4", 120, 50);
    dB.close();
    
    datasetV2 d("/home/relja/Relja/Temp/dset.v2bin");
    std::cout<< d.getFn(0) <<"\n";
    std::cout<< d.getDocID("relja") <<"\n";
    
    datasetV2 df("/home/relja/Relja/Data/tmp/indexing_v2/indexing_mini/dset_oxMini20.v2bin");
//     datasetV2 df("/home/relja/Relja/Data/tmp/indexing_v2/indexing_ox5k/dset_oxc1_5k.v2bin");
    std::cout<< df.getFn(0) <<"\n";
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
