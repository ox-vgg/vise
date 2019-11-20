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

#ifndef _PROTO_DB_FILE_H_
#define _PROTO_DB_FILE_H_


#include <fstream>
#include <stdint.h>
#include <string>
#include <vector>

#include "proto_db_header.pb.h"
#include "macros.h"
#include "proto_db.h"



/*
Organization:

for ID= 0 : numIDs-1  (apart from ones where nDatas=0)
    for iData= 0 : nDatas-1
        uint32_t entrySize
        std::string data
protoDbHeader (contains offsets for random access of ID entries above)
uint32_t protoDbHeaderSize
0x0FD0FD02 (to make sure the file is not currupt)
*/

class protoDbFile : public protoDb {
    
    public:
        
        protoDbFile( std::string fileName );
        
        ~protoDbFile();
        
        inline uint32_t
            numIDs() const { return numIDs_; }
        
        void
            getData( uint32_t ID, std::vector<std::string> &data ) const;
        
        bool
            contains( uint32_t ID ) const;
        
        static uint32_t const endMark;
    
    private:
        
        uint32_t numIDs_;
        rr::protoDbHeader header_;
        FILE *f;
        int f_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoDbFile)
};




class protoDbFileBuilder : public protoDbBuilder {
    
    public:
        
        protoDbFileBuilder( std::string fileName, std::string desc="" );
        
        ~protoDbFileBuilder(){
            if (!hasBeenClosed_)
                close();
        }
        
        void
            addData( uint32_t ID, std::string const &data );
        
        void
            close();
        
        inline bool
            hasBeenClosed() const { return hasBeenClosed_; }
        
        inline std::string getFn() const { return fileName_; }
    
    private:
        
        std::string fileName_;
        bool hasBeenClosed_;
        rr::protoDbHeader header_;
        FILE *f_;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(protoDbFileBuilder)
};


#endif
