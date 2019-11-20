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

#ifndef _HOMOGRAPHY_H_
#define _HOMOGRAPHY_H_

#include <string.h>

#include "ellipse.h"


class ellipse;

class homography {
    
    public:
        
        double H[9];
        
        homography(){ setIdentity(); }
        
        homography( double h[] ){ set(h); }
        
        homography( homography const& aHomography ){ set(aHomography.H); }
        
        inline
            homography& operator=(homography const &aHomography){
                if (this!=&aHomography){
                    this->set(aHomography.H);
                }
                return *this;
            }
        
        // affine homography from matching ellipses using the gravity vector
        //homography( vgl_conic ellipse1, vgl_conic ellipse2 );
        homography( ellipse const &ellipse1, ellipse const &ellipse2 );
        
        double getDetAffine() const;
            
        inline void
            exportToDoubleArray( double h[] ) const
                { memcpy(h,H,9*sizeof(double)); }
        
        inline void
            set( const double h[] )
                { memcpy(H,h,9*sizeof(double)); }
        
        inline void setIdentity(){
            H[0]= 1; H[1]= 0; H[2]= 0;
            H[3]= 0; H[4]= 1; H[5]= 0;
            H[6]= 0; H[7]= 0; H[8]= 1;
        }
        
        inline void getInverse( homography &Hinv ){
            double Hinv_[9];
            getInverse(Hinv_);
            Hinv.set( Hinv_ );
        }
        
        inline void getInverse( double Hinv[] ) const {
            homography::getInverse( H, Hinv );
        }
        
        inline static void getInverse( const double H[], double Hinv[] ){
            double invdet = H[0]*H[4]*H[8] - H[0]*H[5]*H[7] - H[3]*H[1]*H[8] + H[3]*H[2]*H[7] + H[6]*H[1]*H[5] - H[6]*H[2]*H[4];
            invdet= 1.0/invdet;
            Hinv[0]=  (H[4]*H[8] - H[5]*H[7])*invdet;
            Hinv[1]= -(H[1]*H[8] - H[2]*H[7])*invdet;
            Hinv[2]=  (H[1]*H[5] - H[2]*H[4])*invdet;
            Hinv[3]= -(H[3]*H[8] - H[5]*H[6])*invdet;
            Hinv[4]=  (H[0]*H[8] - H[2]*H[6])*invdet;
            Hinv[5]= -(H[0]*H[5] - H[2]*H[3])*invdet;
            Hinv[6]=  (H[3]*H[7] - H[4]*H[6])*invdet;
            Hinv[7]= -(H[0]*H[7] - H[1]*H[6])*invdet;
            Hinv[8]=  (H[0]*H[4] - H[1]*H[3])*invdet;
            homography::normLast( Hinv );
        }
        
        inline static void
            normLast( double h[] ){
                h[0]/= h[8]; h[1]/= h[8]; h[2]/= h[8];
                h[3]/= h[8]; h[4]/= h[8]; h[5]/= h[8];
                h[6]/= h[8]; h[7]/= h[8]; h[8]= 1.0;
            }
        
        inline void
            normLast(){ homography::normLast(H); }
        
        inline static void
            affTransform( const double h[], double x, double y, double &xp, double &yp ){
                xp= h[0]*x+h[1]*y+h[2];
                yp= h[3]*x+h[4]*y+h[5];
            }
        
        double getSimEig();
        
    private:
        
        // C=[at, 0; bt, ct], such that C^T C = [a, b; b; c]
        inline static void
            cholesky( double a, double b, double c, double &at, double &bt, double &ct );
        // inv( [a, b; 0, c] )
        inline static void
            lowerTriInv( double a, double b, double c, double &at, double &bt, double &ct );
    
};

#endif
