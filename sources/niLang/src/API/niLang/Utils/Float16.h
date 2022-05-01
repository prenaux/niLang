#ifndef __FLOAT16_51586324_H__
#define __FLOAT16_51586324_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

niExportFunc(ni::tU16) ni_f32_to_f16(ni::tF32 v);
niExportFunc(ni::tF32) ni_f16_to_f32(ni::tU16 v);

//! 16bits IEEE Floating point number class.
class cFloat16
{
 public:
  inline cFloat16();         // no initialization
  inline cFloat16(const cFloat16 &h);
  inline cFloat16(tF32 f);
  explicit cFloat16(tU16 aValue) : mValue(aValue) {}

  inline cFloat16 operator - () const;
  inline cFloat16 operator = (cFloat16  h);
  inline cFloat16 operator = (tF32 f);
  inline cFloat16 operator += (cFloat16  h);
  inline cFloat16 operator += (tF32 f);
  inline cFloat16 operator -= (cFloat16  h);
  inline cFloat16 operator -= (tF32 f);
  inline cFloat16 operator *= (cFloat16  h);
  inline cFloat16 operator *= (tF32 f);
  inline cFloat16 operator /= (cFloat16  h);
  inline cFloat16 operator /= (tF32 f);

  //---------------------------------------------------------
  // Round to n-bit precision (n should be between 0 and 10).
  // After rounding, the significand's 10-n least significant
  // bits will be zero.
  //---------------------------------------------------------

  inline cFloat16 round(tU32 n) const;


  //--------------------------------------------------------------------
  // Classification:
  //
  //   h.isFinite()      returns true if h is a normalized number,
  //            a denormalized number or zero
  //
  //   h.isNormalized()   returns true if h is a normalized number
  //
  //   h.isDenormalized()   returns true if h is a denormalized number
  //
  //   h.isZero()      returns true if h is zero
  //
  //   h.isNan()      returns true if h is a NAN
  //
  //   h.isInfinity()      returns true if h is a positive
  //            or a negative infinity
  //
  //   h.isNegative()      returns true if the sign bit of h
  //            is set (negative)
  //--------------------------------------------------------------------
  inline bool IsFinite() const;
  inline bool IsNormalized() const;
  inline bool IsDenormalized() const;
  inline bool IsZero() const;
  inline bool IsNan() const;
  inline bool IsInfinity() const;
  inline bool IsNegative() const;


  //--------------------------------------------
  // Special values
  //
  //   posInf()   returns +infinity
  //
  //   negInf()   returns +infinity
  //
  //   qNan()      returns a NAN with the bit
  //         pattern 0111111111111111
  //
  //   sNan()      returns a NAN with the bit
  //         pattern 0111110111111111
  //--------------------------------------------
  static inline cFloat16 posInf();
  static inline cFloat16 negInf();
  static inline cFloat16 qNan();
  static inline cFloat16 sNan();


  //--------------------------------------
  // Access to the internal representation
  //--------------------------------------
  inline tU16 GetBits() const;
  inline void SetBits(tU16 bits);

  inline operator tF32 () const;
  inline tF32   ToFloat() const;

 private:
  tU16   mValue;
};

//-------
// Limits
//-------

#define niMinF16   5.96046448e-08f   // Smallest positive cFloat16

#define niMinNrmF16 6.10351562e-05f   // Smallest positive normalized cFloat16

#define niMaxF16   65504.0f      // Largest positive cFloat16

#define niEpsilonF16   0.00097656f   // Smallest positive e for which cFloat16 (1.0 + e) != cFloat16 (1.0)

#define niMantiDigiF16   11      // Number of digits in mantissa (significand + hidden leading 1)

#define niDifF16   2      // Number of base 10 digits that can be represented without change

#define niRadixF16 2      // Base of the exponent

#define niMinExpF16  -13  // Minimum negative integer such that
// HALF_RADIX raised to the power of
// one less than that integer is a
// normalized cFloat16

#define niMaxExpF16   16  // Maximum positive integer such that
// HALF_RADIX raised to the power of
// one less than that integer is a
// normalized cFloat16

#define niMin10ExpF16   -4    // Minimum positive integer such
// that 10 raised to that power is
// a normalized cFloat16

#define niMax10ExpF16 4   // Maximum positive integer such
// that 10 raised to that power is
// a normalized cFloat16

///////////////////////////////////////////////
inline cFloat16::cFloat16 ()
{
  // no initialization
}

///////////////////////////////////////////////
inline cFloat16::cFloat16(const cFloat16 &h)
{
  mValue = h.mValue;
}

///////////////////////////////////////////////
// float16 from tF32
inline cFloat16::cFloat16(tF32 f)
{
  mValue = ni_f32_to_f16(f);
}

