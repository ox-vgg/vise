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

#ifndef _PROTOBUF_UTIL_H_
#define _PROTOBUF_UTIL_H_

#include <google/protobuf/repeated_field.h>

#include "macros.h"



namespace protobufUtil {
    
    
    
    template <typename E>
    void addRangeToEnd( google::protobuf::RepeatedField<E> const &from,
                        int begin, int end,
                        google::protobuf::RepeatedField<E> &to ){
        ASSERT(end>=begin);
        ASSERT(from.size()>=end);
        to.Reserve( to.size() + end-begin );
        E const *itElem= from.data();
        E const *endElem= itElem + end;
        itElem+= begin;
        for (; itElem!=endElem; ++itElem)
            to.AddAlreadyReserved( *itElem );
    }
    
    
    
    void addRangeToEnd( std::string const &from,
                        int begin, int end,
                        std::string &to );
    
    
    
    #define PROTOBUFUTIL_ADD_ALL(from, to, fieldName) \
    protobufUtil::addRangeToEnd( \
        from.fieldName(), \
        0, thisQueryRep.fieldName ## _size(), \
        *(to.mutable_ ## fieldName()) );
    
    
    
    #define PROTOBUFUTIL_ADD_RANGE_TO_END(from, begin, end, to, fieldName) \
    protobufUtil::addRangeToEnd( \
        from.fieldName(), \
        begin, end, \
        *(to.mutable_ ## fieldName()) );
    
    
    
    template <typename E>
    void addManyToEnd( E val, uint32_t num,
                       google::protobuf::RepeatedField<E> &to ){
        to.Reserve( to.size() + num );
        for (uint32_t i= 0; i<num; ++i)
            to.AddAlreadyReserved( val );
    }
    
    
    
};

#endif
