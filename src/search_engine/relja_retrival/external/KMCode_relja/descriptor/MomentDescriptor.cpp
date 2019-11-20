#include "momentDescriptor.h"
float computMoment000(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment++;
      }
    }
  }
  return moment;
}

float computMoment001(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment002(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}

float computMoment101(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment011(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}

float computMoment111(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment102(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment012(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=j*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment201(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*i*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment021(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=j*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment121(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*j*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment211(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*i*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment301(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*i*i*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}

float computMoment031(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=j*j*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}

float computMoment112(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*j*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment202(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=i*i*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
float computMoment022(DARY *img, int rad){
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(patch_mask>0){
	moment+=j*j*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}


float MomentDescriptor::computMoment(DARY *img, float p, float q, float a){
  
  int rad=img->x()>>1;
  int rad2=rad*rad;
  float moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=pow(i,p)*pow(j,q)*pow(img->fel[rad+i][rad+j],a);
      }
    }
  }
  return moment;
}



void MomentDescriptor::computeComponents(DARY *img){
	if(img==NULL){return;} 
	//int mins = (int)(GAUSS_CUTOFF*c_scale+2);
	//if(!isOK(mins,img->x()-mins,img->y()-mins))return;
	allocVec(20);

	DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
	DARY * dx = new DARY(PATCH_SIZE,PATCH_SIZE);   
	DARY * dy = new DARY(PATCH_SIZE,PATCH_SIZE);   
        normalizeAffine(img,imgn);  
       	normalize(imgn,(int)imgn->x()>>1,(int)imgn->x()>>1,imgn->x()>>1);
	dX2(imgn,dx);
	dY2(imgn,dy);
	delete imgn;
	//imgn->write("points.pgm");getchar();

	int rad=dx->x()>>1;
	float norm=computMoment000(dx,rad);
	//vec[0]=computMoment001(dx,rad)/norm;
	//float mean=computMoment002(grad,rad);

	vec[0]=computMoment101(dx,rad)/norm;
	vec[1]=computMoment011(dx,rad)/norm;
 	vec[2]=computMoment102(dx,rad)/norm;
	vec[3]=computMoment012(dx,rad)/norm;
      
 	vec[4]=computMoment111(dx,rad)/norm;
	vec[5]=computMoment201(dx,rad)/norm;
	vec[6]=computMoment021(dx,rad)/norm;
	vec[7]=computMoment112(dx,rad)/norm;
	vec[8]=computMoment202(dx,rad)/norm;
	vec[9]=computMoment022(dx,rad)/norm;

	//norm=computMoment000(dy,rad);
	vec[10]=computMoment101(dy,rad)/norm;
	vec[11]=computMoment011(dy,rad)/norm;
 	vec[12]=computMoment102(dy,rad)/norm;
	vec[13]=computMoment012(dy,rad)/norm;
      
 	vec[14]=computMoment111(dy,rad)/norm;
	vec[15]=computMoment201(dy,rad)/norm;
	vec[16]=computMoment021(dy,rad)/norm;
	vec[17]=computMoment112(dy,rad)/norm;
	vec[18]=computMoment202(dy,rad)/norm;
	vec[19]=computMoment022(dy,rad)/norm;
	//vec[0]=0;
	state=1;
	delete dx;delete dy;
	changeBase(mom_base);
	int mom_pca_size=20;
	//pca(mom_pca_size, mom_pca_avg, mom_pca_base);	

} 

void computeMomentDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    MomentDescriptor * ds=new MomentDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rmom descriptor "<< c<< " of "<< desc.size() << "    " << flush;;
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
    cout<<endl;
}
