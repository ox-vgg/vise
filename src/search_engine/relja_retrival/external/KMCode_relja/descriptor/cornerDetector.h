#ifndef _cornerDetector_h_
#define _cornerDetector_h_

#include "cornerDescriptor.h"

void getEigen(float a, float b, float c, float d, float &l1, float &l2);

void hessian(DARY* image, vector<CornerDescriptor*>&corners,
	   float threshold,
	   float derive_sigma);

void findMax(DARY* harris_im, vector<CornerDescriptor*>&corners,
	     float threshold,float scale, int type);

void multi_harris_mask(DARY* image, DARY* mask, 
		       vector<CornerDescriptor*>&corners, float har_threshold,
		       float min_scale, float max_scale, float step);
 
void multi_harris(DARY* image, vector<CornerDescriptor*>&corners, 
		  float har_threshold, 
		  float min_scale, float max_scale, float step);
void multi_scale_har(DARY* image, vector<CornerDescriptor*>&corners, 
		  float har_threshold, float step, int aff);
void multi_scale_hes(DARY* image, vector<CornerDescriptor*>&corners, 
		  float har_threshold, float step, int aff);
void multi_hessian(DARY* image, vector<CornerDescriptor*>&corners, 
		   float lap_threshold,
		   float min_scale, float max_scale, float step);


void harris(DARY* image, vector<CornerDescriptor*>&corners,
	    float threshold,
	    float derive_sigma,
	    float sigma, 
	    float alpha);

void removeNeighbours(vector<CornerDescriptor*> &corners);

void drawColorCorners(DARY *im, vector<CornerDescriptor*> &cor, 
		      const char *filename, float color);
 void drawCorners(DARY *im_in, vector<CornerDescriptor*> &cor, 
		  const char *filename, float color);

void selectSimilarPoints(vector<CornerDescriptor*>&cor, 
			 vector<float> scales);

void computLaplacian(DARY* image, vector<CornerDescriptor*>&corners, 
		     float lap_threshold, float step);
#endif
