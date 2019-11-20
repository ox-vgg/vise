/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Fichier include                                                 */
/*                                                                          */
/****************************************************************************/


class Quaternion
{
  public :
    Quaternion(void) ;                                  // constructeurs
    Quaternion(double qx, double qy, double qz) ;
    Quaternion(double q0, double qx, double qy, double qz) ;
    Quaternion(double q0, const Vector& v) ;
    Quaternion(const Vector& v) ;
    Quaternion(const Quaternion& q) ;
   ~Quaternion(void) ;                                  // destructeur

    Quaternion& setQuat(double qx, double qy, double qz) ;       // initialisation
    Quaternion& setQuat(double q0, double qx, double qy, double qz) ;
    Quaternion& setQuat(double q0, const Vector& v) ;
    Quaternion& setQuat(const Vector& v) ;
    Matrix matrixQ(void) const ;
    Matrix matrixW(void) const ;
    Matrix rotation(void) const ;    // matrice 3x3 de rotation associee a q

    // operateurs
    Quaternion& operator =(const Quaternion& q) ;         // affectation
    int operator ==(const Quaternion& q) const ;          // comparaison
    Quaternion operator +(void) const ;                 // + unaire
    Quaternion operator +(const Quaternion& q) const ;    // addition
    Quaternion operator -(void) const ;                 // - unaire
    Quaternion operator -(const Quaternion& q) const ;    // soustraction
    Quaternion operator *(const Quaternion& q) const ;    // multiplication
    friend ostream &operator <<(ostream& out, const Quaternion& q) ;   // affichage

  private :
    Vector vectQ ;
} ;
