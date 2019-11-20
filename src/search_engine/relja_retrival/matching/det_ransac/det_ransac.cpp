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

#include "det_ransac.h"

#include <Eigen/Dense>

#include "putative.h"



inline double mysqr( double x ){ return x*x; }



double
detRansac::matchWords_Hard(
    sameRandomUint32 const &sameRandomObj,
    
    uint32_t &nInliers,
    
    std::vector<quantDesc> const &ids1,
    std::vector<ellipse> const &ellipses1,
    std::vector<uint32_t> const *presortedInds1,
    std::vector<quantDesc> const &ids2,
    std::vector<ellipse> const &ellipses2,
    
    double errorThr,
    double lowAreaChange, double highAreaChange,
    uint32_t nReest,
    
    homography *H,
    matchesType *inlierInds
    
    ){
    
    std::vector<uint32_t> ids1_flat, ids2_flat;
    quantDesc::flattenHard( ids1, ids1_flat );
    quantDesc::flattenHard( ids2, ids2_flat );
    
    std::vector< std::pair<uint32_t, uint32_t> > putativeMatches;
    putative_quantized::getPutativeMatches_Hard( ids1_flat, ids2_flat, putativeMatches, presortedInds1 );
    
    return detRansac::match( sameRandomObj, nInliers, ellipses1, ellipses2, putativeMatches, NULL, errorThr, lowAreaChange, highAreaChange, nReest, H, inlierInds );
    
}



double
detRansac::matchWords_Soft(
    sameRandomUint32 const &sameRandomObj,
    
    uint32_t &nInliers,
    
    std::vector<quantDesc> const &ids1,
    std::vector<ellipse> const &ellipses1,
    std::vector<uint32_t> const *presortedInds1,
    std::vector<quantDesc> const &ids2,
    std::vector<ellipse> const &ellipses2,
    
    double errorThr,
    double lowAreaChange, double highAreaChange,
    uint32_t nReest,
    
    homography *H,
    matchesType *inlierInds
    
    ){
    
    std::vector<wordWeightPair> ids1_flat, ids2_flat;
    std::vector<uint32_t> qDInds1, qDInds2;
    quantDesc::flatten( ids1, ids1_flat, &qDInds1, true );
    quantDesc::flatten( ids2, ids2_flat, &qDInds2, true );
    
    std::vector< std::pair<uint32_t, uint32_t> > putativeMatches;
    std::vector<double> PMweights;
    putative_quantized::getPutativeMatches_Soft( ids1_flat, qDInds1, ids2_flat, qDInds2, putativeMatches, PMweights, presortedInds1 );
    
//     return detRansac::match( sameRandomObj, nInliers, ellipses1, ellipses2, putativeMatches, NULL, errorThr, lowAreaChange, highAreaChange, nReest, H, inlierInds );
    return detRansac::match( sameRandomObj, nInliers, ellipses1, ellipses2, putativeMatches, &PMweights, errorThr, lowAreaChange, highAreaChange, nReest, H, inlierInds );
    
}



double
detRansac::matchDesc(
    sameRandomUint32 const &sameRandomObj,
    
    uint32_t &nInliers,
    
    float const* desc1,
    std::vector<ellipse> const &ellipses1,
    float const* desc2,
    std::vector<ellipse> const &ellipses2,
    uint32_t nDims,
    
    double errorThr,
    double lowAreaChange, double highAreaChange,
    uint32_t nReest,
    
    bool useLowe,
    float deltaSq,
    float epsilon,
    
    homography *H,
    matchesType *inlierInds
    
    ){
    
    std::vector< std::pair<uint32_t, uint32_t> > putativeMatches;
    
    putative_desc<float>::getPutativeMatches( desc1, ellipses1.size(), desc2, ellipses2.size(), nDims, putativeMatches, useLowe, deltaSq, epsilon );
    
    return detRansac::match( sameRandomObj, nInliers, ellipses1, ellipses2, putativeMatches, NULL, errorThr, lowAreaChange, highAreaChange, nReest, H, inlierInds);
    
}



