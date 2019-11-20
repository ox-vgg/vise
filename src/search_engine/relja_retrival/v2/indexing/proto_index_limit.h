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

#ifndef _PROTO_INDEX_LIMIT_H_
#define _PROTO_INDEX_LIMIT_H_


#include <iostream>
#include <stdint.h>
#include <string>
#include <vector>

#include "macros.h"
#include "proto_index.h"



class protoIndexLimit : public protoIndex {
    
    public:
        
        protoIndexLimit(uint64_t limit, protoDb const &db, bool precompQuantEl= true);
        
        inline uint32_t
            getNumWithID( uint32_t ID ) const {
                if (loading_)
                    return protoIndex::getNumWithID(ID);
                return ID>numIDs_ ? 0 : N_[ID];
            }
        
        uint32_t
            getEntries( uint32_t ID, std::vector<rr::indexEntry> &entries ) const {
                if (!loading_ && (ID>numIDs_ || N_[ID]==0))
                    return 0;
                return protoIndex::getEntries(ID, entries);
            }
        
        void
            getUniqEntries( rr::indexEntry &queryRep,
                            uniqEntries &entries ) const;
    
    private:
        uint64_t const limit_;
        bool loading_;
        std::vector<uint32_t> N_;
        DISALLOW_COPY_AND_ASSIGN(protoIndexLimit)
};

#endif
