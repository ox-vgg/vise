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

#include "feat_getter.h"

#include <algorithm>
#include <string>

#include <boost/filesystem.hpp>

#ifdef RR_MAGICK

#include <Magick++.h>

#else

#warning Unable to crop image and then extract features, extracting from the entire image might be inefficient depending on the image

#endif


void
featGetter::getFeats( const char fileName[], uint32_t xl, uint32_t xu, uint32_t yl, uint32_t yu, uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const {
    
    std::string fileName_= util::expandUser( fileName );
    if (xl>xu) std::swap(xl,xu);
    if (yl>yu) std::swap(yl,yu);
    
#ifdef RR_MAGICK
    
    std::string tempCropImageFn= boost::filesystem::unique_path("/tmp/rr_image_%%%%-%%%%-%%%%-%%%%.jpg").native();
    try {
        Magick::Image im;
        im.read(fileName_);
        im.crop( Magick::Geometry(xu-xl, yu-yl, xl, yl) );
        im.write(tempCropImageFn);
    } catch (std::exception &error) {
        std::cout<< "featGetter::getFeats: Exception= "<<error.what()<<"\n";
        numFeats= 0;
        regions.clear();
        descs= new float[0];
        return;
    }
    
#else
    // careful not to delete it later!
    std::string tempCropImageFn= fileName_;
#endif
    
    getFeats( tempCropImageFn.c_str(), numFeats, regions, descs );

#ifdef RR_MAGICK
    
    for (std::vector<ellipse>::iterator itR= regions.begin();
            itR!=regions.end();
            ++itR) {
        
        itR->x+= xl;
        itR->y+= yl;
        
    }
    
    boost::filesystem::remove( tempCropImageFn );
    
#else
    // TODO: for split detector/descriptor (e.g. like hessian affine + RootSIFT like I use), it would be more efficient to detect then based on ROI, then extract descriptors
    
    // do not delete tempCropImageFn here as it is the original image!
    
    std::vector<ellipse> regionsAll;
    regionsAll.swap(regions);
    std::vector<uint32_t> keepInds;
    
    uint32_t i= 0;
    for (std::vector<ellipse>::const_iterator itR= regionsAll.begin();
         itR!=regionsAll.end();
         ++itR, ++i) {
        
        if (itR->x >= xl && itR->x <= xu && itR->y >= yl && itR->y <=yu ) {
            regions.push_back( *itR );
            keepInds.push_back(i);
        }
    }
    
    uint32_t numFeats_old= regionsAll.size();
    numFeats= regions.size();
    // copy the descriptors
    uint32_t numDims_= numDims();
    float *descs_new= new float[numFeats*numDims_];
    for (uint32_t ii= 0; ii < keepInds.size(); ++ii) {
        std::memcpy( descs_new[ii*numDims_], descs[keepInds[ii]*numDims_], numDims_ * sizeof(float) );
    }
    delete []descs;
    descs= descs_new;
    // don't delete descs_new as descs point to the same thing, and will be thus freed later
#endif
    
}
