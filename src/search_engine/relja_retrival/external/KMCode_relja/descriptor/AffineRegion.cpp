#include "affineRegion.h"

void displayCircle(DARY *im, CornerDescriptor *cor, float colori){

  int x=(int)cor->getX();
  int y=(int)cor->getY();
  for(int i=-2;i<=2;i++){
    if(y>0 && y<im->y() && y+i>0 && y+i<im->y() && x>0 && x<im->x() && x+i>0 && x+i< im->x())
      im->fel[y][x+i]=im->fel[y+i][x]=colori;
  }
  float sizef=(cor->getCornerScale());
  float thick=1;
  //cout << "OK1 " << x << "  " << y << "  " << sizef << endl;
  float color=0;
  for(int nb=0;nb<2;nb++){ 
  for(float sizefi = sizef-thick;sizefi<=sizef+thick;sizefi+=0.2){
    
    int size=(int)rint(sizefi);
    for(int i=0;i<=size;i++){
      int yi=(int)rint(sqrt((sizefi*sizefi-i*i)));
      if(x+i>0 && x+i<im->x()){
	if(y+yi<im->y()&& y+yi>0)
	  im->fel[y+yi][x+i]=color;
	if(y-yi>0 && y-yi<im->y()) 
	  im->fel[y-yi][x+i]=color;
      }
      if(x-i>0 && x-i<im->x()){
	if(y+yi<im->y()&& y+yi>0)
	  im->fel[y+yi][x-i]=color;
	if(y-yi>0 && y-yi<im->y()) 
	  im->fel[y-yi][x-i]=color;
      }	  
      if(y+i<im->y() && y+i>0){
	if(x+yi<im->x()&& x+yi>0)
	  im->fel[y+i][x+yi]=color;
	if(x-yi>0 && x-yi<im->x())
	  im->fel[y+i][x-yi]=color;
      } 
      if(y-i<im->y() && y-i>0){
	if(x+yi<im->x()&& x+yi>0)
	  im->fel[y-i][x+yi]=color;
	if(x-yi>0 && x-yi<im->x())
	  im->fel[y-i][x-yi]=color;
      }
    } 
  }  
  thick=0;color=colori;
  }

  //cout << "OK2 " << endl;

  //imn->write("imn.pgm");cout << "wrote" << endl;getchar();
  

}
void displayEllipse(DARY *im, CornerDescriptor *cor, float colori){
  
  Matrix M(2,2),U,V,D,rot;
  M(1,1)=cor->getMi11();
  M(1,2)=cor->getMi12();
  M(2,1)=cor->getMi21();
  M(2,2)=cor->getMi22();
  M.svd(U,D,V);
  rot=V;
  float scalex = cor->getCornerScale();
  //if(scalex<8)return;
  float a,b,scale=1;
  float color=0;
  //  for(float sf=0.98;sf<1.01;sf+=0.005){   
  float sf=1;
  float v=sf*scale*D(1,1)*scalex;
  float n=sf*scale*D(2,2)*scalex;
  //cout << scalex << " scale " << endl;getchar();
  int x=(int)cor->getX();
  int y=(int)cor->getY();

  for(int i=-2;i<=2;i++){
    if(y>0 && y<im->y() && y+i>0 && y+i<im->y() && x>0 && x<im->x() && x+i>0 && x+i< im->x())
      im->fel[y][x+i]=im->fel[y+i][x]=colori;
  }

  float thick=0;
  for(int nb=0;nb<2;nb++){
    for(float a=v-thick;a<=v+thick;a+=0.4)  
      for(float b=n-thick;b<=n+thick;b+=0.4){  
      
  //cout << a << " " << b << endl;
  float b2=b*b,a2=a*a;
  float b2_a2=b2/a2;
  float a2_b2=a2/b2;
  int xe = (int)cor->getX();
  int ye = (int)cor->getY();
  
  float xt, yt;
  int px,py;

  for(float i=0;i<a;i++){               
    yt=sqrt((float)(b2-b2_a2*i*i));
    px=xe+(int)rint(rot(1,1)*i+rot(1,2)*yt);
    py=ye+(int)rint(rot(2,1)*i+rot(2,2)*yt);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(rot(1,1)*i-rot(1,2)*yt);
    py=ye+(int)rint(rot(2,1)*i-rot(2,2)*yt);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(-rot(1,1)*i+rot(1,2)*yt);
    py=ye+(int)rint(-rot(2,1)*i+rot(2,2)*yt);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(-rot(1,1)*i-rot(1,2)*yt);
    py=ye+(int)rint(-rot(2,1)*i-rot(2,2)*yt);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
  }
  for(float j=0;j<b;j++){               
    xt=(int)rint(sqrt((float)(a2-a2_b2*j*j)));
    px=xe+(int)rint(rot(1,1)*xt+rot(1,2)*j);
    py=ye+(int)rint(rot(2,1)*xt+rot(2,2)*j);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(rot(1,1)*xt-rot(1,2)*j);
    py=ye+(int)rint(rot(2,1)*xt-rot(2,2)*j);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(-rot(1,1)*xt+rot(1,2)*j);
    py=ye+(int)rint(-rot(2,1)*xt+rot(2,2)*j);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(-rot(1,1)*xt-rot(1,2)*j);
    py=ye+(int)rint(-rot(2,1)*xt-rot(2,2)*j);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
  }
  }
  thick=0;
  color=colori;
  }
}

void displayEllipse(DARY *im, int xe, int ye, float scalex, float m11, float m12, float m21, float m22, float color){
  
  Matrix M(2,2),U,V,D,rot;
  M(1,1)=m11;
  M(1,2)=m12;
  M(2,1)=m21;
  M(2,2)=m22;
  M.svd(U,D,V);
  rot=V;
  // <<" DDD "<< D << endl;
  // cout << " l1 "<< ((m11+m22)+sqrt((m11+m22)*(m11+m22)-4*(m11*m22-m12*m21)))/2.0<< endl;
  //cout << " l2 "<< ((m11+m22)-sqrt((m11+m22)*(m11+m22)-4*(m11*m22-m12*m21)))/2.0<< endl;

  float a,b;
  // a=1.0/sqrt(D(1,1));b=1.0/sqrt(D(2,2));
  a=1*D(1,1)*scalex;b=1*D(2,2)*scalex;
  // cout << a << " " << b << endl;
  float b2=b*b,a2=a*a;
  float b2_a2=b2/a2;
  float a2_b2=a2/b2;
  
  float xt, yt;
  int px,py;
  //if(xe-2>=0 && xe+2<im->x() && ye-2>=0 && ye+2<im->y()){
  for(int j=xe-2;j<=xe+2;j++)
    for(int i=0;i<=0;i++){
      if(j>=0 && j<im->x() && ye+i>=0 && ye+i<im->y())
	im->fel[ye+i][j]=color;
    }  
  for(int j=ye-2;j<=ye+2;j++)
    for(int i=0;i<=0;i++){
      if(xe+i>=0 && xe+i<im->x() && j>=0 && j<im->y())
	im->fel[j][xe+i]=color;
    }
  //}
  for(float i=0;i<a;i++){               
    yt=sqrt((float)(b2-b2_a2*i*i));
    px=xe+(int)rint(rot(1,1)*i+rot(1,2)*yt);
    py=ye+(int)rint(rot(2,1)*i+rot(2,2)*yt);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(rot(1,1)*i-rot(1,2)*yt);
    py=ye+(int)rint(rot(2,1)*i-rot(2,2)*yt);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(-rot(1,1)*i+rot(1,2)*yt);
    py=ye+(int)rint(-rot(2,1)*i+rot(2,2)*yt);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(-rot(1,1)*i-rot(1,2)*yt);
    py=ye+(int)rint(-rot(2,1)*i-rot(2,2)*yt);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
  }
  for(float j=1;j<b;j++){               
    xt=(int)rint(sqrt((float)(a2-a2_b2*j*j)));
    px=xe+(int)rint(rot(1,1)*xt+rot(1,2)*j);
    py=ye+(int)rint(rot(2,1)*xt+rot(2,2)*j);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(rot(1,1)*xt-rot(1,2)*j);
    py=ye+(int)rint(rot(2,1)*xt-rot(2,2)*j);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(-rot(1,1)*xt+rot(1,2)*j);
    py=ye+(int)rint(-rot(2,1)*xt+rot(2,2)*j);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
    
    px=xe+(int)rint(-rot(1,1)*xt-rot(1,2)*j);
    py=ye+(int)rint(-rot(2,1)*xt-rot(2,2)*j);
    if(px>=0 && px<im->x() && py>=0 && py<im->y())
      im->fel[py][px]=color;
  }
}




