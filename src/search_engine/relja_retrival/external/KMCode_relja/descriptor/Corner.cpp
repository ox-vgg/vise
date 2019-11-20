#include "corner.h"

/**********************************************/
Corner::Corner(void){    init();
} /**********************************************/
Corner::Corner(Corner *corner){
    init();
    type=corner->getType();
    x=corner->getX();
    y=corner->getY();
    cornerness= corner->getCornerness();
    c_scale=corner->getCornerScale();
    angle=corner->getAngle();
} 
/**********************************************/
Corner::Corner(float xin, float yin){
    init();
    x=xin;
    y=yin; 
} 
/**********************************************/
Corner::Corner(float xin, float yin, float cornerness_in){
    init();
    cornerness=cornerness_in;
    x=xin;
    y=yin; 
}
/**********************************************/
Corner::Corner(float xin, float yin, float cornerness_in, float scale_in){
    init();
    cornerness=cornerness_in;
    c_scale=scale_in;
    x=xin;
    y=yin; 
} 
/**********************************************/
Corner::Corner(float xin, float yin, float cornerness_in, float scale_in, float angle_in){
    init();
    cornerness=cornerness_in;
    c_scale=scale_in;
    x=xin;
    y=yin; 
    angle=angle_in;
}
Corner::Corner(float xin, float yin, float scale_in, float angle_in, int extr_in){
    init();
    c_scale=scale_in;
    x=xin;
    y=yin; 
    angle=angle_in;
    extr=extr_in;
}

/**********************************************/
Corner::Corner(float xin, float yin, float cornerness_in, float scale_in, float l1_in, float l2_in){
    init();
    cornerness=cornerness_in;
    c_scale=scale_in;
    x=xin;
    y=yin; 
    l1=l1_in;
    l2=l2_in;
}
/**********************************************/
Corner::Corner(float xin, float yin, int type_in){
    init();
    type=type_in;
    x=xin;
    y=yin; 
}


/**********************************************/
Corner::~Corner(){
}

/**********************************************/
int Corner::isOK(const int minim, const int max_x, const int max_y){
  //check if it's not too close to the border
  if((int)x > max_x || (int)x< minim || (int)y > max_y || (int)y< minim){
    state=0;
    return state;
  } else {state=1;return state;}
  
}

/**********************************************/
void Corner::computeAngleLowe(DARY *grad_im, DARY *angle_im){
    if(grad_im==NULL || angle_im==NULL){return;}
    int mins = (int)(GAUSS_CUTOFF*c_scale+2);
    if(!isOK(mins,grad_im->x()-mins,grad_im->y()-mins))return;
    
    float* hist= new float[73];
    for(int i=0;i<73;i++)hist[i]=0;
    float square_scale = c_scale * c_scale;
    float h_square_scale = (-2)*square_scale;
    for(int j=(int)y-mins;j<(int)y+mins;j++){
      for(int i=(int)x-mins;i<(int)x+mins;i++){
	  hist[(int)angle_im->fel[j][i]]+=(exp((((i-x)*(i-x)+(j-y)*(j-y))/(h_square_scale)))*grad_im->fel[j][i]);
      }
    }
    float max=0;
    int max_i=-1;
    for(int i=0;i<73;i++){
      if(hist[i]>max){ 
	max=hist[i];
	max_i=i;
      }
    }
    if(max_i>=0)angle=(M_2PI*max_i)/72.0;
    state=1;
    delete hist;
}
/**********************************************/
void Corner::computeAngleSimple(DARY *grad_im, DARY *angle_im){
    if(grad_im==NULL || angle_im==NULL){return;}
    int mins = (int)(GAUSS_CUTOFF*c_scale+2);
    if(!isOK(mins,grad_im->x()-mins,grad_im->y()-mins))return;
    angle=(M_2PI*angle_im->fel[(int)y][(int)x]/72.0);
}

