#ifndef _colorDescriptor_h_
#define _colorDescriptor_h_


#include "../ImageContent/imageContent.h"
#include "histoDescriptor.h"

class DllExport ColorDescriptor: public HistoDescriptor{

 public:

    ColorDescriptor():HistoDescriptor(){;}
    ColorDescriptor(DARY *img_in,Zone *zone_in ){
	computeComponents(img_in,zone_in);}
    void computeComponents(DARY *img_in,Zone *zone);      
};

double distance(ColorDescriptor *desc1,ColorDescriptor *desc2);

#endif
