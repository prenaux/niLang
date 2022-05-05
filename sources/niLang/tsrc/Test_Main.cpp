#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>
#include <niLang/Utils/CrashReport.h>

#if defined niEmbedded
// #define EMBEDDED_FIXTURE_NAME "FJSCC-OpenGL"
// #define EMBEDDED_FIXTURE_NAME "FFileSystem-ModuleDataDir"
#define EMBEDDED_FIXTURE_NAME "FStartPath-OpenUrl"
#endif

#if !defined niNoProcess
extern bool Test_ChildProcess_Start(int argc, const char** argv, int& ret);
#endif

int main(int argc, const char** argv) {
  int r = -1;
#if !defined niNoProcess
  if (Test_ChildProcess_Start(argc,argv,r)) {
    return r;
  }
  else
#endif
  {
    niCrashReport_DeclareHandler();
    ni::cString fixtureName;
    ni::ni_log_system_info_once();
#ifdef EMBEDDED_FIXTURE_NAME
    fixtureName = EMBEDDED_FIXTURE_NAME;
#else
    ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(),&fixtureName);
#endif
    return UnitTest::RunAllTests(fixtureName.Chars());
  }
}
