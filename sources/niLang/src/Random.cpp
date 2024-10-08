// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/Utils/Random.h"

#include "API/niLang/ILang.h"
#include "API/niLang/Math/MathUtils.h"
#include "API/niLang/STL/run_once.h"
#include "API/niLang/STL/scope_guard.h"
#include "API/niLang/Types.h"
#include "API/niLang/Utils/CryptoUtils.h"

#if defined niLinux
#include <fcntl.h>
#include <unistd.h>

namespace ni {
tBool RandSecureGetBytes(tPtr apOutput, tSize anSize) {
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
    return eFalse;
  }
  niDefer { close(fd); };
  if (anSize > 0) {
    niPanicAssert(apOutput != nullptr);
    if (read(fd, apOutput, anSize) != anSize) {
      return eFalse;
    }
  }
  return eTrue;
}
}  // namespace ni

#elif defined niOSX || defined niIOS

#include <Security/Security.h>

namespace ni {
tBool RandSecureGetBytes(tPtr apOutput, tSize anSize) {
  if (anSize > 0) {
    niPanicAssert(apOutput != nullptr);
    if (SecRandomCopyBytes(kSecRandomDefault, anSize, (uint8_t*)apOutput) !=
        errSecSuccess) {
      return eFalse;
    }
  }
  return eTrue;
}
}  // namespace ni

#elif defined niWindows
#include <windows.h>
#include <wincrypt.h>
#undef GetCurrentTime

namespace ni {
tBool RandSecureGetBytes(tPtr apOutput, tSize anSize) {
  if (anSize > 0) {
    niPanicAssert(apOutput != nullptr);
    if (!CryptGenRandom(NULL, anSize, apOutput)) {
      return eFalse;
    }
  }
  return eTrue;
}
}  // namespace ni

#else

namespace ni {
tBool RandSecureGetBytes(tPtr apOutput, tSize anSize) {
  return eFalse;
}
}

#endif

namespace ni {
//
// Based off V8's random number generator
//
// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE-v8.txt file
//
// This class is used to generate a stream of pseudo-random numbers. The class
// uses a 64-bit seed, which is passed through MurmurHash3 to create two 64-bit
// state values. This pair of state values is then used in xorshift128+.
// The resulting stream of pseudo-random numbers has a period length of 2^128-1.
// See Marsaglia: http://www.jstatsoft.org/v08/i14/paper
// And Vigna: http://vigna.di.unimi.it/ftp/papers/xorshiftplus.pdf
// NOTE: Any changes to the algorithm must be tested against TestU01.
//       Please find instructions for this in the internal repository.
//
// If two instances of RandomNumberGenerator are created with the same seed, and
// the same sequence of method calls is made for each, they will generate and
// return identical sequences of numbers.
// This class uses (probably) weak entropy by default, but it's sufficient,
// because it is the responsibility of the embedder to install an entropy source
// using v8::V8::SetEntropySource(), which provides reasonable entropy, see:
// https://code.google.com/p/v8/issues/detail?id=2905
// This class is neither reentrant nor threadsafe.
//

niExportFunc(tU64) ni_prng_get_seed_from_time_source() {
  tU64 seed =
      ((tU64)(ni::GetLang()->GetCurrentTime()->GetUnixTimeSecs()) << 24) +
      ((tU64)(ni::GetLang()->TimerInSeconds() * 1e6));
  seed ^= (tU64)(ni::TimerInSeconds() * 1000) << 16;
  seed ^= (tU64)(ni::TimerInSeconds() * 1000) << 8;
  niPanicAssert(seed != 0);
  return seed;
}

niExportFunc(tU64) ni_prng_get_seed_from_secure_source() {
  tU64 seed {0};
  if (!RandSecureGetBytes((tPtr)(&seed), sizeof(seed))) {
    niPanicUnreachable("No secure random entropy source.");
  }
  niPanicAssert(seed != 0);
  return seed;
}

niExportFunc(tU64) ni_prng_get_seed_from_maybe_secure_source() {
  tU64 seed {0};
  if (!RandSecureGetBytes((tPtr)(&seed), sizeof(seed))) {
    return ni_prng_get_seed_from_time_source();
  }
  niPanicAssert(seed != 0);
  return seed;
}

struct RandomNumberGenerator {
  // Returns the next pseudorandom, uniformly distributed int value from this
  // random number generator's sequence. The general contract of |NextInt()| is
  // that one int value is pseudorandomly generated and returned.
  // All 2^32 possible integer values are produced with (approximately) equal
  // probability.
  __forceinline tI32 NextInt() { return Next(32); }

  // Returns a pseudorandom, uniformly distributed int value between 0
  // (inclusive) and the specified max value (exclusive), drawn from this random
  // number generator's sequence. The general contract of |NextInt(int)| is that
  // one int value in the specified range is pseudorandomly generated and
  // returned. All max possible int values are produced with (approximately)
  // equal probability.
  tI32 NextInt(tI64 max) {
    // Fast path if max is a power of 2.
    if (ni::IsPow2(max)) {
      return static_cast<int>((max * static_cast<tI64>(Next(31))) >> 31);
    }

    while (true) {
      int rnd = Next(31);
      int val = rnd % max;
      if ((rnd - val + (max - 1)) >= 0) {
        return val;
      }
    }
  }

