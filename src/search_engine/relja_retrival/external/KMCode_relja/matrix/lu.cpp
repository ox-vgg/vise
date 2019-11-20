/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Decomposition LU                                                */
/*                                                                          */
/****************************************************************************/


#include "matrix.h"


void Matrix::lu(Matrix& L, Matrix& U) const      // decomposition LU
{
  double s;
  int i, j, k, taille;


  taille = ((nbRows() < nbCols()) ? nbRows() : nbCols());
  L.create(nbRows(), taille);
  U.create(taille, nbCols());

  for(i = L.nbRows() - taille + 1; i <= L.nbRows(); i++) L(i, i) = 1.0;

  for(i = 1; i <= U.nbRows(); i++)
    {
      // on remplit la ligne i ligne de U
      for(j = i; j <= U.nbCols(); j++)
        {
          s = 0.0;
          for(k = 1; k <= i - 1; k++) s += L(i, k) * U(k, j);
          U(i, j) = (*this)(i, j) - s;
        }

      // on remplit la colonne i de L
      for(j = i + 1; j <= L.nbRows(); j++)
        {
          s = 0.0;
          for(k = 1; k <= i - 1; k++) s += L(j, k) * U(k, i);
          L(j, i) = 1.0 * ((*this)(j, i) - s) / U(i, i);
        }
    }
}
