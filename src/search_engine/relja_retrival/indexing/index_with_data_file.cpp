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

#include "index_with_data_file.h"

#include <stdio.h>
#include <stdexcept>
#include <algorithm>



indexWithDataFile::indexWithDataFile( std::string fileName ){
    
    f= fopen( fileName.c_str(), "rb" );
    
    uint32_t temp_;
    
    fseeko64(f, -3*sizeof(uint32_t), SEEK_END);
    temp_= fread( &numIDs_, 1, sizeof(uint32_t), f);
    temp_= fread( &maxVecID, 1, sizeof(uint32_t), f);
    
    uint32_t endMark;
    temp_= fread( &endMark, 1, sizeof(uint32_t), f);
    if (indexWithDataFile::endMark!=endMark)
        throw std::runtime_error("indexWithDataFile::indexWithDataFile: File is corrupt");
    
    fseeko64(f, -static_cast<int64_t>(3*sizeof(uint32_t)+numIDs_*sizeof(uint64_t)), SEEK_END);
    offsets.resize( numIDs_ );
    temp_= fread( &(offsets[0]), sizeof(uint64_t), numIDs_, f );
    
    if (false && temp_) {} // to avoid the warning about not checking temp_
    
    f_= fileno(f);
    
}



indexWithDataFile::~indexWithDataFile(){
    fclose(f);
}



uint32_t
indexWithDataFile::getNumWithID( uint32_t ID ) const {
    if (ID>numIDs_ || offsets[ID]==0)
        return 0;
    uint32_t N;
    uint32_t temp_= pread64(f_, &N, sizeof(uint32_t), offsets[ID]);
    if (false && temp_) {} // to avoid the warning about not checking temp_
    return N;
}



uint32_t
indexWithDataFile::getData( uint32_t ID, std::vector<uint32_t> &vecIDs, unsigned char *&data, uint32_t &size ) const {
    
    if (ID>numIDs_ || offsets[ID]==0){
        vecIDs.clear();
        data= new unsigned char[0];
        size= 0;
        return 0;
    }
    
    uint32_t N, temp_;
    temp_= pread64(f_, &N, sizeof(uint32_t), offsets[ID]);
    temp_= pread64(f_, &size, sizeof(uint32_t), offsets[ID]+sizeof(uint32_t));
    vecIDs.clear(); vecIDs.resize(N);
    temp_= pread64(f_, &(vecIDs[0]), N*sizeof(uint32_t), offsets[ID]+2*sizeof(uint32_t));
    data= new unsigned char[size];
    temp_= pread64(f_, data, size, offsets[ID]+(2+N)*sizeof(uint32_t));
    
    if (false && temp_) {} // to avoid the warning about not checking temp_
    
    return N;
}



indexWithDataFileBuilder::indexWithDataFileBuilder( std::string fileName, std::string desc ) : hasBeenClosed(false) {
    
    if (desc.length()>200){
        throw std::runtime_error("indexWithDataFileBuilder::indexWithDataFileBuilder: Description is limited to 200 characters");
    }
    
    f= fopen( fileName.c_str(), "wb" );
    
    fwrite( desc.c_str(), 1, desc.length()+1, f ); // +1 for the \0 termination
    
    // add zeros to be 200 bytes
    unsigned char zero= 0;
    for (uint32_t i= desc.length()+1; i<200; ++i)
        fwrite( &zero, 1, 1, f);
    
}



void
indexWithDataFileBuilder::addData( uint32_t ID, std::vector<uint32_t> const &vecIDs, unsigned char const *data, uint32_t size ){
    
    if (ID<offsets.size())
        throw std::runtime_error("indexWithDataFileBuilder::addData: IDs need to be added in ascending order");
    
    // add empty stuff
    for (; offsets.size()<ID; offsets.push_back(0) );
    
    uint32_t N= vecIDs.size();
    
    if (N==0)
        offsets.push_back(0);
    else {
        
        offsets.push_back( ftello64(f) );
        
        fwrite( &N, sizeof(uint32_t), 1, f );
        fwrite( &size, sizeof(uint32_t), 1, f );
        fwrite( &(vecIDs[0]), sizeof(uint32_t), N, f );
        fwrite( data, 1, size, f );
        
        maxVecID= std::max( maxVecID, *(std::max_element(vecIDs.begin(), vecIDs.end())) );
        
    }
    
}



void
indexWithDataFileBuilder::close(){
    
    if (hasBeenClosed)
        return;
    
    uint32_t numIDs_= offsets.size();
    fwrite( &(offsets[0]), 1, sizeof(uint64_t)*numIDs_, f );
    fwrite( &numIDs_, 1, sizeof(uint32_t), f );
    fwrite( &maxVecID, 1, sizeof(uint32_t), f );
    fwrite( &(indexWithDataFile::endMark), 1, sizeof(uint32_t), f );
    fclose(f);
    hasBeenClosed= true;
    
}
