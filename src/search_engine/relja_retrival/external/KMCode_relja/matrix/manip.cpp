/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Diverses fonctions de manipulation de matrices                  */
/*                                                                          */
/****************************************************************************/


#include "matrix.h"


Matrix& Matrix::addCols(int c1, int c2, double x)
{                            // col2 = col2 + x * col1
  int i;

  for(i = 1; i <= nbRows(); i++)
    (*this)(i, c2) += x * (*this)(i, c1);

  return *this;
}


Matrix& Matrix::addRows(int r1, int r2, double x)
{                            // lig2 = lig2 + x * lig1
  int j;

  for(j = 1; j <= nbCols(); j++)
    (*this)(r2, j) += x * (*this)(r1, j);

  return *this;
}


double Matrix::avgCol(int c) const       // moyenne des elts d'une colonne
{
  return (double)sumCol(c) / nbRows();
}


double Matrix::avgRow(int r) const       // moyenne des elts d'une ligne
{
  return (double)sumRow(r) / nbCols();
}


Matrix Matrix::concat(const Matrix& M) const
{                            // borde une matrice a droite
  int i, j;
  Matrix res(nbRows(), nbCols() + M.nbCols());

  for(i = 1; i <= res.nbRows(); i++)
    {
      for(j = 1; j <= nbCols(); j++) res(i, j) = (*this)(i, j);
      for(j = 1; j <= M.nbCols(); j++) res(i, j + nbCols()) = M(i, j);
    }

  return res;
}


Matrix Matrix::delCol(int c) const
{                            // supprime une colonne
  return delCols(c, 1);
}


Matrix Matrix::delCols(int c, int nc) const
{                            // supprime plusieurs colonnes
  int i, j;
  Matrix res(nbRows(), nbCols() - nc);

  for(i = 1; i <= nbRows(); i++)
    {
      for(j = 1; j < c; j++) res(i, j) = (*this)(i, j);
      for(j = c; j < res.nbCols(); j++) res(i, j) = (*this)(i, j + nc);
    }

  return res;
}


Matrix Matrix::delRow(int r) const
{                            // supprime une ligne
  return delRows(r, 1);
}


Matrix Matrix::delRows(int r, int nr) const
{                            // supprime plusieurs lignes
  int i, j;
  Matrix res(nbRows() - nr, nbCols());

  for(j = 1; j <= nbCols(); j++)
    {
      for(i = 1; i < r; i++) res(i, j) = (*this)(i, j);
      for(i = r; i <= res.nbRows(); i++) res(i, j) = (*this)(i + nr, j);
    }

  return res;
}


Matrix Matrix::extend(int nr, int nc, double val) const
{                  // extension d'une matrice (et initialisation des coeff)
  int i, j;
  Matrix res(nbRows() + nr, nbCols() + nc, val);

  // recopie de la sous matrice
  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      res(i, j) = (*this)(i, j);

  return res;
}


Matrix Matrix::extract(int r, int c) const
{         // extrait une matrice en supprimant la ligne et colonne specifiees
  int i, j;
  Matrix res(nbRows() - 1, nbCols() - 1);

  for(i = 1; i <= r - 1; i++)
    {
      for(j = 1; j <= c - 1; j++) res(i, j) = (*this)(i, j);
      for(j = c; j <= res.nbCols(); j++) res(i, j) = (*this)(i, j + 1);
    }

  for(i = r; i <= res.nbRows(); i++)
    {
      for(j = 1; j <= c - 1; j++) res(i, j) = (*this)(i + 1, j);
      for(j = c; j <= res.nbCols(); j++) res(i, j) = (*this)(i + 1, j + 1);
    }

  return res;
}


Matrix Matrix::getBlock(int r1, int c1, int r2, int c2) const
{                            // extrait une sous matrice
  int i, j;
  Matrix res(r2 - r1 + 1, c2 - c1 + 1);

  for(i = 1; i <= res.nbRows(); i++)
    for(j = 1; j <= res.nbCols(); j++)
      res(i, j) = (*this)(i + r1 - 1, j + c1 - 1);

  return res;
}


Vector Matrix::getCol(int c) const
{                            // renvoie une colonne
  int i;
  Vector res(nbRows());

  for(i = 1; i <= nbRows(); i++)
    res(i) = (*this)(i, c);

  return res;
}


