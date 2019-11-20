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

#ifndef _DATASET_V2_H_
#define _DATASET_V2_H_

#include <limits>
#include <map>
#include <stdint.h>
#include <string>

#include <boost/functional/hash.hpp>

#include "dataset_abs.h"
#include "dataset_entry.pb.h"
#include "macros.h"
#include "proto_db_file.h"



class datasetV2 : public datasetAbs {
    
    public:
        
        datasetV2( std::string fileName,
                   std::string addPrefix= "",
                   std::string removePrefix= "");
        
        ~datasetV2();
        
        inline uint32_t
            getNumDoc() const {
                return numDocs_;
            }
        
        std::string
            getFn( uint32_t docID ) const;
        
        std::string
            getInternalFn( uint32_t docID ) const;
        
        std::pair<uint32_t, uint32_t>
            getWidthHeight( uint32_t docID ) const;
        
        uint32_t
            getDocID( std::string fn ) const;
        
        uint32_t
            getDocIDFromAbsFn( std::string fn ) const;
        
        bool
            containsFn( std::string fn ) const;
    
    private:
        
        // NULL if not found, if not NULL caller should NOT delete
        uint32_t const *
            lookupFn(std::string const &fn) const;
        
        protoDbInRam *db_;
        uint32_t addPrefixFullLen_, removePrefixLen_, numIDs_, numDocs_;
        std::string const addPrefix_, removePrefix_;
        
        uint32_t numPerID_;
        
        bool memoryEffLookup_;
        std::map<std::string, uint32_t> fn2ID_;
        std::vector<uint32_t> hashTable_;
        boost::hash<std::string> hashFunc_;
        static uint32_t const emptyEntry_;

/*    
    private:
        DISALLOW_COPY_AND_ASSIGN(datasetV2)
*/
};



class datasetBuilder {
    
    public:
        
        datasetBuilder(std::string fileName);
        
        ~datasetBuilder(){
            if (!dbBuilder_.hasBeenClosed())
                close();
        }
        
        void
            close();
        
        void
            add(std::string fn, uint32_t width, uint32_t height);
        
    private:
        
        void
            save(bool isLast= false);
        
        bool hasBeenClosed_;
        protoDbFileBuilder dbBuilder_;
        rr::datasetEntry buffer_;
        uint32_t entriesID_;
        static uint32_t const numPerID_= 1000;
    
    private:
        DISALLOW_COPY_AND_ASSIGN(datasetBuilder)
};

#endif
