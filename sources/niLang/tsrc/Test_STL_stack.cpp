#include "stdafx.h"
#include "../src/API/niLang/STL/stack.h"

// template class astl::stack<ni::tInt>;

struct ASTL_stack {};

TEST_FIXTURE(ASTL_stack,push) {
  astl::stack<ni::tInt> q;
  q.push(123);
  CHECK_EQUAL(q.size(), 1);
}
