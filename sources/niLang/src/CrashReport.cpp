// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/CrashReport.h"
#include "API/niLang/ILang.h"

#ifdef niWindows
  #include "API/niLang/Platforms/Win32/Win32_Redef.h"
  #define niCrashReportHasMinidump
namespace ni {
niExportFuncCPP(ni::cString) ni_generate_minidump(void* apExp);
}
  #define NI_SEH_EXCEPTION_PANIC (0xE000BEEF)
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
niExportFunc(void) ni_set_show_fatal_error_message_box(
  int aShowAssertMessageBox)
{
  _bShowFatalErrorMessageBox = aShowAssertMessageBox;
}
niExportFunc(int) ni_get_show_fatal_error_message_box()
{
  if (_bShowFatalErrorMessageBox == -1) {
    if (!ni::GetLang()->HasProperty("niLang.ShowFatalErrorMessageBox")) {
      // by default we dont show a message box on error
      _bShowFatalErrorMessageBox = 0;
    }
    else {
      _bShowFatalErrorMessageBox =
        ni::GetLang()->GetProperty("niLang.ShowFatalErrorMessageBox").Long();
    }
  }
  return _bShowFatalErrorMessageBox;
}

static int _bHarakiriOnPanic = -1;
niExportFunc(void) ni_set_panic_harakiri(int abHarakiriOnPanic)
{
  _bHarakiriOnPanic = abHarakiriOnPanic;
}
niExportFunc(int) ni_get_panic_harakiri()
{
  if (_bHarakiriOnPanic == -1) {
    if (!ni::GetLang()->HasProperty("niLang.HarakiriOnPanic")) {
      // by default we use the standard panic behavior for the platform
      _bHarakiriOnPanic = 0;
    }
    else {
      _bHarakiriOnPanic =
        ni::GetLang()->GetProperty("niLang.HarakiriOnPanic").Long();
    }
  }
  return _bHarakiriOnPanic;
}

struct sPanicDescription : public iPanicDescription {
  sPanicDescription(const iHString* aKind, const cString&& aDesc) noexcept
      : _kind(aKind)
      , _desc(astl::move(aDesc))
  {
    const_cast<iHString*>(_kind)->AddRef();
  }
  virtual ~sPanicDescription()
  {
    if (_kind) {
      const_cast<iHString*>(_kind)->Release();
    }
  }

  const iHString* __stdcall GetKind() const noexcept niImpl
  {
    return _kind;
  }

  virtual const cString& __stdcall GetDesc() const noexcept niImpl
  {
    return _desc;
  }

 private:
  const iHString* _kind;
  const cString _desc;

  sPanicDescription(const sPanicDescription& aRight) noexcept = delete;
  sPanicDescription(sPanicDescription&& aRight) noexcept = delete;
  sPanicDescription() noexcept = delete;
};

#ifdef niWindows
static inline const char* ni_windows_seh_get_excode_string(DWORD excode)
{
  switch (excode) {
  case EXCEPTION_ACCESS_VIOLATION: return "EXCEPTION_ACCESS_VIOLATION";
  case EXCEPTION_DATATYPE_MISALIGNMENT:
    return "EXCEPTION_DATATYPE_MISALIGNMENT";
  case EXCEPTION_BREAKPOINT: return "EXCEPTION_BREAKPOINT";
  case EXCEPTION_SINGLE_STEP: return "EXCEPTION_SINGLE_STEP";
  case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
  case EXCEPTION_FLT_DENORMAL_OPERAND: return "EXCEPTION_FLT_DENORMAL_OPERAND";
  case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
  case EXCEPTION_FLT_INEXACT_RESULT: return "EXCEPTION_FLT_INEXACT_RESULT";
  case EXCEPTION_FLT_INVALID_OPERATION:
    return "EXCEPTION_FLT_INVALID_OPERATION";
  case EXCEPTION_FLT_OVERFLOW: return "EXCEPTION_FLT_OVERFLOW";
  case EXCEPTION_FLT_STACK_CHECK: return "EXCEPTION_FLT_STACK_CHECK";
  case EXCEPTION_FLT_UNDERFLOW: return "EXCEPTION_FLT_UNDERFLOW";
  case EXCEPTION_INT_DIVIDE_BY_ZERO: return "EXCEPTION_INT_DIVIDE_BY_ZERO";
  case EXCEPTION_INT_OVERFLOW: return "EXCEPTION_INT_OVERFLOW";
  case EXCEPTION_PRIV_INSTRUCTION: return "EXCEPTION_PRIV_INSTRUCTION";
  case EXCEPTION_IN_PAGE_ERROR: return "EXCEPTION_IN_PAGE_ERROR";
  case EXCEPTION_ILLEGAL_INSTRUCTION: return "EXCEPTION_ILLEGAL_INSTRUCTION";
  case EXCEPTION_NONCONTINUABLE_EXCEPTION:
    return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
  case EXCEPTION_STACK_OVERFLOW: return "EXCEPTION_STACK_OVERFLOW";
  case EXCEPTION_INVALID_DISPOSITION: return "EXCEPTION_INVALID_DISPOSITION";
  case EXCEPTION_GUARD_PAGE: return "EXCEPTION_GUARD_PAGE";
  case EXCEPTION_INVALID_HANDLE: return "EXCEPTION_INVALID_HANDLE";
  case NI_SEH_EXCEPTION_PANIC: return "NI_SEH_EXCEPTION_PANIC";
  };
  return "EXCEPTION_UNKNOWN";
}

