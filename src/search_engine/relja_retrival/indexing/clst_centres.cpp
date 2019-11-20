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

#include "clst_centres.h"

#include <fstream>
#include <iostream>
#include <stdexcept>



clstCentres::clstCentres( const char fileName[], bool flat ){
    
    std::ifstream clstF( fileName, std::ios::in | std::ios::binary);
    if (!clstF.is_open()){
        std::cout<<fileName<<"\n";
        throw std::runtime_error("Unable to open cluster centre file, does it exist?");
    }
    
    clstF.seekg(0, std::ios::end);
    uint32_t fileSize= clstF.tellg();
    clstF.seekg(0, std::ios::beg);
    
    unsigned char dtypeCode;
    clstF.read( (char*)&dtypeCode, sizeof(dtypeCode) );
    clstF.read( (char*)&numClst, sizeof(numClst) );
    clstF.read( (char*)&numDims, sizeof(numDims) );
    const uint32_t clstHeaderSize= 1 + 4*2 + 5*4 + 4;
    
    if ( fileSize != numClst * numDims * sizeof(float) + clstHeaderSize ){
        std::cout<<fileName<<"\n";
        throw std::runtime_error("Invalid cluster centre file, header and file size contradict each other (did you give me the .h5 file instead of .e3bin)?");
    }
    
    if (dtypeCode!=4){
        throw std::runtime_error("Header states the underylying cluster type is not float - only float is supported");
    }
    
    clstF.seekg(clstHeaderSize, std::ios::beg);
    
    if (flat){
        
        clstC= NULL;
        clstC_flat= new float[ numClst*numDims ];
        clstF.read( (char*)clstC_flat, sizeof(float)*numDims*numClst );
        
    } else {
        
        clstC_flat= NULL;
        clstC= new float*[ numClst ];
        for (uint32_t iC= 0; iC<numClst; ++iC){
            clstC[iC]= new float[numDims];
            clstF.read( (char*)clstC[iC], sizeof(float)*numDims );
        }
        
    }
    
    clstF.close();
    
}



clstCentres::~clstCentres(){
    
    if (clstC!=NULL){
        
        for (uint32_t iC= 0; iC<numClst; ++iC)
            delete []clstC[iC];
        delete []clstC;
    
    }
    
    if (clstC_flat!=NULL){
        delete []clstC_flat;
    }
    
}
