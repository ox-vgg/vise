#include "esiftDescriptor.h"

/* David Lowe's code*/

/* Increment the appropriate locations in the index to incorporate
   this image sample.  The location of the sample in the index is (rx,cx).
*/
void ESiftDescriptor::PlaceInIndex(float *index, 
		  float mag, float ori, float rx, float cx, int eindexSize)
{
   int r, c, ort, ri, ci, oi, rindex, cindex, oindex, rcindex;
   float oval, rfrac, cfrac, ofrac, rweight, cweight, oweight;
   
   oval = EOriSize * ori / (M_2PI);//CHANGED from M_2PI 
   
   ri = (int)((rx >= 0.0) ? rx : rx - 1.0);  /* Round down to next integer. */
   ci = (int)((cx >= 0.0) ? cx : cx - 1.0);
   oi = (int)((oval >= 0.0) ? oval : oval - 1.0);
   rfrac = rx - ri;         /* Fractional part of location. */
   cfrac = cx - ci;
   ofrac = oval - oi;
   /*   assert(ri >= -1  &&  ri < IndexSize  &&  oi >= 0  &&  oi <= OriSize  && rfrac >= 0.0  &&  rfrac <= 1.0);*/
   //if(oval<0 || oval>4 || ofrac>1.0 )cout << "error : "<< oval<<" "<< ofrac <<endl;
   /* Put appropriate fraction in each of 8 buckets around this point
      in the (row,col,ori) dimensions.  This loop is written for
      efficiency, as it is the inner loop of key sampling. */
   for (r = 0; r < 2; r++) {
      rindex = ri + r;
      if (rindex >=0 && rindex < eindexSize) {
         rweight = mag * ((r == 0) ? 1.0 - rfrac : rfrac);
         
         for (c = 0; c < 2; c++) {
            cindex = ci + c;
            if (cindex >=0 && cindex < eindexSize) {
               cweight = rweight * ((c == 0) ? 1.0 - cfrac : cfrac);
               rcindex=(rindex*eindexSize+cindex)<<3;//ATTANTION SHOULD BE 3 for 8 orientations
               for (ort = 0; ort < 2; ort++) {
                  oindex = oi + ort;
                  if (oindex >= EOriSize)  /* Orientation wraps around at PI. */
                     oindex = 0;
                  oweight = cweight * ((ort == 0) ? 1.0 - ofrac : ofrac);
                  
		  index[rcindex+oindex]+=oweight;
               }
            }  
         }
      }
   } 
}
 
 
/* Given a sample from the image gradient, place it in the index array.
*/
void ESiftDescriptor::AddSample(float *index,
	       DARY *grad, DARY *orim, int r, int c, float rpos, float cpos,
	       float rx, float cx, int eindexSize)
{
    float mag, ori;
    
    /* Clip at image boundaries. */
    if (r < 0  ||  r >= grad->y()  ||  c < 0  ||  c >= grad->x())
       return;
    
    /* Compute Gaussian weight for sample, as function of radial distance
       from center.  Sigma is relative to half-width of index. */
    //sigma =  0.5*(IndexSize+1)*(IndexSize+1);
    //sigma = (IndexSize+1)*(IndexSize+1);
    //weight = 0.6*exp(- (rpos * rpos + cpos * cpos) / (sigma) );
    //cout << "rpos "<< rpos << " cpos "<< cpos << " weight " << weight<< endl;
    //mag = weight * grad->fel[r][c];
    mag = patch_mask->fel[r][c] * grad->fel[r][c];
    /* Subtract keypoint orientation to give ori relative to keypoint. */
    ori = orim->fel[r][c];
    
    /* Put orientation in range [0, 2*PI].  If sign of gradient is to
       be ignored, then put in range [0, PI]. */
    
    if (ori > M_2PI)
      ori -= M_2PI;
    if (ori < 0.0)
      ori += M_2PI;//CHANGED from M_2PI    
    PlaceInIndex(index, mag, ori, rx, cx, eindexSize);
} 

/* Add features to vec obtained from sampling the grad and ori images
   for a particular scale.  Location of key is (scale,row,col) with respect
   to images at this scale.  We examine each pixel within a circular
   region containing the keypoint, and distribute the gradient for that
   pixel into the appropriate bins of the index array.
*/

