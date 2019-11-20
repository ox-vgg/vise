/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Operateurs unaires                                              */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include "matrix.h"


const int False = 0;
const int True = 1;


Matrix Matrix::antisymmetrization(void) const    // antisymetrise une matrice
{
  int i, j;

  if (nbRows() != nbCols())
    error("antisymetrization : matrice non carree");

  Matrix res(nbRows(), nbRows());

  for(i = 1; i <= nbRows(); i++)
    {
      for(j = 1; j < i; j++)
        res(i, j) = res(j, i) = ((*this)(i, j) - (*this)(j, i)) / 2.0;
      res(i, i) = 0.0;
    }

  return res;
}


double Matrix::cofactor(int m, int n) const     // cofacteur
{
  return (((m + n + 1) % 2) * 2 - 1) * (*this)(m, n)
         * extract(m, n).determinant();
}


double Matrix::determinant(void) const          // determinant
{
  Matrix mat1, mat2;

  mat1 = jordan(mat2, False);
  //for(int i = 1; i <= mat1.nbRows(); i++)
  //if (fabs(mat1(i, i)) < 1E-15) return 0.0;
  return mat1(1, 1);
}

/*
Matrix Matrix::inverse(void) const              // inverse
{
  Matrix mat1, mat2;

  if (nbRows() != nbCols())
    error("inverse : matrice non carree");

  mat2.identity(nbRows());
  mat1 = jordan(mat2, False);

  return mat2 / mat1(1, 1);
}
*/

double Matrix::Minor(int m, int n) const        // mineur
{
  return extract(m, n).determinant();
}


int Matrix::rank(void) const                    // rang
{
  int rang;
  Matrix U, D, V;

  (*this).svd(U, D, V);
  rang = D.nbRows();
  while (rang > 0 && D(rang, rang) < 1E-7) rang--;

  return rang;
}


Matrix Matrix::sqrtDiag(void) const       // racine carree d'une mat diagonale
{
  int i;
  Matrix res(nbRows(), nbRows(), 0.0);

  for(i = 1; i <= nbRows(); i++)
    res(i, i) = sqrt((*this)(i, i));

  return res;
}


Matrix Matrix::symmetrization(void) const        // symetrise une matrice
{
  int i, j;

  if (nbRows() != nbCols())
    error("symetrization : matrice non carree");

  Matrix res(nbRows(), nbRows());

  for(i = 2; i <= nbRows(); i++)
    for(j = 1; j < i; j++)
      res(i, j) = res(j, i) = ((*this)(i, j) + (*this)(j, i)) / 2.0;

  return res;
}


double Matrix::trace(void) const                // trace
{
  double t = 0.0;
  int i;

  if (nbRows() != nbCols())
    error("trace : matrice non carree");

  for(i = 1; i <= nbRows(); i++) t += (*this)(i, i);
  return t;
}


Matrix Matrix::transpose(void) const            // transposee
{
  int i, j;
  Matrix res(nbCols(), nbRows());

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      res(j, i) = (*this)(i, j);

  return res;
}
