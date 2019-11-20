#include "gauss_iir.h"
      
     
static float** table_exp_lap;
static float** table_exp;
static float** table_exp_x;
static float** table_exp_xx;
static float** table_exp_xy;
static float** table_exp_xxx;
static float** table_exp_xxy;
static float** table_exp_xxxx;
static float** table_exp_xxxy;
static float** table_exp_xxyy;
static float g_scale=0;
static float x_scale=0;
static float xx_scale=0;
static float lap_scale=0;
static float xy_scale=0;
static float xxx_scale=0;
static float xxy_scale=0;
static float xxxx_scale=0;
static float xxxy_scale=0;
static float xxyy_scale=0;
static float total=0;
static int size=0;
static float sum_x=0;
 
void set_nii_and_dii (float sigma, 
		      float a0, float a1, float b0, float b1, float c0, float c1,
		      float omega0, float omega1, float* coefs){
						 
						 
  //   c->n00p 
  coefs[0] = a0 + c0;
  //   c->n11p 
  coefs[1] =   exp(-b1/sigma) * (c1*sin(omega1/sigma)-(c0+2*a0)*cos(omega1/sigma))
    + exp(-b0/sigma) * (a1*sin(omega0/sigma)-(2*c0+a0)*cos(omega0/sigma));
  //   c->n22p 
  coefs[2] = 2 * exp(-b0/sigma -b1/sigma) *  ((a0+c0)*cos(omega1/sigma)*cos(omega0/sigma)
						   - cos(omega1/sigma)*a1*sin(omega0/sigma)
						   - cos(omega0/sigma)*c1*sin(omega1/sigma))
    + c0*exp(-2*b0/sigma) + a0*exp(-2*b1/sigma);
  //   c->n33p 
  coefs[3] =   exp(-b1/sigma-2*b0/sigma) * (c1*sin(omega1/sigma)-c0*cos(omega1/sigma))
    + exp(-b0/sigma-2*b1/sigma) * (a1*sin(omega0/sigma)-a0*cos(omega0/sigma));
  //   c->d11p 
  coefs[8] =  -2*exp(-b1/sigma)*cos(omega1/sigma) - 2*exp(-b0/sigma)*cos(omega0/sigma);
  //   c->d22p 
  coefs[9] =   4*cos(omega1/sigma)*cos(omega0/sigma)*exp(-b0/sigma -b1/sigma)
    + exp(-2*b1/sigma) + exp(-2*b0/sigma);
  //   c->d33p 
  coefs[10] =  -2*cos(omega0/sigma)*exp(-b0/sigma -2*b1/sigma)
    - 2*cos(omega1/sigma)*exp(-b1/sigma -2*b0/sigma);
  //   c->d44p 
  coefs[11] = exp(-2*b0/sigma -2*b1/sigma);
}
    
      void set_smooth_coefficients (float sigma, float* coefs){
	
	float a0, a1, b0, b1, c0, c1, omega0, omega1;
	
	a0     =  1.68;
	a1     =  3.735;
	b0     =  1.783;
	b1     =  1.723;
	c0     = -0.6803;
	c1     = -0.2598;
	omega0 =  0.6318;
	omega1 =  1.997;
	
	set_nii_and_dii(sigma, a0, a1, b0, b1, c0, c1, omega0, omega1, coefs);
	

	//	s->d11m = s->d11p;
	coefs[12]=coefs[8];
	//	s->d22m = s->d22p;
	coefs[13]=coefs[9];
	//	s->d33m = s->d33p;
	coefs[14]=coefs[10];
	//	s->d44m = s->d44p;
	coefs[15]=coefs[11];
	
	//s->n11m = s->n11p - s->d11p * s->n00p;
	coefs[4]=coefs[1] - coefs[8]*coefs[0];
	//s->n22m = s->n22p - s->d22p * s->n00p;
	coefs[5]=coefs[2] - coefs[9]*coefs[0];
	//s->n33m = s->n33p - s->d33p * s->n00p;
	coefs[6]=coefs[3] - coefs[10]*coefs[0];
	//s->n44m =         - s->d44p * s->n00p;
	coefs[7]=           - coefs[11]*coefs[0];
	
	//s->scale = 1/(sqrt(2*3.141)*sigma);
	coefs[16]=            1/(sqrt(2*3.141)*sigma);	
	//for(int i=0;i<16;i++)coefs[i]*=coefs[16];
      }
      void set_derive_coefficients (float sigma, float* coefs){

	float a0, a1, b0, b1, c0, c1, omega0, omega1;
	
	a0     =  -0.6472;
	a1     =  -4.531;
	b0     =   1.527;
	b1     =   1.516;
	c0     =   0.6494;
	c1     =   0.9557;
	omega0 =   0.6719;
	omega1 =   2.072;
	
	set_nii_and_dii(sigma, a0, a1, b0, b1, c0, c1, omega0, omega1, coefs);
	
	//	s->d11m = s->d11p;
	coefs[12]=coefs[8];
	//	s->d22m = s->d22p;
	coefs[13]=coefs[9];
	//	s->d33m = s->d33p;
	coefs[14]=coefs[10];
	//	s->d44m = s->d44p;
	coefs[15]=coefs[11];
	
	//	d->n11m = - (d->n11p - d->d11p * d->n00p);
	//	d->n22m = - (d->n22p - d->d22p * d->n00p);
	//	d->n33m = - (d->n33p - d->d33p * d->n00p);
	//	d->n44m =              d->d44p * d->n00p ;
	coefs[4]=-(coefs[1] - coefs[8]*coefs[0]);
	coefs[5]=-(coefs[2] - coefs[9]*coefs[0]);
	coefs[6]=-(coefs[3] - coefs[10]*coefs[0]);
	coefs[7]=             coefs[11]*coefs[0];
	
	//	d->scale = -1/(sqrt(2*3.141)*sigma*sigma);
	coefs[16]=             -1/(sqrt(2*3.141)*sigma*sigma);
      }
      void set_second_derive_coefficients (float sigma, float* coefs){
	float a0, a1, b0, b1, c0, c1, omega0, omega1;
	
	a0     =  -1.331;
	a1     =  3.661;
	b0     =   1.24;
	b1     =   1.314;
	c0     =   0.3225;
	c1     =   -1.738;
	omega0 =   0.748;
	omega1 =   2.166;
      
	set_nii_and_dii(sigma, a0, a1, b0, b1, c0, c1, omega0, omega1, coefs);
	
	//	s->d11m = s->d11p;
	coefs[12]=coefs[8];
	//	s->d22m = s->d22p;
	coefs[13]=coefs[9];
	//	s->d33m = s->d33p;
	coefs[14]=coefs[10];
	//	s->d44m = s->d44p;
	coefs[15]=coefs[11];
	
	//s->n11m = s->n11p - s->d11p * s->n00p;
	coefs[4]=coefs[1] - coefs[8]*coefs[0];
	//s->n22m = s->n22p - s->d22p * s->n00p;
	coefs[5]=coefs[2] - coefs[9]*coefs[0];
	//s->n33m = s->n33p - s->d33p * s->n00p;
	coefs[6]=coefs[3] - coefs[10]*coefs[0];
	//s->n44m =         - s->d44p * s->n00p;
	coefs[7]=           - coefs[11]*coefs[0];
	
	//   d->scale = 1/(sqrt(2*3.141)*sigma*sigma*sigma);
	
	coefs[16]=            1/(sqrt(2*3.141)*sigma*sigma*sigma);
      }

 


/************************************************************************
   function to calculate the x(k)+, x(k)- and x(k)
   see [1] p. 10
*************************************************************************/


      void vertical (DARY* image,DARY* xp,float* coefs){
	/*   DARY *x  = NULL;*/
	int row_size, col_size;
	
	
	float n00p, n11p, n22p, n33p, n11m, n22m, n33m, n44m,
	    d11p, d22p, d33p, d44p, d11m, d22m, d33m, d44m,
	    scale;

	       	scale = coefs[16];

	//	n00p = c->n00p;n11p = c->n11p; n22p = c->n22p; n33p = c->n33p;
       	n00p = coefs[0]; n11p =  coefs[1]; n22p = coefs[2]; n33p =  coefs[3];
	//	n11m = c->n11m; n22m = c->n22m; n33m = c->n33m; n44m = c->n44m;
	n11m =  coefs[4]; n22m =  coefs[5]; n33m =  coefs[6]; n44m =  coefs[7];
       	//	d11p = c->d11p; d22p = c->d22p; d33p = c->d33p; d44p = c->d44p;
	d11p =  coefs[8]; d22p =  coefs[9]; d33p =  coefs[10]; d44p =  coefs[11];
	//	d11m = c->d11m; d22m = c->d22m; d33m = c->d33m; d44m = c->d44m;
	d11m =  coefs[12]; d22m =  coefs[13]; d33m =  coefs[14]; d44m =  coefs[15];
	//       	scale = c->scale;
   

	row_size = image->y();
	col_size = image->x();
	
	DARY* xm = new DARY(row_size,col_size);
	float np=n00p+n11p+n22p+n33p;
	float dp=d11p+d22p+d33p+d44p;
	float z = np/(1+dp);

	for (int row = 0; row < row_size; row++)
	    {
	      xp->fel[row][0] = (z* image->fel[row][0]);
	      xp->fel[row][1] =   (n00p * image->fel[row][1] + n11p * image->fel[row][0] 
		+ n22p * image->fel[row][0] + n33p * image->fel[row][0]
		- dp * xp->fel[row][0]);
	      xp->fel[row][2] =   (n00p * image->fel[row][2] + n11p * image->fel[row][1]
		+ n22p * image->fel[row][0] + n33p * image->fel[row][0]
		- d11p * xp->fel[row][1] - d22p * xp->fel[row][0] 
		- d33p * xp->fel[row][0] - d44p * xp->fel[row][0]);
	      xp->fel[row][3] =   (n00p * image->fel[row][3] + n11p * image->fel[row][2]
		+ n22p * image->fel[row][1] + n33p * image->fel[row][0]
		- d11p * xp->fel[row][2] - d22p * xp->fel[row][1]
		- d33p * xp->fel[row][0] - d44p * xp->fel[row][0]);
	
	      /*	xp->fel[row][0] =   n00p * image->fel[row][0];
		xp->fel[row][1] =   n00p * image->fel[row][1] + n11p * image->fel[row][0]
		    -  d11p * xp->fel[row][0];
		xp->fel[row][2] =   n00p * image->fel[row][2] + n11p * image->fel[row][1]
		    + n22p * image->fel[row][0]
                      -  d11p * xp->fel[row][1] -  d22p * xp->fel[row][0];
		xp->fel[row][3] =   n00p * image->fel[row][3] + n11p * image->fel[row][2]
		    + n22p * image->fel[row][1] +  n33p * image->fel[row][0]
		    -  d11p * xp->fel[row][2] -  d22p * xp->fel[row][1]
		    -  d33p * xp->fel[row][0];*/
		for (int col = 4; col < col_size; col++)
		    xp->fel[row][col] =  (n00p * image->fel[row][col  ] + n11p * image->fel[row][col-1]
			+ n22p * image->fel[row][col-2] +  n33p * image->fel[row][col-3]
			-  d11p * xp->fel[row][col-1] -  d22p * xp->fel[row][col-2]
			-  d33p * xp->fel[row][col-3] -  d44p * xp->fel[row][col-4]);
	    }
	
	/*  calculate the x(k)-  */
	float nm=n11m+n22m+n33m+n44m;
	float dm=d11m+d22m+d33m+d44m;
	z = nm/(1+dm);

	for (int row = 0; row < row_size; row++)
	    {
	      xm->fel[row][col_size-1] = z*image->fel[row][col_size-1];
	      xm->fel[row][col_size-2] =  nm * image->fel[row][col_size-1] - dm * xm->fel[row][col_size-1];
	      xm->fel[row][col_size-3] =  n11m * image->fel[row][col_size-2] + n22m * image->fel[row][col_size-1]
		+ n33m * image->fel[row][col_size-1] + n44m * image->fel[row][col_size-1]
		- d11m * xm->fel[row][col_size-2] - d22m * xm->fel[row][col_size-1]
		- d33m * xm->fel[row][col_size-1] - d44m * xm->fel[row][col_size-1];
	      xm->fel[row][col_size-4] =  n11m * image->fel[row][col_size-3] + n22m * image->fel[row][col_size-2]
		+ n33m * image->fel[row][col_size-1] + n44m * image->fel[row][col_size-1]
		- d11m * xm->fel[row][col_size-3] - d22m * xm->fel[row][col_size-2]
		- d33m * xm->fel[row][col_size-1] - d44m * xm->fel[row][col_size-1];
	      /*		xm->fel[row][col_size-1] =  0.0;
		xm->fel[row][col_size-2] =   n11m * image->fel[row][col_size-1] 
		    -  d11m * xm->fel[row][col_size-1];
		xm->fel[row][col_size-3] =   n11m * image->fel[row][col_size-2] 
		    +  n22m * image->fel[row][col_size-1]
		    -  d11m * xm->fel[row][col_size-2] 
		    -  d22m * xm->fel[row][col_size-1];
		xm->fel[row][col_size-4] =   n11m * image->fel[row][col_size-3] 
		    +  n22m * image->fel[row][col_size-2]
		    +  n33m * image->fel[row][col_size-1]
		    -  d11m * xm->fel[row][col_size-3] 
		    -  d22m * xm->fel[row][col_size-2]
		    -  d33p * xm->fel[row][col_size-1];*/
		for (int col = col_size-5; col >= 0 ; col--)
		    xm->fel[row][col] =  (n11m * image->fel[row][col+1] +  n22m * image->fel[row][col+2]
			+  n33m * image->fel[row][col+3] + n44m * image->fel[row][col+4]
			-  d11m * xm->fel[row][col+1] -  d22m * xm->fel[row][col+2]
			-  d33m * xm->fel[row][col+3] -  d44m * xm->fel[row][col+4]);
	    }

	
	/*  calculate the x(k)  */
	
	for (int row = 0; row < row_size; row++)
	    for (int col = 0; col < col_size; col++)
	      xp->fel[row][col] = (xp->fel[row][col] + xm->fel[row][col])*scale;
	delete xm;
	
      }
