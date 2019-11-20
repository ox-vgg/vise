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

#include "index_with_data_file_fixed1.h"

#include <stdio.h>
#include <stdexcept>
#include <algorithm>



indexWithDataFileFixed1::indexWithDataFileFixed1( std::string fileName ){
    
    f= fopen( fileName.c_str(), "rb" );
    
    uint32_t temp_;
    
    fseeko64(f, -4*sizeof(uint32_t), SEEK_END);
    temp_= fread( &numIDs_, 1, sizeof(uint32_t), f);
    temp_= fread( &maxVecID, 1, sizeof(uint32_t), f);
    temp_= fread( &size_, 1, sizeof(uint32_t), f);
    
    uint32_t endMark;
    temp_= fread( &endMark, 1, sizeof(uint32_t), f);
    if (indexWithDataFileFixed1::endMark!=endMark)
        throw std::runtime_error("indexWithDataFileFixed1::indexWithDataFileFixed1: File is corrupt");
    
    if (false && temp_) {} // to avoid the warning about not checking temp_
    
    f_= fileno(f);
    
}



indexWithDataFileFixed1::~indexWithDataFileFixed1(){
    fclose(f);
}



uint32_t
indexWithDataFileFixed1::getNumWithID( uint32_t ID ) const {
    if (ID>numIDs_)
        return 0;
    return 1;
}



uint32_t
indexWithDataFileFixed1::getData( uint32_t ID, std::vector<uint32_t> &vecIDs, unsigned char *&data, uint32_t &size ) const {
    
    if (ID>numIDs_){
        vecIDs.clear();
        data= new unsigned char[0];
        size= 0;
        return 0;
    }
    
    uint64_t offset= static_cast<uint64_t>(ID)*(sizeof(uint32_t)+size_) + 200;
    uint32_t temp_;
    size= size_;
    vecIDs.clear(); vecIDs.resize(1);
    temp_= pread64(f_, &(vecIDs[0]), sizeof(uint32_t), offset);
    data= new unsigned char[size];
    temp_= pread64(f_, data, size, offset+sizeof(uint32_t));
    
    if (false && temp_) {} // to avoid the warning about not checking temp_
    
    return 1;
}



indexWithDataFileFixed1Builder::indexWithDataFileFixed1Builder( std::string fileName, uint32_t aSize, std::string desc ) : hasBeenClosed(false), numIDs(0), size_(aSize) {
    
    if (desc.length()>200){
        throw std::runtime_error("indexWithDataFileFixed1Builder::indexWithDataFileFixed1Builder: Description is limited to 200 characters");
    }
    
    f= fopen( fileName.c_str(), "wb" );
    
    fwrite( desc.c_str(), 1, desc.length()+1, f ); // +1 for the \0 termination
    
    // add zeros to be 200 bytes
    unsigned char zero= 0;
    for (uint32_t i= desc.length()+1; i<200; ++i)
        fwrite( &zero, 1, 1, f);
    
};



void
indexWithDataFileFixed1Builder::addData( uint32_t ID, std::vector<uint32_t> const &vecIDs, unsigned char const *data, uint32_t size ){
    
    if (ID<numIDs)
        throw std::runtime_error("indexWithDataFileFixed1Builder::addData: IDs need to be added in ascending order");
    
    uint32_t N= vecIDs.size();
    if (N>1)
        throw std::runtime_error("indexWithDataFileFixed1Builder::addData: number of vecIDs needs to be 0 or 1");
    
    if (size!=size_)
        throw std::runtime_error("indexWithDataFileFixed1Builder::addData: data size needs to be the same as given to the constructor");
    
    // add empty stuff
    {
        uint32_t vecID= 0;
        unsigned char *data_= NULL;
        uint32_t newNumIDs= ID + (N==0);
        if (numIDs < newNumIDs){
            std::cout<<"\nindexWithDataFileFixed1Builder::addData: Warning: not supposed to have blank entries!\n\n";
            data_= new unsigned char [size];
            for (uint32_t i= 0; i<size; ++i) data_[i]= 0;
        }
        for (; numIDs < newNumIDs; ++numIDs){
            fwrite(&vecID, sizeof(uint32_t), 1, f);
            fwrite(data_, 1, size, f);
        }
        if (data_!=NULL)
            delete []data_;
    }
    
    if (N!=0){
        fwrite( &(vecIDs[0]), sizeof(uint32_t), 1, f );
        fwrite(data, 1, size, f);
        maxVecID= std::max( maxVecID, vecIDs[0] );
        ++numIDs;
    }
    
}



void
indexWithDataFileFixed1Builder::close(){
    
    if (hasBeenClosed)
        return;
    
    fwrite( &numIDs, 1, sizeof(uint32_t), f );
    fwrite( &maxVecID, 1, sizeof(uint32_t), f );
    fwrite( &size_, 1, sizeof(uint32_t), f );
    fwrite( &(indexWithDataFileFixed1::endMark), 1, sizeof(uint32_t), f );
    fclose(f);
    hasBeenClosed= true;
    
}
