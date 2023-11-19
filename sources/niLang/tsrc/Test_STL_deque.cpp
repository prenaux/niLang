#include "stdafx.h"
#include "../src/API/niLang/STL/deque.h"

// template class astl::deque<ni::tInt>;

struct ASTL_deque {};

const ni::tU32 kNumTests = 10000;

TEST_FIXTURE(ASTL_deque,insert_time) {
  TEST_TIMEREPORT();
  astl::deque<ni::tU32> v;
  TEST_PRINT(niFmt(_A("DEQUE SIZE: %d\n"),sizeof(v)));
  niLoop(i,kNumTests) {
    v.push_back(i);
  }
  CHECK_EQUAL(10,v[10]);
  CHECK_EQUAL(20,v[20]);
  CHECK_EQUAL(30,v[30]);
  CHECK_EQUAL(kNumTests-1,v[kNumTests-1]);
}

typedef astl::deque<ni::tInt> tIntDeq;
typedef astl::deque<ni::tU8>  tU8Deq;
typedef astl::deque<ni::tU16> tU16Deq;
typedef astl::deque<ni::tU32> tU32Deq;

TEST_FIXTURE(ASTL_deque,IntDeq_pushback) {
  tIntDeq deq;
  deq.push_back(1);
  deq.push_back(2);
  deq.push_back(3);
  CHECK_EQUAL(1,deq[0]);
  CHECK_EQUAL(2,deq[1]);
  CHECK_EQUAL(3,deq[2]);
  CHECK_EQUAL(3,deq.size());
}

TEST_FIXTURE(ASTL_deque,U8Deq_pushback) {
  tU8Deq deq;
  deq.push_back(1);
  deq.push_back(2);
  deq.push_back(3);
  CHECK_EQUAL(1,deq[0]);
  CHECK_EQUAL(2,deq[1]);
  CHECK_EQUAL(3,deq[2]);
  CHECK_EQUAL(3,deq.size());
}

TEST_FIXTURE(ASTL_deque,U16Deq_pushback) {
  tU16Deq deq;
  deq.push_back(1);
  deq.push_back(2);
  deq.push_back(3);
  CHECK_EQUAL(1,deq[0]);
  CHECK_EQUAL(2,deq[1]);
  CHECK_EQUAL(3,deq[2]);
  CHECK_EQUAL(3,deq.size());
}

TEST_FIXTURE(ASTL_deque,U32Deq_pushback) {
  tU32Deq deq;
  deq.push_back(1);
  deq.push_back(2);
  deq.push_back(3);
  CHECK_EQUAL(1,deq[0]);
  CHECK_EQUAL(2,deq[1]);
  CHECK_EQUAL(3,deq[2]);
  CHECK_EQUAL(3,deq.size());
}
