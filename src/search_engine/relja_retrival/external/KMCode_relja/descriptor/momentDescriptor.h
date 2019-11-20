#ifndef _momentDescriptor_h_
#define _momentDescriptor_h_

#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "mom_base.h"
#include "mom_pca_base.h"

class DllExport MomentDescriptor : public CornerDescriptor{

 protected:
    float computMoment(DARY *img, float p, float q, float a);

 public:
	MomentDescriptor():CornerDescriptor(){;}
	MomentDescriptor(float xin, float yin):CornerDescriptor(xin,yin){;}
	MomentDescriptor(float xin, float yin, float cornerness_in, float scale_in)
	    :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);

};
void computeMomentDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
