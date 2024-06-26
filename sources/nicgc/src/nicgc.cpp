#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>
#include <niLang/Utils/CrashReport.h>
niCrashReport_DeclareHandler();

using namespace ni;

extern int legacy_main(const achar* aCmdLine);
extern void ErrorHelp(const achar* aszMsg);

//////////////////////////////////////////////////////////////////////////////////////////////
niConsoleMain()
{
  int r = ParseToolsCommandLine(
    GetCurrentOSProcessCmdLine(), '@',
    [&](const achar* aTool, const achar* aCmdLine, int argc, const achar** argv) -> int {
      TRACE_CMD_LINE(("TOOL: %s", aTool));
      TRACE_CMD_LINE(("CMDLINE: %s", aCmdLine));
      niLoop(i,argc) {
        TRACE_CMD_LINE(("ARGS[%d]: '%s'", i, argv[i]));
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
