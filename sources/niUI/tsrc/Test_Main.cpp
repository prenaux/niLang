#include "stdafx.h"

niConsoleMain() {
  niAppLib_SetBuildText();
  if (false) {
    const char* fixtureName = NULL;
    if (argc > 1) {
      fixtureName = argv[1];
    }
    return UnitTest::RunAllTests(fixtureName);
  }
  else {
    return UnitTest::TestAppNativeMainLoop("Test_niUI",argc,argv);
  }
}
