#ifndef _ccDescriptor_h_
#define _ccDescriptor_h_

#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "cc_base.h"
#include "cc_pca_base.h"

static int CCSize=9;
class DllExport CCDescriptor : public CornerDescriptor{

 protected:
    double computMoment(DARY *img, double p, double q, double a);

 public:
	CCDescriptor():CornerDescriptor(){;}
	CCDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	CCDescriptor(double xin, double yin, double cornerness_in, double scale_in)
	    :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);

};
void computeCCDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
