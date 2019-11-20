#ifndef _jetLocalNormal_h_
#define _jetLocalNormal_h_

#include "jetLocalDirectional.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "gaussFilters.h"
#include "jla_base.h"
#include "jla_pca_base.h"

class DllExport JetLocalAffine : public JetLocal{

 protected:

 public:
    JetLocalAffine():JetLocal(){;}
    //JetLocalAffine(Corner *corner_in):CornerDescriptor(corner_in){;}
    JetLocalAffine(double xin, double yin):JetLocal(xin,yin){;}
    JetLocalAffine(double xin, double yin, double cornerness_in, double scale_in)
	:JetLocal(xin,yin,cornerness_in,scale_in){;}
    
    void computeComponents(DARY *image);
	
};
void computeJLADescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
