#include "stdafx.h"
#include "../src/API/niLang/Utils/CmdLine.h"

using namespace ni;

struct FCmdLine {
};

TEST_FIXTURE(FCmdLine,Params) {
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
