#define TEST_NO_EXCEPTIONS 1 // So that the niPanic* are not overriden
#include <niLang.h>
#include <niUnitTest.h>
#include <niLang/Utils/CrashReport.h>
#include <niLang/ILang.h>

using namespace ni;

struct FCrashReport {};

TEST_FIXTURE(FCrashReport,StackGetCurrent) {
  ni::cString stack;
  ni_stack_get_current(stack,NULL);
  niDebugFmt(("... stack: %s", stack));
  CHECK(stack.contains("FCrashReportStackGetCurrentHelper::RunTest"));
}

TEST_FIXTURE(FCrashReport,PanicAssert) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    niPanicAssert(false);
  }
  else {
    niLog(Info,"Skipped because not interactive.");
  }
}

TEST_FIXTURE(FCrashReport,PanicAssertMsg) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    niPanicAssertMsg(false,"expected to fail when run as an interactive test");
  }
  else {
    niLog(Info,"Skipped because not interactive.");
  }
}

TEST_FIXTURE(FCrashReport,PanicUnreachable) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    niPanicUnreachable();
  }
  else {
    niLog(Info,"Skipped because not interactive.");
  }
}

TEST_FIXTURE(FCrashReport,PanicUnreachableMsg) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    niPanicUnreachable("We shouldn't get here.");
  }
  else {
    niLog(Info,"Skipped because not interactive.");
  }
}

TEST_FIXTURE(FCrashReport,AssertMessageBox) {
  const bool isInteractive = (UnitTest::runFixtureName == m_testName);
  if (isInteractive) {
    niDebugFmt(("... FCrashReport,AssertMessageBox"));
    ni_debug_set_show_assert_message_box(1);
    niDoAssert(false);
  }
  else {
    niLog(Info,"Skipped because not interactive.");
  }
}