void ESiftDescriptor::KeySample(float *index,  
	       DARY *grad, DARY *ori, int eindexSize)
{
   int i, j, iradius;
   float spacing, rpos, cpos, rx, cx;
   
   /* Radius of index sample region must extend to diagonal corner of
      index patch plus half sample for interpolation. */
   
   iradius = grad->x()>>1;
   spacing = (eindexSize + 1) / (2.0*iradius);
   //   printf("spacing %f, scale %f, radius %d\n",spacing,scale, iradius);
   // Examine all points from the gradient image that could lie within the index square. 
   for (i = -iradius; i <= iradius; i++)
     for (j = -iradius; j <= iradius; j++) {
       
       rpos = i * spacing;
       cpos = j * spacing;
        
       // Compute location of sample in terms of real-valued index array
       //  coordinates.  Subtract 0.5 so that rx of 1.0 means to put full
       // weight on index[1] (e.g., when rpos is 0 and eindexSize is 3.
       //rx, cx goes from -1,0,1,2,3,...,IndexSize
       rx = rpos + (eindexSize - 1) / 2.0;
       cx = cpos + (eindexSize - 1) / 2.0;
       
       //cout <<"rx " << rx << " " << cx << endl;
       // Test whether this sample falls within boundary of index patch. 
       if (rx > -1.0 && rx < (float) eindexSize  &&
	   cx > -1.0 && cx < (float) eindexSize)
         //cout << "in" << cpos << " " << rpos << endl;
	 AddSample(index, grad, ori, iradius + i, iradius + j, rpos, cpos,
		   rx, cx, eindexSize);
   } 

}

 
/* Normalize length of vec to 1.0.
*/
void NormalizeEVect(float *vec, int len, float norm)
{
   int i;
   float val, fac, sqlen = 0.0;

   for (i = 0; i < len; i++) {
     val = vec[i];
     //sqlen += val * val;
     sqlen += fabs(val);
   }

   //   fac = norm / sqrt(sqlen);
   fac=sqlen/len;
   for (i = 0; i < len; i++)
     vec[i] = vec[i]/fac *100;
}
 
void ESiftDescriptor::MakeKeypointSample(DARY *grad, DARY *ori, float *gvec, float norm, int eindexSize, int esiftSize)
{
   int i, intval, changed = FALSE;
  
   /* Produce sample vector. */
   KeySample(gvec, grad, ori, eindexSize);

   /* Normalize vector.  This should provide illumination invariance
      for planar lambertian surfaces (except for saturation effects).
      Normalization also improves nearest-neighbor metric by
      increasing relative distance for vectors with few features.
      It is also useful to implement a distance threshold and to
      allow conversion to integer format.
   */

   NormalizeEVect(gvec, esiftSize, norm); 

   /* Now that normalization has been done, threshold elements of
      index vector to decrease emphasis on large gradient magnitudes.
      Admittedly, the large magnitude values will have affected the
      normalization, and therefore the threshold, so this is of
      limited value.
   */ 
   return;
   for (i = 0; i < esiftSize; i++)
     if (gvec[i] > EMaxIndexVal) {
       gvec[i] = EMaxIndexVal;
       changed = TRUE;
     }
   if (changed)
     NormalizeEVect(gvec, esiftSize, norm);
   
   /* Convert float vector to integer. Assume largest value in normalized
      vector is likely to be less than 0.5. */
   for (i = 0; i < esiftSize; i++) {
     intval = (int) (512.0 * gvec[i]);
     gvec[i] = (255 < intval) ? 255 : intval;
     //cout << "vec " << vec[i]<<endl;
   }
} 

 
void ESiftDescriptor::computeComponents(DARY *img){
  if(img==NULL){return;}
 
  size=ESiftSize;//EIndexSize*EIndexSize*OriSize;
  allocVec(size);
  

  //cout << LESiftSize << " " << ESiftSize  << endl;
  DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
  

  normalizeAffine(img,imgn);   
  normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);

  DARY * grad = new DARY(PATCH_SIZE,PATCH_SIZE);
  DARY * gori = new DARY(PATCH_SIZE,PATCH_SIZE);    
  

  GradOriImages(imgn,grad,gori);
  MakeKeypointSample(grad, gori,vec,1.0, EIndexSize,ESiftSize);
  
  delete imgn;delete grad;delete gori;
  state=1;
  
  int esift_pca_size=128;
  pca(esift_pca_size,esift_pca_avg, esift_pca_base);	
  

} 


void computeESiftDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    ESiftDescriptor * ds = new  ESiftDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rgloh descriptor "<< c<< " of "<< desc.size() << "    " << flush;
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
