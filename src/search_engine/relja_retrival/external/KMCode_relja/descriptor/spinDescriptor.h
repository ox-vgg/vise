#ifndef _spinDescriptor_h_
#define _spinDescriptor_h_

#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "spin_base.h"
#include "spin_pca_base.h"



const int SpinNbRings = 5;
const int SpinNbGrLev = 10;

class DllExport SpinDescriptor : public CornerDescriptor{

 protected:
    double computMoment(DARY *img, double p, double q, double a);

 public:
	SpinDescriptor():CornerDescriptor(){;}
	SpinDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	SpinDescriptor(double xin, double yin, double cornerness_in, double scale_in)
	    :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);

};
void computeSpinDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
