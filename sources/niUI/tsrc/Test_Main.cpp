#include "stdafx.h"
#include <niLang/Utils/CrashReport.h>
#include <niUI/Utils/DebugUI.h>
niCrashReport_DeclareHandler();

niConsoleMain() {
  niAppLib_SetBuildText();
  niInitScriptVMForDebugUI();
  return UnitTest::TestAppNativeMainLoop("Test_niUI",argc,argv);
}
