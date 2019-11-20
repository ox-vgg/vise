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

#ifndef _IMAGE_UTIL_H_
#define _IMAGE_UTIL_H_

#include <stdint.h>
#include <string>



namespace imageUtil {
    
    // success?
    bool
        convert(std::string inFn, std::string outFn);
    
    // success?
    bool
        convertToJpegTemp(std::string inFn, std::string &outFn, bool &createdJpeg);
    
    // success?
    bool
        checkAndConvertToJpegTemp(std::string inFn, std::string &outFn, bool &createdJpeg);
    
    std::pair<uint32_t, uint32_t>
        getWidthHeight(std::string imageFn);
    
};

#endif
