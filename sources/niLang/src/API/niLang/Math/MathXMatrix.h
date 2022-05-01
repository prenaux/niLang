#ifndef __MATHMATRIX_5021497_H__
#define __MATHMATRIX_5021497_H__
#ifndef niCore_NoWarnDisable
#if defined __GNUC__
#pragma GCC system_header
#elif defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#endif
#endif
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
template <tU32 N, tU32 M, class T = tF32>
struct XMatrix
{
 public:
  ///////////////////////////////////////////////
  inline XMatrix() {}

  ///////////////////////////////////////////////
  inline XMatrix(const T& v) {
    Fill(v);
  }

  ///////////////////////////////////////////////
  inline XMatrix(const T& a00, const T& a01, const T& a02, const T& a03,
                 const T& a10, const T& a11, const T& a12, const T& a13,
                 const T& a20, const T& a21, const T& a22, const T& a23,
                 const T& a30, const T& a31, const T& a32, const T& a33) {
    niTAssert(N == 4 && M == 4);
    (*this)(0,0) = a00; (*this)(0,1) = a01; (*this)(0,2) = a02; (*this)(0,3) = a03;
    (*this)(1,0) = a10; (*this)(1,1) = a11; (*this)(1,2) = a12; (*this)(1,3) = a13;
    (*this)(2,0) = a20; (*this)(2,1) = a21; (*this)(2,2) = a22; (*this)(2,3) = a23;
    (*this)(3,0) = a30; (*this)(3,1) = a31; (*this)(3,2) = a32; (*this)(3,3) = a33;
  }

  ///////////////////////////////////////////////
  inline XMatrix(const T& a00, const T& a01, const T& a02,
                 const T& a10, const T& a11, const T& a12,
                 const T& a20, const T& a21, const T& a22) {
    niTAssert(N == 3 && M == 3);
    (*this)(0,0) = a00; (*this)(0,1) = a01; (*this)(0,2) = a02;
    (*this)(1,0) = a10; (*this)(1,1) = a11; (*this)(1,2) = a12;
    (*this)(2,0) = a20; (*this)(2,1) = a21; (*this)(2,2) = a22;
  }

  ///////////////////////////////////////////////
  inline XMatrix(const XMatrix &m)
  {
    ni::CopyArray(mData, m.mData, N*M);
  }

  ///////////////////////////////////////////////
  inline XMatrix(const XMatrix<N,1,T>& xv, const XMatrix<N,1,T>& yv, const XMatrix<N,1,T>& zv)
  {
    SetVecX(xv);
    SetVecY(yv);
    SetVecZ(zv);
  }

  ///////////////////////////////////////////////
  inline XMatrix(const XMatrix<N,1,T>& xv, const XMatrix<N,1,T>& yv, const XMatrix<N,1,T>& zv, const XMatrix<N,1,T>& wv)
  {
    SetVecX(xv);
    SetVecY(yv);
    SetVecZ(zv);
    SetVecW(wv);
  }

  ///////////////////////////////////////////////
  inline XMatrix(const T& xv, const T& yv, const T& zv, const T& wv)
  {
    niTAssert(N == 4 && M == 1);
    X() = xv;
    Y() = yv;
    Z() = zv;
    W() = wv;
  }

  ///////////////////////////////////////////////
  inline XMatrix(const sVec4<T>& v) {
    niTAssert(N == 4 && M == 1);
    X() = v.x;
    Y() = v.y;
    Z() = v.z;
    W() = v.w;
  }

  ///////////////////////////////////////////////
  inline XMatrix(const T& xv, const T& yv, const T& zv)
  {
    niTAssert(N == 3 && M == 1);
    X() = xv;
    Y() = yv;
    Z() = zv;
  }

  ///////////////////////////////////////////////
  inline XMatrix(const sVec3<T>& v) {
    niTAssert(N == 3 && M == 1);
    X() = v.x;
    Y() = v.y;
    Z() = v.z;
  }

