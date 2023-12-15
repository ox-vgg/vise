/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

*/

#ifndef _ELLIPSE_UTIL_H_
#define _ELLIPSE_UTIL_H_

#include "query.h"
#include "ellipse.h"



class ellipseUtil {
    
    
    
    public:
        
        
        inline static bool
            inside( ellipse const &reg, query const &query_obj ){
                return
                    ( reg.x >= query_obj.xl ) && 
                    ( reg.x <= query_obj.xu ) &&
                    ( reg.y >= query_obj.yl ) && 
                    ( reg.y <= query_obj.yu );
            }
        
};

#endif
