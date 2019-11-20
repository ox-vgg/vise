#include "cornerDetector.h"
/**
DARY *image_in - input image

DARY *image_out - output image -all pixels must be == 0
**/


int check(int x, int y, int width, int height, int border){
  if(x>border && y>border && x<width-border && y<height-border)return 1;
  else return 0;

}

void drawCorners(DARY *im_in, vector<CornerDescriptor*> &cor, const char *filename, float color){
  int x,y;
  DARY *im=new DARY(im_in->y(),im_in->x(),0.0);
  for(uint j=0;j<im_in->y();j++){
    for(uint i=0;i<im_in->x();i++){
      if(im_in->fel[j][i]>0)im->fel[j][i]=im_in->fel[j][i];//+30;
      //if(im->fel[j][i]>255)im->fel[j][i]=255;
    }
  }
  int size=2;
  /*  int thick=1;
  for(unsigned int c=0;c<cor.size();c++){
      x=(int)cor[c]->getX();y=(int)cor[c]->getY();
      if(x>size && y>size && x<im->x()-size && y<im->y()-size){
          for(int j=-thick;j<=thick;j++){
              for(int i=-size;i<=size;i++){
                  im->fel[y+j][x+i]=im->fel[y+i][x+j]=color;
              }
          }
      }
  } */ 
  int yi,border;
  //float angle, lesin, lecos, tang;
  for(unsigned int c=0;c<cor.size();c++){
    size=(int)(1*rint(cor[c]->getCornerScale()));
    x=(int)cor[c]->getX();y=(int)cor[c]->getY();
    border=4;
    if(check(x,y,im->x(),im->y(),border)){
      for(int i=-2;i<=2;i++)
	im->fel[y][x+i]=im->fel[y+i][x]=color;
      //continue;
      for(int i=-size;i<=size;i++){
	for(int j=0;j<=size;j++){
	  yi=(int)rint(sqrt((float)(size*size-i*i)));
	  if(check(x+i,y+yi,im->x(),im->y(),border))   
	    im->fel[y+yi][x+i]=color;
	  if(check(x+i,y-yi,im->x(),im->y(),border))   
	    im->fel[y-yi][x+i]=color;
	  if(check(x+yi,y+i,im->x(),im->y(),border))   
	    im->fel[y+i][x+yi]=color;
	  if(check(x-yi,y+i,im->x(),im->y(),border))   
	    im->fel[y+i][x-yi]=color;
	}  
      }
      //continue;
      //angle=(2*M_PI*cor[c]->getAngle())/360.0;
 /* angle=cor[c]->getAngle();
      // cout << " a " <<angle ;
      lecos=cos(angle);
      lesin=sin(angle);
      //cout << "cos "<< lecos<< " sin " << lesin << endl;
      if(lecos!=0)
	tang=lesin/lecos;
      else tang=1000;
      // cout << " angle " <<lecos;
      // cout << " " << lesin<< endl;
      int ti;
      if(tang>-0.5 && tang < 0.5){
	int csize=(int)rint(lecos*size);
	for(int i=0;i<=csize;i++){
	  ti=(int)rint(tang*i);
	  if(check(x+i,y+ti,im->x(),im->y(),border))
	    im->fel[y+ti][x+i]=color; 	  
	}
	for(int i=csize;i<=0;i++){
	  ti=(int)rint(tang*i);
	  if(check(x+i,y+ti,im->x(),im->y(),border))
	    im->fel[y+ti][x+i]=color;
	}
      }
      else{
	int ssize=(int)rint(lesin*size);
	for(int i=0;i<=ssize;i++){
	  ti=(int)rint(i/tang);
	  if(check(x+ti,y+i,im->x(),im->y(),border))
	    im->fel[y+i][x+ti]=color;
	}
	for(int i=ssize;i<=0;i++){
	  ti=(int)rint(i/tang);
	  if(check(x+ti,y+i,im->x(),im->y(),border))
	    im->fel[y+i][x+ti]=color;
	}    
      }
*/  
    }
  }
  im->write(filename);
  delete im;
}
void drawColorCorners(DARY *im, vector<CornerDescriptor*> &cor, const char *filename, float color){
  int x,y;
  int size=2;

  int yi,border;
  float angle, lesin, lecos, tang;
  for(unsigned int c=0;c<cor.size();c++){
    size=(int)(3*rint(cor[c]->getCornerScale()));
    x=(int)cor[c]->getX();y=(int)cor[c]->getY();
    border=2;
    if(check(x,y,im->x(),im->y(),border)){
      for(int i=-2;i<=2;i++)
	im->felr[y][x+i]=im->felr[y+i][x]=color;
      
      
      for(int i=-size;i<=size;i++){
	for(int j=0;j<=size;j++){
	  yi=(int)rint(sqrt((float)(size*size-i*i)));
	  if(check(x+i,y+yi,im->x(),im->y(),border))
	    im->felr[y+yi][x+i]=im->felg[y+yi][x+i]=im->felb[y+yi][x+i]=color;
	  if(check(x+i,y-yi,im->x(),im->y(),border))
	    im->felr[y-yi][x+i]=im->felg[y-yi][x+i]=im->felb[y-yi][x+i]=color;
	  if(check(x+yi,y+i,im->x(),im->y(),border))
	    im->felr[y+i][x+yi]=im->felg[y+i][x+yi]=im->felb[y+i][x+yi]=color;
	  if(check(x-yi,y+i,im->x(),im->y(),border))
	    im->felr[y+i][x-yi]=im->felg[y+i][x-yi]=im->felb[y+i][x-yi]=color;
	}  
      }
      
      //angle=(2*M_PI*cor[c]->getAngle())/360.0;
      angle=cor[c]->getAngle();
      // cout << " a " <<angle ;
      lecos=cos(angle);
      lesin=sin(angle);
      //cout << "cos "<< lecos<< " sin " << lesin << endl;
      if(lecos!=0)
	tang=lesin/lecos;
      else tang=1000;
      // cout << " angle " <<lecos;
      // cout << " " << lesin<< endl;
      int ti;
      if(tang>-0.5 && tang < 0.5){
	int csize=(int)rint(lecos*size);
	for(int i=0;i<=csize;i++){
	  ti=(int)rint(tang*i);
	  if(check(x+i,y+ti,im->x(),im->y(),2))
	    im->felr[y+ti][x+i]=im->felg[y+ti][x+i]=im->felb[y+ti][x+i]=color; 	  
	}
	for(int i=csize;i<=0;i++){
	  ti=(int)rint(tang*i);
	  if(check(x+i,y+ti,im->x(),im->y(),2))
	    im->felr[y+ti][x+i]=im->felg[y+ti][x+i]=im->felb[y+ti][x+i]=color;
	}
      }
      else{
	int ssize=(int)rint(lesin*size);
	for(int i=0;i<=ssize;i++){
	  ti=(int)rint(i/tang);
	  if(check(x+ti,y+i,im->x(),im->y(),2))
	  im->felr[y+i][x+ti]=im->felg[y+i][x+ti]=im->felb[y+i][x+ti]=color;
	}
	for(int i=ssize;i<=0;i++){
	  ti=(int)rint(i/tang);
	  if(check(x+ti,y+i,im->x(),im->y(),2))
	    im->felr[y+i][x+ti]=im->felg[y+i][x+ti]=im->felb[y+i][x+ti]=color;
	}    
      }  
    }
  }  
  im->write(filename);
  delete im;
}

