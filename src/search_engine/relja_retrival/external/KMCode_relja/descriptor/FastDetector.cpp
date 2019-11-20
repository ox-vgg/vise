#include "fastDetector.h"

/****** HARRIS FAST derivation sigma = 1, integration sigma =1.4***********/
void harris_fast(DARY *image_in, DARY *corner_image){
    
    
   int col_nb, row_nb;
   float A, B, C, determinant, trace, t1,t2;
   
   row_nb = image_in->y();
   col_nb = image_in->x();
   
   //printf("- Starting to calculate gradients \n");
   DARY  *fx= new DARY(row_nb,col_nb);	
   dX6(image_in, fx);
   DARY  *fy= new DARY(row_nb,col_nb);	
   dY6(image_in, fy);
   DARY *fxy  = new DARY(row_nb, col_nb);
   for (int row = 0; row < row_nb; row++)
       for (int col = 0; col < col_nb; col++){
	   t1 = fx->fel[row][col];
	   t2 = fy->fel[row][col];
	   fx->fel[row][col] = t1*t1;
	   fy->fel[row][col] = t2*t2;
	   fxy->fel[row][col] = t1*t2;
       }

   DARY *sfx2  = new DARY(row_nb, col_nb);   
   smoothSqrt(fx,sfx2);  delete fx;
   DARY *sfy2  = new DARY(row_nb, col_nb);
   smoothSqrt(fy,sfy2);  delete fy;
   DARY *sfxy  = new DARY(row_nb, col_nb);
   smoothSqrt(fxy,sfxy); delete fxy;

   for (int row = 0; row < row_nb; row++)
     for (int col = 0; col < col_nb; col++)
       {
	 /*        A = B = C = determinant = trace = 0.0;*/
        A = sfx2->fel[row][col];
        B = sfy2->fel[row][col];
        C = sfxy->fel[row][col];
        determinant = A * B - (C*C);
        trace = A + B;
        corner_image->fel[row][col] = (determinant - ALPHA * (trace*trace));
       }
    
   delete sfx2; delete sfy2;  delete sfxy;
}

/****** HESSIAN DETECTOR derivation sigma = 1***********/
void hessian_fast(DARY *image_in, DARY *corner_image){
    
    
   int col_nb, row_nb;
   float A, B, C;
   
   row_nb = image_in->y();
   col_nb = image_in->x();
    
   //printf("- Starting to calculate hessian \n");
   DARY  *fxx= new DARY(row_nb,col_nb);	
   DARY  *fyy= new DARY(row_nb,col_nb);	
   DARY  *fxy= new DARY(row_nb,col_nb);	
   dXX7(image_in, fxx);
   dYY7(image_in, fyy);
   dXY7(image_in, fxy); 
   
   for (int row = 0; row < row_nb; row++)
     for (int col = 0; col < col_nb; col++)
       {
	 /*        A = B = C = determinant = trace = 0.0;*/
        A = fxx->fel[row][col];
        B = fyy->fel[row][col];
        C = fxy->fel[row][col];
        corner_image->fel[row][col] =fabs((A*B) - (C*C));	
       }
   delete fxx; delete fyy; delete fxy; 
} 
 


/******************HARRIS - LAPLACE DETECTOR**************/
void multi_harris_fast(DARY* image, vector<CornerDescriptor*>&corners, float har_threshold){
  vector<CornerDescriptor *> cor_tmp;
  
  DARY* image_im = new DARY(image); 
  DARY* harris_im = new DARY(image->y(),image->x()); 
  int levels = 100;
  int sx,sy;
  float scale, rescale, step=1.4;
  for( int i=1;i<levels;i++){ 
    harris_fast(image_im, harris_im);
    scale=pow(step,i);
    findMax(harris_im, cor_tmp, har_threshold, scale, /*type harris 1*/1);
    cout << "\rscale "<< scale <<  "    " << flush;
    rescale=pow(step,i-1);
    for(uint c=0;c<cor_tmp.size();c++){
      cor_tmp[c]->setX_Y(cor_tmp[c]->getX()*rescale,cor_tmp[c]->getY()*rescale);
      corners.push_back(cor_tmp[c]);
    }
    cor_tmp.clear();    
    smooth3(image_im, harris_im);//store smoothed level in harris for a while
    sx=(int)(image_im->x()/step);
    sy=(int)(image_im->y()/step);
    if(sx<20 || sy<20)levels=i;//minimum image size
    delete image_im;
    image_im = new DARY(sy,sx);
    image_im->scale(harris_im,step,step);//rescale new level
    delete harris_im;
    harris_im = new DARY(sy,sx);
  }
  delete harris_im; delete image_im;
}


