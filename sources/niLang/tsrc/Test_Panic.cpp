// #define niNoExceptions
#include "stdafx.h"
#include <niLang/Utils/CrashReport.h>

using namespace ni;
using namespace astl;

namespace foo {
_HSymImpl(my_panic);
_HSymExport(my_panic);
} // namespace foo

struct FPanic {};

void TestPanic()
{
  niPanic(ni, panic, "");
}

void TestPanicMsg(const char* msg)
{
  niPanic(foo, my_panic, msg);
}

TEST_FIXTURE(FPanic, Panic)
{
  tHStringPtr caughtKind;
  cString caughtDesc;
  RecoverPanic([&]() { TestPanic(); },
               [&](const ni::sPanicDesc& e) {
                 caughtKind = e.GetKind();
                 caughtDesc = e.GetDesc();
                 niDebugFmt(("... caught: %s", e.GetKind()));
               });
  CHECK_EQUAL(_HC(panic), caughtKind.raw_ptr());
  CHECK(caughtDesc.contains("--- CALLSTACK ---"));
};

TEST_FIXTURE(FPanic, PanicWithMsg)
{
  tHStringPtr caughtKind;
  cString caughtDesc;
  RecoverPanic([&]() { TestPanicMsg("foo bar qoo"); },
               [&](const ni::sPanicDesc& e) {
                 caughtKind = e.GetKind();
                 caughtDesc = e.GetDesc();
                 niDebugFmt(("... caught: %s", e.GetKind()));
               });
  CHECK_EQUAL(_HSym(foo, my_panic), caughtKind.raw_ptr());
  CHECK(caughtDesc.contains("--- CALLSTACK ---"));
};