void displayEllipse(DARY *im, Matrix *M, int x, int y, float scalex){
  Matrix Mi;
  Mi =(*M);// M->inverse(); 
  int  size=(int)(3*rint(scalex)),xi,yi;
  float color=255;
  Matrix X(2,1),Xe;
  for(int i=-2;i<=2;i++){
    im->fel[y+i][x]=im->fel[y][x+i]=255;
  }
  for(int i=-size;i<=size;i++){
      X(1,1)=i;
      X(2,1)=(int)rint(sqrt((float)(size*size-i*i)));
      Xe=Mi*X;
      xi=(int)rint(Xe(1,1));
      yi=(int)rint(Xe(2,1));
      if(y+yi>0 && y+yi<im->y() && x+xi>0 && x+xi<im->x())
	im->fel[y+yi][x+xi]=color;
      if(y-yi>0 && y-yi<im->y()&& x-xi>0 && x-xi<im->x())
	im->fel[y-yi][x-xi]=color;
      X(1,1)=X(2,1);
      X(2,1)=i;
      Xe=Mi*X;
      xi=(int)rint(Xe(1,1));
      yi=(int)rint(Xe(2,1));
      if(y+yi>0 && y+yi<im->y() && x+xi>0 && x+xi<im->x())
	im->fel[y+yi][x+xi]=color;
      if( y-yi>0 && y-yi<im->y()&& x-xi>0 && x-xi<im->x())
	im->fel[y-yi][x-xi]=color;
  }
}

void drawAffineCorners(DARY *image, vector<CornerDescriptor*> cor, char *filename, float color){
    DARY *im = new DARY(image);
    color=400;
    for(uint i=0;i<cor.size();i++){   
	float sc=cor[i]->getCornerScale();
      //displayCircle(im, cor[i], color);	
	//for(float j=0;j<=0.8;j+=0.01){   
	  //cor[i]->setCornerScale(sc+j);
	  if(cor[i]->getMi11()==1 && cor[i]->getMi22()==1)displayCircle(im, cor[i], color);
	  else  displayEllipse(im, cor[i], color);
	  //}	
	cor[i]->setCornerScale(sc); 
    } 
    //im->write(filename);
    
    DARY *cim = new DARY(im->y(),im->x(),"3uchar");

    for(int j=0;j<cim->y();j++){
      for(int i=0;i<cim->x();i++){
	if(im->fel[j][i]==400){
	  cim->belr[j][i]=255;
	  cim->belg[j][i]=255;
	  cim->belb[j][i]=0;
	}else{
	  cim->belr[j][i]=(unsigned char)im->fel[j][i];
	  cim->belg[j][i]=(unsigned char)im->fel[j][i];
	  cim->belb[j][i]=(unsigned char)im->fel[j][i];

	} 
      }
    }    

    cim->writePNG(filename);
    delete im;delete cim;
}

void computeAngle(DARY *img_in, CornerDescriptor * cor){

	float x = (cor->getX());
	float y = (cor->getY());
	float d_scale=cor->getCornerScale();
	float scmean=10.0;
	float scal=2*d_scale/scmean;
	int im_size=2*(int)rint(GAUSS_CUTOFF*scmean)+1;
	DARY * img = new DARY(im_size,im_size); 
	float a=scal*cor->getMi11();
	float b=scal*cor->getMi12();
	float c=scal*cor->getMi22();

	//cout << x << " " << y << " " << a << " " << b << " " << c<<endl;
	img->interpolate(img_in, x, y, a,b,b,c);
	//img->write("tmp_img.pgm");getchar();
	int xi=img->x()>>1;int yi=img->y()>>1; 
       	normalize(img,xi,yi,(float)xi);
	//img->write("cor.pgm");getchar();
        DARY * fx = new DARY(im_size,im_size);      
        DARY * fy = new DARY(im_size,im_size);      
        //DARY * an = new DARY(im_size,im_size);      
	dX(img, fx, scmean/1.8);
	dY(img, fy, scmean/1.8);
	float t1,t2;
	for(int row = 0; row < im_size; row++)
	  for(int col = 0; col < im_size; col++){
	      t1=fx->fel[row][col];
	      t2=fy->fel[row][col];
	      img->fel[row][col]=atan2(t2,t1);
	  }
	delete fx;delete fy;
	cor->setX_Y(xi,yi);
	cor->setCornerScale(scmean);
	cor->computeAngle(img);
	cor->setCornerScale(d_scale);
	cor->setX_Y(x,y);
	delete img;
}

void computeAngle(DARY *img_in, vector<CornerDescriptor *> &cor){
    for(uint i=0;i<cor.size();i++){
	cout << "\rangle "<<i << " of "<<cor.size()<< "   "<< flush; 
	//computeAngle(img_in, cor[i]);
        cor[i]->computeHistAngle(img_in);
    }
    cout << endl;
}

void displayCircle(DARY *im,int x, int y, float scalex){
  int  size=(int)(rint(scalex)),yi;
  float color=255;
  for(int i=-2;i<=2;i++)im->fel[y+i][x]=im->fel[y][x+i]=color;
  for(int i=-size;i<=size;i++){
    yi=(int)rint(sqrt((float)(size*size-i*i)));	
    im->fel[y+yi][x+i]=im->fel[y-yi][x+i]=color;
    im->fel[y+i][x+yi]=im->fel[y+i][x-yi]=color;  
  }
}
int findAffineRegion(DARY *image, CornerDescriptor * cor);

void findAffineRegion(DARY *image, vector<CornerDescriptor*> &cor){
    int nb=0;uint size=cor.size();
  for(int i=0;i<(int)cor.size();i++){    
    if(//cor[i]->getCornerScale()>2 && 
       cor[i]->getCornerScale()<25
       //&& cor[i]->getX()>10 &&cor[i]->getX()<image->x()-10  
       //&& cor[i]->getY()>10 &&cor[i]->getY()<image->y()-10 
       )
      {
	cout <<nb << " of "<< size << " CORNER aff " << i <<" " << cor[i]->getX();
	cout  << " " << cor[i]->getY();
	cout << "  " << cor[i]->getCornerScale() << " type " << cor[i]->getType();
	cout<<endl;
	int l=findAffineRegion(image,cor[i]);
	//computeAngle(image, cor[i]);int l=1;
	nb++;
	if(l!=0){
	  cout <<nb << " of "<< size << " CORNER aff " << i <<" " << cor[i]->getX();
	  cout  << " " << cor[i]->getY();
	  cout << "  " << cor[i]->getCornerScale()  << "  " <<  cor[i]->getMi11();
	  cout<<"   l "<<l<<endl;
	}else { cor.erase((std::vector<CornerDescriptor*>::iterator)&cor[i]);i--;}
      } else {cor.erase((std::vector<CornerDescriptor*>::iterator)&cor[i]);i--;}
    //nb++;	
  }
  cout << "nb  circ corners "<< nb << endl;
}







float searchMax(char *method, DARY *image, CornerDescriptor * cor, Matrix M);


void write(DARY *im,char *file, int x , int y){    
    for(int i=-2;i<3;i++){
	im->fel[y+i][x]=255;im->fel[y][x+i]=0;
    }
    im->write(file);
}

/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
float func(char *method, int midx, DARY *img, float scalex){
  float res=0;
  if(!strcmp(method,"har2")){
    float dxx,dyy,dxy,AB,CC,determinant,trace;
    dxx=scalex*scalex*dXXf(midx,midx,img,scalex);
    dyy=scalex*scalex*dYYf(midx,midx,img,scalex);
    dxy=scalex*scalex*dXYf(midx,midx,img,scalex);
    CC=dxy*dxy;
    AB=dxx*dyy;
    determinant = AB - (CC);
    trace = dxx + dyy;
    res = (determinant) - 0.05 * (trace*trace);
  } else if(!strcmp(method,"lapf")){
    res=(scalex*scalex*dXX_YYf(midx,midx,img,scalex));
  }
  return res;
}

float searchIntegScale(char *method, DARY *image, CornerDescriptor * cor, Matrix U){  
  float scalexc=cor->getIntSig(),scale;
  float pointx=cor->getX();
  float pointy=cor->getY();
  float maxsc=35.0/scalexc;
  float lmax=1.6;
  lmax=(maxsc<lmax)?maxsc:lmax;
  float lmin=0.4;
  float step=1.123;
  float thres=3;//laplace thres
  int  sizex=(int)(2*rint(GAUSS_CUTOFF*lmax*scalexc)+1);
  int  midx=sizex>>1;
  DARY * img=new DARY(sizex,sizex);
  img->interpolate(image,pointx,pointy,U(1,1),U(1,2),U(2,1),U(2,2));
  //img->write("scale.pgm")
  //int go_on=1;
  float H,H0=10000,H1=10000,H2=10000;
  vector<float> maxims;  
  vector<float> minims;  
  scale=lmin*scalexc;
  H1 = func(method,midx,img,scale); 
  //cout << "scale " << scale << " H " << H1 << endl;
  lmin*=step;
  scale=lmin*scalexc;
  H2 = func(method,midx,img,scale); 
  //cout << "scale " << scale << " H " << H2 << endl;
  lmin*=step;
  for(float l=lmin;l<lmax;l*=step){
    scale=l*scalexc;
    H = func(method,midx,img,scale);
    //cout << "scale " << scale << " H " << H << endl;
    H0=H1;
    H1=H2;
    H2=H;
    if(H1>H0 && H1>H2 && fabs(H1)>thres)maxims.push_back(scale/step);
    else if(H1<H0 && H1<H2 && fabs(H1)>thres)minims.push_back(-scale/step);          
  }
  delete img;
  float best_sc=0,best_rap=200,rap;
  if(cor->isExtr()==1){
      for(int i=0;i<(int)maxims.size();i++){
	  rap=maxims[i]/scalexc; 
	  if(rap<1)rap=1.0/rap;
	  if(rap<best_rap)best_sc=maxims[i];
      }
  } 
  if(cor->isExtr()==-1){
      for(int i=0;i<(int)minims.size();i++){
	  rap=fabs(minims[i])/scalexc;
	  if(rap<1)rap=1.0/rap;
	  if(rap<best_rap)best_sc=minims[i];
      }      
  }
  if(best_sc==0){
      for(int i=0;i<(int)maxims.size();i++){
	  rap=maxims[i]/scalexc;
	  if(rap<1)rap=1.0/rap;
	  if(rap<best_rap)best_sc=maxims[i];
      }
      for(int i=0;i<(int)minims.size();i++){
	  rap=fabs(minims[i])/scalexc;
	  if(rap<1)rap=1.0/rap;
	  if(rap<best_rap)best_sc=minims[i];
      }
  }  

  
  if(best_sc!=0){
      if(best_sc<0)cor->setMin();
      else cor->setMax();
      cor->setIntSig(fabs(best_sc));      
      return fabs(best_sc);          
  } else   
      return scalexc;
}


