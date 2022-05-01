#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>
#include <delayimp.h>
#include "../../StringDef.h"
#include "../../ILang.h"

#pragma comment(lib,"delayimp.lib")

static FARPROC WINAPI delayLoadHook(unsigned dliNotify, PDelayLoadInfo  pdli) {
  switch(dliNotify) {
    // used to bypass or note helper only
    case dliStartProcessing: {
      break;
    }
    // called just before LoadLibrary, can override w/ new HMODULE return val
    case dliNotePreLoadLibrary: {
      break;
    }
    // called just before GetProcAddress, can override w/ new FARPROC return value
    case dliNotePreGetProcAddress: {
      break;
    }
    // failed to load library, fix it by returning a valid HMODULE
    case dliFailLoadLib:{
      ni::cString msg = _ASTR("Can't load '") + pdli->szDll + "'.\nThe program will now exit.";
      ni::GetLang()->FatalError(msg.Chars());
      break;
    }
    // failed to get proc address, fix it by returning a valid FARPROC
    case dliFailGetProc: {
      break;
    }
    // called after all processing is done, no bypass possible at this point
    // except by longjmp()/throw()/RaiseException.
    case dliNoteEndProcessing: {
      break;
    }
  }
  return 0;
}

struct _InitializeDelayLoad {
  _InitializeDelayLoad() {
    __pfnDliNotifyHook2 = delayLoadHook;
    __pfnDliFailureHook2 = delayLoadHook;
  }
};
static _InitializeDelayLoad __initializeDelayLoad;
