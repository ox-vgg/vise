#ifndef _edgeDescriptor_h_
#define _edgeDescriptor_h_

#include "cornerDescriptor.h"
#include "../ImageContent/imageContent.h"
#include "../gauss_iir/gauss_iir.h"
#include "edgeDetector.h"

class DllExport EdgeDescriptor : public CornerDescriptor{

 protected:

 public:
	EdgeDescriptor():CornerDescriptor(){;}
	EdgeDescriptor(double xin, double yin):CornerDescriptor(xin,yin){;}
	EdgeDescriptor(double xin, double yin, double cornerness_in, double scale_in)
	    :CornerDescriptor(xin,yin,cornerness_in,scale_in){;}
	EdgeDescriptor(double xin, double yin, double scale_in, double ori_in, int ext_in)
	    :CornerDescriptor(xin,yin,scale_in, ori_in,ext_in){;}
void computeComponents(DARY *gr1, DARY *ogr1,DARY *lap1, DARY *olap1,
				       int x1, int y1, int size1,
				       DARY*gr2, DARY*ogr2, DARY*lap2, DARY*olap2, 
				       int x2, int y2, int size2);
};

#endif
