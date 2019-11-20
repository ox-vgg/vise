/****************************************************************************/
/*                                                                          */
/* Projet : Reconstruction 3D                                               */
/*                                                                          */
/* Date   : Avril-Septembre 1994                                            */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Module : Valeurs propres et vecteurs propres d'une matrice               */
/*          (Numerical Recipes)                                             */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include "matrix.h"


static void eigsrt(Vector& d, Matrix& V)
{
  int i, j, k, n;
  double p;

  n = d.nbRows();
  for(i = 1; i < n; i++)
    {
      p = d(k = i);
      for(j = i + 1; j <= n; j++)
        if(d(j) >= p) p = d(k = j);
      if (k != i)
	{
          d(k) = d(i);
	  d(i) = p;
	  for(j = 1; j <= n; j++)
	    {
	      p = V(j, i);
	      V(j, i) = V(j, k);
	      V(j, k) = p;
	    }
	}
    }
}


#define ROTATE(a,i,j,k,l) g=a(i,j);h=a(k,l);a(i,j)=g-s*(h+g*tau);\
	a(k,l)=h+s*(g-h*tau);


void Matrix::jacobi(Vector& d, Matrix& V) const
{
  int j, iq, ip, i, n, nrot;
  double tresh, theta, tau, t, sm, s, h, g, c;
  Vector b, z;
  Matrix A = *this;


  n = A.nbRows();
  b.create(n);
  z.create(n);
  d.create(n);
  V.create(n, n);
  for(ip = 1; ip <= n; ip++)
    {
      for (iq = 1; iq <= n; iq++) V(ip, iq) = 0.0;
      V(ip, ip) = 1.0;
    }
  for(ip = 1; ip <= n; ip++)
    {
      b(ip) = d(ip) = A(ip, ip);
      z(ip) = 0.0;
    }
  nrot = 0;
  for(i = 1; i <= 50; i++)
    {
      sm = 0.0;
      for(ip = 1; ip <= n - 1; ip++)
	{
	  for(iq = ip + 1; iq <= n; iq++)
	    sm += fabs(A(ip, iq));
	}
      if (sm == 0.0)
	{
	  eigsrt(d, V);
	  return;
	}
      if (i < 4)
	tresh = 0.2 * sm / (n * n);
      else
	tresh = 0.0;
      for(ip = 1; ip <= n - 1; ip++)
	{
	  for(iq = ip + 1; iq <= n; iq++)
	    {
	      g = 100.0 * fabs(A(ip, iq));
	      if (i > 4 && fabs(d(ip)) + g == fabs(d(ip))
		  && fabs(d(iq)) + g == fabs(d(iq)))
		A(ip, iq) = 0.0;
	      else if (fabs(A(ip, iq)) > tresh)
		{
		  h = d(iq) - d(ip);
		  if (fabs(h) + g == fabs(h))
		    t = (A(ip, iq)) / h;
		  else
		    {
		      theta = 0.5 * h / (A(ip, iq));
		      t = 1.0 / (fabs(theta) + sqrt(1.0 + theta * theta));
		      if (theta < 0.0) t = -t;
		    }
		  c = 1.0 / sqrt(1 + t * t);
		  s = t * c;
		  tau = s / (1.0 + c);
		  h = t * A(ip, iq);
		  z(ip) -= h;
		  z(iq) += h;
		  d(ip) -= h;
		  d(iq) += h;
		  A(ip, iq) = 0.0;
		  for(j = 1; j <= ip - 1; j++)
		    {
		      ROTATE(A, j, ip, j, iq);
		    }
		  for(j = ip + 1; j <= iq - 1; j++)
		    {
		      ROTATE(A, ip, j, j, iq);
		    }
		  for(j = iq + 1; j <= n; j++)
		    {
		      ROTATE(A, ip, j, iq, j);
		    }
		  for(j = 1; j <= n; j++)
		    {
		      ROTATE(V, j, ip, j, iq);
		    }
		  ++nrot;
		}
	    }
	}
      for(ip = 1; ip <= n; ip++)
	{
	  b(ip) += z(ip);
	  d(ip) = b(ip);
	  z(ip) = 0.0;
	}
    }
  A.error("Too many iterations in routine JACOBI");
}

#undef ROTATE