double
detRansac::match(
    sameRandomUint32 const &sameRandomObj,
    
    uint32_t &bestNInliers,
    
    std::vector<ellipse> const &ellipses1,
    std::vector<ellipse> const &ellipses2,
    matchesType const &putativeMatches,
    std::vector<double> const *PMweights,
    
    double errorThr,
    double lowAreaChange, double highAreaChange,
    uint32_t nReest,
    
    homography *H,
    matchesType *inlierInds
    
    ){
    
    bestNInliers= 0;
    
    uint32_t nPutativeMatches= putativeMatches.size();
    
    if (nPutativeMatches<3)
        return 0;
    
    //------- prepare
    
    bool delWeights= false;
    if (PMweights==NULL) {
        PMweights= new std::vector<double>( nPutativeMatches, 1.0 );
        delWeights= true;
    }
    
    //------- generate Hs
    
    detRansac::inlierFinder inlierFinder_obj( ellipses1, ellipses2, putativeMatches, *PMweights, errorThr, lowAreaChange, highAreaChange );
    
    // initialize with identity
    homography Hident; Hident.setIdentity();
    homography bestH= Hident;
    
    double bestScore= 0.0;
    bestNInliers= 0;
    bestScore= inlierFinder_obj.getScore( bestH, bestNInliers, NULL );
    
    std::vector<homography> Hs;
    Hs.reserve( nPutativeMatches );
    
    for (std::vector< std::pair<uint32_t, uint32_t> >::const_iterator itPM= putativeMatches.begin();
         itPM!=putativeMatches.end();
         ++itPM){
        
        Hs.push_back( homography( ellipses1[itPM->first], ellipses2[itPM->second] ) );
        
    }
    
    uint32_t globalNIter= 0;
    
    sameRandomObj.shuffle<homography>( Hs.begin(), Hs.end() );
    
    {
        //------- RANSAC core
        
        static const double pFail= 0.001;
        
        uint32_t nInliers, iH= 0;
        double score;
        
        for (std::vector<homography>::const_iterator itH= Hs.begin();
             itH!=Hs.end() && globalNIter < detRansac::getNStopping(pFail, nPutativeMatches, bestNInliers);
             ++itH, ++globalNIter, ++iH){
            
            score= inlierFinder_obj.getScore( *itH, nInliers, NULL );
            
            if (nInliers>3 && score>bestScore) {
                bestNInliers= nInliers;
                bestScore= score;
                bestH= *itH;
            }
            
        }
        
    }
    
    
    if (bestNInliers > 3){
        
        matchesType bestInliers;
        bestInliers.reserve(bestNInliers);
        uint32_t nInliers_new;
        inlierFinder_obj.getScore( bestH, nInliers_new, &bestInliers );
        
        //------- reestimate
        
        homography H_new= bestH;
        double score_new;
        matchesType inliers_new= bestInliers;
        
        uint32_t iReest;
        for (iReest= 0; nInliers_new>3 && iReest<nReest; ++iReest){
            detRansac::getH( ellipses1, ellipses2, inliers_new, H_new );
            score_new= inlierFinder_obj.getScore( H_new, nInliers_new, &inliers_new );
            if (nInliers_new>3 && score_new>bestScore){
                bestScore= score_new;
                bestNInliers= nInliers_new;
                bestH= H_new;
                bestInliers= inliers_new; // don't swap as getH in next iteration needs it!
            }
        }
        
        if (inlierInds)
            inlierInds->swap( bestInliers );
        
        if (H)
            *H= bestH;
        
    }
    
    //------- cleanup
    
    if (delWeights)
        delete PMweights;
    
    return bestScore;
    
}



void
detRansac::getH( std::vector<ellipse> const &ellipses1,
                 std::vector<ellipse> const &ellipses2,
                 matchesType const &inliers, homography &H ){
    
    Eigen::Matrix<double, Eigen::Dynamic, 7> A( inliers.size()*2, 7 );
    
    double *x1, *y1, *x2, *y2, *_temp;
    ellipse::getCentres( ellipses1, ellipses2, inliers, x1, y1, x2, y2, _temp );
    uint32_t nInliers= inliers.size();
    
    // normalize points
    homography Hnorm1, Hnorm2;
    normPoints(x1, y1, nInliers, Hnorm1);
    normPoints(x2, y2, nInliers, Hnorm2);
    double Hnorm2inv[9];
    Hnorm2.getInverse(Hnorm2inv);
    
    // fit homography
    uint32_t i=0;
    
    for (matchesType::const_iterator itIn= inliers.begin();
         itIn!=inliers.end();
         ++itIn, ++i){
        
        A.coeffRef( i*2   , 0 )=    0.0;
        A.coeffRef( i*2   , 1 )=    0.0;
        A.coeffRef( i*2   , 2 )=    0.0;
        A.coeffRef( i*2   , 3 )=  x1[i];
        A.coeffRef( i*2   , 4 )=  y1[i];
        A.coeffRef( i*2   , 5 )=    1.0;
        A.coeffRef( i*2   , 6 )= -y2[i];
        
        A.coeffRef( i*2+1 , 0 )= -x1[i];
        A.coeffRef( i*2+1 , 1 )= -y1[i];
        A.coeffRef( i*2+1 , 2 )=   -1.0;
        A.coeffRef( i*2+1 , 3 )=    0.0;
        A.coeffRef( i*2+1 , 4 )=    0.0;
        A.coeffRef( i*2+1 , 5 )=    0.0;
        A.coeffRef( i*2+1 , 6 )=  x2[i];
        
    }
    
    delete []x1; delete []y1; delete []x2; delete []y2; delete []_temp;
    
    typedef Eigen::Matrix<double, 7, 7> matrix7x7;
    matrix7x7 AtA;
    AtA.noalias()= A.transpose() * A;
    Eigen::SelfAdjointEigenSolver<matrix7x7> sol( AtA );
    Eigen::Matrix<double,7,1> x= sol.eigenvectors().col(0);
    
    H.H[0]= x.coeff(0,0); H.H[1]= x.coeff(1,0); H.H[2]= x.coeff(2,0);
    H.H[3]= x.coeff(3,0); H.H[4]= x.coeff(4,0); H.H[5]= x.coeff(5,0);
    H.H[6]=          0.0; H.H[7]=          0.0; H.H[8]= x.coeff(6,0);
    H.normLast();
    
    // denormalize H: H= Hnorm2inv * Hnorm * Hnorm1
    
    Eigen::Matrix<double, 3, 3> Hnorm2inv_, Hnorm1_, Hnorm_;
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j){
            Hnorm_.coeffRef(i,j)= H.H[i*3+j];
            Hnorm1_.coeffRef(i,j)= Hnorm1.H[i*3+j];
            Hnorm2inv_.coeffRef(i,j)= Hnorm2inv[i*3+j];
        }
    Eigen::Matrix<double, 3, 3> H_;
    H_.noalias()= Hnorm2inv_ * Hnorm_ * Hnorm1_;
    
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
            H.H[i*3+j]= H_.coeff(i,j);
    
