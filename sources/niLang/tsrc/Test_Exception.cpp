// #define niNoExceptions
#include "stdafx.h"
#include <niLang/Utils/Exception.h>

using namespace ni;
using namespace astl;

_HDecl(my_exception);

struct FException {
};

void TestThrowPanic() {
  niThrowPanic(panic);
  // throw sMyPanic{"sMyPanic",nullptr};
}

void TestThrowPanicMsg(const char* msg) {
  niThrowPanicMsg(my_exception, msg);
  // throw sMyPanic{"sMyPanic",nullptr};
}

TEST_FIXTURE(FException,Test) {
#if 1
  CHECK_THROW(TestThrowPanic(), sPanicException);

  niTry {
    TestThrowPanic();
  }
  niCatchPanic(e) {
    CHECK_EQUAL(_ASTR(niHStr(_HC(panic))), _ASTR(niHStr(e._kind)));
  }

#else
  tBool didThrow = eFalse;
  try {
    mayThrow();
  }
  catch(const sPanicException& e) {
    niDebugFmt(("... yey2: sPanicException: %s", e.what()));
    didThrow = eTrue;
  }
  catch(const std::exception& e) {
    niDebugFmt(("... std::exception: %s", e.what()));
  }
  catch(...) {
    niDebugFmt(("... catch all: some exception"));
  }
  CHECK_EQUAL(eTrue, didThrow);
#endif
};

TEST_FIXTURE(FException,Msg) {
  CHECK_THROW(TestThrowPanicMsg("weee"), sPanicException);

  niTry {
    CHECK_THROW(TestThrowPanicMsg("foo bar qoo"), sPanicException);
  }
  niCatchPanic(e) {
    niDebugFmt(("... sPanicException: %s", e.what()));
    CHECK_EQUAL(_ASTR(niHStr(_HC(my_exception))), _ASTR(niHStr(e._kind)));
    CHECK_EQUAL(_ASTR("foo bar qoo"), e._msg);
  }
};
