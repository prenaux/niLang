#include "API/niLang/Types.h"

#ifdef niJSCC

#include <emscripten.h>
#include "API/niLang/IOSProcess.h"
#include "Lang.h"
#include "FileFd.h"
#include "API/niLang/Utils/CrashReport.h"

using namespace ni;

#define JSCC_WORK_DIR "/Work"
#define JSCC_EXE_PATH JSCC_WORK_DIR "/niLang/bin/web-js/web-js.html"

EM_JS(
  void, _JSCC_InitModule_niLang, (const char* aWorkDir, const char* aExePath), {
    console.log("JSCC_InitModule_niLang");

    // Create a dummy file for the executable in case we're looking for one
    var exePath = UTF8ToString(aExePath);
    FS.writeFile(exePath, "JSCC_EXE: " + exePath);
  }
);

extern "C" void JSCC_InitModule_niLang() {
  // not strictly necessary but convenient to run unit tests
  setenv("WORK", JSCC_WORK_DIR, 1);
  // initialize the file system
  _JSCC_InitModule_niLang(JSCC_WORK_DIR,JSCC_EXE_PATH);
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
  StrZCpy(buffer,AMAX_PATH, JSCC_EXE_PATH);
  return buffer;
}
#endif