int harris_mi(DARY *image_in,Matrix *mi,int &dxe,int &dye,
	      float derive_sigma,
	      float sigma, 
	      float alpha, int fast);

float searchDerivScale( DARY *image, float sigma, Matrix *mi);

void normD(Matrix &D){
  D.tabMat[2][2]=(1.0/sqrt(D.tabMat[2][2]));
  D.tabMat[1][1]=(1.0/sqrt(D.tabMat[1][1]));
  float c=sqrt(D.tabMat[2][2]*D.tabMat[1][1]);
  //c=D.tabMat[2][2];
  D.tabMat[2][2]/=c;
  D.tabMat[1][1]/=c;   
}


void eigen(Matrix M, float &l1, float &l2){
       float trace = M(1,1)+M(2,2);
       float delta = sqrt(trace*trace-4*(M(1,1)*M(2,2)-M(2,1)*M(1,2)));
		    
       l1 = (trace+delta)/2.0;
       l2 = (trace-delta)/2.0;
}

void invMi(Matrix *mi){
  Matrix miL,miU;
  miL=mi->choleskiL();
  cout <<"miL1" <<endl<< 1.0/(miL(1,1))<< endl;
  cout <<"miL2" <<endl<< 1.0/(miL(2,2))<< endl;
  
  miU=mi->choleskiU();

}

void getMi(DARY *img, Matrix *mi, int x, int y, float sigma,float derive_sigma){
    int row_nb= img->y();
    int col_nb= img->y();    
    float  t1,t2;
    DARY  *fx= new DARY(row_nb, col_nb);	
    DARY  *fy= new DARY(row_nb, col_nb);	
    DARY  *fxy  = new DARY(row_nb, col_nb);
    dX(img, fx, derive_sigma);
    dY(img, fy, derive_sigma);
    float norm=derive_sigma;//pow(derive_sigma,1.3);
    for (int row = 0; row < row_nb; row++)
	for (int col = 0; col < col_nb; col++){
	    t1 = norm*fx->fel[row][col];
	    t2 = norm*fy->fel[row][col];
	    fx->fel[row][col] = t1*t1;
	    fy->fel[row][col] = t2*t2;
	    fxy->fel[row][col] = t1*t2;
	}           
    mi->tabMat[1][1]= smoothf(x,y,fx, sigma);
    mi->tabMat[2][2]= smoothf(x,y,fy, sigma);
    mi->tabMat[2][1]= mi->tabMat[1][2]=smoothf(x,y,fxy, sigma);   
    delete fx;delete fy;delete fxy;
} 
int brown(DARY *image_in, Matrix *mi,int &dxe,int &dye,float derive_sigma,float sigma);

int findAffineRegion(DARY *image, CornerDescriptor * cor){

  float derive_sigma=cor->getCornerScale();

  float sigma=derive_sigma*1.4;
  cor->setIntSig(sigma);      
  int sizex;
  DARY *img;
  /*
  float color=255;
  DARY *img_disp=new DARY(image);
  displayEllipse(img_disp, cor,color);
  img_disp->write("disp.pgm");
  cout << "imag disp" << endl;
  getchar();
  */

  Matrix *mi=new Matrix(2,2,0.0);
  Matrix U(2,2,0.0),D(2,2,0.0),V(2,2,0.0),Ui(2,2,0.0),X(2,2,1.0);

  float pointx=cor->getX();
  float pointy=cor->getY();

  U.tabMat[1][1]=1;
  U.tabMat[2][2]=1;
  int go_on=1; 
  int l,ok;
  float dx=0,dy=0;
  float ang1_act=100,ang1_bef=100,ang2_bef=100,ang2_act=100,dangle_act=100,dangle_bef=100,dangle_max=0.027;
  int dxe=0,dye=0;
  pointx=cor->getX();
  pointy=cor->getY();
  float l1,l2;
  float eigen_ratio_act=0.1,eigen_ratio_bef=0.1,deigen=0.05;
  //  static float nba=0;  
  for(l=0;l<50 && go_on;l++){
    derive_sigma=1.0*searchIntegScale("lapf",image, cor,U);
    sigma=1.4*derive_sigma;
    if(sigma==0){
	cout << "no maxima "<< endl;
	return 0;
    }      
    sizex=(int)(2*rint(1.5*GAUSS_CUTOFF*sigma)+1);
    if(sizex<25)sizex=25;
    img=new DARY(sizex,sizex);
    img->interpolate(image,pointx,pointy,U(1,1),U(1,2),U(2,1),U(2,2));
    //image->write("patch.pgm");getchar();
    //derive_sigma=sigma/1.4;
    //derive_sigma=searchDerivScale(img,  sigma, mi);

    //brown(img, mi, dxe, dye, derive_sigma, sigma);
    if(cor->getType()==1){
	ok=harris_mi(img, mi, dxe, dye, derive_sigma, sigma, ALPHA,0);
	//cout << "har " << ok<< endl;
    }
    else if(cor->getType()==2){
      //cout << "brown " << endl;
	ok=brown(img, mi, dxe, dye, derive_sigma, sigma);
    }
    else {
	cout << "wrong point type " << endl;
	return 0;
    }

    //img->write("img.pgm");
    dx=U(1,1)*dxe+U(1,2)*dye;dy=U(2,1)*dxe+U(2,2)*dye;
    //    if(dx!=0)
    pointx+=dx;pointy+=dy;
    cor->setX_Y(pointx,pointy);

    //interpolate(image,pointx,pointy,img,&U);
    //getMi(img, mi, img->x()>>1, img->y()>>1, sigma, derive_sigma);


    mi->svd(Ui,D,V);  
    normD(D);  
    //cout << D<< endl;
    eigen_ratio_bef=eigen_ratio_act;
    eigen_ratio_act=1-D(1,1)/D(2,2);
    U=(V*D*V.transpose())*U;  
    //    U.svd(Ui,D,V);
    //l1=sqrt(D(2,2)*D(1,1));D(2,2)/=l1;D(1,1)/=l1;
    //D(2,2)=D(2,2)*1.4/D(1,1);D(1,1)=1.4;

    //    U=(Ui*D*V.transpose());  
    eigen(U,l1,l2);

    if(l>20 || (l2/l1>8) || ok==0) {
	delete mi;
	delete img;
	cout << "not converging "<< l<< " ok " << ok << " scale_det "<<cor->getCornerScale()<<  " scale_sel "<< derive_sigma << endl;
	return 0;
    }
    /*
      ang1_bef=ang1_act;
     ang1_act=acos(D(2,2)/D(1,1));    

     ang2_bef=ang2_act;
     ang2_act=acos(V(1,1));

     dangle_bef=dangle_act;
     _act=fabs(ang2_act-ang2_bef)+fabs(ang1_act-ang1_bef);
    //    if(dangle_act<dangle_max && dangle_bef<dangle_max){
          
    cout << l <<". step"<< endl;
    cout << "U"<< endl <<U;
    cout << "D"<< endl <<D;
    cout << "derive "<< derive_sigma << " integ "<< sigma << " extr "<<cor->isExtr() << endl;
    cout << "dx "<< dx<< " dy "<< dy << endl;
      
    cout << "eigen "<<  eigen_ratio_act<< endl;
    cout << "dangle1 " << 180*(ang1_act-ang1_bef);    
    cout << " dangle2 " << 180*(ang2_act-ang2_bef) << endl; 
    cout << "dangle_bef " << 180*dangle_bef; 
    cout << " dangle_act " << 180*dangle_act << endl;     
 

  cor->setMi(U(1,1),U(1,2),U(2,1),U(2,2));
  //delete  img_disp;
  //img_disp=new DARY(image);
  cor->setCornerScale(sigma);
  displayEllipse(img_disp, cor,color);
  img_disp->write("disp.pgm");
  if(color==0)exit(0);
  if('c'==getchar())color=0.0;
*/
    if(eigen_ratio_act<deigen && eigen_ratio_bef<deigen){
	go_on=0;
	//      if(l>0 && fabs(ang1_act-ang1_bef)<fabs(ang2_act-ang2_bef))cout << "alarm"<< endl;
    }    
    delete img;
  } 
  delete mi;//delete &Ui;delete &D;delete &V;delete &X; 
  
  U.svd(Ui,D,V);
 
  if((D(1,1)/D(2,2))>8) {
    cout << "too long "<<D(1,1)/D(2,2)<< " "<< l<<endl;
      return 0;
  }
  U=V*D*V.transpose();
  //cout << "U(1,1) " << U(1,1)<< endl;
  cor->setMi(U(1,1),U(1,2),U(2,1),U(2,2));  
  cor->setCornerScale(derive_sigma);
  computeAngle(image, cor);
  return l;
}


