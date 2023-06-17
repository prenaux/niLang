#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>
#include <niLang/Utils/CrashReport.h>

#if defined niEmbedded
// #define EMBEDDED_FIXTURE_NAME "FJSCC-OpenGL"
// #define EMBEDDED_FIXTURE_NAME "FFileSystem-ModuleDataDir"
// #define EMBEDDED_FIXTURE_NAME "FStartPath-OpenUrl"
#define EMBEDDED_FIXTURE_NAME "FProperties-BinDir"
#endif

#if !defined niNoProcess
extern bool Test_ChildProcess_Start(int argc, const char** argv, int& ret);
#endif

int main(int argc, const char** argv, const char** envp) {
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

    if (!envp) {
      printf("I/env: NONE\n");
    }
    else {
      printf("I/env:\n");
      for (const char **env = envp; *env != nullptr; env++) {
        printf("- %s\n", *env);
      }
      fflush(stdout);
    }

    ni::ni_log_system_info_once();

#ifdef EMBEDDED_FIXTURE_NAME
    fixtureName = ni::GetProperty("FIXTURE", EMBEDDED_FIXTURE_NAME);
#else
    ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(),&fixtureName);
#endif
    return UnitTest::RunAllTests(fixtureName.Chars());
  }
}
