#ifndef _complexGauss_h_
#define _complexGauss_h_

#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "complexFilters.h"
#include "cf_base.h"
#include "cf_pca_base.h"

class DllExport ComplexGauss : public CornerDescriptor{

 protected:

 public:
	ComplexGauss():CornerDescriptor(){;}
	ComplexGauss(double xin, double yin):CornerDescriptor(xin,yin){;}
	ComplexGauss(double xin, double yin, double cornerness_in, double scale_in)
	  :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);
	
};
void computeCGDescriptors(DARY *image, vector<CornerDescriptor *> &desc);


















#endif
