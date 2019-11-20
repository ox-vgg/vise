#include "csiftDescriptor.h"

/* David Lowe's code*/

/* Increment the appropriate locations in the index to incorporate
   this image sample.  The location of the sample in the index is (rx,cx). 
*/
void CSiftDescriptor::PlaceInIndex(float *index,
		  float mag, float ori, float frad, float fangle)
{
   int r, c, ort, ri, ci, oi, rindex, cindex, oindex, roindex;
   float oval, rfrac, cfrac, ofrac, rweight, cweight, oweight;
   
   oval = COriSize * ori / (M_2PI);
   
   ri = (int)frad;  /* Round down to next integer. */
   rfrac = frad - ri;         /* Fractional part of location. */
   oi = (oval >= 0.0) ? oval : oval - 1.0;
   ofrac = oval - oi;     
   ci = (int)(fangle);
   cfrac = fangle - ci;
    
   //cout <<  " rad "<< frad << " ri "<< ri << " rfrac " << rfrac << " angle "<< fangle << " ci "<< ci << " cfrac " << cfrac << endl;getchar();



   /*   assert(ri >= -1  &&  ri < CIndexSize  &&  oi >= 0  &&  oi <= COriSize  && rfrac >= 0.0  &&  rfrac <= 1.0);*/
   
   /* Put appropriate fraction in each of 8 buckets around this point
      in the (row,col,ori) dimensions.  This loop is written for
      efficiency, as it is the inner loop of key sampling. */
   
   for (r = 0; r < 2; r++) {
     rindex = ri + r;
     if (rindex < CRadIndexSize) {
       rweight = mag * ((r == 0) ? 1.0 - rfrac : rfrac);       
       for (ort = 0; ort < 2; ort++) {
	 oindex = oi + ort;
	 if (oindex >= COriSize)   //Orientation wraps around at PI. 
	   oindex = 0;
	 oweight = rweight * ((ort == 0) ? 1.0 - ofrac : ofrac);                  
	 roindex=(rindex*COriSize+oindex)*CAngIndexSize;
	 for (c = 0; c < 2; c++) {
	   cindex = ci + c;
	   if (cindex >= CAngIndexSize || rindex ==0) 
	     cindex=0;
	   cweight = oweight * ((c == 0) ? 1.0 - cfrac : cfrac);
	   index[roindex+cindex]+=cweight;	  
	   //cout << " ri "<<rindex << " rfrac "<< rfrac << " oi "<< oindex << " ofrac "<<ofrac<< " cindex " << cindex << " cfrac " << cfrac<< endl;getchar();
	 }  
       }
     }          
   }
}
 
  
/* Given a sample from the image gradient, place it in the index array.
*/
void CSiftDescriptor::AddSample(float *index,
	       DARY *grad, DARY *orim, int r, int c, float frad, float fangle,
	       float rx, float cx)
{
    float mag, ori, sigma, weight;
    
    /* Clip at image boundaries. */
    if (r < 0  ||  r >= grad->y()  ||  c < 0  ||  c >= grad->x())
       return;
    
    /* Compute Gaussian weight for sample, as function of radial distance
       from center.  Sigma is relative to half-width of index. */
    //sigma =  0.5*(CIndexSize+1)*(CIndexSize+1);
    //sigma = (CIndexSize+1)*(CIndexSize+1);
    //weight = 0.6*exp(- (rpos * rpos + cpos * cpos) / (sigma) );
    //cout << "rpos "<< rpos << " cpos "<< cpos << " weight " << weight<< endl;
    //mag = weight * grad->fel[r][c];
    mag = patch_mask->fel[r][c] * grad->fel[r][c];
    /* Subtract keypoint orientation to give ori relative to keypoint. */
    ori = orim->fel[r][c];
    
    /* Put orientation in range [0, 2*PI].  If sign of gradient is to
       be ignored, then put in range [0, PI]. */
    
    while (ori > M_2PI)
      ori -= M_2PI;
    while (ori < 0.0)
      ori += M_2PI;    
    PlaceInIndex(index, mag, ori, frad, fangle);
} 

/* Add features to vec obtained from sampling the grad and ori images
   for a particular scale.  Location of key is (scale,row,col) with respect
   to images at this scale.  We examine each pixel within a circular
   region containing the keypoint, and distribute the gradient for that
   pixel into the appropriate bins of the index array.
*/

