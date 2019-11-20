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

#include <fstream>

#include "dataset_v2.h"
#include "util.h"



int main(){
    
    datasetBuilder dB( util::expandUser("~/Relja/Data/BBC/BBCb/vlad/dset_BBCb.v2bin") );
    std::ifstream flist( util::expandUser("~/Relja/Databases/BBC/BBCb/keyframe_list.txt").c_str() );
//     datasetBuilder dB( util::expandUser("~/Relja/Data/BBC/BBCc/dset_BBCc.v2bin") );
//     std::ifstream flist( util::expandUser("~/Relja/Databases/BBC/BBCc/keyframe_lists/keyframes.txt").c_str() );
    
    std::string imageFn;
    uint32_t docID= 0;
    while (std::getline(flist, imageFn)){
        ++docID;
        if (docID%1000000==0) std::cout<<docID<<"\n";
        dB.add( imageFn, 480, 270 );
    }
    flist.close();
    dB.close();
    
    google::protobuf::ShutdownProtobufLibrary();
}