niExternC __ni_module_export LONG WINAPI
ni_windows_seh_unhandled_exception_filter(EXCEPTION_POINTERS* pExInfo)
{
  // Log the exception
  DWORD code = pExInfo->ExceptionRecord->ExceptionCode;
  cString msg = niFmt("ni_windows_seh_unhandled_exception_filter: 0x%08X: %s.",
                      code, ni_windows_seh_get_excode_string(code));
  niHarakiri(msg.c_str(), nullptr);
  return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#ifdef niUseWindowsSEHExceptions
thread_local sPanicDescription* _lastPanic = nullptr;

niExportFunc(tU32) ni_windows_seh_on_handle(tU32 aExcCode, void* aExcInfo)
{
  EXCEPTION_POINTERS* pExp = reinterpret_cast<EXCEPTION_POINTERS*>(aExcInfo);
  //niDebugFmt(("niWindows_HandleSEH: %p, %p", aExcCode, (tIntPtr)aExcInfo));
  if ((aExcCode == NI_SEH_EXCEPTION_PANIC) &&
      (pExp->ExceptionRecord->NumberParameters >= 1))
  {
    if (_lastPanic) {
      delete _lastPanic;
    }
    _lastPanic = reinterpret_cast<sPanicDescription*>(
      pExp->ExceptionRecord->ExceptionInformation[0]);
    return EXCEPTION_EXECUTE_HANDLER;
  }
  else {
    DWORD code = pExp->ExceptionRecord->ExceptionCode;
    cString msg =
      niFmt("ni_windows_seh_on_handle: UnhandledException 0x%08X: %s.", code,
            ni_windows_seh_get_excode_string(code));
    niHarakiri(msg.c_str(), nullptr);
    return EXCEPTION_EXECUTE_HANDLER;
  }
}

niExportFunc(iPanicDescription*) ni_windows_seh_get_last_panic()
{
  return _lastPanic;
}

#endif

static void _FormatThrowMessage(cString& fmt, const char* file, int line,
                                const char* func, const char* aPrefix,
                                const iHString* exceptionKind,
                                const char* exceptionMsg)
{
  const tBool hasMsg = niStringIsOK(exceptionMsg);
  ni_log_format_message(
    fmt, eLogFlags_Error | eLogFlags_NoLogTypePrefix, file, line, func,
    niFmt("%s: %s%s%s%s", aPrefix, exceptionKind, hasMsg ? _A(": ") : _A(""),
          hasMsg ? exceptionMsg : _A(""),
          hasMsg ? (exceptionMsg[StrSize(exceptionMsg) - 1] == '\n' ? _A("")
                                                                    : _A("\n"))
                 : _A("")),
    -1, -1);
  fmt.append("--- CALLSTACK ------------------\n");
  // dont skip any stack frame, cause we dont know what happens after code
  // optimization, its just more confusing than anything else.
  ni_stack_get_current(fmt, nullptr, 0);
}

#ifdef niJSCC
void JSCC_ConsoleError(const char* message)
{
  EM_ASM({ console.error(UTF8ToString($0)); }, message);
}
#endif

///////////////////////////////////////////////
niExportFuncCPP(void) ni_throw_panic(niConst struct iHString* aKind,
                                     const char* msg, const char* file,
                                     int line, const char* func)
{
#if defined niWindows
  if (::IsDebuggerPresent()) {
    ni_debug_break();
  }
#endif

  if (ni_get_panic_harakiri()) {
    ni_harakiri(aKind, msg, nullptr, file, line, func);
    return;
  }

#if defined niNoExceptions
  ni_harakiri(aKind, msg, nullptr, file, line, func);
#else
  cString fmt;
  fmt.append("================================\n");
  _FormatThrowMessage(fmt, file, line, func, "EXC PANIC", aKind, msg);
  fmt.append("================================\n");
  #ifdef niJSCC
  JSCC_ConsoleError(fmt.Chars());
  #else
  niError(fmt.Chars());
  #endif

  #ifdef niUseWindowsSEHExceptions
  sPanicDescription* pEx = new sPanicDescription{ aKind, std::move(fmt) };
  ULONG_PTR exceptionArgs[1] = { reinterpret_cast<ULONG_PTR>(pEx) };
  RaiseException(NI_SEH_EXCEPTION_PANIC, 0, 1, exceptionArgs);
  #else
  throw sPanicDescription{ aKind, std::move(fmt) };
  #endif

#endif
}

static tpfnHarakiriHandler _pfnHarakiriHandler = nullptr;

extern "C" void __ni_module_export
ni_set_harakiri_handler(tpfnHarakiriHandler apfnHarakiriHandler)
{
  _pfnHarakiriHandler = apfnHarakiriHandler;
}

extern "C" tpfnHarakiriHandler __ni_module_export ni_get_harakiri_handler()
{
  return _pfnHarakiriHandler;
}

extern "C" void __ni_module_export ni_harakiri(niConst iHString* aKind,
                                               niConst char* msg,
                                               void* apExcPtr,
                                               niConst char* file, int line,
                                               niConst char* func)
{
  if (_pfnHarakiriHandler) {
    _pfnHarakiriHandler(aKind, msg, apExcPtr, file, line, func);
  }
  else {
    cString fmt;
    fmt.append("================================\n");
    _FormatThrowMessage(fmt, file, line, func, "HARAKIRI", aKind, msg);
#ifdef niCrashReportHasMinidump
    fmt.append("--- MINIDUMP -------------------\n");
    fmt << ni_generate_minidump(apExcPtr);
    fmt << "\n";
#endif
    fmt.append("================================\n");
    ni::GetLang()->FatalError(fmt.Chars());
  }
}

extern "C" __ni_module_export void cpp_terminate_handler()
{
  niHarakiri("## cpp_terminate_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_purecall_handler()
{
  niHarakiri("## cpp_purecall_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_security_handler(int code, void* x)
{
  niHarakiri("## cpp_security_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_invalid_parameter_handler(
  const wchar_t* expression, const wchar_t* function, const wchar_t* file,
  unsigned int line, uintptr_t pReserved)
{
  niHarakiri("## cpp_invalid_parameter_handler ##", nullptr);
}
extern "C" __ni_module_export int cpp_new_handler(size_t)
{
  niHarakiri("##  cpp_new_handler ##", nullptr);
  return 0; // return 1 to retry alloc...
}
extern "C" __ni_module_export void cpp_sigfpe_handler(int /*code*/, int subcode)
{
  niHarakiri("## cpp_sigfpe_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_sigill_handler(int)
{
  niHarakiri("## cpp_sigill_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_sigsegv_handler(int)
{
  niHarakiri("## cpp_sigsegv_handler ##", nullptr);
}
extern "C" __ni_module_export void cpp_sigterm_handler(int)
{
  niHarakiri("## cpp_sigterm_handler ##", nullptr);
}

extern "C" __ni_module_export void cpp_sigint_handler(int)
{
#ifdef niWindows
  ::TerminateProcess(::GetCurrentProcess(), 444);
#else
  abort();
#endif
}

extern "C" __ni_module_export void cpp_sigabrt_handler(int)
{
#ifdef niWindows
  niHarakiri("## cpp_sigabrt_handler ##", nullptr);
#else
  // When Ctrl-C is pressed...
  exit(0x12345678);
#endif
}

} // namespace ni
