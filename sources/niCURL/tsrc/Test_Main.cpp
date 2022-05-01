#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>

int main(int argc, const char** argv) {
  ni::cString fixtureName;
  ni::ParseCommandLine(ni::GetCurrentOSProcessCmdLine(),&fixtureName);
  return UnitTest::RunAllTests(fixtureName.Chars());
}
