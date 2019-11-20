/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Declarations des classes de points                              */
/*                                                                          */
/****************************************************************************/


class RPoint;

class PPoint:public Vector
// points dans l'espace projectif
{
  public :
    PPoint(void);
    PPoint(int n);
    PPoint(int n, double val);
    PPoint(const PPoint& p);
    PPoint(const Matrix& M);
    PPoint(int n, const double* p);
   ~PPoint(void);

    PPoint& setPoint(int n, ...);
    PPoint& operator =(const RPoint& p);
    int operator ==(const PPoint& p) const;
};


class RPoint:public Vector
// points dans l'espace reel
{
  public :
    RPoint(void);
    RPoint(int n);
    RPoint(int n, double val);
    RPoint(const RPoint& p);
    RPoint(const Matrix& M);
    RPoint(int n, const double* p);
   ~RPoint(void);

    RPoint& setPoint(int n, ...);
    RPoint& operator =(const PPoint& p);
};