  ///////////////////////////////////////////////
  inline XMatrix(const T& xv, const T& yv)
  {
    niTAssert(N == 2 && M == 1);
    X() = xv;
    Y() = yv;
  }

  ///////////////////////////////////////////////
  inline XMatrix(const sVec2<T>& v) {
    niTAssert(N == 2 && M == 1);
    X() = v.x;
    Y() = v.y;
  }

  ///////////////////////////////////////////////
  inline operator const sVec2<T>& () const {
    niTAssert(N == 2 && M == 1);
    return *(const sVec2<T>*)this;
  }
  inline operator     sVec2<T>& ()  {
    niTAssert(N == 2 && M == 1);
    return *(sVec2<T>*)this;
  }

  ///////////////////////////////////////////////
  inline operator const sVec3<T>& () const {
    niTAssert(N == 3 && M == 1);
    return *(const sVec3<T>*)this;
  }
  inline operator     sVec3<T>& ()  {
    niTAssert(N == 3 && M == 1);
    return *(sVec3<T>*)this;
  }

  ///////////////////////////////////////////////
  inline operator const sVec4<T>& () const {
    niTAssert(N == 4 && M == 1);
    return *(const sVec4<T>*)this;
  }
  inline operator     sVec4<T>& () {
    niTAssert(N == 4 && M == 1);
    return *(sVec4<T>*)this;
  }

  ///////////////////////////////////////////////
  inline operator const sMatrix<T>& () const {
    niTAssert(N == 4 && M == 4);
    return *(const sMatrix<T>*)this;
  }
  inline operator     sMatrix<T>& () {
    niTAssert(N == 4 && M == 4);
    return *(sMatrix<T>*)this;
  }

  ///////////////////////////////////////////////
  inline operator XMatrix<2,1,T>()
  {
    niTAssert(N == 2 && M == 1);

    XMatrix<2, 1, T>  result;
    result.Fill(0);
    tU32 c = N;
    if (c > 2) c = 2;
    for (tU32 i = 0; i < c; i++)
      result(i,0) = (*this)(i,0);

    return result;
  }

  ///////////////////////////////////////////////
  inline operator XMatrix<3,1,T>()
  {
    niTAssert(N == 3 && M == 1);

    XMatrix<3,1,T>  result;
    result.Fill(0);

    tU32 c = N;
    if (c > 3) c = 3;

    for (tU32 i = 0; i < c; i++)
      result(i,0) = (*this)(i,0);

    return result;
  }

  ///////////////////////////////////////////////
  inline operator XMatrix<4,1,T>()
  {
    niTAssert(N == 4 && M == 1);

    XMatrix<4, 1, T>  result;
    result.Fill(0);
    result.W() = (T) 1;

    tU32 c = N;
    if (c > 4) c = 4;

    for (tU32 i = 0; i < c; i++)
      result(i,0) = (*this)(i,0);

    return result;
  }

  ///////////////////////////////////////////////
  static XMatrix Zero()
  {
    XMatrix result;
    result.Fill((T) 0);
    return result;
  }

  ///////////////////////////////////////////////
  inline XMatrix& operator = (const XMatrix &m)
  {
    if (&m != this)
      CopyArray(mData, m.mData, N*M);
    return *this;
  }

  ///////////////////////////////////////////////
  inline  const T& operator()(const tU32 i, const tU32 j) const
  {
    return mData[j*N+i];
  }

  ///////////////////////////////////////////////
  inline T& operator()(const tU32 i, const tU32 j)
  {
    return mData[j*N+i];
  }

