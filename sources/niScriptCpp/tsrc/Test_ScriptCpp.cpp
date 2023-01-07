#include "stdafx.h"
#include <niLang/IScriptingHost.h>
#include <niLang/IHString.h>
#include <niLang/IConcurrent.h>
#include <niLang/Utils/TimerSleep.h>
#include <niLang/STL/run_once.h>
#include <niScriptCpp/ScriptCpp.h>
#include <niScriptCpp_ModuleDef.h>

#if !defined niNoScriptCpp

/*

  This test is a bit tricky. rtcpp_uptodate has to run first since it relies
  on the fact that the build system has built the module before. The other
  test will touch a source file which has the side effect of "run time"
  compiling the module and loading the new version.

  Its not ideal, but it does the job of testing the CppScriptingHost.

 */

using namespace ni;

_HDecl(TestScriptCpp);

#define TEST_SCRIPTCPP_PATH(PATH) ("../../sources/niScriptCpp/tsrc_module/" PATH)

static tBool TouchFile(iFile* apFile) {
  return apFile ? apFile->SetTime(eFileTime_LastWrite,ni::GetLang()->GetCurrentTime()) : eFalse;
}

static tBool TouchFile(const achar* aPath) {
  Ptr<iFile> fp = ni::GetRootFS()->FileOpen(aPath,eFileOpenMode_Append);
  return TouchFile(fp);
}

