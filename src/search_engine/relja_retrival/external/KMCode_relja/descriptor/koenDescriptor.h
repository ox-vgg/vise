#ifndef _koenDescriptor_h_
#define _koenDescriptor_h_


#include "jetLocalAffine.h"
#include "koen_base.h"
#include "koen_pca_base.h"

class DllExport KoenDescriptor : public JetLocal {

 private:

 public:
	 KoenDescriptor():JetLocal(){;}
	 //KoenDescriptor(Corner *corner_in):JetLocal(corner_in){;}
	 KoenDescriptor(double xin, double yin):JetLocal(xin,yin){;}
	 KoenDescriptor(double xin, double yin, double cornerness_in, double scale_in)
					 :JetLocal(xin,yin,cornerness_in,scale_in){;}

     void computeComponents(DARY *img_in);

};
void computeKoenDescriptors(DARY *image, vector<CornerDescriptor *> &desc);

#endif
