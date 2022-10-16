#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>
#include <niLang/Utils/CrashReport.h>
niCrashReport_DeclareHandler();

#if defined niEmbedded
// http://localhost:8123/niLang/bin/web-js/Test_niCURL_ra.html?FIXTURE=FCURLFetch-Post
#define EMBEDDED_FIXTURE_NAME "FCURLFetch-Get"
#endif

niConsoleMain() {
  ni::cString fixtureName;
#ifdef EMBEDDED_FIXTURE_NAME
  fixtureName = ni::GetProperty("FIXTURE", EMBEDDED_FIXTURE_NAME);
#else
  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(),&fixtureName);
#endif
  return UnitTest::RunAllTests(fixtureName.Chars());
}