int searchSimilarPoint2(CornerDescriptor *c1, vector<CornerDescriptor*> cor, float sscale, float max_dist){
  float dist;
  vector<int> cor_sim;
  float dl;
  float dl1=(c1->getL2()/c1->getL1());
  // cout << "search scale " << sscale<< endl;
  // cout << "  x " << c1->getX() <<  "  y " << c1->getY() << " sc " << c1->getCornerScale()<< endl;
  for(unsigned int c=0;c<cor.size();c++){    
    if(fabs(sscale-cor[c]->getCornerScale())<0.12){
      //cout << "corscale " << cor[c]->getCornerScale()<<endl;
      dist = ((c1->getX()-cor[c]->getX())*(c1->getX()-cor[c]->getX())+
	(c1->getY()-cor[c]->getY())*(c1->getY()-cor[c]->getY()));
      if(dist<max_dist){
	dl=fabs(dl1-(cor[c]->getL2()/cor[c]->getL1()));
	//cout << "dist " << dist<< " dl1 " << dl1 << " dl "<< dl<< endl;//getchar();
	if(dl<0.1){
	  //	  cout << "dl1 " << dl1<< " dl2 "<<(cor[c]->getL1()/cor[c]->getL2())<<endl;
	  //cout << "dl " << dl<<endl;
	  if(fabs(c1->getAngle()-cor[c]->getAngle())<0.25){
	    //cout << "da " <<(c1->getAngle()-cor[c]->getAngle()) <<endl;
	    //cout << "c " << c << endl;
	    cor_sim.push_back(c);
	  }
	}	
      }
    }
  }
  //  if(c1->getX()==37&&c1->getY()==518){cout <<endl<<endl;getchar();}
  int mimc=-1;
  float mindl=0.12;
  for(unsigned int c=0;c<cor_sim.size();c++){    
    dl=fabs(dl1-(cor[cor_sim[c]]->getL2()/cor[cor_sim[c]]->getL1()));
    if(dl<mindl){
      mindl=dl;mimc=cor_sim[c];
    }    	    
  }
  
  return mimc;
  
}

int searchSimilarPoint(CornerDescriptor *c1, vector<CornerDescriptor*> cor, float sscale, float max_dist){
  float dist;
  vector<int> cor_sim;
  float dl;
    float dl1=(c1->getL2()/c1->getL1());
  float mindl=0.15;
  // cout << "search scale " << sscale<< endl;
  // cout << "  x " << c1->getX() <<  "  y " << c1->getY() << " sc " << c1->getCornerScale()<< endl;
  for(unsigned int c=0;c<cor.size();c++){    
    if(fabs(sscale-cor[c]->getCornerScale())<0.1){
      //if(fabs(c1->getAngle()-cor[c]->getAngle())<0.25){
      dl=fabs(dl1-(cor[c]->getL2()/cor[c]->getL1()));
      if(dl<mindl || fabs(c1->getAngle()-cor[c]->getAngle())<0.1){
	dist = ((c1->getX()-cor[c]->getX())*(c1->getX()-cor[c]->getX())+
		(c1->getY()-cor[c]->getY())*(c1->getY()-cor[c]->getY()));
	if(dist<max_dist){
	  cor_sim.push_back(c);
	}
      }	
	// }
    }
  }
  //  if(c1->getX()==37&&c1->getY()==518){cout <<endl<<endl;getchar();}
  int mimc=-1;
  for(unsigned int c=0;c<cor_sim.size();c++){
    dl=fabs(dl1-(cor[cor_sim[c]]->getL2()/cor[cor_sim[c]]->getL1()));
	//    da=    fabs(c1->getAngle()-cor[c]->getAngle());
    if(dl<mindl){
      mindl=dl;mimc=cor_sim[c];
    }    	    
  }
  
  return mimc;
  
}

void selectBestOne(vector<CornerDescriptor*>&cor_suite, vector<CornerDescriptor*>&cor_sel){  
  
  int OK=0,maxi=0;
  float maxlap=0;
  int size=(int)cor_suite.size();
  //cout << "size "<< size
  //cout << endl;
  for(int i=0;i<size;i++){
    /*       cout <<i<< ". " << cor_suite[i]->getX()<< " " <<  cor_suite[i]->getY();
    cout << " corn "<<  cor_suite[i]->getCornerness() << " angle "<<  cor_suite[i]->getAngle();
    cout << " lap " << cor_suite[i]->getLap() << " l1 " << cor_suite[i]->getL1();
    cout << " l1 " << cor_suite[i]->getL2()<< " l2/l1 "<<(cor_suite[i]->getL2()/cor_suite[i]->getL1());
    cout  << " max "<< cor_suite[i]->isExtr()<< endl;
    */
  }
  for(int i=0;i<size;i++){
    if(cor_suite[i]->isExtr()){
      if(maxlap<fabs(cor_suite[i]->getLap())){
	maxlap=fabs(cor_suite[i]->getLap());
	maxi=i;
	OK=1;
      }
    }
  }
  if(OK)cor_sel.push_back(cor_suite[maxi]);
  else {
    maxlap=0;
    for(int i=0;i<size;i++){
      if(maxlap<fabs(cor_suite[i]->getLap())){
	  maxlap=fabs(cor_suite[i]->getLap());
	maxi=i;
	OK=1;
      }      
    }
    if(OK)cor_sel.push_back(cor_suite[maxi]);    
  }  
  // cout << " sel "<< maxi<< endl;getchar();
  if(size>4){
    if(maxi<=(size>>1)){
      cor_sel.push_back(cor_suite[size-1]);
    } else {
      cor_sel.push_back(cor_suite[0]);
    }    
  }
  
}


void thresholdCorners(vector<CornerDescriptor*>&cor){
  
  vector<CornerDescriptor*> cor_tmp;
  for(int i=0;i<(int)cor.size();i++){
    if(cor[i]->getCornerness()>2000 && fabs(cor[i]->getLap())>15 && cor[i]->isExtr())    
      cor_tmp.push_back(cor[i]);        
  }  
  cor.clear();
  cor=cor_tmp;
}


