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

#include "holidays_public.h"

#include <string>
#include <stdio.h>

#include <boost/filesystem.hpp>

#include <Eigen/LU>
#include <Eigen/SVD>

#include "desc_to_hell.h"
#include "macros.h"



inline float sqr(float x){ return x*x; }



void holidaysPublic::getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float *&descs ) const {
    
    std::string fn= boost::filesystem::basename(fileName);
    
    FILE *f= fopen( (descDir_ + fn + ".siftgeo").c_str() ,"rb");
    ASSERT(f!=NULL);
    
    fseek(f, 0, SEEK_END );
    numFeats= ftell(f)/(9 * 4 + 1 * 4 + 128);
    fseek(f, 0, SEEK_SET);
    
    float geom[9];
    uint8_t sift[128];
    uint8_t *siftEnd= sift+128;
    descs= new float[numFeats*128];
    float *descsIt= descs;
    regions.resize(numFeats);
    int ret= 0;
    int32_t nDimsTemp;
    
    for (uint32_t i=0; i<numFeats; ++i){
        ret= fread(geom, sizeof(float), 9, f);
        
        // Matlab to convert: M=[m11,m12;m21,m22]; where m's are geom[4:7], scale is [2] (0 based indexing)
        // [Vi,D,V]= svd(M);
        // D= D*scale;
        // D= diag(1./(diag(D).^2));
        // V'*D*V
        Eigen::Matrix2f M, V, D2, A;
        float scale= geom[2];
        M(0,0)= geom[4]; M(0,1)= geom[5];
        M(1,0)= geom[6]; M(1,1)= geom[7];
        Eigen::JacobiSVD<Eigen::Matrix2f> svd( M, Eigen::ComputeFullV );
        Eigen::Vector2f D= svd.singularValues();
        V= svd.matrixV();
        D2(0,0)= 1.0/sqr(D(0)*scale);
        D2(0,1)= 0;
        D2(1,0)= 0;
        D2(1,1)= 1.0/sqr(D(1)*scale);
        A.noalias()= V.transpose() * D2 * V;
        ASSERT( fabs(A(0,1)-A(1,0)) < 1e-6 );
        
        // minus in order to correspond to y being flipped in images
        // also check if det(V)==1 or -1, because if -1 then flipped
        regions[i].set(geom[0], geom[1],
            A(0,0),
            -( V.determinant()>0 ? 1: -1 ) * A(0,1),
            A(1,1));
        
        ret= fread(&nDimsTemp, sizeof(int32_t), 1, f);
        ASSERT(nDimsTemp==128);
        
        ret= fread(sift, sizeof(uint8_t), 128, f);
        for (uint8_t *siftIt= sift; siftIt!=siftEnd; ++siftIt, ++descsIt)
            *descsIt= static_cast<float>(*siftIt);
    }
    fclose(f);
    REMOVE_UNUSED_WARNING(ret);
    
    if (useRootSIFT_)
        descToHell::convertToHell(128, numFeats, descs);
}



std::string
holidaysPublic::getRawDescs(float const *descs, uint32_t numFeats) const {
    throw std::runtime_error( "Don't do this - it is only used when making training descriptors, and you shouldn't be doing it for Holidays (use flickr60k)" );
    std::string res(numFeats*128, '\0');
    uint8_t *resIter= reinterpret_cast<uint8_t*>(&res[0]);
    float const *inIter= descs;
    float const *inIterEnd= descs + numFeats*128;
    for (; inIter!=inIterEnd; ++inIter, ++resIter)
        *resIter= static_cast<uint8_t>(*inIter + 0.1); // +0.1 is to counter numerical issues because cast does floor()
    return res;
}