/************************************************************************
   function to calculate the y(k)+, y(k)- and y(k)
   see [1] p. 10
*************************************************************************/


      void horizontal (DARY* image,DARY* yp,float* coefs){
	int row_size, col_size;
	
	
	float n00p, n11p, n22p, n33p, n11m, n22m, n33m, n44m,
	    d11p, d22p, d33p, d44p, d11m, d22m, d33m, d44m,
	    scale;

	
	//	n00p = c->n00p;n11p = c->n11p; n22p = c->n22p; n33p = c->n33p;
       	n00p = coefs[0]; n11p = coefs[1]; n22p =coefs[2]; n33p = coefs[3];
	//	n11m = c->n11m; n22m = c->n22m; n33m = c->n33m; n44m = c->n44m;
	n11m = coefs[4]; n22m = coefs[5]; n33m = coefs[6]; n44m = coefs[7];
       	//	d11p = c->d11p; d22p = c->d22p; d33p = c->d33p; d44p = c->d44p;
	d11p = coefs[8]; d22p = coefs[9]; d33p = coefs[10]; d44p = coefs[11];
	//	d11m = c->d11m; d22m = c->d22m; d33m = c->d33m; d44m = c->d44m;
	d11m = coefs[12]; d22m = coefs[13]; d33m = coefs[14]; d44m = coefs[15];
	//       	scale = c->scale;
       	scale = coefs[16];
	
	

	row_size = image->y();
	col_size = image->x();
	
	DARY* ym = new DARY(row_size,col_size);
	
	
	
	//cout << "  calculate the y(k)+  "<< endl;
	float np=n00p+n11p+n22p+n33p;
	float dp=d11p+d22p+d33p+d44p;
	float z = np/(1+dp);
	//cout << "z "<< z<< endl;
	for (int col = 0; col < col_size; col++)
	    {
	      yp->fel[0][col] = z* image->fel[0][col];
	      yp->fel[1][col] =   n00p * image->fel[1][col] + n11p * image->fel[0][col] 
		+ n22p * image->fel[0][col] + n33p * image->fel[0][col]
		- dp * yp->fel[0][col];
	      yp->fel[2][col] =   n00p * image->fel[2][col] + n11p * image->fel[1][col]
		+ n22p * image->fel[0][col] + n33p * image->fel[0][col]
		- d11p * yp->fel[1][col] - d22p * yp->fel[0][col] 
		- d33p * yp->fel[0][col] - d44p * yp->fel[0][col];
	      yp->fel[3][col] =   n00p * image->fel[3][col] + n11p * image->fel[2][col]
		+ n22p * image->fel[1][col] + n33p * image->fel[0][col]
		- d11p * yp->fel[2][col] - d22p * yp->fel[1][col]
		- d33p * yp->fel[0][col] - d44p * yp->fel[0][col];
	      /*
	      yp->fel[0][col] =   n00p * image->fel[0][col];
		yp->fel[1][col] =   n00p * image->fel[1][col] + n11p * image->fel[0][col]
		    - d11p * yp->fel[0][col];
		yp->fel[2][col] =   n00p * image->fel[2][col] + n11p * image->fel[1][col]
		    + n22p * image->fel[0][col]
		    - d11p * yp->fel[1][col] - d22p * yp->fel[0][col];
		yp->fel[3][col] =   n00p * image->fel[3][col] + n11p * image->fel[2][col]
		    + n22p * image->fel[1][col] + n33p * image->fel[0][col]
                      - d11p * yp->fel[2][col] - d22p * yp->fel[1][col]
		    - d33p * yp->fel[0][col];
	      */
		for (int row = 4; row < row_size; row++)
		    yp->fel[row][col] =  n00p * image->fel[row  ][col] + n11p * image->fel[row-1][col]
			+ n22p * image->fel[row-2][col] + n33p * image->fel[row-3][col]
			- d11p * yp->fel[row-1][col] - d22p * yp->fel[row-2][col]
			- d33p * yp->fel[row-3][col] - d44p * yp->fel[row-4][col];
	    }
	
	//cout << "   /*  calculate the y(k)-  */ "<< endl;

	float nm=n11m+n22m+n33m+n44m;
	float dm=d11m+d22m+d33m+d44m;
	z = nm/(1+dm);
	//cout << "z "<< z<< endl;
	
	for (int col = 0; col < col_size; col++)
	    {
	      ym->fel[row_size-1][col] = z*image->fel[row_size-1][col];
	      ym->fel[row_size-2][col] =  nm * image->fel[row_size-1][col] - dm * ym->fel[row_size-1][col];
	      ym->fel[row_size-3][col] =  n11m * image->fel[row_size-2][col] + n22m * image->fel[row_size-1][col]
		+ n33m * image->fel[row_size-1][col] + n44m * image->fel[row_size-1][col]
		- d11m * ym->fel[row_size-2][col] - d22m * ym->fel[row_size-1][col]
		- d33m * ym->fel[row_size-1][col] - d44m * ym->fel[row_size-1][col];
	      ym->fel[row_size-4][col] =  n11m * image->fel[row_size-3][col] + n22m * image->fel[row_size-2][col]
		+ n33m * image->fel[row_size-1][col] + n44m * image->fel[row_size-1][col]
		- d11m * ym->fel[row_size-3][col] - d22m * ym->fel[row_size-2][col]
		- d33m * ym->fel[row_size-1][col] - d44m * ym->fel[row_size-1][col];
	      
	      /*	      ym->fel[row_size-1][col] = 0;
     	      ym->fel[row_size-2][col] =  n11m * image->fel[row_size-1][col] 
		    - d11m * ym->fel[row_size-1][col];
		ym->fel[row_size-3][col] =  n11m * image->fel[row_size-2][col] 
                              + n22m * image->fel[row_size-1][col]
		    - d11m * ym->fel[row_size-2][col] 
		    - d22m * ym->fel[row_size-1][col];
		ym->fel[row_size-4][col] =  n11m * image->fel[row_size-3][col] 
		    + n22m * image->fel[row_size-2][col]
		    + n33m * image->fel[row_size-1][col]
		    - d11m * ym->fel[row_size-3][col] 
		    - d22m * ym->fel[row_size-2][col]
		    - d33p * ym->fel[row_size-1][col];*/
		for (int row = row_size-5; row >= 0 ; row--)
		    ym->fel[row][col] =  n11m * image->fel[row+1][col] + n22m * image->fel[row+2][col]
			+ n33m * image->fel[row+3][col] + n44m * image->fel[row+4][col]
			- d11m * ym->fel[row+1][col] - d22m * ym->fel[row+2][col]
			- d33m * ym->fel[row+3][col] - d44m * ym->fel[row+4][col];
	    }
	
	
	for (int row = 0; row < row_size; row++)
	    for (int col = 0; col < col_size; col++)
	      yp->fel[row][col] = (yp->fel[row][col] + ym->fel[row][col]) * scale;

	delete ym;	
      }
void HorConvSqrt2(DARY *image,  DARY *result)
{

  //1.4   0.03 0.105 0.222 0.286 0.222 0.105 0.03

  //1.2   0.0146 0.0831 0.2356 0.3333 0.2356 0.0831 0.0146
    int rows, cols, r, c;
    float **pix, **rpix, *prc, p1,*pixr,*rpixr;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      pixr=pix[r];
      rpixr=rpix[r];
      for (c = 3; c < cols - 3; c++) {
	prc =  pixr + c;
	rpixr[c] = 0.030 * prc[-3] + 0.105 * prc[-2] + 0.222 * prc[-1] +
	  0.286 * prc[0] + 0.222 * prc[1] + 0.105 * prc[2] + 0.030 * prc[3];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (c = 0; c < 3; c++) {
	p1 = (c < 1) ? pix[r][0] : pix[r][c-1];
	prc =  pixr + c;
	rpixr[c] = 0.135 * pixr[0] + 0.222 * p1 +
	  0.286 * prc[0] + 0.222 * prc[1] + 0.105 * prc[2] + 0.030 * prc[3];
      }
      for (c = cols - 3; c < cols; c++) {
	p1 = (c >= cols - 1) ? pixr[cols-1] : pixr[c+1];
	prc =  pixr + c;
	rpixr[c] = 0.030 * prc[-3] + 0.105 * prc[-2] + 0.222 * prc[-1] +
	  0.286 * prc[0] + 0.222 * p1 + 0.135 * pixr[cols-1];
      }
    }
}



