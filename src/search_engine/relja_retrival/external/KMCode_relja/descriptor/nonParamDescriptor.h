#ifndef _nonParamDescriptor_h_
#define _nonParamDescriptor_h_


#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"

class DllExport NonParamDescriptor : public CornerDescriptor {

 private:
	double interpolate(double x_in, double y_in, double** el);
	int codeNP(double xi, double yi, double start_angle, 
			   double **image, double radius, int bits);

 public:
	 NonParamDescriptor():CornerDescriptor(){;}
	 NonParamDescriptor(double xin, double yin, double cornerness_in, double scale_in)
	     :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}

	void computeComponents(DARY *image,  const double code_dst=1.5, 
			       const double code_rad=2, const int bits=7);

};
void computeNPDescriptors(DARY *image, vector<CornerDescriptor *> &desc);
#endif
