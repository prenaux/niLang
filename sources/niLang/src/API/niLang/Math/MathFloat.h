#ifndef __MATHFLOAT_44024344_H__
#define __MATHFLOAT_44024344_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"

#include <float.h>

#ifdef __cplusplus
#include <niLang/STL/limits.h>
#endif

namespace ni {
/** \addtogroup niLang
 * @{
 */

//===========================================================================
//
//  Fast Floats
//
//===========================================================================
__forceinline tU32 _FloatIR(tF32 x) {
  return ((ni::tU32&)(x));
}
__forceinline tI32 _FloatSIR(tF32 x) {
  return ((ni::tI32&)(x));
}
__forceinline tF32 _FloatFR(tU32 x) {
  return ((ni::tF32&)(x));
}

//! Integer representation of a floating-point value.
#define niFloatIR(x)          (ni::_FloatIR(x))

//! Signed integer representation of a floating-point value.
#define niFloatSIR(x)         (ni::_FloatSIR(x))

//! Absolute integer representation of a floating-point value
#define niFloatAIR(x)         (niFloatIR(x)&0x7fffffff)

//! Floating-point representation of an integer value.
#define niFloatFR(x)          (ni::_FloatFR(x))

//! Integer-based comparison of a floating point value.
//! Don't use it blindly, it can be faster or slower than the FPU comparison, depends on the context.
#define niFloatIsNegative(x)  (niFloatIR(x)&0x80000000)

//! Check if float is zero
#define niFloatIsZero(x)      (niFloatIR(x)==0)

//! Check whether the specified number if not a valid floating point number (NaN).
#ifdef niPosix
#define niFloatIsNaN(x) isnan(x)
#else
#define niFloatIsNaN(x) _isnan(x)
#endif

#ifdef __cplusplus
template<typename T> inline T TypeNaN() { ; /*error*/ }
template<typename T> inline T TypeInfinity() { ; /*error*/ }

template<> inline tF32 TypeNaN<tF32>() {
  return eastl::numeric_limits<tF32>::quiet_NaN();
}
template<> inline tF64 TypeNaN<tF64>() {
  return eastl::numeric_limits<tF64>::quiet_NaN();
}

template<> inline tF32 TypeInfinity<tF32>() {
  return eastl::numeric_limits<tF32>::infinity();
}
template<> inline tF64 TypeInfinity<tF64>() {
  return eastl::numeric_limits<tF64>::infinity();
}

__forceinline bool IsNaN(const tF64 num) {
  return num != num;
}
__forceinline bool IsNaN(const tF32 num) {
  return num != num;
}

__forceinline bool IsInfinity(const tF32 num) {
  return IsNaN(num - num);
}
__forceinline bool IsInfinity(const tF64 num) {
  return IsNaN(num - num);
}

__forceinline bool IsFinite(const tF32 d) {
#ifdef niMSVC
  return _finite(d);
#elif defined niIOS \
  || defined niJSCC \
  || defined niQNX \
  || (defined niOSX && defined niARM64)
  return ((d) >= -FLT_MAX && (d) <= FLT_MAX);
#else
  return finite(d);
#endif
}
__forceinline bool IsFinite(const tF64 d) {
#ifdef niMSVC
  return _finite(d);
#elif defined niIOS \
  || defined niJSCC \
  || defined niQNX \
  || (defined niOSX && defined niARM64)
  return ((d) >= -DBL_MAX && (d) <= DBL_MAX);
#else
  return finite(d);
#endif
}

__forceinline tF32 InfiniteToZero(tF32 x) {
  return IsFinite(x) ? x : 0.0;
}
__forceinline tF64 InfiniteToZero(tF64 x) {
  return IsFinite(x) ? x : 0.0;
}
#endif

//--------------------------------------------------------------------------------------------
//
//  MSVC x86-32 optimized version
//
//--------------------------------------------------------------------------------------------
#if defined niMSVC && defined niIntelX86 && defined ni32

__forceinline unsigned char FloatNormToByte(float a)
{
  const float mul_val = 255.0f;
  unsigned int r;
  __asm {
    fld   a;
    fmul  mul_val;
    fistp r;
  }
  return (unsigned char)r;
}

//! Fast fabs for floating-point values. It just clears the sign bit.
//! Don't use it blindy, it can be faster or slower than the FPU comparison, depends on the context.
__forceinline tF32 FloatAbs(tF32 x)
{
  tU32 FloatBits = niFloatIR(x)&0x7fffffff;
  return niFloatFR(FloatBits);
}

//! Fast square root for floating-point values.
__forceinline tF32 FloatSqrt(tF32 square)
{
  float retval;

  __asm {
    mov             eax, square;
    sub             eax, 0x3F800000;
    sar             eax, 1;
    add             eax, 0x3F800000;
    mov             [retval], eax;
  }
  return retval;
}

//! Convert a float to the nearest integer.
//! \remark In general you should prefer this method to a standard (int) cast.
//! \remark Reduce the integer range to -2^30 to 2^30-1
__forceinline int FloatToIntNearest(double x)
{
  //  niAssert(x > static_cast <double> (niMinI32 / 2) – 1.0);
  //  niAssert(x < static_cast <double> (niMaxI32 / 2) + 1.0);
  const float round_to_nearest = 0.5f;
  int i;
  __asm {
    fld x;
    fadd st, st (0);
    fadd round_to_nearest;
    fistp i;
    sar i, 1;
  }
  return (i);
}

//! Convert a float to the nearest integer keeping the full integer range.
//! \remark The same trick (using 64bit temp) could be used for the other modes.
__forceinline int FloatToIntNearestX(double x)
{
  //  niAssert(x > static_cast <double> (niMinI32 / 2) – 1.0);
  //  niAssert(x < static_cast <double> (niMaxI32 / 2) + 1.0);
  const float round_to_nearest = 0.5f;
  int i;
  __int64 tmp;
  __asm {
    fld x;
    fadd st, st (0);
    fadd round_to_nearest;
    fistp qword ptr tmp;
    mov eax, dword ptr tmp;
    sar dword ptr tmp + 4, 1;
    rcr eax, 1;
    mov i, eax;
  }
  return (i);
}

//! Convert a float to the floor value of x.
//! \remark reduce the integer range to -2^30 to 2^30-1
__forceinline int FloatToIntFloor(double x)
{
  //  niAssert(x > static_cast <double> (niMinI32 / 2) – 1.0);
  //  niAssert(x < static_cast <double> (niMaxI32 / 2) + 1.0);
  const float round_towards_m_i = -0.5f;
  int i;
  __asm {
    fld x;
    fadd st, st (0);
    fadd round_towards_m_i;
    fistp i;
    sar i, 1;
  }
  return (i);
}

//! Convert a float to the ceil value of x.
//! \remark reduce the integer range to -2^30 to 2^30-1
__forceinline int FloatToIntCeil(double x)
{
  //  niAssert(x > static_cast <double> (niMinI32 / 2) – 1.0);
  //  niAssert(x < static_cast <double> (niMaxI32 / 2) + 1.0);
  const float round_towards_p_i = -0.5f;
  int i;
  __asm {
    fld x;
    fadd st, st (0);
    fsubr round_towards_p_i;
    fistp i;
    sar i, 1;
  }
  return (-i);
}

//! Fast sinus and cosinus computation.
__forceinline void FloatSinCos(const tF32 x, tF32& sine, tF32& cosine)
{
  __asm
  {
    fld x;
    fsincos;
    mov eax,[cosine];
    fstp dword ptr [eax];
    mov eax,[sine];
    fstp dword ptr [eax];
  }
}

//--------------------------------------------------------------------------------------------
//
//  Generic C version
//
//--------------------------------------------------------------------------------------------
#else

inline unsigned char FloatNormToByte(float a)
{
  return (unsigned char)(a*255.0f);
}

inline tF32 FloatAbs(tF32 x)
{
  return ni::Abs(x);
}

inline tF32 FloatSqrt(tF32 square)
{
  return ni::Sqrt(square);
}

inline int FloatToIntNearest(double x)
{
  return (int)ni::Floor(x+0.5);
}

inline int FloatToIntNearestX(double x)
{
  return (int)ni::Floor(x+0.5);
}

inline int FloatToIntFloor(double x)
{
  return (int)ni::Floor(x);
}

inline int FloatToIntCeil(double x)
{
  return (int)ni::Ceil(x);
}

inline void FloatSinCos(const tF32 x, tF32& sine, tF32& cosine) {
  sine = ni::Sin(x);
  cosine = ni::Cos(x);
}

#endif

//===========================================================================
//
//  Double
//
//===========================================================================

/*
 * It's reasonable to ask why we have this header at all.  Don't isnan,
 * copysign, the built-in comparison operators, and the like solve these
 * problems?  Unfortunately, they don't.  We've found that various compilers
 * (MSVC, MSVC when compiling with PGO, and GCC on OS X, at least) miscompile
 * the standard methods in various situations, so we can't use them.  Some of
 * these compilers even have problems compiling seemingly reasonable bitwise
 * algorithms!  But with some care we've found algorithms that seem to not
 * trigger those compiler bugs.
 *
 * For the aforementioned reasons, be very wary of making changes to any of
 * these algorithms.  If you must make changes, keep a careful eye out for
 * compiler bustage, particularly PGO-specific bustage.
 *
 * Some users require that this file be C-compatible.  Unfortunately, this means
 * no mozilla namespace to contain everything, no detail namespace clarifying
 * NiDoublePun to be an internal data structure, and so on.
 */

/*
 * These implementations all assume |tF64| is a 64-bit double format number
 * type, compatible with the IEEE-754 standard.  C/C++ don't require this to be
 * the case.  But we required this in implementations of these algorithms that
 * preceded this header, so we shouldn't break anything if we continue doing so.
 */
// tF64 must be 64 bits
niCAssert(sizeof(tF64) == sizeof(tU64));

/*
 * Constant expressions in C can't refer to consts, unfortunately, so #define
 * these rather than use |const tU64|.
 */
#define NI_DOUBLE_SIGN_BIT          0x8000000000000000ULL
#define NI_DOUBLE_EXPONENT_BITS     0x7ff0000000000000ULL
#define NI_DOUBLE_SIGNIFICAND_BITS  0x000fffffffffffffULL

#define NI_DOUBLE_EXPONENT_BIAS   1023
#define NI_DOUBLE_EXPONENT_SHIFT  52

// sign bit doesn't overlap exponent bits
niCAssert((NI_DOUBLE_SIGN_BIT & NI_DOUBLE_EXPONENT_BITS) == 0);
// sign bit doesn't overlap significand bits
niCAssert((NI_DOUBLE_SIGN_BIT & NI_DOUBLE_SIGNIFICAND_BITS) == 0);
// exponent bits don't overlap significand bits
niCAssert((NI_DOUBLE_EXPONENT_BITS & NI_DOUBLE_SIGNIFICAND_BITS) == 0);
// all bits accounted for
niCAssert((NI_DOUBLE_SIGN_BIT | NI_DOUBLE_EXPONENT_BITS | NI_DOUBLE_SIGNIFICAND_BITS) == ~(tU64)0);

/*
 * This union is NOT a public data structure, and it is not to be used outside
 * this file!
 */
union NiDoublePun {
  /*
   * Every way to pun the bits of a tF64 introduces an additional layer of
   * complexity, across a multitude of platforms, architectures, and ABIs.
   * Use *only* tU64 to reduce complexity.  Don't add new punning here
   * without discussion!
   */
  tU64 u;
  tF64 d;
};

/** Determines whether a tF64 is NaN. */
static __forceinline int
DoubleIsNaN(tF64 d)
{
  union NiDoublePun pun;
  pun.d = d;

  /*
   * A tF64 is NaN if all exponent bits are 1 and the significand contains at
   * least one non-zero bit.
   */
  return (pun.u & NI_DOUBLE_EXPONENT_BITS) == NI_DOUBLE_EXPONENT_BITS &&
      (pun.u & NI_DOUBLE_SIGNIFICAND_BITS) != 0;
}

/** Determines whether a tF64 is +Infinity or -Infinity. */
static __forceinline int
DoubleIsInfinite(tF64 d)
{
  union NiDoublePun pun;
  pun.d = d;

  /* Infinities have all exponent bits set to 1 and an all-0 significand. */
  return (pun.u & ~NI_DOUBLE_SIGN_BIT) == NI_DOUBLE_EXPONENT_BITS;
}

/** Determines whether a tF64 is not NaN or infinite. */
static __forceinline int
DoubleIsFinite(tF64 d)
{
  union NiDoublePun pun;
  pun.d = d;

  /*
   * NaN and Infinities are the only non-finite tF64s, and both have all
   * exponent bits set to 1.
   */
  return (pun.u & NI_DOUBLE_EXPONENT_BITS) != NI_DOUBLE_EXPONENT_BITS;
}

/**
 * Determines whether a tF64 is negative.  It is an error to call this method
 * on a tF64 which is NaN.
 */
static __forceinline int
DoubleIsNegative(tF64 d)
{
  union NiDoublePun pun;
  pun.d = d;

  niAssertMsg(!DoubleIsNaN(d), "NaN does not have a sign");

  /* The sign bit is set if the tF64 is negative. */
  return (pun.u & NI_DOUBLE_SIGN_BIT) != 0;
}

/** Determines whether a tF64 represents -0. */
static __forceinline int
DoubleIsNegativeZero(tF64 d)
{
  union NiDoublePun pun;
  pun.d = d;

  /* Only the sign bit is set if the tF64 is -0. */
  return pun.u == NI_DOUBLE_SIGN_BIT;
}

/** Returns the exponent portion of the tF64. */
static __forceinline int DoubleGetExponent(tF64 d)
{
  union NiDoublePun pun;
  pun.d = d;

  /*
   * The exponent component of a tF64 is an unsigned number, biased from its
   * actual value.  Subtract the bias to retrieve the actual exponent.
   */
  return (int)((pun.u & NI_DOUBLE_EXPONENT_BITS) >> NI_DOUBLE_EXPONENT_SHIFT) -
      NI_DOUBLE_EXPONENT_BIAS;
}

/** Returns +Infinity. */
static __forceinline tF64
DoublePositiveInfinity()
{
  union NiDoublePun pun;

  /*
   * Positive infinity has all exponent bits set, sign bit set to 0, and no
   * significand.
   */
  pun.u = NI_DOUBLE_EXPONENT_BITS;
  return pun.d;
}

/** Returns -Infinity. */
static __forceinline tF64
DoubleNegativeInfinity()
{
  union NiDoublePun pun;

  /*
   * Negative infinity has all exponent bits set, sign bit set to 1, and no
   * significand.
   */
  pun.u = NI_DOUBLE_SIGN_BIT | NI_DOUBLE_EXPONENT_BITS;
  return pun.d;
}

/** Constructs a NaN value with the specified sign bit and significand bits. */
static __forceinline tF64
DoubleGetSpecificNaN(int signbit, tU64 significand)
{
  union NiDoublePun pun;

  niAssert(signbit == 0 || signbit == 1);
  niAssert((significand & ~NI_DOUBLE_SIGNIFICAND_BITS) == 0);
  niAssert(significand & NI_DOUBLE_SIGNIFICAND_BITS);

  pun.u = (signbit ? NI_DOUBLE_SIGN_BIT : 0) |
      NI_DOUBLE_EXPONENT_BITS |
      significand;
  niAssert(DoubleIsNaN(pun.d));
  return pun.d;
}

/**
 * Computes a NaN value.  Do not use this method if you depend upon a particular
 * NaN value being returned.
 */
static __forceinline tF64
DoubleNaN()
{
  return DoubleGetSpecificNaN(0, 0xfffffffffffffULL);
}

static __forceinline tF64 DoubleMaxValue() {
  return 1.7976931348623157E+308;
}

/** Computes the smallest non-zero positive tF64 value. */
static __forceinline tF64 DoubleMinValue()
{
  union NiDoublePun pun;
  pun.u = 1;
  return pun.d;
}

static __forceinline int DoubleIsInt32(tF64 d, tI32* i)
{
  /*
   * XXX Casting a tF64 that doesn't truncate to int32_t, to int32_t, induces
   *     undefined behavior.  We should definitely fix this (bug 744965), but as
   *     apparently it "works" in practice, it's not a pressing concern now.
   */
  return !DoubleIsNegativeZero(d) && d == (*i = (tI32)d);
}

//===========================================================================
//
//  String conversions
//
//===========================================================================
#if niMinFeatures(20)

// Minimum buffer size for any type of double to string conversion.
const tU32 kDoubleToStringBufferSize = 2048;

enum eDoubleToStringMode {
  eDoubleToStringMode_ShortestDouble = 0,
  eDoubleToStringMode_ShortestSingle = 1,
  eDoubleToStringMode_Fixed = 2,
  eDoubleToStringMode_Exponential = 3,
  eDoubleToStringMode_Precision = 4
};

niExportFunc(char*) DoubleToString(char* aBuffer, tSize aBufferSize,
                                   tF64 aNumber, eDoubleToStringMode aMode,
                                   tI32 aDigitsOrPrecision = 15,
                                   const char* aInfinitySymbol = "Infinity",
                                   const char* aNaNSymbol = "NaN",
                                   char aExponentCharacter = 'e');

niExportFunc(char*) DoubleToStringRadix(
    char* aBuffer, tSize aBufferSize,
    tF64 aNumber, tI32 aRadix);

niExportFunc(char*) DoubleToStringEcma(char* aBuffer, tSize aBufferSize,
                                       tF64 aNumber, tI32 aRadix);

/**
 * This purges trailing zeros from the right, i.e.
 * "144124.45600" -> "144124.456"
 * "144124.000" -> "144124"
 * "144124.45600e+12" -> "144124.456e+12"
 * "144124.45600e+07" -> 144124.456e+7"
 */
niExportFunc(char*) DoubleStringPurgeTrailingZeros(char* buf);

enum eStringToDoubleFlags {
  eStringToDoubleFlags_None = 0,
  eStringToDoubleFlags_Hex = 1,
  eStringToDoubleFlags_Octals = 2,
  eStringToDoubleFlags_TrailingJunk = 4,
  eStringToDoubleFlags_LeadingSpaces = 8,
  eStringToDoubleFlags_TrailingSpaces = 16,
  eStringToDoubleFlags_SpacesAfterSign = 32,
  eStringToDoubleFlags_TrailingEPlusMinus = 64,
  eStringToDoubleFlags_Default = eStringToDoubleFlags_Hex|eStringToDoubleFlags_TrailingJunk|eStringToDoubleFlags_TrailingEPlusMinus|eStringToDoubleFlags_LeadingSpaces|eStringToDoubleFlags_TrailingSpaces|eStringToDoubleFlags_SpacesAfterSign
};

//! \see ni::eStringToDoubleFlags
typedef tU32 tStringToDoubleFlags;

niExportFunc(tF64) StringToDouble(const char* aBuffer,
                                  tSize aBufferSize = 0,
                                  tInt* apProcessedCharactersCount = NULL,
                                  tStringToDoubleFlags aFlags = eStringToDoubleFlags_Default,
                                  tF64 aEmptyStringValue = 0,
                                  tF64 aInvalidStringValue = DoubleNaN(),
                                  const char* aInfinitySymbol = "Infinity",
                                  const char* aNaNSymbol = "NaN");

niExportFunc(tF32) StringToFloat(const char* aBuffer,
                                 tSize aBufferSize = 0,
                                 tInt* apProcessedCharactersCount = NULL,
                                 tStringToDoubleFlags aFlags = eStringToDoubleFlags_Default,
                                 tF64 aEmptyStringValue = 0,
                                 tF64 aInvalidStringValue = DoubleNaN(),
                                 const char* aInfinitySymbol = "Infinity",
                                 const char* aNaNSymbol = "NaN");
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __MATHFLOAT_44024344_H__
