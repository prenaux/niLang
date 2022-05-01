#ifndef __MATHMATRIX_8300436_H__
#define __MATHMATRIX_8300436_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "MathVec3.h"
#include "MathQuat.h"
#include "MathUtils.h"
#include "MathRect.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

///////////////////////////////////////////////
template <class T>
void MathUtilMatrixFromQuat(T &r11, T &r12, T &r13,
                            T &r21, T &r22, T &r23,
                            T &r31, T &r32, T &r33,
                            const sQuat<T> &q)
{
  T x, y, z, w, wx, wy, wz, xx, xy, xz, yy, yz, zz;

  x  = 2.0f * q.x;
  y  = 2.0f * q.y;
  z  = 2.0f * q.z;
  w  = q.w;

  wx = q.w * x;
  wy = q.w * y;
  wz = q.w * z;
  xx = q.x * x;
  xy = q.x * y;
  xz = q.x * z;
  yy = q.y * y;
  yz = q.y * z;
  zz = q.z * z;

  // U axis
  r11 = 1.0f - (yy + zz);
  r21 =         xy - wz;
  r31 =         xz + wy;

  // V axis
  r12 =         xy + wz;
  r22 = 1.0f - (xx + zz);
  r32 =         yz - wx;

  // N axis
  r13 =         xz - wy;
  r23 =         yz + wx;
  r33 = 1.0f - (xx + yy);
}

///////////////////////////////////////////////
template <class T>
void MathUtilPostTranslate(sMatrix<T> &m, const T tx, const T ty, const T tz)
{
  m._11 += m._14 * tx; m._21 += m._24 * tx; m._31 += m._34 * tx; m._41 += m._44 * tx;
  m._12 += m._14 * ty; m._22 += m._24 * ty; m._32 += m._34 * ty; m._42 += m._44 * ty;
  m._13 += m._14 * tz; m._23 += m._24 * tz; m._33 += m._34 * tz; m._43 += m._44 * tz;
}

///////////////////////////////////////////////
template <class T>
void MathUtilPostRotateQuat(sMatrix<T> &m, const sQuat<T> &q)
{
  T t1, t2;
  T r11, r12, r13, r21, r22, r23, r31, r32, r33;

  MathUtilMatrixFromQuat(r11, r12, r13,
                         r21, r22, r23,
                         r31, r32, r33,
                         q);

  t1  = m._11; t2 = m._12;
  m._11 =  t1 * r11 + t2 * r21 + m._13 * r31;
  m._12 =  t1 * r12 + t2 * r22 + m._13 * r32;
  m._13 =  t1 * r13 + t2 * r23 + m._13 * r33;

  t1  = m._21; t2 = m._22;
  m._21 =  t1 * r11 + t2 * r21 + m._23 * r31;
  m._22 =  t1 * r12 + t2 * r22 + m._23 * r32;
  m._23 =  t1 * r13 + t2 * r23 + m._23 * r33;

  t1  = m._31; t2 = m._32;
  m._31 =  t1 * r11 + t2 * r21 + m._33 * r31;
  m._32 =  t1 * r12 + t2 * r22 + m._33 * r32;
  m._33 =  t1 * r13 + t2 * r23 + m._33 * r33;

  t1  = m._41; t2 = m._42;
  m._41 =  t1 * r11 + t2 * r21 + m._43 * r31;
  m._42 =  t1 * r12 + t2 * r22 + m._43 * r32;
  m._43 =  t1 * r13 + t2 * r23 + m._43 * r33;
}

///////////////////////////////////////////////
template <class T>
void MathUtilPostRotateQuatInverse(sMatrix<T> &m, const sQuat<T> &q)
{
  T t1, t2;
  T r11, r12, r13, r21, r22, r23, r31, r32, r33;

  MathUtilMatrixFromQuat(r11, r21, r31,
                         r12, r22, r32,
                         r13, r23, r33,
                         q);

  t1  = m._11; t2 = m._12;
  m._11 =  t1 * r11 + t2 * r21 + m._13 * r31;
  m._12 =  t1 * r12 + t2 * r22 + m._13 * r32;
  m._13 =  t1 * r13 + t2 * r23 + m._13 * r33;

  t1  = m._21; t2 = m._22;
  m._21 =  t1 * r11 + t2 * r21 + m._23 * r31;
  m._22 =  t1 * r12 + t2 * r22 + m._23 * r32;
  m._23 =  t1 * r13 + t2 * r23 + m._23 * r33;

  t1  = m._31; t2 = m._32;
  m._31 =  t1 * r11 + t2 * r21 + m._33 * r31;
  m._32 =  t1 * r12 + t2 * r22 + m._33 * r32;
  m._33 =  t1 * r13 + t2 * r23 + m._33 * r33;

  t1  = m._41; t2 = m._42;
  m._41 =  t1 * r11 + t2 * r21 + m._43 * r31;
  m._42 =  t1 * r12 + t2 * r22 + m._43 * r32;
  m._43 =  t1 * r13 + t2 * r23 + m._43 * r33;
}

