/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Janvier 1995                                                    */
/*                                                                          */
/* Module : Decomposition QL                                                */
/*                                                                          */
/****************************************************************************/


#include "matrix.h"


void Matrix::ql(Matrix& Q, Matrix& L) const  // decomposition QL
{
  int i, j;
  double t;
  Matrix A = *this;

  // permutation des colonnes de A
  for(j = 1; j <= A.nbCols() / 2; j++)
    for(i = 1; i <= A.nbRows(); i++)
      {
        t = A(i, j);
        A(i, j) = A(i, A.nbCols() - j + 1);
        A(i, A.nbCols() - j + 1) = t;
      }

  // decomposition QR de la matrice A
  A.qr(Q, L);

  // permutation des colonnes de Q
  for(j = 1; j <= Q.nbCols() / 2; j++)
    for(i = 1; i <= Q.nbRows(); i++)
      {
        t = Q(i, j);
        Q(i, j) = Q(i, Q.nbCols() - j + 1);
        Q(i, Q.nbCols() - j + 1) = t;
      }

  // permutation des lignes et colonnes de L
  for(i = 1; i <= L.nbRows(); i++)
    for(j = 1; j < i; j++)
      {
        t = L(i, j);
        L(i, j) = L(L.nbRows() - i + 1, L.nbCols() - j + 1);
        L(L.nbRows() - i + 1, L.nbCols() - j + 1) = t;
      }
  // coefficients diagonaux
  for(i = 1; i <= L.nbRows() / 2; i++)
    {
      t = L(i, i);
      L(i, i) = L(L.nbRows() - i + 1, L.nbCols() - i + 1);
      L(L.nbRows() - i + 1, L.nbCols() - i + 1) = t;
    }

  // calcul du determinant de la matrice de permutation
  if ((A.nbCols() % 4) == 2 || (A.nbCols() % 4) == 3)
    {
      for(i = 1; i <= Q.nbRows(); i++)
        Q(i, 1) = -Q(i, 1);
      for(j = 1; j <= L.nbCols(); j++)
        L(1, j) = -L(1, j);
    }
}
