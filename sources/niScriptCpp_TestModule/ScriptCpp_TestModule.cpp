#include <niScriptCpp/Utils/ScriptCppImpl.h>
#include <niLang/Utils/ConcurrentImpl.h>
// Note: Here to test build dependencies, not needed for the code
#include <niScriptCpp_ModuleDef.h>

using namespace ni;

Ptr<iRunnable> CreateTestModule() {
  return ni::Runnable([&]() {
#ifdef _RTCPP
    // When compiling a scriptcpp module, so when dynamic compiling
    return "Test_ScriptCpp_rtcpp";
#else
    // Regular compilation
    return "Test_ScriptCpp_module";
#endif
  });
}

niScriptCppExportFunc(TestScriptCpp,ScriptCpp_TestModule,CreateTestModule);