///////////////////////////////////////////////
// tF16-to-tF32 conversion via table lookup
inline cFloat16::operator tF32 () const
{
  return ToFloat();
}

///////////////////////////////////////////////
// Round to n-bit precision
inline cFloat16 cFloat16::round (tU32 n) const
{
  // Parameter check.
  if (n >= 10)
    return *this;

  // Disassemble h into the sign, s,
  // and the combined exponent and significand, e.
  tU16 s = mValue & 0x8000;
  tU16 e = mValue & 0x7fff;

  // Round the exponent and significand to the nearest value
  // where ones occur only in the (10-n) most significant bits.
  // Note that the exponent adjusts automatically if rounding
  // up causes the significand to overflow.
  e >>= 9 - n;
  e  += e & 1;
  e <<= 9 - n;

  // Check for exponent overflow.
  if (e >= 0x7c00)
  {
    // Overflow occurred -- truncate instead of rounding.
    e = mValue;
    e >>= 10 - n;
    e <<= 10 - n;
  }

  // Put the original sign bit back.
  cFloat16 h;
  h.mValue = s | e;
  return h;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator - () const
{
  cFloat16 h;
  h.mValue = mValue ^ 0x8000;
  return h;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator = (cFloat16 h)
{
  mValue = h.mValue;
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator = (tF32 f)
{
  *this = cFloat16 (f);
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator += (cFloat16 h)
{
  *this = cFloat16 (tF32 (*this) + tF32 (h));
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator += (tF32 f)
{
  *this = cFloat16 (tF32 (*this) + f);
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator -= (cFloat16 h)
{
  *this = cFloat16 (tF32 (*this) - tF32 (h));
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator -= (tF32 f)
{
  *this = cFloat16 (tF32 (*this) - f);
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator *= (cFloat16 h)
{
  *this = cFloat16 (tF32 (*this) * tF32 (h));
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator *= (tF32 f)
{
  *this = cFloat16 (tF32 (*this) * f);
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator /= (cFloat16 h)
{
  *this = cFloat16 (tF32 (*this) / tF32 (h));
  return *this;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::operator /= (tF32 f)
{
  *this = cFloat16 (tF32 (*this) / f);
  return *this;
}

///////////////////////////////////////////////
inline bool cFloat16::IsFinite () const
{
  tU16 e = (mValue >> 10) & 0x001f;
  return e < 31;
}

///////////////////////////////////////////////
inline bool cFloat16::IsNormalized () const
{
  tU16 e = (mValue >> 10) & 0x001f;
  return e > 0 && e < 31;
}

///////////////////////////////////////////////
inline bool cFloat16::IsDenormalized () const
{
  tU16 e = (mValue >> 10) & 0x001f;
  tU16 m =  mValue & 0x3ff;
  return e == 0 && m != 0;
}

///////////////////////////////////////////////
inline bool cFloat16::IsZero() const
{
  return (mValue & 0x7fff) == 0;
}

///////////////////////////////////////////////
inline bool cFloat16::IsNan() const
{
  tU16 e = (mValue >> 10) & 0x001f;
  tU16 m =  mValue & 0x3ff;
  return e == 31 && m != 0;
}

///////////////////////////////////////////////
inline bool cFloat16::IsInfinity () const
{
  tU16 e = (mValue >> 10) & 0x001f;
  tU16 m =  mValue & 0x3ff;
  return e == 31 && m == 0;
}

///////////////////////////////////////////////
inline bool cFloat16::IsNegative () const
{
  return (mValue & 0x8000) != 0;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::posInf()
{
  cFloat16 h;
  h.mValue = 0x7c00;
  return h;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::negInf()
{
  cFloat16 h;
  h.mValue = 0xfc00;
  return h;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::qNan()
{
  cFloat16 h;
  h.mValue = 0x7fff;
  return h;
}

///////////////////////////////////////////////
inline cFloat16 cFloat16::sNan ()
{
  cFloat16 h;
  h.mValue = 0x7dff;
  return h;
}

///////////////////////////////////////////////
inline tU16 cFloat16::GetBits() const
{
  return mValue;
}

///////////////////////////////////////////////
inline void cFloat16::SetBits(tU16 bits)
{
  mValue = bits;
}

///////////////////////////////////////////////
inline tF32 cFloat16::ToFloat() const
{
  return ni_f16_to_f32(mValue);
}

typedef cFloat16  tF16;

const tF16 kMaxF16 = tF16((tU16)0x7BFF);
const tF16 kMinF16 = tF16((tU16)0x0001);
const tF16 kEpsilonF16 = tF16((tU16)0x1400);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
} // End of ni Namespace
#endif // __FLOAT16_51586324_H__