///////////////////////////////////////////////
template <class T>
void MathUtilPostScale(sMatrix<T> &m, const T sx, const T sy, const T sz)
{
  m._11 *= sx; m._21 *= sx; m._31 *= sx; m._41 *= sx;
  m._12 *= sy; m._22 *= sy; m._32 *= sy; m._42 *= sy;
  m._13 *= sz; m._23 *= sz; m._33 *= sz; m._43 *= sz;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixAffineTransformation(sMatrix<T>& Out,
                                       T scaling,
                                       const sVec3<T>* pRotationCenter = NULL,
                                       const sQuat<T>* pRotation = NULL,
                                       const sVec3<T>* pTranslation = NULL)
{
  MatrixScaling(Out, scaling, scaling, scaling);

  if (pRotation)
  {
    if (pRotationCenter)
      MathUtilPostTranslate(Out, -pRotationCenter->x, -pRotationCenter->y, -pRotationCenter->z);

    MathUtilPostRotateQuat(Out, *pRotation);

    if (pRotationCenter)
      MathUtilPostTranslate(Out, pRotationCenter->x, pRotationCenter->y, pRotationCenter->z);
  }

  if (pTranslation)
    MathUtilPostTranslate(Out, pTranslation->x, pTranslation->y, pTranslation->z);

  return Out;
}

///////////////////////////////////////////////
template <typename T>
T MatrixDeterminant2(const sMatrix<T>& M)
{
  return Det2x2(M._11, M._12, M._21, M._22);
}

///////////////////////////////////////////////
template <typename T>
T MatrixDeterminant3(const sMatrix<T>& M)
{
  return Det3x3(M._11, M._12, M._13,
                M._21, M._22, M._23,
                M._31, M._32, M._33);
}

///////////////////////////////////////////////
template <typename T>
T MatrixDeterminant(const sMatrix<T>& M)
{
  return Det4x4(
      M._11, M._12, M._13, M._14,
      M._21, M._22, M._23, M._24,
      M._31, M._32, M._33, M._34,
      M._41, M._42, M._43, M._44);
}

///////////////////////////////////////////////
template <typename T>
T MatrixGetHandeness(const sMatrix<T>& M)
{
  return MatrixDeterminant(M) > 0.0f ? 1.0f : -1.0f;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixIdentity(sMatrix<T>& Out)
{
  Out._11 = 1.0f; Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._22 = 1.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f; Out._33 = 1.0f; Out._34 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f; Out._43 = 0.0f; Out._44 = 1.0f;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixInverse(sMatrix<T>& aOut, const sMatrix<T>& aIn, T* apDet = NULL)
{
  const T* pIn;
  sMatrix<T> t;
  if (tPtr(&aOut) == tPtr(&aIn)) {
    t = aIn;
    pIn = t.ptr();
  }
  else {
    pIn = aIn.ptr();
  }
  const tF64 fA0 = pIn[ 0]*pIn[ 5] - pIn[ 1]*pIn[ 4];
  const tF64 fA1 = pIn[ 0]*pIn[ 6] - pIn[ 2]*pIn[ 4];
  const tF64 fA2 = pIn[ 0]*pIn[ 7] - pIn[ 3]*pIn[ 4];
  const tF64 fA3 = pIn[ 1]*pIn[ 6] - pIn[ 2]*pIn[ 5];
  const tF64 fA4 = pIn[ 1]*pIn[ 7] - pIn[ 3]*pIn[ 5];
  const tF64 fA5 = pIn[ 2]*pIn[ 7] - pIn[ 3]*pIn[ 6];
  const tF64 fB0 = pIn[ 8]*pIn[13] - pIn[ 9]*pIn[12];
  const tF64 fB1 = pIn[ 8]*pIn[14] - pIn[10]*pIn[12];
  const tF64 fB2 = pIn[ 8]*pIn[15] - pIn[11]*pIn[12];
  const tF64 fB3 = pIn[ 9]*pIn[14] - pIn[10]*pIn[13];
  const tF64 fB4 = pIn[ 9]*pIn[15] - pIn[11]*pIn[13];
  const tF64 fB5 = pIn[10]*pIn[15] - pIn[11]*pIn[14];

  const tF64 fDet = fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
  if (apDet) {
    *apDet = (T)fDet;
  }
  if (ni::Abs(fDet) <= ni::TypeEpsilon<tF64>()) {
    return aOut;
  }

  const tF64 fInvDet = 1.0/fDet;
  aOut(0,0) = (+ pIn[ 5]*fB5 - pIn[ 6]*fB4 + pIn[ 7]*fB3) * fInvDet;
  aOut(1,0) = (- pIn[ 4]*fB5 + pIn[ 6]*fB2 - pIn[ 7]*fB1) * fInvDet;
  aOut(2,0) = (+ pIn[ 4]*fB4 - pIn[ 5]*fB2 + pIn[ 7]*fB0) * fInvDet;
  aOut(3,0) = (- pIn[ 4]*fB3 + pIn[ 5]*fB1 - pIn[ 6]*fB0) * fInvDet;
  aOut(0,1) = (- pIn[ 1]*fB5 + pIn[ 2]*fB4 - pIn[ 3]*fB3) * fInvDet;
  aOut(1,1) = (+ pIn[ 0]*fB5 - pIn[ 2]*fB2 + pIn[ 3]*fB1) * fInvDet;
  aOut(2,1) = (- pIn[ 0]*fB4 + pIn[ 1]*fB2 - pIn[ 3]*fB0) * fInvDet;
  aOut(3,1) = (+ pIn[ 0]*fB3 - pIn[ 1]*fB1 + pIn[ 2]*fB0) * fInvDet;
  aOut(0,2) = (+ pIn[13]*fA5 - pIn[14]*fA4 + pIn[15]*fA3) * fInvDet;
  aOut(1,2) = (- pIn[12]*fA5 + pIn[14]*fA2 - pIn[15]*fA1) * fInvDet;
  aOut(2,2) = (+ pIn[12]*fA4 - pIn[13]*fA2 + pIn[15]*fA0) * fInvDet;
  aOut(3,2) = (- pIn[12]*fA3 + pIn[13]*fA1 - pIn[14]*fA0) * fInvDet;
  aOut(0,3) = (- pIn[ 9]*fA5 + pIn[10]*fA4 - pIn[11]*fA3) * fInvDet;
  aOut(1,3) = (+ pIn[ 8]*fA5 - pIn[10]*fA2 + pIn[11]*fA1) * fInvDet;
  aOut(2,3) = (- pIn[ 8]*fA4 + pIn[ 9]*fA2 - pIn[11]*fA0) * fInvDet;
  aOut(3,3) = (+ pIn[ 8]*fA3 - pIn[ 9]*fA1 + pIn[10]*fA0) * fInvDet;
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixTransformInverse(sMatrix<T>& Out, const sMatrix<T>& M)
{
  // Transpose the Rotational portion of the matrix
  Out._11 = M._11; Out._12 = M._21; Out._13 = M._31;
  Out._21 = M._12; Out._22 = M._22; Out._23 = M._32;
  Out._31 = M._13; Out._32 = M._23; Out._33 = M._33;

  // Fix the right colum
  Out._14 = 0.0f;
  Out._24 = 0.0f;
  Out._34 = 0.0f;
  Out._44 = 1.0f;

  // Inverse the Translation portion
  Out._41 = -(M._41 * M._11 + M._42 * M._12 + M._43 * M._13);
  Out._42 = -(M._41 * M._21 + M._42 * M._22 + M._43 * M._23);
  Out._43 = -(M._41 * M._31 + M._42 * M._32 + M._43 * M._33);

  return Out;
}

///////////////////////////////////////////////
template <typename T>
tBool MatrixIsIdentity(const sMatrix<T>& M)
{
  if(   M._11 == 1.0f && M._12 == 0.0f && M._13 == 0.0f && M._14 == 0.0f
        &&  M._21 == 0.0f && M._22 == 1.0f && M._23 == 0.0f && M._24 == 0.0f
        &&  M._31 == 0.0f && M._32 == 0.0f && M._33 == 1.0f && M._34 == 0.0f
        &&  M._41 == 0.0f && M._42 == 0.0f && M._43 == 0.0f && M._44 == 1.0f)
  {
    return eTrue;
  }

  return eFalse;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixLookAtRH(sMatrix<T>& Out,
                           const sVec3<T>& Eye,
                           const sVec3<T>& At,
                           const sVec3<T>& Up)
{
  sVec3<T>   uAxis, vAxis, nAxis;

  nAxis = -(At - Eye);
  VecNormalize(nAxis);

  vAxis = Up - nAxis * VecDot(Up,nAxis);

  if (VecLength(vAxis) < niEpsilon5)
  {
    vAxis.x =      - nAxis.y * nAxis.x;
    vAxis.y = 1.0f - nAxis.y * nAxis.y;
    vAxis.z =      - nAxis.y * nAxis.z;

    if (VecLength(vAxis) < niEpsilon5)
    {
      vAxis.x =      - nAxis.z * nAxis.x;
      vAxis.y =      - nAxis.z * nAxis.y;
      vAxis.z = 1.0f - nAxis.z * nAxis.z;
    }
  }

  VecNormalize(vAxis);
  VecCross(uAxis, vAxis, nAxis);

  Out._11 = uAxis.x; Out._12 = vAxis.x; Out._13 = nAxis.x; Out._14 = 0.0f;
  Out._21 = uAxis.y; Out._22 = vAxis.y; Out._23 = nAxis.y; Out._24 = 0.0f;
  Out._31 = uAxis.z; Out._32 = vAxis.z; Out._33 = nAxis.z; Out._34 = 0.0f;

  Out._41 = -VecDot(Eye, uAxis);
  Out._42 = -VecDot(Eye, vAxis);
  Out._43 = -VecDot(Eye, nAxis);
  Out._44 = 1.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixLookAtLH(sMatrix<T>& Out,
                           const sVec3<T>& Eye,
                           const sVec3<T>& At,
                           const sVec3<T>& Up)
{
  sVec3<T>   uAxis, vAxis, nAxis;

  nAxis = At - Eye;
  VecNormalize(nAxis);

  vAxis = Up - nAxis * VecDot(Up,nAxis);

  if (VecLength(vAxis) < niEpsilon5)
  {
    vAxis.x =      - nAxis.y * nAxis.x;
    vAxis.y = 1.0f - nAxis.y * nAxis.y;
    vAxis.z =      - nAxis.y * nAxis.z;

    if (VecLength(vAxis) < niEpsilon5)
    {
      vAxis.x =      - nAxis.z * nAxis.x;
      vAxis.y =      - nAxis.z * nAxis.y;
      vAxis.z = 1.0f - nAxis.z * nAxis.z;
    }
  }

  VecNormalize(vAxis);
  VecCross(uAxis, vAxis, nAxis);

  Out._11 = uAxis.x; Out._12 = vAxis.x; Out._13 = nAxis.x; Out._14 = 0.0f;
  Out._21 = uAxis.y; Out._22 = vAxis.y; Out._23 = nAxis.y; Out._24 = 0.0f;
  Out._31 = uAxis.z; Out._32 = vAxis.z; Out._33 = nAxis.z; Out._34 = 0.0f;

  Out._41 = -VecDot(Eye, uAxis);
  Out._42 = -VecDot(Eye, vAxis);
  Out._43 = -VecDot(Eye, nAxis);
  Out._44 = 1.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixMultiply(sMatrix<T>& Out, const sMatrix<T>& M1, const sMatrix<T>& M2)
{
  T       a1, a2, a3, a4;
  const sMatrix<T>* pA;
  const sMatrix<T>* pB;
  sMatrix<T> A, B;

  if (((sMatrix<T>*)(&M1)) == ((sMatrix<T>*)(&Out)))
  {
    A  = M1;
    pA = &A;
  }
  else
  {
    pA = &M1;
  }

  if (((sMatrix<T>*)(&M2)) == ((sMatrix<T>*)(&Out)))
  {
    B  = M2;
    pB = &B;
  }
  else
  {
    pB = &M2;
  }

  // 1st row * 1st column
  a1 = pA->_11; a2 = pA->_12; a3 = pA->_13; a4 = pA->_14;
  Out._11 = a1 * pB->_11 + a2 * pB->_21 + a3 * pB->_31 + a4 * pB->_41;
  Out._12 = a1 * pB->_12 + a2 * pB->_22 + a3 * pB->_32 + a4 * pB->_42;
  Out._13 = a1 * pB->_13 + a2 * pB->_23 + a3 * pB->_33 + a4 * pB->_43;
  Out._14 = a1 * pB->_14 + a2 * pB->_24 + a3 * pB->_34 + a4 * pB->_44;

  // 2nd row * 2nd column
  a1 = pA->_21; a2 = pA->_22; a3 = pA->_23; a4 = pA->_24;
  Out._21 = a1 * pB->_11 + a2 * pB->_21 + a3 * pB->_31 + a4 * pB->_41;
  Out._22 = a1 * pB->_12 + a2 * pB->_22 + a3 * pB->_32 + a4 * pB->_42;
  Out._23 = a1 * pB->_13 + a2 * pB->_23 + a3 * pB->_33 + a4 * pB->_43;
  Out._24 = a1 * pB->_14 + a2 * pB->_24 + a3 * pB->_34 + a4 * pB->_44;

  // 3rd row * 3rd column
  a1 = pA->_31; a2 = pA->_32; a3 = pA->_33; a4 = pA->_34;
  Out._31 = a1 * pB->_11 + a2 * pB->_21 + a3 * pB->_31 + a4 * pB->_41;
  Out._32 = a1 * pB->_12 + a2 * pB->_22 + a3 * pB->_32 + a4 * pB->_42;
  Out._33 = a1 * pB->_13 + a2 * pB->_23 + a3 * pB->_33 + a4 * pB->_43;
  Out._34 = a1 * pB->_14 + a2 * pB->_24 + a3 * pB->_34 + a4 * pB->_44;

  // 4th row * 4th column
  a1 = pA->_41; a2 = pA->_42; a3 = pA->_43; a4 = pA->_44;
  Out._41 = a1 * pB->_11 + a2 * pB->_21 + a3 * pB->_31 + a4 * pB->_41;
  Out._42 = a1 * pB->_12 + a2 * pB->_22 + a3 * pB->_32 + a4 * pB->_42;
  Out._43 = a1 * pB->_13 + a2 * pB->_23 + a3 * pB->_33 + a4 * pB->_43;
  Out._44 = a1 * pB->_14 + a2 * pB->_24 + a3 * pB->_34 + a4 * pB->_44;

  return Out;
}
template <typename T>
sMatrix<T>* MatrixMultiply(sMatrix<T>* Out, const sMatrix<T>* M1, const sMatrix<T>* M2) {
  return &MatrixMultiply(*Out,*M1,*M2);
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixOrthoRH(sMatrix<T>& Out, T w, T h, T zn, T zf)
{
  T nfRatio;

  niAssert(fabs(w) > niEpsilon5);
  niAssert(fabs(h) > niEpsilon5);

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = 1.0f / nfRatio;

  Out._11 = 2.0f / w;
  Out._22 = 2.0f / h;
  Out._33 = -nfRatio;
  Out._43 = -nfRatio * zn;
  Out._44 = 1.0f;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f; Out._34 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixOrthoLH(sMatrix<T>& Out, T w, T h, T zn, T zf)
{
  T nfRatio;

  niAssert(fabs(w) > niEpsilon5);
  niAssert(fabs(h) > niEpsilon5);

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = 1.0f / nfRatio;

  Out._11 = 2.0f / w;
  Out._22 = 2.0f / h;
  Out._33 = nfRatio;
  Out._43 = -nfRatio * zn;
  Out._44 = 1.0f;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f; Out._34 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixOrthoOffCenterRH(sMatrix<T>& Out, T l, T r, T t, T b, T zn, T zf)
{
  T nfRatio, width, height;

  width  = r - l;
  height = b - t;

  niAssert(fabs(width)  > niEpsilon5);
  niAssert(fabs(height) > niEpsilon5);

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = 1.0f / nfRatio;

  Out._11 = 2.0f / width;
  Out._22 = 2.0f / height;
  Out._33 = -nfRatio;
  Out._41 = -(l + r) / width;
  Out._42 = -(t + b) / height;
  Out._43 = -nfRatio * zn;
  Out._44 = 1.0f;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f; Out._34 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixOrthoOffCenterLH(sMatrix<T>& Out, T l, T r, T t, T b, T zn, T zf)
{
  T nfRatio, width, height;

  width  = r - l;
  height = b - t;

  niAssert(fabs(width)  > niEpsilon5);
  niAssert(fabs(height) > niEpsilon5);

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = 1.0f / nfRatio;

  Out._11 = 2.0f / width;
  Out._22 = 2.0f / height;
  Out._33 = nfRatio;
  Out._41 = -(l + r) / width;
  Out._42 = -(t + b) / height;
  Out._43 = -nfRatio * zn;
  Out._44 = 1.0f;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f; Out._34 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixPerspectiveRH(sMatrix<T>& Out, T w, T h, T zn, T zf)
{
  T   nfRatio;

  niAssert(fabs(w) > niEpsilon5);
  niAssert(fabs(h) > niEpsilon5);

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = zf / nfRatio;

  Out._11 = (2.0f * zn) / w;
  Out._22 = (2.0f * zn) / h;
  Out._33 = -nfRatio;
  Out._34 = -1.0f;
  Out._43 = -nfRatio * zn;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f; Out._44 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixPerspectiveLH(sMatrix<T>& Out, T w, T h, T zn, T zf)
{
  T   nfRatio;

  niAssert(fabs(w) > niEpsilon5);
  niAssert(fabs(h) > niEpsilon5);

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = zf / nfRatio;

  Out._11 = (2.0f * zn) / w;
  Out._22 = (2.0f * zn) / h;
  Out._33 = nfRatio;
  Out._34 = 1.0f;
  Out._43 = -nfRatio * zn;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f; Out._44 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixPerspectiveFovRH(sMatrix<T>& Out, T fovy, T aspect, T zn, T zf)
{
  T nfRatio, cotFovY;

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = zf / nfRatio;

  fovy *= 0.5f;
  cotFovY = T(sin(fovy));
  niAssert(cotFovY != 0);
  cotFovY = T(cos(fovy)) / cotFovY;

  Out._11 = cotFovY / aspect;
  Out._22 = cotFovY;
  Out._33 = -nfRatio;
  Out._34 = -1.0f;
  Out._43 = -nfRatio * zn;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f; Out._44 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixPerspectiveFovLH(sMatrix<T>& Out, T fovy, T aspect, T zn, T zf)
{
  T nfRatio, cotFovY;

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = zf / nfRatio;

  fovy *= 0.5f;
  cotFovY = T(sin(fovy));
  niAssert(cotFovY != 0);
  cotFovY = T(cos(fovy)) / cotFovY;


  Out._11 = cotFovY / aspect;
  Out._22 = cotFovY;
  Out._33 = nfRatio;
  Out._34 = 1.0f;
  Out._43 = -nfRatio * zn;


  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._31 = 0.0f; Out._32 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f; Out._44 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixPerspectiveOffCenterRH(sMatrix<T>& Out, T l, T r, T t, T b, T zn, T zf)
{
  T nfRatio, width, height;

  width  = r - l;
  height = b - t;

  niAssert(fabs(width)  > niEpsilon5);
  niAssert(fabs(height) > niEpsilon5);

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = zf / nfRatio;

  Out._11 = (2.0f * zn) / width;
  Out._22 = (2.0f * zn) / height;
  Out._31 = (l + r) / width;
  Out._32 = (t + b) / height;
  Out._33 = -nfRatio;
  Out._34 = -1.0f;
  Out._43 = -nfRatio * zn;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f; Out._44 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixPerspectiveOffCenterLH(sMatrix<T>& Out, T l, T r, T t, T b, T zn, T zf)
{
  T nfRatio, width, height;

  width  = r - l;
  height = b - t;

  niAssert(fabs(width)  > niEpsilon5);
  niAssert(fabs(height) > niEpsilon5);

  nfRatio = zf - zn;
  niAssert(nfRatio != 0);
  nfRatio = zf / nfRatio;

  Out._11 = (2.0f * zn) / width;
  Out._22 = (2.0f * zn) / height;
  Out._31 = -(l + r) / width;
  Out._32 = -(t + b) / height;
  Out._33 = nfRatio;
  Out._34 = 1.0f;
  Out._43 = -nfRatio * zn;

  Out._12 = 0.0f; Out._13 = 0.0f; Out._14 = 0.0f;
  Out._21 = 0.0f; Out._23 = 0.0f; Out._24 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f; Out._44 = 0.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixReflect(sMatrix<T>& Out, const sVec4<T>& Plane)
{
  T   a, b, c, d, tx, ty, tz, mag;

  a = Plane.x;
  b = Plane.y;
  c = Plane.z;
  d = Plane.w;

  mag = a * a + b * b + c * c;
  if (fabs(1.0f - mag) > niEpsilon5)
  {
    mag = (T)sqrt(mag);
    niAssert(mag != 0);
    mag = 1.0f / mag;

    a *= mag; b *= mag; c *= mag; d *= mag;
  }

  tx = a * d; ty = b * d; tz = c * d;

  Out._11 = -(2.0f * a * a - 1.0f);
  Out._21 = -(2.0f * a * b);
  Out._31 = -(2.0f * a * c);

  Out._12 = -(2.0f * b * a);
  Out._22 = -(2.0f * b * b - 1.0f);
  Out._32 = -(2.0f * b * c);

  Out._13 = -(2.0f * c * a);
  Out._23 = -(2.0f * c * b);
  Out._33 = -(2.0f * c * c - 1.0f);

  Out._41 = 2.0f * (Out._11 * tx + Out._21 * ty + Out._31 * tz);
  Out._42 = 2.0f * (Out._12 * tx + Out._22 * ty + Out._32 * tz);
  Out._43 = 2.0f * (Out._13 * tx + Out._23 * ty + Out._33 * tz);

  Out._14 = 0.0f; Out._24 = 0.0f; Out._34 = 0.0f; Out._44 = 1.0f;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
ni::sMatrix<T>& MatrixRotationAxis(ni::sMatrix<T>& Out, const ni::sVec3<T>& V, T angle)
{
  const T sqLen = VecLengthSq(V);
  if (ni::FuzzyEqual(sqLen,T(0),T(niEpsilon5))) {
    Out = ni::sMatrix<T>::Identity();
    return Out;
  }

  // normalize
  const T len = ni::Sqrt(sqLen);
  const T ilen = T(1.0)/len;
  const T x = V.x*ilen;
  const T y = V.y*ilen;
  const T z = V.z*ilen;

  // negate the angle to rotate clockwise (the algo rotates counter-clockwise)
  const T fSin = ni::Sin(angle);
  const T fCos = ni::Cos(angle);
  const T fInvCos = 1.0f - fCos;

  Out._11 = fInvCos * x * x + fCos;
  Out._12 = fInvCos * x * y - z * fSin;
  Out._13 = fInvCos * x * z + y * fSin;
  Out._14 = 0.0f;

  Out._21 = fInvCos * x * y + z * fSin;
  Out._22 = fInvCos * y * y + fCos;
  Out._23 = fInvCos * y * z - x * fSin;
  Out._24 = 0.0f;

  Out._31 = fInvCos * x * z - y * fSin;
  Out._32 = fInvCos * y * z + x * fSin;
  Out._33 = fInvCos * z * z + fCos;
  Out._34 = 0.0f;

  Out._41 = 0.0f; Out._42 = 0.0f; Out._43 = 0.0f; Out._44 = 1.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationQuat(sMatrix<T>& Out, const sQuat<T>& Q)
{
  MathUtilMatrixFromQuat(
      Out._11, Out._12, Out._13,
      Out._21, Out._22, Out._23,
      Out._31, Out._32, Out._33,
      Q);

  Out._41 = 0.0f; Out._42 = 0.0f; Out._43 = 0.0f;
  Out._14 = 0.0f; Out._24 = 0.0f; Out._34 = 0.0f; Out._44 = 1.0f;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationX(sMatrix<T>& Out, T angle)
{
  T cosAngle, sinAngle;

  cosAngle = T(cos(angle));
  sinAngle = T(sin(angle));

  MatrixIdentity(Out);

  Out._22 =  cosAngle; Out._23 = -sinAngle;
  Out._32 =  sinAngle; Out._33 =  cosAngle;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationY(sMatrix<T>& Out, T angle)
{
  T cosAngle, sinAngle;

  cosAngle = T(cos(angle));
  sinAngle = T(sin(angle));

  MatrixIdentity(Out);

  Out._11 =  cosAngle; Out._13 =  sinAngle;
  Out._31 = -sinAngle; Out._33 =  cosAngle;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationZ(sMatrix<T>& Out, T angle)
{
  T cosAngle, sinAngle;

  cosAngle = T(cos(angle));
  sinAngle = T(sin(angle));

  MatrixIdentity(Out);

  Out._11 =  cosAngle; Out._12 = -sinAngle;
  Out._21 =  sinAngle; Out._22 =  cosAngle;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationYawPitchRoll(sMatrix<T>& Out, T yaw, T pitch, T roll)
{
  T sinY, cosY, sinP, cosP, sinR, cosR;
  T ux, uy, uz, vx, vy, vz, nx, ny, nz;

  sinY = T(sin(yaw));
  cosY = T(cos(yaw));

  sinP = T(sin(pitch));
  cosP = T(cos(pitch));

  sinR = T(sin(roll));
  cosR = T(cos(roll));

  ux = cosY * cosR + sinY * sinP * sinR;
  uy = sinR * cosP;
  uz = cosY * sinP * sinR - sinY * cosR;

  vx = sinY * sinP * cosR - cosY * sinR;
  vy = cosR * cosP;
  vz = sinR * sinY + cosR * cosY * sinP;

  nx = cosP * sinY;
  ny = -sinP;
  nz = cosP * cosY;

  Out._11 =   ux; Out._12 =   uy; Out._13 =   uz; Out._14 = 0.0f;
  Out._21 =   vx; Out._22 =   vy; Out._23 =   vz; Out._24 = 0.0f;
  Out._31 =   nx; Out._32 =   ny; Out._33 =   nz; Out._34 = 0.0f;
  Out._41 = 0.0f; Out._42 = 0.0f; Out._43 = 0.0f; Out._44 = 1.0f;

  return Out;
}

///////////////////////////////////////////////
// Extract the rotation contained in the provided matrix as yaw/Y (heading),
// pitch/X and roll/bank/Z in radians.
template <typename T>
inline sVec3<T> MatrixDecomposeYawPitchRoll(const sMatrix<T>& mat) {
  sVec3<T> euler;
  euler.x = ni::ASin(-mat._32);          // Pitch
  if (ni::ACos(euler.x) > T(0.0001)) {   // Not at poles
    euler.y = atan2f(mat._31, mat._33);  // Yaw
    euler.z = atan2f(mat._12, mat._22);  // Roll
  }
  else {
    euler.y = 0.0f;                      // Yaw
    euler.z = atan2f(-mat._21, mat._11); // Roll
  }
  return euler;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixScaling(sMatrix<T>& Out, T sx, T sy, T sz)
{
  MatrixIdentity(Out);
  Out._11 = sx; Out._22 = sy; Out._33 = sz;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixScaling(sMatrix<T>& Out, const sVec3<T>& V)
{
  MatrixIdentity(Out);
  Out._11 = V.x; Out._22 = V.y; Out._33 = V.z;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixScale(sMatrix<T>& Out, const sMatrix<T>& M, const T x, const T y, const T z)
{
  // Right
  Out._11 = M._11 * x;
  Out._21 = M._21 * x;
  Out._31 = M._31 * x;

  // Up
  Out._12 = M._12 * y;
  Out._22 = M._22 * y;
  Out._32 = M._32 * y;

  // Fwd
  Out._13 = M._13 * z;
  Out._23 = M._23 * z;
  Out._33 = M._33 * z;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixScale(sMatrix<T>& Out, const sMatrix<T>& M, const sVec3<T>& V)
{
  return MatrixScale(Out,M,V.x,V.y,V.z);
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixScale(sMatrix<T>& Out, const sMatrix<T>& M, const T s)
{
  return MatrixScale(Out,M,s,s,s);
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixShadow(sMatrix<T>& Out,
                         const sVec4<T>& vLight,
                         const sVec4<T>& Plane)
{
  T dot, a, b, c, d, x, y, z, w, mag;

  a = Plane.A(); b = Plane.B(); c = Plane.C(); d = Plane.D();

  mag = a * a + b * b + c * c;
  if (fabs(1.0f - mag) > niEpsilon5)
  {
    mag = (T)sqrt(mag);
    niAssert(mag != 0);
    mag = 1.0f / mag;

    a *= mag; b *= mag; c *= mag; d *= mag;
  }

  x = vLight.x; y = vLight.y; z = vLight.z; w = vLight.w;

  dot = a * x + b * y + c * z + d * w;

  Out._11 = dot - x * a;
  Out._21 =     - x * b;
  Out._31 =     - x * c;
  Out._41 =     - x * d;

  Out._12 =     - y * a;
  Out._22 = dot - y * b;
  Out._32 =     - y * c;
  Out._42 =     - y * d;

  Out._13 =     - z * a;
  Out._23 =     - z * b;
  Out._33 = dot - z * c;
  Out._43 =     - z * d;

  Out._14 =     - w * a;
  Out._24 =     - w * b;
  Out._34 =     - w * c;
  Out._44 = dot - w * d;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixTransformation(sMatrix<T>& Out,
                                 const sVec3<T> *pScalingCenter,
                                 const sQuat<T> *pScalingRotation,
                                 const sVec3<T> *pScaling,
                                 const sVec3<T> *pRotationCenter,
                                 const sQuat<T> *pRotation,
                                 const sVec3<T> *pTranslation)
{
  MatrixIdentity(Out);

  if (pScaling)
  {
    if (pScalingCenter)
    {
      MathUtilPostScale(Out, -pScalingCenter->x, -pScalingCenter->y, -pScalingCenter->z);
    }

    if (pScalingRotation)
    {
      MathUtilPostRotateQuatInverse(Out, *pScalingRotation);
    }

    MathUtilPostScale(Out, pScaling->x, pScaling->y, pScaling->z);

    if (pScalingRotation)
    {
      MathUtilPostRotateQuat(Out, *pScalingRotation);
    }

    if (pScalingCenter)
    {
      MathUtilPostScale(Out, pScalingCenter->x, pScalingCenter->y, pScalingCenter->z);
    }
  }

  if (pRotation)
  {
    if (pRotationCenter)
    {
      MathUtilPostTranslate(Out, -pRotationCenter->x, -pRotationCenter->y, -pRotationCenter->z);
    }

    MathUtilPostRotateQuat(Out, *pRotation);

    if (pRotationCenter)
    {
      MathUtilPostTranslate(Out, pRotationCenter->x, pRotationCenter->y, pRotationCenter->z);
    }
  }

  if (pTranslation)
  {
    MathUtilPostTranslate(Out, pTranslation->x, pTranslation->y, pTranslation->z);
  }

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixTranslation(sMatrix<T>& Out, T x, T y, T z)
{
  MatrixIdentity(Out);
  Out._41 = x; Out._42 = y; Out._43 = z;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixTranslation(sMatrix<T>& Out, const sVec3<T>& V)
{
  MatrixIdentity(Out);
  Out._41 = V.x; Out._42 = V.y; Out._43 = V.z;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixTranspose(sMatrix<T>& Out, const sMatrix<T>& M)
{
  if (((sMatrix<T>*)(&Out)) == ((sMatrix<T>*)(&M)))
  {
    Swap(Out._12, Out._21);
    Swap(Out._13, Out._31);
    Swap(Out._14, Out._41);
    Swap(Out._23, Out._32);
    Swap(Out._24, Out._42);
    Swap(Out._34, Out._43);
  }
  else
  {
    Out._11 = M._11; Out._21 = M._12; Out._31 = M._13; Out._41 = M._14;
    Out._12 = M._21; Out._22 = M._22; Out._32 = M._23; Out._42 = M._24;
    Out._13 = M._31; Out._23 = M._32; Out._33 = M._33; Out._43 = M._34;
    Out._14 = M._41; Out._24 = M._42; Out._34 = M._43; Out._44 = M._44;
  }

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixTranspose3x3(sMatrix<T>& Out, const sMatrix<T>& M)
{
  if (((sMatrix<T>*)(&Out)) == ((sMatrix<T>*)(&M)))
  {
    Swap(Out._12, Out._21);
    Swap(Out._13, Out._31);
    Swap(Out._23, Out._32);
  }
  else
  {
    Out._11 = M._11; Out._21 = M._12; Out._31 = M._13;
    Out._12 = M._21; Out._22 = M._22; Out._32 = M._23;
    Out._13 = M._31; Out._23 = M._32; Out._33 = M._33;
  }

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixTextureOffset2(sMatrix<T>& Out, T fBias, tI32 nTexW, tI32 nTexH, tI32 nDepthBits)
{
  T fOffsetX = 0.5f + (0.5f / (T)nTexW);
  T fOffsetY = 0.5f + (0.5f / (T)nTexH);
  tU32 range = 0xFFFFFFFF >> (32 - nDepthBits);
  Out._11 = T(0.5);   Out._12 = T( 0.0);    Out._13 = T(0.0);       Out._14 = T(0.0);
  Out._21 = T(0.0);   Out._22 = T(-0.5);    Out._23 = T(0.0);       Out._24 = T(0.0);
  Out._31 = T(0.0);   Out._32 = T( 0.0);    Out._33 = T(range);       Out._34 = T(0.0);
  Out._41 = T(fOffsetX);  Out._42 = T(fOffsetY);  Out._43 = T(fBias*T(range));  Out._44 = T(1.0);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixTextureOffset(sMatrix<T>& Out, T fBias, tI32 nTexW, tI32 nTexH)
{
  T fOffsetX = 0.5f + (0.5f / (T)nTexW);
  T fOffsetY = 0.5f + (0.5f / (T)nTexH);
  Out._11 = T(0.5);   Out._12 = T( 0.0);    Out._13 = T(0.0);       Out._14 = T(0.0);
  Out._21 = T(0.0);   Out._22 = T(-0.5);    Out._23 = T(0.0);       Out._24 = T(0.0);
  Out._31 = T(0.0);   Out._32 = T( 0.0);    Out._33 = T(1.0f);        Out._34 = T(0.0);
  Out._41 = T(fOffsetX);  Out._42 = T(fOffsetY);  Out._43 = T(fBias);       Out._44 = T(1.0);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixLerp(sMatrix<T>& Out, const sMatrix<T>& A, const sMatrix<T>& B, T fFac)
{
  Out[0] = ni::Lerp(A[0],B[0],fFac);
  Out[1] = ni::Lerp(A[1],B[1],fFac);
  Out[2] = ni::Lerp(A[2],B[2],fFac);
  Out[3] = ni::Lerp(A[3],B[3],fFac);
  Out[4] = ni::Lerp(A[4],B[4],fFac);
  Out[5] = ni::Lerp(A[5],B[5],fFac);
  Out[6] = ni::Lerp(A[6],B[6],fFac);
  Out[7] = ni::Lerp(A[7],B[7],fFac);
  Out[8] = ni::Lerp(A[8],B[8],fFac);
  Out[9] = ni::Lerp(A[9],B[9],fFac);
  Out[10] = ni::Lerp(A[10],B[10],fFac);
  Out[11] = ni::Lerp(A[11],B[11],fFac);
  Out[12] = ni::Lerp(A[12],B[12],fFac);
  Out[13] = ni::Lerp(A[13],B[13],fFac);
  Out[14] = ni::Lerp(A[14],B[14],fFac);
  Out[15] = ni::Lerp(A[15],B[15],fFac);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixGetForward(sVec3<T>& Out, const sMatrix<T>& M)
{
  Out.x = M._13;
  Out.y = M._23;
  Out.z = M._33;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixGetUp(sVec3<T>& Out, const sMatrix<T>& M)
{
  Out.x = M._12;
  Out.y = M._22;
  Out.z = M._32;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixGetRight(sVec3<T>& Out, const sMatrix<T>& M)
{
  Out.x = M._11;
  Out.y = M._21;
  Out.z = M._31;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixGetTranslation(sVec3<T>& Out, const sMatrix<T>& M)
{
  Out.x = M._41;
  Out.y = M._42;
  Out.z = M._43;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixSetForward(sMatrix<T>& Out, const sVec3<T>& V)
{
  Out._13 = V.x;
  Out._23 = V.y;
  Out._33 = V.z;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixSetUp(sMatrix<T>& Out, const sVec3<T>& V)
{
  Out._12 = V.x;
  Out._22 = V.y;
  Out._32 = V.z;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixSetRight(sMatrix<T>& Out, const sVec3<T>& V)
{
  Out._11 = V.x;
  Out._21 = V.y;
  Out._31 = V.z;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixSetTranslation(sMatrix<T>& Out, const sVec3<T>& V)
{
  Out._41 = V.x;
  Out._42 = V.y;
  Out._43 = V.z;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
tBool MatrixIsNormal(const sMatrix<T>& M, T epsilon = niEpsilon5)
{
  sVec3<T> f,u,r;
  return  VecIsNormal(MatrixGetForward(f,M), epsilon) &&
      VecIsNormal(MatrixGetUp(u,M), epsilon) &&
      VecIsNormal(MatrixGetRight(r,M), epsilon);
}

///////////////////////////////////////////////
// return eTrue if matrix is orthogonal
template <typename T>
tBool MatrixIsOrthogonal(const sMatrix<T>& M, T epsilon = niEpsilon5)
{
  sVec3f vRight, vUp, vFwd;
  VecNormalize(MatrixGetRight(vRight, M));
  VecNormalize(MatrixGetUp(vUp, M));
  VecNormalize(MatrixGetForward(vFwd, M));
  return (Abs(VecDot(vRight,vUp)) <= epsilon &&
          Abs(VecDot(vRight,vFwd)) <= epsilon &&
          Abs(VecDot(vFwd,vUp)) <= epsilon);
}

///////////////////////////////////////////////
// return eTrue if matrix is orthonormal
template <typename T>
tBool MatrixIsOrthoNormal(const sMatrix<T>& M, T epsilon = niEpsilon5)
{
  sVec3f vRight, vUp, vFwd;
  MatrixGetRight(vRight, M);
  MatrixGetUp(vUp, M);
  MatrixGetForward(vFwd, M);

  if (!VecIsNormal(vRight) || !VecIsNormal(vUp) || !VecIsNormal(vFwd))
    return eFalse;

  return (Abs(VecDot(vRight,vUp)) <= epsilon &&
          Abs(VecDot(vRight,vFwd)) <= epsilon &&
          Abs(VecDot(vFwd,vUp)) <= epsilon);
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixSetRotation(sMatrix<T>& Out, const sMatrix<T>& M)
{
  Out._11 = M._11; Out._12 = M._12; Out._13 = M._13;
  Out._21 = M._21; Out._22 = M._22; Out._23 = M._23;
  Out._31 = M._31; Out._32 = M._32; Out._33 = M._33;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixToEuler(const sMatrix<T>& M, sVec3<T>& euler)
{
#define EulSafe      "\000\001\002\000"
#define EulNext      "\001\002\000\001"
#define EulGetOrd(ord,i,j,k,n,s,f) {unsigned o=ord;f=o&1;s=o&1; n=o&1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];}

#define EulOrd(i,p,r,f)    (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))

#define EulOrdXYZs    EulOrd(0,0,0,0)
#define EulOrdXYXs    EulOrd(0,0,1,0)
#define EulOrdXZYs    EulOrd(0,1,0,0)
#define EulOrdXZXs    EulOrd(0,1,1,0)
#define EulOrdYZXs    EulOrd(1,0,0,0)
#define EulOrdYZYs    EulOrd(1,0,1,0)
#define EulOrdYXZs    EulOrd(1,1,0,0)
#define EulOrdYXYs    EulOrd(1,1,1,0)
#define EulOrdZXYs    EulOrd(2,0,0,0)
#define EulOrdZXZs    EulOrd(2,0,1,0)
#define EulOrdZYXs    EulOrd(2,1,0,0)
#define EulOrdZYZs    EulOrd(2,1,1,0)

#define EulOrdZYXr    EulOrd(0,0,0,1)
#define EulOrdXYXr    EulOrd(0,0,1,1)
#define EulOrdYZXr    EulOrd(0,1,0,1)
#define EulOrdXZXr    EulOrd(0,1,1,1)
#define EulOrdXZYr    EulOrd(1,0,0,1)
#define EulOrdYZYr    EulOrd(1,0,1,1)
#define EulOrdZXYr    EulOrd(1,1,0,1)
#define EulOrdYXYr    EulOrd(1,1,1,1)
#define EulOrdYXZr    EulOrd(2,0,0,1)
#define EulOrdZXZr    EulOrd(2,0,1,1)
#define EulOrdXYZr    EulOrd(2,1,0,1)
#define EulOrdZYZr    EulOrd(2,1,1,1)

  int order = EulOrdXYZs;
  int i,j,k,n,s,f;

  EulGetOrd(order,i,j,k,n,s,f);

  if (s == 1)
  {
    T sy = (T)sqrt(M(j,i)*M(j,i) + M(k,i)*M(k,i));
    if (sy > T(0.016))
    {
      euler.x = (T)atan2(M(j,i), M(k,i));
      euler.y = (T)atan2(sy, M(i,i));
      euler.z = (T)atan2(M(i,j), -M(i,k));
    }
    else
    {
      euler.x = (T)atan2(-M(k,j), M(j,j));
      euler.y = (T)atan2(sy, M(i,i));
      euler.z = (T)0;
    }
  }
  else
  {
    T cy = (T)sqrt(M(i,i)*M(i,i) + M(i,j)*M(i,j));
    if (cy > T(0.016))
    {
      euler.x = (T)atan2(M(j,k), M(k,k));
      euler.y = (T)atan2(-M(i,k), cy);
      euler.z = (T)atan2(M(i,j), M(i,i));
    }
    else
    {
      euler.x = (T)atan2(-M(k,j), M(j,j));
      euler.y = (T)atan2(-M(i,k), cy);
      euler.z = (T)0;
    }
  }

  if ( n==1 ) { euler = -euler; }
  if ( f==1 ) { T t = euler.x; euler.x = euler.z; euler.z = t; }

  return euler;

#undef EulSafe
#undef EulNext
#undef EulGetOrd
#undef EulOrd
#undef EulOrdXYZs
#undef EulOrdXYXs
#undef EulOrdXZYs
#undef EulOrdXZXs
#undef EulOrdYZXs
#undef EulOrdYZYs
#undef EulOrdYXZs
#undef EulOrdYXYs
#undef EulOrdZXYs
#undef EulOrdZXZs
#undef EulOrdZYXs
#undef EulOrdZYZs
#undef EulOrdZYXr
#undef EulOrdXYXr
#undef EulOrdYZXr
#undef EulOrdXZXr
#undef EulOrdXZYr
#undef EulOrdYZYr
#undef EulOrdZXYr
#undef EulOrdYXYr
#undef EulOrdYXZr
#undef EulOrdZXZr
#undef EulOrdXYZr
#undef EulOrdZYZr
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixToCoordinateSystem(sMatrix<T>& Out,
                                     const sVec3<T>& avRight,
                                     const sVec3<T>& avUp,
                                     const sVec3<T>& avFwd,
                                     const sVec3<T>& avOrg = sVec3<T>::Zero())
{
  Out._11 = avRight.x;  Out._21 = avUp.x; Out._31 = avFwd.x;  Out._41 = avOrg.x;
  Out._12 = avRight.y;  Out._22 = avUp.y; Out._32 = avFwd.y;  Out._42 = avOrg.y;
  Out._13 = avRight.z;  Out._23 = avUp.z; Out._33 = avFwd.z;  Out._43 = avOrg.z;
  Out._14 = 0.0f;     Out._24 = 0.0f;   Out._34 = 0.0f;   Out._44 = 1.0f;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>* MatrixOrthogonalize(sMatrix<T>& Out, const sMatrix<T>& In, const T epsilon = (T)niEpsilon5)
{
  if (tPtr(&Out) != tPtr(&In)) {
    Out = In;
  }

  sVec3<T> X,Y,Z;
  MatrixGetRight(X,In);
  MatrixGetUp(Y,In);
  MatrixGetForward(Z,In);

  T xl = VecLength(X);
  if (xl <= epsilon)
    return NULL;

  xl *= xl;
  Y -= ((VecDot(X,Y))/xl)*X;
  Z -= ((VecDot(X,Z))/xl)*X;

  T yl = VecLength(Y);
  if (yl <= epsilon)
    return NULL;

  yl *= yl;
  Z -= (VecDot(Y,Z)/yl)*Y;

  MatrixSetRight(Out,X);
  MatrixSetUp(Out,Y);
  MatrixSetForward(Out,Z);
  return &Out;
};

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixNormalize(sMatrix<T>& Out, const sMatrix<T>& In)
{
  if (tPtr(&Out) != tPtr(&In)) {
    Out = In;
  }

  sVec3<T> X,Y,Z;
  MatrixGetRight(X,In);
  MatrixGetUp(Y,In);
  MatrixGetForward(Z,In);

  VecNormalize(X);
  VecNormalize(Y);
  VecNormalize(Z);

  MatrixSetRight(Out,X);
  MatrixSetUp(Out,Y);
  MatrixSetForward(Out,Z);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixSetScale(sMatrix<T>& Out, const sMatrix<T>& In, const sVec3f& avScale)
{
  if (tPtr(&Out) != tPtr(&In)) {
    Out = In;
  }

  sVec3<T> X,Y,Z;
  MatrixGetRight(X,In);
  MatrixGetUp(Y,In);
  MatrixGetForward(Z,In);

  VecSetLength(X,X,avScale.x);
  VecSetLength(Y,Y,avScale.y);
  VecSetLength(Z,Z,avScale.z);

  MatrixSetRight(Out,X);
  MatrixSetUp(Out,Y);
  MatrixSetForward(Out,Z);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixGetProjectedTranslation(sVec3<T>& Out,  const sMatrix<T>& M, T *pDeterminant = NULL)
{
  T det;
  T a1, b1, c1, d1;
  T a2, b2, c2, d2;
  T a3, b3, c3, d3;
  T a4, b4, c4, d4;

  if (!pDeterminant) pDeterminant = &det;

  *pDeterminant = MatrixDeterminant(M);
  if (*pDeterminant == 0) {
    Out.x = Out.y = Out.z = T(0);
    return Out;
  }

  det = 1.0f / (*pDeterminant);

  a1 = M._11; b1 = M._12; c1 = M._13; d1 = M._14;
  a2 = M._21; b2 = M._22; c2 = M._23; d2 = M._24;
  a3 = M._31; b3 = M._32; c3 = M._33; d3 = M._34;
  a4 = M._41; b4 = M._42; c4 = M._43; d4 = M._44;

  Out.x = -Det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4) * det;
  Out.y =  Det3x3(a1, a3, a4, b1, b3, b4, c1, c3, c4) * det;
  Out.z = -Det3x3(a1, a2, a4, b1, b2, b4, c1, c2, c4) * det;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixSetProjectedTranslation(sMatrix<T>& Out, const sVec3<T>& avT)
{
  sVec3<T> X,Y,Z,Trans;
  MatrixGetRight(X,Out);
  MatrixGetUp(Y,Out);
  MatrixGetForward(Z,Out);
  Trans.x = -VecDot(X,avT);
  Trans.y = -VecDot(Y,avT);
  Trans.z = -VecDot(Z,avT);
  MatrixSetTranslation(Out,Trans);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixSetNotRotation(sMatrix<T>& Out, const sMatrix<T>& M)
{
  Out._14 = M._14;
  Out._24 = M._24;
  Out._34 = M._34;
  Out._41 = M._41;
  Out._42 = M._42;
  Out._43 = M._43;
  Out._44 = M._44;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotate(sMatrix<T>& Out, const sMatrix<T>& M1, const sMatrix<T>& M2)
{
  T       a1, a2, a3;
  const sMatrix<T>* pA;
  const sMatrix<T>* pB;
  sMatrix<T> A, B;

  if (((sMatrix<T>*)(&M1)) == ((sMatrix<T>*)(&Out))) {
    A  = M1;
    pA = &A;
  }
  else {
    pA = &M1;
  }

  if (((sMatrix<T>*)(&M2)) == ((sMatrix<T>*)(&Out))) {
    B  = M2;
    pB = &B;
  }
  else {
    pB = &M2;
  }

  if (tPtr(&Out) != tPtr(&M1)) {
    MatrixSetNotRotation(Out,M1);
  }

  // 1st row * 1st column
  a1 = pA->_11; a2 = pA->_12; a3 = pA->_13;
  Out._11 = a1 * pB->_11 + a2 * pB->_21 + a3 * pB->_31;
  Out._12 = a1 * pB->_12 + a2 * pB->_22 + a3 * pB->_32;
  Out._13 = a1 * pB->_13 + a2 * pB->_23 + a3 * pB->_33;

  // 2nd row * 2nd column
  a1 = pA->_21; a2 = pA->_22; a3 = pA->_23;
  Out._21 = a1 * pB->_11 + a2 * pB->_21 + a3 * pB->_31;
  Out._22 = a1 * pB->_12 + a2 * pB->_22 + a3 * pB->_32;
  Out._23 = a1 * pB->_13 + a2 * pB->_23 + a3 * pB->_33;

  // 3rd row * 3rd column
  a1 = pA->_31; a2 = pA->_32; a3 = pA->_33;
  Out._31 = a1 * pB->_11 + a2 * pB->_21 + a3 * pB->_31;
  Out._32 = a1 * pB->_12 + a2 * pB->_22 + a3 * pB->_32;
  Out._33 = a1 * pB->_13 + a2 * pB->_23 + a3 * pB->_33;

  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T> MatrixRotationPivot(sMatrix<T>& aOut, const sMatrix<T>& aM2, const sVec3<T>& avPivot) {
  sMatrix<T> M2Translation = sMatrix<T>::Identity(), M2Rotation = sMatrix<T>::Identity();
  MatrixSetNotRotation(M2Translation,aM2);
  MatrixSetRotation(M2Rotation,aM2);
  sMatrixf t,ot;
  aOut = MatrixTranslation(ot,-avPivot) * M2Rotation * MatrixTranslation(t,avPivot) * M2Translation;
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T> MatrixRotationPivotAndScale(sMatrix<T>& aOut, const sMatrix<T>& aM2, const sVec3<T>& avPivot, const sVec3<T>& avScale) {
  sMatrix<T> M2Translation = sMatrix<T>::Identity(), M2Rotation = sMatrix<T>::Identity();
  MatrixSetNotRotation(M2Translation,aM2);
  MatrixSetRotation(M2Rotation,aM2);
  sMatrix<T> t,ot,s;
  aOut = MatrixTranslation(ot,-avPivot) * MatrixScaling(s,avScale) * M2Rotation * MatrixTranslation(t,avPivot) * M2Translation;
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixScalingH(sMatrix<T>& Out, T sx, T sy, T sz, T sh)
{
  MatrixIdentity(Out);
  Out._11 = sx*sh; Out._22 = sy*sh; Out._33 = sz*sh;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixScalingH(sMatrix<T>& Out, const sVec4<T>& aR)
{
  MatrixIdentity(Out);
  Out._11 = aR.x*aR.w; Out._22 = aR.y*aR.w; Out._33 = aR.z*aR.w;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationXYZ(sMatrix<T>& Out, T x, T y, T z) {
  sMatrix<T> tmp;
  MatrixRotationX(Out,x);
  Out *= MatrixRotationY(tmp,y);
  Out *= MatrixRotationZ(tmp,z);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationXYZ(sMatrix<T>& Out, const sVec3<T>& aR) {
  return MatrixRotationXYZ(Out,aR.x,aR.y,aR.z);
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationZYX(sMatrix<T>& Out, T x, T y, T z) {
  sMatrix<T> tmp;
  MatrixRotationZ(Out,z);
  Out *= MatrixRotationY(tmp,y);
  Out *= MatrixRotationX(tmp,x);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixRotationZYX(sMatrix<T>& Out, const sVec3<T>& aR) {
  return MatrixRotationZYX(Out,aR.x,aR.y,aR.z);
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixGetLocalScale(sVec3<T>& aS, const sMatrix<T>& In) {
  aS.x = ni::Sqrt(In(0,0) * In(0,0) + In(0,1) * In(0,1) + In(0,2) * In(0,2));
  aS.y = ni::Sqrt(In(1,0) * In(1,0) + In(1,1) * In(1,1) + In(1,2) * In(1,2));
  aS.z = ni::Sqrt(In(2,0) * In(2,0) + In(2,1) * In(2,1) + In(2,2) * In(2,2));
  return aS;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixGetLocalScaleSq(sVec3<T>& aS, const sMatrix<T>& In) {
  aS.x = (In(0,0) * In(0,0) + In(0,1) * In(0,1) + In(0,2) * In(0,2));
  aS.y = (In(1,0) * In(1,0) + In(1,1) * In(1,1) + In(1,2) * In(1,2));
  aS.z = (In(2,0) * In(2,0) + In(2,1) * In(2,1) + In(2,2) * In(2,2));
  return aS;
}

///////////////////////////////////////////////
//! \remark Recompose the matrix with the formula : scale * rotation [z*y*x order] * translation = original matrix
//!     if inverted is true (aS.w < 0.0), then negate scale and the above formula will be correct.
template <typename T>
void MatrixDecompose(const sMatrix<T>& In, sVec3<T>& aT, sVec3<T>& aR, sVec4<T>& aS, const T aEps = (T)niEpsilon4)
{
  aS.x = ni::Sqrt(In(0,0) * In(0,0) + In(0,1) * In(0,1) + In(0,2) * In(0,2));
  aS.y = ni::Sqrt(In(1,0) * In(1,0) + In(1,1) * In(1,1) + In(1,2) * In(1,2));
  aS.z = ni::Sqrt(In(2,0) * In(2,0) + In(2,1) * In(2,1) + In(2,2) * In(2,2));

  sVec3f savedScale = { aS.x,aS.y,aS.z };
  if (FuzzyEqual(aS.x, 0.0f, aEps)) { aS.x = aEps; }
  if (FuzzyEqual(aS.y, 0.0f, aEps)) { aS.y = aEps; }
  if (FuzzyEqual(aS.z, 0.0f, aEps)) { aS.z = aEps; }

  aS.w = ni::Sign(ni::MatrixDeterminant3(In));
  if (aS.w < 0.0f) {
    // intentionally do not want to do this on the savedScale
    aS.x = -aS.x;
    aS.y = -aS.y;
    aS.z = -aS.z;
  }

  // Calculate the rotation in Y first, using m(2,0), checking for out-of-bounds values
  T c;
  if (In(2,0)/aS.z >= 1.0f-aEps) {
    aR.y = (T)ni::kPi / 2.0f;
    c = 0.0f;
  }
  else if (In(2,0)/aS.z <= -1.0f+aEps) {
    aR.y = (T)ni::kPi / -2.0f;
    c = 0.0f;
  }
  else {
    aR.y = ni::ASin(In(2,0) / aS.z);
    c = ni::Cos(aR.y);
  }

  // Using the cosine of the Y rotation will give us the rotation in X and Z.
  // Check for the infamous Gimbal Lock.
  if (ni::Abs(c) > 0.01f)  {
    T rx = In(2,2) / aS.z / c;
    T ry = -In(2,1) / aS.z / c;
    aR.x = ni::ATan2(ry, rx);
    rx = In(0,0) / aS.x / c;
    ry = -In(1,0) / aS.y / c;
    aR.z = ni::ATan2(ry, rx);
  }
  else {
    aR.z = 0;
    T rx = In(1,1) / aS.y;
    T ry = In(1,2) / aS.y;
    aR.x = ni::ATan2(ry, rx);
  }

  sVec3<T> rt = aR;
  aR.x = rt.x;
  aR.y = rt.y;
  aR.z = rt.z;

  MatrixGetTranslation(aT,In);
  aS.x = savedScale.x;
  aS.y = savedScale.y;
  aS.z = savedScale.z;
}

///////////////////////////////////////////////
template <typename T>
void MatrixDecomposeQ(const sMatrix<T>& In, sVec3<T>& aT, sQuat<T>& aR, sVec4<T>& aS, const T aEps = (T)niEpsilon5)
{
  sVec3<T> vR;
  MatrixDecompose(In,aT,vR,aS,aEps);
  QuatRotationZYX(aR,vR);
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixCompose(sMatrix<T>& Out, const sVec3<T>& aT, const sVec3<T>& aR, const sVec4<T>& aS) {
  sMatrix<T> tmp;
  Out = sMatrix<T>::Identity();
  Out *= MatrixScalingH(tmp,aS);
  Out *= MatrixRotationZYX(tmp,aR);
  Out *= MatrixTranslation(tmp,aT);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixComposeQ(sMatrix<T>& Out, const sVec3<T>& aT, const sQuat<T>& aR, const sVec4<T>& aS) {
  sMatrix<T> tmp;
  Out = sMatrix<T>::Identity();
  Out *= MatrixScalingH(tmp,aS);
  Out *= MatrixRotationQuat(tmp,aR);
  Out *= MatrixTranslation(tmp,aT);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& MatrixViewport(sMatrix<T>& Out, const sVec4<T>& aVP, const T aMinZ, const T aMaxZ) {
  Out = sMatrix<T>::Identity();
  const T w2 = (aVP.Right()-aVP.Left())/T(2);
  const T h2 = (aVP.Bottom()-aVP.Top())/T(2);
  Out._11 = w2;
  Out._22 = -h2;
  Out._33 = aMaxZ-aMinZ;
  Out._41 = aVP.Left() + w2;
  Out._42 = aVP.Top() + h2;
  Out._43 = aMinZ;
  return Out;
}

///////////////////////////////////////////////
template <typename T>
const sMatrix<T>& MatrixAdjustViewport(sMatrix<T>& Out, const sVec4<T>& aContextVP, const sVec4<T>& aVirtualVP, const T afMinZ, const T afMaxZ) {
  if (aContextVP == aVirtualVP) {
    return MatrixIdentity(Out);
  }
  else {
    sMatrix<T> mtxCtxVP, mtxInvCtxVP, mtxVirtVP;
    MatrixViewport(mtxCtxVP,aContextVP,afMinZ,afMaxZ);
    MatrixViewport(mtxVirtVP,aVirtualVP,afMinZ,afMaxZ);
    MatrixInverse(mtxInvCtxVP,mtxCtxVP);
    return MatrixMultiply(Out,mtxVirtVP,mtxInvCtxVP);
  }
}

///////////////////////////////////////////////
template <typename T>
T MatrixGetFovV(const sMatrix<T>& aMtx) {
  const T h = aMtx._22; // get the view space height
  return T(2) * ni::ATan(T(1) / h);
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixDecomposeGetZYX(sVec3<T>& aR, const sMatrix<T>& In, const T aEps = (T)niEpsilon4)
{
  sVec3<T> t;
  sVec4<T> s;
  MatrixDecompose(In,t,aR,s,aEps);
  return aR;
}

///////////////////////////////////////////////
template <typename T>
sQuat<T>& MatrixDecomposeGetQuat(sQuat<T>& aR, const sMatrix<T>& In, const T aEps = (T)niEpsilon4)
{
  sVec3<T> t;
  sVec4<T> s;
  MatrixDecomposeQ(In,t,aR,s,aEps);
  return aR;
}

///////////////////////////////////////////////
template <typename T>
sVec4<T>& MatrixDecomposeGetScale(sVec4<T>& aS, const sMatrix<T>& In, const T aEps = (T)niEpsilon4)
{
  sVec3<T> Tr,R;
  MatrixDecompose(In,Tr,R,aS,aEps);
  return aS;
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& MatrixDecomposeGetTranslation(sVec3<T>& aT, const sMatrix<T>& In)
{
  return MatrixGetTranslation(aT,In);
}

///////////////////////////////////////////////
// Flips the matrix so that it renders in up-side-down RTs (OpenGL)
template <typename T>
sMatrix<T> MatrixFlipProjectionY(const sMatrix<T>& aMatrix) {
  sMatrix<T> m = aMatrix;
  m._12 = -m._12;
  m._22 = -m._22;
  m._32 = -m._32;
  m._42 = -m._42;
  return m;
}

///////////////////////////////////////////////
//! Compute a projection matrix to do pixel perfect 2d rendering.
//! \param afOrthoProjectionOffset is the projection offset to do pixel perfect mapping, usually retrieved from ultof(iGraphicsDriver::GetCaps(eGraphicsCaps_OrthoProjectionOffset)).
//! \param aRect the 2d canvas size.
//! \param afVerticalFov, if > 0 creates a perspective matrix with the specified fov instead of a ortho projection.
//! \param aCameraPos, x & y offset of the camera afVerticalFov is specified.
//! \remark View matrix should be identity with this projection matrix.
template <typename T>
sMatrix<T> MatrixProjection2D(
    const T afOrthoProjectionOffset,
    const sRect<T>& aRect,
    const T afVerticalFov,
    const sVec2<T>& aCameraPos = sVec2<T>::Zero())
{
  sRect<T> rect(0,0,aRect.GetWidth(),aRect.GetHeight());
  sVec2<T> origin = Vec2((T)aRect.Left(),(T)aRect.Top());
  rect -= origin;

  sMatrix<T> proj;
  if (afVerticalFov > 0.0f) {
    const T fov = afVerticalFov; // vertical fov (in radians)
    const T theta = fov/2;
    const T s = 20; // this is arbitrary, just needs to be enought so that there's some room for the near space
    const T opp = aRect.GetHeight()/2.0f/s;
    const T n = opp / tan(theta);
    const T f = 1e6f;
    // huh, ok here doing -fOrthoProjectionOffset produce pixel
    // sharp result... seriously no clue why that is... is it half
    // a unit ? or... seems unlikely because 's' is more-less
    // random... and its not clear how they would be a
    // correlation... Anyhow changing 's' doesn't affect the
    // outcome... which is even wierder... I'm guessing
    // -fOrthoProjectionOffset kind of snaps the pixel back in
    // place as it would with a float to int rounding...
    const T d = s*n-afOrthoProjectionOffset;
    const T o = -afOrthoProjectionOffset;

    const sVec3<T> cameraPos = {
      aRect.GetWidth()/2.0f+aCameraPos.x,
      aRect.GetHeight()/2.0f+aCameraPos.y,
      0
    };
    rect.Move(-(sVec2<T>&)cameraPos);

    sMatrix<T> t,p;
    // perspective
    MatrixLookAtLH(
        t,
        Vec3<T>(o,o,-d)+cameraPos,
        Vec3<T>(o,o,0)+cameraPos,
        sVec3<T>::YAxis());
    MatrixPerspectiveOffCenterLH(
        p,
        rect.Left()/s,
        rect.Right()/s,
        rect.Bottom()/s,
        rect.Top()/s,
        n,
        f);
    proj = t * p;
  }
  else {
    const T o = -afOrthoProjectionOffset;
    // ortho
    MatrixOrthoOffCenterLH(
        proj,
        rect.Left()+o,
        rect.Right()+o,
        rect.Bottom()+o,
        rect.Top()+o,
        -1.0f,
        1.0f);
  }
  return proj;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHMATRIX_8300436_H__