void selectSimilarPoints(vector<CornerDescriptor*>&cor, vector<float> scales){
 
  vector<CornerDescriptor*> cor_sel;
  vector<CornerDescriptor*> cor_tmp;
  vector<CornerDescriptor*> cor_single;
  int similar=-1, scalei=0,flag;
  do{
    cor_tmp.push_back(cor[0]);
    cor.erase((std::vector<CornerDescriptor*>::iterator)&cor[0]);
    scalei=-1;
    //cout << "scale " << (cor_tmp[cor_tmp.size()-1]->getCornerScale())<< endl;
    for(unsigned int i=0;i<(scales.size()-1);i++){
	if(fabs((cor_tmp[cor_tmp.size()-1]->getCornerScale())-scales[i])<0.01){
	    scalei=i+1;
	}
    }
    flag=1;
    if(scalei!=-1){
      do{
	  similar = searchSimilarPoint(cor_tmp[cor_tmp.size()-1], cor,  
				       scales[scalei], 0.8*scales[scalei]*scales[scalei]);
	  if(similar!=-1){
	      flag=0;
	      cor_tmp.push_back(cor[similar]);
	      cor.erase((std::vector<CornerDescriptor*>::iterator)&cor[similar]);
	      if(scalei<(int)(scales.size()-1))scalei++;
	      else similar=-1;
	  } else if(flag){
	      cor_single.push_back(cor_tmp[cor_tmp.size()-1]);
	      cor_tmp.erase((std::vector<CornerDescriptor*>::iterator)&cor_tmp[cor_tmp.size()-1]);
	  }	
      }while(similar!=-1);    
      if(cor_tmp.size()>0){
	  selectBestOne(cor_tmp,cor_sel);
	  cor_tmp.clear();
      }
    }else{
	cor_single.push_back(cor_tmp[cor_tmp.size()-1]);
	cor_tmp.erase((std::vector<CornerDescriptor*>::iterator)&cor_tmp[cor_tmp.size()-1]);
    }
  }while(cor.size()>0);
  cout << endl;
  cout << "nb sel cor "<< cor_sel.size()<< endl;
  cout << "nb sing  cor "<< cor_single.size()<< endl;
  //  writeCorners(cor_sel, "suit.cor");
  //writeCorners(cor_single, "rest.cor");
  thresholdCorners(cor_single);
  cout << "nb sing thres  cor "<< cor_single.size()<< endl;
  for(int i=0;i<(int)cor_single.size();i++){
      cor_sel.push_back(cor_single[i]);
  }
  cor_single.clear();
  cor=cor_sel;
}
 

/****COMPUTING LAPLACIAN AND REJECTING IF IT IS NOT A MAXIMUM******/
void computLaplacian(DARY* image, vector<CornerDescriptor*>&corners, float lap_threshold, float step){
    float sc,dxx1,dxx0,dxx2,sc0,sc2;
    for (unsigned int c =0  ; c < corners.size(); c++){
      cout << "\rcorner "<< c<<flush;
      sc=corners[c]->getCornerScale();//compute the integration scale
      int row = (int)corners[c]->getY();
      int col = (int)corners[c]->getX();
      sc0=sc/step;
      sc2=sc*step;
      dxx0=sc0*sc0*dXX_YYf(col,row,image,sc0);
      dxx1=sc*sc*dXX_YYf(col,row,image,sc);
      dxx2=sc2*sc2*dXX_YYf(col,row,image,sc2);
      corners[c]->setLap(dxx1);
      if(dxx1>dxx2&& dxx1>dxx0)corners[c]->setMax();//is OK
      else if(dxx1<dxx2&& dxx1<dxx0)corners[c]->setMin();//is OK
    }  
    for(uint i=0;i<corners.size();i++){
      if(!corners[i]->isExtr()){
        delete corners[i];
	corners.erase((std::vector<CornerDescriptor*>::iterator)&corners[i]);
	i--;
      }
    }    
}
void findMax(DARY* harris_im, vector<CornerDescriptor*> &corners,float threshold,float scale, int type){
    
  CornerDescriptor *cor;
  float act_pixel;
  int border=5;//(int)rint(GAUSS_CUTOFF*scale+2);
  for (uint row = border ; row+border  < harris_im->y(); row++){
    for (uint col =  border; col+border  < harris_im->x(); col++){
      act_pixel=harris_im->fel[row][col];
      if(act_pixel > harris_im->fel[row-1][col-1] &&
	 act_pixel > harris_im->fel[row-1][col] &&
	 act_pixel > harris_im->fel[row-1][col+1] &&
	 act_pixel > harris_im->fel[row][col-1] &&
	 act_pixel > harris_im->fel[row][col+1] &&
	 act_pixel > harris_im->fel[row+1][col-1] &&
	 act_pixel > harris_im->fel[row+1][col] &&
	 act_pixel > harris_im->fel[row+1][col+1] &&
	 act_pixel>threshold){
	cor=new CornerDescriptor((float)col,(float)row, act_pixel, scale);
	cor->setType(type);
	//if(cor->isOK())
	corners.push_back(cor);
      }
    }
  }   
  //harris_im->write("harris.pgm");   
}


int checkMaskHarris(DARY *harris, DARY *mask, int x, int y, float threshold){
  float val=harris->fel[y][x];  
  for(int j=y-1;j<y+2;j++){
    for(int i=x-1;i<x+2;i++){    
      if((mask->fel[j][i]>0 && val<harris->fel[j][i]) || val<threshold)return 0;
    }
  }
  return 1;
}

void removeNeighbours(vector<CornerDescriptor*> &corners){

  vector<CornerDescriptor*> cors;
  CornerDescriptor* cor;
  int rad;
  if(corners.size()<100)return;
  cout << endl;
  do{
    cout <<"\r corner: " <<  corners.size() << "  " << flush;
    cor=corners[0];
    rad=(int)(cor->getCornerScale()/2.0);
    corners.erase((std::vector<CornerDescriptor*>::iterator)&corners[0]);
    for(int i=0;(uint)i<corners.size() && i>=0;i++){
      if(cor->getCornerScale()==corners[i]->getCornerScale())
	if(fabs(cor->getX()-corners[i]->getX())<rad && fabs(cor->getY()-corners[i]->getY())<rad)
	  if(cor->getCornerness()>corners[i]->getCornerness()){
	    corners.erase((std::vector<CornerDescriptor*>::iterator)&corners[i]);i--;
	  }else {
	    cor=corners[i];
	    corners.erase((std::vector<CornerDescriptor*>::iterator)&corners[i]);i=-1;	    
	  }  
    }
    cors.push_back(cor);
  }while(corners.size()>0);
  corners=cors;
}