void getAnisotropy( DARY *img, DARY *fx, DARY *fy, DARY *fxy, float derive_sigma, float sigma, Matrix &mi){
    int row_nb= img->y();
    int col_nb= img->y();
    int  midx,midy;
    float  t1,t2;
    midx=col_nb>>1;
    midy=row_nb>>1;
    //Matrix U,V,D;
    dX(img, fx, derive_sigma);
    dY(img, fy, derive_sigma);
    float norm=derive_sigma;//pow(derive_sigma,1.3);
    for (int row = 0; row < row_nb; row++)
	for (int col = 0; col < col_nb; col++){
	    t1 = norm*fx->fel[row][col];
	    t2 = norm*fy->fel[row][col];
	    fx->fel[row][col] = t1*t1;
	    fy->fel[row][col] = t2*t2;
	    fxy->fel[row][col] = t1*t2;
	}           
    mi(1,1)= smoothf(midx,midy,fx, sigma);
    mi(2,2)=smoothf(midx,midy,fy, sigma);
    mi(1,2)=mi(2,1)=smoothf(midx,midy,fxy, sigma);    
}
/****** HARRIS WITHOUT ANGLE IMAGE ***********/
float searchDerivScale( DARY *img, float sigma, Matrix *mi){
    int row_nb=img->y();
    int col_nb=img->x();
   DARY  *fx= new DARY(row_nb, col_nb);	
   DARY  *fy= new DARY(row_nb, col_nb);	
   DARY  *fxy  = new DARY(row_nb, col_nb);
   Matrix mit(2,2);
   float A,Amax=0,ds=0;
   float s_min=sigma/2.1;
   float s_max=sigma/1.5;
   float l1,l2;
   for(float derive_sigma=s_max;derive_sigma>s_min;derive_sigma/=1.1){
       getAnisotropy( img, fx, fy, fxy, derive_sigma,sigma,mit);
       eigen(mit,l1,l2);
       A=l2/l1;
       //       cout <<  "sigma "<< sigma  <<  " derive_sigma "<< derive_sigma << " D "<<A<< " L "<< l1<< "  " << l2<< endl;
       if(A>Amax){
	   Amax=A;       
	   ds=derive_sigma;
	   mi->tabMat[1][1]=mit(1,1);mi->tabMat[2][2]=mit(2,2);
	   mi->tabMat[2][1]=mit(2,1);mi->tabMat[1][2]=mit(1,2);
       }
   }
   delete fx; delete fy, delete fxy;
   return ds;
   
}

/****** HARRIS WITHOUT ANGLE IMAGE***********/
int brown(DARY *image_in, Matrix *mi, int &dxe, int &dye,
	      float derive_sigma,
	      float sigma){
   int col_nb, row_nb;
   float A, B, C, determinant, trace;
   
   row_nb = image_in->y();
   col_nb = image_in->x();
   int midx,midy;
   midx=midy=row_nb>>1;
   //image_in->write("imin.pgm");
   //printf("- Starting to calculate gradients \n");
   DARY  *fxx= new DARY(row_nb,col_nb);	
   DARY  *fyy= new DARY(row_nb,col_nb);	
   DARY  *fxy= new DARY(row_nb,col_nb);	
   dXX(image_in, fxx, derive_sigma);
   dYY(image_in, fyy, derive_sigma);
   dXY(image_in, fxy, derive_sigma);
   int row=0, col=0, ok=0;

   float norm=derive_sigma*derive_sigma;
   for ( row = 0; row < row_nb; row++)
       for ( col = 0; col < col_nb; col++){
	   fxx->fel[row][col] = (norm*fxx->fel[row][col]);
	   fyy->fel[row][col] = (norm*fyy->fel[row][col]);
	   fxy->fel[row][col] = (norm*fxy->fel[row][col]);
       }
   float CC,AB;
   DARY  *brow= new DARY(row_nb,col_nb);	
   for ( row = 0; row < row_nb; row++)
     for (int col = 0; col < col_nb; col++)
       {
	 /*        A = B = C = determinant = trace = 0.0;*/
        A = fxx->fel[row][col];
        B = fyy->fel[row][col];
        C = fxy->fel[row][col];
	CC=C*C;
	AB=A*B;
        determinant = AB - (CC);
        //trace = A + B;
        brow->fel[row][col] =fabs(determinant);// - ALPHA * (trace*trace);
       }
   //brow->write("brow.pgm");
   
   delete fxx; delete fyy;  delete fxy;
   int border=5;
   float dist,act_pixel,act_p,dist_min=(brow->x()/2.0);
   int dxt=0,dyt=0;dxe=0;dye=0;
   dist_min*=dist_min;
   for (int row = border ; row+border  < row_nb; row++){
     for (int col = border; col+border  < col_nb; col++){
       act_pixel=brow->fel[row][col];
       if(act_pixel > brow->fel[row-1][col-1] &&
	  act_pixel > brow->fel[row-1][col] &&
	  act_pixel > brow->fel[row-1][col+1] &&
	  act_pixel > brow->fel[row][col-1] &&
	  act_pixel > brow->fel[row][col+1] &&
	  act_pixel > brow->fel[row+1][col-1] &&
	  act_pixel > brow->fel[row+1][col] &&
	  act_pixel > brow->fel[row+1][col+1] &&
	  act_pixel>200){
	 dxt=col-midx;
	 dyt=row-midy;
	 dist=dxt*dxt+dyt*dyt;
	 if(dist<dist_min){
	   dxe=dxt;dye=dyt;dist_min=dist;act_p=act_pixel;
	   ok=1;
	 }
   	 //cout <<"har = "<<act_pixel<< " old x = "<<midx<<" y = "<< midy << " dx = "<< dxt <<" dy = "<< dyt<< " derive_sigma "<<derive_sigma  <<  " dist "<<dist <<  " dist_min "<<dist_min <<endl;
       }
     }      
   }
   if(ok)getMi(image_in, mi, (int)(midx+dxe), (int)(midy+dye), sigma, derive_sigma);
 
   delete brow;
   return ok;
}


/****** HARRIS WITHOUT ANGLE IMAGE***********/
int harris_mi(DARY *image_in,Matrix *mi,int &dxe,int &dye,
	      float derive_sigma,
	      float sigma, 
	      float alpha, int fast){
    
    float t1,t2,A,B,C,determinant,trace;
    int row_nb= image_in->y();
    int col_nb= image_in->y();
    int midx,midy;
    midx=midy=row_nb>>1;
   //printf("- Starting to calculate gradients \n");
   DARY  *har= new DARY(row_nb,col_nb);	
   DARY  *fx= new DARY(row_nb,col_nb);	
   DARY  *fy= new DARY(row_nb,col_nb);	
   dX(image_in, fx, derive_sigma);
   dY(image_in, fy, derive_sigma);
   DARY *fxy  = new DARY(row_nb, col_nb);
   int row, col,ok=0;
   float norm=derive_sigma;//pow(derive_sigma,1.3);
   for ( row = 0; row < row_nb; row++)
       for ( col = 0; col < col_nb; col++){
	   t1 = norm*fx->fel[row][col];
	   t2 = norm*fy->fel[row][col];
	   fx->fel[row][col] = t1*t1;
	   fy->fel[row][col] = t2*t2;
	   fxy->fel[row][col] = t1*t2;
       }
   if(fast){
    mi->tabMat[1][1] = smoothf(midx,midy, fx, sigma);
    mi->tabMat[2][2] =smoothf(midx,midy, fy, sigma);
    mi->tabMat[2][1] =mi->tabMat[1][2] =smoothf(midx,midy,fxy, sigma);   
    delete fx; delete fy;delete fxy;delete har;
    return 1;
   }
   DARY *sfx2  = new DARY(row_nb, col_nb);
   DARY *sfy2  = new DARY(row_nb, col_nb);
   DARY *sfxy  = new DARY(row_nb, col_nb);
   
   
   smooth(fx,sfx2, sigma);delete fx;
   smooth(fy,sfy2, sigma);delete fy;
   smooth(fxy,sfxy, sigma);delete fxy;
     

   for ( row = 0; row < row_nb; row++)
     for (int col = 0; col < col_nb; col++)
       {
	 /*        A = B = C = determinant = trace = 0.0;*/
	 A = sfx2->fel[row][col];
	 B = sfy2->fel[row][col];
	 C = sfxy->fel[row][col];
        determinant = A * B - (C*C);
        trace = A + B;
        har->fel[row][col] = (determinant - alpha * (trace*trace));
       } 
   int border=3;
   float dist,act_pixel,act_p,dist_min=(har->x()/2.0);
   int dxt,dyt;dxe=0;dye=0;
   dist_min*=dist_min;
   for (int row = border ; row+border  < row_nb; row++){
     for (int col = border; col+border  < col_nb; col++){
       act_pixel=har->fel[row][col];
       if(act_pixel > har->fel[row-1][col-1] &&
	  act_pixel > har->fel[row-1][col] &&
	  act_pixel > har->fel[row-1][col+1] &&
	  act_pixel > har->fel[row][col-1] &&
	  act_pixel > har->fel[row][col+1] &&
	  act_pixel > har->fel[row+1][col-1] &&
	  act_pixel > har->fel[row+1][col] &&
	  act_pixel > har->fel[row+1][col+1] &&
	  act_pixel>200){
	 dxt=col-midx;
	 dyt=row-midy;
	 dist=dxt*dxt+dyt*dyt;
	 if(dist<dist_min){
	   dxe=dxt;dye=dyt;dist_min=dist;act_p=act_pixel;
	   ok=1;
	 }
	 // cout <<"har = "<<act_pixel<< " old x = "<<midx<<" y = "<< midy << " new x = "<< col <<" y = "<< row<< " derive_sigma "<<derive_sigma <<  " sigma "<<sigma <<endl;
       }
     }      
   }
   //if(dxe!=0 ||dye!=0)cout << "diff"<< endl;
   // har->write("har.pgm");
   //dxe=0;dye=0;
   // cout << "dxe "<< dxe<< " dye "<< dye<< endl;
   /*if(!ok){
     har->write("har.pgm");
     cout << "not OK sigma"<< sigma<< endl;getchar();
     }*/
   mi->tabMat[1][1] =sfx2->fel[midy+dye][midx+dxe];
   mi->tabMat[2][2] = sfy2->fel[midy+dye][midx+dxe];
   mi->tabMat[1][2] = mi->tabMat[2][1] =sfxy->fel[midy+dye][midx+dxe];

   delete sfx2; delete sfy2;  delete sfxy;delete har;
   return ok;
}




