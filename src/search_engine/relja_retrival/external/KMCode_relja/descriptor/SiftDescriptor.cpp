#include "siftDescriptor.h"
#include "shapeDescriptor.h"

/* David Lowe's code*/

/* Increment the appropriate locations in the index to incorporate
   this image sample.  The location of the sample in the index is (rx,cx). 
*/
void SiftDescriptor::PlaceInIndex(float *index,
		  float mag, float ori, float rx, float cx)
{
   int r, c, ort, ri, ci, oi, rindex, cindex, oindex, rcindex;
   float oval, rfrac, cfrac, ofrac, rweight, cweight, oweight;
   
   oval = OriSize * ori / (M_2PI);
   
   ri = (int)((rx >= 0.0) ? rx : rx - 1.0);  /* Round down to next integer. */
   ci = (int)((cx >= 0.0) ? cx : cx - 1.0);
   oi = (int)((oval >= 0.0) ? oval : oval - 1.0);
   rfrac = rx - ri;         /* Fractional part of location. */
   cfrac = cx - ci;
   ofrac = oval - oi; 
   /*   assert(ri >= -1  &&  ri < IndexSize  &&  oi >= 0  &&  oi <= OriSize  && rfrac >= 0.0  &&  rfrac <= 1.0);*/
   
   /* Put appropriate fraction in each of 8 buckets around this point
      in the (row,col,ori) dimensions.  This loop is written for
      efficiency, as it is the inner loop of key sampling. */
   for (r = 0; r < 2; r++) {
      rindex = ri + r;
      if (rindex >=0 && rindex < IndexSize) {
         rweight = mag * ((r == 0) ? 1.0 - rfrac : rfrac);
         
         for (c = 0; c < 2; c++) {
            cindex = ci + c;
            if (cindex >=0 && cindex < IndexSize) {
               cweight = rweight * ((c == 0) ? 1.0 - cfrac : cfrac);
               rcindex=(rindex*IndexSize+cindex)<<3;
               for (ort = 0; ort < 2; ort++) {
                  oindex = oi + ort;
                  if (oindex >= OriSize)  /* Orientation wraps around at PI. */
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
void SiftDescriptor::AddSample(float *index,
	       DARY *grad, DARY *orim, int r, int c, float rpos, float cpos,
	       float rx, float cx)
{
    float mag, ori, sigma, weight;
    
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
    
    while (ori > M_2PI)
      ori -= M_2PI;
    while (ori < 0.0)
      ori += M_2PI;     
    PlaceInIndex(index, mag, ori, rx, cx);
} 

/* Add features to vec obtained from sampling the grad and ori images
   for a particular scale.  Location of key is (scale,row,col) with respect
   to images at this scale.  We examine each pixel within a circular
   region containing the keypoint, and distribute the gradient for that
   pixel into the appropriate bins of the index array.
*/

void SiftDescriptor::KeySample(float *index, DARY *grad, DARY *ori)
{
   int i, j, iradius, irow, icol;
   float spacing, sine, cosine, rpos, cpos, rx, cx;
   
   /* Radius of index sample region must extend to diagonal corner of
      index patch plus half sample for interpolation. */
   
   iradius = grad->x()>>1;
   spacing = (IndexSize + 1) / (2.0*iradius);
   //   printf("spacing %f, scale %f, radius %d\n",spacing,scale, iradius);
   // Examine all points from the gradient image that could lie within the index square. 
   for (i = -iradius; i <= iradius; i++)
     for (j = -iradius; j <= iradius; j++) {
       
       rpos = i * spacing;
       cpos = j * spacing;
        
       // Compute location of sample in terms of real-valued index array
       //  coordinates.  Subtract 0.5 so that rx of 1.0 means to put full
       // weight on index[1] (e.g., when rpos is 0 and IndexSize is 3. 
       rx = rpos + (IndexSize - 1) / 2.0;
       cx = cpos + (IndexSize - 1) / 2.0;
       
       //cout <<"rx " << rx << " " << cx << endl;
       // Test whether this sample falls within boundary of index patch. 
       if (rx > -1.0 && rx < (float) IndexSize  &&
	   cx > -1.0 && cx < (float) IndexSize)
         //cout << "in" << cpos << " " << rpos << endl;
	 AddSample(index, grad, ori, iradius + i, iradius + j, rpos, cpos,
		   rx, cx);
   } 
}

 
/* L2 Normalize length of vec to 1.0.
*/
void NormalizeVect(float *vec, int len)
{
   int i;
   float val, fac, sqlen = 0.0;

   for (i = 0; i < len; i++) {
     val = vec[i];
     sqlen += val * val;
   }
   fac = 1.0 / sqrt(sqlen);
   for (i = 0; i < len; i++)
     vec[i] *= fac;
}
  
void SiftDescriptor::MakeKeypointSample(DARY *grad, DARY *ori)
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

   NormalizeVect(vec, size); 

   /* Now that normalization has been done, threshold elements of
      index vector to decrease emphasis on large gradient magnitudes.
      Admittedly, the large magnitude values will have affected the
      normalization, and therefore the threshold, so this is of
      limited value.
   */
   for (i = 0; i < size; i++)
     if (vec[i] > MaxIndexVal) { 
       vec[i] = MaxIndexVal;
       changed = TRUE;
     }
   if (changed)
     NormalizeVect(vec, size); 

   /* Convert float vector to integer. Assume largest value in normalized
      vector is likely to be less than 0.5. */
   for (i = 0; i < size; i++) {
     intval = (int) (512.0 * vec[i]);
     vec[i] = (255 < intval) ? 255 : intval;
     //cout << "vec " << vec[i]<<endl;
   }
} 


void SiftDescriptor::computeComponents(DARY *img){
  static int desc_num = 0;
  if(img==NULL){return;}
  //int mins = (int)(GAUSS_CUTOFF*c_scale+2);
  //if(!isOK(mins,img->x()-mins,img->y()-mins))return;
 
  size=SiftSize;//IndexSize*IndexSize*OriSize;
  allocVec(size);
  
  DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
  DARY * grad = new DARY(PATCH_SIZE,PATCH_SIZE);
  DARY * ori = new DARY(PATCH_SIZE,PATCH_SIZE);    
  
  bool ret = normalizeAffine(img,imgn);
  normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);

  // JAMES
  //char temp[256];
  //sprintf(temp, "sift_region_%04d.pgm", desc_num);
  //imgn->write(temp);
  // -----

  //imgn->write("imgn.pgm");cout << "wrote"<< endl;getchar();
  GradOriImages(imgn,grad,ori);
  //canny(imgn, grad, ori, 5, 15);
  MakeKeypointSample(grad, ori);
  delete imgn;delete grad;delete ori;
  
  //if (ret) state = 0;
  //else state=1;
  state = 1;
 

  int sift_pca_size=128;
  //pca(sift_pca_size,sift_pca_avg,sift_pca_base);	
  desc_num++;
} 

   
void computeSiftDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    SiftDescriptor * ds = new  SiftDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	//cout << "\rsift descriptor "<< c<< " of "<< desc.size() << "    " << flush;
	ds->copy(desc[c]);
	ds->computeComponents(image);
 	desc[c]->copy((CornerDescriptor*)ds); 
    }
    delete ds;
    for(unsigned int c=0;c<desc.size();c++){
	if(!desc[c]->isOK()){
	    desc.erase((std::vector<CornerDescriptor*>::iterator)&desc[c]);
	    c--;
	}
    } 
    #ifndef QUIET
    cout << desc.size() << " sift descriptors calculated" << endl;
    #endif
}
