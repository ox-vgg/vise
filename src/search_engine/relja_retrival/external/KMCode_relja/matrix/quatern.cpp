/****************************************************************************/
/*                                                                          */
/* Projet : Librairie Matrices                                              */
/*                                                                          */
/* Auteur : Stephane Christy                                                */
/*                                                                          */
/* Date   : Aout 1994                                                       */
/*                                                                          */
/* Module : Quaternions                                                     */
/*                                                                          */
/****************************************************************************/


#include "matrix.h"


Quaternion::Quaternion(void)
{
  vectQ.create(4, 0.0);
}


Quaternion::Quaternion(double qx, double qy, double qz)
{
  vectQ.create(4);
  vectQ(1) = 0.0; vectQ(2) = qx; vectQ(3) = qy; vectQ(4) = qz;
}


Quaternion::Quaternion(double q0, double qx, double qy, double qz)
{
  vectQ.create(4);
  vectQ(1) = q0; vectQ(2) = qx; vectQ(3) = qy; vectQ(4) = qz;
}


Quaternion::Quaternion(double q0, const Vector& w)
{
  vectQ.create(4, 0.0);
  setQuat(q0, w);
}


Quaternion::Quaternion(const Vector& v)
{
  vectQ.create(4, 0.0);
  setQuat(v);
}


Quaternion::Quaternion(const Quaternion& q)
{
  vectQ.create(4);
  vectQ = q.vectQ;
}


Quaternion::~Quaternion(void)
{
}


Quaternion& Quaternion::setQuat(double qx, double qy, double qz)
{
  vectQ(1) = 0.0; vectQ(2) = qx; vectQ(3) = qy; vectQ(4) = qz;
  return *this;
}


Quaternion& Quaternion::setQuat(double q0, double qx, double qy, double qz)
{
  vectQ(1) = q0; vectQ(2) = qx; vectQ(3) = qy; vectQ(4) = qz;
  return *this;
}


Quaternion& Quaternion::setQuat(double q0, const Vector& w)
{
  vectQ(1) = q0; vectQ(2) = w(1); vectQ(3) = w(2); vectQ(4) = w(3);
  return *this;
}


Quaternion& Quaternion::setQuat(const Vector& v)
{
  int i;

  if (v.nbRows() == 3)
    {
      vectQ(1) = 0.0;
      for(i = 1; i <= 3; i++) vectQ(i + 1) = v(i);
    }
  else
    for(i = 1; i <= 4; i++) vectQ(i) = v(i);

  return *this;
}


Matrix Quaternion::matrixQ(void) const
{
  Matrix mat(4, 4);

  mat(1, 1) = mat(2, 2) = mat(3, 3) = mat(4, 4) = vectQ(1);
  mat(1, 2) = mat(3, 4) = -vectQ(2);
  mat(2, 1) = mat(4, 3) = vectQ(2);
  mat(1, 3) = mat(4, 2) = -vectQ(3);
  mat(2, 4) = mat(3, 1) = vectQ(3);
  mat(1, 4) = mat(2, 3) = -vectQ(4);
  mat(3, 2) = mat(4, 1) = vectQ(4);

  return mat;
}


Matrix Quaternion::matrixW(void) const
{
  Matrix mat(4, 4);

  mat(1, 1) = mat(2, 2) = mat(3, 3) = mat(4, 4) = vectQ(1);
  mat(1, 2) = mat(4, 3) = -vectQ(2);
  mat(2, 1) = mat(3, 4) = vectQ(2);
  mat(1, 3) = mat(2, 4) = -vectQ(3);
  mat(3, 1) = mat(4, 2) = vectQ(3);
  mat(1, 4) = mat(3, 2) = -vectQ(4);
  mat(2, 3) = mat(4, 1) = vectQ(4);

  return mat;
}


Matrix Quaternion::rotation(void) const
{
  Matrix rot(3, 3);
  double q0, qx, qy, qz;

  q0 = vectQ(1); qx = vectQ(2); qy = vectQ(3); qz = vectQ(4);
  rot(1, 1) = q0 * q0 + qx * qx - qy * qy - qz * qz;
  rot(1, 2) = 2 * (qx * qy - q0 * qz);
  rot(1, 3) = 2 * (qx * qz + q0 * qy);
  rot(2, 1) = 2 * (qx * qy + q0 * qz);
  rot(2, 2) = q0 * q0 - qx * qx + qy * qy - qz * qz;
  rot(2, 3) = 2 * (qy * qz - q0 * qx);
  rot(3, 1) = 2 * (qx * qz - q0 * qy);
  rot(3, 2) = 2 * (qy * qz + q0 * qx);
  rot(3, 3) = q0 * q0 - qx * qx - qy * qy + qz * qz;

  return rot;
}


Quaternion& Quaternion::operator =(const Quaternion& q)       // affectation
{
  vectQ = q.vectQ;
  return *this;
}


int Quaternion::operator ==(const Quaternion& q) const
{
  return vectQ == q.vectQ;
}


Quaternion Quaternion::operator +(void) const
{
  return *this;
}


Quaternion Quaternion::operator +(const Quaternion& q) const
{
  Quaternion res;

  res.vectQ = vectQ + q.vectQ;
  return res;
}


Quaternion Quaternion::operator -(void) const
{
  Quaternion res;
  int i;

  for(i = 1; i <= 4; i++) res.vectQ(i) = -vectQ(i);
  return res;
}


Quaternion Quaternion::operator -(const Quaternion &q) const
{
  Quaternion res;

  res.vectQ = vectQ - q.vectQ;
  return res;
}


Quaternion Quaternion::operator *(const Quaternion& q) const
{
  Quaternion res;
  Vector w1, w2, v;


  w1 = vectQ.getBlock(1, 1, 3, 1);
  w2 = q.vectQ.getBlock(1, 1, 3, 1);

  v = cross(w1, w2) + w2 * vectQ(1) + w1 * q.vectQ(1);
  res.setQuat(vectQ(1) * q.vectQ(1) - dot(w1, w2), v);

  return res;
}


ostream &operator <<(ostream& out, const Quaternion& q)
{
  out << "(" << q.vectQ(1) << ", " << q.vectQ(2) << ", "
      << q.vectQ(3) << ", " << q.vectQ(4) << ")" << endl;

  return out;
}