float searchDerScale(DARY *img, CornerDescriptor *cor){
  
  vector<float> scales;
  vector<float> laps;
  float x=cor->getX();
  float y=cor->getX(); 
  float sigma=cor->getCornerScale(); 
  int extr=cor->isExtr();
  int norm_size=41;
  DARY *patch = new DARY(norm_size,norm_size);
  
  float int_scale;
  float norm_sig=(norm_size-1)/(2*GAUSS_CUTOFF);
  float sc_start=0.5*sigma;
  float sc_end=2*sigma;
  //cout <<"sigma "<< sigma<< endl;
  for(float sc=sc_start;sc<sc_end;sc=sc*1.1){
    scales.push_back(sc);
    int_scale=(2.0*rint(GAUSS_CUTOFF*sc)+1.0)/(float)norm_size;
    patch->interpolate(img,(int)x,(int)y,int_scale,0,0,int_scale);
    //cout << "sc "<<sc << endl;patch->write("patch.pgm");getchar();
    laps.push_back(norm_sig*norm_sig*dXX_YYf(patch->x()>>1,patch->y()>>1,patch,norm_sig));
    //cout <<"sc " << sc  <<"  lap "<< laps[laps.size()-1]<< endl;
  }
  // getchar();
  delete patch;
  float is=0;
  float distis=100;
  int bestsc=-1;
  for(uint sc=2;sc<scales.size();sc++){    
    if((extr==-1 || extr==0) && laps[sc]>laps[sc-1] && 
       laps[sc-2]>laps[sc-1] && 
       fabs(1-laps[sc-1]/sigma)<distis){
      distis=fabs(1-laps[sc-1]/sigma);
      bestsc=sc-1;
      cor->setMin();
    }else if((extr==1 || extr==0) && laps[sc]<laps[sc-1] && 
	     laps[sc-2]<laps[sc-1] && 
	     fabs(1-laps[sc-1]/sigma)<distis){
      distis=fabs(1-laps[sc-1]/sigma);
      bestsc=sc-1;      
      cor->setMax();
    }
  }
  if(bestsc>0){
    cor->setCornerScale(scales[bestsc]);
    //cout << "scale "<< scales[bestsc] << " "<< laps[bestsc]<< endl;
    return scales[bestsc];
  }
  return 0;

}


int findScaleRegion(DARY *image, CornerDescriptor *cor){
  
  
  float sigma=cor->getCornerScale();

  int sizex;
  //DARY *img;
  /*
  float color=255;
  DARY *img_disp=new DARY(image);
  displayEllipse(img_disp, cor,color);
  img_disp->write("disp.pgm");
  cout << "imag disp" << endl;
  getchar();
  */
  float pointx=cor->getX();
  float pointy=cor->getY();

  int go_on=1; 
  int l,ok;
  float dx=0,dy=0;
  float dscale_bef=1,dscale_act=1, dloc_act=100,dloc_bef=100, scale_bef;
  int dxe=0,dye=0;

  float dloc_max=0.5,dscale_max=0.05;
  

  Matrix *mi=new Matrix(2,2,0.0);

  int isSmall=0;
  int norm_size=41,norm_sig;
  DARY *img=new DARY(norm_size,norm_size);
  norm_sig=(int)((norm_size-1)/(2*1.5*GAUSS_CUTOFF));
 
  float int_scale;
  for(l=0;l<50 && go_on;l++){
    dscale_bef=dscale_act;
    scale_bef=sigma;
    sigma=1.0*searchDerScale(image, cor);
    //    cout << "sigma "<< sigma<< endl;
    if(sigma<1){
      //cout << "no maxima "<< endl;
      return 0;
    }    
    dscale_act=fabs(1.0-scale_bef/sigma);

    sizex=(int)(2*rint(1.5*GAUSS_CUTOFF*sigma)+1); // patch size in image
    int_scale = (float)sizex/(float)norm_size; //scale in fixed size patch
    //img=new DARY(sizex,sizex);
    //cout << sizex<< " " << int_scale<< endl;
    img->interpolate(image,pointx,pointy,int_scale,0,0,int_scale);
    //if(int_scale>5){img->write("patch.pgm");cout << "wrote"<< endl;getchar();}
    if(cor->getType()==1){      
      //ok=harris_mi(img, mi, dxe, dye, norm_sig, 1.4*norm_sig, ALPHA,0);
    }else if(cor->getType()==2){
      //ok=brown(img, mi, dxe, dye, norm_sig, 1.4*norm_sig);    
    } else return 0;
    //cout << "dxe " << dxe<<  " dye " << dye<<endl;
    dloc_bef=dloc_act;
    pointx+=(dxe/int_scale);pointy+=(dye/int_scale);
    cor->setX_Y(pointx,pointy);
    dloc_act=sqrt((float)dxe*dxe+dye*dye);
    
    if(l>10 || ok==0) {
      //delete mi;
      //      delete img;
      // cout << "not converging "<< l<< " ok " << ok << " scale_det "<<cor->getCornerScale()<<  " scale_sel "<< derive_sigma << endl;
      return 0;
    }
    //  cout  <<  " dloc_act "<<dloc_act << " dloc_bef  " << dloc_bef << " scale_act "<< sigma<< " scale_bef "<< scale_bef<< " dscale_act "<<dscale_act<<  " dscale_bef "<< dscale_bef << endl;getchar();
    
    if(dloc_act<dloc_max && //dloc_bef<dloc_max && 
       dscale_act<dscale_max //&& dscale_bef<dscale_max
       ){
      go_on=0;	
    }    
    //  delete img;
  } 

  //delete mi;//delete &Ui;delete &D;delete &V;delete &X;   
  delete img;  
  cor->setX_Y(pointx,pointy);
  cor->setCornerScale(sigma);
  return l;
}


void findScaleRegion(DARY *image, vector<CornerDescriptor*> &cor){
  int nb=0;
  for(int i=0;i<(int)cor.size();i++){
    //int l=findScaleRegion(image,cor[i]);    
    int l=(int)searchDerScale(image, cor[i]);
    if(l!=0){
      cout <<"\r"<<nb++ << " of "<< cor.size() << " CORNER sc " << i <<" " << cor[i]->getX();
      cout  << " " << cor[i]->getY();
      cout << "  " << cor[i]->getCornerScale();
      cout<<"   l "<<l<<"     "<<flush;
    }else { cor.erase((std::vector<CornerDescriptor*>::iterator)&cor[i]);i--;}
  }
  
}


const static float gsm[3]={0.1664, 0.6672, 0.1664};

