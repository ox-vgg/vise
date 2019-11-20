#include "jetLocalAffine.h"

float Xf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LX[i][j];
    }
  }
  return sum;
}
float Yf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LX[j][i];
    }
  }
  return sum;
}
float XXf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXX[i][j];
    }
  }
  return sum;
}
float YYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXX[j][i];
    }
  }
  return sum;
}
float XYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXY[i][j];
    }
  }
  return sum;
}
float XXXf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXX[i][j];
    }
  }
  return sum;
}
float YYYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXX[j][i];
    }
  }
  return sum;
}
float XXYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXY[i][j];
    }
  }
  return sum;
}
float XYYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXY[j][i];
    }
  }
  return sum;
}
float XXXXf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXXX[i][j];
    }
  }
  return sum;
}
float YYYYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXXX[j][i];
    }
  }
  return sum;
}
float XXXYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXXY[i][j];
    }
  }
  return sum;
}
float XYYYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXXY[j][i];
    }
  }
  return sum;
}
float XXYYf(DARY *img){
  float sum=0;
  for (int i=0;i<PATCH_SIZE;i++){
    for (int j=0;j<PATCH_SIZE;j++){
      sum+=img->fel[i][j]*LXXYY[i][j];
    }
  }
  return sum;
}


void JetLocalAffine::computeComponents(DARY *img_in){
	if(img_in==NULL){return;}
	//int mins = (int)(GAUSS_CUTOFF*c_scale+1);
	//if(!isOK(mins,img_in->x()-mins,img_in->y()-mins))return;
	if(1+(gsize<<1)!=PATCH_SIZE){
	  cout <<"size error in jla " << endl;
	  return;
	} 
	allocVec(14);
	DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
        normalizeAffine(img_in,imgn);  
       	normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);
	vec[0]=Xf(imgn);
	vec[1]=Yf(imgn);

	vec[2]=XXf(imgn);
	vec[3]=XYf(imgn);
	vec[4]=YYf(imgn);

	vec[5]=XXXf(imgn);
	vec[6]=XXYf(imgn);
	vec[7]=XYYf(imgn);
	vec[8]=YYYf(imgn);
	
	vec[9]=XXXXf(imgn);
	vec[10]=XXXYf(imgn);
	vec[11]=XXYYf(imgn);
	vec[12]=XYYYf(imgn);
	vec[13]=YYYYf(imgn);
	state=1;
	delete imgn;


	int jla_pca_size=10;
	//pca(jla_pca_size,jla_pca_avg, jla_pca_base);	
	
} 
 
void computeJLADescriptors(DARY *image, vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    JetLocalAffine * ds=new JetLocalAffine();
    for(unsigned int c=0;c<desc.size();c++){
      cout << "\rjla descriptor "<< c<< " of "<< desc.size() <<"   "<< flush;
	ds->copy(desc[c]);	
	ds->computeComponents(image);
	ds->changeBase(jla_base);
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
