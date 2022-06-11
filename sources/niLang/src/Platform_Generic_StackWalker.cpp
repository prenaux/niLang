#include "API/niLang/Types.h"

#if defined niNoCrashReport || (!defined niWinDesktop && !defined niOSX)

using namespace ni;

niExportFunc(const cString&) ni_stack_get_current(cString& aOutput, void* apExp) {
  aOutput << "NO STACK\n";
  return aOutput;
}

#endif
