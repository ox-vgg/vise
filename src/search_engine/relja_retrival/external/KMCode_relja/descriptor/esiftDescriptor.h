#ifndef _esiftDescriptor_h_
#define _esiftDescriptor_h_

/* David Lowe's code*/
#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "esift_base.h"
#include "esift_pca_base.h"
const int EIndexSize=5; 
const int LEIndexSize=3;
const int EOriSize=12;
const int ESiftSize=EIndexSize*EIndexSize*EOriSize;
const int LESiftSize=LEIndexSize*LEIndexSize*EOriSize;

const float EMaxIndexVal = 0.2;  /* Good value is 0.2 */
class DllExport ESiftDescriptor : public CornerDescriptor{

 protected:
 public:
	ESiftDescriptor():CornerDescriptor(){;}
	ESiftDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	ESiftDescriptor(double xin, double yin, double cornerness_in, double scale_in)
	    :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);
	void PlaceInIndex(float *index,
			  float mag, float ori, float rx, float cx, int eindexSize);
	void AddSample(float *index,
		  DARY *grad, DARY *orim, int r, int c, float rpos, float cpos,
		  float rx, float cx, int eindexSize);
	void KeySample(float *index, 
		  DARY *grad, DARY *ori, int eindexSize);
	void MakeKeypointSample(DARY *grad, DARY *ori, float *gvec, float norm, int eindexSize, int esiftSize);
};
void computeESiftDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
