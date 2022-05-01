// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/Float16.h"

// using namespace ni;

// Primary authors:
//     Florian Kainz <kainz@ilm.com>
//     Rod Bogart <rgb@ilm.com>

//---------------------------------------------------------------------------
//
//   cFloat16 -- a 16-bit floating point number class:
//
//   Type cFloat16 can represent positive and negative numbers, whose
//   magnitude is between roughly 6.1e-5 and 6.5e+4, with a relative
//   error of 9.8e-4; numbers smaller than 6.1e-5 can be represented
//   with an absolute error of 6.0e-8.  All integers from -2048 to
//   +2048 can be represented exactly.
//
//   Type cFloat16 behaves (almost) like the built-in C++ floating point
//   types.  In arithmetic expressions, cFloat16, tF32 and double can be
//   mixed freely.  Here are a few examples:
//
//       cFloat16 a (3.5);
//       tF32 b (a + sqrt (a));
//       a += b;
//       b += a;
//       b = a + 7;
//
//   Conversions from cFloat16 to tF32 are lossless; all cFloat16 numbers
//   are exactly representable as floats.
//
//   Conversions from tF32 to cFloat16 may not preserve the tF32's
//   value exactly.  If a tF32 is not representable as a cFloat16, the
//   tF32 value is rounded to the nearest representable cFloat16.  If
//   a tF32 value is exactly in the middle between the two closest
//   representable cFloat16 values, then the tF32 value is rounded to
//   the cFloat16 with the greater magnitude.
//
//   Overflows during tF32-to-cFloat16 conversions cause arithmetic
//   exceptions.  An overflow occurs when the tF32 value to be
//   converted is too large to be represented as a cFloat16, or if the
//   tF32 value is an infinity or a NAN.
//
//   The implementation of type cFloat16 makes the following assumptions
//   about the implementation of the built-in C++ types:
//
//       tF32 is an IEEE 754 single-precision number
//       sizeof (tF32) == 4
//       sizeof (tU32) == sizeof (tF32)
//       alignof (tU32) == alignof (tF32)
//       sizeof (tU16) == 2
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//
// Implementation --
//
// Representation of a tF32:
//
//   We assume that a tF32, f, is an IEEE 754 single-precision
//   floating point number, whose bits are arranged as follows:
//
//       31 (msb)
//       |
//       | 30     23
//       | |      |
//       | |      | 22                    0 (lsb)
//       | |      | |                     |
//       X XXXXXXXX XXXXXXXXXXXXXXXXXXXXXXX
//
//       s e        m
//
//   S is the sign-bit, e is the exponent and m is the significand.
//
//   If e is between 1 and 254, f is a normalized number:
//
//               s    e-127
//       f = (-1)  * 2      * 1.m
//
//   If e is 0, and m is not zero, f is a denormalized number:
//
//               s    -126
//       f = (-1)  * 2      * 0.m
//
//   If e and m are both zero, f is zero:
//
//       f = 0.0
//
//   If e is 255, f is an "infinity" or "not a number" (NAN),
//   depending on whether m is zero or not.
//
//   Examples:
//
//       0 00000000 00000000000000000000000 = 0.0
//       0 01111110 00000000000000000000000 = 0.5
//       0 01111111 00000000000000000000000 = 1.0
//       0 10000000 00000000000000000000000 = 2.0
//       0 10000000 10000000000000000000000 = 3.0
//       1 10000101 11110000010000000000000 = -124.0625
//       0 11111111 00000000000000000000000 = +infinity
//       1 11111111 00000000000000000000000 = -infinity
//       0 11111111 10000000000000000000000 = NAN
//       1 11111111 11111111111111111111111 = NAN
//
// Representation of a cFloat16:
//
//   Here is the bit-layout for a cFloat16 number, h:
//
//       15 (msb)
//       |
//       | 14  10
//       | |   |
//       | |   | 9        0 (lsb)
//       | |   | |        |
//       X XXXXX XXXXXXXXXX
//
//       s e     m
//
//   S is the sign-bit, e is the exponent and m is the significand.
//
//   If e is between 1 and 30, h is a normalized number:
//
//               s    e-15
//       h = (-1)  * 2     * 1.m
//
//   If e is 0, and m is not zero, h is a denormalized number:
//
//               S    -14
//       h = (-1)  * 2     * 0.m
//
//   If e and m are both zero, h is zero:
//
//       h = 0.0
//
//   If e is 31, h is an "infinity" or "not a number" (NAN),
//   depending on whether m is zero or not.
//
//   Examples:
//
//       0 00000 0000000000 = 0.0
//       0 01110 0000000000 = 0.5
//       0 01111 0000000000 = 1.0
//       0 10000 0000000000 = 2.0
//       0 10000 1000000000 = 3.0
//       1 10101 1111000001 = -124.0625
//       0 11111 0000000000 = +infinity
//       1 11111 0000000000 = -infinity
//       0 11111 1000000000 = NAN
//       1 11111 1111111111 = NAN
//
// Conversion:
//
//   Converting from a tF32 to a cFloat16 requires some non-trivial bit
//   manipulations.  In some cases, this makes conversion relatively
//   slow, but the most common case is accelerated via table lookups.
//
//   Converting back from a cFloat16 to a tF32 is easier because we don't
//   have to do any rounding.  In addition, there are only 65536
//   different cFloat16 numbers; we can convert each of those numbers once
//   and store the results in a table.  Later, all conversions can be
//   done using only simple table lookups.
//
//---------------------------------------------------------------------------

