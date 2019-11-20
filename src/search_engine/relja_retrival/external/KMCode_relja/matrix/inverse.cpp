/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Novembre 1996                                                   */
/*                                                                          */
/* Module : Inversion de matrices                                           */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include <stdlib.h>
#include "matrix.h"


#define TINY 1.0e-20;


static void ludcmp(Matrix& A, int* indx, double& d)
{
  int i, imax=0, j, k, n = A.nbRows();
  double big, dum, sum, temp;

  DVector vv(n);
  d = 1.0;
  for (i = 1; i <= n; i++)
    {
      big = 0.0;
      for(j = 1; j <= n; j++)
	if ((temp = fabs(A(i,j))) > big) big = temp;
      if (big == 0.0) { cerr << "Singular matrix in routine ludcmp" << endl; exit(1); }
      vv(i) = 1.0 / big;
    }
  for(j = 1; j <= n; j++)
    {
      for(i = 1; i < j; i++)
	{
	  sum = A(i, j);
	  for(k = 1; k < i; k++) sum -= A(i,k) * A(k, j);
	  A(i, j) = sum;
	}
      big = 0.0;
      for(i = j; i <= n; i++)
	{
	  sum = A(i, j);
	  for(k = 1; k < j; k++)
	    sum -= A(i, k) * A(k, j);
	  A(i, j) = sum;
	  if ((dum = vv(i) * fabs(sum)) >= big)
	    {
	      big = dum;
	      imax = i;
	    }
	}
      if (j != imax)
	{
	  for(k = 1; k <= n; k++)
	    {
	      dum = A(imax, k);
	      A(imax, k) = A(j, k);
	      A(j, k) = dum;
	    }
	  d = -d;
	  vv(imax) = vv(j);
	}
      indx[j] = imax;
      if (A(j, j) == 0.0) A(j, j) = TINY;
      if (j != n)
	{
	  dum = 1.0 / (A(j, j));
	  for (i = j + 1; i <= n; i++) A(i, j) *= dum;
	}
    }
}


static void lubksb(const Matrix& A, int* indx, Vector& b)
{
  int i, ii = 0, ip, j, n = A.nbRows();
  double sum;

  for(i = 1; i <= n; i++)
    {
      ip = indx[i];
      sum = b(ip);
      b(ip) = b(i);
      if (ii)
	for(j = ii; j <= i - 1; j++) sum -= A(i, j) * b(j);
      else if (sum) ii = i;
      b(i) = sum;
    }
  for (i = n; i >= 1; i--)
    {
      sum = b(i);
      for(j = i + 1; j <= n; j++) sum -= A(i, j) * b(j);
      b(i) = sum / A(i, i);
    }
}


Matrix Matrix::inverse(void) const
{
  if (nbRows() != nbCols())
    error("inverse : matrice non carree");

  double d;
  DMatrix A = *this;
  int n = A.nbRows();
  DMatrix invA(n, n);
  DVector col(n);
  int* indx = new int[n] - 1;
  int i;

  ludcmp(A, indx, d);
  for(int j = 1; j <= n; j++)
    {
      for(i = 1; i <= n; i++) col(i) = 0.0;
      col(j) = 1.0;
      lubksb(A, indx, col);
      for(i = 1; i <= n; i++) invA(i, j) = col(i);
    }
  delete[] &indx[1];
  return invA;
}


#undef TINY
