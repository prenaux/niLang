#include "stdafx.h"
#include "../src/API/niLang/STL/set.h"
#include "../src/API/niLang/STL/hash_set.h"
using namespace ni;

// template class astl::set<ni::tInt>;
// template class astl::hash_set<ni::tInt>;

struct ASTL_set {
};

TEST_FIXTURE(ASTL_set, set) {
  astl::set<tInt> s = { 1, 2 };
  s.insert(123);
  s.emplace(456);
  CHECK_EQUAL(4, s.size());
}

TEST_FIXTURE(ASTL_set, hash_set) {
  astl::hash_set<tInt> s = { 1, 2 };
  s.insert(123);
  s.emplace(456);
  CHECK_EQUAL(4, s.size());
}
