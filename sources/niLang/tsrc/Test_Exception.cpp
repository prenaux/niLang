// #define niNoExceptions
#include "stdafx.h"

using namespace ni;
using namespace astl;

static_assert(sizeof(sPanicException) == sizeof(tIntPtr));

namespace foo {
_HSymImpl(my_exception);
_HSymExport(my_exception);
}

struct FException {
};

void TestThrowPanic() {
  niThrowPanic(ni,panic,"TestThrowPanic Message");
}

void TestThrowPanicMsg(const char* msg) {
  niThrowPanic(foo,my_exception,msg);
}

TEST_FIXTURE(FException,Test) {
#if 1
  // sPanicException should be the size of one pointer
  CHECK_THROW(TestThrowPanic(), sPanicException);

  tHStringPtr caught;
  niTry {
    TestThrowPanic();
  }
  niCatch(ni::sPanicException,e) {
    caught = e.GetKind();
    niDebugFmt(("... caught: %s", caught));
  }
  CHECK_EQUAL(_HC(panic), caught.raw_ptr());

  {
    sPanicException a;
    sPanicException b = a;
    sPanicException c = std::move(b);
    CHECK_EQUAL(_HSym(ni,not_initialized),c.GetKind());
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
  niCatch(ni::sPanicException,e) {
    niDebugFmt(("... sPanicException: %s", e.GetKind()));
    CHECK_EQUAL(_HSym(foo,my_exception), e.GetKind());
  }
};