void findMaxMask(vector<DARY*> harris_im, DARY* mask,vector<CornerDescriptor*> &corners,float threshold,vector<float> scale, int type){
  
  CornerDescriptor *cor;
  float  maxh;
  int maxi, nb=0;;
  int border=5;//(int)rint(GAUSS_CUTOFF*scale+2);
  for (uint row = border ; row+border  < mask->y(); row++){
    for (uint col =  border; col+border  < mask->x(); col++){
      if(mask->fel[row][col]>0){
	nb++;maxh=threshold;maxi=-1;
	for(uint i=0;i<harris_im.size();i++){
	  if(checkMaskHarris(harris_im[i],mask,col,row,threshold)){
	    maxh=harris_im[i]->fel[row][col];
	    maxi=i;
	    cor=new CornerDescriptor((float)col,(float)row, harris_im[maxi]->fel[row][col], scale[maxi]);
	    cor->setType(type);
	    corners.push_back(cor);
	  }
	}
	if(maxi!=-1){
	  //cor=new CornerDescriptor((float)col,(float)row, harris_im[maxi]->fel[row][col], scale[maxi]);
	  //cor->setType(type);
	  //corners.push_back(cor);
	  
	}
      }
    }
  }   
  cout << "nb edges "<< nb << endl;
  //  removeNeighbours(corners);

}
/****** HARRIS***********/
void harris(DARY *image_in, DARY *corner_image,
	    float derive_sigma,
	    float sigma, 
	    float alpha){
    
    
   int col_nb, row_nb;
   float A, B, C, determinant, trace, t1,t2;
   
   row_nb = image_in->y();
   col_nb = image_in->x();
   
   //printf("- Starting to calculate gradients \n");
   DARY  *fx= new DARY(row_nb,col_nb);	
   dX(image_in, fx, derive_sigma);
   DARY  *fy= new DARY(row_nb,col_nb);	
   dY(image_in, fy, derive_sigma);
   DARY *fxy  = new DARY(row_nb, col_nb);
   int row, col;
   float norm=derive_sigma;//pow(derive_sigma,1.3);
   for ( row = 0; row < row_nb; row++)
       for ( col = 0; col < col_nb; col++){
	   t1 = norm*fx->fel[row][col];
	   t2 = norm*fy->fel[row][col];
	   fx->fel[row][col] = t1*t1;
	   fy->fel[row][col] = t2*t2;
	   fxy->fel[row][col] = t1*t2;
       }

   DARY *sfx2  = new DARY(row_nb, col_nb);   
   smooth(fx,sfx2, sigma);  delete fx;
   DARY *sfy2  = new DARY(row_nb, col_nb);
   smooth(fy,sfy2, sigma);  delete fy;
   DARY *sfxy  = new DARY(row_nb, col_nb);
   smooth(fxy,sfxy, sigma); delete fxy;

   for ( row = 0; row < row_nb; row++)
     for (int col = 0; col < col_nb; col++)
       {
	 /*        A = B = C = determinant = trace = 0.0;*/
        A = sfx2->fel[row][col];
        B = sfy2->fel[row][col];
        C = sfxy->fel[row][col];
        determinant = A * B - (C*C);
        trace = A + B;
        corner_image->fel[row][col] = (determinant - alpha * (trace*trace));
       }
   
   delete sfx2; delete sfy2;  delete sfxy;
}

/****** HARRIS***********/
 
/***************LOCAL MAXIMA IN IMAGE PLANE ******************/

/******************HARRIS DETECTOR**************/
void harris(DARY* image, vector<CornerDescriptor*>&corners,
	    float threshold,
	    float derive_sigma,
	    float sigma, 
	    float alpha){
  DARY * corimage = new DARY(image->y(),image->x());
  harris(image, corimage,derive_sigma,sigma, alpha);
  findMax(corimage, corners, threshold, derive_sigma, 1);
  delete corimage;
}


/******************MULTI-SCALE HARRIS DETECTOR**************/
void multi_harris(DARY* image, vector<CornerDescriptor*>&corners, float har_threshold,
		    float min_scale, float max_scale, float step){
  vector<float> scale;
  for(float s=min_scale;s<max_scale;s*=step)scale.push_back(s);  
  
  DARY* harris_im = new DARY(image->y(),image->x()); 

  for(unsigned int i=0;i<scale.size();i++){ 
      cout << "\rscale "<< scale[i]<<  "   " << flush;
    harris(image, harris_im, scale[i], (1.4)*scale[i],ALPHA);
    findMax(harris_im,corners, har_threshold, scale[i], /*type harris 1*/ 1);
  }
  delete harris_im;
  scale.clear(); 
}

/******************MULTI-SCALE HARRIS DETECTOR**************/
void multi_harris_mask(DARY* image, DARY* mask, vector<CornerDescriptor*>&corners, float har_threshold,
		    float min_scale, float max_scale, float step){
  if(image->x()!=mask->x() || image->y()!=mask->y()){
    cout << "mask and image size are different in multi_harris_mask"<< endl;return;
  }
  vector<float> scale;
  for(float s=min_scale;s<max_scale;s*=step)scale.push_back(s);  
  
  // DARY* harris_im = new DARY(image->y(),image->x()); 
  vector<DARY *> har;

  for(unsigned int i=0;i<scale.size();i++){ 
      cout << "\rscale "<< scale[i]<<  "   " << flush;
      har.push_back(new DARY(image->y(),image->x()));
      harris(image, har[i], scale[i], (1.4)*scale[i],ALPHA);
  }
  findMaxMask(har, mask, corners, har_threshold, scale, /*type harris 1*/ 1);
  //delete harris_im;
  har.clear();
  scale.clear();
}


/****** HESSIAN DETECTOR***********/
void hessian(DARY *image_in, DARY *corner_image,
	    float derive_sigma){
    
    
   int col_nb, row_nb;
   float A, B, C, determinant;
   
   row_nb = image_in->y();
   col_nb = image_in->x();
    
   //printf("- Starting to calculate gradients \n");
   DARY  *fxx= new DARY(row_nb,col_nb);	
   DARY  *fyy= new DARY(row_nb,col_nb);	
   DARY  *fxy= new DARY(row_nb,col_nb);	
   dXX(image_in, fxx, derive_sigma);
   dYY(image_in, fyy, derive_sigma);
   dXY(image_in, fxy, derive_sigma);
   int row, col;

   float norm=derive_sigma*derive_sigma;
   for ( row = 0; row < row_nb; row++)
       for ( col = 0; col < col_nb; col++){
	   fxx->fel[row][col] = (norm*fxx->fel[row][col]);
	   fyy->fel[row][col] = (norm*fyy->fel[row][col]);
	   fxy->fel[row][col] = (norm*fxy->fel[row][col]);
       }
   float CC,AB;
   for ( row = 0; row < row_nb; row++)
     for (int col = 0; col < col_nb; col++)
       {
	 /*        A = B = C = determinant = trace = 0.0;*/
        A = fxx->fel[row][col];
        B = fyy->fel[row][col];
        C = fxy->fel[row][col];
	CC=(C*C);
	AB=(A*B);
        determinant = AB - (CC);
        corner_image->fel[row][col] =(determinant) ;//- alpha * (trace*trace);	
       }
   delete fxx; delete fyy; delete fxy; 
}