/**********************************************/
void Corner::computeAngleMean(DARY *grad_im, DARY *angle_im){
    if(grad_im==NULL || angle_im==NULL){return;}
    int mins = (int)(GAUSS_CUTOFF*c_scale+2);
    if(!isOK(mins,grad_im->x()-mins,grad_im->y()-mins))return;
    
    float square_scale = c_scale * c_scale*2;
    float h_square_scale = (-2)*square_scale;
    angle=0;
    float nb=0,g;//,sumg=0;;
    int si=mins/2;
    //float norm=
    for(int j=(int)y-si;j<(int)y+si;j++){
      for(int i=(int)x-si;i<(int)x+si;i++){
	  g=(exp((((i-x)*(i-x)+(j-y)*(j-y))/(h_square_scale))));
	  angle+=angle_im->fel[j][i];
	  nb+=1;//sumg+=g;
	  
      }
    }
    angle/=((nb));
    state=1;
}

/**********************************************/
void Corner::computeAngleAtan2(DARY *img){
    if(img==NULL){return;}
    int mins = (int)(GAUSS_CUTOFF*c_scale+2);
    if(!isOK(mins,img->x()-mins,img->y()-mins))return;
    
    angle=atan2(dYf((int)x, (int)y,img, c_scale),dXf((int)x, (int)y, img, c_scale));
    state=1;
}

/**********************************************/
void Corner::computeAngle(DARY *angle_im){
    if(angle_im==NULL){return;}
    int mins = ((int)(GAUSS_CUTOFF*c_scale+2))>>1;
    if(!isOK(mins,angle_im->x()-mins,angle_im->y()-mins))return;
        
    float square_scale = c_scale * c_scale;
    float h_square_scale = (-2)*square_scale;
    float nb=0;
    int si=mins;
    float diff=0,sum=0;
    angle=angle_im->fel[(int)y][(int)x];
    for(int j=(int)y-si;j<(int)y+si;j++){
      for(int i=(int)x-si;i<(int)x+si;i++){
        diff=angle-angle_im->fel[j][i];
        if(diff>M_PI)diff-=M_2PI;
        else if(diff<(-M_PI))diff+=M_2PI;
        sum+=(exp((((i-x)*(i-x)+(j-y)*(j-y))/(h_square_scale))))*diff;
        nb+=1;
      }
    }
    //cout << "exp " << exp(((si*si+si*si)/(h_square_scale)))<< endl;
    angle-=(sum/nb);
    state=1;
}


void smoothHistogram(float *hist, int bins)
{
   int i;
   float prev, temp;
   
   prev = hist[bins - 1];
   for (i = 0; i < bins; i++) {
      temp = hist[i];
      hist[i] = (prev + hist[i] + hist[(i + 1 == bins) ? 0 : i + 1]) / 3.0;
      prev = temp;
   }
}
float interpPeak(float a, float b, float c)
{
    if (b < 0.0) {
      a = -a; b = -b; c = -c;
    }
    //assert(b >= a  &&  b >= c);
    return 0.5 * (a - c) / (a - 2.0 * b + c + 1.e-30);
}
void GradOriImages(DARY *im, DARY *grad, DARY *ori)
{
    float xgrad, ygrad, **pix;
    int rows, cols, r, c;
   
    rows = im->y();
    cols = im->x();
    pix = im->fel;

    for (r = 0; r < rows; r++)
      for (c = 0; c < cols; c++) {
        if (c == 0)
          xgrad = 2.0 * (pix[r][c+1] - pix[r][c]);
        else if (c == cols-1)
          xgrad = 2.0 * (pix[r][c] - pix[r][c-1]);
        else
          xgrad = pix[r][c+1] - pix[r][c-1];
        if (r == 0)
          ygrad = 2.0 * (pix[r+1][c] - pix[r][c]);
        else if (r == rows-1)
          ygrad = 2.0 * (pix[r][c] - pix[r-1][c]);
        else
          ygrad = pix[r+1][c] - pix[r-1][c];
	grad->fel[r][c] = sqrt(xgrad * xgrad + ygrad * ygrad);
	ori->fel[r][c] = atan2 (ygrad, xgrad);
      }
}
void LapOriImages(DARY *im, DARY *lap, DARY *ori)
{
    float xgrad, ygrad, **pix;
    int rows, cols, r, c;
   
    rows = im->y();
    cols = im->x();
    pix = im->fel;

    for (r = 0; r < rows; r++)
      for (c = 0; c < cols; c++) {
        if (c == 0 || c == cols-1)
	  xgrad=0;
	else if (c == 1)
          xgrad = pix[r][c+2] + pix[r][c-1] - 2.0 * pix[r][c];
        else if (c == cols-2)
          xgrad = pix[r][c+1] + pix[r][c-2] - 2.0 * pix[r][c];
        else
          xgrad = pix[r][c+2] + pix[r][c-2] - 2.0 * pix[r][c];
        if (r == 0 || r==rows-1)
	  ygrad =0;
	  else if (r == 1)
          ygrad = pix[r+2][c] + pix[r-1][c] - 2.0 * pix[r][c];
        else if (r == rows-2)
          ygrad = pix[r+1][c] + pix[r-2][c] - 2.0 * pix[r][c];
        else
          ygrad = pix[r-2][c] + pix[r+2][c] - 2.0 * pix[r][c];
	lap->fel[r][c] = fabs(xgrad + ygrad);
	ori->fel[r][c] = atan2 (ygrad, xgrad);
      }
}

