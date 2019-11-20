/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Resolution de systemes d'equations lineaires                    */
/*                                                                          */
/****************************************************************************/



#include "matrix.h"


Matrix Matrix::equation(const Matrix& B) const
{                           // resolution d'un systeme d'equations lineaire
  return inverse() * B;
}


Matrix Matrix::leastSquares(const Matrix& B) const
{                   // resolution d'un systeme lineaire aux moindres carres
  Matrix tThis = transpose();

  return (tThis * (*this)).inverse() * tThis * B;
}
