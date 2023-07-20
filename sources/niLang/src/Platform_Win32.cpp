#include "API/niLang/Types.h"

using namespace ni;

#ifdef niWinDesktop

#include "Lang.h"
#include "FileFd.h"
#include "API/niLang/IOSProcess.h"

#ifdef niJNI
#include "API/niLang/Utils/JNIUtils.h"
#endif

#include "API/niLang/Utils/CrashReport.h"
#include "API/niLang/Utils/Path.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/STL/set.h"

#pragma comment(lib,"advapi32.lib")

static tIntPtr      _nLastErr = 0;
static cString      _strLastErr;

static tIntPtr __stdcall _GetLastErrorCode() {
  tIntPtr currErr = ::GetLastError();
  if (currErr != _nLastErr) {
    _nLastErr = currErr;
    _strLastErr.Clear();
  }
  return _nLastErr;
}
static const achar* _GetLastErrorMessage(const tIntPtr currErr) {
  if (currErr != _nLastErr || _strLastErr.IsEmpty()) {
    if (currErr == S_OK) {
      _nLastErr = currErr;
      _strLastErr = _A("OK");
    }
    else {
      LPVOID lpMsgBuf = NULL;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, (DWORD)currErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
      _strLastErr = (LPCTSTR)lpMsgBuf;
      LocalFree(lpMsgBuf);
    }
  }
  return _strLastErr.Chars();
}
static const achar* _GetLastErrorMessage() {
  tIntPtr currErr = ::GetLastError();
  return _GetLastErrorMessage(currErr);
}

// Attach the output of the application to the parent console if there's one.
static BOOL _AttachOutputToConsole(void) {
  // The console in Emacs capture the application's output, if we attach to
  // the parent process nothing is output in the emacs shell...
  if (agetenv("EMACS").IEq("t"))
    return FALSE;

  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    // Redirect unbuffered STDOUT to the console
    HANDLE consoleHandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (consoleHandleOut != INVALID_HANDLE_VALUE) {
      freopen("CONOUT$", "w", stdout);
      setvbuf(stdout, NULL, _IONBF, 0);
    }
    else {
      return FALSE;
    }
    // Redirect unbuffered STDERR to the console
    HANDLE consoleHandleError = GetStdHandle(STD_ERROR_HANDLE);
    if (consoleHandleError != INVALID_HANDLE_VALUE) {
      freopen("CONOUT$", "w", stderr);
      setvbuf(stderr, NULL, _IONBF, 0);
    }
    else {
      return FALSE;
    }
    return TRUE;
  }

  return FALSE;
}

// Send the "Enter" to the console to release the command prompt on the parent
// console.
static void _SendEnterKey(void) {
  INPUT ip;

  // Set up a generic keyboard event.
  ip.type = INPUT_KEYBOARD;
  ip.ki.wScan = 0; // hardware scan code for key
  ip.ki.time = 0;
  ip.ki.dwExtraInfo = 0;

  // Send the "Enter" key
  ip.ki.wVk = 0x0D; // virtual-key code for the "Enter" key
  ip.ki.dwFlags = 0; // 0 for key press
  SendInput(1, &ip, sizeof(INPUT));

  // Release the "Enter" key
  ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
  SendInput(1, &ip, sizeof(INPUT));
}

static tBool _hasAttachedOutputToConsole = eFalse;

niExportFunc(void) _niWinMainStartup() {
  _hasAttachedOutputToConsole = _AttachOutputToConsole();
}

niExportFunc(void) _niWinMainShutdown() {
  // Send "enter" to release the application from the console. This is a
  // hack, but without it the console doesn't know that the application has
  // returned. The "enter" key is only sent if the console window is in focus.
  if (_hasAttachedOutputToConsole && (GetConsoleWindow() == GetForegroundWindow())){
    _hasAttachedOutputToConsole = eFalse;
    _SendEnterKey();
  }
}

