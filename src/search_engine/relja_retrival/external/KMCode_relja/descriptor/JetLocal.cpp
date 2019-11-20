#include "jetLocal.h"


void JetLocal::computeComponents(DARY *img){
	if(img==NULL){return;}
	d_scale=c_scale;
	//int mins = (int)(GAUSS_CUTOFF*d_scale);
	//if(!isOK(mins,img->x()-mins,img->y()-mins))return;
	//allocVec(((order+1)*(order+2))>>1);
	allocVec(15);
	if(vec==NULL)return;
	 
	int xi = (int)x;
	int yi = (int)y;
	//state=1;return;
	vec[0] = smoothf(xi, yi, img, d_scale);
	//cout << " xi "<<xi << " yi " << yi << " sc "<<d_scale<< " v0 "<<vec[0]<< endl;//getchar(); 

	double normalize=1.0; 

	if(order>=1){
	  
	  vec[1]=normalize*dXf(xi, yi, img, d_scale);
	  vec[2]=normalize*dYf(xi, yi, img, d_scale);
	  //cout << vec[1] << " " << vec[2]<< " "<< normalize <<  endl;
	}
	if(order>=2){
		normalize*=1.0;
		vec[3]=normalize*dXXf(xi, yi, img, d_scale);
		vec[4]=normalize*dXYf(xi, yi, img, d_scale);
		vec[5]=normalize*dYYf(xi, yi, img, d_scale);
	}
	if(order>=3){
		normalize*=1.0;
		vec[6]=normalize*dXXXf(xi, yi, img, d_scale);
		vec[7]=normalize*dXXYf(xi, yi, img, d_scale);
		vec[8]=normalize*dXYYf(xi, yi, img, d_scale);
		vec[9]=normalize*dYYYf(xi, yi, img, d_scale);
	}
	if(order>=4){
		normalize*=1.0;
		vec[10]=normalize*dXXXXf(xi, yi, img, d_scale);
		vec[11]=normalize*dXXXYf(xi, yi, img, d_scale);
		vec[12]=normalize*dXXYYf(xi, yi, img, d_scale);
		vec[13]=normalize*dXYYYf(xi, yi, img, d_scale);
		vec[14]=normalize*dYYYYf(xi, yi, img, d_scale);
	}
	//normalize=1.0/sqrt(vec[1]*vec[1]+vec[2]*vec[2]);
	//for(int i=0;i<14;i++)vec[i]=vec[i]/normalize;
	state=1;
}

void computeJLDescriptors(DARY *image, vector<CornerDescriptor *> &desc){
    JetLocal * ds=new JetLocal();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rcompute "<< c<< "    "<< flush;;
	ds->copy(desc[c]);
	ds->computeComponents(image);
 	desc[c]->copy((CornerDescriptor*)ds); 
    }
    for(unsigned int c=0;c<desc.size();c++){
      if(!desc[c]->isOK()){
	desc.erase((std::vector<CornerDescriptor*>::iterator)&desc[c]);
	c--;
      }
    }    
}
