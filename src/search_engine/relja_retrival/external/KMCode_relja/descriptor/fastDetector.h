#ifndef _fastDetector_h_
#define _fastDetector_h_

#include "cornerDescriptor.h"
#include "cornerDetector.h"

/******************HARRIS DETECTOR**************/

void multi_harris_fast(DARY* image, vector<CornerDescriptor*>&corners, 
		       float har_threshold);
void multi_hessian_fast(DARY* image, vector<CornerDescriptor*>&corners, 
			float lap_threshold);

void multi_harris_hessian_fast(DARY* image, vector<CornerDescriptor*>&corners, 
			     float har_threshold,  float lap_threshold);

void findAffineRegion_fast(DARY *image, vector<CornerDescriptor*> &cor);

#endif