/******************HESSIAN - LAPLACE DETECTOR**************/
void multi_hessian_fast(DARY* image, vector<CornerDescriptor*>&corners, float hes_threshold){
  vector<CornerDescriptor *> cor_tmp;
  
  DARY* image_im = new DARY(image); 
  DARY* hessian_im = new DARY(image->y(),image->x()); 
  int levels = 100;
  int sx,sy;
  float scale, rescale,  step=1.4;
  for( int i=1;i<levels;i++){ 
    scale=pow(step,i);
    hessian_fast(image_im, hessian_im);
    findMax(hessian_im, cor_tmp, hes_threshold, scale, /*type hessian 2*/2);
    cout << "\rscale "<< scale <<  "      " << flush;
    rescale=pow(step,i-1);
    for(uint c=0;c<cor_tmp.size();c++){
      cor_tmp[c]->setX_Y(cor_tmp[c]->getX()*rescale,cor_tmp[c]->getY()*rescale);
      corners.push_back(cor_tmp[c]);
    }
    cor_tmp.clear();    
    smooth3(image_im, hessian_im);//store smoothed level in hessian for a while
    sx=(int)(image_im->x()/step);
    sy=(int)(image_im->y()/step);
    if(sx<20 || sy<20)levels=i;//minimum image size
    delete image_im;
    image_im = new DARY(sy,sx);
    image_im->scale(hessian_im,step,step);//rescale new level
    delete hessian_im;
    hessian_im = new DARY(sy,sx);
  }
  delete hessian_im; delete image_im;
}

/******************HARRIS - LAPLACE DETECTOR**************/
void multi_harris_hessian_fast(DARY* image, vector<CornerDescriptor*>&corners, float har_threshold,  float hes_threshold){ 
  vector<CornerDescriptor *> cor_tmp;
  
  DARY* image_im = new DARY(image); 
  DARY* harris_im = new DARY(image->y(),image->x()); 
  int levels = 100;
  int sx,sy,nb;
  float scale, rescale, step=1.4;
  vector<float> scales;
  for( int i=1;i<levels;i++){ 
    scale=pow(step,i-1);
    scales.push_back(scale);
    harris_fast(image_im, harris_im);
    findMax(harris_im, cor_tmp, har_threshold, scale, /*type harris 1*/1);
    nb=cor_tmp.size();
    cout << "\rscale "<< scale <<  "       " <<flush;
    hessian_fast(image_im, harris_im);
    findMax(harris_im, cor_tmp, hes_threshold, scale, /*type harris 1*/2);
    rescale=pow(step,i-1);
    for(uint c=0;c<cor_tmp.size();c++){
      cor_tmp[c]->setX_Y(cor_tmp[c]->getX()*rescale,cor_tmp[c]->getY()*rescale);
      corners.push_back(cor_tmp[c]);
    }
    cor_tmp.clear();    
    smooth3(image_im, harris_im);
    sx=(int)(image_im->x()/step);
    sy=(int)(image_im->y()/step);
    if(sx<20 || sy<20)levels=i;
    delete image_im;
    image_im = new DARY(sy,sx);
    image_im->scale(harris_im,step,step);
    delete harris_im;
    harris_im = new DARY(sy,sx);
  }
  delete harris_im; delete image_im;
}