  ///////////////////////////////////////////////
#define ImplementSpecializedConcat(X)                                   \
  inline  const XMatrix<N,X,T> Concat(const XMatrix<M,X,T>& m) const    \
  {                                                                     \
    XMatrix<N,X,T> result;                                              \
    result.Fill((T)0);                                                  \
    for (tU32 i = 0; i < N; ++i) {                                      \
      for (tU32 j = 0; j < M; ++j){                                     \
        for (tU32 k = 0; k < X; ++k) {                                  \
          result(i,j) += (*this)(i,k) * m(k,j);                         \
        }                                                               \
      }                                                                 \
    }                                                                   \
    return result;                                                      \
  }                                                                     \
  inline const XMatrix<N,X,T> operator | (const XMatrix<M,X,T>& m) const { \
    return Concat(m);                                                   \
  }
  ImplementSpecializedConcat(1)
  ImplementSpecializedConcat(2)
  ImplementSpecializedConcat(3)
  ImplementSpecializedConcat(4)
  ImplementSpecializedConcat(5)
  ImplementSpecializedConcat(6)
  ImplementSpecializedConcat(7)
  ImplementSpecializedConcat(8)
  ImplementSpecializedConcat(9)
#undef ImplementSpecializedConcat

  ///////////////////////////////////////////////
  inline XMatrix<N,1,T> Cross(const XMatrix<N,1,T>& m)
  {
    niTAssert(N >= 3);
    XMatrix<N,1,T> d;
    d.X() = Y() * m.Z() - Z() * m.Y();
    d.Y() = Z() * m.X() - X() * m.Z();
    d.Z() = X() * m.Y() - Y() * m.X();
    return d;
  }