/* Same as HorConvSqrt2, but convolve along vertical direction.
*/
void VerConvSqrt2( DARY *image,  DARY *result)
{
  //1.4 0.03 0.105 0.222 0.286 0.222 0.105 0.03
  //1.2   0.0146 0.0831 0.2356 0.3333 0.2356 0.0831 0.0146
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
/* Same as HorConvSqrt2, but convolve along vertical direction.
*/
void smoothYYV9( DARY *image,  DARY *result)
{
  //2 = 0.0276    0.0663    0.1238    0.1802    0.2042    0.1802    0.1238    0.0663    0.0276
    int rows, cols, r, c;
    float **pix, **rpix;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (r = 4; r < rows - 4; r++) {
	rpix[r][c] =  0.0276 * pix[r-4][c] + 0.0663 * pix[r-3][c] + 0.1238 * pix[r-2][c] 
	  + 0.1802 * pix[r-1][c]   + 0.2042 * pix[r][c]  + 0.1802 * pix[r+1][c] + 0.1238 * pix[r+2][c] +
	  0.0663 * pix[r+3][c]+ 0.0276 * pix[r+4][c];
      }
      r=3;
	rpix[r][c] =   0.0939 * pix[r-3][c] + 0.1238 * pix[r-2][c] 
	  + 0.1802 * pix[r-1][c]   + 0.2042 * pix[r][c]  + 0.1802 * pix[r+1][c] + 0.1238 * pix[r+2][c] +
	  0.0663 * pix[r+3][c]+ 0.0276 * pix[r+4][c];
      r=2;
	rpix[r][c] =   0.2177 * pix[r-2][c] 
	  + 0.1802 * pix[r-1][c]   + 0.2042 * pix[r][c]  + 0.1802 * pix[r+1][c] + 0.1238 * pix[r+2][c] +
	  0.0663 * pix[r+3][c]+ 0.0276 * pix[r+4][c];
      r=1;
	rpix[r][c] =   
	   0.3979 * pix[r-1][c]   + 0.2042 * pix[r][c]  + 0.1802 * pix[r+1][c] + 0.1238 * pix[r+2][c] +
	  0.0663 * pix[r+3][c]+ 0.0276 * pix[r+4][c];
      r=0;
	rpix[r][c] =  0.6021 * pix[r][c]  + 0.1802 * pix[r+1][c] + 0.1238 * pix[r+2][c] +
	  0.0663 * pix[r+3][c]+ 0.0276 * pix[r+4][c];
      r=rows-4;      
	rpix[r][c] =  0.0276 * pix[r-4][c] + 0.0663 * pix[r-3][c] + 0.1238 * pix[r-2][c] 
	  + 0.1802 * pix[r-1][c]   + 0.2042 * pix[r][c]  + 0.1802 * pix[r+1][c] + 0.1238 * pix[r+2][c] +
	  0.0939 * pix[r+3][c];
      r=rows-3;      
	rpix[r][c] =  0.0276 * pix[r-4][c] + 0.0663 * pix[r-3][c] + 0.1238 * pix[r-2][c] 
	  + 0.1802 * pix[r-1][c]   + 0.2042 * pix[r][c]  + 0.1802 * pix[r+1][c] + 0.2177 * pix[r+2][c];
      r=rows-2;      
	rpix[r][c] =  0.0276 * pix[r-4][c] + 0.0663 * pix[r-3][c] + 0.1238 * pix[r-2][c] 
	  + 0.1802 * pix[r-1][c]   + 0.2042 * pix[r][c]  + 0.3979 * pix[r+1][c];
      r=rows-1;      
	rpix[r][c] =  0.0276 * pix[r-4][c] + 0.0663 * pix[r-3][c] + 0.1238 * pix[r-2][c] 
	+ 0.1802 * pix[r-1][c]   + 0.6021 * pix[r][c];
    }
}


void smoothXXH9(DARY *image,  DARY *result)
{

  //2 = 0.0276    0.0663    0.1238    0.1802    0.2042    0.1802    0.1238    0.0663    0.0276
    int rows, cols, r, c;
    float **pix, **rpix, *prc;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;
    
    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (c = 4; c < cols - 4; c++) {
	prc =  pix[r] + c;
	rpix[r][c] = 0.0276 * prc[-4] + 0.0663 * prc[-3] + 0.1238 * prc[-2] + 0.1802  * prc[-1]
	   +0.2042 * prc[0]  +0.1802 * prc[1]  + 0.1238 * prc[2]  + 0.0663 * prc[3] + 0.0276 * prc[4];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      prc =  pix[r]+3;
	rpix[r][3] = 0.0939 * prc[-3] + 0.1238 * prc[-2] + 0.1802  * prc[-1]
	   +0.2042 * prc[0]  +0.1802 * prc[1]  + 0.1238 * prc[2]  + 0.0663 * prc[3] + 0.0276 * prc[4];
      prc =  pix[r]+2;
	rpix[r][2] =  0.2177 * prc[-2] + 0.1802  * prc[-1]
	   +0.2042 * prc[0]  +0.1802 * prc[1]  + 0.1238 * prc[2]  + 0.0663 * prc[3] + 0.0276 * prc[4];
      prc =  pix[r]+1;
	rpix[r][1] = 0.3979  * prc[-1]
	   +0.2042 * prc[0]  +0.1802 * prc[1]  + 0.1238 * prc[2]  + 0.0663 * prc[3] + 0.0276 * prc[4];
      prc =  pix[r];
	rpix[r][0] = 
	   0.6021 * prc[0]  +0.1802 * prc[1]  + 0.1238 * prc[2]  + 0.0663 * prc[3] + 0.0276 * prc[4];

      prc =  pix[r]+cols-4;
	rpix[r][cols-4] = 0.0276 * prc[-4] + 0.0663 * prc[-3] + 0.1238 * prc[-2] + 0.1802  * prc[-1]
	   +0.2042 * prc[0]  +0.1802 * prc[1]  + 0.1238 * prc[2]  + 0.0939 * prc[3];
      prc =  pix[r]+cols-3;
	rpix[r][cols-3] = 0.0276 * prc[-4] + 0.0663 * prc[-3] + 0.1238 * prc[-2] + 0.1802  * prc[-1]
	   +0.2042 * prc[0]  +0.1802 * prc[1]  + 0.2177 * prc[2];
      prc =  pix[r]+cols-2;
	rpix[r][cols-2] = 0.0276 * prc[-4] + 0.0663 * prc[-3] + 0.1238 * prc[-2] + 0.1802  * prc[-1]
	   +0.2042 * prc[0]  +0.3979 * prc[1];
      prc =  pix[r]+cols-1;
	rpix[r][cols-1] = 0.0276 * prc[-4] + 0.0663 * prc[-3] + 0.1238 * prc[-2] + 0.1802  * prc[-1]
	+0.6021 * prc[0];
    }
}

void DerXXH7(DARY *image,  DARY *result)
{


  //2nd 1.2 =1 0.0560 0.1051 -0.0471 -0.2281 -0.0471 0.1051 0.0560 
    int rows, cols, r, c;
    float **pix, **rpix, *prc, p1,*pixr,*rpixr;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      pixr=pix[r];
      rpixr=rpix[r];
      for (c = 3; c < cols - 3; c++) {
	prc =  pixr + c;
	rpixr[c] = 0.0560 * prc[-3] + 0.1051 * prc[-2]  -0.0471 * prc[-1] -
	   0.2281 * prc[0]  -0.0471 * prc[1] + 0.1051 * prc[2] + 0.0560 * prc[3];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (c = 0; c < 3; c++) {
	p1 = (c < 1) ? pixr[0] : pixr[c-1];
	prc =  pix[r] + c;
	rpixr[c] = 0.1611 * pix[r][0] -0.0471 * p1 
	   -0.2281 * prc[0]  -0.0471 * prc[1] + 0.1051 * prc[2] + 0.0560 * prc[3];
      }
      for (c = cols - 3; c < cols; c++) {
	p1 = (c >= cols - 1) ? pixr[cols-1] : pixr[c+1];
	prc =  pix[r] + c;
	rpixr[c] = 0.0560 * prc[-3] + 0.1051 * prc[-2]  -0.0471 * prc[-1] 
	   -0.2281 * prc[0]  -0.0471 * p1 + 0.1611 * pixr[cols-1];
      }
    }
}



/* Same as HorConvSqrt2, but convolve along vertical direction.
*/
void DerYYV7( DARY *image,  DARY *result)
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
	rpix[r][c] = 0.0560 * pix[r-3][c] + 0.1051 * pix[r-2][c] 
	  -0.0471 * pix[r-1][c]   -0.2281 * pix[r][c]  -0.0471 * pix[r+1][c] +
	  0.1051 * pix[r+2][c] + 0.0560 * pix[r+3][c];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (r = 0; r < 3; r++) {
	p1 = (r < 1) ? pix[0][c] : pix[r-1][c];
	rpix[r][c] = 0.1611 * pix[0][c]  -0.0471 * p1 
	   -0.2281 * pix[r][c]  -0.0471 * pix[r+1][c] +
	  0.1051 * pix[r+2][c] + 0.0560 * pix[r+3][c];
      }
      for (r = rows - 3; r < rows; r++) {
	p1 = (r >= rows - 1) ? pix[rows-1][c] : pix[r+1][c];
	rpix[r][c] = 0.0560 * pix[r-3][c] + 0.1051 * pix[r-2][c] 
	  -0.0471 * pix[r-1][c]   -0.2281 * pix[r][c] + -0.0471 * p1 +
	  0.1611 * pix[rows-1][c];
      }
    }
}

void DerXXH9(DARY *image,  DARY *result)
{

  //2nd 2 =     0.0128 0.0216 0.0216 0 -0.0317 -0.0485 -0.0317 0 0.0216 0.0216 0.0128

    int rows, cols, r, c;
    float **pix, **rpix, *prc;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;
    
    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (c = 5; c < cols - 5; c++) {
	prc =  pix[r] + c;
	rpix[r][c] = 0.128 * prc[-5] + 0.216 * prc[-4] + 0.216 * prc[-3] -0.317  * prc[-1]
	   -0.485 * prc[0]  -0.317 * prc[1] + 0.216 * prc[3] + 0.216 * prc[4] + 0.128 * prc[5];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      prc =  pix[r]+4;
      rpix[r][4] = 0.344 * prc[-4] + 0.216 * prc[-3] -0.317  * prc[-1]
	-0.485 * prc[0] -0.317 * prc[1]  +0.216 * prc[3] + 0.216 * prc[4] + 0.128 * prc[5];
      prc =  pix[r]+3;
      rpix[r][3] = 0.560 * prc[-3] -0.317  * prc[-1]
	-0.485 * prc[0] -0.317 * prc[1]  +0.216 * prc[3] + 0.216 * prc[4] + 0.128 * prc[5];
      prc =  pix[r]+2;
      rpix[r][2] = 0.560 * prc[-2] -0.317  * prc[-1]
	-0.485 * prc[0] -0.317 * prc[1]  +0.216 * prc[3] + 0.216 * prc[4] + 0.128 * prc[5];
      prc =  pix[r]+1;
      rpix[r][1] = 0.243 * prc[-1]
	-0.485 * prc[0] -0.317 * prc[1]  +0.216 * prc[3] + 0.216 * prc[4] + 0.128 * prc[5];
      prc =  pix[r];
      rpix[r][0] =
	-0.243 * prc[0] -0.317 * prc[1]  +0.216 * prc[3] + 0.216 * prc[4] + 0.128 * prc[5];
      prc =  pix[r]+cols-5;

      rpix[r][cols-5] = 0.128 * prc[-5] + 0.216 * prc[-4] + 0.216 * prc[-3] -0.317  * prc[-1]
	   -0.485 * prc[0]  -0.317 * prc[1] + 0.216 * prc[3] + 0.344 * prc[4];
      prc =  pix[r]+cols-4;
      rpix[r][cols-4] = 0.128 * prc[-5] + 0.216 * prc[-4] + 0.216 * prc[-3] -0.317  * prc[-1]
	   -0.485 * prc[0]  -0.317 * prc[1] + 0.560 * prc[3];
      prc =  pix[r]+cols-3;
      rpix[r][cols-3] = 0.128 * prc[-5] + 0.216 * prc[-4] + 0.216 * prc[-3] -0.317  * prc[-1]
	   -0.485 * prc[0]  -0.317 * prc[1] + 0.560 * prc[2];
      prc =  pix[r]+cols-2;
      rpix[r][cols-2] = 0.128 * prc[-5] + 0.216 * prc[-4] + 0.216 * prc[-3] -0.317  * prc[-1]
	   -0.485 * prc[0]  + 0.243 * prc[1];
      prc =  pix[r]+cols-1;
      rpix[r][cols-1] = 0.128 * prc[-5] + 0.216 * prc[-4] + 0.216 * prc[-3] -0.317  * prc[-1]
	   -0.243 * prc[0];      
    }
}

