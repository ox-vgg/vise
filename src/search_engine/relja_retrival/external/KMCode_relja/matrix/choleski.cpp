/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Entrees/sorties sur fichier                                     */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include "matrix.h"


Matrix DMatrix::choleskiL(void) const
{
  int i, j, k;
  double s;
  Matrix L(nbRows(), nbRows(), 0.);

  for(j = 1; j <= L.nbCols(); j++)
    for(i = j; i <= L.nbRows(); i++)
      {
        for(k = 1, s = 0.; k < j; k++)
          s += L(i, k) * L(j, k);
        if (i == j) L(i, j) = sqrt((*this)(i, j) - s);
               else L(i, j) = ((*this)(i, j) - s) / L(j, j);
      }
  return L;
}


Matrix DMatrix::choleskiU(void) const
{
  int i, j, k;
  double s;
  Matrix U(nbRows(), nbRows(), 0.);

  for(j = U.nbCols(); j >= 1; j--)
    for(i = j; i >= 1; i--)
      {
        for(k = j + 1, s = 0.; k <= U.nbCols(); k++)
          s += U(i, k) * U(j, k);
        if (i == j) U(i, j) = sqrt((*this)(i, j) - s);
               else U(i, j) = ((*this)(i, j) - s) / U(j, j);
      }
  return U;
}
