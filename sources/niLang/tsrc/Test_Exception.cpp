// #define niNoExceptions
#include "stdafx.h"

using namespace ni;
using namespace astl;

namespace foo {
_HSymImpl(my_exception);
_HSymExport(my_exception);
}

struct FException {
};

void TestThrowPanic() {
  niThrowPanic(ni,panic,"");
}

void TestThrowPanicMsg(const char* msg) {
  niThrowPanic(foo,my_exception,msg);
}

TEST_FIXTURE(FException,CheckThrow1) {
  CHECK_THROW(TestThrowPanic(), iPanicException);
}

TEST_FIXTURE(FException,CheckThrow2) {
  CHECK_THROW(TestThrowPanicMsg("weee"), iPanicException);
}

TEST_FIXTURE(FException,Std) {
  tHStringPtr caughtKind;
  cString caughtDesc;
  try {
    TestThrowPanic();
  } catch (const iPanicException& e) {
    caughtKind = e.GetKind();
    caughtDesc = e.GetDesc();
    niDebugFmt(("... caught: %s", e.GetKind()));
  } catch (...) {
    CHECK(false && "Unreachable");
  }
  CHECK_EQUAL(_HC(panic), caughtKind.raw_ptr());
  CHECK(caughtDesc.contains("--- CALLSTACK ---"));
};

TEST_FIXTURE(FException,GenericPanic) {
  tHStringPtr caughtKind;
  cString caughtDesc;
  niTry {
    TestThrowPanic();
  }
  niCatch(ni::iPanicException,e) {
    caughtKind = e.GetKind();
    caughtDesc = e.GetDesc();
    niDebugFmt(("... caught: %s", e.GetKind()));
  }
  CHECK_EQUAL(_HC(panic), caughtKind.raw_ptr());
  CHECK(caughtDesc.contains("--- CALLSTACK ---"));
};

TEST_FIXTURE(FException,WithMsg) {
  tHStringPtr caughtKind;
  cString caughtDesc;
  niTry {
    TestThrowPanicMsg("foo bar qoo");
  }
  niCatch(ni::iPanicException,e) {
    caughtKind = e.GetKind();
    caughtDesc = e.GetDesc();
    niDebugFmt(("... caught: %s", e.GetKind()));
  }
  CHECK_EQUAL(_HSym(foo,my_exception), caughtKind.raw_ptr());
  CHECK(caughtDesc.contains("--- CALLSTACK ---"));
};