union uF32
{
  ni::tU32  i;
  ni::tF32  f;
};

//---------------------------------------------------
// Interpret an unsigned short bit pattern as a half,
// and convert that half to the corresponding float's
// bit pattern.
//---------------------------------------------------
static ni::tU32 halfToFloat(ni::tU16 y)
{
  int s = (y >> 15) & 0x00000001;
  int e = (y >> 10) & 0x0000001f;
  int m =  y        & 0x000003ff;

  if (e == 0)
  {
    if (m == 0)
    {
      // Plus or minus zero
      return s << 31;
    }
    else
    {
      // Denormalized number -- renormalize it
      while (!(m & 0x00000400))
      {
        m <<= 1;
        e -=  1;
      }
      e += 1;
      m &= ~0x00000400;
    }
  }
  else if (e == 31)
  {
    if (m == 0)
    {
      // Positive or negative infinity
      return (s << 31) | 0x7f800000;
    }
    else
    {
      // Nan -- preserve sign and significand bits
      return (s << 31) | 0x7f800000 | (m << 13);
    }
  }

  // Normalized number
  e = e + (127 - 15);
  m = m << 13;

  // Assemble s, e and m.
  return (s << 31) | (e << 23) | m;
}

//-----------------------------------------------------
// Compute a lookup table for float-to-half conversion.
//
// When indexed with the combined sign and exponent of
// a float, the table either returns the combined sign
// and exponent of the corresponding half, or zero if
// the corresponding half may not be normalized (zero,
// denormalized, overflow).
//-----------------------------------------------------
static void initELut (unsigned short eLut[])
{
  for (int i = 0; i < 0x100; i++)
  {
    int e = (i & 0x0ff) - (127 - 15);

    if (e <= 0 || e >= 30)
    {
      // Special case
      eLut[i]         = 0;
      eLut[i | 0x100] = 0;
    }
    else
    {
      // Common case - normalized half, no exponent overflow possible
      eLut[i]         =  (e << 10);
      eLut[i | 0x100] = ((e << 10) | 0x8000);
    }
  }
}

static uF32* _table_F16toF32 = NULL;
static ni::tU16* _table_F32toF16 = NULL;

static ni::tBool ni_f16_init_tables()
{
  if (_table_F16toF32) return ni::eFalse;
  // F16 -> F32 lookup table
  _table_F16toF32 = (uF32*)niMalloc(sizeof(uF32)*(1<<16));
  for (int i = 0; i < 0xFFFF; i++)
    _table_F16toF32[i].i = halfToFloat(i);
  // F32 -> F16 lookup table
  _table_F32toF16 = (ni::tU16*)niMalloc(sizeof(ni::tU16)*(1<<9));
  initELut(_table_F32toF16);
  return ni::eTrue;
}
#if 0 // not used...
static void ni_f16_free_tables() {
  // not used now, but could be later on... if we really
  // need to release the lookup tables...
  if (_table_F16toF32) {
    niFree(_table_F16toF32);
    _table_F16toF32 = NULL;
  }
  if (_table_F32toF16) {
    niFree(_table_F16toF32);
    _table_F16toF32 = NULL;
  }
}
#endif

