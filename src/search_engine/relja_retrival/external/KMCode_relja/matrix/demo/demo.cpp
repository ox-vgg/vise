/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Fichier de demonstration                                        */
/*                                                                          */
/****************************************************************************/

#include "matrix.h"

int main(void)
{
  DMatrix A, L, U, D, V;

  A.rnd(4, 3);
  A.svd(U, D, V);
  cout << A << U << D << V << U * D * V.transpose();

  A.rnd(3, 3);
  A = A.transpose() * A;
  L = A.choleskiL();
  cout << A << L * L.transpose();
  U = A.choleskiU();
  cout << U * U.transpose();

  double *t;
  t = new double[3] - 1;
  t[1] = 1.;
  t[2] = 2.;
  t[3] = 3.;
  DVector v(3, t);
  DPPoint p(3, t);
  cout << v << p;

  return 0;
}
