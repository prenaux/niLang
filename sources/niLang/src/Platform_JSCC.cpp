#include "API/niLang/Types.h"

#ifdef niJSCC

#include <emscripten.h>
#include "API/niLang/IOSProcess.h"
#include "Lang.h"
#include "FileFd.h"
#include "API/niLang/Utils/CrashReport.h"

using namespace ni;

EM_JS(char*, _JSCC_GetConfigProperty, (const char* aProperty), {
  var propertyStr = UTF8ToString(aProperty);
  var jsString = Module["NIAPP_CONFIG"][propertyStr] || "";
  // console.log("... _JSCC_GetConfigProperty: " + propertyStr + " = " + jsString);
  var lengthBytes = lengthBytesUTF8(jsString) + 1;
  var stringOnWasmHeap = _malloc(lengthBytes);
  stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
  return stringOnWasmHeap;
});

namespace ni {
niExportFuncCPP(ni::cString) niJSCC_Get_NIAPP_CONFIG(const char* aProperty) {
  char* asz = _JSCC_GetConfigProperty(aProperty);
  ni::cString str = asz;
  free(asz);
  return str;
}
}

#if niMinFeatures(15)
///////////////////////////////////////////////
iOSProcessManager* cLang::GetProcessManager() const {
  return NULL;
}
#endif

///////////////////////////////////////////////
void cLang::_PlatformExit(tU32 aulErrorCode) {
  niLog(Info,niFmt("_PlatformExit: 0x%X", aulErrorCode));
  emscripten_cancel_main_loop();
}
void cLang::FatalError(const achar* aszMsg) {
  niPrintln(aszMsg);
  this->Exit(0xDEADBEEF);
}

///////////////////////////////////////////////
void cLang::SetEnv(const achar* aaszEnv, const achar* aaszValue) const {
  cString envStr;
  envStr = aaszEnv;
  envStr += "=";
  envStr += aaszValue;
  aputenv(envStr.Chars());
}
cString cLang::GetEnv(const achar* aaszEnv) const {
  return agetenv(aaszEnv);
}

///////////////////////////////////////////////
niExportFunc(achar*) ni_get_exe_path(ni::achar* buffer) {
  cString exePath = niJSCC_Get_NIAPP_CONFIG("exePath");
  StrZCpy(buffer,AMAX_PATH,exePath.Chars());
  return buffer;
}
#endif