void findScales(DARY *img, vector<CornerDescriptor*> &cor,vector<CornerDescriptor*> &corout){
  
  vector<float> scales;
  vector<float> laps;
  DARY *patch;
  int norm_size;
  float sigma,sc_start,sc_end,x,y,lap;
  float norm_sig=(norm_size-1)/(2*GAUSS_CUTOFF);

  CornerDescriptor *corn;
  float threshold = 3;
  int flag;
  for(uint c=0;c<cor.size();c++){
    cout << "\r "<< c << " of "<<  cor.size()<< "             "<< flush;
    x=cor[c]->getX();
    y=cor[c]->getY(); 
    sigma=cor[c]->getCornerScale(); 
    sc_start=0.8*sigma;
    sc_end=3*sigma;
    
    norm_size=(int)(2*rint(3*sigma*GAUSS_CUTOFF)+1);
    patch = new DARY(norm_size,norm_size);
    patch->interpolate(img,(int)x,(int)y,1,0,0,1);
    flag=1;
    for(float sc=sc_start;sc<sc_end && flag;sc=sc*1.1){
      lap=sc*sc*dXX_YYf(patch->x()>>1,patch->y()>>1,patch,sc);
      if(lap!=0){
	scales.push_back(sc); 	
	laps.push_back(lap);
      }else flag=0;
    } 

    //cout << endl<<scales[0]<< "  "  << laps[0]<< endl;
    for(uint s=1;s<laps.size()-1;s++){
      laps[s]=gsm[0]*laps[s-1]+gsm[1]*laps[s]+gsm[2]*laps[s+1];
      //cout << scales[s]<< "  "  << laps[s]<< endl;
    }
    //cout << scales[laps.size()-1]<< "  "  << laps[laps.size()-1]<< endl;
    
      //cout << " X "<<x << " Y "<<y << " S "<<sigma <<endl;
    for(uint s=2;s<scales.size()-1;s++){  
      //cout << "check "<< laps[s-1]<< " "<< laps[s] << " "<<laps[s+1]<<endl;
      if(laps[s-2]>laps[s-1] && laps[s-1]>laps[s] && 
	 laps[s+1]>laps[s] && fabs(laps[s])>threshold){
	corn = new CornerDescriptor();
	corn->setX_Y(x,y);
	corn->setCornerScale(scales[s]);
	//cout << "min "<<scales[s]<< endl;
	corn->setMin();
	corout.push_back(corn);
      } else if(laps[s-2]<laps[s-1] && laps[s-1]<laps[s] && 
		laps[s+1]<laps[s] && fabs(laps[s])>threshold ){
	corn = new CornerDescriptor();
	corn->setX_Y(x,y);
	corn->setCornerScale(scales[s]);
	corn->setMax();
	//cout << "max "<<scales[s]<< endl;
	corout.push_back(corn);
      }
    }
    //patch->write("patch.pgm");
    //if(corout.size())displayCircle(patch,new CornerDescriptor((float)(patch->x()>>1),(float)(patch->y()>>1),1.0,corn->getCornerScale()),255);
    //patch->write("corner.pgm");
    delete patch;
    scales.clear();
    laps.clear();
    //getchar();

  }
}





void getXY(float **patch,int xm, int ym,int &x1, int &y1){
  if((int)patch[ym-1][xm+1]){
    x1=1;y1=-1;patch[ym-1][xm+1]=0;
  }else if((int)patch[ym][xm+1]){
    x1=1;y1=0;patch[ym][xm+1]=0;
  }else if((int)patch[ym+1][xm+1]){
    x1=1;y1=1;patch[ym+1][xm+1]=0;
  }else if((int)patch[ym+1][xm]){
    x1=0;y1=1;patch[ym+1][xm]=0; 
  }else if((int)patch[ym+1][xm-1]){
    x1=-1;y1=1; patch[ym+1][xm-1]=0;   
  }else if((int)patch[ym][xm-1]){
    x1=-1;y1=0;patch[ym][xm-1]=0;    
  }else if((int)patch[ym-1][xm-1]){
    x1=-1;y1=-1;patch[ym-1][xm-1]=0;    
  }else if((int)patch[ym-1][xm]){
    x1=0;y1=-1;patch[ym-1][xm]=0;    
  }else {x1=0;y1=0;patch[ym][xm]=0;}  
}


float checkTangent(DARY *edge, int x, int y, int xr, int yr, float ta){

  if(xr-4<0 || xr+4>edge->x() || yr-4<0 || yr+4>edge->y())return 100000;
  //cout << "ts " << endl;
  int x1,y1,x2,y2,x3,y3,x4,y4;
  float dxr=x-xr;
  float dyr=y-yr;
  float dar=atan2(dyr,dxr);


  //cout << " x " << x << " y " << y << " xr " << xr << " yr " << yr << endl;
  float **patch;
  patch= new float*[5];
  patch[0]= new float[25];
  for(int i=1;i<5;i++)patch[i]=patch[0]+i*5;
  for(int j=-2;j<3;j++){
    //cout << endl;
    for(int i=-2;i<3;i++){
      patch[2+j][2+i]=edge->fel[yr+j][xr+i];
      //cout << patch[2+j][2+i]<< "  ";
    }
  }
  //cout << endl;

  patch[2][2]=0;
  getXY(patch,2,2,x1,y1);
  getXY(patch,2,2,x2,y2);
  getXY(patch,2+x1,2+y1,x3,y3);
  getXY(patch,2+x2,2+y2,x4,y4);
  
  x3=x1+x3;y3=y1+y3;
  x4=x2+x4;y4=y2+y4;

  // cout << x3 << " " << y3<< endl;cout << x4 << " " << y4<< endl;cout << y4-y3 << " diff " << x4-x3<< endl;

  
  
  float dae=atan2((float)(y4-y3),(float)(x4-x3));
  
  
  float diff =fabs(dar-dae);
  if(diff>M_PI)diff-=M_PI;
  diff=fabs(M_PI/2.0-diff);

  float diff2=fabs(ta-dae);
  if(diff2>M_PI)diff2-=M_PI;
  if(diff2>M_PI/2.0)diff2=M_PI-diff2;
  //cout << "diff " <<diff << " diff2 " <<diff2 << endl;  

  //  cout<<  " dar " << dar*360.0/M_2PI << "  dae " <<  dae*360.0/M_2PI<< " diff " << diff*360.0/M_2PI <<  endl;
  /*
  int si=4*(int)rint(sqrt(dxr*dxr+dyr*dyr))+1;
  DARY *img=new DARY(si,si);

  img->interpolate(edge,x,y,1,0,0,1);
  img->fel[si>>1][si>>1]=255;
  img->write("patch.pgm");getchar();
  */
  delete []patch;
  // cout << "te " << endl;

 return diff2;
}


void drawLine(DARY *patch, int x, int y, int xr, int yr, float color){
  
  int dx=(xr-x);
  int _x;
  if(dx>0)_x=1;
  else _x=-1;
  int dy=yr-y;

  float dy_dx=(float)dy/(float)dx;
  
  //cout << x << " "<< y << " " <<xr << " " << yr << endl;
  int xp;int yp;
  for(int xs=0;xs!=dx;xs+=_x){
    xp=x+xs;
    yp=(int)(y+dy_dx*xs);
    patch->fel[yp][xp]=color;
  }
  
}

int checkAngle(int xe, int ye, int px, int py, float angle, float da){
  
  float an = atan2((float)(py-ye),(float)(px-xe));
  
  float dan =  fabs(angle - an);
  if(dan > M_PI)dan=M_2PI-dan;

  if(dan>da)
    return 1;
  else return 0;
}

