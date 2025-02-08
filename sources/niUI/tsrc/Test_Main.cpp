#include "stdafx.h"
#include <niUI/Utils/DebugUI.h>
#include <niLang/Utils/MainImpl.h>

niConsoleMain() {
  niAppLib_SetBuildText();
  niInitScriptVMForDebugUI();
  return UnitTest::TestAppNativeMainLoop("Test_niUI",argc,argv);
}