inline float square(float a){
  return a*a;
}
void GradOriImages_s1(DARY *im, DARY *grad, DARY *ori)
{
  DARY *dx=new DARY(im->y(),im->x());
  DARY *dy=new DARY(im->y(),im->x());
  dX6(im,dx);
  dY6(im,dy);
  for(uint j=0;j<dx->y();j++){
    for(uint i=0;i<dx->x();i++){
      grad->fel[j][i]=sqrt(square(dx->fel[j][i])+square(dy->fel[j][i]));
      ori->fel[j][i]=atan2(dy->fel[j][i],dx->fel[j][i]);
    }      
  }
  delete dx;delete dy;
}


/**********************************************/
bool Corner::normalizeAffine( DARY *image, DARY *patch){
  bool ret = false;
   
  // int imb_size=2*(int)(1.414*GAUSS_CUTOFF*c_scale)+1;//1.414 margin for rotation, CUTOFF for min gaussian kernel size
  int imb_size=2*(int)(1.414*c_scale)+1;//1.414 margin for rotation
  //  float scal=(2*(int)(GAUSS_CUTOFF*c_scale)+1)/((float)patch->x());//scale for sampling without margin
  float scal=(2*(int)(c_scale)+1)/((float)patch->x());//scale for sampling without margin

  //angle=1+M_2PI;

  float xf=0,yf=0;
  float lecos=cos(-angle);
  float lesin=sin(-angle);

  if(angle>M_2PI){// if angle was not estimated yet
    lecos=1;lesin=0;     
  }
  //mi11=1;mi12=0;mi21=0;mi22=1;
  
  float m11=(mi11*lecos-mi12*lesin);
  float m12=(mi11*lesin+mi12*lecos);
  float m21=(mi21*lecos-mi22*lesin);
  float m22=(mi21*lesin+mi22*lecos); 
  
  DARY * imgbs=NULL; 
  if(scal>0.4){ // for larger scale  smooth before sampling
    //cout <<" x "<< x << " y "<< y << " cs "<< c_scale << " sc " << scal<< " a ";
    //cout << angle << " m " <<mi11<< " " << mi12 << " " << mi21 << " " << mi22 << endl;  
    DARY * imgb=new DARY(imb_size,imb_size);
    ret |= imgb->interpolate(image,x,y,m11,m12,m21,m22);//imgb->write("bsnorm.pgm");//normalize affine with scale=1
    imgbs=new DARY(imb_size,imb_size);
    xf = (float)(imb_size>>1);yf = (float)(imb_size>>1);
    smooth(imgb,imgbs,(1.5*scal));//imgbs->write("smnorm.pgm");//smooth 
    //cout << c_scale << " " << 1.5*scal << endl; 
    delete imgb;
    ret |= patch->interpolate(imgbs,xf,yf,1.0/scal,1.0/scal,0.0);//not rotated
    if(angle<M_2PI){
      delete imgbs;
    }
    
  }else { // if scale is small normalize without smoothing
    //  cout << "OK2 " << endl;
    m11*=scal;
    m12*=scal;
    m21*=scal;
    m22*=scal;
    ret |= patch->interpolate(image,x,y,m11,m12,m21,m22); //not rotated 
  }
  //cout << "OK3 " << endl;
  //patch->write("patch.pgm");cout << "wrote"<< endl;
  

  if(angle>M_2PI){     
    DARY * grad = new DARY(patch->x(),patch->x());   
    DARY * ori = new DARY(patch->x(),patch->x());   
    GradOriImages(patch, grad, ori);
    computeHistAngle(grad,ori);
    //cout << "angle " << angle << endl;
    delete grad;delete ori;
    if(scal>0.4){
      ret |= patch->interpolate(imgbs,xf,yf,1.0/scal,1.0/scal,-180*angle/M_PI); // normalization done
      delete imgbs;
    }else{      
      lecos=cos(-angle);
      lesin=sin(-angle);
      m11=scal*(mi11*lecos-mi12*lesin);
      m12=scal*(mi11*lesin+mi12*lecos);
      m21=scal*(mi21*lecos-mi22*lesin);
      m22=scal*(mi21*lesin+mi22*lecos);           
      ret |= patch->interpolate(image,x,y,m11,m12,m21,m22); // normalization done
    }
    
    //patch->write("norm.pgm");
    //if(scal>1.2){patch->write("norm.pgm");cout << "OK "<< endl;getchar(); } 
    //cout << "angle "<< angle << endl;getchar();
  }
  //patch->write("bpatch.pgm");cout << "OK " << endl;getchar();
  return ret;
} 
   /******************Lowe method*********************/
