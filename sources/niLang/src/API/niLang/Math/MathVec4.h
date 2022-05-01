#ifndef __MATHVECTOR4_53492858427_H__
#define __MATHVECTOR4_53492858427_H__
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

///////////////////////////////////////////////
template<typename T>
inline sVec4<T>& VecClamp(sVec4<T>& Out, const sVec4<T>& V, const sVec4<T>& aMin = sVec4<T>::Zero(), const sVec4<T>& aMax = sVec4<T>::One())
{
  niLoop(i,Out.size()) {
    Out[i] = ni::Clamp(V[i],aMin[i],aMax[i]);
  }
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecAdd(sVec4<T>& Out,
                    const sVec4<T>& V1,
                    const sVec4<T>& V2)
{
  Out.x = V1.x + V2.x;
  Out.y = V1.y + V2.y;
  Out.z = V1.z + V2.z;
  Out.w = V1.w + V2.w;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecSub(sVec4<T>& Out,
                    const sVec4<T>& V1,
                    const sVec4<T>& V2)
{
  Out.x = V1.x - V2.x;
  Out.y = V1.y - V2.y;
  Out.z = V1.z - V2.z;
  Out.w = V1.w - V2.w;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecMul(sVec4<T>& Out,
                    const sVec4<T>& V1,
                    const sVec4<T>& V2)
{
  Out.x = V1.x * V2.x;
  Out.y = V1.y * V2.y;
  Out.z = V1.z * V2.z;
  Out.w = V1.w * V2.w;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecDiv(sVec4<T>& Out,
                    const sVec4<T>& V1,
                    const sVec4<T>& V2)
{
  Out.x = V1.x / V2.x;
  Out.y = V1.y / V2.y;
  Out.z = V1.z / V2.z;
  Out.w = V1.w / V2.w;

  return Out;
}

///////////////////////////////////////////////
// pA + pB*pC
template <class T>
sVec4<T>& VecMAD(sVec4<T>& Out,
                    const sVec4<T>& A,
                    const sVec4<T>& B,
                    const sVec4<T>& C)
{
  Out.x = (A.x * B.x) + C.x;
  Out.y = (A.y * B.y) + C.y;
  Out.z = (A.z * B.z) + C.z;
  Out.w = (A.w * B.w) + C.w;
  return Out;
}

///////////////////////////////////////////////
// pA + pB*scale
template <class T>
sVec4<T>& VecMAD(sVec4<T>& Out,
                    const sVec4<T>& A,
                    const T scale,
                    const sVec4<T>& B)
{
  Out.x = (A.x * scale) + B.x;
  Out.y = (A.y * scale) + B.y;
  Out.z = (A.z * scale) + B.z;
  Out.w = (A.w * scale) + B.w;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecBaryCentric(sVec4<T>& Out,
                            const sVec4<T>& V1,
                            const sVec4<T>& V2,
                            const sVec4<T>& V3,
                            T f,
                            T g)
{
  Out.x = V1.x + f * (V2.x - V1.x) + g * (V3.x - V1.x);
  Out.y = V1.y + f * (V2.y - V1.y) + g * (V3.y - V1.y);
  Out.z = V1.z + f * (V2.z - V1.z) + g * (V3.z - V1.z);
  Out.w = V1.w + f * (V2.w - V1.w) + g * (V3.w - V1.w);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecCatmullRom(sVec4<T>& Out,
                           const sVec4<T>& V1,
                           const sVec4<T>& V2,
                           const sVec4<T>& V3,
                           const sVec4<T>& V4,
                           T s)
{
  T ss, sss, a, b, c, d;

  ss  = s * s;
  sss = s * ss;

  a = -0.5f * sss + ss - 0.5f * s;
  b =  1.5f * sss - 2.5f * ss + 1.0f;
  c = -1.5f * sss + 2.0f * ss + 0.5f * s;
  d =  0.5f * sss - 0.5f * ss;

  Out.x = a * V1.x + b * V2.x + c * V3.x + d * V4.x;
  Out.y = a * V1.y + b * V2.y + c * V3.y + d * V4.y;
  Out.z = a * V1.z + b * V2.z + c * V3.z + d * V4.z;
  Out.w = a * V1.w + b * V2.w + c * V3.w + d * V4.w;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecCross(sVec4<T>& Out,
                      const sVec4<T>& V1,
                      const sVec4<T>& V2,
                      const sVec4<T>& V3)
{
  T a, b, c, d, e, f;

  a = V2.x * V3.y - V2.y * V3.x;
  b = V2.x * V3.z - V2.z * V3.x;
  c = V2.x * V3.w - V2.w * V3.x;
  d = V2.y * V3.z - V2.z * V3.y;
  e = V2.y * V3.w - V2.w * V3.y;
  f = V2.z * V3.w - V2.w * V3.z;

  T x =  f * V1.y - e * V1.z + d * V1.w;
  T y = -f * V1.x - c * V1.z + b * V1.w;
  T z =  e * V1.x - c * V1.y + a * V1.w;
  T w = -d * V1.x - b * V1.y + a * V1.z;
  Out.x = x;
  Out.y = y;
  Out.z = z;
  Out.w = w;
  return Out;
}
template <class T>
sVec4<T>* VecCross(sVec4<T>* Out,
                      const sVec4<T>* V1,
                      const sVec4<T>* V2,
                      const sVec4<T>* V3)
{
  return &VecCross(*Out,*V1,*V2,*V3);
}

///////////////////////////////////////////////
template <class T>
T VecDot(const sVec4<T>& V1, const sVec4<T>& V2)
{
  return V1.x * V2.x + V1.y * V2.y + V1.z * V2.z + V1.w * V2.w;
}
template <class T>
T VecDot(const sVec4<T>* V1, const sVec4<T>* V2)
{
  return VecDot(*V1,*V2);
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecHermite(sVec4<T>& Out,
                        const sVec4<T>& V1,
                        const sVec4<T>& T1,
                        const sVec4<T>& V2,
                        const sVec4<T>& T2,
                        T s)
{
  T ss, sss, a, b, c, d;

  ss  = s * s;
  sss = s * ss;

  a =  2.0f * sss - 3.0f * ss + 1.0f;
  b = -2.0f * sss + 3.0f * ss;
  c =  sss - 2.0f * ss + s;
  d =  sss - ss;

  Out.x = a * V1.x + b * V2.x + c * T1.x + d * T2.x;
  Out.y = a * V1.y + b * V2.y + c * T1.y + d * T2.y;
  Out.z = a * V1.z + b * V2.z + c * T1.z + d * T2.z;
  Out.w = a * V1.w + b * V2.w + c * T1.w + d * T2.w;

  return Out;
}

///////////////////////////////////////////////
template <class T>
T VecLength(const sVec4<T>& V)
{
  return (T)sqrt(V.x * V.x + V.y * V.y + V.z * V.z + V.w * V.w);
}

///////////////////////////////////////////////
template <class T>
T VecDistance(const sVec4<T>& A, const sVec4<T>& B)
{
  sVec4<T> v = {B.x-A.x, B.y-A.y, B.z-A.z, B.w-A.w};
  return (T)VecLength(v);
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecSetLength(sVec4<T>& Out, const sVec4<T>& V, T l)
{
  T fac = l/(T)sqrt(V.x*V.x + V.y*V.y + V.z*V.z + V.w*V.w);
  Out.x = V.x * fac;
  Out.y = V.y * fac;
  Out.z = V.z * fac;
  Out.w = V.w * fac;
  return Out;
}

///////////////////////////////////////////////
template <class T>
T VecLengthSq(const sVec4<T>& V)
{
  return V.x * V.x + V.y * V.y + V.z * V.z + V.w * V.w;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecMaximize(sVec4<T>& Out,
                         const sVec4<T>& V1,
                         const sVec4<T>& V2)
{
  Out.x = Max(V1.x, V2.x);
  Out.y = Max(V1.y, V2.y);
  Out.z = Max(V1.z, V2.z);
  Out.w = Max(V1.w, V2.w);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecMinimize(sVec4<T>& Out,
                         const sVec4<T>& V1,
                         const sVec4<T>& V2)
{
  Out.x = Min(V1.x, V2.x);
  Out.y = Min(V1.y, V2.y);
  Out.z = Min(V1.z, V2.z);
  Out.w = Min(V1.w, V2.w);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecNormalize(sVec4<T>& Out,
                          const sVec4<T>& V)
{
  T denom = (T)sqrt(V.x * V.x + V.y * V.y + V.z * V.z + V.w * V.w);
  if (denom == 0) { Out.x = Out.y = Out.z = Out.w = T(0); return Out; }
  denom = 1.0f / denom;
  Out.x = V.x * denom;
  Out.y = V.y * denom;
  Out.z = V.z * denom;
  Out.w = V.w * denom;
  return Out;
}
template <class T>
sVec4<T>* VecNormalize(sVec4<T>* Out,
                          const sVec4<T>* V)
{
  return &VecNormalize(*Out,*V);
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecNormalize(sVec4<T>& Out)
{
  return VecNormalize(Out, Out);
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecScale(sVec4<T>& Out,
                      const sVec4<T>& V,
                      T s)
{
  Out.x = V.x * s;
  Out.y = V.y * s;
  Out.z = V.z * s;
  Out.w = V.w * s;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecTransform(sVec4<T>& Out,
                          const sVec4<T>& V,
                          const sMatrix<T>& M)
{
  T x, y, z, w;

  x = V.x; y = V.y; z = V.z; w = V.w;

  Out.x = x * M._11 + y * M._21 + z * M._31 + w * M._41;
  Out.y = x * M._12 + y * M._22 + z * M._32 + w * M._42;
  Out.z = x * M._13 + y * M._23 + z * M._33 + w * M._43;
  Out.w = x * M._14 + y * M._24 + z * M._34 + w * M._44;

  return Out;
}
template <class T>
sVec4<T>* VecTransform(sVec4<T>* Out,
                          const sVec4<T>* V,
                          const sMatrix<T>* M)
{
  return &VecTransform(*Out,*V,*M);
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecAbs(sVec4<T>& Out, const sVec4<T>& V)
{
  Out.x = (T)Abs(V.x);
  Out.y = (T)Abs(V.y);
  Out.z = (T)Abs(V.z);
  Out.w = (T)Abs(V.w);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecLerp(sVec4<T>& Out,
                     const sVec4<T>& V1,
                     const sVec4<T>& V2,
                     T fac)
{
  Out.x = Lerp(V1.x, V2.x, fac);
  Out.y = Lerp(V1.y, V2.y, fac);
  Out.z = Lerp(V1.z, V2.z, fac);
  Out.w = Lerp(V1.w, V2.w, fac);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecLerp(sVec4<T>& Out,
                     const sVec4<T>& V1,
                     const sVec4<T>& V2,
                     const sVec4<T>& V3)
{
  Out.x = Lerp(V1.x, V2.x, V3.x);
  Out.y = Lerp(V1.y, V2.y, V3.y);
  Out.z = Lerp(V1.z, V2.z, V3.z);
  Out.w = Lerp(V1.w, V2.w, V3.w);
  return Out;
}

///////////////////////////////////////////////
template <class T>
tBool VecIsNormal(const sVec4<T>& v, T epsilon = niEpsilon5)
{
  T len = VecLength(v);
  return (len >= 1-epsilon && len <= 1-epsilon);
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecInverse(sVec4<T>& Out, const sVec4<T>& V)
{
  Out.x = 1/V.x;
  Out.y = 1/V.y;
  Out.z = 1/V.z;
  Out.w = 1/V.w;
  return Out;
}

///////////////////////////////////////////////
template <class T>
inline tBool VecEqual(const sVec4<T> &A, const sVec4<T> &B, T epsilon = niEpsilon5)
{
  return (ni::Abs(A.x-B.x)<=epsilon) &&
      (ni::Abs(A.y-B.y)<=epsilon) &&
      (ni::Abs(A.z-B.z)<=epsilon) &&
      (ni::Abs(A.w-B.w)<=epsilon);
}

///////////////////////////////////////////////
template <class T>
int VecMaxAxis(const sVec4<T>& V)
{
  int ret = 0;
  T fMaxNorm = (T)ni::Abs(V.x);
  if (fMaxNorm < ni::Abs(V.y)) {
    fMaxNorm = (T)ni::Abs(V.y);
    ret = 1;
  }
  if (fMaxNorm < ni::Abs(V.z)) {
    fMaxNorm = (T)ni::Abs(V.z);
    ret = 2;
  }
  if (fMaxNorm < ni::Abs(V.w)) {
    fMaxNorm = (T)ni::Abs(V.w);
    ret = 3;
  }
  return ret;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHVECTOR4_53492858427_H__
