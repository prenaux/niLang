#include "stdafx.h"
#include <niLang/Utils/CmdLine.h>

using namespace ni;
using namespace astl;

struct FCmdLine {
};

TEST_FIXTURE(FCmdLine,ParamBool) {
  {
    Nonnull<tStringCVec> files{tStringCVec::Create()};
    CHECK_EQUAL(eTrue, ParseCommandLine(
      "myapp --foo --no-bar -Dsquirrel -Dno-fagiano --alpha='aaa' -X --beta='BBB' -bla=narf -count=123",
      tCmdLineParseArgFn{[](const char* aProperty, const char* aValue, tBool aIsShorthand) {
        if (aIsShorthand) {
          niDebugFmt(("... ParsedShorthand: '%s' = '%s'", aProperty, aValue));
        }
        else {
          niDebugFmt(("... ParsedProperty: '%s' = '%s'", aProperty, aValue));
        }
      }},
      tCmdLineParseFileFn{[&](const cString& aFile) { files->push_back(aFile); }}));
    CHECK(files->empty());
  }

  {
    Nonnull<tStringCVec> files{tStringCVec::Create()};
    CHECK_EQUAL(eTrue, ParseCommandLine(
      "myapp --foo --no-bar -Dsquirrel -Dno-fagiano --alpha='aaa' -X --beta='BBB' -bla=narf -count=123 afile --bfile",
      tCmdLineParseArgFn{[](const char* aProperty, const char* aValue, tBool aIsShorthand) {
        if (aIsShorthand) {
          niDebugFmt(("... ParsedShorthand: '%s' = '%s'", aProperty, aValue));
        }
        else {
          niDebugFmt(("... ParsedProperty: '%s' = '%s'", aProperty, aValue));
        }
      }},
      tCmdLineParseFileFn{[&](const cString& aFile) { files->push_back(aFile); }}));
    CHECK_EQUAL(2, files->size());
    CHECK_EQUAL(cString{"afile"}, at_default(*files,0,"invalid-index"));
    CHECK_EQUAL(cString{"--bfile"}, at_default(*files,1,"invalid-index"));
  }
}

TEST_FIXTURE(FCmdLine,DashDashFiles) {
  Nonnull<tStringCVec> files{tStringCVec::Create()};
  CHECK_EQUAL(eTrue, ParseCommandLine(
    "myapp -- -afile --bfile",
    tCmdLineParseArgFn{[](const char* aProperty, const char* aValue, tBool aIsShorthand) {
      if (aIsShorthand) {
        niDebugFmt(("... ParsedShorthand: '%s' = '%s'", aProperty, aValue));
      }
      else {
        niDebugFmt(("... ParsedProperty: '%s' = '%s'", aProperty, aValue));
      }
    }},
    tCmdLineParseFileFn{[&](const cString& aFile) { files->push_back(aFile); }}));
  CHECK_EQUAL(2, files->size());
  CHECK_EQUAL(cString{"-afile"}, at_default(*files,0,"invalid-index"));
  CHECK_EQUAL(cString{"--bfile"}, at_default(*files,1,"invalid-index"));
}

TEST_FIXTURE(FCmdLine,OtherAndUnknownParams) {
  {
    cString file;
    Ptr<tStringCVec> otherFiles = tStringCVec::Create();
    Ptr<tStringCVec> unknownParams = tStringCVec::Create();
    CHECK_EQUAL(eTrue, ParseCommandLine("/Applications/niApp.app/Contents/MacOS/niApp/bin/osx-x64/niApp_ra",
                                        &file, otherFiles, unknownParams));
    CHECK(file.empty());
    CHECK(otherFiles->empty());
    CHECK(unknownParams->empty());
  }

  {
    cString file;
    Ptr<tStringCVec> otherFiles = tStringCVec::Create();
    Ptr<tStringCVec> unknownParams = tStringCVec::Create();
    CHECK_EQUAL(eTrue, ParseCommandLine("/Applications/niApp.app/Contents/MacOS/niApp/bin/osx-x64/niApp_ra some.vpk",
                                        &file, otherFiles, unknownParams));
    CHECK_EQUAL(_ASTR("some.vpk"), file);
    CHECK(otherFiles->empty());
    CHECK(unknownParams->empty());
  }

  {
    cString file;
    Ptr<tStringCVec> otherFiles = tStringCVec::Create();
    Ptr<tStringCVec> unknownParams = tStringCVec::Create();
    CHECK_EQUAL(eTrue, ParseCommandLine("/Applications/niApp.app/Contents/MacOS/niApp/bin/osx-x64/niApp_ra -psn_0_4486215",
                                        &file, otherFiles, unknownParams));
    CHECK(file.empty());
    CHECK(otherFiles->empty());
    CHECK_EQUAL(1, unknownParams->size());
    CHECK_EQUAL(_ASTR("psn_0_4486215"), unknownParams->at(0));
  }

  {
    cString file;
    Ptr<tStringCVec> otherFiles = tStringCVec::Create();
    Ptr<tStringCVec> unknownParams = tStringCVec::Create();
    CHECK_EQUAL(eTrue, ParseCommandLine("/Applications/niApp.app/Contents/MacOS/niApp/bin/osx-x64/niApp_ra -psn_0_4486215 -bla foo.vpk alpha beta gamma",
                                        &file, otherFiles, unknownParams));
    CHECK_EQUAL(_ASTR("foo.vpk"), file);
    CHECK_EQUAL(3, otherFiles->size());
    CHECK_EQUAL(_ASTR("alpha"), otherFiles->at(0));
    CHECK_EQUAL(_ASTR("beta"), otherFiles->at(1));
    CHECK_EQUAL(_ASTR("gamma"), otherFiles->at(2));
    CHECK_EQUAL(2, unknownParams->size());
    CHECK_EQUAL(_ASTR("psn_0_4486215"), unknownParams->at(0));
    CHECK_EQUAL(_ASTR("bla"), unknownParams->at(1));
  }

  {
    CHECK_EQUAL(eTrue, ParseCommandLine("/Applications/niApp.app/Contents/MacOS/niApp/bin/osx-x64/niApp_ra -psn_0_4486215 -bla foo.vpk alpha beta gamma",
                                        NULL, NULL, NULL));
  }
}
