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


#include <backward/strstream>
#include <string.h>
#include <limits.h>
#include "matrix.h"

using namespace std;


const int False = 0;
const int True = 1; 

void Matrix::printCoeff(ofstream& out, double x) const
{
  char buf[100];
      ostrstream s(buf, sizeof(buf));
      s << x << ends;
      out << buf; 
      if (!strpbrk(buf, ".") && !strpbrk(buf, "e")
          && !strpbrk(buf, "E")) out << ".";
}

void Matrix::write(ofstream& out, int format) const
// ecriture d'une matrice dans un fichier
// formats :
// 0 : [nb_ligs nb_cols] ((coeffs))
// 1 : nb_ligs nb_cols coeffs
// 2 : coeffs
// 3 : nb_ligs coeffs
{
  int i, j;


  switch (format)
    {
      case 0  : out << "[ " << nbRows() << " " << nbCols() << " ]" << endl;
                break;
      case 1  : out << nbRows() << " " << nbCols() << endl;
                break;
      case 2  : break;
      case 3  : out << nbRows() << endl;
                break;
      case 4  : out << nbRows() << " " << nbCols() << endl;
                break;
      default : error("format d'ecriture de matrice invalide");
    }

  if (format == 0)
    {
      out << "(";
      for(i = 1; i <= nbRows(); i++)
        {
          out << ((i > 1)?" ( ":"( ");
          for(j = 1; j < nbCols(); j++)
	    {
	      printCoeff(out, ((*this)(i, j)));
              out << "  ";
	    }
          printCoeff(out, ((*this)(i, nbCols())));
          out << ((i < nbRows())?" )\n":" )");
        }
      out << ")" << endl;
    }
  else if (format == 4)
    {
      for(i = 1; i <= nbRows(); i++)
        {
          for(j = 1; j < nbCols(); j++)
	    {
              printCoeff(out, ((*this)(i, j)));
              out << ",  ";
	    }
          printCoeff(out, ((*this)(i, nbCols()))); out << ", " << endl;
        }
    }
  else
    {
      for(i = 1; i <= nbRows(); i++)
        {
          for(j = 1; j < nbCols(); j++)
	    {
              printCoeff(out, ((*this)(i, j)));
              out << "  ";
	    }
          printCoeff(out, ((*this)(i, nbCols()))); out << endl;
        }
    }
}


void Matrix::write(const char* filename, int format) const
{                               // ecriture d'une matrice dans un fichier
  ofstream out(filename);
  char msg[150];

  if (!out)
    {
      sprintf(msg, "ouverture du fichier '%s' en ecriture imposible", filename);
      error(msg);
    }

  Matrix::write(out, format);
}


