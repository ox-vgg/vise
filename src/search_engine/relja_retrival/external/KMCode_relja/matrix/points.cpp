/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Points                                                          */
/*                                                                          */
/****************************************************************************/


#include <stdarg.h>
#include <math.h>
#include "matrix.h"


const int False = 0;
const int True = 1;


// points dans l'espace projectif


PPoint::PPoint(void) : Vector()
{ }


PPoint::PPoint(int n) : Vector(n + 1)
{
  (*this)(n + 1) = 1.0;
}


PPoint::PPoint(int n, double val) : Vector(n + 1, val)
{
  (*this)(n + 1) = 1.0;
}


PPoint::PPoint(const PPoint& p) : Vector(p)
{ }


PPoint::PPoint(const Matrix& M) : Vector(M)
{ }


PPoint::PPoint(int n, const double* p) : Vector(n, p)
{ }


PPoint::~PPoint(void)
{ }


PPoint& PPoint::setPoint(int n, ...)
{
  int i;
  va_list coeff;

  create(n + 1, 1);

  va_start(coeff, n);               // lecture des coefficients
  for (i = 1; i <= n; i++)
    (*this)(i) = va_arg(coeff, double);
  va_end(coeff);

  (*this)(nbRows()) = 1.0;
  return *this;
}


PPoint& PPoint::operator =(const RPoint& p)
{
  int i;

  create(p.nbRows() + 1);
  for(i = 1; i <= p.nbRows(); i++)
    (*this)(i) = p(i);
  (*this)(nbRows()) = 1.0;

  return *this;
}


int PPoint::operator ==(const PPoint& p) const
{
  int i, egal = True;

  for(i = 1; i <= (nbRows() - 1) && egal; i++)
    if (fabs((double)(*this)(i) / (*this)(nbRows())
           - (double)p(i) / p(nbRows())) > 1E-10) egal = False;

  return egal;
}


// ************************ points dans l'espace reel ************************


RPoint::RPoint(void) : Vector()
{ }


RPoint::RPoint(int n) : Vector(n)
{ }


RPoint::RPoint(int n, double val) : Vector(n, val)
{ }


RPoint::RPoint(const RPoint& p) : Vector(p)
{ }


RPoint::RPoint(const Matrix& M) : Vector(M)
{ }


RPoint::RPoint(int n, const double* p) : Vector(n, p)
{ }


RPoint::~RPoint(void)
{ }


RPoint& RPoint::setPoint(int n, ...)
{
  int i;
  va_list coeff;

  create(n, 1);

  va_start(coeff, n);               // lecture des coefficients
  for (i = 1; i <= nbRows(); i++)
    (*this)(i) = va_arg(coeff, double);
  va_end(coeff);

  return *this;
}


RPoint& RPoint::operator =(const PPoint& p)
{
  int i;

  create(p.nbRows() - 1);
  for(i = 1; i <= nbRows(); i++)
    (*this)(i) = (double)p(i) / p(p.nbRows());

  return *this;
}
