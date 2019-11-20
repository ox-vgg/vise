 
#ifndef _homography_
#define _homography_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <vector>
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "../stats/stats.h"
#include "../descriptor/descriptor.h"



void displayEllipse(DARY *im, Matrix *M, int x, int y, int xc, int yc, float color);
void getAffMat(CornerDescriptor * cor, Matrix *H,  Matrix &AFF);

int checkHomog(CornerDescriptor* cor1,CornerDescriptor* cor2,Matrix *H, float dist_max, float &dist);
int checkEpip(CornerDescriptor* cor1,CornerDescriptor* cor2,Matrix *F,float dist_max, float &dist);
void loadVibesCorners(const char* points1, vector<CornerDescriptor*> &cor1);
void matchDescriptorsNearest(vector<CornerDescriptor*> &cor1, 
			     vector<CornerDescriptor*> &cor2,
			     const float d_dist_max);
void checkMatchDescriptors( vector<CornerDescriptor*> &cor1, 
		       vector<CornerDescriptor*> &cor2,  Matrix * invCovMat, Matrix * geom,
			    const float d_dist_max);

float  checkAngleByHomography(vector<CornerDescriptor*> &cor1,  
			       vector<CornerDescriptor*> &cor2, 
			       Matrix *H,float max_angle_error);
void  checkScaleByHomography(vector<CornerDescriptor*> &cor1,  
			       vector<CornerDescriptor*> &cor2, 
			       Matrix *H,float max_angle_error);
void checkPointsByHomography(vector<CornerDescriptor *> &cor1, vector<CornerDescriptor *> &cor2, 
			     Matrix *F, float max_dist);
void checkPointsByEpipolar(vector<CornerDescriptor *> &cor1, vector<CornerDescriptor *> &cor2, 
			     Matrix *F, float max_dist);
float matchSimilarDescriptors( vector<CornerDescriptor*> &cor1, 
			      vector<CornerDescriptor*> &cor2,  
			      Matrix * geom,
			      const float d_dist_max, ofstream &out);
void projectAffineCorners(vector<CornerDescriptor *> &cor, vector<CornerDescriptor *> &cor2, Matrix *H);
void getHPoint(float x_in, float y_in, Matrix *H, float &x_out, float &y_out);
void computeHomographyPrec(vector<CornerDescriptor *> &cor1, 
		       vector<CornerDescriptor *> &cor2, 
		       Matrix *H,float max_dist);
void computeHomography(vector<CornerDescriptor *> cor1, 
		       vector<CornerDescriptor *> cor2, 
		       Matrix *H);
void matchScaleByHomography(vector<CornerDescriptor *> &cor1, vector<CornerDescriptor *> &cor2,
			     vector<CornerDescriptor *> &cor1_tmp, vector<CornerDescriptor *> &cor2_tmp, 
			    Matrix *H, float max_scale_error);
void matchPointsByHomography(vector<CornerDescriptor *> &cor1, vector<CornerDescriptor *> &cor2,
			     vector<CornerDescriptor *> &cor1_tmp, vector<CornerDescriptor *> &cor2_tmp, 
			     Matrix *H, float max_dist);
void computeEpipolarMatrix(vector<CornerDescriptor *> cor1, 
			   vector<CornerDescriptor *> cor2, 
			   Matrix *F);
void getScaleAngle(Matrix *H, float &scale, float &angle);
void removeOutLiers(vector<CornerDescriptor*> &cor1, Matrix *H, float width, float height, float margin);
void removeFloatMatches(vector<CornerDescriptor*> &cor1, 
			 vector<CornerDescriptor*> &cor2);
float  matchAngleByHomography(vector<CornerDescriptor*> &cor1,  
			       vector<CornerDescriptor*> &cor2, 
			       Matrix *H,float max_angle_error);
void matchAffinePoints(vector<CornerDescriptor *> &cor1,vector<CornerDescriptor *> &cor2,
		       vector<CornerDescriptor *> &cor1_match,vector<CornerDescriptor *> &cor2_match, Matrix *H, float max_error);
void matchDescriptorsSimilar(vector<CornerDescriptor*> &cor1, 
			     vector<CornerDescriptor*> &cor2, 	 
			     Matrix *covMat, Matrix *geom,
			     const float d_dist_max);
void removeSimilarPoints(vector<CornerDescriptor *> &cor1);
void matchDescriptorsNearest( vector<CornerDescriptor*> &cor1, 
			      vector<CornerDescriptor*> &cor2,  
			      Matrix * invCovMat, Matrix * geom,
			      const float d_dist_max);
void SSD(DARY *img_in1, vector<CornerDescriptor *> &cor1, 
	 DARY *img_in2, vector<CornerDescriptor *> &cor2, float dist_cc);
void checkMatches( vector<CornerDescriptor*> &cor1, vector<CornerDescriptor*> &cor2, Matrix *H, 
		   float dist_max,int model);
#endif




