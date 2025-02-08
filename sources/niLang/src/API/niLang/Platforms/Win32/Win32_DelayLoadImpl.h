//
// Include this header once per exe/module that use -DELAYLOAD:somemodule.dll
// flag to delay load a dll on Windows.
//
#include "Win32_Redef.h"
#define DELAYIMP_INSECURE_WRITABLE_HOOKS
#include <delayimp.h>

#pragma comment(lib,"delayimp.lib")

#define TRACE_WIN32_DLL_DELAY_LOAD(...) //printf(__VA_ARGS__); fflush(stdout);

#if !defined DLLLOAD_SEARCH_PATHS
#error "E/DLLLOAD_SEARCH_PATHS not defined!"
#endif

#define DEFINE_TO_STR_HELPER(x) L#x
#define DEFINE_TO_STR(x) DEFINE_TO_STR_HELPER(x)

#define DEFINE_TO_WSTR_HELPER(x) L#x
#define DEFINE_TO_WSTR(x) DEFINE_TO_WSTR_HELPER(x)

static const wchar_t kDLLLoadSearchPaths[] = DEFINE_TO_WSTR(DLLLOAD_SEARCH_PATHS);

static void DLLLoad_ShowError(
  const wchar_t* basePath,
  const wchar_t* dllName,
  const wchar_t* searchPaths)
{
  wchar_t errorMsg[4096];
  _snwprintf(
    errorMsg,
    sizeof(errorMsg)/sizeof(wchar_t),
    L"Failed to load DLL: '%s'\n"
    L"Base path: '%s'\n"
    L"Search paths: '%s'\n",
    dllName, basePath, searchPaths);
  MessageBoxW(NULL, errorMsg, L"DLL Load Error", MB_OK | MB_ICONERROR);
}

static const wchar_t* DLLLoad_GetNextPath(
  const wchar_t* paths,
  size_t* pos)
{
  static wchar_t buffer[MAX_PATH];
  size_t start = *pos;
  size_t i = 0;

  // Skip any leading semicolons
  while (paths[start] == L';') start++;

  // If we're at the end, return NULL
  if (paths[start] == L'\0') return NULL;

  // Copy until next semicolon or end
  while (paths[start] != L';' && paths[start] != L'\0' && i < MAX_PATH-1) {
    buffer[i++] = paths[start++];
  }
  buffer[i] = L'\0';

  *pos = start;
  return buffer[0] ? buffer : NULL;
}

// Try to load DLL relative to the exePath
static HMODULE DLLLoad_TryLoadInPath(
  const wchar_t* basePath,
  const wchar_t* searchPath,
  const wchar_t* dllName)
{
  wchar_t fullPath[MAX_PATH];
  if (wcscmp(searchPath,L".") == 0) {
    // search in basePath
    _snwprintf(fullPath, MAX_PATH, L"%s\\%s",
               basePath, dllName);
  }
  else if (*searchPath == L'.') {
    // relative path
    _snwprintf(fullPath, MAX_PATH, L"%s\\%s\\%s",
               basePath, searchPath, dllName);
  }
  else {
    // toolkit path
    _snwprintf(fullPath, MAX_PATH, L"%s\\..\\..\\..\\%s\\bin\\nt-x64\\%s",
               basePath, searchPath, dllName);
  }
  return LoadLibraryW(fullPath);
}

static HMODULE DLLLoad_TryLoadInSearchPaths(
  const wchar_t* basePath,
  const wchar_t* dllName,
  const wchar_t* searchPaths)
{
  const wchar_t* path;
  size_t pos = 0;
  while ((path = DLLLoad_GetNextPath(searchPaths, &pos)) != NULL) {
    HMODULE hLib = DLLLoad_TryLoadInPath(basePath, path, dllName);
    if (hLib) return hLib;
  }
  return NULL;
}

static HMODULE DelayLoadDLLW(const wchar_t* dllName) {
  wchar_t exePath[MAX_PATH];
  GetModuleFileNameW(NULL, exePath, MAX_PATH);
  wchar_t* lastSlash = wcsrchr(exePath, L'\\');
  HMODULE hLib = NULL;
  if (lastSlash) {
    *lastSlash = L'\0';
    hLib = DLLLoad_TryLoadInSearchPaths(exePath,dllName,kDLLLoadSearchPaths);
  }
  if (!hLib) {
    DLLLoad_ShowError(exePath, dllName, kDLLLoadSearchPaths);
    ::TerminateProcess(::GetCurrentProcess(), 0xDEADBEEF);
  }
  return hLib;
}

static HMODULE DelayLoadDLLA(const char* dllName) {
  wchar_t dllNameW[MAX_PATH];
  MultiByteToWideChar(CP_UTF8, 0, dllName, -1, dllNameW, MAX_PATH);
  return DelayLoadDLLW(dllNameW);
}

static FARPROC WINAPI delayLoadHook(unsigned dliNotify, PDelayLoadInfo pdli) {
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
      return (FARPROC)DelayLoadDLLA(pdli->szDll);
    }
    // called just before GetProcAddress, can override w/ new FARPROC return value
    case dliNotePreGetProcAddress: {
      TRACE_WIN32_DLL_DELAY_LOAD("... dliNotePreGetProcAddress\n");
      break;
    }
    // failed to load library, fix it by returning a valid HMODULE
    case dliFailLoadLib:{
      TRACE_WIN32_DLL_DELAY_LOAD("... dliFailLoadLib\n");
      return (FARPROC)DelayLoadDLLA(pdli->szDll);
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

#if !defined niNoProcess
extern bool Test_ChildProcess_Start(int argc, const char** argv, int& ret);
#endif
