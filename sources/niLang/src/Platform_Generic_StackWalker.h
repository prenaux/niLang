#pragma once
#ifndef __PLATFORM_GENERIC_STACKWALKER_H_7E17AF4A_7276_EF11_9401_0F1738388A5E__
#define __PLATFORM_GENERIC_STACKWALKER_H_7E17AF4A_7276_EF11_9401_0F1738388A5E__

#include "API/niLang/Types.h"
#include "API/niLang/StringDef.h"

namespace ni {

niExportFunc(void) _StackCatEntry(cString& aOutput, tInt frameIndex, const achar* file, tInt line, const achar* func, const achar* module, tIntPtr address, tBool abIncludeCodeSnippet);

}
#endif // __PLATFORM_GENERIC_STACKWALKER_H_7E17AF4A_7276_EF11_9401_0F1738388A5E__
