/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Jordan                                                          */
/*                                                                          */
/****************************************************************************/


#include <math.h>
#include "matrix.h"


Matrix Matrix::jordan(Matrix& M, int permut) const   // pivot de Jordan
{
  int i, j, k, ligPivot;
  int nbPivots = ((nbRows() < nbCols()) ? nbRows() : nbCols());
  double pivotAnt = 1.0;
  Matrix res = *this;


  for(j = 1; j <= nbPivots; j++)
    {
      // recherche du pivot (le plus grand possible)
      ligPivot = j;

      if (permut)
	{
          for(i = j; i <= nbRows(); i++)
            if (fabs(res(i, j)) > fabs(res(ligPivot, j))) ligPivot = i;

          // on deplace la ligne contenant le pivot trouve a la ligne j
          res.swapRows(j, ligPivot);
          M.swapRows(j, ligPivot);
	}

      // on sort de la boucle si on ne trouve pas de pivot non nul
      if (fabs(res(j, j)) == 0) break;

      // on effectue le pivotage (sur toutes les lignes sauf celle du pivot)
      for(i = 1; i <= nbRows(); i++)
        {
          // on ne modifie pas la ligne contenant le pivot
          if (i == j) continue;

          for(k = 1; k <= nbCols(); k++)
            if (k != j) res(i, k) = (res(j, j) * res(i, k)
                                  - res(i, j) * res(j, k)) / pivotAnt;

          for(k = 1; k <= M.nbCols(); k++)
            M(i, k) = (res(j, j) * M(i, k)
                      - res(i, j) * M(j, k)) / pivotAnt;

          res(i, j) = 0.0;
        }
      pivotAnt = res(j, j);
    }

  return res;
}