/* Same as HorConvSqrt2, but convolve along vertical direction.
*/
void DerYYV9( DARY *image,  DARY *result)
{
  //2nd 2 =     0.128 0.216 0.216 0 -0.317 -0.485 -0.317 0 0.216 0.216 0.128
    int rows, cols, r, c;
    float **pix, **rpix;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (r = 5; r < rows - 5; r++) {
	rpix[r][c] = 0.128 * pix[r-5][c] + 0.216 * pix[r-4][c] + 0.216 * pix[r-3][c] 
	  -0.317 * pix[r-1][c]   -0.485 * pix[r][c]  -0.317 * pix[r+1][c] +
	  0.216 * pix[r+3][c]+ 0.216 * pix[r+4][c]+ 0.128 * pix[r+5][c];
      }
      r=4;
      rpix[r][c] = 0.344 * pix[r-4][c] + 0.216 * pix[r-3][c] 
	-0.317 * pix[r-1][c]   -0.485 * pix[r][c]  -0.317 * pix[r+1][c] +
	0.216 * pix[r+3][c]+ 0.216 * pix[r+4][c]+ 0.128 * pix[r+5][c];
      r=3;
      rpix[r][c] = 0.560 * pix[r-3][c] 
	-0.317 * pix[r-1][c]   -0.485 * pix[r][c]  -0.317 * pix[r+1][c] +
	0.216 * pix[r+3][c]+ 0.216 * pix[r+4][c]+ 0.128 * pix[r+5][c];
      r=2;
      rpix[r][c] = 0.560 * pix[r-2][c] 
	-0.317 * pix[r-1][c]   -0.485 * pix[r][c]  -0.317 * pix[r+1][c] +
	0.216 * pix[r+3][c]+ 0.216 * pix[r+4][c]+ 0.128 * pix[r+5][c];
      r=1;
      rpix[r][c] = 
	0.243 * pix[r-1][c]   -0.485 * pix[r][c]  -0.317 * pix[r+1][c] +
	0.216 * pix[r+3][c]+ 0.216 * pix[r+4][c]+ 0.128 * pix[r+5][c];
      r=0;
      rpix[r][c] = 
	-0.243 * pix[r][c]  -0.317 * pix[r+1][c] +
	0.216 * pix[r+3][c]+ 0.216 * pix[r+4][c]+ 0.128 * pix[r+5][c];
      r=rows-5;      
      rpix[r][c] = 0.128 * pix[r-5][c] + 0.216 * pix[r-4][c] + 0.216 * pix[r-3][c] 
	-0.317 * pix[r-1][c]   -0.485 * pix[r][c]  -0.317 * pix[r+1][c] +
	0.216 * pix[r+3][c]+ 0.344 * pix[r+4][c];
      r=rows-4;      
      rpix[r][c] = 0.128 * pix[r-5][c] + 0.216 * pix[r-4][c] + 0.216 * pix[r-3][c] 
	-0.317 * pix[r-1][c]   -0.485 * pix[r][c]  -0.317 * pix[r+1][c] +
	0.560 * pix[r+3][c];
      r=rows-3;      
      rpix[r][c] = 0.128 * pix[r-5][c] + 0.216 * pix[r-4][c] + 0.216 * pix[r-3][c] 
	-0.317 * pix[r-1][c]   -0.485 * pix[r][c]  -0.317 * pix[r+1][c] +
	0.560 * pix[r+2][c];
      r=rows-2;      
      rpix[r][c] = 0.128 * pix[r-5][c] + 0.216 * pix[r-4][c] + 0.216 * pix[r-3][c] 
	-0.317 * pix[r-1][c]   -0.485 * pix[r][c] +0.243 * pix[r+1][c];
      r=rows-1;      
      rpix[r][c] = 0.128 * pix[r-5][c] + 0.216 * pix[r-4][c] + 0.216 * pix[r-3][c] 
	-0.317 * pix[r-1][c]   -0.243 * pix[r][c];   
    }
}



 

void DerHConv6(DARY *image,  DARY *result)
{

  //1=  0.0133 0.1080 0.2420 0 -0.2420 -0.1080 -0.0133
    int rows, cols, r, c;
    float **pix, **rpix, *prc, p1,*pixr,*rpixr;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      pixr=pix[r];
      rpixr=rpix[r];
      for (c = 3; c < cols - 3; c++) {
	prc =  pixr + c;
	rpixr[c] = - 0.0133 * prc[-3] - 0.1080 * prc[-2] - 0.2420 * prc[-1] +
	   + 0.2420 * prc[1] + 0.1080 * prc[2] + 0.0133 * prc[3];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (c = 0; c < 3; c++) {
	p1 = (c < 1) ? pix[r][0] : pix[r][c-1];
	prc =  pixr + c;
	rpixr[c] = - 0.1213 * pixr[0] - 0.2420 * p1 +
	  0.2420 * prc[1] + 0.1080 * prc[2] + 0.0133 * prc[3];
      }
      for (c = cols - 3; c < cols; c++) {
	p1 = (c >= cols - 1) ? pix[r][cols-1] : pix[r][c+1];
	prc =  pixr + c;
	rpixr[c] =- 0.0133 * prc[-3] - 0.1080 * prc[-2] - 0.2420 * prc[-1] +
	   0.2420 * p1 + 0.1213 * pixr[cols-1];
      }
    }
}



/* Same as HorConvSqrt2, but convolve along vertical direction.
*/
void DerVConv6( DARY *image,  DARY *result)
{
  //1=  0.0133 0.1080 0.2420 0 -0.2420 -0.1080 -0.0133
    int rows, cols, r, c;
    float **pix, **rpix, p1;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (r = 3; r < rows - 3; r++) {
	rpix[r][c] = -0.0133 * pix[r-3][c] -0.1080 * pix[r-2][c] -
	  0.2420 * pix[r-1][c]  + 0.2420 * pix[r+1][c] +
	  0.1080 * pix[r+2][c] + 0.0133 * pix[r+3][c];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (r = 0; r < 3; r++) {
	p1 = (r < 1) ? pix[0][c] : pix[r-1][c];
	rpix[r][c] = -0.1213 * pix[0][c] - 0.2420  * p1 +
	   0.2420 * pix[r+1][c] + 0.1080 * pix[r+2][c] + 0.0133 * pix[r+3][c];
      }
      for (r = rows - 3; r < rows; r++) {
	p1 = (r >= rows - 1) ? pix[rows-1][c] : pix[r+1][c];
	rpix[r][c] = -0.0133 * pix[r-3][c] - 0.1080 * pix[r-2][c] -
	  0.2420 * pix[r-1][c]  + 0.2420 * p1 +
	  0.1213 * pix[rows-1][c];
      }
    }
}
void DerHConv8(DARY *image,  DARY *result)
{

  //1=  0.0133 0.1080 0.2420 0 -0.2420 -0.1080 -0.0133
  //2=  0.0270 0.0486 0.0605 0.0440 0   -0.0440   -0.0605   -0.0486   -0.0270
    int rows, cols, r, c;
    float **pix, **rpix, *prc,*pixr,*rpixr;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      pixr=pix[r];
      rpixr=rpix[r];
      for (c = 4; c < cols - 4; c++) {
	prc =  pixr + c;
	rpixr[c] =  - 0.0270 * prc[-4] - 0.0486 * prc[-3] - 0.0605 * prc[-2] - 0.0486 * prc[-1] +
	   + 0.0486 * prc[1] + 0.0605 * prc[2] + 0.0486 * prc[3] + 0.0270 * prc[4];
      }
      /* For pixels near boundary, use value of boundary pixel. */
	prc =  pixr + 3;
	rpixr[3] =  - 0.0605 * prc[-3] - 0.0605 * prc[-2] - 0.0486 * prc[-1] +
	  + 0.0486 * prc[1] + 0.0605 * prc[2] + 0.0486 * prc[3] + 0.0270 * prc[4];
	prc =  pixr + 2;
	rpixr[2] =  - 0.1361 * prc[-2] - 0.0486 * prc[-1] +
	  + 0.0486 * prc[1] + 0.0605 * prc[2] + 0.0486 * prc[3] + 0.0270 * prc[4];
	rpixr[1]=0;rpixr[0]=0;rpixr[cols-1]=0;rpixr[cols-2]=0;
	prc =  pixr + cols-3;
	rpixr[cols-3]= - 0.0270 * prc[-7] - 0.0486 * prc[-6] - 0.0605 * prc[-5] - 0.0486 * prc[-4] +
	   + 0.0486 * prc[-2] + 0.1361 * prc[2];
	prc =  pixr + cols-4;
        rpixr[cols-4]= - 0.0270 * prc[-4] - 0.0486 * prc[-3] - 0.0605 * prc[-2] - 0.0486 * prc[-1] +
	   + 0.0486 * prc[1] + 0.0605 * prc[2] + 0.0756 * prc[3];

    }
}

