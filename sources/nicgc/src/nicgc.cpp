#include "stdafx.h"
#include <niLang/Utils/CrashReport.h>
#include <niLang/Utils/CmdLine.h>

using namespace ni;

extern int legacy_main(const achar* aCmdLine);
extern void ErrorHelp(const achar* aszMsg);

//////////////////////////////////////////////////////////////////////////////////////////////
niConsoleMain()
{
  niCrashReportModuleInstall();

  int r = ParseToolsCommandLine(
    GetCurrentOSProcessCmdLine(), '@',
    [&](const achar* aTool, const achar* aCmdLine, int argc, const achar** argv) -> int {
      TRACE_CMD_LINE(("D/TOOL: %s", aTool));
      TRACE_CMD_LINE(("D/CMDLINE: %s", aCmdLine));
      niLoop(i,argc) {
        TRACE_CMD_LINE(("D/ARGS[%d]: '%s'", i, argv[i]));
      }

      if (StrEq(aTool,"@default") || StrEq(aTool,"@legacy")) {
        return legacy_main(aCmdLine);
      }
      else {
        ErrorHelp(niFmt("Unknown shader compiler tool: '%s'.", aTool));
      }

      return 0;
    });
  if (r == eInvalidHandle) {
    ErrorHelp("No parameters.");
  }

  return r;
}
