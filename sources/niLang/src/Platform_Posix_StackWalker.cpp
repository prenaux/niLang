#include "API/niLang/Types.h"

#if defined niOSX || defined niIOS || defined niLinux

#include "API/niLang/StringDef.h"
#include <stdio.h>
#include <dlfcn.h>     // for dladdr
#include <cxxabi.h>    // for __cxa_demangle
#include <execinfo.h>  // for backtrace

namespace ni {

niExportFuncCPP(const cString&) ni_stack_get_current(cString& aOutput, void* apExp, int skip) {
  niUnused(apExp);

  const int nMaxFrames = 100;
  void* callstack[nMaxFrames] = {};
  const int nFrames = backtrace(callstack, nMaxFrames);

#if 0
  char** symbols = backtrace_symbols(callstack, nFrames);
  for (int i = skip+1; i < nFrames; ++i) {
    aOutput.CatFormat(
      "%2d: %s + %d\n",
      i-skip, symbols[i],
      (tIntPtr)callstack[i]);
  }
  free(symbols);
#else
  for (int i = skip+1; i < nFrames; ++i) {
    Dl_info info;
    if (dladdr(callstack[i], &info)) {
      char *demangled = NULL;
      int status;
      demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);

      aOutput.CatFormat(
        "%2d: %s + %d [%x]\n",
        i-skip, status == 0 ? demangled : info.dli_sname,
        ((tIntPtr)callstack[i] - (tIntPtr)info.dli_saddr),
        (tIntPtr)callstack[i]);
      free(demangled);
    }
    else {
      aOutput.CatFormat("%d: unknown\n", i);
    }
  }
#endif

  if (nFrames == nMaxFrames) {
    aOutput << "[stack truncated]\n";
  }
  return aOutput;
}

}
#endif // niOSX || niIOS || niLinux