/* Same as HorConvSqrt2, but convolve along vertical direction.
*/
void DerVConv8( DARY *image,  DARY *result)
{
  //2=  0.0270 0.0486 0.0605 0.0440 0   -0.0440   -0.0605   -0.0486   -0.0270
    int rows, cols, r, c;
    float **pix, **rpix;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (r = 4; r < rows - 4; r++) {
	rpix[r][c] =  -0.0270 * pix[r-4][c] -0.0486 * pix[r-3][c] -0.0605 * pix[r-2][c] -
	  0.0440 * pix[r-1][c]  + 0.0440 * pix[r+1][c] +
	  0.0605 * pix[r+2][c] + 0.0486 * pix[r+3][c] + 0.0270 * pix[r+4][c];
      }
      /* For pixels near boundary, use value of boundary pixel. */
	rpix[3][c] =  -0.0756 * pix[0][c] -0.0605 * pix[1][c] -
	  0.0440 * pix[2][c]  + 0.0440 * pix[4][c] +
	  0.0605 * pix[5][c] + 0.0486 * pix[6][c] + 0.0270 * pix[7][c];
	rpix[2][c] =  -0.1361 * pix[0][c] -
	  0.0440 * pix[1][c]  + 0.0440 * pix[3][c] +
	  0.0605 * pix[4][c] + 0.0486 * pix[5][c] + 0.0270 * pix[6][c];
	rpix[2][c] =  -0.1361 * pix[0][c] -
	  0.0440 * pix[1][c]  + 0.0440 * pix[3][c] +
	  0.0605 * pix[4][c] + 0.0486 * pix[5][c] + 0.0270 * pix[6][c];
	rpix[1][c]=0;rpix[0][c]=0;
	rpix[rows-1][c]=0;rpix[rows-2][c]=0;
	rpix[rows-3][c]= -0.0270 * pix[rows-7][c] -0.0486 * pix[rows-6][c] -0.0605 * pix[rows-5][c] -
	  0.0440 * pix[rows-4][c]  + 0.0440 * pix[rows-2][c] +
	  0.1361 * pix[rows-1][c];
	rpix[rows-4][c]= -0.0270 * pix[rows-8][c] -0.0486 * pix[rows-7][c] -0.0605 * pix[rows-6][c] -
	  0.0440 * pix[rows-5][c]  + 0.0440 * pix[rows-3][c] +
	  0.0605 * pix[rows-2][c] + 0.0756 * pix[rows-1][c];

    }
    
}





void DerVConv4(DARY *image,  DARY *result)
{
  //1=   0.1080 0.2420 0 -0.2420 -0.1080 
    int rows, cols, r, c;
    float **pix, **rpix, p1;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (r = 1; r < rows - 2; r++) {
	rpix[r][c] = -0.1213 * pix[r-1][c] -
	  0.2420 * pix[r][c]  + 0.2420 * pix[r+1][c] +
	  0.1213 * pix[r+2][c];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (r = 0; r < 1; r++) {
	p1 = (r < 1) ? pix[0][c] : pix[1][c];
	rpix[r][c] = -0.1213 * pix[0][c] - 0.2420  * p1 +
	   0.2420 * pix[r+1][c] + 0.1213 * pix[r+2][c];
      }
      for (r = rows - 2; r < rows; r++) {
	p1 = (r >= rows - 1) ? pix[rows-1][c] : pix[rows-2][c];
	rpix[r][c] = - 0.1213 * pix[r-1][c] -
	  0.2420 * pix[r][c]  + 0.2420 * p1 +
	   0.1213  * pix[rows-1][c];
      }
    }
}

void DerHConv4(DARY *image,  DARY *result)
{

  //1=  0.1213 0.2420 0 -0.2420 -0.1213
    int rows, cols, r, c;
    float **pix, **rpix, *prc, p1,*pixr,*rpixr;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      pixr=pix[r];
      rpixr=rpix[r];
      for (c = 1; c < cols - 2; c++) {
	prc =  pixr + c;
	rpixr[c] = - 0.1213 * prc[-1] - 0.2420 * prc[0] +
	   + 0.2420 * prc[1] + 0.1213 * prc[2];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (c = 0; c < 1; c++) {
	p1 = (c < 1) ? pix[r][0] : pix[r][1];
	prc =  pixr + c;
	rpixr[c] = - 0.1213 * pixr[0] - 0.2420 * p1 +
	  0.2420 * prc[1] + 0.1213 * prc[2];
      }
      for (c = cols - 2; c < cols; c++) {
	p1 = (c >= cols - 1) ? pix[r][cols-1] : pix[r][cols-2];
	prc =  pixr + c;
	rpixr[c] =- 0.1213 * prc[-1] - 0.2420 * prc[0] +
	  0.2420 * p1 + 0.1213 * pixr[cols-1];
      }
    }
}


void VerConv5( DARY *image,  DARY *result)
{

  //1.2 = 0.0856 0.2427 0.3434 0.2427 0.0856
  //1.0 = 0.0545 0.2442 0.4026 0.2442 0.0545
  //0.8 = 0.0219 0.2285 0.4992 0.2285 0.0219
  //0.7 = 0.0096 0.2054 0.5699 0.2054 0.0096
    int rows, cols, r, c;
    float **pix, **rpix, p1;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (r = 2; r < rows - 2; r++) {
	rpix[r][c] = 0.0545 * pix[r-2][c] + 0.2442 * pix[r-1][c] +
	  0.4026 * pix[r][c] + 0.2442 * pix[r+1][c] + 0.0545 * pix[r+2][c];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (r = 0; r < 2; r++) {
	p1 = (r < 1) ? pix[0][c] : pix[r-1][c];
	rpix[r][c] = 0.0545 * pix[0][c] + 0.2442 * p1 + 0.4026 * pix[r][c] +
	  0.2442 * pix[r+1][c] + 0.0545 * pix[r+2][c];
      }
      for (r = rows - 2; r < rows; r++) {
	p1 = (r >= rows - 1) ? pix[rows-1][c] : pix[r+1][c];
	rpix[r][c] = 0.0545 * pix[r-2][c] + 0.2442 * pix[r-1][c] +
	  0.4026 * pix[r][c] +  0.2442 * p1 +
	  0.0545 * pix[rows-1][c];
      }
    }
}


void HorConv5(DARY *image,  DARY *result)
{

  //1.2 = 0.0856 0.2427 0.3434 0.2427 0.0856
  //1.0 = 0.0545 0.2442 0.4026 0.2442 0.0545
  //0.8 = 0.0219 0.2285 0.4992 0.2285 0.0219
  //0.7 = 0.0096 0.2054 0.5699 0.2054 0.0096
    int rows, cols, r, c;
    float **pix, **rpix, *prc, p1,*pixr,*rpixr;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;

    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      pixr=pix[r];
      rpixr=rpix[r];
      for (c = 2; c < cols - 2; c++) {
	prc =  pixr + c;
	rpixr[c] = 0.0545 * prc[-2] + 0.2442 * prc[-1] +
	  0.4026 * prc[0] + 0.2442 * prc[1] + 0.0545 * prc[2];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (c = 0; c < 2; c++) {
	p1 = (c < 1) ? pixr[0] : pixr[c-1];
	prc =  pixr + c;
	rpixr[c] = 0.0545 * pixr[0] + 0.2442 * p1 +
	 0.4026  * prc[0] + 0.2442 * prc[1] + 0.0545 * prc[2];
      }
      
      for (c = cols - 2; c < cols; c++) {
	p1 = (c >= cols - 1) ? pixr[cols-1] : pixr[c+1];
	prc =  pixr + c;
	rpixr[c] =  0.0545 * prc[-2] + 0.2442 * prc[-1] +
	  0.4026 * prc[0] + 0.2442 * p1 + 0.0545 * pixr[cols-1];
      }
    }
}