void Matrix::read(ifstream& in)
{                                   // lecture d'une matrice dans un fichier
  int i, j, m = 0, n = 0, parenthese = False, space;
  char c, buf[10000];

  // calcul de la taille de la matrice
  in >> c;
  if (c == '[')
    in >> m >> n >> c;
  else
    {
      in.putback(c);
      in.getline(buf, sizeof(buf));
      istrstream s(buf, sizeof(buf));
      s >> m >> c;
      if (c != '\0')
	{
          s.putback(c);
          s >> n >> c;
          if (c != '\0') m = n = 0;
	}

      //cout << "m = " << m << ", n = " << n << endl;

      if (n)
	{
          // on verifie qu'il y a le bon nb de coeff sur la ligne suiv
          i = 0; space = True;
          while (in.get(c) && c != '\n')
	    {
              if (!space && (c == ' ' || c == '\t'))
                space = True;
              else
              if (space && c != ' ' && c != '\t')
                { i++; space = False; }
            }
          in.clear();
          if (i != n)
            { m = 0; n = i; }
          else
	    {
              in.seekg(0, ios::beg);
              in.getline(buf, sizeof(buf));
	    }
        }
      if (n == 0)   // on compte le nombre de colonnes
	{
          // si le nombre de lignes n'est pas dans le fichier, on
	  // reinitialise le fichier
          if (m == 0) { in.seekg(0, ios::beg); }
          space = True;
          while (in.get(c) && c != '\n')
	    {
              if (!space && (c == ' ' || c == '\t'))
                space = True;
              else
              if (space && c != ' ' && c != '\t')
                { n++; space = False; }
	    }
          in.clear();
          in.seekg(0, ios::beg);
          if (m) in.getline(buf, sizeof(buf));
	}
      if (m == 0)   // on compte le nombre de lignes
	{
          in.clear();
          in.seekg(0, ios::beg);
          while (in >> c) { m++; in.ignore(INT_MAX, '\n'); }
          in.clear();
          in.seekg(0, ios::beg);
	}
    }
  create(m, n);

  // lecture des coefficients
  in >> c;
  if (c == '(')
    {
      for(i = 1; i <= m; i++)     // boucle sur chaque ligne
        {
          in >> c;        // parenthese (
          if (c != '(') { in.putback(c); parenthese = False; }
            else parenthese = True;
          for(j = 1; j <= n; j++) in >> (*this)(i, j);
          if (parenthese) in >> c;        // parenthese )
        }
      in >> c;      // parenthese )
    }
  else
    {
      in.putback(c);
      for(i = 1; i <= m; i++)     // lecture de chaque ligne
        for(j = 1; j <= n; j++)
          { in >> (*this)(i, j); }
    }
}


void Matrix::read(const char* filename)
{                             // lecture d'une matrice dans un fichier
  ifstream in(filename);
  char msg[150];

  if (!in)
    {
      sprintf(msg, "ouverture du fichier '%s' en lecture impossible", filename);
      error(msg);
    }

  Matrix::read(in);
}


ostream& operator <<(ostream& out, const Matrix& M)  // affichage
{
  int i, j;

  out << "(";
  for(i = 1; i <= M.nbRows(); i++)
    {
      out << ((i > 1)?" ( ":"( ");
      for(j = 1; j <= M.nbCols(); j++)
        out << M(i, j) << ((j < M.nbCols())?"  ":"");
      out << " )";
      if (i < M.nbRows()) out << endl;
    }
  out << ")" << endl;
  return out;
}


istream& operator >>(istream& in, Matrix& M)
{
  int i, j, m, n, parenthese = False;
  char c;

  // calcul de la taille de la matrice
  in >> c;
  if (c == '[')
    {
      in >> m >> n >> c >> c;
      M.create(m, n);
    }
  else
  if (M.nbRows() == 0 && M.nbCols() == 0)
    {
      in.putback(c);
      in >> m >> n >> c;
      M.create(m, n);
    }

  // lecture des coefficients
  if (c == '(')
    {
      for(i = 1; i <= M.nbRows(); i++)     // boucle sur chaque ligne
        {
          in >> c;        // parenthese (
          if (c != '(') { in.putback(c); parenthese = False; }
            else parenthese = True;
          for(j = 1; j <= M.nbCols(); j++) in >> M(i, j);
          if (parenthese) in >> c;        // parenthese )
        }
      in >> c;      // parenthese )
    }
  else
    {
      in.putback(c);
      for(i = 1; i <= M.nbRows(); i++)     // lecture de chaque ligne
        for(j = 1; j <= M.nbCols(); j++)
          { in >> M(i, j); }
    }

  return in;
}


void Matrix::print(int n)  // affichage (pour le debugger par exemple)
{
  int i, j;

  if (n == 0) n = INT_MAX;
  cout << "(";
  for(i = 1; i <= nbRows() && n; i++)
    {
      cout << ((i > 1)?" ( ":"( ");
      for(j = 1; j <= nbCols() && n; j++)
        cout << (*this)(i, j) << ((j < nbCols())?"  ":"");
      cout << " )";
      if (i < nbRows()) cout << endl;
    }
  cout << ")" << endl;
}
