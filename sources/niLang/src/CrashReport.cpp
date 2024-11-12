// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/CrashReport.h"
#include "API/niLang/ILang.h"

#ifdef niWindows
#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#define niCrashReportHasMinidump
#endif

#ifdef niJSCC
#include <emscripten.h>
#endif

namespace ni {

_HSymImpl(not_initialized);
_HSymImpl(panic);
_HSymImpl(unreachable);
_HSymImpl(debug_assert);
_HSymImpl(nullptr);
_HSymImpl(astl);
_HSymImpl(invalid_cast);
_HSymImpl(harakiri);

static int _bShowFatalErrorMessageBox = -1;
niExportFunc(void) ni_set_show_fatal_error_message_box(int aShowAssertMessageBox) {
  _bShowFatalErrorMessageBox = aShowAssertMessageBox;
}
niExportFunc(int) ni_get_show_fatal_error_message_box() {
  if (_bShowFatalErrorMessageBox == -1) {
    if (!ni::GetLang()->HasProperty("niLang.ShowFatalErrorMessageBox")) {
      _bShowFatalErrorMessageBox = 0;
    }
    else {
      _bShowFatalErrorMessageBox = ni::GetLang()->GetProperty("niLang.ShowAssertMessageBox").Long();
    }
  }
  return _bShowFatalErrorMessageBox;
}

struct __ni_module_export sPanicException : public iPanicException {
  sPanicException(const iHString* aKind, const cString&& aDesc) noexcept
      : _kind(aKind), _desc(astl::move(aDesc))
  {
    const_cast<iHString*>(_kind)->AddRef();
  }
  virtual ~sPanicException() {
    if (_kind) {
      const_cast<iHString*>(_kind)->Release();
    }
  }

  const char* what() const noexcept niImpl {
    return _desc.c_str();
  }

  const iHString* __stdcall GetKind() const noexcept niImpl {
    return _kind;
  }

  virtual const cString& GetDesc() const noexcept niImpl {
    return _desc;
  }

private:
  const iHString* _kind;
  const cString _desc;

  sPanicException(const sPanicException& aRight) noexcept = delete;
  sPanicException(sPanicException&& aRight) noexcept = delete;
  sPanicException() noexcept = delete;
};

static void _FormatThrowMessage(
  cString& fmt,
  const char* file,
  int line,
  const char* func,
  const char* aPrefix,
  const iHString* exceptionKind,
  const char* exceptionMsg)
{
  const tBool hasMsg = niStringIsOK(exceptionMsg);
  ni_log_format_message(
    fmt,
    eLogFlags_Error|eLogFlags_NoLogTypePrefix,
    file,
    line,
    func,
    niFmt("%s: %s%s%s%s",
          aPrefix,
          exceptionKind,
          hasMsg?_A(": "):_A(""),
          hasMsg?exceptionMsg:_A(""),
          hasMsg?(exceptionMsg[StrSize(exceptionMsg)-1]=='\n'?_A(""):_A("\n")):_A("")),
    -1, -1);
  fmt.append("--- CALLSTACK ------------------\n");
  ni_stack_get_current(fmt,nullptr,2); // 1 to skip _FormatThrowMessage and its caller
}

#ifdef niJSCC
void JSCC_ConsoleError(const char* message) {
  EM_ASM({
    console.error(UTF8ToString($0));
  }, message);
}
#endif

///////////////////////////////////////////////
niExportFuncCPP(void) ni_throw_panic(
  niConst struct iHString* aKind,
  const char* msg,
  const char* file,
  int line,
  const char* func)
{
#if defined niNoExceptions
  ni_harakiri(aKind,msg,nullptr,file,line,func);
#else
  cString fmt;
  fmt.append("================================\n");
  _FormatThrowMessage(
    fmt,
    file, line, func,
    "EXC PANIC",
    aKind,
    msg);
  fmt.append("================================\n");
  #ifdef niJSCC
  JSCC_ConsoleError(fmt.Chars());
  #else
  niError(fmt.Chars());
  #endif
  throw sPanicException{aKind,std::move(fmt)};
#endif
}

static tpfnHarakiriHandler _pfnHarakiriHandler = nullptr;

extern "C" void __ni_module_export ni_set_harakiri_handler(tpfnHarakiriHandler apfnHarakiriHandler) {
  _pfnHarakiriHandler = apfnHarakiriHandler;
}

extern "C" tpfnHarakiriHandler __ni_module_export ni_get_harakiri_handler() {
  return _pfnHarakiriHandler;
}

extern "C" void __ni_module_export ni_harakiri(niConst iHString* aKind, niConst char* msg, void* apExcPtr, niConst char* file, int line, niConst char* func) {
  if (_pfnHarakiriHandler) {
    _pfnHarakiriHandler(aKind, msg, apExcPtr, file, line, func);
  }
  else {
    cString fmt;
    fmt.append("================================\n");
    _FormatThrowMessage(
      fmt,
      file, line, func,
      "HARAKIRI",
      aKind,
      msg);
#ifdef niCrashReportHasMinidump
    fmt.append("--- MINIDUMP -------------------\n");
    fmt << ni_generate_minidump(apExcPtr);
    fmt << "\n";
#endif
    fmt.append("================================\n");
    ni::GetLang()->FatalError(fmt.Chars());
  }
}

extern "C" __ni_module_export void cpp_terminate_handler() {
  niHarakiri("## cpp_terminate_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_purecall_handler() {
  niHarakiri("## cpp_purecall_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_security_handler(int code, void *x) {
  niHarakiri("## cpp_security_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_invalid_parameter_handler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved) {
  niHarakiri("## cpp_invalid_parameter_handler ##", nullptr);
}
extern "C" __ni_module_export int cpp_new_handler(size_t) {
  niHarakiri("##  cpp_new_handler ##", nullptr);
  return 0; // return 1 to retry alloc...
}
extern "C" __ni_module_export void cpp_sigfpe_handler(int /*code*/, int subcode) {
  niHarakiri("## cpp_sigfpe_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_sigill_handler(int) {
  niHarakiri("## cpp_sigill_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_sigsegv_handler(int) {
  niHarakiri("## cpp_sigsegv_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_sigterm_handler(int) {
  niHarakiri("## cpp_sigterm_handler ##", nullptr);
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
  niHarakiri("## cpp_sigabrt_handler ##", nullptr);
#else
  // When Ctrl-C is pressed...
  exit(0x12345678);
#endif
}

}
