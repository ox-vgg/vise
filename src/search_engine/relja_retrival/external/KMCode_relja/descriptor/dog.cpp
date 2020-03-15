#include "dog.h"
 
void GradOriImages(DARY *im, DARY  *ori);
void ReduceSize(DARY *image,DARY*& newimage);
float FindOri(DARY * grad, DARY * ori, int row, int col);
/* Convolve image with the a 1-D Gaussian kernel vector along image rows.
   The Gaussian has a sigma of sqrt(2), which results in the following kernel:
     (.030 .105 .222 .286 .222 .105 .030)
   Pixels outside the image are set to the value of the closest image pixel.
*/
void _HorConvSqrt2(DARY *image,  DARY *result)
{
    int rows, cols, r, c;
    float **pix, **rpix, *prc, p1;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (c = 3; c < cols - 3; c++) {
	prc =  pix[r] + c;
	rpix[r][c] = 0.030 * prc[-3] + 0.105 * prc[-2] + 0.222 * prc[-1] +
	  0.286 * prc[0] + 0.222 * prc[1] + 0.105 * prc[2] + 0.030 * prc[3];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (c = 0; c < 3; c++) {
	p1 = (c < 1) ? pix[r][0] : pix[r][c-1];
	prc =  pix[r] + c;
	rpix[r][c] = 0.135 * pix[r][0] + 0.222 * p1 +
	  0.286 * prc[0] + 0.222 * prc[1] + 0.105 * prc[2] + 0.030 * prc[3];
      }
      for (c = cols - 3; c < cols; c++) {
	p1 = (c >= cols - 1) ? pix[r][cols-1] : pix[r][c+1];
	prc =  pix[r] + c;
	rpix[r][c] = 0.030 * prc[-3] + 0.105 * prc[-2] + 0.222 * prc[-1] +
	  0.286 * prc[0] + 0.222 * p1 + 0.135 * pix[r][cols-1];
      }
    }
}

/* Same as _HorConvSqrt2, but convolve along vertical direction.
*/
void _VerConvSqrt2( DARY *image,  DARY *result)
{
    int rows, cols, r, c;
    float **pix, **rpix, p1;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (r = 3; r < rows - 3; r++) {
	rpix[r][c] = 0.030 * pix[r-3][c] + 0.105 * pix[r-2][c] +
	  0.222 * pix[r-1][c] + 0.286 * pix[r][c] + 0.222 * pix[r+1][c] +
	  0.105 * pix[r+2][c] + 0.030 * pix[r+3][c];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (r = 0; r < 3; r++) {
	p1 = (r < 1) ? pix[0][c] : pix[r-1][c];
	rpix[r][c] = 0.135 * pix[0][c] + 0.222 * p1 +
	  0.286 * pix[r][c] + 0.222 * pix[r+1][c] +
	  0.105 * pix[r+2][c] + 0.030 * pix[r+3][c];
      }
      for (r = rows - 3; r < rows; r++) {
	p1 = (r >= rows - 1) ? pix[rows-1][c] : pix[r+1][c];
	rpix[r][c] = 0.030 * pix[r-3][c] + 0.105 * pix[r-2][c] +
	  0.222 * pix[r-1][c] + 0.286 * pix[r][c] + 0.222 * p1 +
	  0.135 * pix[rows-1][c];
      }
    }
}


/* Check whether the image point closest to (row,col) is below the
   given value.  It would be possible to use bilinear interpolation to
   sample the image more accurately, but this is not necessary as we
   are looking for a worst-case peak value.
*/
int CheckDipPoint(float val, DARY * image, float row, float col)
{
    int r, c;
    
    r = (int) (row + 0.5);
    c = (int) (col + 0.5);
    if (r < 0  ||  c < 0  ||  r >= (int)image->y()  ||
        	c >= (int)image->x())
	return 1;
    if (val > 0.0) {
	if (image->fel[r][c] > val)
	    return 0;
    } else {
	if (image->fel[r][c] < val)
	    return 0;
    }
    return 1;
}
/* Check whether 20 points sampled on a ring of radius 3 pixels around
   the center point are all less than Dip * "val".
*/
int CheckDip(float val, DARY *image, int row, int col)
{
    float x, y, nx, ny, cosine, sine;
    int i;
    float Dip=0.95;
    /* Create a vector of length 4 pixels (2*scale was used in orig). */
    x = 3.0;
    y = 0.0;

    /* Rotate this vector around a circle in increments of PI/20 and
       check the dip of each point. */
    sine = sin(M_PI / 20.0);
    cosine = cos(M_PI / 20.0);
    for (i = 0; i < 40; i++) {
	if (! CheckDipPoint(Dip * val, image, row + 0.5 + y, col + 0.5 + x))
	    return 0;
	nx = cosine * x - sine * y;
	ny = sine * x + cosine * y;
	x = nx;
	y = ny;
    }
    return 1;
}

