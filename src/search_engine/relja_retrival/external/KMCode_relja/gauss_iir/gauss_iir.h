 
#ifndef _gauss_
#define _gauss_
#ifdef WIN32
#ifndef DllExport
#define DllExport   __declspec( dllexport )
#define DllImport   __declspec( dllimport )
#endif
#endif
#ifndef WIN32
#define DllExport  
#define DllImport  
#endif

#ifndef M_2PI
//#define M_PI  3.1415926537
#define M_2PI 6.2831853072
#endif
typedef unsigned int uint;
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "../ImageContent/imageContent.h"

/************************************************************************
   structure used to store the coefficients nii+, nii-, dii+, dii- 
   and the normalisation factor 'scale'
   see [1] p. 9 - 11; p. 13, 14 
*************************************************************************/

#define  GAUSS_CUTOFF 3

void  dXY9(DARY* image_in, DARY* smooth_image);
void  dXX9(DARY* image_in, DARY* smooth_image);
void  dYY9(DARY* image_in, DARY* smooth_image);
void  smooth9(DARY* image_in, DARY* smooth_image);
void  dXY7(DARY* image_in, DARY* smooth_image);
void  dXX7(DARY* image_in, DARY* smooth_image);
void  dYY7(DARY* image_in, DARY* smooth_image);
void  dX6(DARY* image_in, DARY* smooth_image);
void  dY6(DARY* image_in, DARY* smooth_image);
void  dX4(DARY* image_in, DARY* smooth_image);
void  dY4(DARY* image_in, DARY* smooth_image);
void dX2(DARY* image_in, DARY* dximage);
void dY2(DARY* image_in, DARY* dyimage);
void HorConv3(DARY *image,  DARY *result);
void VerConv3(DARY *image,  DARY *result);
void grad2(DARY* image_in, DARY* dyimage);
void dXX3(DARY* image_in, DARY* dximage);
void dYY3(DARY* image_in, DARY* dyimage);
void dXX5(DARY* image_in, DARY* dximage);
void dYY5(DARY* image_in, DARY* dyimage);
void dXX_YY3(DARY* image_in, DARY* dyimage);
void  smooth3(DARY* image_in, DARY* smooth_image);
void  smooth5(DARY* image_in, DARY* smooth_image);
void  smoothSqrt(DARY* image_in, DARY* smooth_image);
DllExport float smooth(int x, int y, DARY* image_in, float scale);
DllExport float dX(int x, int y, DARY* image_in, float scale);
DllExport float dY(int x, int y, DARY* image_in, float scale);
DllExport float dXX(int x, int y, DARY* image_in, float scale);
DllExport float dYY(int x, int y, DARY* image_in, float scale);
DllExport float dXY(int x, int y, DARY* image_in, float scale);

DllExport void smooth (DARY* image_in, DARY* out_image, float scale);
DllExport void dX (DARY* image_in, DARY* out_image, float scale);
DllExport void dY (DARY* image_in, DARY* out_image, float scale);
DllExport void dXX (DARY* image_in, DARY* out_image, float scale);
DllExport void dXY (DARY* image_in, DARY* out_image, float scale);
DllExport void dYY (DARY* image_in, DARY* out_image, float scale);
DllExport void dXX_YY (DARY* image_in, DARY* out_image, float scale);
DllExport void dX (DARY* image_in,  DARY* image_out, float scalex, float scaley);	
DllExport void dY (DARY* image_in,  DARY* image_out, float scalex, float scaley);	
DllExport float smooth(int x, int y, DARY* image_in, float scalex, float scaley);
DllExport void  smooth(DARY* image_in, DARY* smooth_image, float scalex, float scaley);


DllExport float smoothf(int x, int y, DARY* image_in, float scale);
DllExport float dXf(int x, int y, DARY* image_in, float scale);
DllExport float dYf(int x, int y, DARY* image_in, float scale);
DllExport float dXXf(int x, int y, DARY* image_in, float scale);
DllExport float dXYf(int x, int y, DARY* image_in, float scale);
DllExport float dYYf(int x, int y, DARY* image_in, float scale);
DllExport float dXX_YYf(int x, int y, DARY* image_in, float scale);
DllExport float dXXXf(int x, int y, DARY* image_in, float scale);
DllExport float dXXYf(int x, int y, DARY* image_in, float scale);
DllExport float dXYYf(int x, int y, DARY* image_in, float scale);
DllExport float dYYYf(int x, int y, DARY* image_in, float scale);
DllExport float dXXXXf(int x, int y, DARY* image_in, float scale);
DllExport float dXXXYf(int x, int y, DARY* image_in, float scale);
DllExport float dXXYYf(int x, int y, DARY* image_in, float scale);
DllExport float dXYYYf(int x, int y, DARY* image_in, float scale);
DllExport float dYYYYf(int x, int y, DARY* image_in, float scale);
void drawGauss(DARY* image_in,int x, int y, float scale);



#endif




