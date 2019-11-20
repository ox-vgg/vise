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

#include "image_util.h"

#include <iostream>
#include <stdexcept>

#include <boost/algorithm/string.hpp>

#include "util.h"


#ifdef RR_MAGICK

#include <Magick++.h>

bool
imageUtil::convert(std::string inFn, std::string outFn) {
    try {
        Magick::Image im;
        im.read(inFn);
        im.write(outFn);
        return true;
    } catch (std::exception &error) {
        std::cerr<< "imageUtil::convert: Exception= "<<error.what()<<"\n";
        return false;
    }
}



bool
imageUtil::convertToJpegTemp(std::string inFn, std::string &outFn, bool &createdJpeg){
    createdJpeg= false;
    try {
        Magick::Image im;
        im.read(inFn);
        if (im.magick()=="JPEG") {
            createdJpeg= false;
            outFn= inFn;
        } else {
            outFn= util::getTempFileName( "", "rr_image_", ".jpg" );
            im.write(outFn);
            createdJpeg= true;
        }
        return true;
    } catch (std::exception &error) {
        std::cerr<< "imageUtil::convertToJpegTemp: Exception= "<<error.what()<<"\n";
        return false;
    }
}



bool
imageUtil::checkAndConvertToJpegTemp(std::string inFn, std::string &outFn, bool &createdJpeg){
    createdJpeg= false;
    try {
        Magick::Image im;
        im.read(inFn);
        if (im.columns()<10 || im.rows()<10)
            return false;
        if (im.magick()=="JPEG") {
            createdJpeg= false;
            outFn= inFn;
        } else {
            outFn= util::getTempFileName( "", "rr_image_", ".jpg" );
            im.write(outFn);
            createdJpeg= true;
        }
        return true;
    } catch (std::exception &error) {
        std::cerr<< "imageUtil::convertToJpegTemp: Exception= "<<error.what()<<"\n";
        return false;
    }
}



std::pair<uint32_t, uint32_t>
imageUtil::getWidthHeight(std::string imageFn){
    try {
        Magick::Image im;
        im.read(imageFn);
        return std::make_pair(im.columns(), im.rows());
    } catch (std::exception &error) {
        std::cerr<< "imageUtil::getWidthHeight: Exception= "<<error.what()<<"\n";
        return std::make_pair(0,0);
    }
}



#else


#warning Unable to get image width/height without Magick++
#warning Unable to convert images without Magick++


bool
imageUtil::convert(std::string inFn, std::string outFn) {
    std::cerr<< "imageUtil::convert: Need Magick++ for this\n";
    return false;
}



bool
imageUtil::convertToJpegTemp(std::string inFn, std::string &outFn, bool &createdJpeg){
    createdJpeg= false;
    outFn= inFn;
    
    boost::algorithm::to_lower(inFn);
    
    if ( boost::algorithm::ends_with(inFn, ".jpg") || boost::algorithm::ends_with(inFn, ".jpeg") || boost::algorithm::ends_with(inFn, ".jpe") )
        return true;
    else
        return false;
}



bool
imageUtil::checkAndConvertToJpegTemp(std::string inFn, std::string &outFn, bool &createdJpeg){
    return convertToJpegTemp(inFn, outFn, createdJpeg);
}



std::pair<uint32_t, uint32_t>
imageUtil::getWidthHeight(std::string imageFn){
    std::cerr<< "imageUtil::getWidthHeight: Need Magick++ for this so returning (0,0)\n";
    return std::make_pair(0,0);
}



#endif
