/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Declaration de la classe vecteur                                */
/*                                                                          */
/****************************************************************************/


class Vector : public Matrix
{
  public :
    double* tabVect;

    // constructeurs
    Vector(void);                  // creation sans allocation
    Vector(int n);                 // creation et allocation
    Vector(int n, double val);     // creation avec valeur par defaut
    Vector(const Matrix& M);       // creation par recopie
    Vector(const Vector& v);
    Vector(int n, const double* v);
    Vector(const char* filename);

    // destructeur
    virtual ~Vector(void);

    // creation ou reallocation
    virtual void create(int n);
    virtual void create(int n, double val);
    virtual void clear(void);

    // creation de vecteurs particuliers
    Vector& basisVector(int n, int i);   // renvoie un vect de base (dim, num)
    Vector& setVect(int n, ...);         // initialise les coeff

    // acces aux elements
    virtual inline double operator ()(int r) const;
    virtual inline double& operator ()(int r);

    // operateurs
    Vector& operator =(const Vector& v);        // affectation
    Vector& operator =(const Matrix& M);
    Vector& operator =(const double* v);
    friend double angle(const Vector& v1, const Vector& v2);   // angle entre 2 vect
    friend Vector cross(const Vector& M1, const Vector& M2, ...);
    friend void swap(Vector& v1, Vector& v2);   // echange 2 vecteurs

  protected :
    virtual void create(int nr, int nc);
    virtual void create(int nr, int nc, double val);

    virtual inline double operator ()(int r, int c) const;
    virtual inline double& operator ()(int r, int c);
};


#include "./vector.icc"
