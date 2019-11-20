#ifndef _jetHist_h_
#define _jetHist_h_

#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"

class DllExport JetHist : public CornerDescriptor{

 protected:
    float *fdx; 
    float *fdy; 
    float *fdxx; 
    float *fdyy; 
    float *fdxy; 
    
 public:
	JetHist():CornerDescriptor(){;}
	JetHist(double xin, double yin):CornerDescriptor(xin,yin){;}
	JetHist(double xin, double yin, double cornerness_in, double scale_in)
	  :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);
	void samplePoint(float x, float y, double dx, double dy, double dxx, double dyy, double dxy, float isize);
	
};
void computeJHDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
