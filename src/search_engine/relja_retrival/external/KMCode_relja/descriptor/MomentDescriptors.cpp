#include "momentDescriptor.h"
double computMoment000(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment++;
      }
    }
  }
  return moment;
}

double computMoment001(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment002(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}

double computMoment101(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment011(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}

double computMoment111(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment102(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment012(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=j*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment201(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*i*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment021(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=j*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment121(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*j*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment211(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*i*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment301(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*i*i*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}

double computMoment031(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=j*j*j*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}

double computMoment112(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*j*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment202(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=i*i*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}
double computMoment022(DARY *img, int rad, int rad2){
  double moment=0;
  for(int i=-rad;i<=rad;i++){
    for(int j=-rad;j<=rad;j++){
      if(i*i+j*j<rad2){
	moment+=j*j*img->fel[rad+i][rad+j]*img->fel[rad+i][rad+j];
      }
    }
  }
  return moment;
}


double MomentDescriptor::computMoment(DARY *img, double p, double q, double a){
  
  int rad=img->x()>>1;
  int rad2=rad*rad;
  double moment=0;
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
	int mins = (int)(GAUSS_CUTOFF*c_scale+2);
	if(!isOK(mins,img->x()-mins,img->y()-mins))return;
	allocVec(15);
	d_scale=c_scale;
	double scmean=7;
        int im_size=2*(int)rint(GAUSS_CUTOFF*scmean)+3;
        DARY * imgn = new DARY(im_size,im_size); 
	c_scale=2*c_scale;     
       	normalizeAffine(img,imgn);   
	c_scale=d_scale;
       	normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);

	//imgn->write("points.pgm");getchar();

	int rad=imgn->x()>>1;
	int rad2=(rad-1)*(rad-1);
	double norm=computMoment000(imgn,rad,rad2);
	vec[0]=computMoment001(imgn,rad,rad2)/norm;
	double mean=computMoment002(imgn,rad,rad2);

	vec[1]=computMoment101(imgn,rad,rad2)/vec[0];
	vec[2]=computMoment011(imgn,rad,rad2)/vec[0];
 	vec[3]=computMoment102(imgn,rad,rad2)/mean;
	vec[4]=computMoment012(imgn,rad,rad2)/mean;
      
 	vec[5]=computMoment111(imgn,rad,rad2)/vec[0];
	vec[6]=computMoment201(imgn,rad,rad2)/vec[0];
	vec[7]=computMoment021(imgn,rad,rad2)/vec[0];
	vec[8]=computMoment112(imgn,rad,rad2)/mean;
	vec[9]=computMoment202(imgn,rad,rad2)/mean;
	vec[10]=computMoment022(imgn,rad,rad2)/mean;
	state=1;
	delete imgn;
} 

void computeMomentDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    MomentDescriptor * ds=new MomentDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rdescriptor mom "<< c<< " of "<< desc.size() << "    " << flush;;
	ds->copy(desc[c]);
	ds->computeComponents(image);
	ds->changeBase(ds->getSize(),baseMomDog);
 	desc[c]->copy((CornerDescriptor*)ds); 
    }
    for(unsigned int c=0;c<desc.size();c++){
	if(!desc[c]->isOK()){
	    desc.erase(&desc[c]);
	    c--;
	}
    }
    cout<<endl;
}
