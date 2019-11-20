#include "jetHist.h"
static int DIM=5;

void JetHist::samplePoint(float x, float y, double dx, double dy, double dxx, double dyy, double dxy, float isize){
  
  float sx=DIM*x/isize; 
  float sy=DIM*y/isize; 
  
  float fsx=floor(sx);
  float fsy=floor(sy);
  
  float xfrac=sx-fsx;
  float yfrac=sy-fsy;
  
  int loc0=DIM*fsy+fsx;
  int loc1=DIM*(fsy+1)+fsx;
  
  Matrix H(2,2),U,D,V;
  H(1,1)=dxx;H(1,2)=dxy;H(2,1)=dxy;H(2,2)=dyy;
  
  float det=dxx*dyy-2*dxy;
  if(det>0)cout << det<< endl;
  // H.svd(U,D,V);
  // if(D(1,1)<0 || D(2,2)<0)cout << endl<<H<< U<< D<< V<< endl;//getchar();
  
  //fdx[loc0]+=
  
  
  
}

void JetHist::computeComponents(DARY *img){
	if(img==NULL){return;}
	d_scale=c_scale;
	int mins = (int)(GAUSS_CUTOFF*d_scale+1);
	if(!isOK(mins,img->x()-mins,img->y()-mins))return;
	//allocVec(((order+1)*(order+2))>>1);
	allocVec(15);
	if(vec==NULL)return;
	
	int xi = (int)x;
	int yi = (int)y;
	d_scale=c_scale;
	double scmean=7;
	int im_size=2*(int)rint(GAUSS_CUTOFF*scmean)+3;
	DARY * imgn = new DARY(im_size,im_size);   
	c_scale=2*c_scale;
	normalizeAffine(img,imgn); 
	c_scale=d_scale;
	normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);
	DARY *dx=new DARY(imgn->y(),imgn->x());
	DARY *dy=new DARY(imgn->y(),imgn->x());
	DARY *dxx=new DARY(imgn->y(),imgn->x());
	DARY *dyy=new DARY(imgn->y(),imgn->x());
	DARY *dxy=new DARY(imgn->y(),imgn->x());
	int dim=DIM*DIM;
	fdx=new float[dim];
	fdy=new float[dim];
	fdxx=new float[dim];
	fdyy=new float[dim];
	fdxy=new float[dim];
	for(int i=0;i<dim;i++){
	  fdx[i]=0;fdy[i]=0;fdxx[i]=0;fdyy[i]=0;fdxy[i]=0;
	}
	
	dX6(imgn,dx);
	dY6(imgn,dy);
	dXX7(imgn,dxx);
	dYY7(imgn,dyy);
	dXY7(imgn,dxy);
	//dx->write("dx.pgm");dy->write("dy.pgm");dxx->write("dxx.pgm");dyy->write("dyy.pgm");dxy->write("dxy.pgm");getchar();
	int isize=(float)imgn->y()-2;
	for(int j=2;j<isize;j++){
	  for(int i=2;i<isize;i++){
	    samplePoint((float)i,(float)j,dx->fel[j][i],dy->fel[j][i],dxx->fel[j][i],dyy->fel[j][i],dxy->fel[j][i],isize-2);
	  }
	}
	delete []fdx;delete []fdy;delete []fdxx;delete []fdyy;delete []fdxy;
	state=1;delete imgn;delete dx;delete dy;delete dxx;delete dyy;delete dxy;
	
}

void computeJHDescriptors(DARY *image, vector<CornerDescriptor *> &desc){
    JetHist * ds=new JetHist();
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
