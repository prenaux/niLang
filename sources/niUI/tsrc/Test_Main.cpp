#include "stdafx.h"
#include <niLang/Utils/CrashReport.h>
niCrashReport_DeclareHandler();

niConsoleMain() {
  niAppLib_SetBuildText();
  return UnitTest::TestAppNativeMainLoop("Test_niUI",argc,argv);
}
