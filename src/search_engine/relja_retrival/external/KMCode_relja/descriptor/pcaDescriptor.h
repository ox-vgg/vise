#ifndef _pcaDescriptor_h_
#define _pcaDescriptor_h_

/* David Lowe's code*/
#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "pca_base.h"
#include "pca_eig.h"

const int GPLEN=3042;
const int PCALEN=36;

class DllExport PcaDescriptor : public CornerDescriptor{

 private:

 protected:
 public:
	PcaDescriptor():CornerDescriptor(){;}
	PcaDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	PcaDescriptor(double xin, double yin, double cornerness_in, double scale_in)
	    :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);

};
void computePcaDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
