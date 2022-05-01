#ifndef __MATHQUATERNION_36380400_H__
#define __MATHQUATERNION_36380400_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Utils/Random.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatBaryCentric(sQuat<T>& Out,
                          const sQuat<T>& Q1,
                          const sQuat<T>& Q2,
                          const sQuat<T>& Q3,
                          T f,
                          T g)
{
  sQuat<T>  u, v;

  f += g;
  niAssert(f != 0);

  QuatSlerp(u, Q1, Q2, f);
  QuatSlerp(v, Q1, Q3, f);
  QuatSlerp(Out, u, v, g/f);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatConjugate(sQuat<T>& Out, const sQuat<T>& Q)
{
  Out.x = -Q.x;
  Out.y = -Q.y;
  Out.z = -Q.z;
  Out.w =  Q.w;
  return Out;
}

///////////////////////////////////////////////
template <class T>
T QuatDot(const sQuat<T>& Q1, const sQuat<T>& Q2)
{
  return Q1.x * Q2.x + Q1.y * Q2.y + Q1.z * Q2.z + Q1.w * Q2.w;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatExp(sQuat<T>& Out, const sQuat<T>& Q)
{
  T theta, sinTheta, coeff;

  theta  = (T)sqrt(Q.x * Q.x + Q.y * Q.y + Q.z * Q.z);
  sinTheta = sinf(theta);

  if (fabs(sinTheta) >= niEpsilon5)
  {
    coeff = sinTheta / theta;
    Out.x = coeff * Q.x;
    Out.y = coeff * Q.y;
    Out.z = coeff * Q.z;
  }
  else
  {
    Out.x = Q.x;
    Out.y = Q.y;
    Out.z = Q.z;
  }

  Out.w = cosf(theta);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatIdentity(sQuat<T>& Out)
{
  Out.x = 0.0f;
  Out.y = 0.0f;
  Out.z = 0.0f;
  Out.w = 1.0f;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatInverse(sQuat<T>& Out, const sQuat<T>& Q)
{
  T denom;

  denom = Q.x * Q.x + Q.y * Q.y + Q.z * Q.z + Q.w * Q.w;
  niAssert(denom != 0);

  denom = 1.0f / denom;

  Out.x = -Q.x * denom;
  Out.y = -Q.y * denom;
  Out.z = -Q.z * denom;
  Out.w =  Q.w * denom;

  return Out;
}

///////////////////////////////////////////////
template <class T>
tBool QuatIsIdentity(const sQuat<T>& Q)
{
  return (Q.x == 0.0f) && (Q.y == 0.0f) && (Q.z == 0.0f) && (Q.w == 1.0f);
}

///////////////////////////////////////////////
template <class T>
T QuatLength(const sQuat<T>& Q)
{
  return (T)sqrt(Q.x * Q.x + Q.y * Q.y + Q.z * Q.z + Q.w * Q.w);
}

///////////////////////////////////////////////
template <class T>
T QuatLengthSq(const sQuat<T>& Q)
{
  return Q.x * Q.x + Q.y * Q.y + Q.z * Q.z + Q.w * Q.w;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatLn(sQuat<T>& Out, const sQuat<T>& Q)
{
  T theta, sinTheta, coeff;

  if (fabs(Q.w) < 1.0f)
  {
    theta  = acosf(Q.w);
    sinTheta = sinf(theta);

    if (fabs(sinTheta) >= niEpsilon5)
    {
      coeff = theta / sinTheta;
      Out.x = coeff * Q.x;
      Out.y = coeff * Q.y;
      Out.z = coeff * Q.z;
      Out.w = 0.0f;
      return Out;
    }
  }

  Out.x = Q.x;
  Out.y = Q.y;
  Out.z = Q.z;
  Out.w = 0.0f;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatMultiply(sQuat<T>& Out,
                       const sQuat<T>& Q1,
                       const sQuat<T>& Q2)
{
  Out = Q1.Mul(Q2);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatNormalize(sQuat<T>& Out, const sQuat<T>& Q)
{
  T denom;

  denom = (T)sqrt(Q.x * Q.x + Q.y * Q.y + Q.z * Q.z + Q.w * Q.w);
  niAssert(denom != 0);

  denom = 1.0f / denom;

  Out.x = Q.x * denom;
  Out.y = Q.y * denom;
  Out.z = Q.z * denom;
  Out.w = Q.w * denom;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatNormalize(sQuat<T>& Out)
{
  return QuatNormalize(Out,Out);
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationAxis(sQuat<T>& Out, const sVec3<T>& V, T angle)
{
  T sinHalfAngle;

  sinHalfAngle = ni::Sin(T(0.5) * angle);
  Out.x    = sinHalfAngle * V.x;
  Out.y    = sinHalfAngle * V.y;
  Out.z    = sinHalfAngle * V.z;
  Out.w    = ni::Cos(T(0.5) * angle);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotateX(sQuat<T>& Out, const sQuat<T>& Src, T angle)
{
  T c = cosf(angle*0.5f);
  T s = sinf(angle*0.5f);
  sQuat<T> tmp(Src);
  Out.x = c*tmp.x + s*tmp.w;
  Out.y = c*tmp.y - s*tmp.z;
  Out.z = c*tmp.z + s*tmp.y;
  Out.w = c*tmp.w - s*tmp.x;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotateY(sQuat<T>& Out, const sQuat<T>& Src, T angle)
{
  T c = cosf(angle*0.5f);
  T s = sinf(angle*0.5f);
  sQuat<T> tmp(Src);
  Out.x = c*tmp.x + s*tmp.z;
  Out.y = c*tmp.y + s*tmp.w;
  Out.z = c*tmp.z - s*tmp.x;
  Out.w = c*tmp.w - s*tmp.y;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotateZ(sQuat<T>& Out, const sQuat<T>& Src, T angle)
{
  T c = cosf(angle*0.5f);
  T s = sinf(angle*0.5f);
  sQuat<T> tmp(Src);
  Out.x = c*tmp.x - s*tmp.y;
  Out.y = c*tmp.y - s*tmp.x;
  Out.z = c*tmp.z + s*tmp.w;
  Out.w = c*tmp.w + s*tmp.z;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationX(sQuat<T>& Out, T angle)
{
  return QuatRotationAxis(Out, sVec3<T>::XAxis(), angle);
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationY(sQuat<T>& Out, T angle)
{
  return QuatRotationAxis(Out, sVec3<T>::YAxis(), angle);
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationZ(sQuat<T>& Out, T angle)
{
  return QuatRotationAxis(Out, sVec3<T>::ZAxis(), angle);
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationMatrix(sQuat<T>& Out, const sMatrix<T>& M)
{
  static int  next[3] = { 1, 2, 0 };
  T   trace, root;
  int   i, j, k;

  trace = M._11 + M._22 + M._33;
  if(trace > 0.0f)
  {
    root  = (T)sqrt(trace + 1.0f);
    Out.w = 0.5f * root;
    root  = 0.5f / root;

    Out.x = root * (M._23 - M._32);
    Out.y = root * (M._31 - M._13);
    Out.z = root * (M._12 - M._21);
  }
  else
  {
    i = 0;
    if(M._22 > M._11)  i = 1;
    if(M._33 > M(i,i)) i = 2;

    j = next[i];
    k = next[j];

    root = (T)sqrt(M(i,i) - M(j,j) - M(k,k) + 1.0f);

    Out[i] = 0.5f * root;
    root              = 0.5f / root;
    Out[j] = root * (M(i,j) + M(j,i));
    Out[k] = root * (M(i,k) + M(k,i));
    Out.w             = root * (M(j,k) - M(k,j));
  }

  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationYawPitchRoll(sQuat<T>& Out, const T yaw, const T pitch, const T roll)
{
  T sinY, cosY, sinP, cosP, sinR, cosR;

  sinY = sinf(0.5f * yaw);
  cosY = cosf(0.5f * yaw);

  sinP = sinf(0.5f * pitch);
  cosP = cosf(0.5f * pitch);

  sinR = sinf(0.5f * roll);
  cosR = cosf(0.5f * roll);

  Out.x = cosR * sinP * cosY + sinR * cosP * sinY;
  Out.y = cosR * cosP * sinY - sinR * sinP * cosY;
  Out.z = sinR * cosP * cosY - cosR * sinP * sinY;
  Out.w = cosR * cosP * cosY + sinR * sinP * sinY;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationVector(sQuat<T>& Out, const sVec3<T>& vFrom, const sVec3<T>& vTo)
{
  sVec3<T> crossFT;
  return QuatRotationAxis(Out,VecNormalize(VecCross(crossFT,vFrom,vTo)),ni::Cos(VecDot(vFrom,vTo)));
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatSlerp(sQuat<T>& Out,
                    const sQuat<T>& Q1,
                    const sQuat<T>& Q2,
                    T t,
                    eQuatSlerp mode = eQuatSlerp_Short)
{
  T cosTheta, sinTheta, theta, invT;
  T q1x, q1y, q1z, q1w;
  T q2x, q2y, q2z, q2w;

  q1x = Q1.x; q1y = Q1.y; q1z = Q1.z; q1w = Q1.w;
  q2x = Q2.x; q2y = Q2.y; q2z = Q2.z; q2w = Q2.w;

  cosTheta = QuatDot(Q1, Q2);

  if (((mode == eQuatSlerp_Short) && (cosTheta < 0.0)) ||
      ((mode == eQuatSlerp_Long)  && (cosTheta > 0.0)))
  {
    cosTheta = -cosTheta;
    q2x = -q2x;
    q2y = -q2y;
    q2z = -q2z;
    q2w = -q2w;
  }
  invT = 1.0f - t;

  if (1.0f - cosTheta > niEpsilon5) {
    theta = acosf(cosTheta);
    sinTheta = sinf(theta);
    invT = sinf(theta * invT) / sinTheta;
    t = sinf(theta *   t) / sinTheta;
  }

  Out.x = invT * q1x + t * q2x;
  Out.y = invT * q1y + t * q2y;
  Out.z = invT * q1z + t * q2z;
  Out.w = invT * q1w + t * q2w;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatSquad(sQuat<T>& Out,
                    const sQuat<T>& Q1,
                    const sQuat<T>& Q2,
                    const sQuat<T>& Q3,
                    const sQuat<T>& Q4,
                    T t)
{
  sQuat<T> u, v;

  QuatSlerp(u, Q1, Q4, t);
  QuatSlerp(v, Q2, Q3, t);
  QuatSlerp(Out, u, v, 2.0f * t * (1.0f - t));

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec3<T>& QuatGetAxis(sVec3<T>& vAxis, const sQuat<T>& Q)
{
  T squareLength, invLength;
  squareLength = Q.x * Q.x + Q.y * Q.y + Q.z * Q.z;
  if (squareLength > niEpsilon5) {
    invLength = 1.0f / (T)sqrt(squareLength);
    vAxis.x = Q.x * invLength;
    vAxis.y = Q.y * invLength;
    vAxis.z = Q.z * invLength;
  }
  else {
    vAxis.x = 1.0f;
    vAxis.y = 0.0f;
    vAxis.z = 0.0f;
  }
  return vAxis;
}

///////////////////////////////////////////////
template <class T>
T QuatGetAngle(const sQuat<T>& Q) {
  return 2.0f * acosf(Q.w);
}

///////////////////////////////////////////////
template <class T>
void QuatToAxisAngle(const sQuat<T>& Q,
                     sVec3<T>& vAxis,
                     T& Angle)
{
  QuatGetAxis(vAxis,Q);
  Angle = QuatGetAngle(Q);
}

///////////////////////////////////////////////
// convert a quaternion to euler angles (in radians)
template <class T>
sVec3<T>& QuatToEuler(const sQuat<T>& Q, sVec3<T>& euler)
{
  T matrix[3][3];
  T cx,sx;
  T cy,sy,yr;
  T cz,sz;

  matrix[0][0] = T(1.0) - (T(2.0) * Q.y * Q.y) - (T(2.0) * Q.z * Q.z);
  matrix[1][0] = (T(2.0) * Q.x * Q.y) + (T(2.0) * Q.w * Q.z);
  matrix[2][0] = (T(2.0) * Q.x * Q.z) - (T(2.0) * Q.w * Q.y);
  matrix[2][1] = (T(2.0) * Q.y * Q.z) + (T(2.0) * Q.w * Q.x);
  matrix[2][2] = T(1.0) - (T(2.0) * Q.x * Q.x) - (T(2.0) * Q.y * Q.y);

  sy = -matrix[2][0];
  cy = (T)sqrt(T(1) - (sy * sy));
  yr = (T)atan2(sy,cy);
  euler.y = yr;

  // avoid divide by zero only where y ~90 or ~270
  if (sy != T(1.0) && sy != T(-1.0))
  {
    cx = matrix[2][2] / cy;
    sx = matrix[2][1] / cy;
    euler.x = (T)atan2(sx,cx);

    cz = matrix[0][0] / cy;
    sz = matrix[1][0] / cy;
    euler.z = (T)atan2(sz,cz);
  }
  else
  {
    matrix[1][1] = T(1.0) - (T(2.0) * Q.x * Q.x) - (T(2.0) * Q.z * Q.z);
    matrix[1][2] = (T(2.0) * Q.y * Q.z) - (T(2.0) * Q.w * Q.x);
    cx = matrix[1][1];
    sx = -matrix[1][2];
    euler.x = (T)atan2(sx,cx);

    cz = T(1.0);
    sz = T(0.0);
    euler.z = (T)atan2(sz,cz);
  }

  return euler;
}

///////////////////////////////////////////////
template <typename T>
sQuat<T>& QuatRandom(sQuat<T>& aQ)
{
  // From: "Uniform Random Rotations", Ken Shoemake, Graphics Gems III,
  //       pg. 124-132
  T x0 = ni::RandFloat();
  T r1 = ::sqrtf(T(1.0) - x0);
  T r2 = ::sqrtf(x0);
  T t1 = ni::k2Pi * ni::RandFloat();
  T t2 = ni::k2Pi * ni::RandFloat();
  T c1 = ::cosf(t1);
  T s1 = ::sinf(t1);
  T c2 = ::cosf(t2);
  T s2 = ::sinf(t2);
  aQ.x = s1*r1;
  aQ.y = c1*r1;
  aQ.z = s2*r2;
  aQ.w = c2*r2;
  return aQ;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationZYX(sQuat<T>& Out, T rx, T ry, T rz)
{
  sQuat<T> tmp; // QX is Out
  QuatRotationZ(Out, rz);
  Out *= QuatRotationY(tmp,ry);
  Out *= QuatRotationX(tmp,rx);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationZYX(sQuat<T>& Out, const sVec3<T>& V)
{
  return QuatRotationZYX(Out, V.x, V.y, V.z);
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationXYZ(sQuat<T>& Out, T rx, T ry, T rz)
{
  sQuat<T> tmp; // QX is Out
  QuatRotationX(Out, rx);
  Out *= QuatRotationY(tmp,ry);
  Out *= QuatRotationZ(tmp,rz);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sQuat<T>& QuatRotationXYZ(sQuat<T>& Out, const sVec3<T>& V)
{
  return QuatRotationXYZ(Out, V.x, V.y, V.z);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHQUATERNION_36380400_H__
