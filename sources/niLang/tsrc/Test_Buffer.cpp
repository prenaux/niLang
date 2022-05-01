#include "stdafx.h"
#include "../src/API/niLang/Utils/Buffer.h"

using namespace ni;

struct FBuffer {
};

TEST_FIXTURE(FBuffer,MallocZero) {
  tPtr zero1 = (tPtr)niMalloc(0);
  tPtr zero2 = (tPtr)niMalloc(0);
  CHECK_EQUAL(zero1, zero2);
  tU8 buffer[8];
  ni::MemCopy(buffer, zero1, sizeof(buffer));
  niLoop(i,niCountOf(buffer)) {
    CHECK_EQUAL(0,buffer[i]);
  }
  niFree(zero1);
  tPtr foo = (tPtr)niRealloc(zero1, 16);
  CHECK_NOT_EQUAL(zero1, foo);
  niFree(foo);
}

TEST_FIXTURE(FBuffer,Realloc) {
#ifdef CLANG_SANITIZE
  AUTO_WARNING_MODE();
#endif

  // Use a non aligned size which should lead to reallocating in place
  const tSize startSize = 500;

  //
  // Use a larger allocation size, small allocations are usually bucketed so
  // realloc will always move the memory. However a sane allocator should
  // resize larger buffers in place.
  //
  tPtr foo = (tPtr)niRealloc(NULL, startSize);

  const tSize newSize = startSize + 16;

  const tPtr prevFoo = foo;
  foo = (tPtr)niRealloc(foo, newSize);

  // Test that realloc has resized the buffer in place
  CHECK_EQUAL(foo, prevFoo);

  niFree(foo);
}
