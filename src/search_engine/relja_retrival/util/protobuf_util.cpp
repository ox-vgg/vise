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

#include "protobuf_util.h"



void
protobufUtil::addRangeToEnd(
        std::string const &from,
        int begin, int end,
        std::string &to ){
    ASSERT(end>=begin);
    ASSERT(from.size() >= static_cast<uint32_t>(end) );
    
    uint32_t itStr= to.size();
    to.resize( to.size() + end-begin );
    char const *itElem= from.data();
    char const *endElem= itElem + end;
    itElem+= begin;
    for (; itElem!=endElem; ++itElem, ++itStr)
        to[itStr]= *itElem;
}
