/****************************************************************************/
/*                                                                          */
/* Projet : Reconstruction 3D                                               */
/*                                                                          */
/* Date   : Avril-Septembre 1994                                            */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Module : Decomposition d'une matrice en valeurs singulieres              */
/*          (Numerical Recipes)                                             */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include "matrix.h"

static double maxarg1, maxarg2;
#define FMAX(a, b) (maxarg1 = (a), maxarg2 = (b),(maxarg1) > (maxarg2) ?\
        (maxarg1) : (maxarg2))

static int iminarg1, iminarg2;
#define IMIN(a,b) (iminarg1 = (a), iminarg2 = (b), (iminarg1) < (iminarg2) ?\
        (iminarg1) : (iminarg2))

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

static double sqrarg;
#define SQR(a) ((sqrarg = (a)) == 0.0 ? 0.0 : sqrarg * sqrarg)


static double pythag(double a, double b)
{
  double absa, absb;

  absa = fabs(a);
  absb = fabs(b);
  if (absa > absb)
    return absa * sqrt(1.0 + SQR(absb / absa));
  else
    return (absb == 0.0 ? 0.0 : absb * sqrt(1.0 + SQR(absa / absb)));
}


void Matrix::svd(Matrix& U, Matrix& W, Matrix& V) const
{                                      // decomposition en valeurs singulieres
  int flag, i, its, j, jj, k, l=0, nm=0, m, n, tp = 0;
  double c, f, h, s, x, y, z;
  double anorm = 0.0, g = 0.0, scale = 0.0;
  Vector rv1;
  Matrix A = *this;

  if (A.nbRows() < A.nbCols())
    {
      U = A.transpose(); tp = 1;
    }
  else U = A;

  m = U.nbRows();
  n = U.nbCols();
  W.create(n, n, 0.0);
  V.create(n, n);
  rv1.create(n, 0.0);

  for(i = 1; i <= n; i++)
    {
      l = i + 1;
      rv1(i) = scale * g;
      g = s = scale = 0.0;
      if (i <= m)
        {
          for(k = i; k <= m; k++) scale += fabs(U(k, i));
          if (scale)
            {
              for(k = i; k <= m; k++)
                {
                  U(k, i) /= scale;
                  s += U(k, i) * U(k, i);
                }
              f = U(i, i);
              g = -SIGN(sqrt(s), f);
              h = f * g - s;
              U(i, i) = f - g;
              for(j = l; j <= n; j++)
                {
                  for(s = 0.0, k = i; k <= m; k++) s += U(k, i)*U(k, j);
                  f = s / h;
                  for(k = i; k <= m; k++) U(k, j) += f * U(k, i);
                }
              for(k = i; k <= m; k++) U(k, i) *= scale;
            }
        }
      W(i, i) = scale * g;
      g = s = scale = 0.0;
      if (i <= m && i != n)
        {
          for(k = l; k <= n; k++) scale += fabs(U(i, k));
          if (scale)
            {
              for(k = l; k <= n; k++)
                {
                  U(i, k) /= scale;
                  s += U(i, k) * U(i, k);
                }
              f = U(i, l);
              g = -SIGN(sqrt(s), f);
              h = f * g - s;
              U(i, l) = f - g;
              for(k = l; k <= n; k++) rv1(k) = U(i, k) / h;
              for(j = l; j <= m; j++)
                {
                  for(s = 0.0, k = l; k <= n; k++) s += U(j, k)*U(i, k);
                  for(k = l; k <= n; k++) U(j, k) += s * rv1(k);
                }
              for(k = l; k <= n; k++) U(i, k) *= scale;
            }
        }
      anorm = FMAX(anorm, (fabs(W(i,i)) + fabs(rv1(i))));
    }

  for(i = n; i >= 1; i--)
    {
      if (i < n)
        {
          if (g)
            {
              for(j = l; j <= n; j++)
              V(j, i) = (U(i, j) / U(i, l)) / g;
              for(j = l; j <= n; j++)
                {
                  for(s = 0.0, k = l; k <= n; k++) s += U(i, k) * V(k, j);
                  for(k = l; k <= n; k++) V(k, j) += s * V(k, i);
                }
            }
          for(j = l; j <= n; j++) V(i, j) = V(j, i) = 0.0;
        }
      V(i, i) = 1.0;
      g = rv1(i);
      l = i;
    }

  for(i = IMIN(m, n); i >= 1; i--)
    {
      l = i + 1;
      g = W(i, i);
      for(j = l; j <= n; j++) U(i, j) = 0.0;
      if (g)
        {
          g = 1.0 / g;
          for(j = l; j <= n; j++)
            {
              for(s = 0.0, k = l; k <= m; k++) s += U(k, i) * U(k, j);
              f = (s / U(i,i)) * g;
              for(k = i; k <= m; k++) U(k, j) += f * U(k, i);
            }
          for(j = i; j <= m; j++) U(j, i) *= g;
        }
      else
        for(j = i; j <= m; j++) U(j, i) = 0.0;
      ++U(i, i);
    }

  for(k = n; k >= 1; k--)
    {
      for(its = 1; its <= 30; its++)
        {
          flag = 1;
          for(l = k; l >= 1; l--)
            {
              nm = l - 1;
              if ((double)(fabs(rv1(l)) + anorm) == anorm)
                {
                  flag = 0;
                  break;
                }
              if ((double)(fabs(W(nm, nm)) + anorm) == anorm) break;
            }
          if (flag)
            {
              c = 0.0;
              s = 1.0;
              for(i = l; i <= k; i++)
                {
                  f = s * rv1(i);
                  if ((double)(fabs(f) + anorm) == anorm) break;
                  g = W(i, i);
                  h = pythag(f, g);
                  W(i, i) = h;
                  h = 1.0 / h;
                  c = g * h;
                  s = -f * h;
                  for(j = 1; j <= m; j++)
                    {
                      y = U(j, nm);
                      z = U(j, i);
                      U(j, nm) = y * c + z * s;
                      U(j, i) = z * c - y * s;
                    }
                }
            }
          z = W(k, k);
          if (l == k)
            {
              if (z < 0.0)
                {
                  W(k, k) = -z;
                  for(j = 1; j <= n; j++) V(j, k) = -V(j, k);
                }
              break;
            }
          if (its == 30) A.error("No convergence in 30 SVDCMP iterations");
          x = W(l, l);
          nm = k - 1;
          y = W(nm, nm);
          g = rv1(nm);
          h = rv1(k);
          f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
          g = pythag(f, 1.0);
          f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
          c = s = 1.0;
          for(j = l; j <= nm; j++)
            {
              i = j + 1;
              g = rv1(i);
              y = W(i, i);
              h = s * g;
              g = c * g;
              z = pythag(f, h);
              rv1(j) = z;
              c = f / z;
              s = h / z;
              f = x * c + g * s;
              g = g * c - x * s;
              h = y * s;
              y = y * c;
              for(jj = 1; jj <= n; jj++)
                {
                  x = V(jj, j);
                  z = V(jj, i);
                  V(jj, j) = x * c + z * s;
                  V(jj, i) = z * c - x * s;
                }
              z = pythag(f, h);
              W(j, j) = z;
              if (z)
                {
                  z = 1.0 / z;
                  c = f * z;
                  s = h * z;
                }
              f = (c * g) + (s * y);
              x = (c * y) - (s * g);
              for(jj = 1; jj <= m; jj++)
                {
                  y = U(jj, j);
                  z = U(jj, i);
                  U(jj, j) = y * c + z * s;
                  U(jj, i) = z * c - y * s;
                }
            }
          rv1(l) = 0.0;
          rv1(k) = f;
          W(k, k) = x;
        }
    }
  if (tp) swap(U, V);

  // les valeurs singulieres sont pratiquement deja triees
  // tri par insertion par ordre decroissant des valeurs singulieres
  for(i = 2; i <= W.nbCols(); i++)
    {
      j = i;     // j = position d'insertion
      while (j > 1 && W(j - 1, j - 1) < W(i, i)) j--;
      for(k = i; k > j; k--)
        {
          W.swapElts(k, k, k - 1, k - 1);
          U.swapCols(k, k - 1);
          V.swapCols(k, k - 1);
        }
    }
}

#undef SIGN
#undef MAX
#undef PYTHAG
