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

#include "register_images.h"

#include <algorithm>
#include <math.h>
#include <vector>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include <Magick++.h>

#include "ellipse.h"
#include "feat_getter.h"
#include "feat_standard.h"
#include "putative.h"
#include "det_ransac.h"
#include "macros.h"



//---- stuff for parallel feature extraction

class featWorker {
    
    public:
        
        featWorker( featGetter *aFeatGetterObj, const char aFn[], double aXl, double aXu, double aYl, double aYu, uint32_t &aNumFeats, std::vector<ellipse> &aRegions, float *&aDescs ) : featGetterObj(aFeatGetterObj), fn(aFn), xl(aXl), xu(aXu), yl(aYl), yu(aYu), numFeats(&aNumFeats), regions(&aRegions), descs(&aDescs)
            {}
    
    void
        operator()(){
            featGetterObj->getFeats( fn,
                            static_cast<uint32_t>(xl), static_cast<uint32_t>(xu),
                            static_cast<uint32_t>(yl), static_cast<uint32_t>(yu),
                            *numFeats, *regions, *descs );
        }
    
    private:
        featGetter *featGetterObj;
        const char *fn;
        double xl, xu, yl, yu;
        uint32_t *numFeats;
        std::vector<ellipse> *regions;
        float **descs;
    
};

//----



void
registerImages::registerFromGuess(
        sameRandomUint32 const &sameRandomObj,
        const char image_fn1[], const char image_fn2[],
        double xl, double xu, double yl, double yu,
        homography &Hinit,
        const char outFn1[], const char outFn2[], const char outFn2t[],
        const char *fullSizeFn1, const char *fullSizeFn2 ) {
    
    static const double expandOutBy= 0.1;
    featGetter *featGetterObj= new featGetter_standard( "hesaff-rootsift" );
    
    bool fullSizeExist= false;
    if (fullSizeFn1!=NULL || fullSizeFn2!=NULL)
        fullSizeExist= true;
    if (fullSizeFn1==NULL) fullSizeFn1= image_fn1;
    if (fullSizeFn2==NULL) fullSizeFn2= image_fn2;
    
    Magick::Image im1; im1.read( fullSizeFn1 );
    Magick::Image im2; im2.read( fullSizeFn2 );
    Magick::Image im2t;
    
    if (fullSizeExist) {
        // modify Hinit to account for scale change
        Magick::Image imSmall1; imSmall1.read( image_fn1 );
        Magick::Image imSmall2; imSmall2.read( image_fn2 );
        double sc1w= static_cast<double>(im1.columns())/imSmall1.columns();
        double sc2w= static_cast<double>(im2.columns())/imSmall2.columns();
        double sc1h= static_cast<double>(im1.rows())/imSmall1.rows();
        double sc2h= static_cast<double>(im2.rows())/imSmall2.rows();
        double sc21w= sc2w/sc1w, sc21h= sc2h/sc1h;
        Hinit.H[0]*= sc21w;
        Hinit.H[1]*= sc21w;
        Hinit.H[2]*= sc2w;
        Hinit.H[3]*= sc21h;
        Hinit.H[4]*= sc21h;
        Hinit.H[5]*= sc2h;
        xl*= sc1w; xu*= sc1w;
        yl*= sc1h; yu*= sc1h;
    }
    
    homography H= Hinit;
    
    uint32_t numFeats1, numFeats2, bestNInliers;
    float *descs1, *descs2;
    std::vector<ellipse> regions1, regions2;
    
    boost::thread *thread1, *thread2;
    
    // compute RootSIFT: image 1
    
    thread1= new boost::thread( featWorker( featGetterObj, fullSizeFn1, xl, xu, yl, yu, numFeats1, regions1, descs1 ) );
    
    
    bool firstGo= true, extractFinished1= false;
    uint32_t loopNum_= 0;
    
    boost::filesystem::path fullSizeFn2_t= boost::filesystem::unique_path("/tmp/rr_register_%%%%-%%%%-%%%%-%%%%.jpg");
    
    matchesType inlierInds;
    
    while (1){
        
        if (!firstGo){
            
            // compute RootSIFT: image 2
            
            thread2= new boost::thread( featWorker( featGetterObj, fullSizeFn2_t.c_str(), xl, xu, yl, yu, numFeats2, regions2, descs2 ) );
            
            // wait for feature extraction of image 1 to finish
            if (!extractFinished1){
                thread1->join();
                delete thread1;
                extractFinished1= true;
            }
            
            // wait for feature extraction of image 2 to finish
            thread2->join();
            delete thread2;
            
            // run RANSAC
            
            homography Hnew;
            
            detRansac::matchDesc(
                sameRandomObj,
                bestNInliers,
                descs1, regions1,
                descs2, regions2,
                featGetterObj->numDims(),
                loopNum_>1?1.0:5.0, 0.0, 1000.0, static_cast<uint32_t>(4),
                true, 0.81f, 100.0f,
                &Hnew, &inlierInds
                );
            
            bool success= bestNInliers>9;
            
            if (!success)
                break;
            
            // apply new H to current H (i.e. H= H * Hnew)
            {
                double Happlied[9];
                for (int i= 0; i<3; ++i)
                    for (int j=0; j<3; ++j)
                        Happlied[i*3+j]= H.H[i*3  ] * Hnew.H[  j] +
                                         H.H[i*3+1] * Hnew.H[3+j] +
                                         H.H[i*3+2] * Hnew.H[6+j];
                H.set(Happlied);
            }
            
        }
        
        H.normLast();
        
        // im2 -> im1 transformation
        double Hinv[9];
        H.getInverse(Hinv);
        homography::normLast(Hinv);
        
        // warp image 2 into image 1
        im2t= im2;
        // AffineProjection(sx, rx, ry, sy, tx, ty) <=> H=[sx, ry, tx; sy, rx, ty; 0 0 1]
        double MagickAffine[6]={Hinv[0],Hinv[3],Hinv[1],Hinv[4],Hinv[2],Hinv[5]};
        im2t.virtualPixelMethod(Magick::BlackVirtualPixelMethod);
        im2t.distort(Magick::AffineProjectionDistortion, 6, MagickAffine, false);
        
        firstGo= false;
        
        ++loopNum_;
        if (loopNum_>1)
            break;
        
        im2t.write( fullSizeFn2_t.c_str() );
        
    }
    
    delete []descs1;
    delete []descs2;
    boost::filesystem::remove( fullSizeFn2_t );
    
    
    // draw resulting images
    
    double xl_= xl, xu_= xu, yl_= yl, yu_= yu;
    double dw_= expandOutBy*(xu-xl), dh_= expandOutBy*(yu-yl);
    xl_= std::max(0.0, xl_-dw_/2);
    yl_= std::max(0.0, yl_-dh_/2);
    xu_= std::min(static_cast<double>(im1.columns() ), xu_+dw_/2);
    yu_= std::min(static_cast<double>(im1.rows()), yu_+dh_/2);
    Magick::Geometry cropRect1(xu_-xl_, yu_-yl_, xl_, yl_);
    double xl2_, xu2_, yl2_, yu2_;
    findBBox2( xl_, xu_, yl_, yu_, H, xl2_, xu2_, yl2_, yu2_, im2.columns(), im2.rows() );
    Magick::Geometry cropRect2(xu2_-xl2_, yu2_-yl2_, xl2_, yl2_);
    
    im1.crop( cropRect1 );
    im1.write( outFn1 );
    im2.crop( cropRect2 );
    im2.write( outFn2 );
    im2t.crop( cropRect1 );
    im2t.write( outFn2t );
    
    delete featGetterObj;
}