/* Check whether this pixel is a local maximum (positive value) or
   minimum (negative value) compared to the 3x3 neighbourhood that
   is centered at (row,col).  If level is -1 (or 1), then (row,col) must
   be scaled down (or up) a level by factor 1.5.
*/
int LocalMaxMin(float val, DARY *dog, int row, int col, int level, float scale)
{
    int r, c;
    float **pix;

    pix = dog->fel;

    /* Calculate coordinates for image one level down (or up) in pyramid. */
    if (level < 0) {
      row = (3 * row + 1) / 2;
      col = (3 * col + 1) / 2;
    } else if (level > 0) {
      row = (2 * row) / 3;
      col = (2 * col) / 3;
    }

    if (val > 0.0) {
      for (r = row - 1; r <= row + 1; r++)
	for (c = col - 1; c <= col + 1; c++)
	  if (pix[r][c] > val)
	    return 0;
    } else {
      for (r = row - 1; r <= row + 1; r++)
	for (c = col - 1; c <= col + 1; c++)
	  if (pix[r][c] < val)
	    return 0;
    }

    return 1;
}


/* Return a number in the range [-0.5, 0.5] that represents the location
   of the peak of a parabola passing through the 3 samples.  The center
   value is assumed to be greater than or equal to the other values if
   positive, or less than if negative.
*/
float InterpPeak(float a, float b, float c)
{
    if (b < 0.0) {
	a = -a; b = -b; c = -c;
    }
     assert(b >= a  &&  b >= c);
    //if(b < a  ||  b < c)cout <<"ass "<< a << "  "<< b << " "<< c<< endl;
    return 0.5 * (a - c) / (a - 2.0 * b + c);
}
/* Find the position of the key point by interpolation in position, and
   create key vector.  Output its location as a square.
     The (grad2,ori2) provide gradient and orientation sampled one octave
   above that of (grad,ori).
*/
void InterpKeyPoint(int level, DARY *dog, DARY *grad, DARY *ori, 
		    DARY *grad2, DARY *ori2, int r, int c, float scale, 
		    vector<CornerDescriptor*> &corners)
{
  float center, lscale, rval, cval, opeak;

    center = dog->fel[r][c];
    /* Scale relative to input image.  Represents size of Gaussian
       for smaller of DOG filters used at this scale. */
    lscale = 1.414 * pow(1.5, (double) level);

    rval = r + InterpPeak(dog->fel[r-1][c], center, dog->fel[r+1][c]);
    cval = c + InterpPeak(dog->fel[r][c-1], center, dog->fel[r][c+1]);

    /* Scale up center location to account for level's scale change.
       A peak at (0,0) in gradient image would be at (0.5,0,5) in
       original image.  Add another 0.5 to place at pixel center.
       Therefore, peak at (0,0) gives location on boundary from 0 to 1. */
    rval = (rval + 1.0) * lscale / 1.414;
    cval = (cval + 1.0) * lscale / 1.414;

    /* If image was floatd to find keypoints, then row,col must be reduced
       by factor of 2 to put in coordinates of original image. */
    /* Find orientation(s) for this keypoint.*/
    opeak = FindOri(grad, ori, r, c);

    corners.push_back(new CornerDescriptor(cval,rval,rval,lscale,opeak));
    corners[corners.size()-1]->setType(3);
    //MakeKeypoint(lscale, opeak, r, c, rval, cval, grad, ori, grad2, ori2);
}


