// Definition de la classe ImageContent
#ifndef _imageContent_h_
#define _imageContent_h_

#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include "png.h"
using namespace std;


#define COL 5
#define GRAY 1
#define FLOAT 2
#define UCHAR 1
#define UCHARFLOAT 3
#define CFLOAT 8
#define CUCHAR 5
#define CUCHARFLOAT 12

const int PNG_BYTES_TO_CHECK = 4;
const int ERROR = -1;

// added by @Abhishek to support compilation in Mac/CentOS
#define png_voidp_NULL    (png_voidp)NULL
#define png_infopp_NULL   (png_infopp)NULL
#define png_set_gray_1_2_4_to_8(arg) png_set_expand_gray_1_2_4_to_8(arg)
typedef unsigned int uint;


class ImageContent;
typedef ImageContent DARY;
class ImageContent {
   
  private :
    uint x_size,y_size;
    uint tsize;
    void writePGM(const char *nom,unsigned char *buff, const char* comments);
    void write(const char *nom, const char* comments);
    void initFloat(uint ,uint);	   
    void initUChar(uint ,uint);	   
    void init3Float(uint ,uint);	   
    void init3UChar(uint ,uint);	   
    int buftype;
  
  public :
    float **fel;
    float **felr;
    float **felg;
    float **felb;
    unsigned char **bel;
    unsigned char **belr;
    unsigned char **belg;
    unsigned char **belb;
    ImageContent(void){};   
    ImageContent(const char *);	   
	  ImageContent(ImageContent *im);	   
	  ImageContent(uint y_size_in,uint x_size_in){initFloat( y_size_in, x_size_in);};	   
	  ImageContent(int y_size_in ,int x_size_in){initFloat((uint)y_size_in,(uint)x_size_in);};	   
	  ImageContent(uint ,uint, const char *);	   
	  ImageContent(uint ,uint, const char *, float);	   
	  ImageContent(uint y_size_in, uint x_size_in, float val){initFloat( y_size_in, x_size_in);set(val);};	   

	 ~ImageContent();

	  inline uint  const  x() const { return x_size;}
	  inline uint  const  y() const { return y_size;}
	  inline uint  const  size() const { return tsize;}
	  int const getType() const { return buftype;}
	  void write(const char *nom);
	  void writePNG(const char* name);
	  void writeR(const char *nom);
	  void writeG(const char *nom);
	  void writeB(const char *nom);
	  void RGB2xyY();
	  void RGB2lbrg();
	  void RGB2rgb();
	  void float2char();
	  void char2float();
	  void flipH();
	  void flipV();
	  void toGRAY();
	  void set(float);
	  void set(DARY*);
	  void normalize(float min_in, float max_in);
	  void scale(DARY *im_in, float scalex, float scaley);
	  float getValue(float x, float y);
	  bool interpolate(DARY *sface, float m_x, float m_y, 
			   float scalex, float scaley, float angle);
	  bool interpolate(DARY *im_in, float m_x, float m_y, float vec0x, float vec0y,
			   float vec1x, float vec1y);
};
       


 
#endif
