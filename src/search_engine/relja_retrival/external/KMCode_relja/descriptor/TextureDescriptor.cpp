#include "textureDescriptor.h"

/**********************************************/
void TextureDescriptor::init( const char *dirmodel){
    wave = new Wavelet(dirmodel);    
    thres=new double[5];	
    thres[0]=0;thres[1]=0;thres[2]=35;thres[3]=45;thres[4]=55;
}

/**********************************************/
void TextureDescriptor::computeComponents(DARY *image, Zone *zone){
    allocVec(45);
    //  double nb=0;
    wave->wt(image,4);
    DARY *qnimage= new DARY(image->y(),image->x(),0.0);
    wave->quantize(image,qnimage,thres,2,/*levels*/ 4);    
    //qnimage->write("texture.pgm");
    DARY *codes= new DARY(image->y(),image->x(),0.0);
    computeCodes(qnimage,codes);
    delete qnimage;delete codes;
    //codes->write("codes.pgm");
}

double TextureDescriptor::getCode(int a, int b, int c, int d){
  
  int sum_m=0,sum_p=0,sum_o=0;
  
  if(a==1)sum_o++;
  else if(a==0)sum_m++;
  else if(a==2)sum_p++;
        
  if(b==1)sum_o++;
  else if(b==0)sum_m++;
  else if(b==2)sum_p++; 
       
  if(c==1)sum_o++;
  else if(c==0)sum_m++;
  else if(c==2)sum_p++;   
     
  if(d==1)sum_o++;
  else if(d==0)sum_m++;
  else if(d==2)sum_p++;       

  if((sum_o+sum_m+sum_p)!=4)cout << "wrong "<<sum_o<<" " << sum_m<< " "<< sum_p<< endl;
  
  if(sum_m==4)return 1; 
  else if(sum_m==3&&sum_o==1)return 2; 
  else if(sum_m==3&&sum_p==1)return 3; 
  else if(sum_m==2&&sum_o==2)return 4; 
  else if(sum_m==2&&sum_p==2)return 5; 
  else if(sum_m==1&&sum_o==3)return 6; 
  else if(sum_m==1&&sum_p==3)return 7; 

  else if(sum_o==4)return 0; 
  else if(sum_o==3&&sum_p==1)return 8; 
  else if(sum_o==2&&sum_p==2)return 9; 
  else if(sum_o==1&&sum_p==3)return 10; 

  else if(sum_p==4)return 11; 

  else if(sum_o==2&&sum_m==1&&sum_p==1)return 12; 
  else if(sum_o==1&&sum_m==2&&sum_p==1)return 13; 
  else if(sum_o==1&&sum_m==1&&sum_p==2)return 14;
  else {cout << "wrong "<<a<<" " << b<< " "<< c<< " d" <<endl;return 0;}
  
}

/**********************************************/
void TextureDescriptor::computeCodesDistribution(DARY *codes,int sxs, int sys, int sxe, int sye, 
						 int offset){
  int code=0;
  for(int y=sys;y<sye;y++){
    for(int x=sxs;x<sxe;x++){
      code=(int)codes->fel[y][x];
	if(code>15 || code<0)code=0;
	if(code!=0)vec[offset+code]++;	       
    }
  }        
}

/**********************************************/
void TextureDescriptor::computeCodes(DARY *image,DARY *codes){
    int sxe=image->x()>>1;
    int sye=image->y()>>1;
    for(int y=0;y<sye;y++){
      for(int x=0;x<sxe;x++){
	codes->fel[y][x]=getCode((int)image->fel[y][x],(int)image->fel[y][x+1],
				(int)image->fel[y+1][x],(int)image->fel[y+1][x+1]);	       
      }
    }    
    int sxs=image->x()>>2; 
    int sys=image->y()>>2;
    computeCodesDistribution(codes,0,sys,sxe,sye,30);
    computeCodesDistribution(codes,sxs,0,sxe,sys,30); 
    double normal=0.333/((sxe-sxs)*(sye-sys));
    for(int i=30;i<size;i++)vec[i]*=normal;

    sxs>>=1;sxe>>=1;sys>>=1;sye>>=1;	
    computeCodesDistribution(codes,0,sys,sxe,sye,15);
    computeCodesDistribution(codes,sxs,0,sxe,sys,15); 
    normal=0.333/((sxe-sxs)*(sye-sys));
    for(int i=15;i<30;i++)vec[i]*=normal;
    
    sxs>>=1;sxe>>=1;sys>>=1;sye>>=1;	
    computeCodesDistribution(codes,0,sys,sxe,sye,0);
    computeCodesDistribution(codes,sxs,0,sxe,sys,0); 
    normal=0.333/((sxe-sxs)*(sye-sys));
    for(int i=0;i<15;i++)vec[i]*=normal;
}

TextureDescriptor::~TextureDescriptor(){
  if(vec!=NULL)delete[] vec;
  delete [] thres;
  delete wave;
}


/**********************************************/
double distance(TextureDescriptor *desc1,TextureDescriptor *desc2){
  int size=desc1->getSize();
  double dist=0;//,diff,sum;
  for(int i=0;i<size;i++){	
    // diff=(desc1->getV(i)-desc2->getV(i));
    // sum=(desc1->getV(i)+desc2->getV(i));
    // if(sum)dist+=(diff*diff)/(sum*sum);
      dist+=fabs(desc1->getV(i)-desc2->getV(i));
    }    
    return dist;
}
