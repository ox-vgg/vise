#ifndef _shapeDescriptor_h_
#define _shapeDescriptor_h_

/* David Lowe's code*/
#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "sc_base.h"
#include "sc_pca_base.h"
const int SIndexSize=4;
const int SOriSize=8;
const int ShapeSize=SIndexSize*SIndexSize*SOriSize;

const int SIndexSigma=1;
const int SMagFactor=2;
const float SMaxIndexVal = 0.2;  /* Good value is 0.2 */
class DllExport ShapeDescriptor : public CornerDescriptor{

 protected:
 public:
	ShapeDescriptor():CornerDescriptor(){;}
	ShapeDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	ShapeDescriptor(double xin, double yin, double cornerness_in, double scale_in)
	    :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);
	void PlaceInIndex(float *index,
			  float mag, float ori, float rx, float cx);
	void AddSample(float *index,
		  DARY *grad, DARY *orim, int r, int c, float rpos, float cpos,
		  float rx, float cx);
	void KeySample(float *index, 
		  DARY *grad, DARY *ori);
	void MakeKeypointSample(DARY *grad, DARY *ori);
};
void computeShapeDescriptors(DARY *image, vector<CornerDescriptor *> &desc);
void canny(DARY* img, DARY* grad, DARY* ori, float lower_threshold, float higher_threshold);

#endif
