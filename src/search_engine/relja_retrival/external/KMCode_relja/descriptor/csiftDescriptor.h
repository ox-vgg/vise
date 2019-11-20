#ifndef _csiftDescriptor_h_
#define _csiftDescriptor_h_

/* David Lowe's code*/
#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
const int CRadIndexSize=3;
const int CAngIndexSize=4;
const int COriSize=8;
const int CSiftSize=CRadIndexSize*CAngIndexSize*COriSize;

const float CMaxIndexVal = 0.2;  /* Good value is 0.2 */
class DllExport CSiftDescriptor : public CornerDescriptor{

 protected:
 public:
	CSiftDescriptor():CornerDescriptor(){;}
	CSiftDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	CSiftDescriptor(double xin, double yin, double cornerness_in, double scale_in)
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
void computeCSiftDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
