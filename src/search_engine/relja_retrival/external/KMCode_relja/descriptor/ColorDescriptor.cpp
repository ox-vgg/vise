#include "colorDescriptor.h"


/**********************************************/
void ColorDescriptor::computeComponents( DARY *image, Zone *zone){
    allocVec(64);
    double nb=0;
    for(int j=0;j<image->y();j++){
	for(int i=0;i<image->x();i++){
	    if(zone->isIn(i,j)){
		nb++;
		vec[(((int)image->rel[j][i])>>6)|
		    ((((int)image->bel[j][i])>>6)<<2)|
		    ((((int)image->gel[j][i])>>6)<<4)]++;
	    }
	}
    }    
    for(int i=0;i<64;i++){
      vec[i]/=nb;
    }
}

double distance(ColorDescriptor *desc1,ColorDescriptor *desc2){
    int size=desc1->getSize();
    double dist=0,diff,sum;
    for(int i=0;i<size;i++){	
      // diff=(desc1->getV(i)-desc2->getV(i));
      // sum=(desc1->getV(i)+desc2->getV(i));
      // if(sum)dist+=(diff*diff)/(sum*sum);
      dist+=fabs(desc1->getV(i)-desc2->getV(i));
    }    
    return dist;
}
