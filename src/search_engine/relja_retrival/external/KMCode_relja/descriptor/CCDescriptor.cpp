#include "cCDescriptor.h"

void CCDescriptor::computeComponents(DARY *img_in){
        if(img_in==NULL){return;}
	//int mins = (int)(GAUSS_CUTOFF*c_scale+1);
	//if(!isOK(mins,img_in->x()-mins,img_in->y()-mins))return;
	
	DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
	DARY * simgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
	normalizeAffine(img_in,imgn);  
	allocVec(CCSize*CCSize);
	smoothSqrt(imgn,simgn);
	normalize(simgn,simgn->x()>>1,simgn->y()>>1,simgn->x()>>1);
	//imgn->write("imgn.pgm");
	//simgn->write("simgn.pgm");cout << " OK " << endl;getchar();
	int step = PATCH_SIZE/CCSize;
	for(int j=0,yi=step;j<CCSize;j++,yi+=step){
	  for(int i=0,xi=step;i<CCSize;i++,xi+=step){
	    vec[j*CCSize+i]=simgn->fel[yi][xi];
	  }
	}

	float sum=0,val;
	for(int j=0;j<size;j++){
	  val=vec[j];
	  sum+=val*val;
	}
	
	float fac=1.0/sqrt(sum);
	for(int j=0;j<size;j++){
	  vec[j]*=fac;
	}
	
	

	/* Convert float vector to integer. Assume largest value in normalized
	   vector is likely to be less than 0.5. */
	int intval;
	for (int i = 0; i < size; i++) {
	  intval = (int) (512.0 * vec[i]);
	  vec[i] = (255 < intval) ? 255 : intval;
	  //cout << "vec " << vec[i]<<endl;
	}
	state=1;
	delete imgn;
	delete simgn;
	int cc_pca_size=81;
	//pca(cc_pca_size,cc_pca_avg,cc_pca_base);	

} 


void computeCCDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    CCDescriptor * ds = new  CCDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rcc descriptor "<< c<< " of "<< desc.size() << "    " << flush;;
	ds->copy(desc[c]);
	ds->computeComponents(image);
 	desc[c]->copy((CornerDescriptor*)ds); 
    }
    for(unsigned int c=0;c<desc.size();c++){
	if(!desc[c]->isOK()){
	    //desc.erase((std::vector<CornerDescriptor*>::iterator)&desc[c]);
		desc.erase(desc.begin() + c);
	    c--;
	}
    }
    cout<<endl;
}