float checkTangentEllipse(DARY *edge, Matrix M,int xe, int ye, float angle,float ta, float &pxf, float &pyf){
  

  
  Matrix U,V,D,rot;
  M.svd(U,D,V);
  rot=V;
  //cout <<"D "<<  D<< endl;
  //cout <<"V "<< V<< endl;

  // cout << " l1 "<< ((m11+m22)+sqrt((m11+m22)*(m11+m22)-4*(m11*m22-m12*m21)))/2.0<< endl;
  // cout << " l1 "<< ((m11+m22)-sqrt((m11+m22)*(m11+m22)-4*(m11*m22-m12*m21)))/2.0<< endl;


  float a,b;
  a=1.0/sqrt(D(1,1));b=1.0/sqrt(D(2,2));


  // cout << a << " " << b << endl;
  float b2=b*b,a2=a*a;
  float b2_a2=b2/a2;
  float a2_b2=a2/b2;
  float xt, yt;
  int px,py,found=1;
  float pxc,pyc, del,cb,ea,diff, min_diff=1000;
  for(float i=0;i<a;i++){               
    yt=sqrt((float)(b2-b2_a2*i*i));
    pxc=(int)(rint(rot(1,1)*i + rot(1,2)*yt));
    pyc=(int)(rint(rot(2,1)*i + rot(2,2)*yt));
    px=(int)(xe+pxc); py=(int)(ye+pyc);
    if(px>=0 && px<edge->x() && py>=0 && py<edge->y() && checkAngle(xe,ye,px,py,angle,ta)){
      if(edge->fel[py][px]>0){
	cb=M(1,2)*pxc+M(2,2)*pyc;
	if(cb!=0)del=-(M(1,1)*pxc+M(1,2)*pyc)/cb;
	else del=1000000;
	ea=atan2(del,1);
        diff=checkTangent(edge,  xe, ye, px, py, ea);
	if(diff<min_diff){
	  min_diff=diff;pxf=px;pyf=py;
	}
      }
	//edge->fel[py][px]=color;
    }
    pxc=(int)rint(rot(1,1)*i-rot(1,2)*yt);
    pyc=(int)rint(rot(2,1)*i-rot(2,2)*yt);
    px=xe+pxc;py=ye+pyc;
    if(px>=0 && px<edge->x() && py>=0 && py<edge->y() && checkAngle(xe,ye,px,py,angle,ta)){
      if(edge->fel[py][px]>0){
	cb=M(1,2)*pxc+M(2,2)*pyc;
	if(cb!=0)del=-(M(1,1)*pxc+M(1,2)*pyc)/cb;
	else del=1000000;
	ea=atan2(del,1);
        diff=checkTangent(edge,  xe, ye, px, py, ea);
	if(diff<min_diff){
	  min_diff=diff;pxf=px;pyf=py;
	}
      }
      //edge->fel[py][px]=color;
    }
    pxc=xe+(int)rint(-rot(1,1)*i+rot(1,2)*yt);
    pyc=ye+(int)rint(-rot(2,1)*i+rot(2,2)*yt);
    px=xe+pxc;py=ye+pyc;
    if(px>=0 && px<edge->x() && py>=0 && py<edge->y() && checkAngle(xe,ye,px,py,angle,ta)){
      if(edge->fel[py][px]>0){
	cb=M(1,2)*pxc+M(2,2)*pyc;
	if(cb!=0)del=-(M(1,1)*pxc+M(1,2)*pyc)/cb;
	else del=1000000;
	ea=atan2(del,1);
        diff=checkTangent(edge,  xe, ye, px, py, ea);
	if(diff<min_diff){
	  min_diff=diff;pxf=px;pyf=py;
	}
      }
      //edge->fel[py][px]=color;
    }
    pxc=xe+(int)rint(-rot(1,1)*i-rot(1,2)*yt);
    pyc=ye+(int)rint(-rot(2,1)*i-rot(2,2)*yt);
    px=xe+pxc;py=ye+pyc;
    if(px>=0 && px<edge->x() && py>=0 && py<edge->y() && checkAngle(xe,ye,px,py,angle,ta)){
      if(edge->fel[py][px]>0){
	cb=M(1,2)*pxc+M(2,2)*pyc;
	if(cb!=0)del=-(M(1,1)*pxc+M(1,2)*pyc)/cb;
	else del=1000000;
	ea=atan2(del,1);
        diff=checkTangent(edge,  xe, ye, px, py, ea);
	if(diff<min_diff){
	  min_diff=diff;pxf=px;pyf=py;
	}
      }
      //edge->fel[py][px]=color;
    }
  }
  for(float j=1;j<b;j++){               
    xt=(int)rint(sqrt((float)(a2-a2_b2*j*j)));
    pxc=xe+(int)rint(rot(1,1)*xt+rot(1,2)*j);
    pyc=ye+(int)rint(rot(2,1)*xt+rot(2,2)*j);
    px=xe+pxc;py=ye+pyc;
    if(px>=0 && px<edge->x() && py>=0 && py<edge->y() && checkAngle(xe,ye,px,py,angle,ta)){
      if(edge->fel[py][px]>0){
	cb=M(1,2)*pxc+M(2,2)*pyc;
	if(cb!=0)del=-(M(1,1)*pxc+M(1,2)*pyc)/cb;
	else del=1000000;
	ea=atan2(del,1);
        diff=checkTangent(edge,  xe, ye, px, py, ea);
	if(diff<min_diff){
	  min_diff=diff;pxf=px;pyf=py;
	}
      }
      //edge->fel[py][px]=color;
    }
    pxc=xe+(int)rint(rot(1,1)*xt-rot(1,2)*j);
    pyc=ye+(int)rint(rot(2,1)*xt-rot(2,2)*j);
    px=xe+pxc;py=ye+pyc;
    if(px>=0 && px<edge->x() && py>=0 && py<edge->y() && checkAngle(xe,ye,px,py,angle,ta)){
      if(edge->fel[py][px]>0){
	cb=M(1,2)*pxc+M(2,2)*pyc;
	if(cb!=0)del=-(M(1,1)*pxc+M(1,2)*pyc)/cb;
	else del=1000000;
	ea=atan2(del,1);
        diff=checkTangent(edge,  xe, ye, px, py, ea);
	if(diff<min_diff){
	  min_diff=diff;pxf=px;pyf=py;
	}
      }
      //edge->fel[py][px]=color;
    }
    pxc=xe+(int)rint(-rot(1,1)*xt+rot(1,2)*j);
    pyc=ye+(int)rint(-rot(2,1)*xt+rot(2,2)*j);
    px=xe+pxc;py=ye+pyc;
    if(px>=0 && px<edge->x() && py>=0 && py<edge->y() && checkAngle(xe,ye,px,py,angle,ta)){
      if(edge->fel[py][px]>0){
	cb=M(1,2)*pxc+M(2,2)*pyc;
	if(cb!=0)del=-(M(1,1)*pxc+M(1,2)*pyc)/cb;
	else del=1000000;
	ea=atan2(del,1);
        diff=checkTangent(edge,  xe, ye, px, py, ea);
	if(diff<min_diff){
	  min_diff=diff;pxf=px;pyf=py;
	}
      }
      //edge->fel[py][px]=color;
    }
    pxc=xe+(int)rint(-rot(1,1)*xt-rot(1,2)*j);
    pyc=ye+(int)rint(-rot(2,1)*xt-rot(2,2)*j);
    px=xe+pxc;py=ye+pyc;
    if(px>=0 && px<edge->x() && py>=0 && py<edge->y() && checkAngle(xe,ye,px,py,angle,ta)){
      if(edge->fel[py][px]>0){
	cb=M(1,2)*pxc+M(2,2)*pyc;
	if(cb!=0)del=-(M(1,1)*pxc+M(1,2)*pyc)/cb;
	else del=1000000;
	ea=atan2(del,1);
        diff=checkTangent(edge,  xe, ye, px, py, ea);
	if(diff<min_diff){
	  min_diff=diff;pxf=px;pyf=py;
	}
      }
      //edge->fel[py][px]=color;
    }
  }
          

  return min_diff;
}


int getEllipseABC(float x0, float y0, float dy_dx, float rad, Matrix &M, float &scale){

  float l=1.0/((float)rad*rad);
  float e,f,g,h,m,n,z,p,r,a1=0,b1=0,c1=0,a2,b2,c2;
  if(x0!=0 && y0!=0){
    e=1.0/(x0*x0);f=2.0*y0/x0;g=(y0*y0)/(x0*x0);//a=e-fc-gb
    h=(y0*dy_dx-x0*g); m=(dy_dx*x0+y0-x0*f)/h;
    n=e*x0/h;
    p=(f*m-g*m*m-1); r=(f*n-e*m+l*m-l*g*m+l*f-2*g*m*n);
    z=(l*l-l*e-l*g*n+l*n-e*n-g*n*n);
    if(fabs(p)<0.00001 && r!=0){
      c1=-z/r;
    }else {
      c1=(sqrt(r*r-4*p*z)-r)/(2*p);
      c2=(-sqrt(r*r-4*p*z)-r)/(2*p);
      //cout <<(r*r-4*p*z) << endl;
      //cout <<sqrt(r*r-4*p*z)*sqrt(r*r-4*p*z) << endl;
    }
    b1=-c1*m-n;
    b2=-c2*m-n;
    a1=e-f*c1-g*b1;
    a2=e-f*c2-g*b2;
  } else if(x0==0){
    b1=1/(y0*y0);
    c1=-dy_dx*b1;
    a1=(c1*c1-l*l+b1*l)/(b1-l);
  } else if(y0==0){
    a1=1/(x0*x0);
    c1=-a1/dy_dx;
    b1=(c1*c1-l*l+a1*l)/(a1-l);
  }else return 0;
  
  if(fabs(a1)<1){
    M(1,1)=a1;M(1,2)=c1;M(2,1)=c1;M(2,2)=b1;
  }else {
    M(1,1)=a2;M(1,2)=c2;M(2,1)=c2;M(2,2)=b2;
  }
  //cout << D<< endl;
  float l2=fabs(M(1,1)*M(2,2)-M(1,2)*M(2,1))/l;

  /*float csa=0,sna=0;
  float u=(M(1,2)*M(2,1))/(fabs(l2-l)*fabs(l2-l));
  if(u<0.25){
     csa=sqrt((1-sqrt(1-4*u))/2);
     sna = sqrt(1-csa*csa);
    cout << " sina " << sna << " cosa " << csa<< endl;
  }else cout << "error U " << u << endl;
  */

  


  //l2=1/(sqrt(l2));
  //l=1/(sqrt(l));
  


  float l2_l1=sqrt(l2/l);
  //cout << " l2 "<< l2 << " l1 " << l << " l/l "<<l2_l1<< endl; 
  if(l2_l1>1)l2_l1=1/l2_l1;
  if((l2_l1)<0.3)return 0;
  
  //  cout << " x0 "<< x0 << " y0 " << y0 << " dy_dx "<< dy_dx  << endl;
  //  cout << "eq1 "<< M(1,1)*x0*x0+2*M(1,2)*x0*y0+M(2,2)*y0*y0<<endl;
  //  cout << "eq2 " << M(1,1)*x0+y0*M(1,2)+x0*dy_dx*M(1,2)+y0*dy_dx*M(2,2)<< endl;
  //  cout<< "eq4 " << l*l-M(1,1)*l-M(2,2)*l+M(1,1)*M(2,2)-M(1,2)*M(1,2)<< endl;

  /* float sc=sqrt(l2*l);
  l2=l2/sc;
  l=l/sc; 
  cout << " l2 "<< l2 << " l1 " << l << "   " <<sc <<  endl; 
  if(l<l2){
    float tmp=l;
    l=l2;
    l2=tmp;
  }

  float m22=csa*csa*l+sna*sna*l2;
  float m11=sna*sna*l+csa*csa*l2;
  float m12=-csa*sna*l+csa*sna*l2;
  cout << "m11 "<< m11 << " m22 " << m22 << " m12 " << m12 << endl;
  */
  Matrix U,V,D;
  M.svd(U,D,V);
  D(1,1)=1/sqrt(D(1,1));
  D(2,2)=1/sqrt(D(2,2));
  scale=sqrt(D(1,1)*D(2,2));
  D(1,1)=D(1,1)/scale;
  D(2,2)=D(2,2)/scale;
  M=V*D*V.transpose();
  //cout << " sina " << V(1,2) << " cosa " << V(1,1)<< endl;
  // cout << "scale " << scale << " D " << D << endl;
  //cout << M<< endl;getchar();
  return 1;
}