/******************HESSIAN DETECTOR**************/
void hessian(DARY* image, vector<CornerDescriptor*>&corners,
	    float threshold,
	     float derive_sigma){
  DARY * corimage = new DARY(image->y(),image->x());
  hessian(image, corimage,derive_sigma);
  findMax(corimage, corners, threshold, derive_sigma,2);
  delete corimage;
}

/******************MULTI HESSIAN DETECTOR**************/
void multi_hessian(DARY* image, vector<CornerDescriptor*>&corners, 
		   float lap_threshold,
		   float min_scale, float max_scale, float step){
  vector<float> scale;
  for(float s=min_scale;s<max_scale;s*=step)scale.push_back(s);  
  
  DARY* harris_im = new DARY(image->y(),image->x()); 
  
  for(unsigned int i=0;i<scale.size();i++){ 
    cout << "\rscale "<< scale[i]<<  "   " << flush;
    hessian(image, harris_im, scale[i]);
    findMax(harris_im,corners, lap_threshold, scale[i], /*type hessian 2*/ 2);
  }
  delete harris_im;
}




/******************MULTI HESSIAN DETECTOR**************/

/******************MULTI HESSIAN DETECTOR**************/

/******************MULTI HESSIAN DETECTOR**************/

/******************MULTI HESSIAN DETECTOR**************/

/******************MULTI HESSIAN DETECTOR**************/

/******************MULTI HESSIAN DETECTOR**************/

/******************MULTI HESSIAN DETECTOR**************/

void laplacian(DARY *sm, DARY * lap){
  
  DARY *fxx = new DARY(sm->y(), sm->x());
  DARY *fyy = new DARY(sm->y(), sm->x());
  
  dXX7(sm,fxx);
  dYY7(sm,fyy);
  for(uint x=0;x<fxx->x();x++){
    for(uint y=0;y<fxx->y();y++){
      lap->fel[y][x]=fxx->fel[y][x]+fyy->fel[y][x];
    }
  }
  delete fxx;delete fyy;
  //lap->write("lap.pgm");cout << "ci"<< endl;getchar();
}
/****** HESSIAN DETECTOR***********/
void hessian(DARY *image_in, DARY *hes, DARY *lap){
    
    
   int col_nb, row_nb;
   float A, B, C, determinant;
   
   row_nb = image_in->y();
   col_nb = image_in->x();
    
   //printf("- Starting to calculate gradients \n");
   DARY  *fxx= new DARY(row_nb,col_nb);	
   DARY  *fyy= new DARY(row_nb,col_nb);	
   DARY  *fxy= new DARY(row_nb,col_nb);	
   dXX9(image_in, fxx);//fxx->write("fxx.pgm");
   dYY9(image_in, fyy);//fyy->write("fyy.pgm");
   dXY9(image_in, fxy);//fxy->write("fxy.pgm");//getchar();

   int row, col;
   float CC,AB;
   for ( row = 0; row < row_nb; row++)
     for ( col = 0; col < col_nb; col++)
       {
	 /*        A = B = C = determinant = trace = 0.0;*/
        A = fxx->fel[row][col];
        B = fyy->fel[row][col];
        C = fxy->fel[row][col];
	CC=(C*C);
	AB=(A*B);
        determinant = AB-20.0*(CC);
	lap->fel[row][col]=A+B;
        hes->fel[row][col] =(determinant);	
       }
   //hes->write("hes.pgm");cout << "ci"<< endl;getchar();
   delete fxx; delete fyy; delete fxy; 
}

void harris(DARY *img,DARY *har,DARY *har11,DARY *har12,DARY *har22){    
  
   int col_nb, row_nb;
   float A, B, C, determinant, trace, t1,t2;
   
   row_nb = img->y();
   col_nb = img->x();
   
   //   printf("- Starting to calculate gradients %d %d\n",col_nb, row_nb);
   DARY  *fx= new DARY(row_nb,col_nb);	
   dX6(img, fx);
   DARY  *fy= new DARY(row_nb,col_nb);	
   dY6(img, fy);
   DARY *fxy  = new DARY(row_nb, col_nb);
   int row, col;
   for ( row = 0; row < row_nb; row++)
       for ( col = 0; col < col_nb; col++){
	   t1 = fx->fel[row][col];
	   t2 = fy->fel[row][col];
	   fx->fel[row][col] = t1*t1;
	   fy->fel[row][col] = t2*t2;
	   fxy->fel[row][col] = t1*t2;
       }

   smoothSqrt(fx,har11);  delete fx;
   smoothSqrt(fy,har22);  delete fy;
   smoothSqrt(fxy,har12); delete fxy;

   for ( row = 0; row < row_nb; row++)
     for ( col = 0; col < col_nb; col++)
       {
	 /*        A = B = C = determinant = trace = 0.0;*/
        A = har11->fel[row][col];
        B = har22->fel[row][col];
        C = har12->fel[row][col];
        determinant = A * B - (C*C);
        trace = A + B;
        har->fel[row][col] = (determinant - ALPHA * (trace*trace));
       }
   //har->write("har.pgm");cout << "ci"<< endl;getchar();

}


