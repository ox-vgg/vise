#ifndef _edgeDetector_h_
#define _edgeDetector_h_
#include "cornerDescriptor.h"
//#include "descriptor.h"

class ChainPoint{
 public:
    int x, y;
    float ori,ddori;
    ChainPoint(int in_x, int in_y, float in_ori){x=in_x; y=in_y;ori=in_ori;}
    ~ChainPoint(){}
};



class Segment{
 protected:
 public:
    vector<ChainPoint * > chain;
    /****CONSTRUCTORS***/
    Segment(void);    
    Segment(int x, int y, float ori){chain.push_back(new ChainPoint(x,y,ori));}    
    ~Segment(){chain.clear();}    
};

void matchBikes(DARY *img1, DARY *img2, vector<CornerDescriptor *> &corners1 , vector<CornerDescriptor *> &corners2);
void edgeFeatureDetector(DARY *img, vector<CornerDescriptor *> &cor);
void drawSegments(DARY *segments ,vector<Segment*> seg, const char *filename);
void detectEdgeSegments(DARY *edge,vector<Segment*> &seg);
void cannyEdges(DARY *img, DARY *edge,  float sigma, 
		float higher_threshold,
		float lower_threshold);
void cannyEdgesGrad(DARY *img, DARY *edge,  float sigma, 
		    float lower_threshold,
		    float higher_threshold);
void detectPointsOnEdges( vector<Segment*> &seg, vector<CornerDescriptor*> &cor);

#endif


