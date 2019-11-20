/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Normes de matrices                                              */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include "matrix.h"


double Matrix::normInfty(void) const       // norme infinie
{
  double norme = 0.0;
  int i, j;

  for (i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      if (fabs((*this)(i, j)) > norme) norme = fabs((*this)(i, j));

  return norme;
}


double Matrix::norm1(void) const          // norme indice 1
{
  return normN(1);
}


double Matrix::norm2(void) const          // norme indice 2
{
  return normN(2);
}


double Matrix::normN(int n) const          // norme indice n
{
  double norme = 0.0;
  int i, j;

  // if (nbCols() > 1)
  //   error("la norme indice n d'une matrice n'est pas definie");

  for (i = 1; i <= nbRows(); i++)
    for(j = 1; j <= nbCols(); j++)
      norme += pow(fabs((*this)(i, j)), n);

  return pow(norme,1.0/n);
}


double Matrix::maxSingVal(void) const
{
  Matrix U, D, V;

  svd(U, D, V);
  // on retourne la plus grande valeur singuliere
  return D(1, 1);
}