#if 0
    // this is how it used to be done in OpenCV, not updating though so it might be obsolete..
    
    cv::Mat A( inliers.size()*2, 7, cv::DataType<double>::type ), x;
    
    double *x1, *y1, *x2, *y2, *_temp;
    ellipse::getCentres( ellipses1, ellipses2, inliers, x1, y1, x2, y2, _temp );
    uint32_t nInliers= inliers.size();
    
    // normalize points
    homography Hnorm1, Hnorm2;
    normPoints(x1, y1, nInliers, Hnorm1);
    normPoints(x2, y2, nInliers, Hnorm2);
    double Hnorm2inv[9];
    Hnorm2.getInverse(Hnorm2inv);
    
    // fit homography
    uint32_t i=0;
    
    for (matchesType::const_iterator itIn= inliers.begin();
         itIn!=inliers.end();
         ++itIn, ++i){
        
        // normalize transform
        
        A.at<double>( i*2   , 0 )=    0.0;
        A.at<double>( i*2   , 1 )=    0.0;
        A.at<double>( i*2   , 2 )=    0.0;
        A.at<double>( i*2   , 3 )=  x1[i];
        A.at<double>( i*2   , 4 )=  y1[i];
        A.at<double>( i*2   , 5 )=    1.0;
        A.at<double>( i*2   , 6 )= -y2[i];
        
        A.at<double>( i*2+1 , 0 )= -x1[i];
        A.at<double>( i*2+1 , 1 )= -y1[i];
        A.at<double>( i*2+1 , 2 )=   -1.0;
        A.at<double>( i*2+1 , 3 )=    0.0;
        A.at<double>( i*2+1 , 4 )=    0.0;
        A.at<double>( i*2+1 , 5 )=    0.0;
        A.at<double>( i*2+1 , 6 )=  x2[i];
        
    }
    
    delete []x1; delete []y1; delete []x2; delete []y2; delete []_temp;
    
    cv::SVD::solveZ(A, x);
    
    H.H[0]= x.at<double>(0,0); H.H[1]= x.at<double>(1,0); H.H[2]= x.at<double>(2,0);
    H.H[3]= x.at<double>(3,0); H.H[4]= x.at<double>(4,0); H.H[5]= x.at<double>(5,0);
    H.H[6]=               0.0; H.H[7]=               0.0; H.H[8]= x.at<double>(6,0);
    H.normLast();
    
    // denormalize H: H= Hnorm2inv * Hnorm * Hnorm1
    
    cv::Mat Hnorm2inv_(3,3, cv::DataType<double>::type),
            Hnorm1_(3,3, cv::DataType<double>::type),
            Hnorm_(3,3, cv::DataType<double>::type);
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j){
            Hnorm_.at<double>(i,j)= H.H[i*3+j];
            Hnorm1_.at<double>(i,j)= Hnorm1.H[i*3+j];
            Hnorm2inv_.at<double>(i,j)= Hnorm2inv[i*3+j];
        }
    cv::Mat H_= Hnorm2inv_ * Hnorm_ * Hnorm1_;
    
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
            H.H[i*3+j]= H_.at<double>(i,j);
#endif
    
}



