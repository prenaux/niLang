#ifndef __RANDOM_15007279_H__
#define __RANDOM_15007279_H__
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

//! Generate a seed from the secure random entropy source.
//! \remark You can use RandSecureGetBytes(&someInt,sizeof(someInt)) to
//!         detect if a secure entropy source is available.
//! \remark Panic if they are no secure entropy source available.
niExportFunc(tU64) ni_prng_get_seed_from_secure_source();
//! Generate a seed from a weak time based entropy source.
//! \remark This is not a secure source of entropy.
niExportFunc(tU64) ni_prng_get_seed_from_time_source();
//! Generate a seed from a secure entropy source is possible otherwise fallback
//! to the time based source.
//! \remark This is not a secure source of entropy.
niExportFunc(tU64) ni_prng_get_seed_from_maybe_secure_source();

//! Initialize a new PRNG with the specified seed.
niExportFunc(int4) ni_prng_init(tU64 anSeed);
//! Seed a random number generator
niExportFunc(void) ni_prng_seed(int4* aPRNG, tU64 anSeed);
//! Get the next i32 prn of the specified PRNG.
//! \remark The general contract of |NextInt(int)| is that one int value in
//!         the specified range is pseudorandomly generated and returned. All
//!         max possible int values are produced with (approximately) equal
//!         probability.
niExportFunc(tI32) ni_prng_next_i32(int4* aPRNG);
//! Get the next i64 prn of the specified PRNG.
//! \remark The general contract of ni_prng_next_i64 is that one 64-bit int
//!         value is pseudorandomly generated and returned. All 2^64 possible
//!         integer values are produced with (approximately) equal
//!         probability.
niExportFunc(tI64) ni_prng_next_i64(int4* aPRNG);
//! Get the next f64 prn of the specified PRNG.
//! \remark The general contract of ni_prng_next_f64 is that one double value,
//!         chosen (approximately) uniformly from the range 0.0 (inclusive) to
//!         1.0 (exclusive), is pseudorandomly generated and returned.
niExportFunc(tF64) ni_prng_next_f64(int4* aPRNG);
//! Get the next bool prn of the specified PRNG.
//! \remark The general contract of ni_prng_next_bool is that one boolean
//!         value is pseudorandomly generated and returned. The values true
//!         and false are produced with (approximately) equal probability.
niExportFunc(tBool) ni_prng_next_bool(int4* aPRNG);
//! Get the next bytes prn of the specified PRNG.
niExportFunc(void) ni_prng_next_bytes(int4* aPRNG, tPtr apBytes, tSize anNumBytes);

//! Get the default/global prng.
niExportFunc(int4*) ni_prng_global();

//! Computes a 64bit seed from the specified string.
niExportFunc(tU64) ni_prng_seed_from_string(const achar* aString, const tI32 aStrLen = 0);

//! Get random bytes from a secure random entropy source.
//! \return eFalse if we can't get bytes from a secure random entropy source.
//! \remark Note that we rely on the OS/underlying platform to respect the
//!         definition of the APIs we're using.
niExportFunc(tBool) RandSecureGetBytes(tPtr apOutput, tSize anSize);

//! Seed a random number generator.
inline void RandSeed(tU64 aSeed, int4* aPRNG = ni_prng_global()) {
  ni_prng_seed(aPRNG,aSeed);
}

//! Return a random boolean
inline tBool RandBool(int4* aPRNG = ni_prng_global()) {
  return ni_prng_next_bool(aPRNG);
}

//! Return a random bytes
inline void RandBytes(tPtr apBytes, tSize anNumBytes, int4* aPRNG = ni_prng_global()) {
  ni_prng_next_bytes(aPRNG, apBytes, anNumBytes);
}

//! Return a random integer in the range [-TypeMin(tI32) ; TypeMax(tI32)]
inline tI32 RandInt(int4* aPRNG = ni_prng_global()) {
  return ni_prng_next_i32(aPRNG);
}

//! Return a random integer in the range [-TypeMin(tI64) ; TypeMax(tI64)]
inline tI64 RandInt64(int4* aPRNG = ni_prng_global()) {
  return ni_prng_next_i64(aPRNG);
}

//! Return a random integer in the range [ aMin ; aMax ].
inline tI32 RandIntRange(tI32 aMin, tI32 aMax, int4* aPRNG = ni_prng_global()) {
  const tI64 r = ni_prng_next_i32(aPRNG) + (tI64)TypeMax<tI32>();
  const tI64 q = (r % ((aMax-aMin)+1)) + aMin;
  return (tI32)q;
}

//! Return a random float in the range [-1 ; 1]
inline tF64 RandSignedFloat(int4* aPRNG = ni_prng_global()) {
  return (ni_prng_next_f64(aPRNG) * 2.0) - 1.0;
}

//! Return a random float in the range [0 ; 1]
inline tF64 RandFloat(int4* aPRNG = ni_prng_global()) {
  return ni_prng_next_f64(aPRNG);
}

//! Return a random float in the specified range.
inline tF64 RandFloatRange(tF64 aMin, tF64 aMax, int4* aPRNG = ni_prng_global()) {
  const tF64 size = aMax-aMin;
  return (ni_prng_next_f64(aPRNG) * size) + aMin;
}

//! Return a random number with a normal distribution.
inline tF32 RandN(tF32 sigma, int4* aPRNG = ni_prng_global()) {
#define ONE_OVER_SIGMA_EXP (1.0f / 0.7975f)
  float y;

  do {
    y = (float)-log(ni_prng_next_f64(aPRNG));
  } while (ni_prng_next_f64(aPRNG) > exp(-(y - 1.0f)*(y - 1.0f)*0.5f));

  if (ni_prng_next_bool(aPRNG)) {
    return y * sigma * ONE_OVER_SIGMA_EXP;
  }
  else {
    return -y * sigma * ONE_OVER_SIGMA_EXP;
  }
}

///////////////////////////////////////////////
inline tU32 RandColorA(tU8 aA, int4* aPRNG = ni_prng_global()) {
  return ULColorBuild(
      ni::RandIntRange(0,255,aPRNG),
      ni::RandIntRange(0,255,aPRNG),
      ni::RandIntRange(0,255,aPRNG),
      aA);
}

///////////////////////////////////////////////
inline tU32 RandColorAf(tF32 aA, int4* aPRNG = ni_prng_global()) {
  return ULColorBuild(
      ni::RandIntRange(0,255,aPRNG),
      ni::RandIntRange(0,255,aPRNG),
      ni::RandIntRange(0,255,aPRNG),
      tU8(tU32(aA*255.0f)&0xFF));
}

///////////////////////////////////////////////
inline tU32 RandColor(int4* aPRNG = ni_prng_global()) {
  return RandColorA((tU8)ni::RandIntRange(0,255,aPRNG));
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}; // End of ni
#endif // __RANDOM_15007279_H__
