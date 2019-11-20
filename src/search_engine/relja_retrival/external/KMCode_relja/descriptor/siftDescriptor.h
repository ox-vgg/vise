#ifndef _siftDescriptor_h_
#define _siftDescriptor_h_

/* David Lowe's code*/
#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "sift_base.h"
#include "sift_pca_base.h"
const int IndexSize=4;
const int OriSize=8;
const int SiftSize=IndexSize*IndexSize*OriSize;

const int IndexSigma=1;
const int MagFactor=2;
const float MaxIndexVal = 0.2;  /* Good value is 0.2 */
class DllExport SiftDescriptor : public CornerDescriptor{

 protected:
 public:
	SiftDescriptor():CornerDescriptor(){;}
	SiftDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	SiftDescriptor(double xin, double yin, double cornerness_in, double scale_in)
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
void computeSiftDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