void getMif(DARY *img, Matrix *mi, int x, int y, float sigma,float derive_sigma){
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

void eigenf(Matrix M, float &l1, float &l2){
       float trace = M(1,1)+M(2,2);
       float delta = sqrt(trace*trace-4*(M(1,1)*M(2,2)-M(2,1)*M(1,2)));
		    
       l1 = (trace+delta)/2.0;
       l2 = (trace-delta)/2.0;
}

void normDf(Matrix &D){
  D.tabMat[2][2]=(1.0/sqrt(D.tabMat[2][2]));
  D.tabMat[1][1]=(1.0/sqrt(D.tabMat[1][1]));
  float c=sqrt(D.tabMat[2][2]*D.tabMat[1][1]);
  //c=D.tabMat[2][2];
  D.tabMat[2][2]/=c;
  D.tabMat[1][1]/=c;   
}


int findAffineRegion_fast(DARY *image_in, CornerDescriptor * cor){
  Matrix *mi=new Matrix(2,2,0.0);
  Matrix U(2,2,0.0),D(2,2,0.0),V(2,2,0.0),Ui(2,2,0.0),X(2,2,1.0),MInv(2,2);
  U.tabMat[1][1]=1;
  U.tabMat[2][2]=1;

  float pointx=cor->getX();
  float pointy=cor->getY();
  
  int l,go_on=1,sizex;
  float l1=1,l2=1;
  float eigen_ratio_act=0.1,eigen_ratio_bef=0.1,deigen=0.03;
  //static float nba=0; 
  float derive_sigma=cor->getCornerScale(), sigma, sc;
  sigma=derive_sigma*1.4;
  DARY *image,*img; 

  if(derive_sigma>=3){//sample the image for larger scales
    sc=3.0/derive_sigma;
    derive_sigma=3;
    sigma=derive_sigma*1.4;
    sizex=(int)(2*rint(1.5*GAUSS_CUTOFF*sigma)+1);
    img=new DARY(sizex,sizex);    
    image=new DARY(3*sizex,3*sizex);    
    image->interpolate(image_in, pointx, pointy, sc,  sc,  0);
    pointx=(3*sizex)>>1;
    pointy=(3*sizex)>>1;
  } else {
    sizex=(int)(2*rint(1.5*GAUSS_CUTOFF*sigma)+1);
    if(sizex<25)sizex=25;
    img=new DARY(sizex,sizex);
    image=image_in;
  }

  for(l=0;l<50 && go_on;l++){ //estimate affine structure
    img->interpolate(image,pointx,pointy,U(1,1),U(1,2),U(2,1),U(2,2));
    getMif(img, mi, sizex>>1, sizex>>1, sigma, derive_sigma);
    mi->svd(Ui,D,V);  
    normDf(D);  
    eigen_ratio_bef=eigen_ratio_act;
    eigen_ratio_act=1-D(1,1)/D(2,2);
    U=(V*D*V.transpose())*U;  
    eigenf(U,l1,l2);

    if(l>15 || (l2/l1>7)) {
	delete mi;
	delete img;
	return 0;
    }
    
    if(eigen_ratio_act<deigen && eigen_ratio_bef<deigen)go_on=0;       
  }
  delete mi;
  delete img;
  
  U.svd(Ui,D,V);
 
  if((D(1,1)/D(2,2))>7) { //reject long structures
    return 0;
  }
  U=V*D*V.transpose();
  cor->setMi(U(1,1),U(1,2),U(2,1),U(2,2));  
  return l;
}

void findAffineRegion_fast(DARY *image, vector<CornerDescriptor*> &cor){
  int nb=0;uint size=cor.size(),l;
  for(int i=0;i<(int)cor.size();i++){    
    cout <<"\rAffine point " << i << " of "<< size;
    cout <<" x:" << cor[i]->getX() <<  " y:" << cor[i]->getY();
    cout << "  scale:" << cor[i]->getCornerScale();
    cout << "  type: " << cor[i]->getType()<<"  ";
    l=findAffineRegion_fast(image,cor[i]);
    if(l!=0){	  
      cout << "  m11: " <<  cor[i]->getMi11();
      cout<<"  iterations: "<<l<<"     "<<flush;
      nb++;
    }else { 
      cout << "  rejected";
      delete cor[i];
      cor.erase((std::vector<CornerDescriptor*>::iterator)&cor[i]);i--;
    }
    cout << flush;
  }	

}


