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

#ifndef _INDEX_WITH_DATA_FILE_FIXED1_H_
#define _INDEX_WITH_DATA_FILE_FIXED1_H_


#include <stdint.h>
#include <vector>
#include <string>

#include "index_with_data.h"

/*
Organization:
200 chars for textual description
for ID=0 : numIDs-1
    uint32_t vecID
    unsigned char data[size]
uint32_t numIDs
uint32_t maxVecID
uint32_t size
0x1FD0FDFD (to make sure the file is not currupt)
*/



class indexWithDataFileFixed1 : public indexWithData {
    
    public:
        
        indexWithDataFileFixed1( std::string fileName );
        
        ~indexWithDataFileFixed1();
        
        uint32_t
            numIDs() const { return numIDs_; }
        
        uint32_t
            getNumWithID( uint32_t ID ) const;
        
        uint32_t
            getData( uint32_t ID, std::vector<uint32_t> &vecIDs, unsigned char *&data, uint32_t &size ) const;
        
        static uint32_t const endMark;
    
    private:
        
        uint32_t numIDs_, maxVecID, size_;
        FILE *f;
        int f_;
        
    
};

uint32_t const indexWithDataFileFixed1::endMark= 0x1FD0FDFD;



class indexWithDataFileFixed1Builder : public indexWithDataBuilder {
    
    public:
        
        indexWithDataFileFixed1Builder( std::string fileName, uint32_t aSize, std::string desc="" );
        
        ~indexWithDataFileFixed1Builder(){
            if (!hasBeenClosed)
                close();
        }
        
        void
            addData( uint32_t ID, std::vector<uint32_t> const &vecIDs, unsigned char const *data, uint32_t size );
        
        void
            close();
    
    private:
        
        bool hasBeenClosed;
        uint32_t numIDs, size_;
        FILE *f;
    
};

// TODO indexWithDataFileFixed1 inRam

#endif
