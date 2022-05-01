#include "stdafx.h"
#include <niLang/STL/vector.h>
#include <niLang/STL/utils.h>

// template class astl::vector<ni::tInt,astl::raw_allocator_vector>;
typedef astl::vector<ni::tInt> tIntVec;
typedef astl::vector<ni::tU8>  tU8Vec;
typedef astl::vector<ni::tU16> tU16Vec;
typedef astl::vector<ni::tU32> tU32Vec;

const ni::tU32 kNumTests = 10000;

struct ASTL_vector {
};

TEST_FIXTURE(ASTL_vector,initializer) {
  astl::vector<ni::cString> v { "a", "b", "c" };
  CHECK_EQUAL(_ASTR("a"),v[0]);
  CHECK_EQUAL(_ASTR("b"),v[1]);
  CHECK_EQUAL(_ASTR("c"),v[2]);
}

TEST_FIXTURE(ASTL_vector,at_default) {
  {
    astl::vector<ni::cString> v { "a", "b", "c" };
    CHECK_EQUAL(_ASTR("c"),astl::at_default(v,2,""));
    CHECK_EQUAL(_ASTR(""),astl::at_default(v,3,""));
    CHECK_EQUAL(_ASTR(""),astl::at_default(v,123,""));
  }

  {
    astl::vector<ni::tInt> v { 1, 2, 3 };
    CHECK_EQUAL(3,astl::at_default(v,2,ni::eInvalidHandle));
    CHECK_EQUAL(ni::eInvalidHandle,astl::at_default(v,3,ni::eInvalidHandle));
    CHECK_EQUAL(ni::eInvalidHandle,astl::at_default(v,123,ni::eInvalidHandle));
  }
}

TEST_FIXTURE(ASTL_vector,insert_time) {
  TEST_TIMEREPORT();
  astl::vector<ni::tU32> v;
  TEST_PRINT(niFmt(_A("VECTOR SIZE: %d\n"),sizeof(v)));
  niLoop(i,kNumTests) {
    v.push_back(i);
  }
  CHECK_EQUAL(10,v[10]);
  CHECK_EQUAL(20,v[20]);
  CHECK_EQUAL(30,v[30]);
  CHECK_EQUAL(kNumTests-1,v[kNumTests-1]);
}

TEST_FIXTURE(ASTL_vector,reserve_insert_time) {
  TEST_TIMEREPORT();
  astl::vector<ni::tU32> v;
  v.reserve(kNumTests);
  niLoop(i,kNumTests) {
    v.push_back(i);
  }
  CHECK_EQUAL(10,v[10]);
  CHECK_EQUAL(20,v[20]);
  CHECK_EQUAL(30,v[30]);
  CHECK_EQUAL(kNumTests-1,v[kNumTests-1]);
}

TEST_FIXTURE(ASTL_vector,IntVec_pushback) {
  tIntVec vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  CHECK_EQUAL(1,vec[0]);
  CHECK_EQUAL(2,vec[1]);
  CHECK_EQUAL(3,vec[2]);
  CHECK_EQUAL(3,vec.size());
}

TEST_FIXTURE(ASTL_vector,U8Vec_pushback) {
  tU8Vec vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  CHECK_EQUAL(1,vec[0]);
  CHECK_EQUAL(2,vec[1]);
  CHECK_EQUAL(3,vec[2]);
  CHECK_EQUAL(3,vec.size());
}
TEST_FIXTURE(ASTL_vector,U16Vec_pushback) {
  tU16Vec vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  CHECK_EQUAL(1,vec[0]);
  CHECK_EQUAL(2,vec[1]);
  CHECK_EQUAL(3,vec[2]);
  CHECK_EQUAL(3,vec.size());
}

TEST_FIXTURE(ASTL_vector,U32Vec_pushback) {
  tU32Vec vec;
  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  CHECK_EQUAL(1,vec[0]);
  CHECK_EQUAL(2,vec[1]);
  CHECK_EQUAL(3,vec[2]);
  CHECK_EQUAL(3,vec.size());

  tU16Vec vec2;
  vec2.push_back(1);
  vec2.push_back(2);
  vec2.push_back(3);
  CHECK_EQUAL(1,vec2[0]);
  CHECK_EQUAL(2,vec2[1]);
  CHECK_EQUAL(3,vec2[2]);
  CHECK_EQUAL(3,vec2.size());
}
