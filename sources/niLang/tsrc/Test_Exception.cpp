// #define niNoExceptions
#include "stdafx.h"
#include <niLang/Utils/CrashReport.h>

using namespace ni;
using namespace astl;

namespace foo {
_HSymImpl(my_exception);
_HSymExport(my_exception);
} // namespace foo

struct FException {};

void TestThrowPanic()
{
  niThrowPanic(ni, panic, "");
}

void TestThrowPanicMsg(const char* msg)
{
  niThrowPanic(foo, my_exception, msg);
}

#if 0
TEST_FIXTURE(FException,CheckThrow1) {
  CHECK_THROW(TestThrowPanic(), iPanicDescription);
}

TEST_FIXTURE(FException,CheckThrow2) {
  CHECK_THROW(TestThrowPanicMsg("weee"), iPanicDescription);
}
#endif

TEST_FIXTURE(FException, Panic)
{
  tHStringPtr caughtKind;
  cString caughtDesc;
  TryCatchPanic([&]() { TestThrowPanic(); },
                [&](const ni::iPanicDescription& e) {
                  caughtKind = e.GetKind();
                  caughtDesc = e.GetDesc();
                  niDebugFmt(("... caught: %s", e.GetKind()));
                });
  CHECK_EQUAL(_HC(panic), caughtKind.raw_ptr());
  CHECK(caughtDesc.contains("--- CALLSTACK ---"));
};

TEST_FIXTURE(FException, PanicWithMsg)
{
  tHStringPtr caughtKind;
  cString caughtDesc;
  TryCatchPanic([&]() { TestThrowPanicMsg("foo bar qoo"); },
                [&](const ni::iPanicDescription& e) {
                  caughtKind = e.GetKind();
                  caughtDesc = e.GetDesc();
                  niDebugFmt(("... caught: %s", e.GetKind()));
                });
  CHECK_EQUAL(_HSym(foo, my_exception), caughtKind.raw_ptr());
  CHECK(caughtDesc.contains("--- CALLSTACK ---"));
};
