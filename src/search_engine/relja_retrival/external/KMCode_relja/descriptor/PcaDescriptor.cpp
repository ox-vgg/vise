#include "pcaDescriptor.h"

/* Yan Ke code*/


void Normvec(float * v, unsigned int len) {
	float total = 0;
	

	for (unsigned int i = 0; i < len; i++) {
		total += fabs(v[i]);
	}
	

	total /= len;

	
	for (unsigned int i = 0; i < len; i++) {
		v[i] = v[i] / total * 100.0;
	}	
}

  
void PcaDescriptor::computeComponents(DARY *img){
  if(img==NULL){return;}
 
  allocVec(PCALEN);
  
  DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
  
  normalizeAffine(img,imgn);  
  normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);
  //imgn->write("imgn.pgm");cout << "wrote"<< endl;getchar();
  float *tvec = new float[GPLEN];
  uint count=0;
  for(int j=1;j<PATCH_SIZE-1;j++){
    for(int i=1;i<PATCH_SIZE-1;i++){      
      tvec[count++]=imgn->fel[j][i+1]-imgn->fel[j][i-1];
      tvec[count++]=imgn->fel[j+1][i]-imgn->fel[j-1][i];
    }
  }

  Normvec(tvec, GPLEN);

	
  for (uint i = 0; i < GPLEN; i++) {
    tvec[i] -= avgs[i];
  }
  for (int ldi = 0; ldi < PCALEN; ldi++) {
    for (int x = 0; x < GPLEN; x++)
      vec[ldi] += eigs[ldi][x] * tvec[x];
  }
  delete imgn;;delete tvec;
  state=1;
} 

   
void computePcaDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    PcaDescriptor * ds = new  PcaDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rpca descriptor "<< c<< " of "<< desc.size() << "    " << flush;
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
