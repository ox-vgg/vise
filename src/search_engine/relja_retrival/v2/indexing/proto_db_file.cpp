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

#include "proto_db_file.h"

#include <algorithm>
#include <stdexcept>
#include <stdio.h>
#include <unistd.h>



uint32_t const protoDbFile::endMark= 0x0FD0FD02;



protoDbFile::protoDbFile( std::string fileName ){
    
    f= fopen( fileName.c_str(), "rb" );
    if (f==NULL)
        throw std::runtime_error( std::string("protoDbFile::protoDbFile: Unable to open file ") + fileName);
    
    // read header and check for corruption (e.g. the file is incomplete because construction halted)
    uint32_t temp_;
    uint32_t headerSize;
    
    fseeko64(f, -2*sizeof(uint32_t), SEEK_END);
    temp_= fread( &headerSize, 1, sizeof(uint32_t), f);
    
    uint32_t endMark;
    temp_= fread( &endMark, 1, sizeof(uint32_t), f);
    if (protoDbFile::endMark!=endMark)
        throw std::runtime_error("protoDbFile::protoDbFile: File is corrupt");
    
    fseeko64(f, -static_cast<int64_t>(2*sizeof(uint32_t)+headerSize), SEEK_END);
    std::string headerStr(headerSize, '\0');
    temp_= fread( &(headerStr[0]), sizeof(char), headerSize, f );
    REMOVE_UNUSED_WARNING(temp_);
    
    // parse header and set numIDs_
    GOOGLE_CHECK( header_.ParseFromString(headerStr) );
    ASSERT(header_.offset_size()>0);
    numIDs_= header_.offset_size()-1;
    
    f_= fileno(f);
}



protoDbFile::~protoDbFile(){
    fclose(f);
}



void
protoDbFile::getData( uint32_t ID, std::vector<std::string> &data ) const {
    
    if (ID>numIDs_ || header_.offset(ID)==header_.offset(ID+1))
        data.clear();
    
    uint32_t temp_;
    uint64_t currOffset= header_.offset(ID), nextOffset= header_.offset(ID+1);
    
    uint32_t dataSize;
    data.clear();
    
    while (currOffset < nextOffset) {
        // read data chunk
        temp_= pread64(f_, &dataSize, sizeof(uint32_t), currOffset);
        currOffset+= sizeof(uint32_t);
        
        data.resize(data.size()+1);
        data.back().resize(dataSize, '\0');
        temp_= pread64(f_, &(data.back()[0]), dataSize*sizeof(char), currOffset);
        currOffset+= dataSize*sizeof(char);
    }
    ASSERT(currOffset==nextOffset);
    
    REMOVE_UNUSED_WARNING(temp_);
}



bool
protoDbFile::contains( uint32_t ID ) const {
    return !( ID>numIDs_ || header_.offset(ID)==header_.offset(ID+1) );
}



protoDbFileBuilder::protoDbFileBuilder( std::string fileName, std::string desc ) : fileName_(fileName), hasBeenClosed_(false) {
    
    header_.set_description(desc);
    
    f_= fopen( fileName.c_str(), "wb" );
    if (f_==NULL)
        throw std::runtime_error( std::string("protoDbFileBuilder::protoDbFileBuilder: Unable to open file ") + fileName);
    
}



void
protoDbFileBuilder::addData( uint32_t ID, std::string const &data ) {
    
    ASSERT(!hasBeenClosed_);
    
    if (ID+1 < static_cast<uint32_t>(header_.offset_size()))
        throw std::runtime_error("protoDbFileBuilder::addData: IDs need to be added in non-descending order");
    
    // add empty stuff and next offset
    for (; static_cast<uint32_t>(header_.offset_size())<=ID; header_.add_offset( ftello64(f_) ) );
    
    uint32_t dataSize= data.length();
    
    fwrite( &dataSize, sizeof(uint32_t), 1, f_ );
    fwrite( data.c_str(), dataSize, sizeof(char), f_ );
    
}



void
protoDbFileBuilder::close(){
    
    if (hasBeenClosed_)
        return;
    
    // add final offset
    header_.add_offset( ftello64(f_) );
    
    // write header and end marker
    std::string headerStr;
    header_.SerializeToString(&headerStr);
    uint32_t headerSize= headerStr.length();
    fwrite( &(headerStr[0]), headerSize, sizeof(char), f_ );
    fwrite( &(headerSize), sizeof(uint32_t), 1, f_ );
    fwrite( &(protoDbFile::endMark), sizeof(uint32_t), 1, f_ );
    fclose(f_ );
    hasBeenClosed_= true;
    
}
