#include "stdafx.h"
#include "../src/API/niLang/ILang.h"
#include "../src/API/niLang/STL/array.h"

namespace {

using namespace ni;

struct FHString {
};

///////////////////////////////////////////////
TEST_FIXTURE(FHString,Basics) {
  TEST_TRACK_MEMORY_BEGIN(); {
    tHStringPtr hspA = ni::CreateHStringFromView("A");
    tHStringPtr hspA1 = ni::CreateHStringFromView("A");
    tHStringPtr hspB = ni::CreateHStringFromView("B");
    tHStringPtr hspB1 = ni::CreateHStringFromView("B");
    CHECK_EQUAL((tI64)hspA.ptr(),(tI64)hspA1.ptr());
    CHECK_EQUAL(_ASTR("A"),_ASTR(niHStr(hspA)));
    CHECK_EQUAL((tI64)hspB.ptr(),(tI64)hspB1.ptr());
    CHECK_EQUAL(_ASTR("B"),_ASTR(niHStr(hspB)));
  } TEST_TRACK_MEMORY_END();
}

///////////////////////////////////////////////
TEST_FIXTURE(FHString,LoopSame) {
  TEST_TRACK_MEMORY_BEGIN(); {
    tHStringPtr hspA = ni::CreateHStringFromView("A");
    tHStringPtr hspB = ni::CreateHStringFromView("B");
    niLoop(i,10) {
      tHStringPtr hspA1 = ni::CreateHStringFromView("A");
      tHStringPtr hspB1 = ni::CreateHStringFromView("B");
      CHECK_EQUAL((tI64)hspA.ptr(),(tI64)hspA1.ptr());
      CHECK_EQUAL((tI64)hspB.ptr(),(tI64)hspB1.ptr());
    }
  } TEST_TRACK_MEMORY_END();
}

///////////////////////////////////////////////
TEST_FIXTURE(FHString,LoopI) {
  astl::array<ni::cString,10> As;
  astl::array<ni::cString,As.size()> Bs;
  niLoop(i,As.size()) {
    As[i] = niFmt("A%d", i);
    Bs[i] = niFmt("B%d", i);
  }
  TEST_TRACK_MEMORY_BEGIN(); {
    niLoop(i,As.size()) {
      tHStringPtr hspAi = ni::CreateHStringFromView(As[i]);
      tHStringPtr hspBi = ni::CreateHStringFromView(Bs[i]);
      CHECK_NOT_EQUAL((tI64)hspAi.ptr(),(tI64)hspBi.ptr());
    }
    TEST_TRACK_MEMORY_EXPECTED_ALLOCS(40);
    TEST_TRACK_MEMORY_EXPECTED_OBJ_ALLOCS(20);
  } TEST_TRACK_MEMORY_END();
}

///////////////////////////////////////////////
TEST_FIXTURE(FHString,EmptyString) {
  TEST_TRACK_MEMORY_BEGIN(); {
    tHStringPtr hspA = ni::CreateHStringFromView("");
    // niDebugFmt(("... hspA: %p", (tIntPtr)hspA.ptr()));
    CHECK_NOT_EQUAL(nullptr,hspA.ptr());
    tHStringPtr hspB = ni::CreateHStringFromView("");
    // niDebugFmt(("... hspB: %p", (tIntPtr)hspA.ptr()));
    CHECK_NOT_EQUAL(nullptr,hspB.ptr());
    CHECK_EQUAL((tI64)hspA.ptr(),(tI64)hspB.ptr());
    CHECK_EQUAL(_ASTR(""),_ASTR(niHStr(hspA)));
    CHECK_EQUAL(_ASTR(""),_ASTR(niHStr(hspB)));
    TEST_TRACK_MEMORY_EXPECTED_ALLOCS(0);
  } TEST_TRACK_MEMORY_END();
}

} // end of anonymous namespace
