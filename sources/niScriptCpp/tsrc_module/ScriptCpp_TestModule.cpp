#include <niScriptCpp/Utils/ScriptCppImpl.h>
#include <niLang/Utils/ConcurrentImpl.h>

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

SCRIPTCPP_EXPORT_FUNC(TestScriptCpp,ScriptCpp_TestModule,CreateTestModule);
