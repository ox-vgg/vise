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

#ifndef _INDEX_WITH_DATA_FILE_H_
#define _INDEX_WITH_DATA_FILE_H_


#include <stdint.h>
#include <vector>
#include <string>

#include "index_with_data.h"

/*
Organization:
200 chars for textual description
for ID=0 : numIDs-1  (apart from ones where N=0)
    uint32_t N (number of vectors with this ID)
    uint32_t size (size of data)
    uint32_t vecIDs[N]
    unsigned char data[size]
uint32_t offsets[numID] (marks byte position of each above item, 0 if no vecIDs have the ID)
uint32_t numIDs
uint32_t maxVecID
0x0FD0FDFD (to make sure the file is not currupt)
*/



class indexWithDataFile : public indexWithData {
    
    public:
        
        indexWithDataFile( std::string fileName );
        
        ~indexWithDataFile();
        
        uint32_t
            numIDs() const { return numIDs_; }
        
        uint32_t
            getNumWithID( uint32_t ID ) const;
        
        uint32_t
            getData( uint32_t ID, std::vector<uint32_t> &vecIDs, unsigned char *&data, uint32_t &size ) const;
        
        static uint32_t const endMark;
    
    private:
        
        uint32_t numIDs_, maxVecID;
        std::vector<uint64_t> offsets;
        FILE *f;
        int f_;
        
    
};

uint32_t const indexWithDataFile::endMark= 0x0FD0FDFD;



class indexWithDataFileBuilder : public indexWithDataBuilder {
    
    public:
        
        indexWithDataFileBuilder( std::string fileName, std::string desc="" );
        
        ~indexWithDataFileBuilder(){
            if (!hasBeenClosed)
                close();
        }
        
        void
            addData( uint32_t ID, std::vector<uint32_t> const &vecIDs, unsigned char const *data, uint32_t size );
        
        void
            close();
    
    private:
        
        bool hasBeenClosed;
        std::vector<uint64_t> offsets;
        FILE *f;
    
};

#endif
