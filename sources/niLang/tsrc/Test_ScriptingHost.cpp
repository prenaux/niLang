#include "stdafx.h"
#include <niLang/IScriptingHost.h>
#include <niLang/IHString.h>
#include <niLang/IConcurrent.h>
#include <niLang/Utils/TimerSleep.h>
#include "../CppScriptingHost.h"

// Disabled RTCpp test by default
#if 0 && !defined niNoRTCpp

/*

  This test is a bit tricky. rtcpp_uptodate has to run first since it relies
  on the fact that the build system has built the module before. The other
  test will touch a source file which has the side effect of "run time"
  compiling the module and loading the new version.

  Its not ideal, but it does the job of testing the CppScriptingHost.

 */

using namespace ni;

_HDecl(TestRTCpp);

#define TEST_RTCPP_PATH(PATH) ("../../sources/Test_RTCpp/" PATH)

static tBool TouchFile(iFile* apFile) {
  return apFile ? apFile->SetTime(eFileTime_LastWrite,ni::GetLang()->GetCurrentTime()) : eFalse;
}

static tBool TouchFile(const achar* aPath) {
  Ptr<iFile> fp = ni::GetRootFS()->FileOpen(aPath,eFileOpenMode_Append);
  return TouchFile(fp);
}

static tBool _TouchRTCppModule() {
  ni::SleepMs(1100); // make sure our file's timestamp will be different from the previous build
  return TouchFile(TEST_RTCPP_PATH("Test_RTCpp_module.cpp"));
}

Ptr<iUnknown> EvalImpl(iHString* ahspContext, iHString* ahspCodeFile, const tUUID& aIID) {
  ni::Ptr<ni::iScriptingHost> host = ni::GetLang()->FindScriptingHost(
    ahspContext,ahspCodeFile);
  if (!host.IsOK()) {
    niError(niFmt("Can't find scripting host for code file '%s'.",ahspCodeFile));
    return NULL;
  }

  ni::Ptr<ni::iUnknown> ptrInst = host->EvalImpl(ahspContext,ahspCodeFile,aIID);
  if (!niIsOK(ptrInst)) {
    niError(niFmt("Can't create instance of '%s' from code file '%s'.",ahspContext,ahspCodeFile));
    return NULL;
  }

  return ptrInst;
};

struct FScriptingHostRTCpp {
  FScriptingHostRTCpp() {
  }
  ~FScriptingHostRTCpp() {
  }
};

TEST_FIXTURE(FScriptingHostRTCpp,uptodate) {
  // No Compile
  {
    const sRTCppStats& currStats = *RTCpp_GetStats();
    const sRTCppStats wasStats = currStats;
    CHECK_EQUAL(eFalse, RTCpp_GetCompileEnabled());
    QPtr<iRunnable> runGetTestTag = EvalImpl(_HC(TestRTCpp), _H("Test_RTCpp/Test_RTCpp_module.cpp"), niGetInterfaceUUID(iRunnable));
    CHECK_RETURN_IF_FAILED(runGetTestTag.IsOK());
    CHECK_EQUAL(_ASTR("Test_RTCpp_module"), VarGetString(runGetTestTag->Run()));
    CHECK_EQUAL(wasStats._numUpToDate, currStats._numUpToDate);
    CHECK_EQUAL(wasStats._numOutOfDate, currStats._numOutOfDate);
    CHECK_EQUAL(wasStats._numCompiled, currStats._numCompiled);
  }

  // Compile
  {
    ni::SetProperty("rtcpp.compile","1");
    const sRTCppStats& currStats = *RTCpp_GetStats();
    const sRTCppStats wasStats = currStats;
    CHECK_EQUAL(eTrue, RTCpp_GetCompileEnabled());
#ifdef _DEBUG
    CHECK_EQUAL(_ASTR("da"), RTCpp_GetCompileModuleType());
#else
    CHECK_EQUAL(_ASTR("ra"), RTCpp_GetCompileModuleType());
#endif
    QPtr<iRunnable> runGetTestTag = EvalImpl(_HC(TestRTCpp), _H("Test_RTCpp/Test_RTCpp_module.cpp"), niGetInterfaceUUID(iRunnable));
    CHECK_RETURN_IF_FAILED(runGetTestTag.IsOK());
    CHECK_EQUAL(_ASTR("Test_RTCpp_module"), VarGetString(runGetTestTag->Run()));
    CHECK_EQUAL(wasStats._numUpToDate+1, currStats._numUpToDate);
    CHECK_EQUAL(wasStats._numOutOfDate, currStats._numOutOfDate);
    CHECK_EQUAL(wasStats._numCompiled, currStats._numCompiled);
    ni::SetProperty("rtcpp.compile","0");
  }
}

TEST_FIXTURE(FScriptingHostRTCpp,touched) {
  // No Compile
  {
    CHECK(_TouchRTCppModule());
    const sRTCppStats& currStats = *RTCpp_GetStats();
    const sRTCppStats wasStats = currStats;
    CHECK_EQUAL(eFalse, RTCpp_GetCompileEnabled());
    QPtr<iRunnable> runGetTestTag = EvalImpl(_HC(TestRTCpp), _H("Test_RTCpp/Test_RTCpp_module.cpp"), niGetInterfaceUUID(iRunnable));
    CHECK(runGetTestTag.IsOK());
    // Shouldn't have change since compile isnt enabled
    CHECK_EQUAL(_ASTR("Test_RTCpp_module"), VarGetString(runGetTestTag->Run()));
    CHECK_EQUAL(wasStats._numUpToDate, currStats._numUpToDate);
    CHECK_EQUAL(wasStats._numOutOfDate, currStats._numOutOfDate);
    CHECK_EQUAL(wasStats._numCompiled, currStats._numCompiled);
    CHECK(_TouchRTCppModule());
  }

  // Compile
  {
    ni::SetProperty("rtcpp.compile","1");
    CHECK(_TouchRTCppModule());
    const sRTCppStats& currStats = *RTCpp_GetStats();
    const sRTCppStats wasStats = currStats;
    CHECK_EQUAL(eTrue, RTCpp_GetCompileEnabled());
#ifdef _DEBUG
    CHECK_EQUAL(_ASTR("da"), RTCpp_GetCompileModuleType());
#else
    CHECK_EQUAL(_ASTR("ra"), RTCpp_GetCompileModuleType());
#endif
    QPtr<iRunnable> runGetTestTag = EvalImpl(_HC(TestRTCpp), _H("Test_RTCpp/Test_RTCpp_module.cpp"), niGetInterfaceUUID(iRunnable));
    CHECK(runGetTestTag.IsOK());
    if (runGetTestTag.IsOK()) {
      CHECK_EQUAL(_ASTR("Test_RTCpp_rtcpp"), VarGetString(runGetTestTag->Run()));
      CHECK_EQUAL(wasStats._numUpToDate, currStats._numUpToDate);
      CHECK_EQUAL(wasStats._numOutOfDate+1, currStats._numOutOfDate);
      CHECK_EQUAL(wasStats._numCompiled+1, currStats._numCompiled);
    }
    CHECK(_TouchRTCppModule());
    ni::SetProperty("rtcpp.compile","0");
  }
}

#endif // niNoRTCpp
