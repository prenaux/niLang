#include "API/niLang/Types.h"

#if defined niNoCrashReport

#include "API/niLang/StringDef.h"

using namespace ni;

niExportFunc(const cString&) ni_stack_get_current(cString& aOutput, void* apExp) {
  aOutput << "NO STACK\n";
  return aOutput;
}

#endif
