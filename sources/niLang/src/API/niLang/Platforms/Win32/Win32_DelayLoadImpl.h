//
// Include this header once per exe/module that use -DELAYLOAD:somemodule.dll
// flag to delay load a dll on Windows.
//
#include "Win32_Redef.h"
#define DELAYIMP_INSECURE_WRITABLE_HOOKS
#include <delayimp.h>
#include "../../StringDef.h"
#include "../../ILang.h"

#pragma comment(lib,"delayimp.lib")

#define TRACE_WIN32_DLL_DELAY_LOAD(...) // printf(__VA_ARGS__); fflush(stdout);

static FARPROC WINAPI delayLoadHook(unsigned dliNotify, PDelayLoadInfo  pdli) {
  TRACE_WIN32_DLL_DELAY_LOAD("... delayLoadHook: %d\n", dliNotify);
  switch(dliNotify) {
    // used to bypass or note helper only
    case dliStartProcessing: {
      TRACE_WIN32_DLL_DELAY_LOAD("... dliStartProcessing\n");
      break;
    }
    // called just before LoadLibrary, can override w/ new HMODULE return val
    case dliNotePreLoadLibrary: {
      TRACE_WIN32_DLL_DELAY_LOAD("... dliNotePreLoadLibrary\n");
      break;
    }
    // called just before GetProcAddress, can override w/ new FARPROC return value
    case dliNotePreGetProcAddress: {
      TRACE_WIN32_DLL_DELAY_LOAD("... dliNotePreGetProcAddress\n");
      break;
    }
    // failed to load library, fix it by returning a valid HMODULE
    case dliFailLoadLib:{
      TRACE_WIN32_DLL_DELAY_LOAD("... dliFailLoadLib\n");
      ni::cString msg = _ASTR("Can't load '") + pdli->szDll + "'.\nThe program will now exit.";
      ni::GetLang()->FatalError(msg.Chars());
      break;
    }
    // failed to get proc address, fix it by returning a valid FARPROC
    case dliFailGetProc: {
      TRACE_WIN32_DLL_DELAY_LOAD("... dliFailGetProc\n");
      break;
    }
    // called after all processing is done, no bypass possible at this point
    // except by longjmp()/throw()/RaiseException.
    case dliNoteEndProcessing: {
      TRACE_WIN32_DLL_DELAY_LOAD("... dliNoteEndProcessing\n");
      break;
    }
  }
  return 0;
}

// assign hook functions
#if !defined(DELAYIMP_INSECURE_WRITABLE_HOOKS)
const
#endif
__declspec(selectany) PfnDliHook __pfnDliNotifyHook2 = delayLoadHook;
#if !defined(DELAYIMP_INSECURE_WRITABLE_HOOKS)
const
#endif
__declspec(selectany) PfnDliHook __pfnDliFailureHook2 = delayLoadHook;