static void _FatalError(const achar* aszMsg) {
  if (::IsDebuggerPresent()) {
    ni_debug_break();
  }

  cString dialogMessage, logMessage;

  // Log message
  {
    logMessage = niFmt(_A("[FATAL ERROR]\n%s\n"), aszMsg);
    tIntPtr errCode = _GetLastErrorCode();
    if (errCode && errCode != 6) {
      logMessage += niFmt(_A("--- OS Error (%d:%x) ---\n"),errCode,errCode);
      logMessage += _GetLastErrorMessage();
    }
  }

  // Dialog message
  {
    dialogMessage = logMessage;

    astl::vector<cString> logs;
    ni_get_last_logs(&logs,20);
    if (!logs.empty()) {
      dialogMessage += "--- Last logs ---\n";
      niLoop(i,logs.size()) {
        dialogMessage += logs[i];
      }
    }
  }

  niLog(Error,logMessage);

  if (ni_get_show_fatal_error_message_box()) {
    ni::Windows::UTF16Buffer wMsg;
    niWin32_UTF8ToUTF16(wMsg,dialogMessage.Chars());
    ::MessageBoxW(NULL,wMsg.begin(),L"Fatal Error",
                  MB_OK|MB_ICONERROR|MB_SYSTEMMODAL|MB_TOPMOST|MB_SETFOREGROUND);
  }

  ::TerminateProcess(::GetCurrentProcess(),0xDEADBEEF);
}

//--------------------------------------------------------------------------------------------
//
//  Platform implementation
//
//--------------------------------------------------------------------------------------------
static HINSTANCE  _hInstance = NULL;

///////////////////////////////////////////////
iOSProcessManager* cLang::GetProcessManager() const {
  return ni::GetOSProcessManager();
}

///////////////////////////////////////////////
void cLang::_PlatformExit(tU32 aulErrorCode) {
  _niWinMainShutdown();

#ifdef niMSVC
  __try
#endif
  {
    exit(aulErrorCode);
  }
#ifdef niMSVC
  __finally {
    niPrintln("= Exception on Exit =");
    ::TerminateProcess(::GetCurrentProcess(),0x4444);
  }
#endif
}
void cLang::FatalError(const achar* aszMsg) {
  _FatalError(aszMsg);
}

///////////////////////////////////////////////
void cLang::SetEnv(const achar* aaszEnv, const achar* aaszValue) const {
  cString envStr;
  envStr = aaszEnv;
  envStr += _A("=");
  envStr += aaszValue;
  aputenv(envStr.Chars());
}
cString cLang::GetEnv(const achar* aaszEnv) const {
  return agetenv(aaszEnv);
}

///////////////////////////////////////////////
niExportFunc(achar*) ni_get_exe_path(ni::achar* buffer) {
  return ni::Windows::utf8_GetModuleFileName(NULL,buffer);
}

///////////////////////////////////////////////
//
// - AddDllDirectory requires Windows 8+ / KB2533623 on Windows 7.
//     See: https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-adddlldirectory
//
// - SetDllDirectoryW could be used instead on XP, it can be used as a "push/pop" before/after every call.
//     See https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-setdlldirectoryw
//
// Right now we use SetDllDirectoryW by default since its more compatible and
// it doesnt require a first call to SetDefaultDllDirectories. Finally this is
// needed only when we get a ERROR_MOD_NOT_FOUND which is not the common case.
//

// #define USE_ADD_DLL_DIRECTORY
#define TRACE_ADD_DLL_DIR(MSG)  // niDebugFmt(MSG)

#ifdef USE_ADD_DLL_DIRECTORY
struct sWindowDllLoaderDirectories {
  __sync_mutex();
  astl::set<cString> _addedDirs;

  sWindowDllLoaderDirectories() {
    ::SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS|LOAD_LIBRARY_SEARCH_USER_DIRS);
  }

  void addDllDir(const ni::achar* aName) {
    cPath path = ni::GetRootFS()->GetAbsolutePath(aName);
    const cString& dir = path.GetDirectory();
    __sync_lock();
    if (dir.IsNotEmpty() && (_addedDirs.find(dir) == _addedDirs.end())) {
      TRACE_ADD_DLL_DIR(("... ni_dll_load: Adding DLL directory '%s' for dll '%s'", dir, aName));
      ni::Windows::UTF16Buffer wDir;
      niWin32_UTF8ToUTF16(wDir,dir.c_str());
      ::AddDllDirectory(wDir.begin());
      _addedDirs.insert(dir);
    }
  }
};
static sWindowDllLoaderDirectories _dllDirectories;
#else
extern "C" BOOL WINAPI SetDllDirectoryW( LPCWSTR lpPathName );
#endif

