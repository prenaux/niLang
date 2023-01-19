#include "API/niLang/Types.h"

#if defined niJSCC || defined niAndroid

#include "API/niLang/StringDef.h"

namespace ni {

niExportFuncCPP(const cString&) ni_stack_get_current(cString& aOutput, void*, int) {
  aOutput << "NO STACK\n";
  return aOutput;
}

}
#endif
