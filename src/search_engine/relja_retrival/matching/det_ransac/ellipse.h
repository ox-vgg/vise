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

#ifndef _ELLIPSE_H_
#define _ELLIPSE_H_

#include <stdint.h>
#include <vector>
#include <cmath>
#include <string.h>

#include "homography.h"

class homography;

class ellipse {
    
    public:
        
        double x, y, a, b, c;
        
        ellipse( double aX,  double aY, double aA, double aB, double aC );
        
        ellipse() : x(0),y(0),a(0),b(0),c(0) {}
        
        inline void
            getCentre( double &aX, double &aY ) const;
        
        static void
            getCentres( std::vector<ellipse> const &ellipses,
                        double *&x, double *&y );
        
        static void
            getCentres( std::vector<ellipse> const &ellipses1,
                        std::vector<ellipse> const &ellipses2,
                        std::vector< std::pair<uint32_t, uint32_t> > const &inds,
                        double *&x1, double *&y1, double *&x2, double *&y2,
                        double *&areaDiffSq );
        
        // assuming C=[a b 0; b c 0; 0 0 -1] => Cdual= [ inv([a b; b c]) 0; 0 0 -1 ]
        inline void
            getDual( double &adual, double &bdual, double &cdual );
        
        // assumes Haff=[p q 0; r s 0; 0 0 t]
        void
            transformAffine( double Haff[] );
        
        // angle is the angle of the two tangents, i.e. tangents are (homogeneous) lines of the form [-sin(angle); cos(angle); d]
        inline double
            getDistBetweenTangents( double cosAngle, double sinAngle );
        
        // a*c-b^2
        inline double
            getPropAreaSq() const;
        
        bool
            operator==( ellipse const &el2 ) const {
                return fabs(x-el2.x)<1e-5 && fabs(y-el2.y)<1e-5 &&
                       fabs(a-el2.a)<1e-5 && fabs(b-el2.b)<1e-5 && fabs(c-el2.c)<1e-5;
            }
        
        
        // ------- for saving/loading
        
        inline void
            setMem( char *&dest ) const {
                float x_= static_cast<float>(x),
                      y_= static_cast<float>(y),
                      a_= static_cast<float>(a),
                      b_= static_cast<float>(b),
                      c_= static_cast<float>(c);
                memcpy(dest, &x_, sizeof(float));
                memcpy(dest+  sizeof(float), &y_, sizeof(float));
                memcpy(dest+2*sizeof(float), &a_, sizeof(float));
                memcpy(dest+3*sizeof(float), &b_, sizeof(float));
                memcpy(dest+4*sizeof(float), &c_, sizeof(float));
                dest+= getSize();
            }
        
        inline void
            setMem( char *&source ) {
                float x_= static_cast<float>(x),
                      y_= static_cast<float>(y),
                      a_= static_cast<float>(a),
                      b_= static_cast<float>(b),
                      c_= static_cast<float>(c);
                memcpy( &x_, source                , sizeof(float) ); x= static_cast<double>(x_);
                memcpy( &y_, source+  sizeof(float), sizeof(float) ); y= static_cast<double>(y_);
                memcpy( &a_, source+2*sizeof(float), sizeof(float) ); a= static_cast<double>(a_);
                memcpy( &b_, source+3*sizeof(float), sizeof(float) ); b= static_cast<double>(b_);
                memcpy( &c_, source+4*sizeof(float), sizeof(float) ); c= static_cast<double>(c_);
                source+= getSize();
            }
        
        inline void
            set( double aX, double aY, double aA, double aB, double aC ){
                x= aX; y= aY; a= aA; b= aB; c= aC;
            }
        
        inline void
            get ( double &aX, double &aY, double &aA, double &aB, double &aC ) const {
                aX= x; aY= y; aA= a; aB= b; aC= c;
            }
        
        inline static uint32_t
            getSize(){ return 5*sizeof(float); }
        
};



inline void
ellipse::getDual( double &adual, double &bdual, double &cdual ){
    double det= a*c-b*b;
    adual=  c/det;
    bdual= -b/det;
    cdual=  a/det;
}



inline double
ellipse::getDistBetweenTangents( double cosAngle, double sinAngle ){
    double det= a*c-b*b;
    double duala= c/det, dualb= -b/det, dualc= a/det;
    // dualC=[ duala dualb 0; dualb dualc 0; 0 0 -1 ]
    // l=[-sin(angle); cos(angle); d]
    // l'*dualC*l=0 =>
    // d= duala*duala*sinAngle - 2*dualb*sinAngle*cosAngle + dualc*dualc*cosAngle*cosAngle;
    // return 2*sqrt(d)
    return 2*std::sqrt( duala*duala*sinAngle - 2*dualb*sinAngle*cosAngle + dualc*dualc*cosAngle*cosAngle );
}



inline double
ellipse::getPropAreaSq() const{
    return a*c-b*b;
}

#endif
