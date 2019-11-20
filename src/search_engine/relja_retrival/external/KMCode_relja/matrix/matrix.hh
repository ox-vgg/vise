/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Declaration de la classe matrice                                */
/*                                                                          */
/****************************************************************************/



#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

//#ifdef minor
//#undef minor
//#endif

class Vector;

class Matrix
{
  public :
    double** tabMat;

    // constructeurs
  Matrix(void);                         // creation sans allocation
  Matrix(int nr, int nc);               // creation et allocation
  Matrix(int nr, int nc, double val);   // creation avec valeur par defaut
  Matrix(const Matrix& M);              // creation par recopie
  Matrix(int nr, int nc, const double** M);
  Matrix(const char* filename);
  
  // destructeur
  virtual ~Matrix(void);

  // creation ou reallocation
  virtual void create(int nr, int nc);
  virtual void create(int nr, int nc, double val);
  virtual void clear(void);
  
  // taille de la matrice
  inline int nbRows(void) const;        // nb de lignes
  inline int nbCols(void) const;        // nb de colonnes
  inline int nbElts(void) const;        // nb d'elements
  
  // creation de matrices particulieres
  Matrix& crossMatrix(const Vector& M); // matrice associee au prod vectoriel
  Matrix& diag(int n, ...);             // creation d'une mat diagonale
  Matrix& identity(int n);              // renvoie une matrice Id
  Matrix& rnd(int nr, int nc);          // tire une matrice aleatoire
  Matrix& rotationX(double a);          // renvoie une mat de rotation 3D
  Matrix& rotationY(double a);
  Matrix& rotationZ(double a);
  Matrix& setMat(int nr, int nc, ...);  // initialise les coefficients
  
  // acces aux elements
  virtual inline double operator ()(int r, int c) const;
  virtual inline double& operator ()(int r, int c);
  
    // operateurs
    Matrix& operator =(const Matrix& M);            // affectation
    Matrix& operator =(const double** M);
    int operator ==(const Matrix& M) const;         // comparaison
    int operator !=(const Matrix& M) const;

    Matrix operator +(const Matrix& M) const;       // A + B
    Matrix operator +(double x) const;              // A + x
    friend Matrix operator +(double x, const Matrix& M);  // x + A
    const Matrix& operator +(void) const;                 // +A
    Matrix& operator +=(const Matrix& M);           // A += B
    Matrix& operator +=(double x);                  // A += x
    Matrix operator -(const Matrix& M) const;       // A - B
    Matrix operator -(double x) const;              // A -= x
    Matrix operator -(void) const;                  // -A
    Matrix& operator -=(const Matrix& M);           // A -= B
    Matrix& operator -=(double x);                  // A -= x
    Matrix operator *(const Matrix& M) const;       // A * B
    Matrix operator *(double x) const;              // A * x
    friend Matrix operator *(double x, const Matrix& M);   // x * A
    Matrix& operator *=(const Matrix& M);           // A *= B
    Matrix& operator *=(double x);                  // A *= x
    Matrix operator /(double x) const;              // A / x
    Matrix& operator /=(double x);                  // A /= B
    friend double dot(const Matrix& M1, const Matrix& M2);  // prod scalaire
    friend void swap(Matrix& M1, Matrix& M2);               // echange 2 matrices

    // tests
    // certains tests peuvent etre effectues a epsilon pres
    int isAllocated(void) const;                 // tst si une matrice est allouee
    int isAntisymmetric(double e = 0.0) const;   // tst si une mat est antisym
    int isDiagonal(double e = 0.0) const;        // tst si une mat est diagonale
    int isDiff(const Matrix& M, double e = 0.0) const;  // tst si 2 mat sont differentes
    int isEgal(const Matrix& M, double = 0.0) const;    // tst si 2 mat sont egales
    int isNullCol(int c, double e = 0.0) const;  // tst si une colonne est nulle
    int isNullRow(int r, double e = 0.0) const;  // tst si une ligne est nulle
    int isOrthogonal(double e = 0.0) const;      // tst si une mat est orthogonale
    int isSize(int nc, int nr = 1) const;        // tst si une mat a la taille spec
    int isSymmetric(double e = 0.0) const;       // tst si une mat est symetrique

    // fonctions unaires
    Matrix antisymmetrization(void) const;       // antisymetrise une matrice
    double cofactor(int r, int c) const;         // cofacteur
    double determinant(void) const;              // determinant
    Matrix inverse(void) const;                  // inverse
    double Minor(int r, int c) const;            // mineur
    int rank(void) const;                        // rang
    Matrix sqrtDiag(void) const;                 // racine carree d'une mat diag
    Matrix symmetrization(void) const;           // symetrise une matrice
    double trace(void) const;                    // trace
    Matrix transpose(void) const;                // transpose

