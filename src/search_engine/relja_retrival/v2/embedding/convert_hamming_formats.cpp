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

#include "hamming_embedder.h"



int main(int argc, char* argv[]){
    
    ASSERT(argc==3);
    
    std::string inFn= argv[1];
    std::string outFn= argv[2];
    
    hammingEmbedderFactory::convertFormats(inFn, outFn);
    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
