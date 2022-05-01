#ifndef __MATHCOMPLEX_2570459_H__
#define __MATHCOMPLEX_2570459_H__
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

//! Complex number template
template <typename T>
struct Complex : public ni::sVec2<T>
{
  //! Check whether the complex number is a pure real number.
  tBool __stdcall IsReal() const {
    return this->im() == T(0);
  }
  //! Check whether the complex number is a pure imaginary number.
  tBool __stdcall IsImaginary() const {
    return this->re() == T(0);
  }
  //! Check whether the complex number is a complex number.
  tBool __stdcall IsComplex() const {
    return !IsReal() && !IsImaginary();
  }

  //! positive sign
  inline Complex operator + () const {
    return *this;
  }

  //! negate sign
  inline Complex operator - () const {
    Complex c;
    c.re() = -this->re();
    c.im() = -this->im();
    return c;
  }

  //! this += complex operator
  inline Complex& operator += (const ni::sVec2<T>& c) {
    this->re() += c.re();
    this->im() += c.im();
    return *this;
  }
  //! this += scalar operator
  inline Complex& operator += (T s) {
    this->re() += s;
    return *this;
  }
  //! this + complex operator
  inline Complex operator + (const ni::sVec2<T>& c) const {
    Complex r;
    r.re() = this->re()+c.re();
    r.im() = this->im()+c.im();
    return r;
  }
  //! this + scalar operator
  inline Complex operator + (T s) const {
    Complex c;
    c.re() = this->re()+s;
    c.im() = this->im();
    return c;
  }

  //! this -= complex operator
  inline Complex& operator -= (const ni::sVec2<T>& c) {
    this->re() -= c.re();
    this->im() -= c.im();
    return *this;
  }
  //! this -= scalar operator
  inline Complex& operator -= (T s) {
    this->re() -= s;
    return *this;
  }
  //! this - complex operator
  inline Complex operator - (const ni::sVec2<T>& c) const {
    Complex r;
    r.re() = this->re()-c.re();
    r.im() = this->im()-c.im();
    return r;
  }
  //! this - scalar operator
  inline Complex operator - (T s) const {
    Complex c;
    c.re() = this->re()-s;
    c.im() = this->im();
    return c;
  }

  //! this *= complex operator
  inline Complex& operator *= (const ni::sVec2<T>& c) {
    T r = this->re() * c.re() - this->im() * c.im();
    T i = this->re() * c.im() + this->im() * c.re();
    this->re() = r;
    this->im() = i;
    return *this;
  }
  //! this *= scalar operator
  inline Complex& operator *= (T s) {
    this->re() *= s;
    this->im() *= s;
    return *this;
  }
  //! this * complex operator
  inline Complex operator * (const ni::sVec2<T>& c) const {
    Complex<T> r;
    r.re() = this->re() * c.re() - this->im() * c.im();
    r.im() = this->re() * c.im() + this->im() * c.re();
    return r;
  }
  //! this * scalar operator
  inline Complex operator * (T s) const {
    Complex<T> r;
    r.re() = this->re()*s;
    r.im() = this->im()*s;
    return r;
  }

  //! helper complex division method
  inline static void _div(const T& ar, const T& ai, const T& br, const T& bi, T& rr, T& ri) {
    T tr = br >= 0 ? br : -br;
    T ti = bi >= 0 ? bi : -bi;
    if (tr <= ti) {
      T ratio = br / bi;
      T denom = bi * (1 + ratio * ratio);
      rr = (ar * ratio + ai) / denom;
      ri = (ai * ratio - ar) / denom;
    }
    else {
      T ratio = bi / br;
      T denom = br * (1 + ratio * ratio);
      rr = (ar + ai * ratio) / denom;
      ri = (ai - ar * ratio) / denom;
    }
  }
  inline static void _div(const T& ar, const T& br, const T& bi, T& rr, T& ri) {
    T tr = br >= 0 ? br : -br;
    T ti = bi >= 0 ? bi : -bi;
    if (tr <= ti) {
      T ratio = br / bi;
      T denom = bi * (1 + ratio * ratio);
      rr = (ar * ratio) / denom;
      ri = (- ar) / denom;
    }
    else {
      T ratio = bi / br;
      T denom = br * (1 + ratio * ratio);
      rr = (ar) / denom;
      ri = (- ar * ratio) / denom;
    }
  }