void VerConv3( DARY *image,  DARY *result)
{
  //0.8 = 0.0219 0.2285 0.4992 0.2285 0.0219
  //0.7 = 0.0096 0.2054 0.5699 0.2054 0.0096
  //0.6 = 0.1664 0.6672 0.1664
  //0.4 = 0.1664 0.8 0.1

    int rows, cols, r, c,rows1;
    float **pix, **rpix;

    rows = image->y();
    cols = image->x();
    pix = image->fel;
    rpix = result->fel;
    rows1=rows-1;
    for (c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (r = 1; r < rows - 1; r++) {
	rpix[r][c] =  0.1664 * pix[r-1][c] + 0.6672 * pix[r][c] + 0.1664 * pix[r+1][c];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      rpix[0][c] =  0.8336 * pix[0][c] + 0.1664 * pix[1][c];
      
      rpix[rows1][c] = 0.1664 * pix[rows-2][c] + 0.8336 * pix[rows1][c];
      
    }
}

void HorConv3(DARY *image,  DARY *result)
{

  //0.8 = 0.0219 0.2285 0.4992 0.2285 0.0219
  //0.7 = 0.0096 0.2054 0.5699 0.2054 0.0096
  //0.6 = 0.1664 0.6672 0.1664
    int rows, cols, r, c,cols1;
    float **pix, **rpix, *prc;

    rows = image->y();
    cols = image->x();
    pix = image->fel; 
    rpix = result->fel;
    cols1=cols-1;

    for (r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (c = 1; c < cols - 1; c++) {
	prc =  pix[r] + c;
	rpix[r][c] =  0.1664 * prc[-1] + 0.6672 * prc[0] + 0.1664 * prc[1];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      rpix[r][0] = 0.8336 * pix[r][0] + 0.1664 * pix[r][1];
      
      rpix[r][cols1] =   0.1664* pix[r][cols1-1] + 0.8336 * pix[r][cols1];      
    }
}
 

     void  smoothSqrt(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	HorConvSqrt2(image_in,image_tmp);
	VerConvSqrt2(image_tmp,smooth_image);
	delete image_tmp;
    }     
     void  smooth5(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	HorConv5(image_in,image_tmp);
	VerConv5(image_tmp,smooth_image);
	delete image_tmp;
     }     
     void  smooth3(DARY* image_in, DARY* smooth_image){
       DARY* image_tmp = new DARY(image_in->y(),image_in->x());
       HorConv3(image_in,image_tmp);
       VerConv3(image_tmp,smooth_image);
       delete image_tmp;
     }     

     void  dX2(DARY* image_in, DARY* dximage){
	float *dx_y0;
	float *dx_ym;
	uint endx=image_in->x();
	uint endy=image_in->y()-1;
	dx_y0=dximage->fel[0];
	dx_ym=dximage->fel[endy];
	for(uint x=0;x<endx;x++){
	  dx_y0[x]=0;
	  dx_ym[x]=0;
	}
	endx--;	   
        for(uint y=1;y<endy;y++){
	  dx_ym=dximage->fel[y];
	  dx_y0=image_in->fel[y];
	  dx_ym[0]=0;
	  dx_ym[endx]=0;
	  for(uint x=1;x<endx;x++){
	    dx_ym[x]=dx_y0[x+1]-dx_y0[x-1];	   
	  } 
       } 
     }       
      void dY2(DARY* image_in, DARY* dyimage){
	uint endx=image_in->x()-1;
	uint endy=image_in->y();
	
        for(uint y=0;y<endy;y++){
	  dyimage->fel[y][0]=0;
	  dyimage->fel[y][endx]=0;
	}
	endy--; 
        for(uint x=1;x<endx;x++){
	  dyimage->fel[0][x]=0;
	  dyimage->fel[endy][x]=0;
	  for(uint y=1;y<endy;y++){
	    dyimage->fel[y][x]=image_in->fel[y+1][x]-image_in->fel[y-1][x];	   
	  }
	}
      }     







      void  dXX5(DARY* image_in, DARY* dximage){
	for(uint x=0;x<image_in->x();x++){
	  dximage->fel[0][x]=0;
	  dximage->fel[1][x]=0;
	  dximage->fel[image_in->y()-1][x]=0;
	  dximage->fel[image_in->y()-2][x]=0;
	}
	for(uint y=0;y<image_in->y();y++){
	  dximage->fel[y][0]=0;
	  dximage->fel[y][1]=0;
	  dximage->fel[y][image_in->x()-1]=0;
	  dximage->fel[y][image_in->x()-2]=0;
	}	   
	   
        for(uint y=2;y<image_in->y()-2;y++){
	  for(uint x=2;x<image_in->x()-2;x++){
	    dximage->fel[y][x]=-2*image_in->fel[y][x]+image_in->fel[y][x+2]+image_in->fel[y][x-2];	   
	  } 
	} 
      }      
      void dYY5(DARY* image_in, DARY* dyimage){
	for(uint x=0;x<image_in->x();x++){
	  dyimage->fel[0][x]=0;
	  dyimage->fel[1][x]=0;
	  dyimage->fel[image_in->y()-1][x]=0;
	  dyimage->fel[image_in->y()-2][x]=0;
	}
	for(uint y=0;y<image_in->y();y++){
	  dyimage->fel[y][0]=0;
	  dyimage->fel[y][1]=0;
	  dyimage->fel[y][image_in->x()-1]=0;
	  dyimage->fel[y][image_in->x()-2]=0;
	}	   
        for(uint x=2;x<image_in->x()-2;x++){
	  for(uint y=2;y<image_in->y()-2;y++){
	    dyimage->fel[y][x]=-2*image_in->fel[y][x]+image_in->fel[y+2][x]+image_in->fel[y-2][x];	   
	  }
	}
      }     

      void  dXX3(DARY* image_in, DARY* dximage){
	for(uint x=0;x<image_in->x();x++){
	  dximage->fel[0][x]=0;
	  dximage->fel[image_in->y()-1][x]=0;
	}
	   
        for(uint y=1;y<image_in->y()-1;y++){
	  dximage->fel[y][0]=0;dximage->fel[y][image_in->x()-1]=0;
	    dximage->fel[y][1]=image_in->fel[y][0]-2*image_in->fel[y][1]+image_in->fel[y][2];
	    dximage->fel[y][image_in->x()-2]=
	      image_in->fel[y][image_in->x()-1]-2*image_in->fel[y][image_in->x()-2]+image_in->fel[y][image_in->x()-3];
	  for(uint x=1;x<image_in->x()-1;x++){
	    dximage->fel[y][x]=-2*image_in->fel[y][x]+image_in->fel[y][x+1]+image_in->fel[y][x-1];	   
	  } 
	} 
      }      
      void dYY3(DARY* image_in, DARY* dyimage){
	for(uint y=0;y<image_in->y();y++){
	  dyimage->fel[y][0]=0;
	  dyimage->fel[y][image_in->x()-1]=0;
	}	   
        for(uint x=1;x<image_in->x()-1;x++){
	  dyimage->fel[0][x]=0;dyimage->fel[image_in->y()-1][x]=0;	  
	  dyimage->fel[1][x]=image_in->fel[0][x]-2*image_in->fel[1][x]+image_in->fel[2][x];
	  dyimage->fel[image_in->y()-2][x]=
	    image_in->fel[image_in->y()-3][x]-2*image_in->fel[image_in->y()-2][x]+image_in->fel[image_in->y()-1][x];
	  for(uint y=1;y<image_in->y()-1;y++){
	    dyimage->fel[y][x]=-2*image_in->fel[y][x]+image_in->fel[y+1][x]+image_in->fel[y-1][x];	   
	  }
	}
      }     



      void dXX_YY3(DARY* image_in, DARY* dyimage){
	DARY *tmp = new DARY(image_in->y(),image_in->x());
	dXX3(image_in,tmp);
	dYY3(image_in,dyimage);
        for(uint x=0;x<image_in->x();x++){
	  for(uint y=0;y<image_in->y();y++){
	    dyimage->fel[y][x]+=tmp->fel[y][x];	   
	  }
	}
	delete tmp;
      }     

     void  dX6(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerHConv6(image_in,image_tmp);
	VerConv5(image_tmp,smooth_image);
	delete image_tmp;
      }     
     void  dY6(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerVConv6(image_in,image_tmp);
	HorConv5(image_tmp,smooth_image);
	delete image_tmp;
    }     

     void  dX4(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerHConv4(image_in,image_tmp);
	VerConv3(image_tmp,smooth_image);
	delete image_tmp;
     } 

     void  dY4(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerVConv4(image_in,image_tmp);
	HorConv3(image_tmp,smooth_image);
	delete image_tmp;
    }     

     void  dYY7(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerYYV7(image_in,image_tmp);
	HorConv5(image_tmp,smooth_image);
	delete image_tmp;
    }     
     void  dXX7(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerXXH7(image_in,image_tmp);
	VerConv5(image_tmp,smooth_image);
	delete image_tmp;
    }     
     void  dXY7(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerVConv6(image_in,image_tmp);
	DerHConv6(image_tmp,smooth_image);
	delete image_tmp;
    }    
 
      void  dYY9(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerYYV9(image_in,image_tmp);
	smoothXXH9(image_tmp,smooth_image);
	delete image_tmp;
    }     
     void  dXX9(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerXXH9(image_in,image_tmp);
	smoothYYV9(image_tmp,smooth_image);
	delete image_tmp;
    }     

     void  smooth9(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	smoothXXH9(image_in,image_tmp);
	smoothYYV9(image_tmp,smooth_image);
	delete image_tmp;
    }     
 
     void  dXY9(DARY* image_in, DARY* smooth_image){
        DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DerVConv8(image_in,image_tmp);
	DerHConv8(image_tmp,smooth_image);
	delete image_tmp;
    }     
        
        

 
     void  smooth(DARY* image_in, DARY* smooth_image, float scale){
	
	DARY* image_tmp = new DARY(image_in->y(),image_in->x());

	float* coefs = new float[17];
	set_smooth_coefficients (scale, coefs);
	horizontal(image_in,image_tmp, coefs);
	vertical  (image_tmp,smooth_image, coefs);
	delete []coefs;delete image_tmp;
    }     
     void  smooth(DARY* image_in, DARY* smooth_image, float scalex, float scaley){
	
	DARY* image_tmp = new DARY(image_in->y(),image_in->x());

	float* coefs = new float[17];
	set_smooth_coefficients (scaley, coefs);
	horizontal(image_in,image_tmp, coefs);
	set_smooth_coefficients (scalex, coefs);
	vertical  (image_tmp,smooth_image, coefs);
	delete []coefs;delete image_tmp;
    }     
     void dX (DARY* image_in, DARY* x_image, float scale){
	
	DARY* image_tmp = new DARY(image_in->y(),image_in->x());

	float* coefs = new float[17];

	set_smooth_coefficients (scale, coefs);
	horizontal (image_in,image_tmp,coefs);
	set_derive_coefficients (scale, coefs);
	vertical   (image_tmp,x_image,coefs);
	delete []coefs;delete image_tmp;
  }
     void dY (DARY* image_in,DARY*  y_image, float scale){
	
	DARY* image_tmp = new DARY(image_in->y(),image_in->x());

	float* coefs = new float[17];
	
	set_smooth_coefficients (scale, coefs);
	vertical (image_in,image_tmp,coefs);
	set_derive_coefficients (scale, coefs);
        horizontal (image_tmp,y_image,coefs);
	delete []coefs;delete image_tmp;
  }

    void dXY (DARY* image_in, DARY* xy_image, float scale){
	
	DARY* image_tmp = new DARY(image_in->y(),image_in->x());

	float* coefs = new float[17];

	set_derive_coefficients (scale, coefs);
	vertical (image_in,image_tmp,coefs);
        horizontal (image_tmp,xy_image,coefs);
	delete []coefs;delete image_tmp;
    }
     void dYY (DARY* image_in, DARY* yy_image, float scale){
	
	DARY* image_tmp = new DARY(image_in->y(),image_in->x());

	float* coefs = new float[17];

	set_smooth_coefficients (scale,coefs);
	vertical (image_in,image_tmp,coefs);
	set_second_derive_coefficients (scale,coefs);
        horizontal (image_tmp,yy_image,coefs);
	delete []coefs;delete image_tmp;

    }
     void dXX (DARY* image_in, DARY* xx_image, float scale){

	DARY* image_tmp = new DARY(image_in->y(),image_in->x());

	float* coefs = new float[17];

	set_smooth_coefficients (scale,coefs);	
	horizontal (image_in,image_tmp,coefs);
	set_second_derive_coefficients (scale,coefs);
        vertical (image_tmp,xx_image,coefs);
	delete []coefs;delete image_tmp;

    }
     void dXX_YY(DARY* image_in, DARY* xx_image, float scale){

	DARY* image_tmp = new DARY(image_in->y(),image_in->x());
	DARY* image_tmp2 = new DARY(image_in->y(),image_in->x());

	float* coefs = new float[17];

	//set_smooth_coefficients (scale,coefs);	
	//horizontal (image_in,image_tmp,coefs);
	//vertical (image_in,image_tmp2,coefs);
	set_second_derive_coefficients (scale,coefs);
        vertical (image_in,image_tmp,coefs);
	horizontal (image_in,image_tmp2,coefs);
	set_smooth_coefficients (scale,coefs);	
	horizontal (image_tmp,xx_image,coefs);
	vertical (image_tmp2,image_tmp,coefs);
	for(uint j=0;j<image_tmp->y();j++){
	  for(uint i=0;i<image_tmp->x();i++){
	    xx_image->fel[j][i]=(xx_image->fel[j][i]+image_tmp->fel[j][i]);
	  }
	}
	delete []coefs;delete image_tmp;delete image_tmp2;
    }

     float smooth (int x, int y, DARY* image_in, float scale){       	
	DARY* image = new DARY(2*size+1,2*size+1);
	DARY* image_tmp = new DARY(2*size+1,2*size+1);

	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		image->fel[i+size][j+size]=image_in->fel[y+i][x+j];
	    }
	

	float* coefs = new float[17];
	set_smooth_coefficients (scale,coefs);	
	
	horizontal (image,image_tmp,coefs);
        vertical (image_tmp,image,coefs);
       
	float ret=image->fel[size][size];
	delete []coefs;delete image_tmp;delete image;
	
	return ret;
    }

     float dX (int x, int y, DARY* image_in, float scale){		
	DARY* image = new DARY(2*size+1,2*size+1);
	DARY* image_tmp = new DARY(2*size+1,2*size+1);

	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		image->fel[i+size][j+size]=image_in->fel[y+i][x+j];
	    }
	
 	float* coefs = new float[17];
	set_smooth_coefficients (scale,coefs);	
	horizontal (image,image_tmp,coefs);
	set_derive_coefficients (scale,coefs);	
        vertical (image_tmp,image,coefs);
	
	float ret=image->fel[size][size];
	delete []coefs;delete image_tmp;delete image;
	
	return ret;
    }

 
