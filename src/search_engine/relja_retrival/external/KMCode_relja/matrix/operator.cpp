/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Operateurs                                                      */
/*                                                                          */
/****************************************************************************/


#include <stdarg.h>
#include "matrix.h"


Matrix& Matrix::operator =(const Matrix& M)    // affectation
{
  int i, j;

  if (this != &M)      // verifie que l'affectation n'est pas du type A = A
    {
      create(M.nbRows(), M.nbCols());

      for(i = 1; i <= nbRows(); i++)
        for(j = 1; j <= nbCols(); j++)
          (*this)(i, j) = M(i, j);
    }

  return *this;
}


Matrix& Matrix::operator =(const double** M)
{
  int i, j;

  if (tabMat != (double**)M)
    {
      for(i = 1; i <= nbRows(); i++)
        for(j = 1; j <= nbCols(); j++)
          (*this)(i, j) = M[i][j];
    }
  return *this;
}


int Matrix::operator ==(const Matrix& M) const       // comparaison
{
  return isEgal(M);
}


int Matrix::operator !=(const Matrix& M) const       // comparaison
{
  return !isEgal(M);
}


Matrix Matrix::operator +(const Matrix& M) const  // addition : A + B
{
  int i, j;

  if (nbRows() != M.nbRows() ||
      nbCols() != M.nbCols())
    error("addition de matrices : tailles des matrices incompatibles");

  Matrix res(nbRows(), nbCols());

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      res(i, j) = (*this)(i, j) + M(i, j);

  return res;
}


Matrix Matrix::operator +(double x) const       // addition : A + x
{
  int i, j;
  Matrix res = *this;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      res(i, j) += x;

  return res;
}


Matrix operator +(double x, const Matrix& M)  // addition : x + A
{
  int i, j;
  Matrix res = M;

  for(i = 1; i <= res.nbRows(); i++)
    for(j = 1; j <= res.nbCols(); j++)
      res(i, j) += x;

  return res;
}


const Matrix& Matrix::operator +(void) const                // + unaire
{
  return *this;
}


Matrix& Matrix::operator +=(const Matrix& M)   // addition : A += B
{
  int i, j;

  if (nbRows() != M.nbRows() ||
      nbCols() != M.nbCols())
    error("addition de matrices : tailles des matrices incompatibles");

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      (*this)(i, j) += M(i, j);

  return *this;
}


Matrix& Matrix::operator +=(double x)      // addition : A += x
{
  int i, j;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      (*this)(i, j) += x;

  return *this;
}


Matrix Matrix::operator -(const Matrix& M) const  // soustraction : A - B
{
  int i, j;
  Matrix res = *this;

  if (nbRows() != M.nbRows() ||
      nbCols() != M.nbCols())
    error("soustraction de matrices : tailles des matrices incompatibles");

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      res(i, j) -= M(i, j);

  return res;
}


Matrix Matrix::operator -(double x) const       // soustraction : A - x
{
  int i, j;
  Matrix res = *this;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      res(i, j) -= x;

  return res;
}


Matrix Matrix::operator -(void) const           // - unaire
{
  int i, j;
  Matrix res(nbRows(), nbCols());

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      res(i, j) = -(*this)(i, j);

  return res;
}


Matrix& Matrix::operator -=(const Matrix& M)   // soustraction : A -= B
{
  int i, j;

  if (nbRows() != M.nbRows() ||
      nbCols() != M.nbCols())
    error("addition de matrices : tailles des matrices incompatibles");

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      (*this)(i, j) -= M(i, j);

  return *this;
}


Matrix& Matrix::operator -=(double x)      // soustraction : A -= x
{
  int i, j;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      (*this)(i, j) -= x;

  return *this;
}


Matrix Matrix::operator *(const Matrix& M) const    // multiplication : A * B
{
  int i, j, k;
  double s;

  if (nbCols() != M.nbRows())
    error("multiplication de matrices : tailles des matrices incompatibles");

  Matrix res(nbRows(), M.nbCols());

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= M.nbCols(); j++)
      {
        s = 0.0;
        for(k = 1; k <= nbCols(); k++) s += (*this)(i, k) * M(k, j);
        res(i, j) = s;
      }

  return res;
}


Matrix Matrix::operator *(double x) const       // multiplication : A * x
{
  int i, j;
  Matrix res = *this;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      res(i, j) *= x;

  return res;
}


Matrix operator *(double x, const Matrix& M)   // multiplication : x * A
{
  int i, j;
  Matrix res = M;

  for(i = 1; i <= res.nbRows(); i++)
    for(j = 1; j <= res.nbCols(); j++)
      res(i, j) *= x;

  return res;
}


Matrix& Matrix::operator *=(const Matrix& M)   // multiplication : A *= B
{
  // ne peut pas etre optimisee (recopie necessaire)
  *this = *this * M;
  return *this;
}


Matrix& Matrix::operator *=(double x)      // multiplication : A *= x
{
  int i, j;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      (*this)(i, j) *= x;

  return *this;
}


Matrix Matrix::operator /(double x) const       // division : A / x
{
  int i, j;
  Matrix res = *this;

  for(i = 1; i <= res.nbRows(); i++)
    for(j = 1; j <= res.nbCols(); j++)
      res(i, j) /= (double)x;

  return res;
}


Matrix& Matrix::operator /=(double x)      // division : A /= x
{
  int i, j;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      (*this)(i, j) /= (double)x;

  return *this;
}


double dot(const Matrix& M1, const Matrix& M2)     // produit scalaire de 2 matrices
{
  double ps = 0.0;
  int i, j;

  if (M1.nbRows() != M2.nbRows() ||
      M1.nbCols() != M2.nbCols())
    M1.error("dot : dimensions incompatibles");

  for (i = 1; i <= M1.nbRows(); i++)
    for(j = 1; j <= M1.nbCols(); j++)
      ps += M1(i, j) * M2(i, j);

  return ps;
}


void swap(Matrix& M1, Matrix& M2)            // echange 2 matrices
{
  int t;
  double** m;

  t = M1.rows; M1.rows = M2.rows; M2.rows = t;
  t = M1.cols; M1.cols = M2.cols; M2.cols = t;

  m = M1.tabMat; M1.tabMat = M2.tabMat; M2.tabMat = m;
}
