/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Creation de matrices particulieres                              */
/*                                                                          */
/****************************************************************************/


#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <sys/time.h>
#include "matrix.h"


Matrix& Matrix::crossMatrix(const Vector& v)
{                                     // matrice associee au prod vectoriel
  create(3, 3, 0.0);

  (*this)(1, 2) = -v(3);
  (*this)(1, 3) = v(2);
  (*this)(2, 1) = v(3);
  (*this)(2, 3) = -v(1);
  (*this)(3, 1) = -v(2);
  (*this)(3, 2) = v(1);
  return *this;
}


Matrix& Matrix::diag(int n, ...)      // creation d'une matrice diagonale
{
  int i;
  va_list coeff;

  create(n, n, 0.0);

  va_start(coeff, n);               // lecture des coefficients
  for (i = 1; i <= nbRows(); i++)
    ((*this)(i, i) = va_arg(coeff, double));
  va_end(coeff);

  return *this;
}


Matrix& Matrix::identity(int n)       // renvoie une matrice Id
{
  int i;

  create(n, n, 0.0);
  for(i = 1; i <= n; i++)
    (*this)(i, i) = 1.0;

  return *this;
}


Matrix& Matrix::rnd(int nr, int nc)     // tire une matrice aleatoire
{
  int i, j;

  // initialise rand
  srand(1000);
  // srand((int)time(NULL));

  create(nr, nc);

  for (i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      (*this)(i, j) = rand() % 100;

  return *this;
}


Matrix& Matrix::rotationX(double a)
{                       // renvoie une matrice de rotation 3D suivant l'axe x
  create(3, 3, 0.0);

  (*this)(1, 1) = 1.0;
  (*this)(2, 2) = (*this)(3, 3) = cos(a);
  (*this)(2, 3) = -sin(a);
  (*this)(3, 2) = sin(a);

  return *this;
}


Matrix& Matrix::rotationY(double a)
{                       // renvoie une matrice de rotation 3D suivant l'axe y
  create(3, 3, 0.0);

  (*this)(2, 2) = 1.0;
  (*this)(1, 1) = (*this)(3, 3) = cos(a);
  (*this)(1, 3) = sin(a);
  (*this)(3, 1) = -sin(a);

  return *this;
}


Matrix& Matrix::rotationZ(double a)
{                       // renvoie une matrice de rotation 3D suivant l'axe z
  create(3, 3, 0.0);

  (*this)(3, 3) = 1.0;
  (*this)(1, 1) = (*this)(2, 2) = cos(a);
  (*this)(1, 2) = -sin(a);
  (*this)(2, 1) = sin(a);

  return *this;
}


Matrix& Matrix::setMat(int nr, int nc, ...)
{                // creation d'une matrice et initialisation des coefficients
  int i, j;
  va_list coeff;

  create(nr, nc);

  va_start(coeff, nc);               // lecture des coefficients
  for (i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      (*this)(i, j) = va_arg(coeff, double);
  va_end(coeff);

  return *this;
}