Vector Matrix::getRow(int r) const
{                            // renvoie une ligne
  int j;
  Vector res(nbCols());

  for(j = 1; j <= nbCols(); j++)
    res(j) = (*this)(r, j);

  return res;
}


Matrix Matrix::insertCol(int c, double val) const
{                            // insert une colonne
  return insertCols(c, 1, val);
}


Matrix Matrix::insertCols(int c, int nc, double val) const
{                            // insert plusieurs colonnes
  int i, j;
  Matrix res(nbRows(), nbCols() + nc);

  for(i = 1; i <= nbRows(); i++)
    {
      for(j = 1; j < c; j++) res(i, j) = (*this)(i, j);
      for(j = c; j < c + nc; j++) res(i, j) = val;
      for(j = c + nc; j <= res.nbCols(); j++) res(i, j) = (*this)(i, j + c + nc - 1);
    }

  return res;
}


Matrix Matrix::insertRow(int r, double val) const
{                            // insert une ligne
  return insertRows(r, 1, val);
}


Matrix Matrix::insertRows(int r, int nr, double val) const
{                            // insert plusieurs lignes
  int i, j;
  Matrix res(nbRows() + nr, nbCols());

  for(j = 1; j <= nbCols(); j++)
    {
      for(i = 1; i < r; i++) res(i, j) = (*this)(i, j);
      for(i = r; i < r + nr; i++) res(i, j) = val;
      for(i = r + nr; i <= res.nbRows(); i++) res(i, j) = (*this)(i + r + nr - 1, j);
    }

  return res;
}


Matrix& Matrix::setBlock(const Matrix& M, int r, int c)
{                            // remplace une sous matrice
  int i, j;

  for(i = 1; i <= M.nbRows(); i++)
    for(j = 1; j <= M.nbCols(); j++)
      (*this)(r + i - 1, c + j - 1) = M(i, j);

  return *this;
}


Matrix& Matrix::setCol(const Vector& v, int c)
{                            // remplace une colonne
  return setBlock(v, 1, c);
}


Matrix& Matrix::setRow(const Vector& v, int r)
{                            // remplace une ligne
  int j;

  for(j = 1; j <= nbCols(); j++)
    (*this)(r, j) = v(j);

  return *this;
}


Matrix Matrix::stack(const Matrix& M) const
{                            // empile une matrice en dessous
  int i, j;
  Matrix res(nbRows() + M.nbRows(), nbCols());

  for(j = 1; j <= res.nbCols(); j++)
    {
      for(i = 1; i <= nbRows(); i++) res(i, j) = (*this)(i, j);
      for(i = 1; i <= M.nbRows(); i++) res(i + nbRows(), j) = M(i, j);
    }

  return res;
}


double Matrix::sumCol(int c) const
{                            // somme des elts d'une colonne
  double s = 0.0;
  int i;

  for(i = 1; i <= nbRows(); i++) s += (*this)(i, c);
  return s;
}


double Matrix::sumElts(void) const
{                            // somme des elts d'une matrice
  double s = 0.0;
  int i, j;

  for(i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      s += (*this)(i, j);
  return s;
}


double Matrix::sumRow(int r) const
{                            // somme des elts d'une ligne
  double s = 0.0;
  int j;

  for(j = 1; j <= nbCols(); j++) s += (*this)(r, j);
  return s;
}


Matrix& Matrix::swapCols(int c1, int c2)
{                            // echange 2 colonnes
  double x;
  int i;

  for(i = 1; i <= nbRows(); i++)
    {
      x = (*this)(i, c1);
      (*this)(i, c1) = (*this)(i, c2);
      (*this)(i, c2) = x;
    }

  return *this;
}


Matrix& Matrix::swapRows(int r1, int r2)
{                            // echange 2 lignes
  double x;
  int j;

  for(j = 1; j <= nbCols(); j++)
  {
    x = (*this)(r1, j);
    (*this)(r1, j) = (*this)(r2, j);
    (*this)(r2, j) = x;
  }

  return *this;
}


Matrix& Matrix::swapElts(int r1, int c1, int r2, int c2)
{                            // echange 2 coefficients
  double x;

  x = (*this)(r1, c1);
  (*this)(r1, c1) = (*this)(r2, c2);
  (*this)(r2, c2) = x;

  return *this;
}
