/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module :                                                                 */
/*                                                                          */
/****************************************************************************/


#include "matrix.h"


Matrix bestRotation(const Matrix& M1, const Matrix& M2)
{
  int j;
  Vector c1, c2, v;
  Matrix A, B, D, O1, O2;
  Quaternion q;

  B.create(4, 4, 0.0);
  for(j = 1; j <= M1.nbCols(); j++)
    {
      c1 = M2.getCol(j);
      c2 = M1.getCol(j);
      A = Quaternion((double)c1(1), (double)c1(2), (double)c1(3)).matrixQ()
        - Quaternion((double)c2(1), (double)c2(2), (double)c2(3)).matrixW();
      B += A.transpose() * A;
    }

  // calcul des valeurs propres
  B.svd(O1, D, O2);

  v = O2.getCol(O2.nbCols());
  q.setQuat(v);
  return q.rotation();
}


Matrix Matrix::normalizeRotation(void)
{
  Matrix I;

  I.identity(3);
  return bestRotation(transpose(), I);
}