  tI32 Next(tU32 bits) {
    niAssert(bits <= 32);
    XorShift128(&state0_, &state1_);
    return static_cast<tI32>((state0_ + state1_) >> (64 - bits));
  }

  // Returns the next pseudorandom, uniformly distributed boolean value from
  // this random number generator's sequence. The general contract of
  // |NextBoolean()| is that one boolean value is pseudorandomly generated and
  // returned. The values true and false are produced with (approximately) equal
  // probability.
  __forceinline tBool NextBool() { return Next(1) != 0; }

  // Returns the next pseudorandom, uniformly distributed double value between
  // 0.0 and 1.0 from this random number generator's sequence.
  // The general contract of |NextDouble()| is that one double value, chosen
  // (approximately) uniformly from the range 0.0 (inclusive) to 1.0
  // (exclusive), is pseudorandomly generated and returned.
  tF64 NextDouble() {
    XorShift128(&state0_, &state1_);
    return ToDouble(state0_, state1_);
  }

  // Returns the next pseudorandom, uniformly distributed int64 value from this
  // random number generator's sequence. The general contract of |NextInt64()|
  // is that one 64-bit int value is pseudorandomly generated and returned.
  // All 2^64 possible integer values are produced with (approximately) equal
  // probability.
  tI64 NextInt64() {
    XorShift128(&state0_, &state1_);
    return ni::bit_cast<tI64>(state0_ + state1_);
  }

  // Fills the elements of a specified array of bytes with random numbers.
  void NextBytes(void* buffer, size_t buflen) {
    for (size_t n = 0; n < buflen; ++n) {
      static_cast<tU8*>(buffer)[n] = static_cast<tU8>(Next(8));
    }
  }

  // Override the current ssed.
  void SetSeed(tI64 seed) {
    state0_ = MurmurHash3(ni::bit_cast<tU64>(seed));
    state1_ = MurmurHash3(state0_);
  }

  // Static and exposed for external use.
  static inline tF64 ToDouble(tU64 state0, tU64 state1) {
    // Exponent for double values for [1.0 .. 2.0)
    static const tU64 kExponentBits = niU64Const(0x3FF0000000000000);
    static const tU64 kMantissaMask = niU64Const(0x000FFFFFFFFFFFFF);
    tU64 random = ((state0 + state1) & kMantissaMask) | kExponentBits;
    return ni::bit_cast<double>(random) - 1;
  }

  // Static and exposed for external use.
  static inline void XorShift128(tU64* state0, tU64* state1) {
    tU64 s1 = *state0;
    tU64 s0 = *state1;
    *state0 = s0;
    s1 ^= s1 << 23;
    s1 ^= s1 >> 17;
    s1 ^= s0;
    s1 ^= s0 >> 26;
    *state1 = s1;
  }

  static tU64 MurmurHash3(tU64 h) {
    h ^= h >> 33;
    h *= niU64Const(0xFF51AFD7ED558CCD);
    h ^= h >> 33;
    h *= niU64Const(0xC4CEB9FE1A85EC53);
    h ^= h >> 33;
    return h;
  }

 private:
  static const tI64 kMultiplier = niU64Const2Parts(0x5, deece66d);
  static const tI64 kAddend = 0xb;
  static const tI64 kMask = niU64Const2Parts(0xffff, ffffffff);

  tU64 state0_;
  tU64 state1_;
};
niCAssert(sizeof(int4) == sizeof(RandomNumberGenerator));

niExportFunc(tU64)
    ni_prng_seed_from_string(const achar* aString, const tI32 aStrLen) {
  Ptr<iCryptoHash> hash = ni::CreateHash("SHA512_256");
  ni::HashString(hash, "prng", 4);
  ni::HashString(hash, aString, aStrLen);
  return HashToInt256(hash).x;
}

niExportFunc(int4) ni_prng_init(tU64 anSeed) {
  int4 prng;
  ni_prng_seed(&prng, anSeed);
  return ni::bit_cast<int4>(prng);
}

niExportFunc(void) ni_prng_seed(int4* aPRNG, tU64 anSeed) {
  niCAssert(sizeof(*aPRNG) == sizeof(RandomNumberGenerator));
  ((RandomNumberGenerator*)aPRNG)->SetSeed(anSeed);
}

niExportFunc(tI32) ni_prng_next_i32(int4* aPRNG) {
  return ((RandomNumberGenerator*)aPRNG)->NextInt();
}

niExportFunc(tI64) ni_prng_next_i64(int4* aPRNG) {
  return ((RandomNumberGenerator*)aPRNG)->NextInt64();
}

niExportFunc(tF64) ni_prng_next_f64(int4* aPRNG) {
  return ((RandomNumberGenerator*)aPRNG)->NextDouble();
}

niExportFunc(void)
    ni_prng_next_bytes(int4* aPRNG, tPtr apBytes, tSize anNumBytes) {
  ((RandomNumberGenerator*)aPRNG)->NextBytes(apBytes, anNumBytes);
}

niExportFunc(tBool) ni_prng_next_bool(int4* aPRNG) {
  return ((RandomNumberGenerator*)aPRNG)->NextBool();
}

niExportFunc(int4*) ni_prng_global() {
  static int4 _sState = ni_prng_init(ni_prng_get_seed_from_maybe_secure_source());
  return &_sState;
}

}  // namespace ni
