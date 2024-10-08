#include "stdafx.h"
#include <niLang/STL/bit.h>
#include <niLang/STL/random.h>

using namespace ni;

struct ASTL_bit {};

TEST_FIXTURE(ASTL_bit,countl_zero_basic) {
  // Test 8-bit integers
  CHECK_EQUAL(8, astl::countl_zero(ni::tU8(0)));
  CHECK_EQUAL(7, astl::countl_zero(ni::tU8(1)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU8(128)));
  CHECK_EQUAL(4, astl::countl_zero(ni::tU8(15)));

  // Test 16-bit integers
  CHECK_EQUAL(16, astl::countl_zero(ni::tU16(0)));
  CHECK_EQUAL(15, astl::countl_zero(ni::tU16(1)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU16(32768)));
  CHECK_EQUAL(8, astl::countl_zero(ni::tU16(255)));

  // Test 32-bit integers
  CHECK_EQUAL(32, astl::countl_zero(ni::tU32(0)));
  CHECK_EQUAL(31, astl::countl_zero(ni::tU32(1)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU32(0x80000000)));
  CHECK_EQUAL(16, astl::countl_zero(ni::tU32(65535)));

  // Test 64-bit integers
  CHECK_EQUAL(64, astl::countl_zero(ni::tU64(0)));
  CHECK_EQUAL(63, astl::countl_zero(ni::tU64(1)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU64(0x8000000000000000)));
  CHECK_EQUAL(32, astl::countl_zero(ni::tU64(0xFFFFFFFF)));
}

TEST_FIXTURE(ASTL_bit,countl_zero_edgecases) {
  // Test all bits set
  CHECK_EQUAL(0, astl::countl_zero(ni::tU8(0xFF)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU16(0xFFFF)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU32(0xFFFFFFFF)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU64(0xFFFFFFFFFFFFFFFF)));

  // Test alternating bits
  CHECK_EQUAL(0, astl::countl_zero(ni::tU8(0xAA)));  // 10101010
  CHECK_EQUAL(0, astl::countl_zero(ni::tU16(0xAAAA)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU32(0xAAAAAAAA)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU64(0xAAAAAAAAAAAAAAAA)));

  // Test single bit set at different positions
  CHECK_EQUAL(7, astl::countl_zero(ni::tU8(0x01)));
  CHECK_EQUAL(6, astl::countl_zero(ni::tU8(0x02)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU8(0x80)));

  CHECK_EQUAL(15, astl::countl_zero(ni::tU16(0x0001)));
  CHECK_EQUAL(8, astl::countl_zero(ni::tU16(0x00FF)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU16(0x8000)));

  CHECK_EQUAL(31, astl::countl_zero(ni::tU32(0x00000001)));
  CHECK_EQUAL(16, astl::countl_zero(ni::tU32(0x0000FFFF)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU32(0x80000000)));

  CHECK_EQUAL(63, astl::countl_zero(ni::tU64(0x0000000000000001)));
  CHECK_EQUAL(32, astl::countl_zero(ni::tU64(0x00000000FFFFFFFF)));
  CHECK_EQUAL(0, astl::countl_zero(ni::tU64(0x8000000000000000)));
}

TEST_FIXTURE(ASTL_bit,countl_zero_randomvalues) {
  astl::mt19937_64 gen { 42 };  // Fixed seed for reproducibility

  for (int i = 0; i < 1000; ++i) {
    ni::tU64 value = gen();
    int expected = 0;
    ni::tU64 mask = ni::tU64(1) << 63;

    while ((value & mask) == 0 && mask != 0) {
      ++expected;
      mask >>= 1;
    }

    CHECK_EQUAL(expected, astl::countl_zero(value));
  }
}