/* Find the local maxima and minima of the DOG images in the dog pyramid.
*/
void FindMaxMin(vector<DARY *> gauss,vector<DARY *> dog,vector<DARY *> ori,float scale, vector<CornerDescriptor*> &corners, 
		float thres)
{
    int i, r, c, rows, cols;
    float val;

    // fprintf(stderr, "Searching %d image levels, from size %dx%d\n",
    //    (int)dog.size(), dog[0]->y(), dog[0]->x());
    int levels=(int)dog.size();
    uint cor_nb=corners.size();
    /* Search through each scale, leaving 1 scale below and 2 above. */
    for (i = 1; i < levels - 2; i++) {
      rows = dog[i]->y();
      cols = dog[i]->x();
      
      /* Only find peaks at least 5 pixels from borders. */
      for (r = 5; r < rows - 5; r++)
	for (c = 5; c < cols - 5; c++) {
	  val = dog[i]->fel[r][c];
	  if (fabs(val) > thres  &&
	      LocalMaxMin(val, dog[i], r, c, 0,1.5) &&
	      LocalMaxMin(val, dog[i-1], r, c, -1,1.5) &&
	      LocalMaxMin(val, dog[i+1], r, c, 1,1.5) /*&&
	      CheckDip(val, dog[i], r, c)*/){	   
	    InterpKeyPoint(i, dog[i],gauss[i],ori[i],
			   gauss[i + 2], ori[i + 2], r, c,scale, corners);
	    
	  }
	}
      cout << "\rscale "<< (i*1.414)<< "      "<< flush;
      cor_nb=corners.size();
      
    }
}

  

void dog(DARY *im, vector<CornerDescriptor*> &corners){
  float scale = 1.414;
  vector<DARY *> dog;
  vector<DARY *> gauss;
  vector<DARY *> ori;
  
  printf("%d,%d\n", im->y(), im->x());

  printf("*\n");
  DARY *image = new DARY(im->y(),im->x());
  for (uint r = 0; r < im->y(); r++)
      for (uint c = 0; c < im->x(); c++)
	  image->fel[r][c]  = (im->fel[r][c]/255.0);  

  DARY *tmp = new DARY(image->y(),image->x());
  DARY *blur, *smaller;
  _HorConvSqrt2(image, tmp);
  _VerConvSqrt2(tmp,  image);
  
  uint lev=0,MinImageSize=10, first = 1;;
  while (image->y() > MinImageSize &&
	 image->x() > MinImageSize && lev < 20) {

    blur = new DARY(image->y(),image->x());
    _HorConvSqrt2(image, tmp);
    _VerConvSqrt2(tmp, blur);
    for (uint r = 0; r < image->y(); r++)
      for (uint c = 0; c < image->x(); c++){
    	tmp->fel[r][c] = 10.0 * (image->fel[r][c] - blur->fel[r][c]);
      }
    dog.push_back(tmp); 
    //DARY *bt = new DARY(blur);
    //tmp->write("dog.pgm");bt->normalize(0,1);bt->write("gauss.pgm");cout << "OK"<< endl;getchar();
    ReduceSize(blur,smaller);
    gauss.push_back(image);
    ori.push_back(blur);
    if (! first)
      GradOriImages(gauss[lev], ori[lev]);
    first = 0;
    //tmp->write("dogf.pgm");cout << "OKs"<< endl;getchar();
    //printf("%d,%d\n", smaller->y(), smaller->x());
    tmp = new DARY(smaller->y(),smaller->x());     
    //cout << "level "<< lev<< endl;
    lev++;
    image = smaller;
  }  
  FindMaxMin(gauss,dog,ori, scale,corners,0.3); 
  for(uint i=0;i<dog.size();i++){
      delete dog[i]; 
      delete gauss[i]; 
      delete ori[i]; 
  }
}

void ReduceSize(DARY *image,DARY*& newimage)
{
    int nrows, ncols, r, c, r1, r2, c1, c2;
    float **im, **newe;

    nrows = (2 * image->y()) / 3;
    ncols = (2 * image->x()) / 3;
    newimage=new DARY(nrows, ncols);
    im = image->fel;
    newe = newimage->fel;

    for (r = 0; r < nrows; r++)
      for (c = 0; c < ncols; c++) {
	if (r % 2 == 0) {
	  r1 = (r >> 1) * 3;
	  r2 = r1 + 1;
	} else {
	  r1 = (r >> 1) * 3 + 2;
	  r2 = r1 - 1;
	}      
	if (c % 2 == 0) {
	  c1 = (c >> 1) * 3;
	  c2 = c1 + 1;
	} else {
	  c1 = (c >> 1) * 3 + 2;
	  c2 = c1 - 1;
	}      
	newe[r][c] = 0.5625 * im[r1][c1] + 0.1875 * (im[r2][c1] + im[r1][c2]) +
	  0.0625 * im[r2][c2];
      }
}