  //! inverse of this complex number
  inline Complex<T> __stdcall Inverse() const {
    Complex<T> ret;
    _div(1.0f,this->re(),this->im(),ret.re(),ret.im());
    return ret;
  }

  //! this /= complex operator
  inline Complex& operator /= (const ni::sVec2<T>& c) {
    T r, i;
    _div(this->re(),this->im(),c.re(),c.im(),r,i);
    this->re() = r;
    this->im() = i;
    return *this;
  }
  //! this /= scalar operator
  inline Complex& operator /= (T s) {
    this->re() /= s;
    this->im() /= s;
    return *this;
  }
  //! this / complex operator
  inline Complex operator / (const ni::sVec2<T>& c) const {
    Complex ret;
    _div(this->re(),this->im(),c.re(),c.im(),ret.re(),ret.im());
    return ret;
  }
  //! this / scalar operator
  inline Complex operator / (T s) const {
    Complex c;
    c.re() = this->re()/s;
    c.im() = this->im()/s;
    return c;
  }

  //! Conjugate
  inline Complex __stdcall Conjugate() const {
    Complex c;
    c.re() = this->re();
    c.im() = -this->im();
    return c;
  }

  //! Modulus
  inline T __stdcall Modulus() const {
    return ni::Sqrt(ModulusSq());
  }

  //! Squared Modulus
  inline T __stdcall ModulusSq() const {
    return ni::Sqr(this->re()) + ni::Sqr(this->im());
  }

  //! Argument
  inline T __stdcall Argument() const {
    return ni::ATan2(this->im(),this->re());
  }

  //! Square root
  inline Complex<T> __stdcall Sqrt() const {
    T mag = Modulus();
    Complex<T> result;

    if (mag == T(0)) {
      result.re() = result.im() = T(0);
    }
    else if (this->re() > T(0)) {
      result.re() = ni::Sqrt(T(0.5) * (mag + this->re()));
      result.im() = this->im()/result.re()/2;
    }
    else {
      result.im() = ni::Sqrt(T(0.5) * (mag - this->re()));
      if (this->im() < T(0)) {
        result.im() = -result.im();
      }
      result.re() = this->im()/result.im()/2;
    }

    return result;
  }

  //! Exponent
  inline Complex<T> __stdcall Exp() const  {
    Complex c;
    T expx = ni::Exp(this->re());
    c.re() = expx * ni::Cos(this->im());
    c.im() = expx * ni::Sin(this->im());
    return c;
  }

  //! Logarithm base 10
  inline Complex<T> __stdcall Log10() const
  {
    static T ln10_inv = T(1) / ni::LogE(T(10));
    Complex<T> r;
    r.im() = ni::ATan2(this->im(),this->re()) * ln10_inv;
    r.re() = ni::Log10(Modulus());
    return r;
  }

  //! Logarithm base e
  inline Complex<T> __stdcall LogE() const
  {
    Complex<T> r;
    r.im() = ni::ATan2(this->im(),this->re());
    r.re() = ni::LogE(Modulus());
    return r;
  }

  //! Power
  inline Complex<T> __stdcall Pow(const Complex<T>& b) const {
    Complex<T> c;
    T logr = ni::LogE(ni::Hypot(this->re(),this->im()));
    T logi = ni::ATan2(this->im(),this->re());
    T x = ni::Exp(logr*b.re() - logi*b.im());
    T y = logr*b.im() + logi*b.re();
    c.re() = x * ni::Cos(y);
    c.im() = x * ni::Sin(y);
    return c;
  }

