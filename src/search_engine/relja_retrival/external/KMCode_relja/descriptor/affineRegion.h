#ifndef _affineRegion_h_
#define _affineRegion_h_

#include "cornerDescriptor.h"
#include "../homography/homography.h"
#include "cornerDetector.h"
#include "edgeDetector.h"

void computeAngle(DARY *img_in, vector<CornerDescriptor *> &cor);
void drawAffineCorners(DARY *image, vector<CornerDescriptor*> cor, char *filename, float color);
void findAffineRegion(DARY *image, vector<CornerDescriptor *> &cor);
void interpolate(DARY *im_in, float m_x, float m_y, DARY *img, Matrix *rot);
void displayEllipse(DARY *im, CornerDescriptor *cor, float color);
void displayCircle(DARY *im, CornerDescriptor *cor, float color);
void findScaleRegion(DARY *image, vector<CornerDescriptor*> &cor);
void findScales(DARY *img, vector<CornerDescriptor*> &cor,vector<CornerDescriptor*> &corout);
void hitEdge(DARY *img, DARY *edge, vector<CornerDescriptor*> &cor, vector<Segment*> seg, vector<CornerDescriptor*> &corout, int flag);
#endif
