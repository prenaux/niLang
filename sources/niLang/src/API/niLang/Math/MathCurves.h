#ifndef __MATHCURVES_11143252084_H__
#define __MATHCURVES_11143252084_H__
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
template <typename T>
inline T ClampT(T aX, T aMin, T aMax)
{
  if (aX < aMin) {
    return (T)0;
  }
  else if (aX >= aMax) {
    return (T)1;
  }
  const T range = aMax-aMin;
  return ni::FMod(aX-aMin,range)/range;
}

///////////////////////////////////////////////
template <typename T>
inline T RepeatT(T aX, T aMin, T aMax)
{
  const T range = aMax-aMin;
  const T cycleBase = ni::Abs(ni::FMod(aX-aMin,range)/range);
  T t;
  if (aX < aMin) {
    t = (T)1-cycleBase;
  }
  else {
    t = cycleBase;
  }
  return t;
}

///////////////////////////////////////////////
template <typename T>
inline T MirrorT(T aX, T aMin, T aMax)
{
  const T range = aMax-aMin;
  const T cycleBase = ni::Abs(ni::FMod(aX-aMin,range)/range);
  T t;
  t = cycleBase;
  return t;
}

///////////////////////////////////////////////
template <typename T>
inline T CycleT(T aX, T aMin, T aMax)
{
  const T range = aMax-aMin;
  const T cycleBase = ni::Abs(ni::FMod(aX-aMin,range*2)/range);
  T t;
  if (cycleBase > 1) {
    t = (T)1-(cycleBase-(T)1);
  }
  else {
    t = cycleBase;
  }
  return t;
}

///////////////////////////////////////////////
template <typename T, typename S>
T CurveStep(const T& a, const T& b, const S t) {
  return t < (S)0.5 ? a : b;
}

///////////////////////////////////////////////
template <class T, class S>
T CurveLinear(const T& V1,
              const T& V2,
              S s)
{
  return Lerp(V1,V2,s);
}

///////////////////////////////////////////////
template <typename T, typename S>
T CurveCos(const T& a, const T& b, const S t) {
  const S cosT = ((ni::Cos(ni::Lerp((S)niPi,(S)0,t)))+(S)1)/(S)2;
  return ni::Lerp(a,b,cosT);
}

///////////////////////////////////////////////
template <class T, class S>
T CurveHermite(const T& V1,
               const T& T1,
               const T& V2,
               const T& T2,
               S s)
{
  S ss, sss, a, b, c, d;
  ss  = s * s;
  sss = s * ss;
  a =  (S)2 * sss - (S)3 * ss + (S)1;
  b = -(S)2 * sss + (S)3 * ss;
  c =  sss - (S)2 * ss + s;
  d =  sss - ss;
  return a * V1 + b * V2 + c * T1 + d * T2;
}

///////////////////////////////////////////////
template <class T, class S>
T CurveCatmullRom(const T& V1,
                  const T& V2,
                  const T& V3,
                  const T& V4,
                  S s)
{
  S ss, sss, a, b, c, d;
  ss  = s * s;
  sss = s * ss;
  a = -(S)0.5 * sss + ss - (S)0.5 * s;
  b =  (S)1.5 * sss - (S)2.5 * ss + (S)1.0;
  c = -(S)1.5 * sss + (S)2.0 * ss + (S)0.5 * s;
  d =  (S)0.5 * sss - (S)0.5 * ss;
  return a * V1 + b * V2 + c * V3 + d * V4;
}

///////////////////////////////////////////////
template <class T, class S>
T CurveCardinal(const T& V1,
                const T& V2,
                const T& V3,
                const T& V4,
                S s, S a)
{
  T T2 = (V3-V1)*a;
  T T3 = (V4-V2)*a;
  return CurveHermite(V2,T2,V3,T3,s);
}

///////////////////////////////////////////////
template <class T, class S>
T CurveCardinal(const T& V1, const T& V2, S s, S a)
{
  return CurveCardinal(V1,V1,V2,V2,s,a);
}

///////////////////////////////////////////////
template <typename T>
inline T CycleLinear(T aX, T aMin, T aMax) {
  return ni::CurveLinear(aMin,aMax,ni::CycleT(aX,aMin,aMax));
}

///////////////////////////////////////////////
template <typename T>
inline T CycleCos(T aX, T aMin, T aMax) {
  return ni::CurveCos(aMin,aMax,ni::CycleT(aX,aMin,aMax));
}

///////////////////////////////////////////////
template <typename T>
inline T CycleCardinal(T aX, T aMin, T aMax, const T a) {
  return ni::CurveCardinal(aMin,aMax,ni::CycleT(aX,aMin,aMax),a);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHCURVES_11143252084_H__