  //! Sinus
  inline Complex<T> __stdcall Sin() const {
    Complex<T> c;
    c.re() = ni::Sin(this->re()) * ni::Cosh(this->im());
    c.im() = ni::Cos(this->re()) * ni::Sinh(this->im());
    return c;
  }

  //! Cosinus
  inline Complex<T> __stdcall Cos() const {
    Complex<T> c;
    c.re() = ni::Cos(this->re()) * ni::Cosh(this->im());
    c.im() = ni::Sin(this->re()) * ni::Sinh(this->im());
    return c;
  }

  //! Tangent
  inline Complex<T> __stdcall Tan() const {
    Complex<T> c;
    T re2 = T(2) * this->re();
    T im2 = T(2) * this->im();
    if (ni::Abs(im2) >= ni::TypeMax<T>()) {
      c.re() = T(0);
      c.im() = (im2 > T(0) ? T(1) : T(-1));
    }
    else {
      T den = ni::Cos(re2) + ni::Cosh(im2);
      c.re() = ni::Sin(re2) / den;
      c.im() = ni::Sinh(im2) / den;
    }
    return c;
  }
};

template<typename T> inline Complex<T> cx(T re, T im = T(0)) { Complex<T> c; c.re() = re; c.im() = im; return c; }
template<typename T> inline Complex<T> cx(const ni::sVec2<T>& c) { cx<T>(c.re(),c.im()); }
template<typename T> inline Complex<T> re(T v) { return cx<T>(v,T(0)); }
template<typename T> inline Complex<T> im(T v) { return cx<T>(T(0),v); }
template<typename T> inline T re(const Complex<T>& v) { return v.re(); }
template<typename T> inline T im(const Complex<T>& v) { return v.im(); }
template<> inline Complex<tF32> Num<Complex<tF32> >(const tF64 a) { return cx<tF32>((tF32)a); }
template<> inline Complex<tF64> Num<Complex<tF64> >(const tF64 a) { return cx<tF64>((tF64)a); }

template <typename T>
inline bool operator == (T v, const Complex<T>& c) {
  return v == c.re();
}
template <typename T>
inline bool operator != (T v, const Complex<T>& c) {
  return v != c.re();
}

template <typename T>
inline Complex<T> Polar(T rho, T phi) {
  return cx<T>(rho*ni::Cos(phi),rho*ni::Sin(phi));
}

template <typename T>
inline Complex<T> __stdcall Conjugate(const Complex<T>& c) {
  return c.Conjugate();
}

template <typename T>
inline T __stdcall Modulus(const Complex<T>& c) {
  return c.Modulus();
}

template <typename T>
inline T __stdcall ModulusSq(const Complex<T>& c) {
  return c.ModulusSq();
}

template <typename T>
inline T __stdcall Argument(const Complex<T>& c) {
  return c.Argument();
}

template <typename T>
inline Complex<T> Sqrt(const Complex<T>& c) {
  return c.Sqrt();
}

template <typename T>
inline Complex<T> Exp(const Complex<T>& c) {
  return c.Exp();
}

template <typename T>
inline Complex<T> Log10(const Complex<T>& c) {
  return c.Log10();
}

template <typename T>
inline Complex<T> LogE(const Complex<T>& c) {
  return c.LogE();
}

template <typename T>
inline Complex<T> Pow(const Complex<T>& c, const Complex<T>& b) {
  return c.Pow(b);
}

template <typename T>
inline Complex<T> Sin(const Complex<T>& c) {
  return c.Sin();
}

template <typename T>
inline Complex<T> Cos(const Complex<T>& c) {
  return c.Cos();
}

template <typename T>
inline Complex<T> Tan(const Complex<T>& c) {
  return c.Tan();
}

//! tF32 complex number
typedef Complex<tF32> Complexf;
inline Complexf ref(tF32 v) { return re<tF32>(v); }
inline Complexf imf(tF32 v) { return im<tF32>(v); }
//! tF64 complex number
typedef Complex<tF64> Complexd;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHCOMPLEX_2570459_H__