niExportFunc(ni::tIntPtr) ni_dll_load(const ni::achar* aName) {
  ni::Windows::UTF16Buffer wName;
  niWin32_UTF8ToUTF16(wName,aName);

  {
    ni::tIntPtr hModule = (ni::tIntPtr)::LoadLibraryW(wName.begin());
    if (hModule) {
      return hModule;
    }
  }

  tIntPtr errCode = _GetLastErrorCode();
  if (errCode == ERROR_MOD_NOT_FOUND) {
    TRACE_ADD_DLL_DIR(("... ni_dll_load: ERROR_MOD_NOT_FOUND: Setting DLL directory for dll '%s'", aName));
#ifdef USE_ADD_DLL_DIRECTORY
    _dllDirectories.addDllDir(aName);
#else
    cPath path = ni::GetRootFS()->GetAbsolutePath(aName);
    const cString& dir = path.GetDirectory();
    ni::Windows::UTF16Buffer wDir;
    niWin32_UTF8ToUTF16(wDir,dir.c_str());
    ::SetDllDirectoryW(wDir.begin());
#endif
    ni::tIntPtr hModule = (ni::tIntPtr)::LoadLibraryW(wName.begin());
#if !defined USE_ADD_DLL_DIRECTORY
    ::SetDllDirectoryW(NULL);
#endif
    if (hModule != NULL) {
      return hModule;
    }
  }

  if (errCode != ERROR_FILE_NOT_FOUND) {
    niError(niFmt("ni_dll_load: error: (%d:%x) %s.", errCode, errCode, _GetLastErrorMessage(errCode)));
  }
  return NULL;
}
niExportFunc(void) ni_dll_free(ni::tIntPtr aModule) {
  ::FreeLibrary((HMODULE)aModule);
}
niExportFunc(ni::tPtr) ni_dll_get_proc(ni::tIntPtr aModule, const char* aProcName) {
  return (ni::tPtr)::GetProcAddress((HMODULE)aModule,aProcName);
}
#endif

//--------------------------------------------------------------------------------------------
//
//  UTF8 implementation
//
//--------------------------------------------------------------------------------------------
#if defined niWin32

#include <stdio.h>
#include "API/niLang/StringDef.h"
#include "API/niLang/Platforms/Win32/Win32_UTF.h"

#if !defined niWinRT
ni::cString _GetCommandLine() {
  return Win32GetCommandLine();
}
#endif

niExportFunc(FILE*) afopen(const achar* file, const achar* mode, ni::cString* apPathOnDisk) {
  if (!niStringIsOK(file)) return NULL;
  if (!niStringIsOK(mode)) return NULL;
  ni::Windows::UTF16Buffer wFile;
  niWin32_UTF8ToUTF16(wFile,file);
  ni::Windows::UTF16Buffer wMode;
  niWin32_UTF8ToUTF16(wMode,mode);
  FILE* fp = _wfopen(wFile.begin(),wMode.begin());
  if (fp && apPathOnDisk) {
    *apPathOnDisk = file;
  }
  return fp;
}
niExportFunc(int) amkdir(const achar* dir) {
  if (!niStringIsOK(dir)) return -1;
  ni::Windows::UTF16Buffer wDir;
  niWin32_UTF8ToUTF16(wDir,dir);
  return _wmkdir(wDir.begin());
}
niExportFunc(int) armdir(const achar* dir) {
  if (!niStringIsOK(dir)) return -1;
  ni::Windows::UTF16Buffer wDir;
  niWin32_UTF8ToUTF16(wDir,dir);
  return _wrmdir(wDir.begin());
}
niExportFunc(int) aunlink(const achar* file) {
  if (!niStringIsOK(file)) return -1;
  ni::Windows::UTF16Buffer wFile;
  niWin32_UTF8ToUTF16(wFile,file);
  return _wunlink(wFile.begin());
}

niExportFunc(int) FdOpen(const achar* path, ni::cString* apPathOnDisk, int mode, int pmode) {
  ni::BufferUTF16 wFileName(path);
  int r = _wopen(wFileName.Chars(),mode,pmode);
  if (r >= 0 && apPathOnDisk) {
    *apPathOnDisk = path;
  }
  return r;
}