void Corner::computeHistAngle(DARY *img){

  
  int im_size=PATCH_SIZE; 
  DARY * imgn = new DARY(im_size,im_size);   
  DARY * grad = new DARY(im_size,im_size);   
  DARY * ori = new DARY(im_size,im_size);   
  
  normalizeAffine(img,imgn);
  GradOriImages(imgn, grad, ori);
  computeHistAngle(grad,ori);

  
  delete grad; delete ori; delete imgn;
}
/******************Lowe method*********************/
void Corner::computeHistAngle(DARY *grad,DARY *ori){
  if(ori==NULL ||grad==NULL ){return;}
  static const int OriBins = 36;
  int i, r, c, row, col, rows, cols, bin, prev, next;
  float hist[OriBins],  gval, langle,  interp, maxval = 0.0;/*OriBins=36*/
   
   row = grad->y()>>1;
   col = grad->x()>>1;
   rows = grad->y();
   cols = grad->x();
   
   for (i = 0; i < OriBins; i++)
      hist[i] = 0.0;
   
   for (r = 1; r < rows - 1; r++)
     for (c = 1; c <= cols - 1; c++){
       gval = grad->fel[r][c];   
       if (gval > 1.0  &&  patch_mask->fel[r][c]>0) {
	 /* Ori is in range of -PI to PI. */
	 langle = ori->fel[r][c];
	 bin = (int) (OriBins * (langle + M_PI + 0.001) / (2.0 * M_PI));
	 //assert(bin >= 0 && bin <= OriBins);
	 bin = (bin < OriBins - 1)?bin:(OriBins - 1);
	 hist[bin] +=  gval * patch_mask->fel[r][c];
       }
     }

   /* Apply smoothing 6 times for accurate Gaussian approximation. */
   for (i = 0; i < 6; i++)
     smoothHistogram(hist, OriBins);
   

   /* Find maximum value in histogram. */
   int maxi=-1;
   for (i = 0; i < OriBins; i++)
     if (hist[i] > maxval){
       maxval = hist[i];
       maxi=i;
     }
   prev = (maxi == 0 ? OriBins - 1 : maxi - 1);
   next = (maxi == OriBins - 1 ? 0 : maxi + 1);
   
   interp = interpPeak(hist[prev], hist[maxi], hist[next]);
   angle = (2.0 * M_PI * (maxi + 0.5 + interp) / OriBins - M_PI);
   
}

