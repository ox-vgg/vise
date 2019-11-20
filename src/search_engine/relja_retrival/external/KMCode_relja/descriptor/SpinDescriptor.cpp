#include "spinDescriptor.h"

void SpinDescriptor::computeComponents(DARY *img_in){
        if(img_in==NULL){return;}
	
	DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
        normalizeAffine(img_in,imgn);  
       	normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);
	allocVec(SpinNbRings*SpinNbGrLev);


	
	//DARY *desc= new DARY(10,10,0.0);
	float dist,val,dd,dv;
	int vi,di;
	int rad = PATCH_SIZE>>1;
	for(int j=-rad;j<rad;j++){
	  for(int i=-rad;i<rad;i++){
	    if(patch_mask->fel[rad+j][rad+i]>0){
	      dist=((float)SpinNbRings)*sqrt((float)(i*i+j*j))/(float)rad;
	      val=((float)SpinNbGrLev)*(imgn->fel[rad+j][rad+i]/256.0);
	      di=(int)dist;
	      dd=dist-di;
	      vi=(int)val;
	      dv=val-vi;
	      
	      //	      if(di<0 || vi<0)continue;//cout << "dist "<< di << " val "<< vi<<"  " <<(1-dd)*(1-dv)  <<  endl;

	      if(di>=0 && di <SpinNbRings && vi>=0 && vi<SpinNbGrLev){ 
		//desc->fel[di][vi]+=(1-dd)*(1-dv);
		vec[SpinNbGrLev*di+vi]+=(1-dd)*(1-dv);
		if(vi<SpinNbGrLev-1){
		  //desc->fel[di][vi+1]+=(1-dd)*(dv);	
		  vec[SpinNbGrLev*di+vi+1]+=(1-dd)*(dv);
		}
		if(di<SpinNbRings-1){
		  //desc->fel[di+1][vi]+=(dd)*(1-dv);		
		  vec[SpinNbGrLev*(di+1)+vi]+=(dd)*(1-dv);
		  if(vi<SpinNbGrLev-1){
		    //desc->fel[di+1][vi+1]+=(dd)*(dv);
		    vec[SpinNbGrLev*(di+1)+vi+1]+=(dd)*(dv);
		  }
		} 
	      }
	    }
	  } 
	}
	float sum;
	for(int j=0;j<SpinNbRings;j++){
	  sum=0;
	  for(int i=0;i<SpinNbGrLev;i++){
	    //sum+=desc->fel[j][i];
	    sum+=vec[SpinNbGrLev*j+i];
	  }
	  for(int i=0;i<SpinNbGrLev;i++){
	    //desc->fel[j][i]=255.0*desc->fel[j][i]/sum;
	   vec[SpinNbGrLev*j+i]=(int)(512.0*vec[SpinNbGrLev*j+i]/sum);
	  }
	}
	
	//desc->write("desc.pgm");
	//imgn->write("imgn.pgm");
	//getchar();	delete desc;



	
	state=1;
	delete imgn;

	int spin_pca_size=50;
	//pca(spin_pca_size,spin_pca_avg,spin_pca_base);	


} 


void computeSpinDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    SpinDescriptor * ds = new  SpinDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rspin descriptor "<< c<< " of "<< desc.size() << "    " << flush;;
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
