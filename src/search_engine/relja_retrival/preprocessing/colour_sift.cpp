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

#include "colour_sift.h"

#include <string>
#include <stdio.h>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "image_util.h"
#include "macros.h"



void readRegsAndDescs( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs, uint32_t const expectDim ){
    
    int ret;
    uint32_t numDims, elementsPerPoint, bytesPerElement;
    
    FILE *fin= fopen(fileName, "rb");
    
    if (fin==NULL){
        regions.clear();
        numFeats= 0;
        descs= new float[0];
        return;
    }
    
    {
        uint8_t temp[16];
        ret= fread(temp, 1, 16, fin); // ignore headers
    }
    
    ret= fread(&elementsPerPoint, sizeof(elementsPerPoint), 1, fin);
    ASSERT( elementsPerPoint == 5); // x y scale orientation cornerness
    ret= fread(&numDims, sizeof(numDims), 1, fin);
    ret= fread(&numFeats, sizeof(numFeats), 1, fin);
    if (numFeats==0){
        regions.clear();
        descs= new float[0];
        return;
    }
    ASSERT(numDims == expectDim);
    ret= fread(&bytesPerElement, sizeof(numFeats), 1, fin);
    ASSERT(bytesPerElement == 8);
    
    double *regsRaw= new double[elementsPerPoint*numFeats];
    ret= fread(regsRaw, sizeof(double), elementsPerPoint*numFeats, fin);
    
    double *itRegs= regsRaw;
    double r, a;
    regions.resize( numFeats );
    for (uint32_t i=0; i<numFeats; ++i, itRegs+= 5){
        r= *(itRegs+2);
        a= 1.0/(r*r);
        regions[i].set( *itRegs, *(itRegs+1), a, 0.0, a);
    }
    
    delete []regsRaw;
    
    double *descsRaw= new double[numDims*numFeats];
    ret= fread(descsRaw, sizeof(double), numDims*numFeats, fin);
    fclose(fin);
    REMOVE_UNUSED_WARNING(ret);
    
    descs= new float[numDims*numFeats];
    float *descsEnd= descs + numDims*numFeats;
    double *descsRawIt= descsRaw;
    for (float *descsIt= descs; descsIt != descsEnd; ++descsIt, ++descsRawIt)
        *descsIt= *descsRawIt;
    delete []descsRaw;
}



vanDeSande::vanDeSande(std::string descriptor, double harrisK, double laplaceThreshold)
        : descriptor_(descriptor),
          harrisK_(harrisK),
          laplaceThreshold_(laplaceThreshold) {
    if (descriptor=="opponentsift")
        numDims_= 384;
    else if (descriptor=="sift")
        numDims_= 128;
    else
        throw std::runtime_error( "Unknown descriptor" );
}



void vanDeSande::getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const {
    
    std::string tempDescsFn= boost::filesystem::unique_path("/tmp/rr_feat_%%%%-%%%%-%%%%-%%%%.txt").native();
    
    // convert to jpg if it isn't jpg already
    std::string fileName_jpeg;
    bool doDelJpeg= false;
    if (!imageUtil::checkAndConvertToJpegTemp(fileName, fileName_jpeg, doDelJpeg)){
        numFeats= 0;
        regions.clear();
        descs= new float[0];
        return;
    }
    
    // execute
    int ret= system(
        ( boost::format("colorDescriptor \"%s\" --detector harrislaplace --harrisK %.3f --laplaceThreshold %.3f --descriptor %s --noErrorLog --outputFormat binary --output \"%s\" > /dev/null") % fileName % harrisK_ % laplaceThreshold_ % descriptor_ % tempDescsFn ).str().c_str() );
    REMOVE_UNUSED_WARNING(ret);
    
    // image cleanup
    if (doDelJpeg)
        boost::filesystem::remove( fileName_jpeg );
    
    // read
    readRegsAndDescs( tempDescsFn.c_str(), numFeats, regions, descs, numDims_ );
    
    // cleanup
    boost::filesystem::remove( tempDescsFn );
    
}



std::string
vanDeSande::getRawDescs(float const *descs, uint32_t numFeats) const {
    std::string res(numFeats*numDims_, '\0');
    uint8_t *resIter= reinterpret_cast<uint8_t*>(&res[0]);
    float const *inIter= descs;
    float const *inIterEnd= descs + numFeats*numDims_;
    for (; inIter!=inIterEnd; ++inIter, ++resIter)
        *resIter= static_cast<uint8_t>(*inIter + 0.1); // +0.1 is to counter numerical issues because cast does floor()
    return res;
}
