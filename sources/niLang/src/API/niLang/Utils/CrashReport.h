#ifndef __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
#define __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../StringDef.h"
#include "../STL/type_traits.h"
#include "../STL/exception.h"

#if defined __cplusplus
  #include <exception> // for std::set_terminate
#endif

#ifdef niWindows
  // Unfortunately we have no choice to get SetUnhandledExceptionFilter as
  // forward declaring it sanely is almost impossible to do a in a robust way.
  #include "../Platforms/Win32/Win32_Redef.h"
  #include <signal.h>
  #define niCrashReportHasMinidump
niExternC __ni_module_export LONG WINAPI
ni_windows_seh_unhandled_exception_filter(EXCEPTION_POINTERS* pExInfo);
  #define niUseWindowsSEHExceptions
#endif

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

niExportFuncCPP(ni::cString&) ni_stack_get_current(ni::cString& aOutput,
                                                   void* apExp, int skip);

#if !defined __cplusplus

  #define niCrashReport_DeclareHandler()
  #define niCrashReport_ModuleInstall()

#else

niExportFunc(void) cpp_terminate_handler();
niExportFunc(void) cpp_purecall_handler();
niExportFunc(void) cpp_security_handler(int code, void* x);
niExportFunc(void) cpp_invalid_parameter_handler(const wchar_t* expression,
                                                 const wchar_t* function,
                                                 const wchar_t* file,
                                                 unsigned int line,
                                                 uintptr_t pReserved);
niExportFunc(int) cpp_new_handler(size_t);
niExportFunc(void) cpp_sigabrt_handler(int);
niExportFunc(void) cpp_sigfpe_handler(int /*code*/, int subcode);
niExportFunc(void) cpp_sigill_handler(int);
niExportFunc(void) cpp_sigint_handler(int);
niExportFunc(void) cpp_sigsegv_handler(int);
niExportFunc(void) cpp_sigterm_handler(int);

//! Install the handlers into a specified module, this should be called
//! for each module that uses its own CRT.
static inline void __niCrashReportModuleInstall()
{
  // niPrintln("__niCrashReportModuleInstall");
  std::set_terminate(cpp_terminate_handler);

  #ifdef niWindows
  SetUnhandledExceptionFilter(ni_windows_seh_unhandled_exception_filter);
  _set_error_mode(_OUT_TO_STDERR);
  #endif

  #if defined _MSC_VER
    #if _MSC_VER >= 1300
  // Pure Virtual call handler
  _set_purecall_handler(cpp_purecall_handler);

  // Catch new operator memory allocation exceptions
  _set_new_mode(1); // Force malloc() to call new handler too
  _set_new_handler(cpp_new_handler);
    #endif

    #if _MSC_VER >= 1400
  // Catch invalid parameter exceptions.
  _set_invalid_parameter_handler(cpp_invalid_parameter_handler);
    #endif

    #if _MSC_VER >= 1300 && _MSC_VER < 1400
  // Catch buffer overrun exceptions
  // The _set_security_error_handler is deprecated in VC8 C++ run time library
  _set_security_error_handler(cpp_security_handler);
    #endif

      // Set up C++ signal handlers
    #if _MSC_VER >= 1400
  _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
    #endif
  #endif

  // Catch an abnormal program termination
  signal(SIGABRT, cpp_sigabrt_handler);

  // Catch illegal instruction handler
  signal(SIGINT, cpp_sigint_handler);

  // Catch a termination request
  signal(SIGTERM, cpp_sigterm_handler);

  #if !defined niWindows
  // Catch a sigill request
  signal(SIGILL, cpp_sigill_handler);

  // Catch a sigsegv request
  signal(SIGSEGV, cpp_sigsegv_handler);
  #endif
}

