#include "shapeDescriptor.h" 

/* David Lowe's code*/

/* Increment the appropriate locations in the index to incorporate
   this image sample.  The location of the sample in the index is (rx,cx). 
*/
void ShapeDescriptor::PlaceInIndex(float *index,
		  float mag, float ori, float rx, float cx)
{
   int r, c, ort, ri, ci, oi, rindex, cindex, oindex, rcindex;
   float oval, rfrac, cfrac, ofrac, rweight, cweight, oweight;
   
   oval = SOriSize * ori / (M_2PI);
   
   ri = (rx >= 0.0) ? rx : rx - 1.0;  /* Round down to next integer. */
   ci = (cx >= 0.0) ? cx : cx - 1.0;
   oi = (oval >= 0.0) ? oval : oval - 1.0;
   rfrac = rx - ri;         /* Fractional part of location. */
   cfrac = cx - ci;
   ofrac = oval - oi; 
   /*   assert(ri >= -1  &&  ri < SIndexSize  &&  oi >= 0  &&  oi <= SOriSize  && rfrac >= 0.0  &&  rfrac <= 1.0);*/
   
   /* Put appropriate fraction in each of 8 buckets around this point
      in the (row,col,ori) dimensions.  This loop is written for
      efficiency, as it is the inner loop of key sampling. */
   for (r = 0; r < 2; r++) {
      rindex = ri + r;
      if (rindex >=0 && rindex < SIndexSize) {
         rweight = mag * ((r == 0) ? 1.0 - rfrac : rfrac);
         
         for (c = 0; c < 2; c++) {
            cindex = ci + c;
            if (cindex >=0 && cindex < SIndexSize) {
               cweight = rweight * ((c == 0) ? 1.0 - cfrac : cfrac);
               rcindex=(rindex*SIndexSize+cindex)<<3;
               for (ort = 0; ort < 2; ort++) {
                  oindex = oi + ort;
                  if (oindex >= SOriSize)  /* Orientation wraps around at PI. */
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
void ShapeDescriptor::AddSample(float *index,
	       DARY *grad, DARY *orim, int r, int c, float rpos, float cpos,
	       float rx, float cx)
{
    float mag, ori, sigma, weight;
    
    /* Clip at image boundaries. */
    if (r < 0  ||  r >= grad->y()  ||  c < 0  ||  c >= grad->x())
       return;
    
    /* Compute Gaussian weight for sample, as function of radial distance
       from center.  Sigma is relative to half-width of index. */
    //sigma =  0.5*(SIndexSize+1)*(SIndexSize+1);
    //sigma = (SIndexSize+1)*(SIndexSize+1);
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

void ShapeDescriptor::KeySample(float *index, DARY *grad, DARY *ori)
{
   int i, j, iradius, irow, icol;
   float spacing, sine, cosine, rpos, cpos, rx, cx;
   
   /* Radius of index sample region must extend to diagonal corner of
      index patch plus half sample for interpolation. */
   
   iradius = grad->x()>>1;
   spacing = (SIndexSize + 1) / (2.0*iradius);
   //   printf("spacing %f, scale %f, radius %d\n",spacing,scale, iradius);
   // Examine all points from the gradient image that could lie within the index square. 
   for (i = -iradius; i <= iradius; i++)
     for (j = -iradius; j <= iradius; j++) {
       
       rpos = i * spacing;
       cpos = j * spacing;
        
       // Compute location of sample in terms of real-valued index array
       //  coordinates.  Subtract 0.5 so that rx of 1.0 means to put full
       // weight on index[1] (e.g., when rpos is 0 and SIndexSize is 3. 
       rx = rpos + (SIndexSize - 1) / 2.0;
       cx = cpos + (SIndexSize - 1) / 2.0;
       
       //cout <<"rx " << rx << " " << cx << endl;
       // Test whether this sample falls within boundary of index patch. 
       if (rx > -1.0 && rx < (float) SIndexSize  &&
	   cx > -1.0 && cx < (float) SIndexSize)
         //cout << "in" << cpos << " " << rpos << endl;
	 AddSample(index, grad, ori, iradius + i, iradius + j, rpos, cpos,
		   rx, cx);
   } 
}

 
/* Normalize length of vec to 1.0.
*/
void SNormalizeVect(float *vec, int len)
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
  
void ShapeDescriptor::MakeKeypointSample(DARY *grad, DARY *ori)
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

   SNormalizeVect(vec, size); 

   /* Now that normalization has been done, threshold elements of
      index vector to decrease emphasis on large gradient magnitudes.
      Admittedly, the large magnitude values will have affected the
      normalization, and therefore the threshold, so this is of
      limited value.
   */
   for (i = 0; i < size; i++)
     if (vec[i] > SMaxIndexVal) { 
       vec[i] = SMaxIndexVal;
       changed = TRUE;
     }
   if (changed)
     SNormalizeVect(vec, size); 

   /* Convert float vector to integer. Assume largest value in normalized
      vector is likely to be less than 0.5. */
   for (i = 0; i < size; i++) {
     intval = (int) (512.0 * vec[i]);
     vec[i] = (255 < intval) ? 255 : intval;
     //cout << "vec " << vec[i]<<endl;
   }
} 

float sbilin(DARY *grad, float x, float y){

  int xi = (int)floor(x);
  float a = x-xi;
  int yi = (int)floor(y);
  float b = y-yi;
  
  float a1,b1,v00,v10,v01,v11;
  a1=1.0-a;
  b1=1.0-b;
   
  v00 = a1*grad->fel[yi][xi];
  v10 = a*grad->fel[yi][xi+1];
  v01 = a1*grad->fel[yi+1][xi];
  v11 = a*grad->fel[yi+1][xi+1];
  return b1*(v00+v10)+b*(v01+v11);
}

void canny(DARY *img, DARY *grad, DARY *ori, 
	   float lower_threshold,
	   float higher_threshold){
  
  DARY *dx = new DARY(img->y(),img->x(),0.0);
  DARY *dy = new DARY(img->y(),img->x(),0.0);
  DARY *tmp_edge = new DARY(img->y(),img->x(),0.0);
  DARY *edge = new DARY(img->y(),img->x(),0.0);
  dX2(img,dx);
  dY2(img,dy);
  for(int j=0;j<grad->y();j++){
    for(int i=0;i<grad->x();i++){
      grad->fel[j][i]=sqrt(dx->fel[j][i]*dx->fel[j][i]+dy->fel[j][i]*dy->fel[j][i]); 
      if(grad->fel[j][i]<1){
	grad->fel[j][i]=0;dx->fel[j][i]=0;dy->fel[j][i]=0;
      }	
    }
  }
  vector<int> cor_edge;cor_edge.push_back(0);
  float color=185;
  float x1,y1,x2,y2,ux,uy,g,g1,g2,nb=1;
  for(int j=1;j<grad->y()-2;j++){
    for(int i=1;i<grad->x()-2;i++){
      ux=dx->fel[j][i];
      uy=dy->fel[j][i];
      g=grad->fel[j][i];
      x1=i+ux/g;
      y1=j+uy/g;
      x2=i-ux/g;
      y2=j-uy/g;
      //cout <<g<< " " << ux/g << " " << uy/g <<" "<<i<< " " << j << " " << x1 << " " << y1 << " " << x2 << " " << y2 << endl;
      g1=sbilin(grad,x1,y1);
      g2=sbilin(grad,x2,y2);    
      if(g<g1 || g<=g2 || g<lower_threshold)continue;
      else if(g>higher_threshold)edge->fel[j][i]=color;
      else if(g>lower_threshold && (edge->fel[j-1][i-1]==color  ||
				    edge->fel[j-1][i]==color ||
				    edge->fel[j-1][i+1]==color ||
				    edge->fel[j][i-1]==color)){	
	edge->fel[j][i]=color;}
      else if(g>lower_threshold){
	if(tmp_edge->fel[j-1][i-1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i-1];
	}
	else if(tmp_edge->fel[j-1][i]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i];
	}
	else if(tmp_edge->fel[j-1][i+1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j-1][i+1];
	}
	else if(tmp_edge->fel[j][i-1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j][i-1];
	}
	else if(tmp_edge->fel[j][i+1]>0){
	  tmp_edge->fel[j][i]=tmp_edge->fel[j][i+1];
	}else {
	  tmp_edge->fel[j][i]=nb;cor_edge.push_back(0);nb++;
	}
      }
    }
  }

  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
	if((edge->fel[j-1][i-1]==color ||
	    edge->fel[j-1][i]==color ||
	    edge->fel[j-1][i+1]==color ||
	    edge->fel[j][i-1]==color ||
	    edge->fel[j][i+1]==color ||
	    edge->fel[j+1][i-1]==color ||
	    edge->fel[j+1][i]==color ||
	    edge->fel[j+1][i+1]==color))cor_edge[(int)tmp_edge->fel[j][i]]=1;	
    }
  }  
  for(int j=edge->y()-2;j>0;j--){
    for(int i=edge->x()-2;i>0;i--){
      if(tmp_edge->fel[j][i]>0 && cor_edge[(int)tmp_edge->fel[j][i]]==1){	
	edge->fel[j][i]=color;
      }
    }
  } 

  for(int j=0;j<grad->y();j++){
    for(int i=0;i<grad->x();i++){
      if(edge->fel[j][i]==0)grad->fel[j][i]=0;
      ori->fel[j][i]=atan2(dy->fel[j][i],dx->fel[j][i]);
    }
  }


  delete dx;delete dy;delete edge;delete tmp_edge;
  cor_edge.clear();

}



void ShapeDescriptor::computeComponents(DARY *img){
  if(img==NULL){return;}
 
  size=ShapeSize;//SIndexSize*SIndexSize*SOriSize;
  allocVec(size);
  
  DARY * imgn = new DARY(PATCH_SIZE,PATCH_SIZE);   
  DARY * grad = new DARY(PATCH_SIZE,PATCH_SIZE);
  DARY * ori = new DARY(PATCH_SIZE,PATCH_SIZE);    
  
  normalizeAffine(img,imgn);  
  normalize(imgn,imgn->x()>>1,imgn->y()>>1,imgn->x()>>1);
  canny(imgn,grad,ori,5,15);
  
//  grad->write("edge.pgm");cout << "OK"<< endl;getchar();
  MakeKeypointSample(grad, ori);
  delete imgn;delete grad;delete ori;
  state=1;
  int sc_pca_size=36;
  pca(sc_pca_size,sc_pca_avg,sc_pca_base);	

} 

   
void computeShapeDescriptors(DARY *image,  vector<CornerDescriptor *> &desc){
    initPatchMask(PATCH_SIZE);
    ShapeDescriptor * ds = new  ShapeDescriptor();
    for(unsigned int c=0;c<desc.size();c++){
	cout << "\rshape descriptor "<< c<< " of "<< desc.size() << "    " << flush;
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
