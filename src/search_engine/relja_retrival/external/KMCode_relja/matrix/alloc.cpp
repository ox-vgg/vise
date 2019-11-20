/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Allocation de la memoire                                        */
/*                                                                          */
/****************************************************************************/


#include <stdlib.h>
#include "matrix.h"


void Matrix::create(int nr, int nc) // allocation d'une matrice (non initialisee)
{
  int i;

  if (nr < 0 || nc < 0)
    error("allocation matrice (les nombres de lignes et colonnes doivent etre positifs)");

  if (nr == 0 || nc == 0) nr = nc = 0;

  // allocation
  if (nr != rows || nc != cols)
    {
      clear();
      rows = nr; cols = nc;
      if (nr != 0 && nc != 0)
	{
	  tabMat = new double*[nr] - 1;
	  if (tabMat == NULL) error("plus de place memoire");

	  for(i = 1; i <= nr; i++)
	    {
	      tabMat[i] = new double[nc] - 1;
	      if (tabMat[i] == NULL) error("plus de place memoire");
	    }
	}
    }
}


void Matrix::create(int nr, int nc, double val)
{                                 // creation et initialisation d'une matrice
  int i, j;

  if (nr < 0 || nc < 0)
    error("allocation matrice (les nombres de lignes et colonnes doivent etre positifs)");

  if (nr == 0 || nc == 0) nr = nc = 0;

  // allocation
  if (nr != rows || nc != cols)
    {
      clear();
      rows = nr; cols = nc;
      if (nr != 0 && nc != 0)
	{
	  tabMat = new double*[nr] - 1;
	  if (tabMat == NULL) error("plus de place memoire");

	  for(i = 1; i <= nr; i++)
	    {
	      tabMat[i] = new double[nc] - 1;
	      if (tabMat[i] == NULL) error("plus de place memoire");
	    }
	}
    }
  // initialisation des coefficients
  for(i = 1; i <= nr; i++)
    for(j = 1; j <= nc; j++)
      tabMat[i][j] = val;
}


void Matrix::clear(void)                  // desallocation d'une matrice
{
  int i;

  if (tabMat)
    {
      for(i = 1; i <= nbRows(); i++) delete[] &tabMat[i][1];
      delete[] &tabMat[1];
      rows = cols = 0; tabMat = NULL;
    }
}


void Matrix::error(char* msg) const       // affichage d'un message d'erreur
{
  cerr << "ERREUR classe Matrix" << endl;
  if (msg != NULL) cerr << msg << endl;
  cerr << "Interruption du programme..." << endl;
  abort();
}
