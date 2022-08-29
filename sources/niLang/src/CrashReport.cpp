// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/CrashReport.h"
#include "API/niLang/ILang.h"

#ifdef niWindows
#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#define niCrashReportHasMinidump
#endif

namespace ni {

///////////////////////////////////////////////
niExportFunc(void) ni_panic_assert(
    int expression,
    const char* exp,
    const char* file,
    int line,
    const char* func,
    const char* desc)
{
  if (!expression) {
    const tBool hasDesc = niStringIsOK(desc);
    cString fmt;
    ni_log_format_message(
      fmt,
      eLogFlags_Error|eLogFlags_NoLogTypePrefix,
      file,
      line,
      func,
      niFmt("PANIC ASSERT: %s%s%s%s",
            exp,
            hasDesc?_A(": "):_A(""),
            hasDesc?desc:_A(""),
            hasDesc?(desc[StrSize(desc)-1]=='\n'?_A(""):_A("\n")):_A("")),
      -1, -1);
    fmt.CatFormat("PANIC STACK:\n");
    ni_stack_get_current(fmt,NULL);
    niError(fmt.Chars());
#ifdef niWindows
    if (::IsDebuggerPresent())
#endif
    {
      ni_debug_break();
    }
    ni::GetLang()->Exit(0xDEADBEEF);
  }
}

}

#if !defined niNoCrashReport

using namespace ni;

static tpfnHarakiriHandler _pfnHarakiriHandler = NULL;

extern "C" void __ni_module_export ni_set_harakiri_handler(tpfnHarakiriHandler apfnHarakiriHandler) {
  _pfnHarakiriHandler = apfnHarakiriHandler;
}
extern "C" tpfnHarakiriHandler __ni_module_export ni_get_harakiri_handler() {
  return _pfnHarakiriHandler;
}
extern "C" void __ni_module_export ni_harakiri(const char* msg, void* apExp) {
  if (_pfnHarakiriHandler) {
    _pfnHarakiriHandler(msg,apExp);
  }
  else {
    cString o;
    o << "--- Harakiri ---\n";
    o << msg;
    if (!o.EndsWith("\n"))
      o << "\n";
#ifdef niCrashReportHasMinidump
    o << "--- Minidump ---\n";
    o << ni_generate_minidump(apExp);
    o << "\n";
#endif
    o << "--- Stack ---\n";
    ni_stack_get_current(o,apExp);
    ni::GetLang()->FatalError(o.Chars());
  }
}

extern "C" __ni_module_export void cpp_terminate_handler() {
  ni_harakiri("## cpp_terminate_handler ##", NULL);
}
extern "C" __ni_module_export void cpp_unexp_handler() {
  ni_harakiri("## cpp_unexp_handler ##", NULL);
}
extern "C" __ni_module_export void cpp_purecall_handler() {
  ni_harakiri("## cpp_purecall_handler ##", NULL);
}
extern "C" __ni_module_export void cpp_security_handler(int code, void *x) {
  ni_harakiri("## cpp_security_handler ##", NULL);
}
extern "C" __ni_module_export void cpp_invalid_parameter_handler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved) {
  ni_harakiri("## cpp_invalid_parameter_handler ##", NULL);
}
extern "C" __ni_module_export int cpp_new_handler(size_t) {
  ni_harakiri("##  cpp_new_handler ##", NULL);
  return 0; // return 1 to retry alloc...
}
extern "C" __ni_module_export void cpp_sigfpe_handler(int /*code*/, int subcode) {
  ni_harakiri("## cpp_sigfpe_handler ##", NULL);
}
extern "C" __ni_module_export void cpp_sigill_handler(int) {
  ni_harakiri("## cpp_sigill_handler ##", NULL);
}
extern "C" __ni_module_export void cpp_sigsegv_handler(int) {
  ni_harakiri("## cpp_sigsegv_handler ##", NULL);
}
extern "C" __ni_module_export void cpp_sigterm_handler(int) {
  ni_harakiri("## cpp_sigterm_handler ##", NULL);
}
extern "C" __ni_module_export void cpp_sigint_handler(int) {
#ifdef niWindows
  ::TerminateProcess(::GetCurrentProcess(),444);
#else
  abort();
#endif
}
extern "C" __ni_module_export void cpp_sigabrt_handler(int) {
#ifdef niWindows
  ni_harakiri("## cpp_sigabrt_handler ##", NULL);
#else
  // When Ctrl-C is pressed...
  exit(0x12345678);
#endif
}
#endif // #if !defined niNoCrashReport
