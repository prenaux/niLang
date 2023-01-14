#include "API/niLang/Types.h"

#if defined niOSX || defined niIOS || defined niLinux

#include "API/niLang/StringDef.h"
#include <stdio.h>
#include <dlfcn.h>     // for dladdr
#include <cxxabi.h>    // for __cxa_demangle

// Pick one, libunwind shows slightly cleaner callstacks
#define USE_LIBUNWIND
// #define USE_BACKTRACE

#ifdef USE_BACKTRACE
// backtrace() version

#include <execinfo.h>  // for backtrace

namespace ni {

// A C++ function that will produce a stack trace with demangled function and method names.
static cString& Backtrace(cString& aOutput, int skip)
{
  void* callstack[128];
  const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
  const int nFrames = backtrace(callstack, nMaxFrames);

  for (int i = skip; i < nFrames; ++i) {
    Dl_info info;
    if (dladdr(callstack[i], &info)) {
      char *demangled = NULL;
      int status;
      demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
      aOutput.CatFormat(
        "%d: %s + %d\n",
        i-skip, status == 0 ? demangled : info.dli_sname,
        (ssize_t)((char *)callstack[i] - (char *)info.dli_saddr));
      free(demangled);
    }
    else {
      aOutput.CatFormat("%d: unknown\n", i);
    }
  }
  if (nFrames == nMaxFrames) {
    aOutput << "  [truncated]\n";
  }
  return aOutput;
}

niExportFunc(const cString&) ni_stack_get_current(cString& aOutput, void* apExp, int skip) {
  niUnused(apExp);
  // +2 to skip the Backtrace and ni_stack_get_current functions
  return Backtrace(aOutput, ni::Max(0,skip)+2);
}

}
#endif // USE_BACKTRACE

#if defined USE_LIBUNWIND
// libunwind version

#define UNW_LOCAL_ONLY
#include <cxxabi.h>
#include <libunwind.h>

namespace ni {

niExportFunc(const cString&) ni_stack_get_current(cString& aOutput, void*, int skip) {
  unw_cursor_t cursor = {};
  unw_context_t context = {};

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  int i = 0;

  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset = {}, pc = {};
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }

    ++i;
    if (i > skip) {
      aOutput.CatFormat("%d:", i-skip-1);
    }

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char* nameptr = sym;
      int status;
      char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      if (i > skip) {
        aOutput.CatFormat(" %s + %d\n", nameptr, offset);
      }
      std::free(demangled);
    } else {
      if (i > skip) {
        aOutput << " -- error: unable to obtain symbol name for this frame\n";
      }
    }
  }
  return aOutput;
}

}
#endif // USE_LIBUNWIND

#endif // niOSX || niIOS || niLinux
