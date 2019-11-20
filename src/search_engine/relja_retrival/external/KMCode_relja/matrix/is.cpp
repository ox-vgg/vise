/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Tests                                                           */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include "matrix.h"


const int False = 0;
const int True = 1;


int Matrix::isAllocated(void) const
{
  if (nbRows() == 0 || nbCols() == 0)
    return False;
  else
    return True;
}


int Matrix::isAntisymmetric(double eps) const
{
  int i, j, antisym = True;

  if (nbRows() != nbCols()) antisym = False;

  for(i = 1; i <= nbRows() && antisym; i++)
    if (fabs((*this)(i, i)) > fabs(eps)) antisym = False;

  for(i = 2; i <= nbRows() && antisym; i++)
    for(j = 1; j <= i - 1 && antisym; j++)
      if (fabs((*this)(i, j) + (*this)(j, i)) > fabs(eps)) antisym = False;

  return antisym;
}


int Matrix::isDiagonal(double eps) const
{
  int i, j, diag = True;

  if (nbRows() != nbCols()) diag = False;

  for(i = 1; i <= nbRows() && diag; i++)
    for(j = 1; j <= nbCols() && diag; j++)
      if (i != j && fabs((*this)(i, j)) > fabs(eps)) diag = False;

  return diag;
}


int Matrix::isDiff(const Matrix& M, double eps) const
{
  int i, j, n = 0;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      if (fabs((*this)(i, j) - M(i, j)) > fabs(eps)) n++;

  return n;
}


int Matrix::isEgal(const Matrix& M, double eps) const
{
  int i, j, egal = True;

  for(i = 1; i <= nbRows() && egal; i++)
    for(j = 1; j <= nbCols() && egal; j++)
      if (fabs((*this)(i, j) - M(i, j)) > fabs(eps)) egal = False;

  return egal;
}


int Matrix::isNullCol(int c, double eps) const
{
  int i, nul = True;

  for(i = 1; i <= nbRows() && nul; i++)
    if (fabs((*this)(i, c)) > fabs(eps)) nul = False;

  return nul;
}


int Matrix::isNullRow(int r, double eps) const
{
  int j, nul = True;

  for(j = 1; j <= nbCols() && nul; j++)
    if (fabs((*this)(r, j)) > fabs(eps)) nul = False;

  return nul;
}


int Matrix::isOrthogonal(double) const
{
  Matrix Id;

  Id.identity(nbCols());
  return (transpose() * (*this)).isEgal(Id, 1E-7);
}


int Matrix::isSize(int nr, int nc) const
{
  return nbRows() == nr && nbCols() == nc;
}


int Matrix::isSymmetric(double eps) const
{
  int i, j, sym = True;

  if (nbRows() != nbCols()) sym = False;

  for(i = 2; i <= nbRows() && sym; i++)
    for(j = 1; j <= i - 1 && sym; j++)
      if (fabs((*this)(i, j) - (*this)(j, i)) > fabs(eps)) sym = False;

  return sym;
}
