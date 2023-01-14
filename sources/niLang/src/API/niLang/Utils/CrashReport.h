#ifndef __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
#define __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../StringDef.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

niExportFunc(ni::cString&) ni_stack_get_current(ni::cString& aOutput, void* apExp, int skip);

#if !defined __cplusplus

#define niCrashReport_DeclareHandler()
#define niCrashReport_ModuleInstall()

#else

#ifdef niWindows
#include <signal.h>
#define niCrashReportHasMinidump
niExportFuncCPP(ni::cString) ni_generate_minidump(void* apExp);
#endif

niExportFunc(void) cpp_terminate_handler();
niExportFunc(void) cpp_unexp_handler();
niExportFunc(void) cpp_purecall_handler();
niExportFunc(void) cpp_security_handler(int code, void *x);
niExportFunc(void) cpp_invalid_parameter_handler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
niExportFunc(int)  cpp_new_handler(size_t);
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

#ifdef niWindows
  _set_error_mode(_OUT_TO_STDERR);
#endif

#if defined _MSC_VER
#if _MSC_VER>=1300
  // Pure Virtual call handler
  _set_purecall_handler(cpp_purecall_handler);

  // Catch new operator memory allocation exceptions
  _set_new_mode(1); // Force malloc() to call new handler too
  _set_new_handler(cpp_new_handler);
#endif

#if _MSC_VER>=1400
  // Catch invalid parameter exceptions.
  _set_invalid_parameter_handler(cpp_invalid_parameter_handler);
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400
  // Catch buffer overrun exceptions
  // The _set_security_error_handler is deprecated in VC8 C++ run time library
  _set_security_error_handler(cpp_security_handler);
#endif

  // Set up C++ signal handlers
#if _MSC_VER>=1400
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

#ifdef niWindows
  _set_error_mode(_OUT_TO_STDERR);
#endif

#if defined _MSC_VER
#if _MSC_VER>=1300
  // Pure Virtual call handler
  _set_purecall_handler(NULL);

  // Catch new operator memory allocation exceptions
  _set_new_mode(1); // Force malloc() to call new handler too
  _set_new_handler(NULL);
#endif

#if _MSC_VER>=1400
  // Catch invalid parameter exceptions.
  _set_invalid_parameter_handler(NULL);
#endif

#if _MSC_VER>=1300 && _MSC_VER<1400
  // Catch buffer overrun exceptions
  // The _set_security_error_handler is deprecated in VC8 C++ run time library
  _set_security_error_handler(NULL);
#endif

  // Set up C++ signal handlers
#if _MSC_VER>=1400
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
  sNiCrashReport() { __niCrashReportModuleInstall(); }
  ~sNiCrashReport() { __niCrashReportModuleUninstall(); }
};

// Should be declared above your main() function
#define niCrashReport_DeclareHandler() ni::sNiCrashReport _niCrashReport;

#ifdef niWindows
// This should only be necessary on Windows since on other platforms the
// standard C library is shared between the modules. On Windows DLLs can have
// a different CRT and thus set of signal table and handlers.
#define niCrashReport_ModuleInstall() __niCrashReportModuleInstall()
#else
#define niCrashReport_ModuleInstall()
#endif

#endif

/**@}*/
/**@}*/
} // namespace ni
#endif // __CRASHRPT_H_39E74FD9_5FD5_4D06_A7BE_773E182E83CC__
