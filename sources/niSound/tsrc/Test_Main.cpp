#include "stdafx.h"
#include <niAppLib.h>
#include <niLang/Utils/URLFileHandler.h>
#include <niLang/Utils/CrashReport.h>
niCrashReport_DeclareHandler();

niConsoleMain() {
  niAppLib_SetBuildText();
  ni::RegisterModuleDataDirDefaultURLFileHandler("niLang","Test_niSound");
  return UnitTest::TestAppNativeMainLoop("Test_niSound",argc,argv);
}
