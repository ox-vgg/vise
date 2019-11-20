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

#ifndef _QUERY_IN_H_
#define _QUERY_IN_H_

#include <limits>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include "macros.h"

static double inf= std::numeric_limits<double>::infinity();

class query {
    
    public:
        
        query( uint32_t aDocID, bool aIsInternal= true, std::string aCompDataFn= "",
               double aXl= -inf, double aXu= inf, double aYl= -inf, double aYu= inf ) :
            docID(aDocID), xl(aXl), xu(aXu), yl(aYl), yu(aYu), isInternal(aIsInternal), compDataFn(aCompDataFn) {
                
                ASSERT( isInternal == (compDataFn=="") );
                double temp;
                if (xl>xu){ temp= xl; xl= xu; xu= temp; }
                if (yl>yu){ temp= yl; yl= yu; yu= temp; }
                
        }
        
        inline bool allInf() const { return (xl == -inf) && (xu == inf) && (yl == -inf) && (yu == inf); }
        
        uint32_t docID;
        double xl, xu, yl, yu;
        bool isInternal;
        std::string compDataFn;
        
};

#endif
