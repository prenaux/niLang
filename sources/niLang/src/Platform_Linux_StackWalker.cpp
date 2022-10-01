#include "API/niLang/Types.h"

#if defined niLinux && !defined niNoCrashReport

#include "API/niLang/StringDef.h"

using namespace ni;

#if 0
// backtrace() version

#include <stdio.h>
#include <execinfo.h>  // for backtrace
#include <dlfcn.h>     // for dladdr
#include <cxxabi.h>    // for __cxa_demangle

// A C++ function that will produce a stack trace with demangled function and method names.
static cString& _Backtrace(cString& out, int skip = 1)
{
  void *callstack[128];
  const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
  char buf[1024];
  int nFrames = backtrace(callstack, nMaxFrames);

  for (int i = skip; i < nFrames; i++) {
    Dl_info info;
    if (dladdr(callstack[i], &info)) {
      char *demangled = NULL;
      int status;
      demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
      snprintf(buf, sizeof(buf), "%-3d %p %s + %zd\n",
               i, (void*)(2 + sizeof(void*) * 2),
               status == 0 ? demangled : info.dli_sname,
               (ssize_t)((char *)callstack[i] - (char *)info.dli_saddr));
      free(demangled);
    } else {
      snprintf(buf, sizeof(buf), "%-3d %p\n",
               i, (void*)(2 + sizeof(void*) * 2));
    }
    out += buf;
  }
  if (nFrames == nMaxFrames)
    out += "  [truncated]\n";
  return out;
}

niExportFunc(const cString&) ni_stack_get_current(cString& aOutput, void* apExp) {
  niUnused(apExp);
  return _Backtrace(aOutput,1);
}

#else
// libunwind version

#define UNW_LOCAL_ONLY
#include <cxxabi.h>
#include <libunwind.h>

niExportFunc(const cString&) ni_stack_get_current(cString& aOutput, void* apExp) {
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
    aOutput.CatFormat("%d:", i);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char* nameptr = sym;
      int status;
      char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      aOutput.CatFormat(" %s + %d\n", nameptr, offset);
      std::free(demangled);
    } else {
      aOutput << " -- error: unable to obtain symbol name for this frame\n";
    }
  }
  return aOutput;
}

#endif
#endif
