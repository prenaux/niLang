#include <niLang/Types.h>
#include <niLang/Utils/RTCppImpl.h>
#include <niLang/Utils/ConcurrentImpl.h>

using namespace ni;

#define RTCPP_DECLARE_NEW(CATEGORY,CLASS)                 \
  niExportFunc(ni::iUnknown*) New_##CATEGORY##_##CLASS()

RTCPP_DECLARE_NEW(TestRTCpp,Test_RTCpp_module) {
  return ni::Runnable([&]() {
#ifdef _RTCPP
    return "Test_RTCpp_rtcpp";
#else
    return "Test_RTCpp_module";
#endif
  }).GetRawAndSetNull();
}
