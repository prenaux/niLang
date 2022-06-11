#include "stdafx.h"
#include "../src/API/niLang/Utils/CrashReport.h"

using namespace ni;

struct FCrashReport {};

TEST_FIXTURE(FCrashReport,StackGetCurrent) {
  ni::cString stack;
  ni_stack_get_current(stack,NULL);
  niDebugFmt(("... stack: %s", stack));
  CHECK(stack.contains("FCrashReportStackGetCurrentHelper::RunTest"));
}
