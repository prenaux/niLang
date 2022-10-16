#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>
#include <niLang/Utils/CrashReport.h>
niCrashReport_DeclareHandler();

#if defined niEmbedded
#define EMBEDDED_FIXTURE_NAME "FCURLFetch-GetJson"
#endif

niConsoleMain() {
  ni::cString fixtureName;
#ifdef EMBEDDED_FIXTURE_NAME
  fixtureName = EMBEDDED_FIXTURE_NAME;
#else
  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(),&fixtureName);
#endif
  return UnitTest::RunAllTests(fixtureName.Chars());
}
