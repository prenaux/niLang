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
  // exit(aulErrorCode);
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
niExportFunc(int) ni_debug_assert(
    int expression,     // The assert occur if this is eFalse
    const char* exp,  // Expression
    int line,     // Line number where the assert occur
    const char* file, // File where the assert occur
    int *alwaysignore,  // Pointer to the alwaysignore flag
    const char* desc) // Description of the assert
{
  if(alwaysignore && *alwaysignore == 1)
    return 0;

  if (!expression) {
    if (ni_debug_get_print_asserts()) {
      cString fmt;
      fmt.Format(_A("%s:%d: ASSERT: %s%s%s%s"),
                 file, line,
                 exp,
                 desc?_A(": "):_A(""),
                 desc?desc:_A(""),
                 desc?(desc[StrSize(desc)-1]=='\n'?_A(""):_A("\n")):_A(""));
      fmt.CatFormat("\nSTACK:\n");
      ni_stack_get_current(fmt,NULL);
      niPrintln(fmt.Chars());
      // always ignore after first print...
      if (alwaysignore) *alwaysignore = 1;
    }
  }

  return 0;
}

///////////////////////////////////////////////
niExportFunc(achar*) ni_get_exe_path(ni::achar* buffer) {
  StrZCpy(buffer,AMAX_PATH, JSCC_EXE_PATH);
  return buffer;
}
#endif