namespace ni { namespace Windows {
niExportFunc(void) utf8_OutputDebugString(const char* aaszMsg) {
  if (aaszMsg && *aaszMsg) {
    UTF16Buffer wMsg;
    niWin32_UTF8ToUTF16(wMsg,aaszMsg);
    ::OutputDebugStringW(wMsg.begin());
  }
}
}}

#ifdef niWinDesktop
niExportFuncCPP(cString) agetenv(const achar* env) {
  if (!niStringIsOK(env)) return "";
  ni::Windows::UTF16Buffer wEnv;
  niWin32_UTF8ToUTF16(wEnv,env);
  ni::Windows::UTF8Buffer utf8EnvRet;
  WCHAR* envRet = _wgetenv(wEnv.begin());
  if (!envRet) return AZEROSTR;
  niWin32_UTF16ToUTF8(utf8EnvRet,envRet);
  return utf8EnvRet.begin();
}
niExportFunc(int) aputenv(const achar* envString) {
  if (!niStringIsOK(envString)) return -1;
  ni::Windows::UTF16Buffer wEnvString;
  niWin32_UTF8ToUTF16(wEnvString,envString);
  return _wputenv(wEnvString.begin());
}

namespace ni { namespace Windows {

niExportFunc(char*) utf8_GetCommandLine() {
  static ni::Windows::UTF8Buffer _cmdLine;
  const WCHAR* cmdLine = ::GetCommandLineW();
  niWin32_UTF16ToUTF8(_cmdLine,cmdLine);
  utf8_FixDriveLetter(_cmdLine.begin());
  return _cmdLine.begin();
}

niExportFunc(BOOL) utf8_SHGetSpecialFolderPath(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) {
  WCHAR buffer[_MAX_PATH];
  if (!SHGetSpecialFolderPathW(hwndOwner, buffer, nFolder, fCreate))
    return FALSE;
  ni::Windows::UTF8Buffer uPath; uPath.Adopt(lpszPath);
  niWin32_UTF16ToUTF8(uPath,buffer);
  return TRUE;
}

niExportFunc(BOOL) utf8_SHGetFolderPath(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPSTR lpszPath) {
  WCHAR buffer[_MAX_PATH];
  if (!SUCCEEDED(SHGetFolderPathW(hwndOwner, nFolder, hToken, dwFlags, buffer)))
    return FALSE;
  ni::Windows::UTF8Buffer uPath; uPath.Adopt(lpszPath);
  niWin32_UTF16ToUTF8(uPath,buffer);
  return TRUE;
}

niExportFunc(int) utf8_access(const char* aaszPath, int mode) {
  ni::Windows::UTF16Buffer wPath;
  niWin32_UTF8ToUTF16(wPath,aaszPath);
  return _waccess(wPath.begin(),mode);
}

niExportFunc(char*) utf8_FixDriveLetter(char* apOut) {
  // make sure the drive letter is lower case for consistency
  int c0 = StrToLower(apOut[0]);
  if ((c0 >= 'a') && (c0 <= 'z') && (apOut[1] == ':'))  {
    apOut[0] = c0;
  }
  return apOut;
}

niExportFunc(char*) utf8_GetModuleFileName(HMODULE ahDLL, char* apOut) {
  WCHAR buffer[_MAX_PATH];
  ::GetModuleFileNameW(ahDLL,buffer,_MAX_PATH);
  *apOut = 0;
  UTF8Buffer uOut; uOut.Adopt(apOut);
  niWin32_UTF16ToUTF8(uOut,buffer);
  return utf8_FixDriveLetter(apOut);
}

niExportFunc(HMODULE) utf8_GetModuleHandle(const char* aaszPath) {
  if (aaszPath) {
    UTF16Buffer wPath;
    niWin32_UTF8ToUTF16(wPath,aaszPath);
    return ::GetModuleHandleW(wPath.begin());
  }
  else {
    return ::GetModuleHandle(NULL);
  }
}

niExportFuncCPP(cString) utf8_getcwd() {
  WCHAR buffer[_MAX_PATH];
  _wgetcwd(buffer,_MAX_PATH);
  UTF8Buffer uOut;
  niWin32_UTF16ToUTF8(uOut,buffer);
  return utf8_FixDriveLetter(uOut.begin());
}

}}
#endif

#endif
