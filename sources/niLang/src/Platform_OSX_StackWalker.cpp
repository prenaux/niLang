#include "API/niLang/Types.h"

#if defined niOSX || defined niIOS

#include "API/niLang/StringDef.h"
#include <stdio.h>
#include <execinfo.h>  // for backtrace
#include <dlfcn.h>     // for dladdr
#include <cxxabi.h>    // for __cxa_demangle

namespace ni {

// A C++ function that will produce a stack trace with demangled function and method names.
static cString& Backtrace(cString& aOutput, int skip = 1)
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
        i, status == 0 ? demangled : info.dli_sname,
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

niExportFunc(const cString&) ni_stack_get_current(cString& aOutput, void* apExp) {
  return Backtrace(aOutput);
}

}
#endif // niOSX || niIOS
