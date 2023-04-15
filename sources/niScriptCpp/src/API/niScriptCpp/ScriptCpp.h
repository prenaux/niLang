#pragma once
#ifndef __SCRIPTCPP_H_BE92705B_0C3B_9140_9DE3_B0097DBB64C1__
#define __SCRIPTCPP_H_BE92705B_0C3B_9140_9DE3_B0097DBB64C1__

#include <niLang/Types.h>

#if defined niEmbedded || (!defined niWindows && !defined niOSX && !defined niLinux)

#define niNoScriptCpp

#else

#include <niLang/StringDef.h>
#include <niLang/ITime.h>
#include "Utils/ScriptCppImpl.h"

namespace ni {
/** \addtogroup niScriptCpp
 * @{
 */

#define SCRIPTCPP_HAMEXE "ham-rtcpp"
#define SCRIPTCPP_COMPILE_PROPERTY "niScriptCpp.Compile"
#define SCRIPTCPP_MODULE_TYPE_PROPERTY "niScriptCpp.ModuleType"

struct sScriptCppModuleCache {
  ni::tIntPtr hDLL = 0;
  ni::cString name;
  ni::cString path;
  ni::Ptr<ni::iTime> date;
};

typedef astl::map<ni::cString,sScriptCppModuleCache> tScriptCppModuleMap;

struct sScriptCppStats {
  ni::tU32 _numCompiled = 0;
  ni::tU32 _numOutOfDate = 0;
  ni::tU32 _numUpToDate = 0;
};

niExportFunc(ni::tBool) ScriptCpp_GetCompileEnabled();
niExportFuncCPP(ni::cString) ScriptCpp_GetCompileModuleType();
niExportFunc(sScriptCppStats*) ScriptCpp_GetStats();
niExportFunc(void) ScriptCpp_CleanupDLLs();
niExportFunc(ni::iScriptingHost*) ScriptCpp_CreateScriptingHost();

/**@}*/
}

#endif // niNoScriptCpp

#endif // __SCRIPTCPP_H_BE92705B_0C3B_9140_9DE3_B0097DBB64C1__