void
detRansac::normPoints( double *x, double *y, uint32_t n, homography &Hnorm ){
    
    double EX= 0, EY= 0, stdX, stdY, EX2= 0, EY2= 0;
    uint32_t i;
    
    // get mean/variance
    for (i= 0; i<n; ++i){
        EX+= x[i];
        EY+= y[i];
        EX2+= mysqr(x[i]);
        EY2+= mysqr(y[i]);
    }
    EX/=n; EY/=n;
    EX2/=n; EY2/=n;
    // max is there just in case std=0 (as later norm=NaN in this case), in this case the dimension gets mapped to 0 (as mean is 0) so scaling makes no difference.. This is not nice however, as it means the problem is illposed
    stdX= std::max( sqrt(EX2-mysqr(EX)), 1e-4 );
    stdY= std::max( sqrt(EY2-mysqr(EY)), 1e-4 );
    
    double normX= sqrt(2.0)/stdX, normY= sqrt(2.0)/stdY;
    
    Hnorm.setIdentity();
    Hnorm.H[0]= normX; Hnorm.H[2]= -EX*normX;
    Hnorm.H[4]= normY; Hnorm.H[5]= -EY*normY;
    
    // normalize points
    for (i= 0; i<n; ++i){
        x[i]= (x[i]-EX)*normX;
        y[i]= (y[i]-EY)*normY;
    }
    
}



detRansac::inlierFinder::inlierFinder(
    std::vector<ellipse> const &aEllipses1,
    std::vector<ellipse> const &aEllipses2,
    matchesType const &aPutativeMatches,
    std::vector<double> const &aPMweights,
    double aErrorThr,
    double aLowAreaChange, double aHighAreaChange) :
    
    nIter(0),
    putativeMatches(&aPutativeMatches), PMweights(&aPMweights),
    errorThrSq(mysqr(aErrorThr)),
    lowAreaChangeSq(mysqr(aLowAreaChange)),
    highAreaChangeSq(mysqr(aHighAreaChange)),
    nPutativeMatches(aPutativeMatches.size()) {
    
    //------- get largest pIDs
    
    uint32_t maxpID1= 0, maxpID2= 0;
    
    for (matchesType::const_iterator itPM=putativeMatches->begin();
         itPM != putativeMatches->end();
         ++itPM){
        maxpID1= std::max(maxpID1, itPM->first);
        maxpID2= std::max(maxpID2, itPM->second);
    }
    point1Used.clear(); point1Used.resize(maxpID1+1,0);
    point1Used.clear(); point2Used.resize(maxpID2+1,0);
    
    ellipse::getCentres( aEllipses1, aEllipses2, *putativeMatches, x1, y1, x2, y2, areaDiffSq );
    
}



detRansac::inlierFinder::~inlierFinder(){
    delete []x1; delete []y1; delete []x2; delete []y2; delete []areaDiffSq;
}



double
detRansac::inlierFinder::getScore( homography const &H, uint32_t &nInliers, matchesType *inliers ){
    
    
    double score, detASq;
        
    score= 0.0;
    nInliers= 0;
    if (inliers)
        inliers->clear();
    
    detASq= mysqr( H.getDetAffine() );
    if (detASq<1e-4) return 0.0;
    
    double error;
    uint32_t pID1, pID2, iPM;
    double x, y, xi, yi;
    double lowAreaChangeSqByD =  lowAreaChangeSq / detASq;
    double highAreaChangeSqByD= highAreaChangeSq / detASq;
    
    double Hinv[9];
    H.getInverse( Hinv );
    
    ++nIter;
    
    iPM= 0;
    for (matchesType::const_iterator itPM= putativeMatches->begin();
         itPM!=putativeMatches->end();
         ++itPM, ++iPM){
        
        pID1= itPM->first;
        pID2= itPM->second;
        if (point1Used[ pID1 ]==nIter || point2Used[ pID2 ]==nIter)
            continue;
        
        homography::affTransform( H.H , x1[iPM], y1[iPM], x , y );
        homography::affTransform( Hinv, x2[iPM], y2[iPM], xi, yi );
        
        error= mysqr( x1[iPM]-xi ) + mysqr( y1[iPM]-yi ) +
               mysqr( x2[iPM]-x  ) + mysqr( y2[iPM]-y  );
        
        if (error < errorThrSq){
            /*
            areaChangeSq= detASq * ellipses2[ pID2 ].getPropAreaSq() / ellipses1[ pID1 ].getPropAreaSq();
            if ( areaChangeSq > lowAreaChangeSq && areaChangeSq < highAreaChangeSq ){
            */
            if (areaDiffSq[iPM] > lowAreaChangeSqByD && areaDiffSq[iPM] < highAreaChangeSqByD) {
                score+= PMweights->at(iPM);
                ++nInliers;
                point1Used[ pID1 ]= nIter;
                point2Used[ pID2 ]= nIter;
                if (inliers)
                    inliers->push_back(std::make_pair(pID1,pID2));
            }
        }
        
    }
    
    return score;
    
}
