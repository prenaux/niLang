#include "API/niLang/Types.h"

#ifdef niWinRT

using namespace ni;
using namespace Platform;

#include "Lang.h"
#include "FileFd.h"
#include "API/niLang/IOSProcess.h"
#include "API/niLang/Utils/CrashReport.h"

//--------------------------------------------------------------------------------------------
//
//  Platform implementation
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
iOSProcessManager* cLang::GetProcessManager() const {
  return NULL;
}

///////////////////////////////////////////////
void cLang::_PlatformExit(tU32 aulErrorCode) {
  throw ref new Platform::Exception(-1,ref new Platform::String(L"cOSPlatformWindows::Exit"));
}
void cLang::FatalError(const achar* aszMsg) {
  ni::Windows::UTF16Buffer wMsg;
  niWin32_UTF8ToUTF16(wMsg,aszMsg);
  throw ref new Platform::Exception(-1,ref new Platform::String(wMsg.begin()));
}

///////////////////////////////////////////////
void cLang::SetEnv(const achar* aaszEnv, const achar* aaszValue) const {
}
cString cLang::GetEnv(const achar* aaszEnv) const {
  return AZEROSTR;
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
niExportFunc(ni::tIntPtr) ni_dll_load(const ni::achar* aName) {
  return 0;
}
niExportFunc(void) ni_dll_free(ni::tIntPtr aModule) {
}
niExportFunc(ni::tPtr) ni_dll_get_proc(ni::tIntPtr aModule, const char* aProcName) {
  return 0;
}

#endif
