#include "nonParamDescriptor.h"

double NonParamDescriptor::interpolate(double x_in, double y_in, double** el){
  int arondx = (int)floor(x_in);
  int arondy = (int)floor(y_in);
  double dx = x_in -  arondx;
  double dy = y_in -  arondy;	    

  double interp   = (1.0 - dy)*((1.0 - dx)*el[arondy][arondx] +
			       dx*el[arondy][arondx+1]) +
    dy*((1.0 - dx)*el[arondy+1][arondx] +
    dx *el[arondy+1][arondx+1]);	 
  return interp;

}


int NonParamDescriptor::codeNP(double xi, double yi, double start_angle, 
			       double **image, double radius, int bits)
{
  double pi=interpolate(xi,yi,image);//central pixel
  double p=0;

  double x_;
  double y_;
  int code = 0;
  
  double angle=start_angle;
  double dangle=((2.0*M_PI/bits));
  for(int i=0;i<bits;i++){
      y_=yi+sin(angle)*radius;
      x_=xi+cos(angle)*radius;	 
      //cout <<" xi  " << xi <<" x_  " << x_ << " yi " << yi <<" y_  " << y_ <<endl;
      //cout <<radius<< " angle "<< (180*angle/M_PI) << " x  " << x_-xi <<" y " << y_-yi<< " r "<< (y_-yi)*(y_-yi)+(x_-xi)*(x_-xi) << endl;getchar();
      
      p=interpolate(x_,y_,image);
      code+=((pi<p)?0:1)*(int)pow(2,i);
      angle+=dangle;
  }
  return code;
}

void NonParamDescriptor::computeComponents(DARY *img_in, const double code_dist,	
					   const double code_rad, const int bits){
  if(img_in==NULL){return;}
  int mins = (int)(GAUSS_CUTOFF*c_scale+1);
  if(!isOK(mins,img_in->x()-mins,img_in->y()-mins))return;
  
  d_scale=c_scale;
  allocVec((int)pow(2,bits));
  if(vec==NULL)return;
  double xt,yt; 

  double lecos=cos(-angle);
  double lesin=sin(-angle);
  double scmean=5.0;
  double scal=2*d_scale/scmean;
  
  double m11=scal*(mi11*lecos-mi12*lesin);
  double m12=scal*(mi11*lesin+mi12*lecos);
  double m21=scal*(mi21*lecos-mi22*lesin);
  double m22=scal*(mi21*lesin+mi22*lecos);
  int sizex=2*(int)rint(GAUSS_CUTOFF*scmean)+3;
  DARY* imgn = new DARY(sizex,sizex);
  imgn->interpolate(img_in,x,y,m11,m12,m21,m22);
  xt=imgn->x()>>1;yt=imgn->y()>>1;
  //imgn->write("point1a.pgm");getchar();

  double max_angle = 2*M_PI;
  double angle,x_,y_,dangle=0;
  double i= 3; //starting ring
  int code;
  double patch_rad=xt-1;
  while(i<patch_rad){//i=distance from the corner
    angle = 0;
    //2*atan(1/(sqrt(4*((i*i)/dist)-1))) = angle between i,i for a given triangle (i;i;pix_dist), pix_dist=1;
    dangle=fabs(2*atan(1/(sqrt(4*(i*i)/code_dist-1)))); 
    do{
      y_=yt+sin(angle)*i;
      x_=xt+cos(angle)*i;
      code = codeNP(x_, y_, angle,  imgn->fel, code_rad, bits);
      vec[code]++;
      angle+=dangle;
    }while(angle<max_angle);  
    i+=code_dist;
  }
  delete imgn;
  double sum=0;
  for(int i=0;i<size;i++)sum+=vec[i];
  for(int i=0;i<size;i++)vec[i]/=sum;

  if(!isOK()){
    cout << "x "<<xt << " y  "<< yt << "; 3*sc  "<< 3*d_scale <<" angle "<< angle<< endl;
    state=0;  
  }
  else 	state=1;
 
}


void computeNPDescriptors(DARY *image, vector<CornerDescriptor *> &desc){
  NonParamDescriptor * ds = new NonParamDescriptor();
  for(unsigned int c=0;c<desc.size();c++){
    cout << "\rdescriptor "<< c<< " of "<< desc.size() <<"   "<< flush;
    ds->copy(desc[c]);	
    ds->computeComponents(image);	
    //ds->changeBase(ds->getSize(),baseJLAD);
    desc[c]->copy((CornerDescriptor*)ds); 
  }
  for(unsigned int c=0;c<desc.size();c++){
    if(!desc[c]->isOK()){
      desc.erase(&desc[c]);
      c--;
    }
  }
  cout << endl;
}
