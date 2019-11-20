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

#include "ellipse.h"

#include <stdint.h>
#include <vector>

#include <Eigen/Dense>



ellipse::ellipse( double aX,  double aY, double aA, double aB, double aC ){
    x=aX; y=aY;
    a=aA; b=aB; c=aC;
}



void
ellipse::getCentres( std::vector<ellipse> const &ellipses,
                     double *&x, double *&y  ){
    
    x= new double[ellipses.size()];
    y= new double[ellipses.size()];
    uint32_t iEllipse= 0;
    
    for ( std::vector<ellipse>::const_iterator it=ellipses.begin(); it!=ellipses.end(); ++it, ++iEllipse ){
        x[iEllipse]= it->x;
        y[iEllipse]= it->y;
    }
        
    
}



void
ellipse::getCentres( std::vector<ellipse> const &ellipses1,
                     std::vector<ellipse> const &ellipses2,
                     std::vector< std::pair<uint32_t, uint32_t> > const &inds,
                     double *&x1, double *&y1, double *&x2, double *&y2,
                     double *&areaDiffSq ){
    
    x1= new double[inds.size()];
    y1= new double[inds.size()];
    x2= new double[inds.size()];
    y2= new double[inds.size()];
    areaDiffSq= new double[inds.size()];
    uint32_t iPair= 0;
    
    for ( std::vector< std::pair<uint32_t, uint32_t> >::const_iterator it=inds.begin();
          it!=inds.end();
          ++it, ++iPair ){
        x1[iPair]= ellipses1[ it->first ].x;
        y1[iPair]= ellipses1[ it->first ].y;
        x2[iPair]= ellipses2[ it->second ].x;
        y2[iPair]= ellipses2[ it->second ].y;
        areaDiffSq[iPair]= ellipses2[ it->second ].getPropAreaSq() / ellipses1[ it->first ].getPropAreaSq();
    }
    
}






void
ellipse::transformAffine( double Haff[] ){
    
    // transform centre
    double x_new= (Haff[0]*x+Haff[1]*y+Haff[2])/Haff[8];
    double y_new= (Haff[3]*x+Haff[4]*y+Haff[5])/Haff[8];
    
    // transform a,b,c
    typedef Eigen::Matrix<double, 3, 3> matrix3d;
    matrix3d C0;
    C0 <<   a,   b,  0.0,
            b,   c,  0.0,
          0.0, 0.0, -1.0;

    homography Haff_(Haff);
    double Hinv_[9];
    Haff_.getInverse( Hinv_ );
    
    matrix3d Hinv;
    Hinv<< Hinv_[0], Hinv_[1], Hinv_[2],
           Hinv_[3], Hinv_[4], Hinv_[5],
           Hinv_[6], Hinv_[7], Hinv_[8];
    
    Hinv/= Hinv.coeff(2,2);
    
    // translate Hinv (same as translating C but more efficient)
    Hinv.coeffRef(0,2)-= x;
    Hinv.coeffRef(1,2)-= y;
    matrix3d C0t;
    C0t.noalias()= Hinv.transpose() * C0 * Hinv;
    
    // now translate C0t to Ct by xt,yt, but the following is not affected
    a= C0t.coeff(0,0);
    b= C0t.coeff(0,1);
    c= C0t.coeff(1,1);
    x= x_new;
    y= y_new;
    
#if 0
    // this is how it used to be done in OpenCV, not updating though so it might be obsolete..
    
    C0.at<double>(0,0)=   a; C0.at<double>(0,1)=   b; C0.at<double>(0,2)=  0.0;
    C0.at<double>(1,0)=   b; C0.at<double>(1,1)=   c; C0.at<double>(1,2)=  0.0;
    C0.at<double>(2,0)= 0.0; C0.at<double>(2,1)= 0.0; C0.at<double>(2,2)= -1.0;
    
    cv::Mat Hinv(3,3, cv::DataType<double>::type);
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
            Hinv.at<double>(i,j)= Hinv_[i*3+j];
    
    Hinv/= Hinv.at<double>(2,2);
    
    Hinv.at<double>(0,2)-= x;
    Hinv.at<double>(1,2)-= y;
    cv::Mat C0t= Hinv.t()*C0*Hinv;
    
    a= C0t.at<double>(0,0);
    b= C0t.at<double>(0,1);
    c= C0t.at<double>(1,1);
    x= x_new;
    y= y_new;
#endif
    
}