void normalize(DARY * img, int x, int y, float radius){
    float sum=0;
    float gsum=0; 

   for(uint j=0;j<img->y();j++){ 
	for(uint i=0;i<img->x();i++){ 
	  if(patch_mask->fel[j][i]>0){
	    sum+=img->fel[j][i]; 
	    gsum++;
	  }
        } 
    }    
    sum=sum/gsum;
    float var=0;
    for(uint j=0;j<img->y();j++){ 
      for(uint i=0;i<img->x();i++){ 
	if(patch_mask->fel[j][i]>0){	
	  var+=(sum-img->fel[j][i])*(sum-img->fel[j][i]);	
	}
      }
    }     
    var=sqrt(var/gsum);    

    //  cout << "mean "<<sum<< " " <<img->fel[y][x] << " var " << var << endl;
    float fac=50.0/var;
    float max=0,min=1000;
    for(uint j=0;j<img->y();j++){ 
      for(uint i=0;i<img->x();i++){ 
	img->fel[j][i]=128+fac*(img->fel[j][i]-sum);
	if(max<img->fel[j][i])max=img->fel[j][i];
	if(min>img->fel[j][i])min=img->fel[j][i];
	if(img->fel[j][i]>255)img->fel[j][i]=255;
	if(img->fel[j][i]<0)img->fel[j][i]=0;
      }
    }   
    // cout << "max " << max << " min "<< min <<endl;
}

void normalize1(DARY * img, int x, int y, float radius){
  float g;// = exp(((x*x)/(h_square_scale)));
  int rad=(int)ceil(0.5*radius);
  float h_square_scale=-0.25*rad*rad;
  float sum=0;
  float gsum=0;
  for(int j=-rad;j<=rad;j++){
	for(int i=-rad;i<=rad;i++){
	  g=exp(((i*i+j*j)/(h_square_scale)));
	  gsum+=g;
	  sum+=(img->fel[y+j][x+i])*g;
	} 
    }    
  
  sum/=gsum;
  
  rad=(int)ceil(0.8*radius);
  h_square_scale=rad*rad;
  float var=0;
  gsum=0;
  for(int j=-rad;j<=rad;j++){
	for(int i=-rad;i<=rad;i++){
	  g=((i*i+j*j)<(h_square_scale))?1:0;	    
	  gsum+=g;
	  //cout <<(sum-img->fel[y+j][x+i])<< endl;getchar();
	  var+=(sum-img->fel[y+j][x+i])*(sum-img->fel[y+j][x+i])*g;
	}
    } 
    
    var=sqrt(var/gsum);  
    //cout << "mean "<<sum<< " " <<img->fel[y][x] << " " << var << endl;

    float fac=100.0/var;
    rad=(int) radius; 
    for(int j=-rad;j<=rad;j++){
	for(int i=-rad;i<=rad;i++){
	    img->fel[y+j][x+i]=128+fac*(img->fel[y+j][x+i]-sum);
	}
    }   


 
} 
 
DARY *patch_mask = new DARY(PATCH_SIZE,PATCH_SIZE);
//float PATCH_SUM;
void initPatchMask(int size){ 
  //DARY * mask = new DARY(PATCH_SIZE,PATCH_SIZE,0.0);    
   //patch_mask;    
    int center=size>>1;
    float radius = center*center;
    float sigma=0.9*radius;
    float disq;
    //PATCH_SUM=0;
    for(int i=0;i<size;i++)
	for(int j=0;j<size;j++){
	  disq=(i-center)*(i-center)+(j-center)*(j-center);
	    if(disq < radius){
	      patch_mask->fel[j][i]= exp(- disq / sigma);
	      //mask->fel[j][i]= 255*exp(- disq / sigma);   
	      //cout << patch_mask->fel[j][i]<< endl; 
	      //PATCH_SUM+=patch_mask->fel[j][i];
	    }else { 
	      patch_mask->fel[j][i]=0;
	    }		
	} 

    //patch_mask->normalize(0,1);patch_mask->write("mask.pgm");cout << "mask "<< endl;getchar();
} 

