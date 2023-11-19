#include "stdafx.h"
#include "../src/API/niLang/STL/queue.h"

// template class astl::queue<ni::tInt>;

struct ASTL_queue {};

TEST_FIXTURE(ASTL_queue, push) {
  astl::queue<ni::tInt> q;
  q.push(123);
  CHECK_EQUAL(q.size(), 1);
}
