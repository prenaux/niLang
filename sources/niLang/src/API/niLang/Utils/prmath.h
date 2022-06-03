#pragma once
#ifndef __PRMATH_H_D180A6E6_60F0_8441_8D9A_67144A922C87__
#define __PRMATH_H_D180A6E6_60F0_8441_8D9A_67144A922C87__
//
// Modernize niLang/Math with value based APIs.
//
#include "../Math/MathVec3.h"
#include "../Math/MathMatrix.h"

namespace prmath {

template <typename T>
T dot(const ni::sVec3<T>& a, const ni::sVec3<T>& b) {
  return VecDot(a,b);
}

template <typename T>
ni::sVec3<T> normalize(const ni::sVec3<T>& v) {
  ni::sVec3<T> r = v;
  return VecNormalize(r);
}

template <typename T>
T saturate(const T& v) {
  return ni::Clamp(v,(T)0,(T)1);
}

template <typename T>
ni::sVec2<T> saturate(const ni::sVec2<T>& v) {
  ni::sVec2<T> r = v;
  VecMinimize(r,ni::sVec2f::One(),r);
  VecMaximize(r,ni::sVec2f::Zero(),r);
  return r;
}

template <typename T>
ni::sVec3<T> saturate(const ni::sVec3<T>& v) {
  ni::sVec3<T> r = v;
  VecMinimize(r,ni::sVec3f::One(),r);
  VecMaximize(r,ni::sVec3f::Zero(),r);
  return r;
}

template <typename T>
ni::sVec4<T> saturate(const ni::sVec4<T>& v) {
  ni::sVec4<T> r = v;
  VecMinimize(r,ni::sVec4f::One(),r);
  VecMaximize(r,ni::sVec4f::Zero(),r);
  return r;
}

template <typename T>
ni::sMatrix<T> translation(const ni::sVec3<T>& p)
{
  ni::sMatrix<T> m = ni::sMatrix<T>::Identity();
  return ni::MatrixTranslation(m,p);
}

template <typename T>
ni::sVec3<T> cross(const ni::sVec3<T>& a, const ni::sVec3<T>& b) {
  ni::sVec3<T> r;
  return VecCross(r,a,b);
}

}
#endif // __PRMATH_H_D180A6E6_60F0_8441_8D9A_67144A922C87__