///////////////////////////////////////////////
static inline ni::tI16 convert(ni::tI32 i)
{
  // Our floating point number, f, is represented by the bit
  // pattern in integer i.  Disassemble that bit pattern into
  // the sign, s, the exponent, e, and the significand, m.
  // Shift s into the position where it will go in in the
  // resulting half number.
  // Adjust e, accounting for the different exponent bias
  // of float and half (127 versus 15).
  int s =  (i >> 16) & 0x00008000;
  int e = ((i >> 23) & 0x000000ff) - (127 - 15);
  int m =   i        & 0x007fffff;

  // Now reassemble s, e and m into a half:
  if (e <= 0)
  {
    if (e < -10)
    {
      // E is less than -10.  The absolute value of f is
      // less than HALF_MIN (f may be a small normalized
      // float, a denormalized float or a zero).
      //
      // We convert f to a half zero.
      return 0;
    }

    // E is between -10 and 0.  F is a normalized float,
    // whose magnitude is less than HALF_NRM_MIN.
    //
    // We convert f to a denormalized half.
    m = (m | 0x00800000) >> (1 - e);

    // Round to nearest, round "0.5" up.
    //
    // Rounding may cause the significand to overflow and make
    // our number normalized.  Because of the way a half's bits
    // are laid out, we don't have to treat this case separately;
    // the code below will handle it correctly.
    if (m &  0x00001000)
      m += 0x00002000;

    // Assemble the half from s, e (zero) and m.
    return s | (m >> 13);
  }
  else if (e == 0xff - (127 - 15))
  {
    if (m == 0)
    {
      // F is an infinity; convert f to a half
      // infinity with the same sign as f.
      return s | 0x7c00;
    }
    else
    {
      // F is a NAN; produce a half NAN that preserves
      // the sign bit and the 10 leftmost bits of the
      // significand of f.
      return s | 0x7c00 | (m >> 13);
    }
  }
  else
  {
    // E is greater than zero.  F is a normalized float.
    // We try to convert f to a normalized half.
    //
    // Round to nearest, round "0.5" up
    if (m &  0x00001000)
    {
      m += 0x00002000;
      if (m & 0x00800000)
      {
        m =  0;      // overflow in significand,
        e += 1;      // adjust exponent
      }
    }

    // Handle exponent overflow
    if (e > 30)
    {
      return s|0x7c00;   // if this returns, the half becomes an infinity with the same sign as f.
    }

    // Assemble the half from s, e and m.
    return s | (e << 10) | (m >> 13);
  }
}

niExportFunc(ni::tU16) ni_f32_to_f16(ni::tF32 f)
{
  ni_f16_init_tables();
  ni::tU16 v = 0;
  if (f == 0)
  {
    // Common special case - zero.
    // For speed, we don't preserve the zero's sign.
    v = 0;
  }
  else
  {
    // We extract the combined sign and exponent, e, from our
    // floating-point number, f.  Then we convert e to the sign
    // and exponent of the cFloat16 number via a table lookup.
    //
    // For the most common case, where a normalized cFloat16 is produced,
    // the table lookup returns a non-zero value; in this case, all
    // we have to do, is round f's significand to 10 bits and combine
    // the result with e.
    //
    // For all other cases (overflow, zeroes, denormalized numbers
    // resulting from underflow, infinities and NANs), the table
    // lookup returns zero, and we call a longer, non-inline function
    // to do the F32-to-F16 conversion.
    uF32 x;
    x.f = f;
    int e = (x.i >> 23) & 0x000001ff;
    e = _table_F32toF16[e];

    if (e)
    {
      // Simple case - round the significand and
      // combine it with the sign and exponent.
      v = (ni::tU16)(e + (((x.i & 0x007fffff) + 0x00001000) >> 13));
    }
    else
    {
      // Difficult case - call a function.
      v = convert (x.i);
    }
  }

  return v;
}

niExportFunc(ni::tF32) ni_f16_to_f32(ni::tU16 v) {
  ni_f16_init_tables();
  return _table_F16toF32[v].f;
}