float smooth(int x, int y, DARY* image_in, float scalex, float scaley){

        uint sizex=(uint)rint(GAUSS_CUTOFF*scalex);
	uint sizey=(uint)rint(GAUSS_CUTOFF*scaley);
	if(sizex>((image_in->x()-1)>>1))sizex=((image_in->x()-1)>>1);
	if(sizey>((image_in->y()-1)>>1))sizey=((image_in->y()-1)>>1);
	if((2*sizex+1)>image_in->x() ||(2*sizey+1)>image_in->y()){
	  cout << "smoothing scale bigger than image"<< endl;exit(0);
	}
	float **table_exp_aff;
	table_exp_aff = new float*[(2*sizey+1)];
	table_exp_aff[0]=new float[(2*sizey+1)*(2*sizex+1)];
	for(uint i=1;i<(2*sizey+1);i++)table_exp_aff[i]=table_exp_aff[0]+i*(2*sizex+1);  
 
	
	float h_square_scalex = (-2)* scalex * scalex;
	float h_square_scaley = (-2)* scaley * scaley;
       	for (uint i=0;i<=sizey;i++)
	  for (uint j=0;j<=sizex;j++){
	    table_exp_aff[i+sizey][-j+sizex] = table_exp_aff[-i+sizey][j+sizex] = 
	      table_exp_aff[i+sizey][j+sizex] = table_exp_aff[-i+sizey][-j+sizex] = 
	      exp(((i*i)/h_square_scaley)+((j*j)/(h_square_scalex)));   
	  }
	float totals = 0;
	for (int i=-sizey;i<=(int)sizey;i++)
	  for (int j=-sizex;j<=(int)sizex;j++)totals+=table_exp_aff[i+sizey][j+sizex];

	//       	write(table_exp,"gauss_test.mat");
      	//cout <<"total "<< total<< endl;//getchar();
	/*
	DARY * im=new DARY(2*sizey+1,2*sizex+1);
	for (int i=0;i<2*sizey+1;i++)
	    for (int j=0;j<2*sizex+1;j++){
	      im->fel[i][j]=table_exp_aff[i][j];
	    }
	im->normalize(0,2);
	im->write("gauss.pgm");
	cout <<"OK2"<< endl;getchar();
	*/

	float value = 0;	
	//if(x<size || y<size || x+size>=image_in->x() || y+size>=image_in->y())return(value);
	for (int i=-sizey;i<=(int)sizey;i++)
	    for (int j=-sizex;j<=(int)sizex;j++){
		value += image_in->fel[y+i][x+j] * table_exp_aff[i+sizey][j+sizex];
	    }

	delete [] table_exp_aff[0];delete [] table_exp_aff;
	return (value/totals);

}

    void dX (DARY* image_in,  DARY* image_out, float scalex, float scaley){		
	DARY* image_tmp = new DARY(image_out->y(),image_out->x());
	float* coefs = new float[17];
	set_derive_coefficients (scalex,coefs);	
        vertical (image_in,image_tmp,coefs);
	set_smooth_coefficients (scaley,coefs);	
	horizontal (image_tmp,image_out,coefs);
	delete []coefs;delete image_tmp;	
    }

     void dY (DARY* image_in, DARY* image_out, float scalex, float scaley){
	DARY* image_tmp = new DARY(image_out->y(),image_out->x());
	float* coefs = new float[17];
	set_smooth_coefficients (scalex,coefs);	
	vertical (image_in,image_tmp,coefs);
	set_derive_coefficients (scaley,coefs);	
        horizontal (image_tmp,image_out,coefs);
	delete []coefs;delete image_tmp;
     }


     float dY (int x, int y, DARY* image_in, float scale){
	DARY* image = new DARY(2*size+1,2*size+1);
	DARY* image_tmp = new DARY(2*size+1,2*size+1);

	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		image->fel[i+size][j+size]=image_in->fel[y+i][x+j];
	    }
	
	float* coefs = new float[17];
	set_smooth_coefficients (scale,coefs);	
	vertical (image,image_tmp,coefs);
	set_derive_coefficients (scale,coefs);	
        horizontal (image_tmp,image,coefs);

	float ret=image->fel[size][size];
	delete []coefs;delete image_tmp;delete image;
	
	return ret;
    }

     float dXY (int x, int y, DARY* image_in, float scale){
	DARY* image = new DARY(2*size+1,2*size+1);
	DARY* image_tmp = new DARY(2*size+1,2*size+1);

	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		image->fel[i+size][j+size]=image_in->fel[y+i][x+j];
	    }
	
	float* coefs = new float[17];
	set_derive_coefficients (scale,coefs);	
	vertical (image,image_tmp,coefs);
        horizontal (image_tmp,image,coefs);

	float ret=image->fel[size][size];
	delete []coefs;delete image_tmp;delete image;
	
	return ret;


    }
      float dXX (int x, int y, DARY* image_in, float scale){
	DARY* image = new DARY(2*size+1,2*size+1);
	DARY* image_tmp = new DARY(2*size+1,2*size+1);

	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		image->fel[i+size][j+size]=image_in->fel[y+i][x+j];
	    }
	
	float* coefs = new float[17];
	set_smooth_coefficients (scale,coefs);	
        horizontal (image,image_tmp,coefs);
	set_second_derive_coefficients (scale,coefs);	
	vertical (image_tmp,image,coefs);

	float ret=image->fel[size][size];
	delete []coefs;delete image_tmp;delete image;
	
	return ret;
    }

    float dYY (int x, int y, DARY* image_in, float scale){
	DARY* image = new DARY(2*size+1,2*size+1);
	DARY* image_tmp = new DARY(2*size+1,2*size+1);

	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		image->fel[i+size][j+size]=image_in->fel[y+i][x+j];
	    }
	
	float* coefs = new float[17];
	set_smooth_coefficients (scale,coefs);	
        vertical (image,image_tmp,coefs);
	set_second_derive_coefficients (scale,coefs);	
	horizontal (image_tmp,image,coefs);

	float ret=image->fel[size][size];
	delete []coefs;delete image_tmp;delete image;
	
	return ret;
    }



 












      void write(float** gauss, const char *filename){
	  ofstream out(filename);
	  if(!out){cout << "not OK\n";return;}
	  for (int i=-size;i<=size;i++){
	      for (int j=-size;j<=size;j++){
		  out << gauss[i+size][j+size]<< " ";
	      }
	      out << endl;
	  }
	  out.close();
      }

      void initGauss(float scale){

	if(scale==g_scale)return;
	g_scale=scale;
	size=(int)rint(GAUSS_CUTOFF*g_scale);
	if(table_exp!=NULL){delete [] table_exp[0];delete [] table_exp;}
	table_exp = new float*[(2*size+1)];
	table_exp[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp[i]=table_exp[0]+i*(2*size+1);  
	float square_scale = g_scale * g_scale;
	float h_square_scale = (-2)*square_scale;
	total = 0;
 
	
	int size_loc=(int)rint(5*g_scale);
	float *g_loc=new float[size_loc+1];
	for (int i=0;i<=size_loc;i++){
	    g_loc[i]=exp(((i*i)/(h_square_scale)));	
	}
	for (int i=1;i<=(size_loc-size);i++){
	    g_loc[size-i]+=g_loc[size+i];
	}
	for (int i=0;i<=size;i++)
	    for (int j=0;j<=size;j++){
		table_exp[i+size][-j+size] = table_exp[-i+size][j+size] = 
		    table_exp[i+size][j+size] = table_exp[-i+size][-j+size] =
		  g_loc[i]*g_loc[j];
	    } 
	/*	for (int i=0;i<=size;i++)
		for (int j=0;j<=size;j++){
		table_exp[i+size][-j+size] = table_exp[-i+size][j+size] = 
		table_exp[i+size][j+size] = table_exp[-i+size][-j+size] = 
		exp(((i*i+j*j)/(h_square_scale)));   
	  }
	*/
	/*for (int i=-size;i<=size;i++)
	  for (int j=-size;j<=size;j++){
	  table_exp[i+size][j+size] =
	  exp(((i*i+j*j)/(h_square_scale)));   		
	  }*/
	for (int i=-size;i<=size;i++)
	  for (int j=-size;j<=size;j++)total+=table_exp[i+size][j+size];
	delete []g_loc;
	//write(table_exp,"gauss_test.mat");
      	//cout <<"total "<< total<< endl;//getchar();
    }
    
      void initGaussX(float scale){
	initGauss(scale);

	if(scale==x_scale)return;
	x_scale=scale;
	if(table_exp_x!=NULL){delete [] table_exp_x[0];delete [] table_exp_x;}

	table_exp_x = new float*[(2*size+1)];
	table_exp_x[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_x[i]=table_exp_x[0]+i*(2*size+1);  
	
	float square_scale = x_scale * x_scale;

	/*	for (int i=-size;i<=size;i++)
		for (int j=-size;j<=size;j++)
 		table_exp_x[j+size][i+size] = 
		(-i/(square_scale))*table_exp[j+size][i+size];*/
	for (int i=0;i<=size;i++)
	  for (int j=0;j<=size;j++){
	    table_exp_x[-j+size][i+size] = 
	    table_exp_x[j+size][i+size] = 
	      (-i/(square_scale))*table_exp[j+size][i+size];
	    table_exp_x[-j+size][-i+size] = 
	    table_exp_x[j+size][-i+size] = 
	      -1*table_exp_x[j+size][i+size]; 
	  }
	sum_x=0;
	for (int i=0;i<=size;i++)
		sum_x+=table_exp_x[size][i];
	//write(table_exp_x,"gaussX_test.mat");
    }

    void initGaussXX(float scale){
	initGauss(scale);
	if(scale==xx_scale)return;
	xx_scale=scale;
	if(table_exp_xx!=NULL){delete [] table_exp_xx[0];delete [] table_exp_xx;}

	table_exp_xx = new float*[(2*size+1)];
	table_exp_xx[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_xx[i]=table_exp_xx[0]+i*(2*size+1); 
	
	float square_scale = xx_scale * xx_scale;
	float mo_square_scale=((-1/(square_scale)));
	float square_square_scale=square_scale*square_scale;
	
	/*	for (int i=-size;i<=size;i++)
		for (int j=-size;j<=size;j++)table_exp_xx[j+size][i+size] =  
		(mo_square_scale+(i*i/(square_square_scale)))* 
		table_exp[j+size][i+size];
	*/
	for (int i=0;i<=size;i++)
	  for (int j=0;j<=size;j++){
	    table_exp_xx[-j+size][i+size] =  
	    table_exp_xx[-j+size][-i+size] =  
	    table_exp_xx[j+size][-i+size] =  
	    table_exp_xx[j+size][i+size] =  
	      (mo_square_scale+(i*i/(square_square_scale)))* 
	      table_exp[j+size][i+size];
	  }
	//       	write(table_exp_xx,"gaussXX_test.mat");
      }
    
      void initGaussXY(float scale){
	initGauss(scale);
	if(scale==xy_scale)return;
	xy_scale=scale;
	
	if(table_exp_xy!=NULL){delete [] table_exp_xy[0];delete [] table_exp_xy;}

	table_exp_xy = new float*[(2*size+1)];
	table_exp_xy[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_xy[i]=table_exp_xy[0]+i*(2*size+1); 

	float square_scale = xy_scale * xy_scale;
	float square_square_scale=square_scale*square_scale;

	/*	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++)table_exp_xy[j+size][i+size] =  
					     (i*j/(square_square_scale))*table_exp[j+size][i+size];
	*/
	for (int i=0;i<=size;i++)
	    for (int j=0;j<=size;j++){
	      table_exp_xy[-j+size][-i+size] =  
	      table_exp_xy[j+size][i+size] =  
		(i*j/(square_square_scale))*table_exp[j+size][i+size];
	      table_exp_xy[j+size][-i+size] =  
	      table_exp_xy[-j+size][i+size] =  
		-1*table_exp_xy[j+size][i+size]; 
	    }
	//	write(table_exp_xy,"gaussXY_test.mat");
    }
 
     void initGaussLap(float scale){
	initGaussXX(scale);
	if(scale==lap_scale)return;
	lap_scale=scale;
	if(table_exp_lap!=NULL){delete [] table_exp_lap[0];delete [] table_exp_lap;}
	table_exp_lap = new float*[(2*size+1)];
	table_exp_lap[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_lap[i]=table_exp_lap[0]+i*(2*size+1); 

	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++)table_exp_lap[j+size][i+size] =  
					     (table_exp_xx[i+size][j+size]+table_exp_xx[j+size][i+size]);

	//	write(table_exp_lap,"gaussLAP_test.mat");
    }

    void initGaussXXX(float scale){
	initGauss(scale);
	if(scale==xxx_scale)return;
	xxx_scale=scale;

	if(table_exp_xxx!=NULL){delete [] table_exp_xxx[0];delete [] table_exp_xxx;}
	table_exp_xxx = new float*[(2*size+1)];
	table_exp_xxx[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_xxx[i]=table_exp_xxx[0]+i*(2*size+1); 
	
	float square_scale = xxx_scale * xxx_scale;
	float square_square_scale=square_scale*square_scale;
	
	/*	for (int i=-size;i<=size;i++)
		for (int j=-size;j<=size;j++)table_exp_xxx[j+size][i+size] =  
		(i/square_square_scale)*(3-((i*i)/(square_scale)))*
		table_exp[j+size][i+size];
	*/
	for (int i=0;i<=size;i++)
	  for (int j=0;j<=size;j++){
	    table_exp_xxx[-j+size][i+size] =  
	    table_exp_xxx[j+size][i+size] =  
	      (i/square_square_scale)*(3-((i*i)/(square_scale)))*
	      table_exp[j+size][i+size];
	    table_exp_xxx[j+size][-i+size] =  
	    table_exp_xxx[-j+size][-i+size] =  
	      -1*table_exp_xxx[j+size][i+size]; 
	    
	  }
	//	write(table_exp_xxx,"gaussXXX_test.mat");
      }
    void initGaussXXY(float scale){
	initGauss(scale);
	if(scale==xxy_scale)return;
	xxy_scale=scale;

	if(table_exp_xxy!=NULL){delete [] table_exp_xxy[0];delete [] table_exp_xxy;}
	table_exp_xxy = new float*[(2*size+1)];
	table_exp_xxy[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_xxy[i]=table_exp_xxy[0]+i*(2*size+1); 
	
	float square_scale = xxy_scale * xxy_scale;
	float square_square_scale=square_scale*square_scale;
	
	/*	for (int i=-size;i<=size;i++)
		for (int j=-size;j<=size;j++)table_exp_xxy[j+size][i+size] = 
		(j/square_square_scale)*(1-((i*i)/(square_scale)))*
		table_exp[j+size][i+size];
	*/
	for (int i=0;i<=size;i++)
	  for (int j=0;j<=size;j++){
	    table_exp_xxy[j+size][-i+size] = 
	    table_exp_xxy[j+size][i+size] = 
	      (j/square_square_scale)*(1-((i*i)/(square_scale)))*
	      table_exp[j+size][i+size];
	    table_exp_xxy[-j+size][-i+size] = 
	    table_exp_xxy[-j+size][i+size] = 
	      -1*table_exp_xxy[j+size][i+size];
	  }
	//	write(table_exp_xxy,"gaussXXY_test.mat");
      }

    void initGaussXXXX(float scale){
	initGauss(scale);
	if(scale==xxxx_scale)return;
	xxxx_scale=scale;

	if(table_exp_xxxx!=NULL){delete [] table_exp_xxxx[0];delete [] table_exp_xxxx;}
	table_exp_xxxx = new float*[(2*size+1)];
	table_exp_xxxx[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_xxxx[i]=table_exp_xxxx[0]+i*(2*size+1); 
	
	float square_scale = xxxx_scale * xxxx_scale;
	float o_square_square_scale=(1/(square_scale*square_scale));
	
	for (int i=-size;i<=size;i++)
	  for (int j=-size;j<=size;j++)table_exp_xxxx[j+size][i+size] = 
					 (o_square_square_scale*(3-(((i*i)/square_scale)*
								    (6-((i*i)/square_scale)))))*
					 table_exp[j+size][i+size];
	//		write(table_exp_xxxx,"gaussXXXX_test.mat");
      }
    
    void initGaussXXXY(float scale){
	initGauss(scale);
	if(scale==xxxy_scale)return;
	xxxy_scale=scale;

	if(table_exp_xxxy!=NULL){delete [] table_exp_xxxy[0];delete [] table_exp_xxxy;}
	table_exp_xxxy = new float*[(2*size+1)];
	table_exp_xxxy[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_xxxy[i]=table_exp_xxxy[0]+i*(2*size+1); 
	
	float square_scale = xxxy_scale * xxxy_scale;
	float square_square_square_scale=square_scale*square_scale*square_scale;
	
	for (int i=-size;i<=size;i++)
	  for (int j=-size;j<=size;j++)table_exp_xxxy[j+size][i+size] = 
                                     ((j*i)/square_square_square_scale)*(((i*i)/square_scale)-3)*
                                     table_exp[j+size][i+size];
	//		write(table_exp_xxxy,"gaussXXXY_test.mat");
      }
    void initGaussXXYY(float scale){
	initGauss(scale);
	if(scale==xxyy_scale)return;
	xxyy_scale=scale;

	if(table_exp_xxyy!=NULL){delete [] table_exp_xxyy[0];delete [] table_exp_xxyy;}
	table_exp_xxyy = new float*[(2*size+1)];
	table_exp_xxyy[0]=new float[(2*size+1)*(2*size+1)];
	for(int i=1;i<(2*size+1);i++)table_exp_xxyy[i]=table_exp_xxyy[0]+i*(2*size+1); 
	
	float square_scale = xxyy_scale * xxyy_scale;
	float square_square_scale=square_scale*square_scale;
	float o_square_square_scale=(1/square_square_scale);
	float square_square_square_scale=square_scale*square_square_scale;
	float square_square_square_square_scale=square_square_scale*square_square_scale;
	
	for (int i=-size;i<=size;i++)
	  for (int j=-size;j<=size;j++)table_exp_xxyy[j+size][i+size] = 
					 (o_square_square_scale-((j*j)/square_square_square_scale)-
					  ((i*i)/square_square_square_scale)+
					  ((i*i*j*j)/square_square_square_square_scale))*
					 table_exp[j+size][i+size];
	//	 write(table_exp_xxyy,"gaussXXYY_test.mat");
	//	cout <<"done "<< endl;getchar();
    }

    float convolution(int x, int y, DARY* image_in, float** gauss){
	/* convolution with the mask in x,y */
	float value = 0;
	//cout << x << " " << y << " " << size << "  " << image_in->x()<< endl;
	if(x<size || y<size || x+size>=(int)image_in->x() || y+size>=(int)image_in->y())return(value);
	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
	      value += image_in->fel[y+j][x+i]*gauss[j+size][i+size];
	    }
	return(value);	
    }
    
     float inv_convolution(int x, int y, DARY* image_in,   float** gauss){

	/* convolution with the mask in x,y */
	float value = 0;	
	if(x<size || y<size || x+size>=(int)image_in->x() || y+size>=(int)image_in->y())return(value);
	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		value += image_in->fel[y+j][x+i] * gauss[i+size][j+size];
	    }
	return(value);
	
    }    

     float smoothf(int x, int y, DARY* image_in, float scale){
	initGauss(scale);
	//write(table_exp, "gauss.mat");
	return (convolution( x, y,image_in, table_exp)/total);
    }
     float dXX_YYf(int x, int y, DARY* image_in, float scale){       
	initGaussLap(scale);
	//cout << scale << " " << size<< endl;
	return (convolution( x, y,image_in, table_exp_lap)/total);
    }
     float dXf(int x, int y, DARY* image_in, float scale){
	initGaussX(scale);
	//write(table_exp_x,"gaussX_test.mat");
	return (convolution( x, y, image_in,table_exp_x)/total)/sum_x;
    }
     float dYf(int x, int y, DARY* image_in, float scale){
	initGaussX(scale);
	return (inv_convolution( x, y, image_in,table_exp_x)/total)/sum_x;
    }
     float dXYf(int x, int y, DARY* image_in, float scale){
	initGaussXY(scale);
	//write(table_exp_xy,"gaussXY_test.mat");
	return (convolution( x, y, image_in,table_exp_xy)/total);
    }
     float dXXf(int x, int y, DARY* image_in, float scale){
	initGaussXX(scale);
	//write(table_exp_xx,"gaussXX_test.mat");
	return (convolution( x, y, image_in,table_exp_xx)/total);
    }
     float dYYf(int x, int y, DARY* image_in, float scale){
	initGaussXX(scale);
	return (inv_convolution( x, y, image_in,table_exp_xx)/total);
    }
     float dXXXf(int x, int y, DARY* image_in, float scale){
	initGaussXXX(scale);
	//write(table_exp_xxx,"gaussXXX_test.mat");
	return (convolution( x, y, image_in,table_exp_xxx)/total);
    }
     float dXXYf(int x, int y, DARY* image_in, float scale){
	initGaussXXY(scale);
	//write(table_exp_xxy,"gaussXXY_test.mat");
	return (convolution( x, y, image_in,table_exp_xxy)/total);
    }
     float dXYYf(int x, int y, DARY* image_in, float scale){
	initGaussXXY(scale);
	return (inv_convolution( x, y, image_in,table_exp_xxy)/total);
    }
     float dYYYf(int x, int y, DARY* image_in, float scale){
	initGaussXXX(scale);
	return (inv_convolution( x, y, image_in,table_exp_xxx)/total);
    }
     float dXXXXf(int x, int y, DARY* image_in, float scale){
	initGaussXXXX(scale);
	//write(table_exp_xxxx,"gaussXXXX_test.mat");
	return (convolution( x, y, image_in,table_exp_xxxx)/total);
    }
     float dXXXYf(int x, int y, DARY* image_in, float scale){
	initGaussXXXY(scale);
	//write(table_exp_xxxy,"gaussXXXY_test.mat");
	return (convolution( x, y, image_in,table_exp_xxxy)/total);
    }
     float dXXYYf(int x, int y, DARY* image_in, float scale){
	initGaussXXYY(scale);
	//write(table_exp_xxyy,"gaussXXYY_test.mat");
	return (convolution( x, y, image_in,table_exp_xxyy)/total);
    }
     float dXYYYf(int x, int y, DARY* image_in, float scale){
	initGaussXXXY(scale);
	return (inv_convolution( x, y, image_in,table_exp_xxxy)/total);
    }
    float dYYYYf(int x, int y, DARY* image_in, float scale){
        initGaussXXXY(scale);
	return (inv_convolution( x, y, image_in,table_exp_xxxx)/total);
    }

    void drawGauss(DARY* image_in,int x, int y, float scale){
        initGaussLap(scale);
	float **gauss1=table_exp_lap;
	float max=0,min=1000;
	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		if(max<gauss1[i+size][j+size])
		    max=gauss1[i+size][j+size];
		if(min>gauss1[i+size][j+size])
		    min=gauss1[i+size][j+size];
		//cout <<image_in->fel[y+i][x+j] << endl;
	    }  
	float sc=0;    
	if(fabs(min)>max)
	    sc=127.0/fabs(min);
	else 
	    sc=127.0/max;
	if(x<size || y<size || x+size>=(int)image_in->x() || y+size>=(int)image_in->y())return;  

	for (int i=-size;i<=size;i++)
	    for (int j=-size;j<=size;j++){
		image_in->fel[y+i][x+j]=128+sc*gauss1[i+size][j+size];
		//cout <<image_in->fel[y+i][x+j] << endl;
	    }      
    }
  
