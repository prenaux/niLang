// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/CrashReport.h"

#if !defined niNoCrashReport
#include "API/niLang/ILang.h"

#ifdef niWindows
#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#define HAS_MINIDUMP
#endif

using namespace ni;

static tpfnHarakiriHandler _pfnHarakiriHandler = NULL;

#ifdef HAS_MINIDUMP
niExportFuncCPP(cString) ni_generate_minidump(void* apExp);
#endif

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
#ifdef HAS_MINIDUMP
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
