#include "complexGauss.h"


double K00(int x, int y, DARY* image_in){
  double real=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr00[j+ksize][i+ksize];
      //cout << " "<< ksize << " " << image_in->fel[y+j][x+i]<< " " << Kr00[j+ksize][i+ksize] << endl;
    }

  return real;
} 
double K11(int x, int y, DARY* image_in){
  double real=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr11[j+ksize][i+ksize];
    }
  return real;
}
double K22(int x, int y, DARY* image_in){
  double real=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr22[j+ksize][i+ksize];
    }
  return real;
}
double K33(int x, int y, DARY* image_in){
  double real=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr33[j+ksize][i+ksize];
    }
  return real;
}
double K10(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr10[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki10[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K21(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr21[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki21[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K32(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr32[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki32[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K20(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr20[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki20[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K31(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr31[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki31[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K42(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr42[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki42[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K30(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr30[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki30[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K41(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr41[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki41[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K40(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr40[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki40[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
} 
double K51(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr51[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki51[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K50(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr50[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki50[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
double K60(int x, int y, DARY* image_in){
  double real=0;
  double imag=0;
  for (int i=-ksize;i<=ksize;i++)
    for (int j=-ksize;j<=ksize;j++){
      real += image_in->fel[y+j][x+i]*Kr60[j+ksize][i+ksize];
      imag += image_in->fel[y+j][x+i]*Ki60[j+ksize][i+ksize];
    }
  return (real*real+imag*imag);
}
 


void ComplexGauss::computeComponents(DARY *img_in){
	if(img_in==NULL){return;}
	allocVec(15);
	if(vec==NULL)return;
		
        DARY * img = new DARY(PATCH_SIZE,PATCH_SIZE);
       	//cout << " OK " <<c_scale<< endl;
      	normalizeAffine(img_in,img);
 
	int xi=img->x()>>1;
	int yi=img->y()>>1;	
	normalize(img,xi,yi,xi);
       	//img->write("imgn.pgm");getchar();
     	
	//vec[0]=K00(xi, yi, img);  
     	vec[0]=K11(xi, yi, img); 
     	vec[1]=K22(xi, yi, img);
     	vec[2]=K33(xi, yi, img);
     	vec[3]=K10(xi, yi, img);
     	vec[4]=K21(xi, yi, img);
     	vec[5]=K32(xi, yi, img);
     	vec[6]=K20(xi, yi, img);
     	vec[7]=K31(xi, yi, img);
     	vec[8]=K42(xi, yi, img);
     	vec[9]=K30(xi, yi, img);
     	vec[10]=K41(xi, yi, img);
     	vec[11]=K40(xi, yi, img);
     	vec[12]=K51(xi, yi, img);
     	vec[13]=K50(xi, yi, img);
     	vec[14]=K60(xi, yi, img);
	
	state=1;
	delete img;
	changeBase(cf_base);
	int cf_pca_size=15;
	//pca(cf_pca_size, cf_pca_avg, cf_pca_base);	
}
 

void computeCGDescriptors(DARY *image, vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    ComplexGauss * ds=new ComplexGauss();
    for(unsigned int c=0;c<desc.size();c++){
      cout << "\rcf descriptor "<< c<< " of "<< desc.size()<< flush;;
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
    cout << endl;  
}