void CSiftDescriptor::KeySample(float *index, DARY *grad, DARY *ori)
{
   int i, j, iradius, irow, icol,irad, iangle;
   float spacing, sine, cosine, rpos, cpos, rx, cx, frad, fangle;
   
   /* Radius of index sample region must extend to diagonal corner of
      index patch plus half sample for interpolation. */
   
   iradius = grad->x()>>1;
   float radspacing = CRadIndexSize / (1.0*iradius);
   float angspacing = CAngIndexSize / (1.0*M_2PI);
   //   printf("spacing %f, scale %f, radius %d\n",spacing,scale, iradius);
   // Examine all points from the gradient image that could lie within the index square. 
   //iradius =5;
   for (i = -iradius; i <= iradius; i++)
     for (j = -iradius; j <= iradius; j++) {
       
        
       frad=radspacing*sqrt((float)(i*i+j*j));
       if(frad>=CRadIndexSize)continue;
       fangle=angspacing*(M_PI+atan2((float)i,(float)j));
       
       // Test whether this sample falls within boundary of index patch. 
       //if (frad >=0 && frad < (float)CRadIndexSize  &&
       //	   fangle > 0 && fangle < (float)CAngIndexSize )
         //cout << "in" << cpos << " " << rpos << endl;
	 AddSample(index, grad, ori, iradius + i, iradius + j, frad, fangle,
		   rx, cx);
   } 

}

 
/* Normalize length of vec to 1.0.
*/
void NormalizeCVect(float *vec, int len) 
{
   int i,roindex,oindex;
   float val, fac, sqlen = 0.0;

   sqlen = 0.0; 
   for(int r=0;r<CRadIndexSize;r++){
     roindex=r*COriSize;
     for(int o=0;o<COriSize;o++){ 
       oindex=(roindex+o)*CAngIndexSize;
       for(int c=0;c<CAngIndexSize;c++){
	 val=vec[oindex+c];
	 sqlen+=val*val;
       }
     }
     /*     fac=1.0 / (sqrt(sqlen));
       for(int o=0;o<COriSize;o++){
       oindex=(roindex+o)*CAngIndexSize;
       for(int c=0;c<CAngIndexSize;c++){
	 vec[oindex+c]*=fac;
       }
       }
     */
     
   }
   fac=1.0 / (sqrt(sqlen));
   for(int r=0;r<CRadIndexSize;r++){
     roindex=r*COriSize;
     for(int o=0;o<COriSize;o++){
       oindex=(roindex+o)*CAngIndexSize;
       for(int c=0;c<CAngIndexSize;c++){
	 vec[oindex+c]*=fac;
       }
     }
   }
   
}
 
void CSiftDescriptor::MakeKeypointSample(DARY *grad, DARY *ori)
{
   int i, intval, changed = FALSE;
  
   /* Produce sample vector. */
   KeySample(vec, grad, ori);
   
   /* Normalize vector.  This should provide illumination invariance
      for planar lambertian surfaces (except for saturation effects).
      Normalization also improves nearest-neighbor metric by
      increasing relative distance for vectors with few features.
      It is also useful to implement a distance threshold and to
      allow conversion to integer format.
   */

   NormalizeCVect(vec, size); 

   /* Now that normalization has been done, threshold elements of
      index vector to decrease emphasis on large gradient magnitudes.
      Admittedly, the large magnitude values will have affected the
      normalization, and therefore the threshold, so this is of
      limited value.
   */
   for (i = 0; i < size; i++)
     if (vec[i] > CMaxIndexVal) { 
       vec[i] = CMaxIndexVal;
       changed = TRUE;
     }
   if (changed)
     NormalizeCVect(vec, size); 

   /* Convert float vector to integer. Assume largest value in normalized
      vector is likely to be less than 0.5. */
   for (i = 0; i < size; i++) {
     intval = (int) (512.0 * vec[i]);
     vec[i] = (255 < intval) ? 255 : intval;
     //cout << "vec " << vec[i]<<endl;
   }
} 


void CSiftDescriptor::computeComponents(DARY *img){
  if(img==NULL){return;}
  int mins = (int)(GAUSS_CUTOFF*c_scale+2);
  if(!isOK(mins,img->x()-mins,img->y()-mins))return;
 
  size=CSiftSize;//CIndexSize*CIndexSize*CCOriSize;
  allocVec(size);
  
  DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE,"float");   
  DARY * grad = new DARY(PATCH_SIZE,PATCH_SIZE,"float");
  DARY * ori = new DARY(PATCH_SIZE,PATCH_SIZE,"float");    
  
  c_scale=2*c_scale;
  normalizeAffine(img,imgn);  
  c_scale=c_scale/2;

  normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);
  //imgn->write("imgn.pgm");cout << "wrote"<< endl;getchar();
  GradOriImages(imgn,grad,ori);
  MakeKeypointSample(grad, ori);
  delete imgn;delete grad;delete ori;
  state=1;
} 


void computeCSiftDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    CSiftDescriptor * ds = new  CSiftDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rsift descriptor "<< c<< " of "<< desc.size() << "    " << flush;;
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
    return;
    double *varvec=new double[128]; 
    double *meanvec=new double[128];
    for(uint v=0;v<128;v++){
      meanvec[v]=0;
      varvec[v]=0;
    }
    for(unsigned int c=0;c<desc.size();c++){	
      for(uint v=0;v<128;v++){
	meanvec[v]+=desc[c]->getV(v);
      }
    }
    for(uint v=0;v<128;v++){
      meanvec[v]/=((double)desc.size());
    }
    for(unsigned int c=0;c<desc.size();c++){	
      for(uint v=0;v<128;v++){
	varvec[v]+=((meanvec[v]-desc[c]->getV(v))*(meanvec[v]-desc[c]->getV(v)));
      }
    }
    for(uint v=0;v<128;v++){
      varvec[v]=sqrt(varvec[v]/((double)desc.size()));
      cout << meanvec[v]<< " "<< varvec[v]<<endl;
    }

    
}