//! Uninstall the handlers into a specified module, this should be called
//! for each module that uses its own CRT.
static inline void __niCrashReportModuleUninstall()
{
    // niPrintln("__niCrashReportModuleUninstall");

  #if defined _MSC_VER
    #if _MSC_VER >= 1300
  // Pure Virtual call handler
  _set_purecall_handler(NULL);

  // Catch new operator memory allocation exceptions
  _set_new_mode(1); // Force malloc() to call new handler too
  _set_new_handler(NULL);
    #endif

    #if _MSC_VER >= 1400
  // Catch invalid parameter exceptions.
  _set_invalid_parameter_handler(NULL);
    #endif

    #if _MSC_VER >= 1300 && _MSC_VER < 1400
  // Catch buffer overrun exceptions
  // The _set_security_error_handler is deprecated in VC8 C++ run time library
  _set_security_error_handler(NULL);
    #endif

      // Set up C++ signal handlers
    #if _MSC_VER >= 1400
  _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
    #endif
  #endif

  // Catch an abnormal program termination
  signal(SIGABRT, NULL);

  // Catch illegal instruction handler
  signal(SIGINT, NULL);

  // Catch a termination request
  signal(SIGTERM, NULL);

  #if !defined niWindows
  // Catch a sigill request
  signal(SIGILL, NULL);
  // Catch a sigsegv request
  signal(SIGSEGV, NULL);
  #endif
}

struct sNiCrashReport {
  sNiCrashReport()
  {
    __niCrashReportModuleInstall();
  }
  ~sNiCrashReport()
  {
    __niCrashReportModuleUninstall();
  }
};

  // Should be declared above your main() function
  #define niCrashReport_DeclareHandler() ni::sNiCrashReport _niCrashReport;

  #ifdef niWindows
    // This should only be necessary on Windows since on other platforms the
    // standard C library is shared between the modules. On Windows DLLs can have
    // a different CRT and thus set of signal table and handlers.
    #define niCrashReport_ModuleInstall() ni::__niCrashReportModuleInstall()
  #else
    #define niCrashReport_ModuleInstall()
  #endif

struct __ni_module_export sPanicException : public astl::exception {
  sPanicException(const iHString* aKind, cString&& aDesc) noexcept;
  virtual ~sPanicException();

  const iHString* GetKind() const noexcept;
  const cString& GetDesc() const noexcept;

  // Implement std::exception::what()
  const char* what() const noexcept override;

 private:
  const iHString* _kind;
  const cString _desc;

  sPanicException(const sPanicException& aRight) noexcept = delete;
  sPanicException(sPanicException&& aRight) noexcept = delete;
  sPanicException() noexcept = delete;
};

  #ifdef niUseWindowsSEHExceptions
extern "C" void* __cdecl _exception_info(void);
    #pragma intrinsic(_exception_info)

extern "C" unsigned long __cdecl _exception_code(void);
    #pragma intrinsic(_exception_code)

niExportFunc(tU32) ni_windows_seh_on_handle(tU32 aExcCode, void* aExcInfo);
niExportFunc(sPanicException*) ni_windows_seh_get_last_panic();

template <typename RunFunc, typename CatchFunc>
auto TryCatchPanic(RunFunc&& aRun, CatchFunc&& aCatch) -> decltype(aRun())
{
  using RunReturnType = decltype(aRun());
  using CatchReturnType = decltype(aCatch(*ni_windows_seh_get_last_panic()));
  static_assert(std::is_same_v<RunReturnType, CatchReturnType>,
                "Run and catch functions must return the same type");

  if constexpr (std::is_void_v<RunReturnType>) {
    __try
    {
      aRun();
    } __except (ni_windows_seh_on_handle(_exception_code(), _exception_info()))
    {
      aCatch(*ni_windows_seh_get_last_panic());
    }
  }
  else {
    __try
    {
      return aRun();
    } __except (ni_windows_seh_on_handle(_exception_code(), _exception_info()))
    {
      return aCatch(*ni_windows_seh_get_last_panic());
    }
  }
}

  #else // #ifdef niUseWindowsSEHExceptions

template <typename RunFunc, typename CatchFunc>
auto TryCatchPanic(RunFunc&& aRun, CatchFunc&& aCatch) -> decltype(aRun())
{
  using RunReturnType = decltype(aRun());
  using CatchReturnType =
    decltype(aCatch(astl::declval<ni::sPanicException>()));
  static_assert(std::is_same_v<RunReturnType, CatchReturnType>,
                "Run and catch functions must return the same type");

  if constexpr (std::is_void_v<RunReturnType>) {
    niTry {
      aRun();
    }
    niCatch (ni::sPanicException, e) {
      aCatch(e);
    }
  }
  else {
    niTry {
      return aRun();
    }
    niCatch (ni::sPanicException, e) {
      return aCatch(e);
    }
  }
}

  #endif // #ifdef niUseWindowsSEHExceptions

#endif // #if !defined __cplusplus

/**@}*/
/**@}*/
} // namespace ni
#endif // __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
