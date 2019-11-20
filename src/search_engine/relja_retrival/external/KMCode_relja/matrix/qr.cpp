/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Decomposition QR                                                */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include "matrix.h"


void Matrix::qr(Matrix& Q, Matrix& R) const     // decomposition QR
{
  double s, alpha, gamma, pivot;
  int i, j, k;
  Matrix A = *this, H;
  Vector beta, diagr, v;

  if (nbCols() > nbRows())
    error("decomposition QR : la matrice doit verifier m > n");

  R.create(nbCols(), nbCols());
  beta.create(nbCols());
  diagr.create(nbCols());
  if (nbRows() > nbCols()) v.create(nbRows()); else v.create(nbCols());

  for(k = 1; k <= nbCols(); k++)
    {
      s = 0.0;
      for(i = k; i <= nbRows(); i++)
        {
          v(i) = A(i, k);
          s += v(i) * v(i);
        }
      alpha = sqrt(s);
      pivot = A(k, k);
      if (pivot > 0.0) alpha = -alpha;
      diagr(k) = alpha;
      beta(k) = s - pivot * alpha;
      pivot -= alpha;
      v(k) = pivot;
      A(k, k) = pivot;
      for(j = k + 1; j <= nbCols(); j++)
        {
          s = 0.0;
          for(i = k; i <= nbRows(); i++) s += v(i) * A(i, j);
          gamma = s / beta(k);
          for(i = k; i <= nbRows(); i++) A(i, j) -= gamma * v(i);
        }
    }

  // creation de la matrice Q
  H.identity(nbRows());
  for(i = 1; i <= nbCols(); i++)
    {
      for(j = 1; j <= i - 1; j++) v(j) = 0.0;
      for(j = i; j <= nbRows(); j++) v(j) = A(j, i);
      H += -H * v * v.transpose() / beta(i);
    }
  Q = H.getBlock(1, 1, nbRows(), nbCols());

  // creation de la matrice R
  for(i = 1; i <= nbCols(); i++)
    {
       R(i, i) = diagr(i);
       for(j = i + 1; j <= nbCols(); j++) R(i, j) = A(i, j);
       for(j = 1; j <= i - 1; j++) R(i, j) = 0.0;
    }
}
