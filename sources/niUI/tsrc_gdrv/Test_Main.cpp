//
// ham Run_Test_niUI_GDRV FIXTURE=FGDRV,Clear
//
#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>
#include <niLang/Utils/MainImpl.h>

niConsoleMain()
{
  ni::cString fixtureName = ni::GetProperty("FIXTURE", "");
  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(), &fixtureName);
  return UnitTest::RunAllTests(fixtureName.Chars());
}
