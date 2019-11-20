/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Implementation des vecteurs                                     */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include <stdarg.h>
#include "matrix.h"


Vector::Vector(void) : Matrix()
{
  tabVect = NULL;
}


Vector::Vector(int n) : Matrix()
{
  tabVect = NULL;
  create(n);
}


Vector::Vector(int n, double val) : Matrix()
{
  tabVect = NULL;
  create(n, 1, val);
}


Vector::Vector(const Matrix& M) : Matrix()
{
  int i;

  tabVect = NULL;

  create(M.nbRows(), M.nbCols());
  for(i = 1; i <= M.nbRows(); i++)
    (*this)(i) = M(i, 1);
}


Vector::Vector(const Vector& v) : Matrix()
{
  int i;

  tabVect = NULL;
  create(v.nbRows());
  for(i = 1; i <= v.nbRows(); i++)
    (*this)(i) = v(i);
}


Vector::Vector(int n, const double* v) : Matrix()
{
  int i;

  tabVect = NULL;
  create(n);
  for(i = 1; i <= n; i++)
    (*this)(i) = v[i];
}


Vector::Vector(const char* filename) : Matrix(filename)
{ }


Vector::~Vector(void)
{
  clear();
}


void Vector::create(int n)
{
  create(n, 1);
}


void Vector::create(int nr, int nc)
{
  if (nc != 1)
    error("allocation vecteur (le nombre de colonnes doit etre egal a 1)");
  if (nr <= 0)
    error("allocation vecteur (le nombre de lignes doit etre positif)");

  if (nr != rows)
    {
      clear();
      rows = nr; cols = 1;
      tabVect = new double[nr] - 1;
      if (tabVect == NULL) error("plus de place memoire");
    }
}


void Vector::create(int n, double val)
{
  create(n, 1, val);
}


void Vector::create(int nr, int nc, double val)
{
  int i;

  if (nc != 1)
    error("allocation vecteur (le nombre de colonnes doit etre egal a 1)");
  if (nr <= 0)
    error("allocation vecteur (le nombre de lignes doit etre positif)");

  if (nr != rows)
    {
      clear();
      rows = nr; cols = 1;
      tabVect = new double[nr] - 1;
      if (tabVect == NULL) error("plus de place memoire");
    }
  // initialisation des coefficients
  for(i = 1; i <= nr; i++)
    tabVect[i] = val;
}


void Vector::clear(void)                  // desallocation d'un vecteur
{
  if (tabVect)
    {
      delete[] &tabVect[1];
      rows = cols = 0; tabVect = NULL;
    }
}


Vector& Vector::basisVector(int n, int i)
{                                    // renvoie un vecteur de base (dim, num)
  create(n, 0.0);
  (*this)(i) = 1.0;

  return *this;
}


Vector& Vector::setVect(int n, ...)
{                // creation d'un vecteur et initialisation des coefficients
  int i;
  va_list coeff;

  create(n);

  va_start(coeff, n);               // lecture des coefficients
  for (i = 1; i <= nbRows(); i++)
    (*this)(i) = va_arg(coeff, double);
  va_end(coeff);

  return *this;
}


Vector& Vector::operator =(const Vector& v)        // affectation
{
  int i;

  if (this != &v)      // verifie que l'affectation n'est pas du type A = A
    {
      create(v.nbRows());

      for(i = 1; i <= nbRows(); i++)
        (*this)(i) = v(i, 1);
    }
  return *this;
}


Vector& Vector::operator =(const Matrix& M)        // affectation
{
  int i;

  if (this != &M)      // verifie que l'affectation n'est pas du type A = A
    {
      create(M.nbRows(), M.nbCols());
      for(i = 1; i <= nbRows(); i++)
        (*this)(i) = M(i, 1);
    }
  return *this;
}


Vector& Vector::operator =(const double* v)
{
  int i;

  if (tabVect != v)
    {
      for(i = 1; i <= nbRows(); i++)
        (*this)(i) = v[i];
    }
  return *this;
}


double angle(const Vector& v1, const Vector& v2) // angle entre 2 vecteurs
{
  double norme1, norme2;
  double prodScal, cosAng;
  int signe = 1;


  if (!v1.isAllocated() || !v2.isAllocated())
    v1.error("angle : vecteurs non definis");

  if (v1.nbCols() != 1 || v2.nbCols() != 1)
    v1.error("angle : vecteurs attendus");

  if (v1.nbRows() != v2.nbRows())
    v1.error("angle : vecteurs de tailles differentes");

  if (v1.nbRows() == 2)
    if (fabs(v1(1) * v2(2) - v1(2) * v2(1)) > 0)
      signe = 1;
    else
      signe = -1;

  norme1 = v1.norm2();
  norme2 = v2.norm2();
  prodScal = dot(v1, v2);

  cosAng = 1.0 * prodScal / (norme1 * norme2);

  if (cosAng > 1.0) cosAng = 1.0; else
  if (cosAng < -1.0) cosAng = -1.0;
  return signe * acos(cosAng);
}


Vector cross(const Vector& M1, const Vector& M2, ...)
{                                          // produit vectoriel en dimension n
  int i;
  Matrix mat(M1.nbRows(), M1.nbRows());

  mat.setCol(M1, 1);
  mat.setCol(M2, 2);

#ifndef __GNUC__
  va_list vect;

  va_start(vect, M2);
  for (i = 3; i < mat.nbCols(); i++)
    mat.setCol(va_arg(vect, Matrix), i);
  va_end(vect);
#endif

  Vector res(mat.nbRows(), 1);
  for(i = 1; i <= mat.nbRows(); i++)
    res(i) = (((i + mat.nbCols() + 1) % 2) * 2 - 1)
           * mat.Minor(i, mat.nbCols());

  return res;
}


void swap(Vector& v1, Vector& v2)            // echange 2 vecteurs
{
  int t;
  double* v;

  t = v1.rows; v1.rows = v2.rows; v2.rows = t;
  t = v1.cols; v1.cols = v2.cols; v2.cols = t;

  v = v1.tabVect; v1.tabVect = v2.tabVect; v2.tabVect = v;
}