int getLapMax(vector<DARY *> lap, vector<float> scale, uint level, uint minlev, uint maxlev, float x, float y,float &sc){

  vector<float> llap;
  float fx,fy,lp,lthres=10;
  for(uint i=0; i<lap.size() && i<(level+maxlev);i++){
    fx=x/scale[i];
    fy=y/scale[i];
    if(fx>5 && fx< (lap[i]->x()-5) && fy>5 && fy< (lap[i]->y()-5)){
      lp=lap[i]->getValue(fx,fy);
      //cout << fx << " "<< fy<< " "<< lp<<  " l "<< i <<endl;
      llap.push_back(lp);
    }  
  } 
  if(level<minlev+2)minlev=level-2;
  for(uint i=level-minlev;i<(level+maxlev) && i+2<llap.size();i++){
    if(llap[i-1]> llap[i-2] && llap[i]> llap[i-1] && llap[i]>llap[i+1] && llap[i+1]>llap[i+2] && llap[i]>lthres){
      float ds=0.5*(llap[i-1]-llap[i+1])/(llap[i-1]+llap[i+1]-2*llap[i]);
      if(ds>0){
	sc=scale[i]+ds*(scale[i+1]-scale[i]);	
      }else {	
	sc=scale[i-1]+(1-ds)*(scale[i]-scale[i-1]);	
      }
      //cout << "+scale i "<<scale[i] << " "<< scale[i+1]<< " ds " << ds<<  " dsc "<< ds*(scale[i+1]-scale[i])<<" sc "<< sc <<  endl;getchar();
      //cout << "pos "<<llap[i]<< " "<< i<< " "<< scale[i]<< endl;getchar();
      llap.clear();      
      return i;      
    } else if(llap[i-1]< llap[i-2] && llap[i]< llap[i-1] && llap[i]<llap[i+1] && llap[i+1]<llap[i+2] && llap[i]<-lthres){      
      //cout << "neg "<<llap[i]<< " "<< i << " "<< scale[i]<< endl;getchar();
      float ds=0.5*(llap[i-1]-llap[i+1])/(llap[i-1]+llap[i+1]-2*llap[i]);
      if(ds>0){
	sc=scale[i]+ds*(scale[i+1]-scale[i]);	
      }else {	
	sc=scale[i-1]+(1-ds)*(scale[i]-scale[i-1]);	
      }
      //cout << "-scale i "<<scale[i] << " "<< scale[i+1] << " ds " << ds<<  " dsc "<< ds*(scale[i+1]-scale[i])<<" sc "<< sc <<  endl;getchar();
      llap.clear();    
      return i;
    }
    
  }
  llap.clear();
  return -1;
}
void invSqrRoot(float &a, float &b, float &c, float &r_l, float &r_m){
  
  //cout << "in a " << a << " b " << b << " c " << c << endl;
  float cos_2t = a - c;
  float sin_2t =  2*b;
  float r = sqrt(cos_2t*cos_2t + sin_2t*sin_2t);
  float u,v,l,m;
  if (r == 0) {
    // We have a = c and b = 0, i.e. the matrix is diagonal.
    u = 1;
    v = 0;
    l = a;
    m = c;
    return;
  }else{
    cos_2t /= r;
    sin_2t /= r;
    // use half-angle formulae:
    float cos_t = sqrt((1 + cos_2t)/2);
    float sin_t = sqrt(1 - cos_t*cos_t);
    if (sin_2t < 0)
      sin_t = -sin_t;
    u = cos_t;  
    v = sin_t;
    
    l = a*cos_t*cos_t + 2*b*cos_t*sin_t + c*sin_t*sin_t;
    m = a*sin_t*sin_t - 2*b*cos_t*sin_t + c*cos_t*cos_t;
    //cout << " l "<< l << " m "<< m << " r " << r<< " cos_t " << cos_t<< " sin_t "<< sin_t<< endl;  
  }
 
  if ((l >= 0) && (m >= 0)) {
    r_l = 1/sqrt(l);
    r_m = 1/sqrt(m);
    float x=sqrt(r_l*r_m);
    r_l/=x;
    r_m/=x;
    a = r_l*u*u + r_m*v*v;
    b = r_l*u*v - r_m*u*v;
    c = r_l*v*v + r_m*u*u;
    //cout << "out a " << a << " b " << b << " c " << c << endl;
  }else{

    cout << "errors"<< endl;
  }  
}


void harris_lap(vector<DARY *> har,vector<DARY *> har11,vector<DARY *> har12,vector<DARY *> har22, vector<DARY *> lap, vector<float> scale, vector<CornerDescriptor*>&corners, float threshold, int type){
  
  CornerDescriptor *cor,*cor1;
  float act_pixel,fx,fy;
  int level;
  int border=5;//(int)rint(GAUSS_CUTOFF*scale+2);
  vector<CornerDescList>tmpcor;
  for(uint i=0;i<har.size()-1;i++){
    tmpcor.push_back(CornerDescList(0));
  }  
  float a,b,c,l1,l2,fcol,frow,sc,dc=0,dr=0,thres=threshold/2.0;  
  int cbad=0,cgood=0;
  for(uint i=2;i<har.size()-1;i++){
    for (uint row = border ; row+border  < har[i]->y(); row++){
      for (uint col =  border; col+border  < har[i]->x(); col++){
	act_pixel=har[i]->fel[row][col];
	if(act_pixel > har[i]->fel[row-1][col-1] &&
	   act_pixel > har[i]->fel[row-1][col] &&
	   act_pixel > har[i]->fel[row-1][col+1] &&
	   act_pixel > har[i]->fel[row][col-1] &&
	   act_pixel > har[i]->fel[row][col+1] &&
	   act_pixel > har[i]->fel[row+1][col-1] &&
	   act_pixel > har[i]->fel[row+1][col] &&
	   act_pixel > har[i]->fel[row+1][col+1] &&
	   har[i]->fel[row-1][col-1]>thres &&
	   har[i]->fel[row-1][col]>thres &&
	   har[i]->fel[row-1][col+1]>thres &&
	   har[i]->fel[row][col-1]>thres &&
	   har[i]->fel[row][col+1]>thres &&
	   har[i]->fel[row+1][col-1]>thres &&
	   har[i]->fel[row+1][col]>thres &&
	   har[i]->fel[row+1][col+1]>thres &&	   
	   act_pixel>threshold){
	  /*dc=0.0*(har[i]->fel[row][col-1]-har[i]->fel[row][col+1])/
	    (har[i]->fel[row][col-1]+har[i]->fel[row][col+1]-2*act_pixel);
	  dr=0.0*(har[i]->fel[row-1][col]-har[i]->fel[row+1][col])/
	    (har[i]->fel[row-1][col]+har[i]->fel[row+1][col]-2*act_pixel);
	  if(dc<-0.5 || dc>0.5 || dr<-0.5 || dr>0.5){
	    cout<< dc << " " << dr << endl;
	    }*/
	  fcol=(float)col+dc;
	  frow=(float)row+dr;
	  //cout<< col << "  "<< row << "  "<< scale[i]*col << " " << scale[i]*row<<  endl;
	  //cout << fcol << " " << frow<< "  "<< scale[i]*fcol << " " << scale[i]*frow<<  endl;getchar();
	  fx=scale[i]*fcol;
	  fy=scale[i]*frow;
	  level=getLapMax(lap, scale, i, 1, 7, fx, fy, sc);
	  //fx+=0.001;
	  //fy+=0.001;
	  if(level>0){cgood++;
	    cor=new CornerDescriptor(fx,fy, act_pixel, 3*sc);
	    cor->setDerSig(scale[i]);
	    cor->setIntSig(2*sc);
	    cor->setDerLev(i);
	    cor->setIntLev(level);
	    cor->setType(type);
	    if(type==1){
	      a=har11[i]->fel[row][col];
	      b=har12[i]->fel[row][col];
	      c=har22[i]->fel[row][col];
	      //cout << a << " "<< b << " " << c << endl;
	      invSqrRoot(a,b,c,l2,l1);		      
	      cor->setMi(a,b,b,c);
	    }else {
	      cor->setMi(1,0,0,1);
	    }
	    //if(cor->isOK())
	    //if(ok)
	      tmpcor[level].push_back(cor);
	      //ok=1;
	  }else cbad++;
	}
      }
    }   
  }
  #ifndef QUIET
  cout << "cgood "<< cgood << " cbad "<< cbad << " all " << (cgood+cbad) << endl;
  #endif

//  for(uint i=0;i<tmpcor.size();i++){
//    for(uint c=0;c<tmpcor[i].size();c++){
//      flag=1;
//      cor=tmpcor[i][c];
//      for(uint s=i;s<i+2 && s<tmpcor.size() && flag;s++){
//	for(uint c1=0;c1<tmpcor[s].size() && flag;c1++){
//	  if(cor!=tmpcor[s][c1]){
//	    cor1=tmpcor[s][c1];
//	    if(fabs(cor->getX()-cor1->getX())<0.3*scale[i] && fabs(cor->getY()-cor1->getY())<0.3*scale[i]){
//	      if(cor->getCornerness()>cor1->getCornerness()){
//		//tmpcor[s].erase((std::vector<CornerDescriptor*>::iterator)&tmpcor[s][c1]);
//		//c1--;
//	      } else{
//		//tmpcor[i].erase((std::vector<CornerDescriptor*>::iterator)&tmpcor[i][c]);
//		//c--;
//                flag=0;	
//	      }
//	    }
//	  }
//	}
//      }
//      if (flag) corners.push_back(tmpcor[i][c]);
//    }
//  }
  int flag=1;
  for(uint i=0;i<tmpcor.size();i++){
    for(uint c=0;c<tmpcor[i].size();c++){
      flag=1;
      cor=tmpcor[i][c];
      for(uint s=i;s<i+2 && s<tmpcor.size() && flag;s++){
	for(uint c1=0;c1<tmpcor[s].size() && flag;c1++){
	  if(cor!=tmpcor[s][c1]){
	    cor1=tmpcor[s][c1];
	    if(fabs(cor->getX()-cor1->getX())<0.3*scale[i] && fabs(cor->getY()-cor1->getY())<0.3*scale[i]){
	      if(cor->getCornerness()>cor1->getCornerness()){
                delete tmpcor[s][c1];
		tmpcor[s].erase((std::vector<CornerDescriptor*>::iterator)&tmpcor[s][c1]);
		c1--;
	      } else{
                delete tmpcor[i][c];
		tmpcor[i].erase((std::vector<CornerDescriptor*>::iterator)&tmpcor[i][c]);
		c--;flag=0;	
	      }
	    }
	  }
	}
      }
    }
  }
 
  for(uint i=0;i<tmpcor.size();i++){
    for(uint c=0;c<tmpcor[i].size();c++){
      corners.push_back(tmpcor[i][c]);
    }
   tmpcor[i].clear();
  }  
  tmpcor.clear();
}

