#pragma once
#ifndef __BIT_H_5437D1E3_3985_EF11_82C1_49A1AE17A34C__
#define __BIT_H_5437D1E3_3985_EF11_82C1_49A1AE17A34C__

#include "../Types.h"
#include "EASTL/type_traits.h"

namespace astl {

///////////////////////////////////////////////////////////////////////////
// countl_zero
//
// Count leading zeroes in an integer.
//

static constexpr ni::tU64 _CountLeadingZeros64(ni::tU64 x) {
  if (x) {
    ni::tU64 n = 0;
    if(x & UINT64_C(0xFFFFFFFF00000000)) { n += 32; x >>= 32; }
    if(x & 0xFFFF0000)                   { n += 16; x >>= 16; }
    if(x & 0xFF00)                       { n +=  8; x >>=  8; }
    if(x & 0xF0)                         { n +=  4; x >>=  4; }
    if(x & 0xC)                          { n +=  2; x >>=  2; }
    if(x & 0x2)                          { n +=  1;           }
    return 63 - n;
  }
  return 64;
}

static constexpr ni::tU32 _CountLeadingZeros32(ni::tU32 x) {
  if(x) {
    ni::tU32 n = 0;
    if(x <= 0x0000FFFF) { n += 16; x <<= 16; }
    if(x <= 0x00FFFFFF) { n +=  8; x <<=  8; }
    if(x <= 0x0FFFFFFF) { n +=  4; x <<=  4; }
    if(x <= 0x3FFFFFFF) { n +=  2; x <<=  2; }
    if(x <= 0x7FFFFFFF) { n +=  1;           }
    return n;
  }
  return 32;
}

template <typename T>
static inline ni::tU32 countl_zero(T x) {
  if (x == 0) return sizeof(T) * 8;
  static_assert(eastl::is_unsigned<T>::value, "countl_zero requires unsigned integer type");
  static_assert(sizeof(T) <= sizeof(ni::tU64), "countl_zero type must be <= ni::tU64");

  if constexpr (sizeof(T) <= sizeof(ni::tU32)) {
    return _CountLeadingZeros32(static_cast<ni::tU32>(x)) -
        ((sizeof(ni::tU32) - sizeof(T)) * 8);
  } else {
    return _CountLeadingZeros64(static_cast<ni::tU64>(x));
  }
}

}
#endif // __BIT_H_5437D1E3_3985_EF11_82C1_49A1AE17A34C__