    // normes
    double normInfty(void) const;                // norme infinie
    double norm1(void) const;                    // norme indice 1
    double norm2(void) const;                    // norme indice 2
    double normN(int n) const;                   // norme indice n
    double maxSingVal(void) const;               // retourne la plus grande val singuliere

    // manipulation des lignes et colonnes
    Matrix& addCols(int c1, int c2, double x = 1.0); // col2 = col2 + x * col1
    Matrix& addRows(int r1, int r2, double x = 1.0); // lig2 = lig2 + x * lig1
    double avgCol(int c = 1) const;                  // moyenne des elts d'une colonne
    double avgRow(int r) const;                      // moyenne des elts d'une ligne
    Matrix concat(const Matrix& M) const;            // borde une matrice a droite
    Matrix delCol(int c) const;                      // supprime une colonne
    Matrix delCols(int c, int nc) const;             // supprime plusieurs colonnes
    Matrix delRow(int r) const;                      // supprime une ligne
    Matrix delRows(int r, int nr) const;             // supprime plusieurs lignes
    Matrix extend(int nr, int nc, double val = 0.0) const;  // extension d'une matrice
    Matrix extract(int c, int r) const;              // extrait une sous matrice
    Matrix getBlock(int r1, int c1, int r2, int c2) const;  // extrait une sous matrice
    Vector getCol(int c) const;                      // renvoie une colonne
    Vector getRow(int r) const;                      // renvoie une ligne
    Matrix insertCol(int c, double val = 0.0) const; // insert une ou + col
    Matrix insertCols(int c, int nc, double val = 0.0) const;  // insert une ou + col
    Matrix insertRow(int r, double val = 0.0) const; // insert une ou + lig
    Matrix insertRows(int r, int nr, double val = 0.0) const;   // insert une ou + lig
    Matrix& setBlock(const Matrix& M, int r, int c); // remplace une sous matrice
    Matrix& setCol(const Vector& M, int c);          // remplace une colonne
    Matrix& setRow(const Vector& M, int r);          // remplace une ligne
    Matrix stack(const Matrix& M) const;             // empile une matrice en dessous
    double sumCol(int c = 1) const;                  // somme des elts d'une colonne
    double sumElts(void) const;                      // somme des elts de la matrice
    double sumRow(int r) const;                      // somme des elts d'une ligne
    Matrix& swapCols(int c1, int c2);                // echange 2 colonnes
    Matrix& swapRows(int r1, int r2);                // echange 2 lignes
    Matrix& swapElts(int r1, int c1, int r2, int c2);     // echange 2 coefficients

    // decompositions et systemes lineaires
    void lu(Matrix& L, Matrix& U) const;          // decomposition LU
    Matrix equation(const Matrix& B) const;       // resolution d'un systeme d'equat
    Matrix jordan(Matrix& M, int p = 0) const;    // pivot de Jordan
    Matrix leastSquares(const Matrix& B) const;   // resolution aux moindres carres
    void qr(Matrix& Q, Matrix& R) const;          // decomposition QR
    void ql(Matrix& Q, Matrix& L) const;          // decomposition QL
    void svd(Matrix& U, Matrix& D, Matrix& V) const;    // decomposition SVD (A = U D Vt)
    void jacobi(Vector& d, Matrix& V) const;            // vecteurs et valeurs propres pour une matrice symetrique
    Matrix choleskiL(void) const;
    Matrix choleskiU(void) const;
    friend Matrix bestRotation(const Matrix& M1, const Matrix& M2);   // rot tq rot * M1 = M2
    Matrix normalizeRotation(void);               // normalise une matrice de rotation

    // entrees/sorties
    void write(ofstream& out, int format = 0) const;    // ecriture d'une matrice sur un fichier
    void write(const char* filename, int format = 0) const;
    void read(ifstream& in);            // lecture d'une matrice sur un fichier
    void read(const char* filename);
    friend ostream& operator <<(ostream& out, const Matrix& M);
  friend istream& operator >>(istream& in, Matrix& M);
  void print(int n = 0);     // affichage (n contient le nb de coeff a imprimer)
  void printCoeff(ofstream& out, const double x) const;

  protected :
    int rows, cols;

    void error(char* = NULL) const;         // message d'erreur
};

#include "./matrix.icc"