void findAffineRegion(vector<DARY *> image,vector<DARY *> laplacian, vector<float> scale ,vector<CornerDescriptor*> &cor, int lmax);


void multi_scale_har(DARY* img, vector<CornerDescriptor*>&corners, 
		   float threshold,
		   float step, int aff){

  vector<DARY *> sm; 
  vector<DARY *> lap; 
  vector<DARY *> har; 
  vector<DARY *> har11; 
  vector<DARY *> har12; 
  vector<DARY *> har22; 
  vector<float> sc;
  int sx=img->x(),sy=img->y();
  sm.push_back(new DARY(sy,sx));
  har.push_back(new DARY(sy,sx));
  har11.push_back(new DARY(sy,sx));
  har12.push_back(new DARY(sy,sx));
  har22.push_back(new DARY(sy,sx));
  lap.push_back(new DARY(sy,sx));
  sc.push_back(1);
  while(sx>12 && sy>12){
    sx=(int)rint(sx/step);sy=(int)rint(sy/step);    
    sm.push_back(new DARY(sy,sx));
    lap.push_back(new DARY(sy,sx));
    har.push_back(new DARY(sy,sx));
    har11.push_back(new DARY(sy,sx));
    har12.push_back(new DARY(sy,sx));
    har22.push_back(new DARY(sy,sx));
    sc.push_back(step*sc[sc.size()-1]);
  }

  // sm[0]->set(img);
  smoothSqrt(img,sm[0]);
  sm[1]->scale(sm[0],step,step);
  float sc2=step*step;
  //sm[2]->scale(sm[0],sc2,sc2);
  //float sc3=step*step*step;
 
  for(uint i=2;i<sm.size()-1;i+=2){
    sm[i]->scale(sm[i-2],sc2,sc2);
    sm[i+1]->scale(sm[i-1],sc2,sc2);
    //sm[i+2]->scale(sm[i-1],sc3,sc3);
  }

  laplacian(sm[0],lap[0]);
  for(uint i=1;i<sm.size();i++){
    harris(sm[i],har[i],har11[i],har12[i],har22[i]);
    laplacian(sm[i],lap[i]);//sm[i]->write("sm.pgm");har[i]->write("har.pgm");lap[i]->write("lap.pgm");getchar();
    //cout <<i << " "<< sc[i]<< endl;
  }
   
  if(aff==0)harris_lap(har,har11,har12,har22,lap,sc,corners,threshold,0);
  else harris_lap(har,har11,har12,har22,lap,sc,corners,threshold,1);


  if(aff>1)findAffineRegion(sm,lap,sc,corners,aff);


  for(uint i=0;i<sm.size();i++){
    delete sm[i];delete lap[i];delete har[i];delete har11[i];delete har12[i];delete har22[i];
  }
  sc.clear();
  sm.clear();
  lap.clear();  
  har.clear();  
  har11.clear();  
  har12.clear();  
  har22.clear();  
}


