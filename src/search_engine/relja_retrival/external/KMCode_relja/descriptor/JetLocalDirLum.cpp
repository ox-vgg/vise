#include "jetLocalDirLum.h"


void JetLocalDirLum::computeComponents(DARY *img_in){
	if(img_in==NULL){return;}
	int mins = (int)(GAUSS_CUTOFF*c_scale+1);
	if(!isOK(mins,img_in->x()-mins,img_in->y()-mins))return;
	d_scale=c_scale;
	allocVec(((order+1)*(order+2))/2);
	if(vec==NULL)return;
	double xt=x,yt=y; 
	double scmean=7;
        int im_size=2*(int)rint(GAUSS_CUTOFF*scmean)+3;
        DARY * imgn = new DARY(im_size,im_size);      
	double c_scalet=c_scale;
	c_scale=2*c_scale;
       	normalizeAffine(img_in,imgn);
	x=imgn->x()>>1;y=imgn->y()>>1;
        c_scale=d_scale=scmean;
	((JetLocal*)this)->computeComponents(imgn);
	c_scale=d_scale=c_scalet;
	x=xt;y=yt;
	delete imgn;

  double *val = new double[size-3];
  double normal_ = 1.0/sqrt(vec[1]*vec[1]+vec[2]*vec[2]);
  if(order>=2){
      val[0]=vec[3]*normal_;
      val[1]=vec[4]*normal_;
      val[2]=vec[5]*normal_;
  }
  if(order>=3){
      val[3]=vec[6]*normal_;
      val[4]=vec[7]*normal_;
      val[5]=vec[8]*normal_;
      val[6]=vec[9]*normal_;
  }
  if(order>=4){
      val[7]=vec[10]*normal_;
      val[8]=vec[11]*normal_;
      val[9]=vec[12]*normal_;
      val[10]=vec[13]*normal_;
      val[11]=vec[14]*normal_;
  }

  allocVec(size-3);
  for( int v=0;v<size;v++){ 
    vec[v]=val[v];
  }
  delete []val;
  if(!isOK()){
    cout << "x "<<xt << " y  "<< yt << "; 3*sc  "<< 3*d_scale <<" angle "<< angle<< endl;
    state=0;  
  }
  else 	state=1;

}

void computeJLDLDescriptors(DARY *image, vector<CornerDescriptor *> &desc){
  JetLocalDirLum * ds=new JetLocalDirLum();
  for(unsigned int c=0;c<desc.size();c++){
    cout << "\rljla descriptor"<< c<< "    "<< flush;;
    ds->copy(desc[c]);
    ds->computeComponents(image);
    ds->changeBase(ds->getSize(),baseJlaLDog);
    desc[c]->copy((CornerDescriptor*)ds); 
  }
  for(unsigned int c=0;c<desc.size();c++){
    if(!desc[c]->isOK()){
      desc.erase((std::vector<CornerDescriptor*>::iterator)&desc[c]);
      c--;
    }
  }    
  cout<<endl;
}

