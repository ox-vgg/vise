#ifndef _jetLocal_h_
#define _jetLocal_h_

#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"

class DllExport JetLocal : public CornerDescriptor{

 protected:

 public:
	JetLocal():CornerDescriptor(){;}
	JetLocal(double xin, double yin):CornerDescriptor(xin,yin){;}
	JetLocal(double xin, double yin, double cornerness_in, double scale_in)
	  :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	
	void computeComponents(DARY *image);
	
};
void computeJLDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