void
registerImages::registerFromQuery(
        query const &query_obj,
        const char inFn1[], uint32_t docID2,
        datasetAbs const &datasetObj,
        spatialRetriever const &spatialRetriever_obj,
        const char outFn1[], const char outFn2[], const char outFn2t[],
        const char *fullSizeFn1, const char *fullSizeFn2 ) {
    
    homography H;
    std::vector< std::pair<ellipse,ellipse> > matches;
    spatialRetriever_obj.getMatches( query_obj, docID2, H, matches );
    
    std::string image_fn1= inFn1;
    if (query_obj.isInternal)
        image_fn1= datasetObj.getFn( query_obj.docID );
    std::string image_fn2= datasetObj.getFn( docID2 );
    
    registerImages::registerFromGuess( *(spatialRetriever_obj.getSameRandom()), image_fn1.c_str(), image_fn2.c_str(), query_obj.xl, query_obj.xu, query_obj.yl, query_obj.yu, H, outFn1, outFn2, outFn2t, fullSizeFn1, fullSizeFn2 );
    
}


void
registerImages::findBBox2( double xl, double xu, double yl, double yu, homography const &H, double &xl2, double &xu2, double &yl2, double &yu2, uint32_t w2, uint32_t h2 ){
    
    ASSERT( fabs(H.H[8]-1.0)<1e-5 );
    
    xl2= 10000; xu2= -10000; yl2= 10000; yu2= -10000;
    double x_, y_;
    
    homography::affTransform(H.H, xl, yl, x_, y_);
    xl2= std::min(xl2,x_); xu2= std::max(xu2,x_); yl2= std::min(yl2,y_); yu2= std::max(yu2,y_);
    
    homography::affTransform(H.H, xl, yu, x_, y_);
    xl2= std::min(xl2,x_); xu2= std::max(xu2,x_); yl2= std::min(yl2,y_); yu2= std::max(yu2,y_);
    
    homography::affTransform(H.H, xu, yl, x_, y_);
    xl2= std::min(xl2,x_); xu2= std::max(xu2,x_); yl2= std::min(yl2,y_); yu2= std::max(yu2,y_);
    
    homography::affTransform(H.H, xu, yu, x_, y_);
    xl2= std::min(xl2,x_); xu2= std::max(xu2,x_); yl2= std::min(yl2,y_); yu2= std::max(yu2,y_);
    
    xl2= std::max(0.0,xl2);
    yl2= std::max(0.0,yl2);
    xu2= std::min(xu2,static_cast<double>(w2));
    yu2= std::min(yu2,static_cast<double>(h2));
    
}