void multi_scale_hes(DARY* img, vector<CornerDescriptor*>&corners, 
		     float threshold,
		     float step, int aff){
  vector<DARY *> sm; 
  vector<DARY *> lap; 
  vector<DARY *> hes; 
  vector<float> sc;
  int sx=img->x(),sy=img->y();
  sm.push_back(new DARY(sy,sx));
  hes.push_back(new DARY(sy,sx));
  lap.push_back(new DARY(sy,sx));
  sc.push_back(1);
  while(sx>12 && sy>12){
    sx=(int)rint(sx/step);sy=(int)rint(sy/step);    
    sm.push_back(new DARY(sy,sx));
    lap.push_back(new DARY(sy,sx));
    hes.push_back(new DARY(sy,sx));
    sc.push_back(step*sc[sc.size()-1]);
  }

  // sm[0]->set(img);
  smoothSqrt(img,sm[0]);
  sm[1]->scale(sm[0],step,step);
  float sc2=step*step;
  //sm[2]->scale(sm[0],sc2,sc2);
  //float sc3=step*step*step;
 
  // Relja fixes:
  /* orig:
  for(uint i=2;i<sm.size()-1;i+=2){
    sm[i]->scale(sm[i-2],sc2,sc2);
    sm[i+1]->scale(sm[i-1],sc2,sc2);
    //sm[i+2]->scale(sm[i-1],sc3,sc3);
  }
//  if (sm.size() % 2) {
//    sm[sm.size() - 1]->scale(sm[sm.size() - 3], sc2, sc2);
//  }
  */
  // Relja fix (don't know the author complicated this, the last one was often uninitialized!)
  for(uint i=2; i<sm.size(); ++i)
      sm[i]->scale(sm[i-2],sc2,sc2);


  for(uint i=0;i<sm.size();i++){   
    hessian(sm[i],hes[i],lap[i]);
    //cout <<i << " "<< sc[i]<< endl;
  }

  harris_lap(hes,hes,hes,hes,lap, sc, corners,threshold,2);  
 
  for(uint i=0;i<sm.size();i++){   
    //char name[512];sprintf(name,"hes%d.pgm",i);hes[i]->write(name);
    //cout <<i << " "<< sc[i]<< endl;
  }



  #ifndef QUIET
  cout << "cor nb "<< corners.size()<< endl;
  #endif
  if(aff>0)findAffineRegion(sm,lap,sc,corners,aff);
  
  for(uint i=0;i<sm.size();i++){
    delete sm[i];delete lap[i];delete hes[i];
  }
  sc.clear();
  sm.clear();
  lap.clear();  
  hes.clear();  
}

void getEigen(float a, float b, float c, float d, float &l1, float &l2){
       float trace = a+d;
       float delta1=(trace*trace-4*(a*d-b*c));
       if(delta1<0){	 
	 l1=1;l2=100;
	 return;
       } 
       float delta = sqrt(delta1);
		    
       l1 = (trace+delta)/2.0;
       l2 = (trace-delta)/2.0;
}

void getMi(DARY *img, float &a, float &b, float &c, float x, float y){
    int row_nb= img->y();
    int col_nb= img->y();    
    float  t1,t2;
    DARY  *fx= new DARY(row_nb, col_nb);	
    DARY  *fy= new DARY(row_nb, col_nb);	
    DARY  *fxy  = new DARY(row_nb, col_nb);
    dX6(img, fx);
    dY6(img, fy);
    for (int row = 0; row < row_nb; row++)
	for (int col = 0; col < col_nb; col++){
	    t1 = fx->fel[row][col];
	    t2 = fy->fel[row][col];
	    fx->fel[row][col] = t1*t1;
	    fy->fel[row][col] = t2*t2;
	    fxy->fel[row][col] = t1*t2;
	}          
    a= smoothf((int)x,(int)y, fx, 3);
    c= smoothf((int)x,(int)y, fy, 3);
    b= smoothf((int)x,(int)y, fxy, 3);
    
    delete fx;delete fy;delete fxy;
}


int fastfindAffineRegion(vector<DARY *> image,vector<DARY *> laplacian,vector<float>scale, CornerDescriptor * cor, int lmax){

  int level = cor->getDerLev();
  float pointx=cor->getX()/scale[level];
  float pointy=cor->getY()/scale[level];
  int sizex=19;//2*(1.44*GAUSS_CUTOFF+3)+1;
  int l,go_on=1;
  float l1=1,l2=1;
  float eigen_ratio_act=0.1,eigen_ratio_bef=0.1,deigen=0.02;
  //static float nba=0; 
  DARY *img=new DARY(sizex,sizex);
  DARY *cimg=image[level];
  float a,b,c,u11=cor->getMi11(),u12=cor->getMi12(),u21=cor->getMi12(),u22=cor->getMi22(),u11t,u12t;
  getEigen(u11,u12,u21,u22,l1,l2);
  eigen_ratio_act=1-l2/l1;
 
  //cout << pointx << " " << pointy << " level " << level << " " << eigen_ratio_act <<endl;getchar();
  for(l=0;l<lmax && go_on;l++){ //estimate affine structure
    img->interpolate(cimg,pointx,pointy,u11,u12,u21,u22);
    //img->write("img.pgm");
    getMi(img, a,b,c, sizex>>1, sizex>>1);
    //cout <<l1 <<"  " << l2 <<" a " << a << " b " <<b << "  c " << c << endl;
    invSqrRoot(a,b,c,l2,l1);
    
    eigen_ratio_bef=eigen_ratio_act;
    eigen_ratio_act=1-l2/l1;
    
    u11t=u11;u12t=u12;
    u11=a*u11t+b*u21;
    u12=a*u12t+b*u22;
    u21=b*u11t+c*u21;
    u22=b*u12t+c*u22;
    
    //cout << u11 << " "<< u12 << endl;
    //cout << u21 << " "<< u22 << endl;
    //cout << endl << l1 << " "<< l2 << endl;
    getEigen(u11,u12,u21,u22,l1,l2); 
    
    if(l>15 || (l1/l2>6) || (l2/l1>6)) {
      delete img;
      return 0;
    }
    
    if(eigen_ratio_act<deigen && eigen_ratio_bef<deigen)go_on=0;       
  }
  delete img;
  //cout <<"eigen_ratio_act "<<eigen_ratio_act<<"  "<< l1<< " "<< l2 << " "<< l<< endl;getchar();
  
  cor->setMi(u11,u12,u21,u22,l1,l2);  
  return l;    
}


void findAffineRegion(vector<DARY *> image,vector<DARY *> laplacian, vector<float>scale,vector<CornerDescriptor*> &cor, int lmax){
  for(int i=0;i<(int)cor.size();i++){        
    int l=fastfindAffineRegion(image,laplacian,scale,cor[i],lmax);    
    if(l!=0){
      //cout<<"\r  cor  "<<i<<" of "<< size << "  "<<cor[i]->getDerLev()<< "  " << cor[i]->getX() << "  " << cor[i]->getY()<<"  yes  "<< flush;
    }else { 
      //cout<<"\r  cor  "<<i<<" of "<< size << "  "<<cor[i]->getDerLev()<< "  " << cor[i]->getX() << "  " << cor[i]->getY()<<"  no  "<< flush;
      delete cor[i];
      cor.erase((std::vector<CornerDescriptor*>::iterator)&cor[i]);i--;}    
  }    
}