  ///////////////////////////////////////////////
  inline  const T Dot(const XMatrix& m) const
  {
    T result = (T) 0;
    for (tU32 i = 0; i < N; i++)
      result += mData[i] * m.El(i);
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator * (const XMatrix& m) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = mData[i] * m.mData[i];
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator * (const T &value) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = mData[i] * value;
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator *= (const XMatrix &m)
  {
    for (tU32 i = 0; i < N*M; i++) mData[i] *= m.mData[i];
    return *this;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator *= (const T &value)
  {
    for (tU32 i = 0; i < N*M; i++) mData[i] *= value;
    return *this;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator / (const XMatrix &m) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = mData[i] / m.mData[i];
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator / (const T &value) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = mData[i] / value;
    return result;
  }

  ///////////////////////////////////////////////
  inline  const XMatrix InverseDivide(const T &value) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = value / mData[i];
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator /= (const XMatrix &m)
  {
    for (tU32 i = 0; i < N*M; i++) mData[i] /= m.mData[i];
    return *this;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator /= (const T &value)
  {
    for (tU32 i = 0; i < N*M; i++) mData[i] /= value;
    return *this;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator + (const XMatrix &m) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = mData[i] + m.mData[i];
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator + (const T &value) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = mData[i] + value;
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator += (const XMatrix &m)
  {
    for (tU32 i = 0; i < N*M; i++) mData[i] += m.mData[i];
    return *this;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator += (const T &value)
  {
    for (tU32 i = 0; i < N*M; i++) mData[i] += value;
    return *this;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator - () const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = -mData[i];
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator - (const XMatrix &m) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = mData[i] - m.mData[i];
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator - (const T &value) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = mData[i] - value;
    return result;
  }

  ///////////////////////////////////////////////
  inline  const XMatrix InverseSubtract(const T &value) const
  {
    XMatrix result;
    for (tU32 i = 0; i < N*M; i++) result.mData[i] = value - mData[i];
    return result;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator -= (const XMatrix &m)
  {
    for (tU32 i = 0; i < N*M; i++) mData[i] -= m.mData[i];
    return *this;
  }

  ///////////////////////////////////////////////
  inline const XMatrix operator -= (const T &value)
  {
    for (tU32 i = 0; i < N*M; i++) mData[i] -= value;
    return *this;
  }

  ///////////////////////////////////////////////
  inline T Total() const
  {
    T tot = (T) 0;
    for (tU32 i = 0; i < N*M; i++) tot += mData[i];
    return tot;
  }

  ///////////////////////////////////////////////
  inline const bool operator == (const XMatrix &m) const
  {
    for (tU32 i = 0; i < N*M; i++)
      if (mData[i] != m.mData[i]) return false;
    return true;
  }

  ///////////////////////////////////////////////
  inline const bool operator != (const XMatrix &m) const
  {
    return !(*this == m);
  }

  ///////////////////////////////////////////////
  inline const bool operator < (const XMatrix &m) const
  {
    for (tU32 i = 0; i < N*M; i++)
      if (mData[i] >= m.mData[i])
        return false;
    return true;
  }

  ///////////////////////////////////////////////
  inline const bool operator <= (const XMatrix &m) const
  {
    for (tU32 i = 0; i < N*M; i++)
      if (mData[i] > m.mData[i])
        return false;
    return true;
  }

  ///////////////////////////////////////////////
  inline const bool operator > (const XMatrix &m) const
  {
    for (tU32 i = 0; i < N*M; i++)
      if (mData[i] <= m.mData[i])
        return false;
    return true;
  }

  ///////////////////////////////////////////////
  inline const bool operator >= (const XMatrix &m) const
  {
    for (tU32 i = 0; i < N*M; i++)
      if (mData[i] < m.mData[i])
        return false;
    return true;
  }

  ///////////////////////////////////////////////
  // Generates a converted matrix. Result is a matrix that has been converted from one type to another.
  //
  // Offers two options:
  //
  // rowColumnSwap: if true, toggle between row-major and column-major
  // leftRightSwap: if true, toggle between left-handed and right-handed coordinate systems
  //
  // Default behaviour is a row-/column-major swap, but not a left-/right-handed swap.
  inline XMatrix GenConvertedType(const tBool rowColumnSwap = eTrue, const tBool leftRightSwap = eFalse) const
  {
    niTAssert(N >= 3 && M >= 3);

    XMatrix result = *this;
    if (leftRightSwap) {
      for (unsigned int i = 2; i < N; i++) {
        result(i,2) = -result(i,2);
      }
    }

    if (rowColumnSwap) {
      result.Transpose();
    }
    return result;
  }

  ///////////////////////////////////////////////
  inline void Transpose()
  {
    niTAssert(N == M);
    XMatrix result;
    for (tU32 j = 0; j < M; j++)
    {
      for (tU32 i = 0; i < N; i++)
      {
        result(j,i) = (*this)(i,j);
      }
    }
    *this = result;
  }

  ///////////////////////////////////////////////
  inline T Determinant()
  {
    niTAssert(N == 4 && M == 4);
    XMatrix &m = *this;
    return    (m(0,0) * m(1,1) - m(1,0) * m(0,1)) * (m(2,2) * m(3,3) - m(3,2) * m(2,3))
        - (m(0,0) * m(2,1) - m(2,0) * m(0,1)) * (m(1,2) * m(3,3) - m(3,2) * m(1,3))
        + (m(0,0) * m(3,1) - m(3,0) * m(0,1)) * (m(1,2) * m(2,3) - m(2,2) * m(1,3))
        + (m(1,0) * m(2,1) - m(2,0) * m(1,1)) * (m(0,2) * m(3,3) - m(3,2) * m(0,3))
        - (m(1,0) * m(3,1) - m(3,0) * m(1,1)) * (m(0,2) * m(2,3) - m(2,2) * m(0,3))
        + (m(2,0) * m(3,1) - m(3,0) * m(2,1)) * (m(0,2) * m(1,3) - m(1,2) * m(0,3));
  }

  ///////////////////////////////////////////////
  inline void Invert()
  {
    niTAssert(N == 4 && M == 4);

    T d = Determinant();
    if (d == 0.0) return;

    d = T(1.0) / d;
    XMatrix &m = *this;
    XMatrix result;
    result(0,0) = d * (m(1,1) * (m(2,2) * m(3,3) - m(3,2) * m(2,3)) + m(2,1) * (m(3,2) * m(1,3) - m(1,2) * m(3,3)) + m(3,1) * (m(1,2) * m(2,3) - m(2,2) * m(1,3)));
    result(1,0) = d * (m(1,2) * (m(2,0) * m(3,3) - m(3,0) * m(2,3)) + m(2,2) * (m(3,0) * m(1,3) - m(1,0) * m(3,3)) + m(3,2) * (m(1,0) * m(2,3) - m(2,0) * m(1,3)));
    result(2,0) = d * (m(1,3) * (m(2,0) * m(3,1) - m(3,0) * m(2,1)) + m(2,3) * (m(3,0) * m(1,1) - m(1,0) * m(3,1)) + m(3,3) * (m(1,0) * m(2,1) - m(2,0) * m(1,1)));
    result(3,0) = d * (m(1,0) * (m(3,1) * m(2,2) - m(2,1) * m(3,2)) + m(2,0) * (m(1,1) * m(3,2) - m(3,1) * m(1,2)) + m(3,0) * (m(2,1) * m(1,2) - m(1,1) * m(2,2)));
    result(0,1) = d * (m(2,1) * (m(0,2) * m(3,3) - m(3,2) * m(0,3)) + m(3,1) * (m(2,2) * m(0,3) - m(0,2) * m(2,3)) + m(0,1) * (m(3,2) * m(2,3) - m(2,2) * m(3,3)));
    result(1,1) = d * (m(2,2) * (m(0,0) * m(3,3) - m(3,0) * m(0,3)) + m(3,2) * (m(2,0) * m(0,3) - m(0,0) * m(2,3)) + m(0,2) * (m(3,0) * m(2,3) - m(2,0) * m(3,3)));
    result(2,1) = d * (m(2,3) * (m(0,0) * m(3,1) - m(3,0) * m(0,1)) + m(3,3) * (m(2,0) * m(0,1) - m(0,0) * m(2,1)) + m(0,3) * (m(3,0) * m(2,1) - m(2,0) * m(3,1)));
    result(3,1) = d * (m(2,0) * (m(3,1) * m(0,2) - m(0,1) * m(3,2)) + m(3,0) * (m(0,1) * m(2,2) - m(2,1) * m(0,2)) + m(0,0) * (m(2,1) * m(3,2) - m(3,1) * m(2,2)));
    result(0,2) = d * (m(3,1) * (m(0,2) * m(1,3) - m(1,2) * m(0,3)) + m(0,1) * (m(1,2) * m(3,3) - m(3,2) * m(1,3)) + m(1,1) * (m(3,2) * m(0,3) - m(0,2) * m(3,3)));
    result(1,2) = d * (m(3,2) * (m(0,0) * m(1,3) - m(1,0) * m(0,3)) + m(0,2) * (m(1,0) * m(3,3) - m(3,0) * m(1,3)) + m(1,2) * (m(3,0) * m(0,3) - m(0,0) * m(3,3)));
    result(2,2) = d * (m(3,3) * (m(0,0) * m(1,1) - m(1,0) * m(0,1)) + m(0,3) * (m(1,0) * m(3,1) - m(3,0) * m(1,1)) + m(1,3) * (m(3,0) * m(0,1) - m(0,0) * m(3,1)));
    result(3,2) = d * (m(3,0) * (m(1,1) * m(0,2) - m(0,1) * m(1,2)) + m(0,0) * (m(3,1) * m(1,2) - m(1,1) * m(3,2)) + m(1,0) * (m(0,1) * m(3,2) - m(3,1) * m(0,2)));
    result(0,3) = d * (m(0,1) * (m(2,2) * m(1,3) - m(1,2) * m(2,3)) + m(1,1) * (m(0,2) * m(2,3) - m(2,2) * m(0,3)) + m(2,1) * (m(1,2) * m(0,3) - m(0,2) * m(1,3)));
    result(1,3) = d * (m(0,2) * (m(2,0) * m(1,3) - m(1,0) * m(2,3)) + m(1,2) * (m(0,0) * m(2,3) - m(2,0) * m(0,3)) + m(2,2) * (m(1,0) * m(0,3) - m(0,0) * m(1,3)));
    result(2,3) = d * (m(0,3) * (m(2,0) * m(1,1) - m(1,0) * m(2,1)) + m(1,3) * (m(0,0) * m(2,1) - m(2,0) * m(0,1)) + m(2,3) * (m(1,0) * m(0,1) - m(0,0) * m(1,1)));
    result(3,3) = d * (m(0,0) * (m(1,1) * m(2,2) - m(2,1) * m(1,2)) + m(1,0) * (m(2,1) * m(0,2) - m(0,1) * m(2,2)) + m(2,0) * (m(0,1) * m(1,2) - m(1,1) * m(0,2)));
    *this = result;
  }

  ///////////////////////////////////////////////
  inline void Fill(const T& value)
  {
    T *ptr = mData;
    for (tU32 i = 0; i < N*M; i++, ptr++) *ptr = value;
  }

  ///////////////////////////////////////////////
  static XMatrix Identity()
  {
    XMatrix result;
    T*      ptr = result.mData;
    for (tU32 j = 0; j < M; j++)
    {
      for (tU32 i = 0; i < N; i++, ptr++)
      {
        if (i == j) *ptr = (T) 1;
        else    *ptr = (T) 0;
      }
    }
    return result;
  }

  ///////////////////////////////////////////////
  static XMatrix Rotation2D(const T& theta)
  {
    niTAssert(N >= 2 && M >= 2);
    XMatrix result = Identity();
    T ct = (T)cos((double)theta);
    T st = (T)sin((double)theta);
    result(0,0) =   ct;
    result(1,0) =   st;
    result(0,1) =  -st;
    result(1,1) =   ct;
    return result;
  }

  ///////////////////////////////////////////////
  static XMatrix RotationX(const T& theta)
  {
    niTAssert(N >= 3 && M >= 3);
    XMatrix result = Identity();
    T ct = (T)cos((double)theta);
    T st = (T)sin((double)theta);
    result(1,1) =  ct;
    result(2,1) =  st;
    result(1,2) = -st;
    result(2,2) =  ct;
    return result;
  }

  ///////////////////////////////////////////////
  static  XMatrix RotationY(const T& theta)
  {
    niTAssert(N >= 3 && M >= 3);
    XMatrix result = Identity();
    T ct = (T) cos((double) theta);
    T st = (T) sin((double) theta);
    result(0,0) =  ct;
    result(2,0) =  st;
    result(0,2) = -st;
    result(2,2) =  ct;
    return result;
  }

  ///////////////////////////////////////////////
  static  XMatrix RotationZ(const T& theta)
  {
    niTAssert(N >= 3 && M >= 3);
    XMatrix result = Identity();
    T ct = (T) cos((double) theta);
    T st = (T) sin((double) theta);
    result(0,0) =  ct;
    result(1,0) = -st;
    result(0,1) =  st;
    result(1,1) =  ct;
    return result;
  }

  ///////////////////////////////////////////////
  static XMatrix Scale(const XMatrix<N,1,T>& m)
  {
    niTAssert(N <= M);

    XMatrix result;
    result = Identity();
    for (tU32 i = 0; i < N; i++)
    {
      result(i,i) *= m(i,0);
    }
    return result;
  }

  ///////////////////////////////////////////////
  static XMatrix Translation(const XMatrix<N,1,T>& m)
  {
    niTAssert(M <= N);

    XMatrix result;
    result = Identity();
    for (tU32 i = 0; i < M; i++)
    {
      result(N-1,i) += m(i,0);
    }
    return result;
  }

  ///////////////////////////////////////////////
  static XMatrix Shear(const T x, const T y)
  {
    niTAssert(N > 1 && M > 1);
    XMatrix result;
    result = Identity();
    result(1,0) = x;
    result(0,1) = y;
    return result;
  }

  ///////////////////////////////////////////////
  inline XMatrix<N,1,T> GetVecX() const
  {
    niTAssert(M > 0);
    XMatrix<N,1,T>  result;
    for (tU32 i = 0; i < N; i++) {
      result(i,0) = (*this)(i,0);
    }
    return result;
  }

  ///////////////////////////////////////////////
  inline XMatrix<N,1,T> GetVecY() const
  {
    niTAssert(M > 1);
    XMatrix<N,1,T>  result;
    for (tU32 i = 0; i < N; i++) {
      result(i,0) = (*this)(i,1);
    }
    return result;
  }

  ///////////////////////////////////////////////
  inline XMatrix<N, 1, T> GetVecZ() const
  {
    niTAssert(M > 2);
    XMatrix<N,1,T> result;
    for (tU32 i = 0; i < N; i++) {
      result(i,0) = (*this)(i,2);
    }
    return result;
  }

  ///////////////////////////////////////////////
  inline XMatrix<N,1,T> GetVecW() const
  {
    niTAssert(M > 3);
    XMatrix<N, 1, T>  result;
    for (tU32 i = 0; i < N; i++) {
      result(i,0) = (*this)(i,3);
    }
    return result;
  }

  ///////////////////////////////////////////////
  inline void SetVecX(const XMatrix<N,1,T> &m)
  {
    for (tU32 i = 0; i < N; i++) {
      (*this)(i,0) = m(i,0);
    }
  }

  ///////////////////////////////////////////////
  inline void SetVecY(const XMatrix<N,1,T>& m)
  {
    for (tU32 i = 0; i < N; i++) {
      (*this)(i,1) = m(i,0);
    }
  }

  ///////////////////////////////////////////////
  inline void SetVecZ(const XMatrix<N, 1, T> &m)
  {
    for (tU32 i = 0; i < N; i++) {
      (*this)(i,2) = m(i,0);
    }
  }

  ///////////////////////////////////////////////
  inline void SetVecW(const XMatrix<N, 1, T> &m)
  {
    for (tU32 i = 0; i < N; i++) {
      (*this)(i,3) = m(i,0);
    }
  }

  ///////////////////////////////////////////////
  inline  const T LengthSquared() const
  {
    niTAssert(M == 1);
    return Dot(*this);
  }

  ///////////////////////////////////////////////
  inline  const T Length() const
  {
    niTAssert(M == 1);
    return (T)ni::Sqrt(LengthSquared());
  }

  ///////////////////////////////////////////////
  inline void SetLength(const T &len)
  {
    niTAssert(M == 1);
    T l = len / Length();
    for (tU32 i = 0; i < N; ++i) {
      El(i) *= l;
    }
  }

  ///////////////////////////////////////////////
  inline T Distance(const XMatrix &m) const
  {
    niTAssert(M == 1);
    XMatrix temp = *this - m;
    return temp.Length();
  }

  ///////////////////////////////////////////////
  inline void Normalize()
  {
    SetLength((T) 1);
  }

  ///////////////////////////////////////////////
  inline  void OrthoNormalize()
  {
    niTAssert(N == 3 && M == 3);

    XMatrix<N,1,T>  xVec = GetVecX();
    XMatrix<N,1,T>  yVec = GetVecY();
    XMatrix<N,1,T>  zVec = GetVecZ();

    xVec -= yVec * (xVec * yVec);
    xVec -= zVec * (xVec * zVec);
    xVec.Normalize();

    yVec -= xVec * (yVec * xVec);
    yVec -= zVec * (yVec * zVec);
    yVec.Normalize();

    zVec = xVec.Cross(yVec);

    SetVecX(xVec);
    SetVecY(yVec);
    SetVecZ(zVec);
  }

  ///////////////////////////////////////////////
  inline XMatrix<N,M,T> Abs() const
  {
    XMatrix<N,M,T> r;
    for (tU32 i = 0; i < N*M; ++i)
      r.mData[i] = ni::Abs(mData[i]);
    return r;
  }

  ///////////////////////////////////////////////
  inline const T& X() const { niTAssert(N > 0 && M == 1); return mData[0]; }
  inline       T& X()       { niTAssert(N > 0 && M == 1); return mData[0]; }
  ///////////////////////////////////////////////
  inline const T& Y() const { niTAssert(N > 1 && M == 1); return mData[1]; }
  inline     T& Y()       { niTAssert(N > 1 && M == 1); return mData[1]; }
  ///////////////////////////////////////////////
  inline const T& Z() const { niTAssert(N > 2 && M == 1); return mData[2]; }
  inline     T& Z()       { niTAssert(N > 2 && M == 1); return mData[2]; }
  ///////////////////////////////////////////////
  inline const T& W() const { niTAssert(N > 3 && M == 1); return mData[3]; }
  inline     T& W()       { niTAssert(N > 3 && M == 1); return mData[3]; }

  ///////////////////////////////////////////////
  inline const T& R() const { return X(); }
  inline     T& R()       { return X(); }
  ///////////////////////////////////////////////
  inline const T& G() const { return Y(); }
  inline     T& G()       { return Y(); }
  ///////////////////////////////////////////////
  inline const T& B() const { return Z(); }
  inline     T& B()       { return Z(); }
  ///////////////////////////////////////////////
  inline const T& A() const { return W(); }
  inline     T& A()       { return W(); }

  ///////////////////////////////////////////////
  inline const T& Phi() const   { return X(); } // phi = rotational distance from the axis (like lattitude)
  inline     T& Phi()         { return X(); }
  ///////////////////////////////////////////////
  inline const T& Theta() const { return Y(); } // theta = rotation around the axis (like longitude)
  inline     T& Theta()       { return Y(); }
  ///////////////////////////////////////////////
  inline const T& Rho() const   { return Z(); } // rho = distance from origin
  inline     T& Rho()         { return Z(); }

  ///////////////////////////////////////////////
  inline const T* Data() const { return mData; }
  ///////////////////////////////////////////////
  inline     T* Data()       { return mData; }

  ///////////////////////////////////////////////
  inline const T& El(tU32 i) const { return mData[i]; }
  ///////////////////////////////////////////////
  inline     T& El(tU32 i)     { return mData[i]; }

  ///////////////////////////////////////////////
  inline tU32 GetWidth()  const { return N; }
  ///////////////////////////////////////////////
  inline tU32 GetHeight() const { return M; }

 private:
  T   mData[N*M];
};

//////////////////////////////////////////////////////////////////////////////////////////////
typedef XMatrix<2,2,tI32> XMatrix22l;
typedef XMatrix<2,3,tI32> XMatrix23l;
typedef XMatrix<3,3,tI32> XMatrix33l;
typedef XMatrix<4,4,tI32> XMatrix44l;
typedef XMatrix<2,1,tI32> XVec2l;
typedef XMatrix<3,1,tI32> XVec3l;
typedef XMatrix<4,1,tI32> XVec4l;
typedef XMatrix<2,2,tF32> XMatrix22f;
typedef XMatrix<2,3,tF32> XMatrix23f;
typedef XMatrix<3,3,tF32> XMatrix33f;
typedef XMatrix<4,4,tF32> XMatrix44f;
typedef XMatrix<2,1,tF32> XVec2f;
typedef XMatrix<3,1,tF32> XVec3f;
typedef XMatrix<4,1,tF32> XVec4f;
typedef XMatrix<2,2,tF64> XMatrix22d;
typedef XMatrix<2,3,tF64> XMatrix23d;
typedef XMatrix<3,3,tF64> XMatrix33d;
typedef XMatrix<4,4,tF64> XMatrix44d;
typedef XMatrix<2,1,tF64> XVec2d;
typedef XMatrix<3,1,tF64> XVec3d;
typedef XMatrix<4,1,tF64> XVec4d;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#ifndef niCore_NoWarnDisable
#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif
#endif
#endif // __MATHMATRIX_5021497_H__
