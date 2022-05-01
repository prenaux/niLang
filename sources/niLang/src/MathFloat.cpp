// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Math/MathFloat.h"

#if niMinFeatures(20)

#include "API/niLang/StringLib.h"
#include "API/niLang/Utils/StringIntToStr.h"
#include "double-conversion/double-conversion.h"
#include "API/niLang/STL/EASTL/algorithm.h"

namespace ni {

// The buffer for a 32-bit integer, base 10, needs to be >= 12: -2147483647 or 4294967294
const tU32 kDoubleToStringInt32Base10ToString = 12;
// The buffer for a 64-bit integer needs to be >= 65: sign, up to 63 (64 w/o sign) bits if base =-=2, NUL
const tU32 kDoubleToStringInt64ToString = 65;
// For security reason, the buffer must be well over 347 characters:
// Bug 192033: Number.MAX_VALUE is 1.79e+308, so this was 312.
// Bug 230183: That wasn't big enough.  Number.MIN_VALUE.toPrecision(21)
// needs 347.  But why be stingy?  There may be other cases that are
// even bigger, it's hard to say.
const tU32  kDoubleToStringBase10BufferSize = 380;
// Buffer size for converting IEEE-754 double to string using worst-case
// radix(2) plus a leading '-' char. Denormalized (i.e., very small) numbers
// are truncated to "0" so they don't require additional space.
const tU32 kDoubleToStringBase2BufferSize = 1025;

niExportFunc(char*) DoubleStringPurgeTrailingZeros(char* buf)
{
  int i = ni::StrSize(buf)-1;
  int Epos = 0;
  int Elen = 0;
  for (int findE = 2; findE <= 4; ++findE) {
    const int a = findE;
    const int b = findE+1;
    if (i > b && buf[i-a] == 'e') {
      Epos = i-a;
      Elen = b;
      i = i-b;
      break;
    }
  }
  for ( ; i > 0; --i) {
    if (buf[i] != '0')
      break;
  }

  if (Epos) {
    // copy e+12 to the current place
    ni::StrNCpy(buf+i+1, buf+Epos, Elen+1);
    if (buf[i+(Elen-2)] == '0') {
      // this looks like e+07, so turn it into e+7
      buf[i+(Elen-2)] = buf[i+(Elen-1)];
      buf[i+(Elen-1)] = '\0';
    }
  }
  else {
    buf[i+1]='\0';
  }

  return buf;
}

niExportFunc(char*) DoubleToString(char* aBuffer, tSize aBufferSize, tF64 aNumber,
                                   eDoubleToStringMode aMode, tI32 aDigitsOrPrecision,
                                   const char* aInfinitySymbol,
                                   const char* aNaNSymbol,
                                   char aExponentCharacter)
{
  niAssert(aBuffer != NULL);
  niAssert(aBufferSize >= kDoubleToStringBufferSize);
  if (!aBuffer || aBufferSize == 0)
    return aBuffer;

  tI32 numberAsInt;
  if ((aMode == eDoubleToStringMode_ShortestDouble) &&
      DoubleIsInt32(aNumber,&numberAsInt))
  {
    return StringIntToStr(aBuffer,aBufferSize,numberAsInt);
  }

  /*
   * This is V8's implementation of the algorithm described in the
   * following paper:
   *
   *   Printing floating-point numbers quickly and accurately with integers.
   *   Florian Loitsch, PLDI 2010.
   */
  static double_conversion::DoubleToStringConverter
      converter(double_conversion::DoubleToStringConverter::UNIQUE_ZERO |
                double_conversion::DoubleToStringConverter::EMIT_POSITIVE_EXPONENT_SIGN,
                aInfinitySymbol,
                aNaNSymbol,
                aExponentCharacter,
                -5, 21,
                6, 0);
  double_conversion::StringBuilder builder(aBuffer,aBufferSize);
  switch (aMode) {
    default:
    case eDoubleToStringMode_ShortestDouble:
      converter.ToShortestSingle(aNumber, &builder);
      break;
    case eDoubleToStringMode_ShortestSingle:
      converter.ToShortest(aNumber, &builder);
      break;
    case eDoubleToStringMode_Fixed:
      converter.ToFixed(aNumber, aDigitsOrPrecision, &builder);
      break;
    case eDoubleToStringMode_Exponential:
      converter.ToExponential(aNumber, aDigitsOrPrecision, &builder);
      break;
    case eDoubleToStringMode_Precision:
      converter.ToPrecision(aNumber, aDigitsOrPrecision, &builder);
      break;
  }

  return builder.Finalize();
}

niExportFunc(char*) DoubleToStringRadix(
    char* aBuffer, tSize aBufferSize,
    tF64 aNumber, tI32 aRadix)
{
  static char digits[] ="0123456789abcdefghijklmnopqrstuvwxyz";

  niAssert(aBuffer != NULL);
  niAssert(aBufferSize >= kDoubleToStringBufferSize);
  niAssert(aRadix >= 2 && aRadix <= 36);
  if (!aBuffer || aBufferSize == 0 || (aRadix < 2))
    return aBuffer;

  tF64 v = aNumber;
  const tF64 r = (tF64)aRadix;
  bool negative = v<0;
  if (negative)
    v = -v;

  tSize n = 0;
  do {
    if (n >= (aBufferSize-1)) {
      break;
    }
    aBuffer[n++] = digits[(int)(v-(floor(v/r)*r))];
    v = v/r;
  } while (v >= 1.0);

  if ((n < (aBufferSize-1)) && negative) {
    aBuffer[n++] = '-';
  }

  eastl::reverse(aBuffer,aBuffer+n);

  niAssert(n < aBufferSize);

  aBuffer[n] = 0;
  return aBuffer;
}

niExportFunc(char*) DoubleToStringEcma(char* aBuffer, tSize aBufferSize,
                                       tF64 aNumber, tI32 aRadix)
{
  niAssert(aBuffer != NULL);
  niAssert(aBufferSize >= kDoubleToStringBufferSize);
  if (!aBuffer || aBufferSize == 0)
    return aBuffer;

  if (ni::IsNaN(aNumber)) {
    ni::StrCpy(aBuffer,"NaN");
    return aBuffer;
  }
  else if (ni::IsInfinity(aNumber)) {
    if (aNumber < 0) {
      ni::StrCpy(aBuffer,"-Infinity");
    }
    else {
      ni::StrCpy(aBuffer,"Infinity");
    }
    return aBuffer;
  }

  if (aRadix < 2 || aRadix > 36)
    aRadix = 10;

  if (aRadix != 10) {
    return DoubleToStringRadix(aBuffer,aBufferSize,aNumber,aRadix);
  }

  ni::tI32 valueAsInt;
  if (ni::DoubleIsInt32(aNumber,&valueAsInt)) {
    return ni::StringIntToStr(aBuffer,aBufferSize,valueAsInt);
  }

  char* ret;
  if (ni::Abs(aNumber) >= 1e+21 || ni::Abs(aNumber) <= 1e-5) {
    ret = (char*)ni::DoubleToString(aBuffer,aBufferSize,aNumber,
                                    ni::eDoubleToStringMode_Exponential,14);
  }
  else {
    ret = (char*)ni::DoubleToString(aBuffer,aBufferSize,aNumber,
                                    ni::eDoubleToStringMode_Precision,15);
  }

  return DoubleStringPurgeTrailingZeros(ret);
}

niExportFunc(tF64) StringToDouble(const char* aBuffer,
                                  tSize aBufferSize,
                                  tInt* apProcessedCharactersCount,
                                  tStringToDoubleFlags aFlags,
                                  tF64 aEmptyStringValue,
                                  tF64 aInvalidStringValue,
                                  const char* aInfinitySymbol,
                                  const char* aNaNSymbol)
{
  const double_conversion::StringToDoubleConverter
      converter(aFlags,aEmptyStringValue,aInvalidStringValue,aInfinitySymbol,aNaNSymbol);
  if (!aBufferSize || aBufferSize == eInvalidHandle)
    aBufferSize = ni::StrSize(aBuffer);
  tInt processedCharacterCountTmp;
  if (!apProcessedCharactersCount)
    apProcessedCharactersCount = &processedCharacterCountTmp;
  return converter.StringToDouble(aBuffer,aBufferSize,apProcessedCharactersCount);
}

niExportFunc(tF32) StringToFloat(const char* aBuffer,
                                 tSize aBufferSize,
                                 tInt* apProcessedCharactersCount,
                                 tStringToDoubleFlags aFlags,
                                 tF64 aEmptyStringValue,
                                 tF64 aInvalidStringValue,
                                 const char* aInfinitySymbol,
                                 const char* aNaNSymbol)
{
  const double_conversion::StringToDoubleConverter
      converter(aFlags,aEmptyStringValue,aInvalidStringValue,aInfinitySymbol,aNaNSymbol);
  if (!aBufferSize || aBufferSize == eInvalidHandle)
    aBufferSize = ni::StrSize(aBuffer);
  tInt processedCharacterCountTmp;
  if (!apProcessedCharactersCount)
    apProcessedCharactersCount = &processedCharacterCountTmp;
  return converter.StringToFloat(aBuffer,aBufferSize,apProcessedCharactersCount);
}

} // namespace ni

#endif