static tBool _TouchScriptCppModule() {
  ni::SleepMs(1100); // make sure our file's timestamp will be different from the previous build
  return TouchFile(TEST_SCRIPTCPP_PATH("ScriptCpp_TestModule.cpp"));
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

struct FScriptCpp {
  FScriptCpp() {
    niRunOnce {
      QPtr<iScriptingHost> ptrScriptingHost = niCreateInstance(niScriptCpp,ScriptingHost,NULL,NULL);
      ni::GetLang()->AddScriptingHost(_H("cpp"),ptrScriptingHost.ptr());
      ni::GetLang()->AddScriptingHost(_H("cpp2"),ptrScriptingHost.ptr());
      ni::GetLang()->AddScriptingHost(_H("cni"),ptrScriptingHost.ptr());
    };
  }
  ~FScriptCpp() {
  }
};

TEST_FIXTURE(FScriptCpp,uptodate_compile) {
  ni::SetProperty(SCRIPTCPP_COMPILE_PROPERTY,"1");
  const sScriptCppStats& currStats = *ScriptCpp_GetStats();
  const sScriptCppStats wasStats = currStats;
  CHECK_EQUAL(eTrue, ScriptCpp_GetCompileEnabled());
#ifdef _DEBUG
  CHECK_EQUAL(_ASTR("da"), ScriptCpp_GetCompileModuleType());
#else
  CHECK_EQUAL(_ASTR("ra"), ScriptCpp_GetCompileModuleType());
#endif
  QPtr<iRunnable> runGetTestTag = EvalImpl(
    _HC(TestScriptCpp),
    _H("Test_niScriptCpp_Module#ScriptCpp_TestModule#niScriptCpp/tsrc_module/ScriptCpp_TestModule.cpp"),
    niGetInterfaceUUID(iRunnable));
  CHECK_RETURN_IF_FAILED(runGetTestTag.IsOK());
  CHECK_EQUAL(_ASTR("Test_ScriptCpp_module"), VarGetString(runGetTestTag->Run()));
  CHECK_EQUAL(wasStats._numUpToDate+1, currStats._numUpToDate);
  CHECK_EQUAL(wasStats._numOutOfDate, currStats._numOutOfDate);
  CHECK_EQUAL(wasStats._numCompiled, currStats._numCompiled);
  ni::SetProperty(SCRIPTCPP_COMPILE_PROPERTY,"0");
}

// XXX: !!! Note that 'FScriptCpp,uptodate_compile' needs to run first since
// this test DOES NOT compile the code it expects that the test module has
// been compiled as a module first.
TEST_FIXTURE(FScriptCpp,uptodate_nocompile) {
  ni::SetProperty(SCRIPTCPP_COMPILE_PROPERTY,"0");
  const sScriptCppStats& currStats = *ScriptCpp_GetStats();
  const sScriptCppStats wasStats = currStats;
  CHECK_EQUAL(eFalse, ScriptCpp_GetCompileEnabled());
  QPtr<iRunnable> runGetTestTag = EvalImpl(
    _HC(TestScriptCpp),
    _H("Test_niScriptCpp_Module#niScriptCpp/tsrc_module/ScriptCpp_TestModule.cpp"),
    niGetInterfaceUUID(iRunnable));
  CHECK_RETURN_IF_FAILED(runGetTestTag.IsOK());
  CHECK_EQUAL(_ASTR("Test_ScriptCpp_module"), VarGetString(runGetTestTag->Run()));
  CHECK_EQUAL(wasStats._numUpToDate, currStats._numUpToDate);
  CHECK_EQUAL(wasStats._numOutOfDate, currStats._numOutOfDate);
  CHECK_EQUAL(wasStats._numCompiled, currStats._numCompiled);
}

TEST_FIXTURE(FScriptCpp,uptodate_invalid_resource_def) {
  ni::SetProperty(SCRIPTCPP_COMPILE_PROPERTY,"0");
  const sScriptCppStats& currStats = *ScriptCpp_GetStats();
  const sScriptCppStats wasStats = currStats;
  CHECK_EQUAL(eFalse, ScriptCpp_GetCompileEnabled());
#ifdef _DEBUG
  CHECK_EQUAL(_ASTR("da"), ScriptCpp_GetCompileModuleType());
#else
  CHECK_EQUAL(_ASTR("ra"), ScriptCpp_GetCompileModuleType());
#endif
  QPtr<iRunnable> runGetTestTag = EvalImpl(
    _HC(TestScriptCpp),
    _H("Foo#Bar#Test_niScriptCpp_Module#ScriptCpp_TestModule#niScriptCpp/tsrc_module/ScriptCpp_TestModule.cpp"),
    niGetInterfaceUUID(iRunnable));
  CHECK_EQUAL(eFalse,runGetTestTag.IsOK());
  CHECK_EQUAL(wasStats._numUpToDate, currStats._numUpToDate);
  CHECK_EQUAL(wasStats._numOutOfDate, currStats._numOutOfDate);
  CHECK_EQUAL(wasStats._numCompiled, currStats._numCompiled);
}

TEST_FIXTURE(FScriptCpp,touched) {
  // No Compile
  {
    CHECK(_TouchScriptCppModule());
    const sScriptCppStats& currStats = *ScriptCpp_GetStats();
    const sScriptCppStats wasStats = currStats;
    CHECK_EQUAL(eFalse, ScriptCpp_GetCompileEnabled());
    QPtr<iRunnable> runGetTestTag = EvalImpl(
      _HC(TestScriptCpp),
      _H("Test_niScriptCpp_Module#niScriptCpp/tsrc_module/ScriptCpp_TestModule.cpp"),
      niGetInterfaceUUID(iRunnable));
    CHECK(runGetTestTag.IsOK());
    // Shouldn't have change since compile isnt enabled
    CHECK_EQUAL(_ASTR("Test_ScriptCpp_module"), VarGetString(runGetTestTag->Run()));
    CHECK_EQUAL(wasStats._numUpToDate, currStats._numUpToDate);
    CHECK_EQUAL(wasStats._numOutOfDate, currStats._numOutOfDate);
    CHECK_EQUAL(wasStats._numCompiled, currStats._numCompiled);
    CHECK(_TouchScriptCppModule());
  }

  // Compile
  {
    ni::SetProperty(SCRIPTCPP_COMPILE_PROPERTY,"1");
    CHECK(_TouchScriptCppModule());
    const sScriptCppStats& currStats = *ScriptCpp_GetStats();
    const sScriptCppStats wasStats = currStats;
    CHECK_EQUAL(eTrue, ScriptCpp_GetCompileEnabled());
#ifdef _DEBUG
    CHECK_EQUAL(_ASTR("da"), ScriptCpp_GetCompileModuleType());
#else
    CHECK_EQUAL(_ASTR("ra"), ScriptCpp_GetCompileModuleType());
#endif
    QPtr<iRunnable> runGetTestTag = EvalImpl(
      _HC(TestScriptCpp),
      _H("Test_niScriptCpp_Module#niScriptCpp/tsrc_module/ScriptCpp_TestModule.cpp"),
      niGetInterfaceUUID(iRunnable));
    CHECK(runGetTestTag.IsOK());
    if (runGetTestTag.IsOK()) {
      CHECK_EQUAL(_ASTR("Test_ScriptCpp_rtcpp"), VarGetString(runGetTestTag->Run()));
      CHECK_EQUAL(wasStats._numUpToDate, currStats._numUpToDate);
      CHECK_EQUAL(wasStats._numOutOfDate+1, currStats._numOutOfDate);
      CHECK_EQUAL(wasStats._numCompiled+1, currStats._numCompiled);
    }
    CHECK(_TouchScriptCppModule());
    ni::SetProperty(SCRIPTCPP_COMPILE_PROPERTY,"0");
  }
}

#endif // niNoScriptCpp
