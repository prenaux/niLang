#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>
#include <niLang/Utils/CrashReport.h>
#include <niLang/Utils/URLFileHandler.h>
niCrashReport_DeclareHandler();

using namespace ni;

#if defined niEmbedded
#define EMBEDDED_FIXTURE_NAME "FCppNiClient-Calculator"
#endif

const char* GetTestThriftServiceUrl() {
  static const ni::cString _serviceUrl = GetProperty(
    "url", "https://ham-test-thrift-master-dev-xywazuhpsq-as.a.run.app/");
  return _serviceUrl.c_str();
}

niConsoleMain() {
  ni::cString fixtureName;
#ifdef EMBEDDED_FIXTURE_NAME
  fixtureName = ni::GetProperty("FIXTURE", EMBEDDED_FIXTURE_NAME);
#else
  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(),&fixtureName);
#endif

  {
    niPrintln(niFmt("... TestThriftServiceUrl: %s", GetTestThriftServiceUrl()));

    cString moduleDataDir = GetModuleDataDir("niLang","Test_niThrift");
    niDebugFmt(("... moduleDataDir: '%s'", moduleDataDir));
    ni::RegisterModuleDataDirDefaultURLFileHandler("niLang","Test_niThrift");
  }

  return UnitTest::RunAllTests(fixtureName.Chars());
}