void drawPatch(Segment* se,Segment* sep, Matrix M, int x, int y, float scale, int tx, int ty){

  int  sizex=(int)(10*sqrt((float)(tx-x)*(tx-x)+(ty-y)*(ty-y))+1);//patch
  int mid=sizex>>1;//patch
  DARY *patch = new DARY(sizex,sizex,0.0);//patch
  
  drawLine(patch,mid,mid,mid+tx-x,mid+ty-y,100);
  
  cout << " x0 " << mid+tx-x<<  " y0 " << mid+ty-y  << endl;

  //  Matrix U,V,D;
  //  M.svd(U,D,V);
  //  cout << D << endl;

  displayEllipse(patch, mid, mid, scale, M(1,1), M(1,2), M(2,1), M(2,2), 100);
  //displayEllipse(newedge, x, y, M(1,1), M(1,2), M(2,1), M(2,2), 100);
  // displayEllipse(patch, mid, mid,a1,c1,c1,b1, 100);
  //  displayEllipse(patch, mid, mid,a2,c2,c2,b2, 100);
  //DRAWING
  int px,py;
  for(uint p=0;p<se->chain.size();p++){
    px=se->chain[p]->x-x+mid;py=se->chain[p]->y-y+mid;
    if(px>0&& px<sizex && py>0 && py<sizex)
      patch->fel[py][px]=255;
  }
  for(uint p=0;p<sep->chain.size();p++){
    px=sep->chain[p]->x-x+mid;py=sep->chain[p]->y-y+mid;
    if(px>0&& px<sizex && py>0 && py<sizex)
      patch->fel[py][px]=255;
  }
  
  patch->fel[mid][mid]=180;
  patch->fel[mid+ty-y][mid+tx-x]=150;
  patch->write("patch.pgm");
  cout << "patch wrote"<< endl;getchar();
  delete patch; 
}


int evaluateEllipse(DARY *img, int x, int y, Matrix M, float scale, Matrix &bestM, float &bestScale, float &cost){
  int sizex=(int)(2*rint(scale*GAUSS_CUTOFF)+1);
  int mid=sizex>>1;
  DARY *patch = new DARY(sizex,sizex);
  patch->interpolate(img,x,y,M(1,1),M(1,2),M(2,1),M(2,2));
  //patch->write("norm.pgm");
  Matrix *mi = new Matrix(2,2);
  mi->tabMat[1][1]=10;
  mi->tabMat[2][2]=10;
  //getMi(patch, mi,  mid, mid , scale, scale/1.4);
  delete patch;
  float a=mi->tabMat[1][1];
  float b=mi->tabMat[2][2];
  float c=mi->tabMat[1][2];
  float sq=sqrt((a+b)*(a+b)-4*(a*b-c*c));
  float l1=((a+b)+sq)/2;
  float l2=((a+b)-sq)/2;
  delete mi;
  float cost_tmp=(l2<l1)?(l2/l1):(l1/l2);
  cost_tmp = 1-cost_tmp;
  //cout << l1 << "  " << l2 << " cost_tmp " << cost_tmp << " cost "<< cost <<  endl;
  if(cost > cost_tmp){
    cost = cost_tmp;
    bestM(1,1)=M(1,1);
    bestM(1,2)=M(1,2);
    bestM(2,1)=M(2,1);
    bestM(2,2)=M(2,2);
    bestScale=scale;
  }
  return 1;
}

int findAffineTangent(DARY *edge, DARY *img, int x, int y, int xr,int yr,float &bestScale, vector<Segment*> seg, Matrix &bestM){
  
  Segment *se;
  Segment *sep;
  int flag=1;
  int ptouch=0;
  //FIND point and tangent segments
  for(uint s=0;s<seg.size() && flag;s++){
    for(uint p=0;p<seg[s]->chain.size() && flag;p++){
      if(seg[s]->chain[p]->x==xr && seg[s]->chain[p]->y==yr){
	flag=0;
	se=seg[s];
	ptouch=p;
      }            
    }    
  }
  flag=1;
  for(uint s=0;s<seg.size() && flag;s++){
    for(uint p=0;p<seg[s]->chain.size() && flag;p++){
      if(seg[s]->chain[p]->x==x && seg[s]->chain[p]->y==y){
	flag=0;
	sep=seg[s];
      }      
      
    }    
  }

  if(!se)return 0;

  

  float dy_dx=1000,dy=0,dx=0,angle,_an;int x1,y1,x2,y2;
  int px,py;
  float rad=sqrt((float)((x-xr)*(x-xr)+(y-yr)*(y-yr)));
  float scale;
  Matrix M(2,2,0.0);
  float da,amax,amin, min_diff, cost=10;
  int found =1,st;
  DARY *patch;
  int tx=xr,ty=yr;float diff;
  float max_diff=M_PI/6.0;
  uint p_start=ptouch-10;
  if(p_start<2)p_start=2;
  uint p_stop=ptouch+10;
  if(p_stop>se->chain.size()-2)p_stop=se->chain.size()-2;
  for(uint ps=p_start;ps<p_stop && found;ps++){  
    tx=se->chain[ps]->x; ty=se->chain[ps]->y;//tangent point
    dy=se->chain[ps+2]->y - se->chain[ps-2]->y;
    dx=se->chain[ps+2]->x - se->chain[ps-2]->x;
    if(dx!=0)dy_dx=dy/dx;
    else dy_dx=1000000.0;
    angle = -atan2(dy,dx); //segment tangent
    _an=atan2((float)(ty-y),(float)(tx-x));//radius angle
    diff=fabs(-angle-_an);
    if(diff>M_PI)diff=fabs(M_2PI-diff);
    diff=fabs(M_PI/2.0-diff);//deviation from perfect tangent
    
    // cout << " dy_dx " << dy_dx << " dx " << dx<< " dy " << dy<< " angle " << angle*180.0/M_PI;
    // cout <<  " an "<< _an*180.0/M_PI<< " diff " << diff*180.0/M_PI<<endl;
    
    for(float r=rad/3.0;r<rad*3 && diff < max_diff  && found;r*=1.1){ 
      //cout << "r " << r << endl;
      if(!getEllipseABC(tx-x,ty-y,dy_dx,r,M, scale))continue;
      else evaluateEllipse(img,x,y,M,scale, bestM, bestScale,cost);
      //cout << "FOUND "<< endl;found=0;	           
    }
  }
  //cout << "cost " << cost << endl;
  //drawPatch(se, sep, bestM, x, y, bestScale, tx, ty);
  if(cost < 1)return 1;
  else return 0;
}



void hitEdge(DARY *img, DARY *edge, vector<CornerDescriptor*> &cor, vector<Segment*> seg, vector<CornerDescriptor*> &corout, int flag){
  float r_min,r_max,sigma,dy=0, xc,yc,ddy, ta;
  int xr,yr,x,y,found=1;
  float r_scale=0,d,scale;
  int xsize=edge->x()-2;
  int ysize=edge->y()-2;
  Matrix M(2,2,0.0);
  M(1,1)=1;M(2,2)=1;
  DARY * newedge = new DARY(edge);
  CornerDescriptor* cnew;
  for(uint c=0;c<cor.size();c++){
    x=(int)cor[c]->getX();
    y=(int)cor[c]->getY(); 
    cout << "\r "<< c << " of "<<  cor.size()<< "  " << x<<"  " <<y<<"     "<< flush;
    sigma=cor[c]->getCornerScale();
    r_min=rint(sigma);
    r_max=rint(sigma*3.0);
    //if(sigma<)
    found=1;
    for(float r=r_min;r<r_max;r++){
      for(float a =0;a<M_2PI&&found;a+=0.05){
	xc=(int)rint(r*cos(a));yc=(int)rint(r*sin(a));
	if(yc!=0)ddy=-xc/yc;
	else ddy=10000000;
	ta=atan2(ddy,1);
	xr=x+xc;yr=y+yc;
	if(xr<xsize && xr>2 && yr<ysize && yr>2){	  
	  if((int)edge->fel[yr][xr]){
	    if((d=checkTangent(edge,x,y,xr,yr,ta))<0.1){
	      //cout << "diff " << d*180.0/M_PI<< endl;
	      if(flag || findAffineTangent(edge,img, x, y, xr,yr,scale,seg,M)){
		found=0;if(flag)scale=r;
		cnew = new CornerDescriptor(x,y,100,scale);
		cnew->setMi(M(1,1),M(1,2),M(2,1),M(2,2));
		corout.push_back(cnew);
		r*=1.4;
	      }
	    }
	  }
	}
      }
    }
    if(found){cor.erase((std::vector<CornerDescriptor*>::iterator)&cor[c]);c--;}    
  }     
  cor.clear();
  for(int i=0;i<corout.size();i+=20){
    //cor.push_back(corout[i]);
  }
  drawAffineCorners(newedge, corout, "newedge.pgm",150);

  //newedge->write("newedge.pgm");
  cout << "newedge wrote"<< endl;getchar();
}



/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/



