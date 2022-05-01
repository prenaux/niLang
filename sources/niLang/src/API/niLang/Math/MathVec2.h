#ifndef __MATHVECTOR2_11143252084_H__
#define __MATHVECTOR2_11143252084_H__
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
inline sVec2<T>& VecClamp(sVec2<T>& Out, const sVec2<T>& V, const sVec2<T>& aMin = sVec2<T>::Zero(), const sVec2<T>& aMax = sVec2<T>::One())
{
  niLoop(i,Out.size()) {
    Out[i] = ni::Clamp(V[i],aMin[i],aMax[i]);
  }
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecAdd(sVec2<T>& Out,
                    const sVec2<T>& V1,
                    const sVec2<T>& V2)
{
  Out.x = V1.x + V2.x;
  Out.y = V1.y + V2.y;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecMul(sVec2<T>& Out,
                    const sVec2<T>& V1,
                    const sVec2<T>& V2)
{
  Out.x = V1.x * V2.x;
  Out.y = V1.y * V2.y;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecDiv(sVec2<T>& Out,
                    const sVec2<T>& V1,
                    const sVec2<T>& V2)
{
  Out.x = V1.x / V2.x;
  Out.y = V1.y / V2.y;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecMAD(sVec2<T>& Out,
                    const sVec2<T>& A,
                    const sVec2<T>& B,
                    const sVec2<T>& C)
{
  Out.x = (A.x*B.x) + C.x;
  Out.y = (A.y*B.y) + C.y;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecMAD(sVec2<T>& Out,
                    const sVec2<T>& A,
                    T scale,
                    const sVec2<T>& B)
{
  Out.x = (A.x*scale) + B.x;
  Out.y = (A.y*scale) + B.y;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecMAD(sVec2<T>& Out,
                    const sVec2<T>& A,
                    T scale,
                    T add)
{
  Out.x = (A.x*scale) + add;
  Out.y = (A.y*scale) + add;
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecSub(sVec2<T>& Out, const sVec2<T>& V1, const sVec2<T>& V2)
{
  Out.x = V1.x - V2.x;
  Out.y = V1.y - V2.y;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecBaryCentric(sVec2<T>& Out,
                            const sVec2<T>& V1,
                            const sVec2<T>& V2,
                            const sVec2<T>& V3,
                            T f,
                            T g)
{
  Out.x = V1.x + f * (V2.x - V1.x) + g * (V3.x - V1.x);
  Out.y = V1.y + f * (V2.y - V1.y) + g * (V3.y - V1.y);

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecCatmullRom(sVec2<T>& Out,
                           const sVec2<T>& V1,
                           const sVec2<T>& V2,
                           const sVec2<T>& V3,
                           const sVec2<T>& V4,
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

  return Out;
}

///////////////////////////////////////////////
template <class T>
T VecCCW(const sVec2<T>& V1,
         const sVec2<T>& V2)
{
  return V1.x * V2.y - V1.y * V2.x;
}

///////////////////////////////////////////////
template <class T>
T VecDot(const sVec2<T>& V1,
         const sVec2<T>& V2)
{
  return V1.x * V2.x + V1.y * V2.y;
}
template <class T>
T VecDot(const sVec2<T>* V1,
         const sVec2<T>* V2)
{
  return VecDot(*V1,*V2);
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecHermite(sVec2<T>& Out,
                        const sVec2<T>& V1,
                        const sVec2<T>& T1,
                        const sVec2<T>& V2,
                        const sVec2<T>& T2,
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

  return Out;
}

///////////////////////////////////////////////
template <class T>
T VecLength(const sVec2<T>& V)
{
  return (T)sqrt(V.x * V.x + V.y * V.y);
}

///////////////////////////////////////////////
template <class T>
T VecDistance(const sVec2<T>& A, const sVec2<T>& B)
{
  sVec2<T> v = {B.x-A.x, B.y-A.y};
  return (T)VecLength(v);
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecSetLength(sVec2<T>& Out, const sVec2<T>& V, T l)
{
  T fac = l/(T)sqrt(V.x*V.x + V.y*V.y);
  Out.x = V.x * fac;
  Out.y = V.y * fac;
  return Out;
}

///////////////////////////////////////////////
template <class T>
T VecLengthSq(const sVec2<T>& V)
{
  return V.x * V.x + V.y * V.y;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecMaximize(sVec2<T>& Out,
                         const sVec2<T>& V1,
                         const sVec2<T>& V2)
{
  Out.x = Max(V1.x, V2.x);
  Out.y = Max(V1.y, V2.y);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecMinimize(sVec2<T>& Out,
                         const sVec2<T>& V1,
                         const sVec2<T>& V2)
{
  Out.x = Min(V1.x, V2.x);
  Out.y = Min(V1.y, V2.y);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecNormalize(sVec2<T>& Out, const sVec2<T>& V)
{
  T denom = ni::Sqrt(V.x * V.x + V.y * V.y);
  if (denom == 0) { Out.x = Out.y = T(0); return Out; }
  denom = 1.0f / denom;
  Out.x = V.x * denom;
  Out.y = V.y * denom;
  return Out;
}
template <class T>
sVec2<T>* VecNormalize(sVec2<T>* Out, const sVec2<T>* V)
{
  return &VecNormalize(*Out,*V);
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecNormalize(sVec2<T>& Out)
{
  return VecNormalize(Out, Out);
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecScale(sVec2<T>& Out,
                      const sVec2<T>& V,
                      T s)
{
  Out.x = V.x * s;
  Out.y = V.y * s;

  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec4<T>& VecTransform(sVec4<T>& Out,
                          const sVec2<T>& V,
                          const sMatrix<T>& M)
{
  T x, y;

  x = V.x; y = V.y;

  Out.x = x * M._11 + y * M._21 + M._41;
  Out.y = x * M._12 + y * M._22 + M._42;
  Out.z = x * M._13 + y * M._23 + M._43;
  Out.w = x * M._14 + y * M._24 + M._44;

  return Out;
}
template <class T>
sVec4<T>* VecTransform(sVec4<T>* Out,
                          const sVec2<T>* V,
                          const sMatrix<T>* M)
{
  return &VecTransform(*Out,*V,*M);
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecTransformCoord(sVec2<T>& Out,
                               const sVec2<T>& V,
                               const sMatrix<T>& M)
{
  T rhw, x, y;

  x = V.x; y = V.y;

  rhw = (x * M._14 + y * M._24 + M._44);
  if (rhw == 0)
  {
    Out.x = T(0);
    Out.y = T(0);
    return Out;
  }

  rhw = 1.0f / rhw;

  Out.x = rhw * (x * M._11 + y * M._21 + M._41);
  Out.y = rhw * (x * M._12 + y * M._22 + M._42);

  return Out;
}
template <class T>
sVec2<T>* VecTransformCoord(sVec2<T>* Out,
                               const sVec2<T>* V,
                               const sMatrix<T>* M)
{
  return &VecTransformCoord(*Out,*V,*M);
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecTransformNormal(sVec2<T>& Out,
                                const sVec2<T>& V,
                                const sMatrix<T>& M)
{
  T x, y;

  x = V.x; y = V.y;

  Out.x = x * M._11 + y * M._21;
  Out.y = x * M._12 + y * M._22;

  return Out;
}
template <class T>
sVec2<T>* VecTransformNormal(sVec2<T>* Out,
                                const sVec2<T>* V,
                                const sMatrix<T>* M)
{
  return &VecTransformNormal(&Out,&V,&M);
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecAbs(sVec2<T>& Out, const sVec2<T>& V)
{
  Out.x = (T)Abs(V.x);
  Out.y = (T)Abs(V.y);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecLerp(sVec2<T>& Out,
                     const sVec2<T>& V1,
                     const sVec2<T>& V2,
                     T fac)
{
  Out.x = Lerp(V1.x, V2.x, fac);
  Out.y = Lerp(V1.y, V2.y, fac);
  return Out;
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecLerp(sVec2<T>& Out,
                     const sVec2<T>& V1,
                     const sVec2<T>& V2,
                     const sVec2<T>& V3)
{
  Out.x = Lerp(V1.x, V2.x, V3.x);
  Out.y = Lerp(V1.y, V2.y, V3.y);
  return Out;
}

///////////////////////////////////////////////
template <class T>
tBool VecIsNormal(const sVec2<T>& v, T epsilon = niEpsilon5)
{
  T len = VecLength(v);
  return (len >= 1-epsilon && len <= 1-epsilon);
}

///////////////////////////////////////////////
template <class T>
sVec2<T>& VecInverse(sVec2<T>& Out, const sVec2<T>& V)
{
  Out.x = 1/V.x;
  Out.y = 1/V.y;
  return Out;
}

///////////////////////////////////////////////
template <class T>
inline tBool VecEqual(const sVec2<T>& A, const sVec2<T>& B, T epsilon = niEpsilon5)
{
  return (ni::Abs(A.x-B.x)<=epsilon) &&
      (ni::Abs(A.y-B.y)<=epsilon);
}

///////////////////////////////////////////////
template <class T>
int VecMaxAxis(const sVec2<T>& V)
{
  int ret = 0;
  T fMaxNorm = (T)ni::Abs(V.x);
  if (fMaxNorm < ni::Abs(V.y)) {
    fMaxNorm = (T)ni::Abs(V.y);
    ret = 1;
  }
  return ret;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHVECTOR2_11143252084_H__
