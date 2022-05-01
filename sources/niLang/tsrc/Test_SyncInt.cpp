#include "stdafx.h"
using namespace ni;

struct FSyncInt {};

niCAssert(sizeof(SYNC_INT_TYPE) == 4);

TEST_FIXTURE(FSyncInt, IncDec) {
  tSyncInt v = 0;
  CHECK_EQUAL(0,v);
  CHECK_EQUAL(1,SYNC_INCREMENT(&v));
  CHECK_EQUAL(0,SYNC_DECREMENT(&v));
  niLoop(i,3) {
    CHECK_EQUAL(i+1,SYNC_INCREMENT(&v));
  }
  niLoopr(i,3) {
    CHECK_EQUAL(i,SYNC_DECREMENT(&v));
  }
}
