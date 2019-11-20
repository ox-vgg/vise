#ifndef _corner_h_
#define _corner_h_
 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//typedef ofstream outfstream;
//typedef ifstream inputfstream;
using namespace std;
#include<vector>


#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"


#define ALPHA 0.05
#define PATCH_SIZE 41
#define FALSE 0;
#define TRUE 1;

#define BPOINT asm("int $3;");

template<class T>
class watchpoint {
public:
  T variable_;

  watchpoint<T>&
  operator=(float rhs)
  {
    if (rhs != 1000.0)
      BPOINT
    variable_ = rhs;
    return *this;
  }

  watchpoint<T>&
  operator+=(float rhs)
  {
    BPOINT
    variable_ += rhs;
    return *this;
  }
  
  watchpoint<T>&
  operator/=(float rhs)
  {
    BPOINT
    variable_ /= rhs;
    return *this;
  }
  
  watchpoint<T>&
  operator*=(float rhs)
  {
    BPOINT
    variable_ *= rhs;
    return *this;
  }
  
  watchpoint<T>&
  operator-=(float rhs)
  {
    BPOINT
    variable_ -= rhs;
    return *this;
  }

  operator T() const
  {
    return variable_;
  }
};

template<class T>
inline
istream&
operator >>(istream& is, watchpoint<T>& wp)
{
  is >> wp.variable_;
  return is;
}

extern DARY *patch_mask;
extern float PATCH_SUM;
void initPatchMask(int size);
//float PATCH_SUM;
class DllExport Corner{
    
 protected:
  int type;
  float x,y,l1,l2,lap,sdx2,sdy2,sdxdy, mi11,mi12,mi21,mi22; 
  float cornerness;
  float c_scale, int_sig, der_sig;
  int int_lev, der_lev;
  float angle;
  //watchpoint<float> angle;
  int state;
  int extr;
    
 public:
    /****CONSTRUCTORS***/
    Corner(void);
    Corner(Corner *);
    Corner(float xin, float yin);
    Corner(float xin, float yin, float cornerness_in);
    Corner(float xin, float yin, float cornerness_in, float scale_in);
    Corner(float xin, float yin, float cornerness_in, float scale_in, float angle_in);
    Corner(float xin, float yin, float scale_in, float angle_in, int extr_in);
    Corner(float xin, float yin, float cornerness_in, float scale_in, float l1_in, float l2_in);
    Corner(float xin, float yin ,int type_in);
    inline void init(){type=0; angle=1000;x=0;y=0; cornerness=0;c_scale=0;state=0;angle=1000;lap=0;extr=0;l1=0.1;l2=0;mi11=mi22=1;mi12=mi21=0;}

    inline int   const  getType() const { return type;}

    /****CORNERNESS***/
    inline float   const  getCornerness(void) const { return cornerness;}
    inline void setCornerness(float cornerness_in) {cornerness=cornerness_in;}
    inline void setMi(float m11,float m12,float m21,float m22) {mi11=m11;mi12=m12;mi21=m21;mi22=m22;}
    inline void setMi(float m11,float m12,float m21,float m22,float e1,float e2) 
	{mi11=m11;mi12=m12;mi21=m21;mi22=m22;l1=e1;l2=e2;}
    inline float   const  getMi11() const { return mi11;}
    inline float   const  getMi12() const { return mi12;}
    inline float   const  getMi21() const { return mi21;}
    inline float   const  getMi22() const { return mi22;}
    
    /****LOCALISATION***/
    inline float   const  getX() const { return x;}
    inline float   const  getY() const { return y;}
    inline void     setX_Y(float x_in,float y_in)  {x=x_in;y=y_in;}

    /****LOCALISATION***/
    inline float   const  getL1() const { return l1;}
    inline float   const  getL2() const { return l2;}
    inline int   const  getIntLev() const { return int_lev;}
    inline int   const  getDerLev() const { return der_lev;}
    inline void setIntLev(int lev_in) {int_lev=lev_in;}
    inline void setDerLev(int lev_in) {der_lev=lev_in;}


    /****LAPLACIAN VALUE***/
    inline float   const  getLap() const { return lap;}
    inline void setLap(float lap_in) {lap=lap_in;}

    /****LAPLACIAN VALUE***/
    inline void setExtr() {extr=1;}
    inline void setMax() {extr=1;}
    inline void setMin() {extr=-1;}
    inline int   const  isExtr() const { return extr;}
    

    /****ANGLE*****/
    inline float   const  getAngle() const { return angle;}
    inline void    setAngle(const float angle_in){ angle=angle_in;}
    void computeAngle( DARY *angle_im);
    void computeHistAngle(DARY *grad_im, DARY *angle_im);
    void computeHistAngle(DARY *img);
    void computeAngleMean(DARY *grad_im, DARY *angle_im);
    void computeAngleLowe(DARY *grad_im, DARY *angle_im);
    void computeAngleSimple(DARY *grad_im, DARY *angle_im);
    void computeAngleAtan2(DARY *img);
    bool normalizeAffine( DARY *image, DARY *patch);

    /*****SCALE******/
    inline float   const  getCornerScale() const { return c_scale;}
    inline void     setCornerScale(const float scale_in){c_scale=scale_in;}
    inline void     setIntSig(const float sig_in){int_sig=sig_in;}
    inline float   const  getIntSig() const { return int_sig;}
    inline void     setDerSig(const float sig_in){der_sig=sig_in;}
    inline float   const  getDerSig() const { return der_sig;}

 
    inline void     setType(int type_in)  {type=type_in;}
    int isOK(const int minim, const int max_x, const int max_y);
    inline int const isOK() const {return state;}

    ~Corner();     
    
};

void GradOriImages_s1(DARY *im, DARY *grad, DARY *ori);
void GradOriImages(DARY *im, DARY *grad, DARY *ori);  
void LapOriImages(DARY *im, DARY *grad, DARY *ori);  
void interpolate(DARY *im_in, float m_x, float m_y, 
		 DARY *img, float vec0x,float vec0y,float vec1x,float vec1y);
void normalize(DARY * img,int x, int y, float radius);
#endif