/* Given a smoothed image "im", return edge gradients and orientations
      in "im" and "ori".  The gradient is computed from pixel differences,
      so is offset by half a pixel from original image.
   Result is normalized so that threshold value is 1.0, for ease of
      display and to make results less sensitive to change in threshold.
*/
void GradOriImages(DARY *im, DARY  *ori)
{
    float xgrad, ygrad, invthresh;
    float **pix;
    int rows, cols, r, c;
    float GradThresh=0.1;
    rows = im->y();
    cols = im->x();
    pix = im->fel;

    for (r = 0; r < rows-1; r++)
      for (c = 0; c < cols-1; c++) {
	xgrad = pix[r][c+1] - pix[r][c];
	ygrad = pix[r][c] - pix[r+1][c];
	pix[r][c] = sqrt(xgrad * xgrad + ygrad * ygrad);
	ori->fel[r][c] = atan2 (ygrad, xgrad);
      }
    /* Threshold all edge magnitudes at GradThresh and scale to 1.0. */
    invthresh = 1.0 / GradThresh;
    for (r = 0; r < rows; r++)
      for (c = 0; c < cols; c++) {
	if (pix[r][c] > GradThresh)
	  pix[r][c] = 1.0;
	else pix[r][c] *= invthresh;
      }
}

/* Smooth a histogram by using the [0.25, 0.5, 0.25] kernel.  Assume
   the histogram is connected in a circular buffer.
*/
void SmoothHistogram(float *hist, int bins)
{
    int i;
    float prev, temp;

    prev = hist[bins - 1];
    for (i = 0; i < bins; i++) {
      temp = hist[i];
      hist[i] = 0.25 * prev + 0.5 * hist[i] + 0.25 *
	          hist[(i + 1 == bins) ? 0 : i + 1];
      prev = temp;
    }
}


/* Find a peak in the histogram and return corresponding angle.
*/
float FindOriPeaks(float *hist, int bins)
{
    int i, maxloc = 0;
    float maxval = 0.0;

    /* Find peak in histogram. */
    for (i = 0; i < bins; i++)
      if (hist[i] > maxval) {
	maxval = hist[i];
	maxloc = i;
      }
    /* Set angle in range -PI to PI. */
    return (2.0 * M_PI * (maxloc + 0.5) / bins - M_PI);
}



/* Assign an orientation to this keypoint.  This is done by
     creating a Gaussian weighted histogram of the gradient directions in
     the region.  The histogram is smoothed and the largest peak selected.
     The results are in the range of -PI to PI.
*/
float FindOri(DARY * grad, DARY * ori, int row, int col)
{
    int i, r, c, rows, cols, radius, bin;
    const int OriBins=36;
    float hist[OriBins], distsq, gval, weight, angle;
    float OriSigma=3.0;
    rows = grad->y();
    cols = grad->x();

    for (i = 0; i < OriBins; i++)
      hist[i] = 0.0;

    /* Look at pixels within 3 sigma around the point and put their
       Gaussian weighted values in the histogram. */
    radius = (int) (OriSigma * 3.0);
    for (r = row - radius; r <= row + radius; r++)
      for (c = col - radius; c <= col + radius; c++)
	/* Do not use last row or column, which are not valid. */
	if (r >= 0 && c >= 0 && r < rows - 2 && c < cols - 2) {
	  gval = grad->fel[r][c];
	  distsq = (r - row) * (r - row) + (c - col) * (c - col);
	  if (gval > 0.0  &&  distsq < radius * radius + 0.5) {
	    weight = exp(- distsq / (2.0 * OriSigma * OriSigma));
	    /* Ori is in range of -PI to PI. */
	    angle = ori->fel[r][c];
	    bin = (int) (OriBins * (angle + M_PI + 0.001) / (2.0 * M_PI));
	    assert(bin >= 0 && bin <= OriBins);
	    if(bin>(OriBins - 1))bin=(OriBins - 1);
	    hist[bin] += weight * gval;
	  }
	}
    /* Apply smoothing twice. */
    SmoothHistogram(hist, OriBins);
    SmoothHistogram(hist, OriBins);

    return FindOriPeaks(hist, OriBins);
}
