#include "stdafx.h"
#include "../src/API/niLang/STL/limits.h"

///////////////////////////////////////////////
TEST(ASTL_limits_I8) {
  CHECK_EQUAL(niMinI8,astl::numeric_limits<ni::tI8>::min());
  CHECK_EQUAL(niMaxI8,astl::numeric_limits<ni::tI8>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_U8) {
  CHECK_EQUAL(niMinU8,astl::numeric_limits<ni::tU8>::min());
  CHECK_EQUAL(niMaxU8,astl::numeric_limits<ni::tU8>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_I16) {
  CHECK_EQUAL(niMinI16,astl::numeric_limits<ni::tI16>::min());
  CHECK_EQUAL(niMaxI16,astl::numeric_limits<ni::tI16>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_U16) {
  CHECK_EQUAL(niMinU16,astl::numeric_limits<ni::tU16>::min());
  CHECK_EQUAL(niMaxU16,astl::numeric_limits<ni::tU16>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_I32) {
  CHECK_EQUAL(niMinI32,astl::numeric_limits<ni::tI32>::min());
  CHECK_EQUAL(niMaxI32,astl::numeric_limits<ni::tI32>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_U32) {
  CHECK_EQUAL(niMinU32,astl::numeric_limits<ni::tU32>::min());
  CHECK_EQUAL(niMaxU32,astl::numeric_limits<ni::tU32>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_I64) {
  CHECK_EQUAL(niMinI64,astl::numeric_limits<ni::tI64>::min());
  CHECK_EQUAL(niMaxI64,astl::numeric_limits<ni::tI64>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_U64) {
  CHECK_EQUAL(niMinU64,astl::numeric_limits<ni::tU64>::min());
  CHECK_EQUAL(niMaxU64,astl::numeric_limits<ni::tU64>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_F32) {
  CHECK_EQUAL(niMinF32,astl::numeric_limits<ni::tF32>::min());
  CHECK_EQUAL(niMaxF32,astl::numeric_limits<ni::tF32>::max());
}

///////////////////////////////////////////////
TEST(ASTL_limits_F64) {
  CHECK_EQUAL(niMinF64,astl::numeric_limits<ni::tF64>::min());
  CHECK_EQUAL(niMaxF64,astl::numeric_limits<ni::tF64>::max());
}
