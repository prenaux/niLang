// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"

#if niMinFeatures(15)

#include "API/niLang/IOSProcess.h"
#include "API/niLang/IFile.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/IRegex.h"
#include "API/niLang/STL/deque.h"
#include "API/niLang/Utils/Buffer.h"
#include "API/niLang/IFileSystem.h"
#include "API/niLang/ILang.h"
#include "Platform.h"
#include "API/niLang/Utils/StringTokenizerImpl.h"
#include "API/niLang/STL/utils.h"

using namespace ni;

cString _GetCommandLine(void);

niExternC char** environ;
static char** _GetEnviron() {
  return environ;
}

niExportFuncCPP(ni::cString) agetcwd();

//--------------------------------------------------------------------------------------------
//
//  Windows Utils
//
//--------------------------------------------------------------------------------------------
#ifdef niWindows

#include <psapi.h>
#include "API/niLang/Platforms/Win32/Win32_File.h"

#define WIN_HANDLE_IS_VALID(H) (((H) != NULL) && ((H) != INVALID_HANDLE_VALUE))
// #define USE_WINDOWS_STD_HANDLE

/*
 * Note, inheritRead == FALSE means "inheritWrite", i.e. one of the
 * pipe ends is always expected to be inherited. The pipe end that should
 * be inherited is opened without overlapped io flags, as the child program
 * would expect stdout not to demand overlapped I/O.
 */
static BOOL _CreateAnonPipes(HANDLE *phRead, HANDLE *phWrite,
                             BOOL inheritRead)
{
  SECURITY_ATTRIBUTES sa = {0};
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.bInheritHandle = TRUE;
  sa.lpSecurityDescriptor = NULL;

  int success;
  HANDLE non_inherited; /* Non-inherited copy of handle. */

  if (!CreatePipe(phRead, phWrite, &sa, 0)) {
    return FALSE;
  }

  if (inheritRead) {
    success = DuplicateHandle(GetCurrentProcess(), *phWrite,
                              GetCurrentProcess(), &non_inherited, 0,
                              FALSE, DUPLICATE_SAME_ACCESS);
    CloseHandle(*phWrite);
    *phWrite = non_inherited;
  } else {
    success = DuplicateHandle(GetCurrentProcess(), *phRead,
                              GetCurrentProcess(), &non_inherited, 0,
                              FALSE, DUPLICATE_SAME_ACCESS);
    CloseHandle(*phRead);
    *phRead = non_inherited;
  }

  return success;
}
#if 0
static BOOL _CreateNamedPipes(HANDLE *phRead, HANDLE *phWrite,
                              BOOL inheritRead)
{
  SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};

  static ni::SyncCounter _count;
  tI32 calls = _count.Inc();
  cCString pipeName;
  pipeName << "\\\\.\\pipe\\ni_"
           << (tU32)::GetCurrentProcessId()
           << "_"
           << calls;

  sa.bInheritHandle = inheritRead;
  if ((*phRead = CreateNamedPipeA(pipeName.Chars(),
                                  PIPE_ACCESS_INBOUND,
                                  PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
                                  1,
                                  0,
                                  0,
                                  2000,
                                  &sa)) == NULL) {
    return FALSE;
  }

  sa.bInheritHandle = !inheritRead;
  if ((*phWrite = CreateFileA(pipeName.Chars(),
                              GENERIC_WRITE,
                              0, /* No sharing */
                              &sa,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL |
                              ((inheritRead) ? FILE_FLAG_OVERLAPPED : 0),
                              NULL)) == INVALID_HANDLE_VALUE) {
    CloseHandle(*phRead);
    return FALSE;
  }

  return TRUE;
}
#endif

static HANDLE _GetJobObject() {
  static HANDLE _hJobObject = NULL;
  if (!_hJobObject) {
    _hJobObject = ::CreateJobObject(NULL,NULL);
    if (!_hJobObject) {
      niError("Can't CreateJobObject");
    }
    else {
      JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
      // Configure all child processes associated with the job to
      // terminate when the current process terminates
      jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
      if (0 == SetInformationJobObject(
              _hJobObject, JobObjectExtendedLimitInformation,
              &jeli, sizeof(jeli)))
      {
        niError("Can't SetInformationJobObject");
        ::CloseHandle(_hJobObject);
        _hJobObject = NULL;
      }
    }
  }
  return _hJobObject;
};

static ni::iFile* _CreateFilePipeRead(HANDLE hFile, const ni::achar* aaszName, tBool abOwned = eTrue) {
  ni::iFile* fp = ni::CreateFile(
      niNew cOSWinFile(
          hFile,NULL,
          (abOwned)?0:eOSWinFileFlags_DontOwnRW|
          eOSWinFileFlags_Pipe,
          aaszName));
  return fp;
}
static ni::iFile* _CreateFilePipeWrite(HANDLE hFile, const ni::achar* aaszName, tBool abOwned = eTrue) {
  ni::iFile* fp = ni::CreateFile(
      niNew cOSWinFile(
          NULL,hFile,
          (abOwned)?0:eOSWinFileFlags_DontOwnRW|
          eOSWinFileFlags_Pipe,
          aaszName));
  return fp;
}
#endif

//--------------------------------------------------------------------------------------------
//
//  Posix Utils
//
//--------------------------------------------------------------------------------------------
#if defined niPosix && !defined niNoProcess
#include "FileFd.h"

static ni::iFile* _CreateFilePipeRead(int hFile, const ni::achar* aaszName, tBool abOwned = ni::eTrue) {
  ni::iFile* fp = ni::CreateFile(
      ni::CreateFileFd(
          hFile,-1,
          (abOwned)?0:eFileFdFlags_DontOwnRW|
          eFileFdFlags_Pipe,
          aaszName));
  return fp;
}
static ni::iFile* _CreateFilePipeWrite(int hFile, const ni::achar* aaszName, tBool abOwned = ni::eTrue) {
  ni::iFile* fp = ni::CreateFile(
      ni::CreateFileFd(
          -1,hFile,
          (abOwned)?0:eFileFdFlags_DontOwnRW|
          eFileFdFlags_Pipe,
          aaszName));
  return fp;
}
#endif

//--------------------------------------------------------------------------------------------
//
//  Utils
//
//--------------------------------------------------------------------------------------------
#if !defined niNoProcess
static tU32 _IsParentProcess(tInt aPID, tInt aParentPID) {
  tBool isParent = eFalse;
  tU32 c = 0;
  tInt cpid = aPID;
  while (cpid) {
    cpid = base::GetParentProcessFromPid(cpid);
    if (!cpid)
      break;
    ++c;
    if (cpid == aParentPID) {
      isParent = eTrue;
      break;
    }
  }
  return isParent ? c : 0;
}
#endif

//--------------------------------------------------------------------------------------------
//
//  Current Process
//
//--------------------------------------------------------------------------------------------
class cOSCurrentProcess : public ImplLocal<ni::iOSProcess> {
  niBeginClass(cOSCurrentProcess);
 public:
  cOSCurrentProcess() {
#if !defined niNoProcess
    _p = base::Process::Current();
#endif
  }
  ~cOSCurrentProcess() {
  }

  ///////////////////////////////////////////////
  virtual const ni::achar* __stdcall GetExePath() const {
    if (_exePath.empty()) {
      achar exePathBuff[AMAX_PATH];
      niThis(cOSCurrentProcess)->_exePath = ni_get_exe_path(exePathBuff);
    }
    return _exePath.Chars();
  }
  virtual const ni::achar* __stdcall GetCommandLine() const {
    if (_cmdLine.empty()) {
      niThis(cOSCurrentProcess)->_cmdLine = _GetCommandLine();
    }
    return _cmdLine.Chars();
  }

  ///////////////////////////////////////////////
  virtual tInt __stdcall GetPID() const {
#ifdef niNoProcess
    return 0;
#else
    return _p.pid();
#endif
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsCurrent() const {
#ifdef niNoProcess
    return eTrue;
#else
    return !!_p.is_current();
#endif
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetDidCrash() const {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Kill(tInt anExitCode, tBool abWait) {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Wait(tU32 anTimeMs) {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual sVec2i __stdcall WaitForExitCode(tU32 anTimeMs) {
    return Vec2<tI32>(0,0);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Terminate(tInt aResultCode) {
#if !defined niNoProcess
    _p.Terminate(aResultCode);
#endif
  }

  ///////////////////////////////////////////////
  virtual iFile* __stdcall GetFile(eOSProcessFile aFile) const {
    switch (aFile) {
      case eOSProcessFile_StdIn: {
#ifdef USE_WINDOWS_STD_HANDLE
        if (!_fpStdin.IsOK()) {
          HANDLE hStdin = ::GetStdHandle(STD_INPUT_HANDLE);
          if (hStdin) {
            niThis(cOSCurrentProcess)->_fpStdin =
                _CreateFilePipeRead(hStdin,_A("STDIN"),eFalse);
          }
        }
#endif
        if (!_fpStdin.IsOK()) {
          niThis(cOSCurrentProcess)->_fpStdin =
              ni::GetRootFS()->FileOpen(_A(":STDIN:"),eFileOpenMode_Read);
          if (!_fpStdin.IsOK()) {
            niError("Couldn't get the current process's STDIN.");
            return NULL;
          }
        }
        return _fpStdin;
      }
      case eOSProcessFile_StdOut: {
#ifdef USE_WINDOWS_STD_HANDLE
        if (!_fpStdout.IsOK()) {
          HANDLE hStdout = ::GetStdHandle(STD_OUTPUT_HANDLE);
          if (hStdout) {
            niThis(cOSCurrentProcess)->_fpStdout =
                _CreateFilePipeWrite(hStdout,_A("STDOUT"),eFalse);
          }
        }
#endif
        if (!_fpStdout.IsOK()) {
          niThis(cOSCurrentProcess)->_fpStdout =
              ni::GetRootFS()->FileOpen(_A(":STDOUT:"),eFileOpenMode_Read);
          if (!_fpStdout.IsOK()) {
            niError("Couldn't get the current process's STDOUT.");
            return NULL;
          }
        }
        return _fpStdout;
      }
      case eOSProcessFile_StdErr: {
#ifdef USE_WINDOWS_STD_HANDLE
        if (!_fpStderr.IsOK()) {
          HANDLE hStderr = ::GetStdHandle(STD_ERROR_HANDLE);
          if (hStderr) {
            niThis(cOSCurrentProcess)->_fpStderr =
                _CreateFilePipeWrite(hStderr,_A("STDERR"),eFalse);
          }
        }
#endif
        if (!_fpStderr.IsOK()) {
          niThis(cOSCurrentProcess)->_fpStderr =
              ni::GetRootFS()->FileOpen(_A(":STDERR:"),eFileOpenMode_Read);
          if (!_fpStderr.IsOK()) {
            niError("Couldn't get the current process's STDERR.");
            return NULL;
          }
        }
        return _fpStderr;
      }
      default: break;
    }
    return NULL;
  }

  ///////////////////////////////////////////////
  virtual tInt __stdcall GetParentPID() const {
#ifdef niNoProcess
    return 0;
#else
    tInt thisPID = GetPID();
    return thisPID ? base::GetParentProcessFromPid(thisPID) : 0;
#endif
  }
  virtual tU32 __stdcall IsParentProcess(tInt aParentPID) const {
#ifdef niNoProcess
    return 0;
#else
    tInt thisPID = GetPID();
    return thisPID ? _IsParentProcess(thisPID,aParentPID) : 0;
#endif
  }

 private:
#if !defined niNoProcess
  base::Process _p;
  inline base::ProcessHandle _GetHandle() const { return _p.handle(); }
#endif

  cString _exePath;
  cString _cmdLine;
  Ptr<iFile> _fpStdin;
  Ptr<iFile> _fpStdout;
  Ptr<iFile> _fpStderr;

  niEndClass(cOSCurrentProcess);
};

//--------------------------------------------------------------------------------------------
//
//  Spawned Process
//
//--------------------------------------------------------------------------------------------
#if !defined niNoProcess
class cOSProcess : public ImplRC<ni::iOSProcess> {
  niBeginClass(cOSProcess);
 public:
  cOSProcess(base::Process* aProc,
             const ni::achar* aaszExePath,
             const ni::achar* aaszCmdLine)
      : _p(aProc)
  {
    _exePath = aaszExePath;
    _cmdLine = aaszCmdLine;
  }
  ~cOSProcess() {
    Invalidate();
    if (_p) {
      delete _p;
      _p = NULL;
    }
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Invalidate() {
    niLoop(i,niCountOf(_files)) {
      if (_files[i].IsOK()) {
        _files[i]->Invalidate();
      }
    }
    if (_p) {
      _p->Close();
    }
  }

  ///////////////////////////////////////////////
  virtual const ni::achar* __stdcall GetExePath() const {
    if (_exePath.empty()) {
      niThis(cOSProcess)->_exePath = base::GetProcessExePathFromPid(GetPID());
    }
    return _exePath.Chars();
  }
  virtual const ni::achar* __stdcall GetCommandLine() const {
    if (_cmdLine.empty()) {
      // Only the exe path here, we can't get the command line of other
      // processes for security reason...
      niThis(cOSProcess)->_cmdLine = base::GetProcessExePathFromPid(GetPID());
    }
    return _cmdLine.Chars();
  }

  ///////////////////////////////////////////////
  virtual tInt __stdcall GetPID() const {
    return _p ? _p->pid() : 0;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsCurrent() const {
    return _p ? !!_p->is_current() : eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetDidCrash() const {
    return !!base::DidProcessCrash(_GetHandle());
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Kill(tInt anExitCode, tBool abWait) {
    return !!base::KillProcess(_GetHandle(),anExitCode,!!abWait);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Wait(tU32 anTimeMs) {
    return !!base::WaitForSingleProcess(_GetHandle(),anTimeMs);
  }

  ///////////////////////////////////////////////
  virtual sVec2i __stdcall WaitForExitCode(tU32 anTimeMs) {
    int exitCode = eInvalidHandle;
    bool r = base::WaitForExitCode(_GetHandle(),&exitCode,anTimeMs);
    return Vec2<tI32>(!!r,exitCode);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Terminate(tInt aResultCode) {
    _p->Terminate(aResultCode);
  }

  ///////////////////////////////////////////////
  virtual iFile* __stdcall GetFile(eOSProcessFile aFile) const {
    niCheckSilent(aFile < eOSProcessFile_Last,NULL);
    return _files[aFile];
  }
  void _SetFile(eOSProcessFile aFile, iFile* apFP) {
    _files[aFile] = apFP;
  }

  ///////////////////////////////////////////////
  virtual tInt __stdcall GetParentPID() const {
    tInt thisPID = GetPID();
    return thisPID ? base::GetParentProcessFromPid(thisPID) : 0;
  }
  virtual tU32 __stdcall IsParentProcess(tInt aParentPID) const {
    tInt thisPID = GetPID();
    return thisPID ? _IsParentProcess(thisPID,aParentPID) : 0;
  }

 private:
  cString _exePath;
  cString _cmdLine;
  base::Process* _p;
  Ptr<iFile> _files[eOSProcessFile_Last];
  inline base::ProcessHandle _GetHandle() const { return _p->handle(); }

  niEndClass(cOSProcess);
};

///////////////////////////////////////////////
static cOSProcess* __stdcall _CreateOSProcessFromHandle(
  base::ProcessHandle aHandle,
  int aPID,
  const ni::achar* aaszExePath,
  const ni::achar* aaszCmdLine)
{
  niAssert(aHandle != ((base::ProcessHandle)0));
  base::Process* p = new base::Process(aHandle,aPID);
  return niNew cOSProcess(p,aaszExePath,aaszCmdLine);
}
#endif

//--------------------------------------------------------------------------------------------
//
//  Process Manager
//
//--------------------------------------------------------------------------------------------
class cOSProcessManager : public ImplLocal<ni::iOSProcessManager> {
  niBeginClass(cOSProcessManager);

 public:
  ///////////////////////////////////////////////
  virtual tInt __stdcall GetCurrentProcessID() const {
    return this->GetCurrentProcess()->GetPID();
  }

  ///////////////////////////////////////////////
  iOSProcess* __stdcall GetCurrentProcess() const {
    static cOSCurrentProcess _current;
    return &_current;
  }

  ///////////////////////////////////////////////
  virtual iOSProcess* __stdcall CreateProcess(tInt aPID) {
#ifdef niNoProcess
    niError("Not implemented.");
    return nullptr;
#else
    base::ProcessHandle handle =
#ifdef niProcessProcessHandleNotPID
        base::OpenProcessHandle(aPID)
#else
        aPID
#endif
        ;
    niCheck(handle != ((base::ProcessHandle)0),NULL);
    return _CreateOSProcessFromHandle(handle,aPID,AZEROSTR,AZEROSTR);
#endif
  }

  ///////////////////////////////////////////////
  virtual iOSProcess* __stdcall SpawnProcess(
      const ni::achar* aaszCmdLine,
      tOSProcessSpawnFlags aSpawn)
  {
#ifdef niNoProcess
    niError("Not implemented.");
    return nullptr;
#else
    return SpawnProcessEx(aaszCmdLine,NULL,NULL,aSpawn);
#endif
  }

  ///////////////////////////////////////////////
  virtual iOSProcess* __stdcall SpawnProcessEx(
      const ni::achar* aaszCmdLine,
      const achar* aaszWorkDir,
      const tStringCMap* apEnvs,
      tOSProcessSpawnFlags aSpawn)
  {
#ifdef niNoProcess
    niError("Not implemented.");
    return nullptr;
#else
    niCheck(niStringIsOK(aaszCmdLine),NULL);
    cString strExePath;
    cString strCmdLine = aaszCmdLine;
    {
      strCmdLine.Trim();
      tU32 char0 = strCmdLine[0];
      if (char0 == '\"' || char0 == '\'') {
        int lastDelim = strCmdLine.find(char0,1);
        if (lastDelim <= 1) {
          niError(niFmt("Invalid command line '%s'",aaszCmdLine));
          return NULL;
        }
        strExePath = strCmdLine.slice(1,lastDelim);
      }
      else {
        strExePath = strCmdLine.Before(" ");
        if (strExePath.IsEmpty()) {
          strExePath = strCmdLine;
          strExePath.Normalize();
        }
      }
    }
    if (strExePath.length() < 1) {
      niError(niFmt("Invalid executable name in command line '%s'",aaszCmdLine));
      return NULL;
    }

    Ptr<tStringCMap> defaultEnvs;
    if (!apEnvs) {
      defaultEnvs = this->GetEnvs();
      apEnvs = defaultEnvs.ptr();
    }

#ifdef niWindows
    //
    // Windows Spawn
    //

    base::ProcessHandle handle = NULL;
    Ptr<cOSProcess> p;
    bool r = false;

    ni::Buffer env;
    for (tStringCMap::const_iterator it = apEnvs->begin(); it != apEnvs->end(); ++it) {
      env.AppendUTF8ToUTF16(it->first.Chars(),it->first.size(),eFalse);
      env.AppendUTF8ToUTF16("=",1,eFalse);
      env.AppendUTF8ToUTF16(it->second.Chars(),it->second.size(),eTrue);
    }
    env.AppendUTF8ToUTF16("\0",0,eTrue);

    const tBool sameOutAndErr = niFlagIsNot(aSpawn,eOSProcessSpawnFlags_DifferentStdOutAndStdErr);
    HANDLE hToChildStdin = INVALID_HANDLE_VALUE; /* Write handle to child. */
    HANDLE hFromChildStdout = INVALID_HANDLE_VALUE; /* Read handle from child. */
    HANDLE hFromChildStderr = INVALID_HANDLE_VALUE; /* Read handle from child. */
    HANDLE hChildStdin = INVALID_HANDLE_VALUE;    /* Child's stdin. */
    HANDLE hChildStdout = INVALID_HANDLE_VALUE; /* Child's stout. */
    HANDLE hChildStderr = INVALID_HANDLE_VALUE; /* Child's sterr. */

    if (niFlagIs(aSpawn,eOSProcessSpawnFlags_StdFiles)) {
      if (!_CreateAnonPipes(&hFromChildStdout,&hChildStdout,FALSE)) {
        niError("Can't create stdout pipe.");
        goto error;
      }
      if (!sameOutAndErr) {
        if (!_CreateAnonPipes(&hFromChildStderr,&hChildStderr,FALSE)) {
          niError("Can't create stderr pipe.");
          goto error;
        }
      }
      else {
        hFromChildStderr = hFromChildStdout;
        hChildStderr = hChildStdout;
      }
      if (!_CreateAnonPipes(&hChildStdin,&hToChildStdin,TRUE)) {
        niError("Can't create stdin pipe.");
        goto error;
      }
    }

    r = base::LaunchApp(strCmdLine.Chars(),
                        hChildStdin,hChildStdout,hChildStderr,
                        niStringIsOK(aaszWorkDir) ? aaszWorkDir : NULL,
                        (const WCHAR*)env.GetData(),
                        &handle,
                        aSpawn);
    if (niFlagIs(aSpawn,eOSProcessSpawnFlags_StdFiles)) {
      if (WIN_HANDLE_IS_VALID(hChildStdin)) {
        CloseHandle(hChildStdin);
      }
      if (WIN_HANDLE_IS_VALID(hChildStdout)) {
        CloseHandle(hChildStdout);
      }
      if (!sameOutAndErr) {
        if (WIN_HANDLE_IS_VALID(hChildStderr)) {
          CloseHandle(hChildStderr);
        }
      }
    }
    if (!r) {
      niError(niFmt("Can't launch process: %s", strCmdLine));
      goto error;
    }
    if (!handle) {
      niError(niFmt("Can't get process handle: %s", strCmdLine));
      goto error;
    }
    p = _CreateOSProcessFromHandle(
        handle,
        base::GetProcIdFromHandle(handle),
        strExePath.Chars(),
        strCmdLine.Chars());
    if (!p.IsOK()) {
      niError(niFmt("Can't create os process object: %s", strCmdLine));
      goto error;
    }

    // Initialize the file handles of the child process
    if (niFlagIs(aSpawn,eOSProcessSpawnFlags_StdFiles)) {
      {
        cString inName;
        inName << (tI32)p->GetPID() << "_STDIN_" << p->GetExePath();
        Ptr<iFile> fpIn = _CreateFilePipeWrite(hToChildStdin,inName.Chars());
        p->_SetFile(eOSProcessFile_StdIn,fpIn);
      }
      Ptr<iFile> fpOut;
      {
        cString outName;
        outName << (tI32)p->GetPID() << "_STDOUT_" << p->GetExePath();
        fpOut = _CreateFilePipeRead(hFromChildStdout,outName.Chars());
        p->_SetFile(eOSProcessFile_StdOut,fpOut);
      }
      if (sameOutAndErr) {
        p->_SetFile(eOSProcessFile_StdErr,fpOut);
      }
      else {
        cString errName;
        errName << (tI32)p->GetPID() << "_STDERR_" << p->GetExePath();
        Ptr<iFile> fpErr = _CreateFilePipeRead(hFromChildStderr,errName.Chars());
        p->_SetFile(eOSProcessFile_StdErr,fpErr);
      }
    }

    return p.GetRawAndSetNull();

 error:
    if (WIN_HANDLE_IS_VALID(hFromChildStdout)) {
      CloseHandle(hFromChildStdout);
    }
    if (!sameOutAndErr) {
      if (WIN_HANDLE_IS_VALID(hFromChildStderr)) {
        CloseHandle(hFromChildStderr);
      }
    }
    if (WIN_HANDLE_IS_VALID(hToChildStdin)) {
      CloseHandle(hToChildStdin);
    }
    if (handle) {
      ::TerminateProcess(handle,0xDEADBEEF);
    }
    return NULL;

#else
    //
    // POSIX Spawn
    //

    astl::vector<cString> args;
    cCommandLineStringTokenizer tokCmdLine;
    StringTokenize(strCmdLine,args,&tokCmdLine);

    // niDebugFmt(("... aaszCmdLine: %s", aaszCmdLine));
    // niDebugFmt(("... exePath: %s", strExePath));
    // niDebugFmt(("... args: %d", args.size()));
    niLoop(i,args.size()) {
      // unquote the string, spawnp doesn't handle them
      args[i] = args[i].StripQuotes();
      // niDebugFmt(("... arg[%d]: '%s'", i, args[i]));
    }

#define PIPE_READ 0
#define PIPE_WRITE 1
    int pipeStdin[2] = {-1,-1};
    int pipeStdout[2] = {-1,-1};
    int pipeStderr[2] = {-1,-1};
    int hToChildStdin = -1, hFromChildStdout = -1, hFromChildStderr = -1;
    Ptr<cOSProcess> p;
    base::ProcessHandle handle = NULL;
    const tBool sameOutAndErr = niFlagIsNot(aSpawn,eOSProcessSpawnFlags_DifferentStdOutAndStdErr);
    astl::vector<ni::cString> vEnvs;

    base::file_handle_mapping_vector fds_to_remap;
    if (niFlagIs(aSpawn,eOSProcessSpawnFlags_StdFiles)) {
      if (pipe(pipeStdin) < 0) {
        niError("Can't create stdin pipe.");
        goto error;
      }
      if (pipe(pipeStdout) < 0) {
        niError("Can't create stdout pipe.");
        goto error;
      }
      if (sameOutAndErr) {
        pipeStderr[0] = pipeStdout[0];
        pipeStderr[1] = pipeStdout[1];
      }
      else {
        if (pipe(pipeStderr) < 0) {
          niError("Can't create stderr pipe.");
          goto error;
        }
      }
      fds_to_remap.push_back(eastl::make_pair(pipeStdin[PIPE_READ],STDIN_FILENO));
      fds_to_remap.push_back(eastl::make_pair(pipeStdout[PIPE_WRITE],STDOUT_FILENO));
      fds_to_remap.push_back(eastl::make_pair(pipeStderr[PIPE_WRITE],STDERR_FILENO));
      hToChildStdin = pipeStdin[PIPE_WRITE];
      hFromChildStdout = pipeStdout[PIPE_READ];
      hFromChildStderr = pipeStderr[PIPE_READ];
    }
    else {
      fds_to_remap.push_back(eastl::make_pair(STDIN_FILENO,STDIN_FILENO));
      fds_to_remap.push_back(eastl::make_pair(STDOUT_FILENO,STDOUT_FILENO));
      fds_to_remap.push_back(eastl::make_pair(STDERR_FILENO,STDERR_FILENO));
    }

    for (ni::tStringCMap::const_iterator itEnv = apEnvs->begin();
         itEnv != apEnvs->end(); ++itEnv)
    {
      ni::cString& envStr = astl::push_back(vEnvs);
      envStr += itEnv->first;
      envStr += "=";
      envStr += itEnv->second;
    }

    if (!base::LaunchApp(args,fds_to_remap,&handle,vEnvs)) {
      niError(niFmt("Can't launch process: %s", strCmdLine));
      goto error;
    }

    p = _CreateOSProcessFromHandle(
        handle,handle,
        strExePath.Chars(),
        strCmdLine.Chars());
    if (!p.IsOK()) {
      niError(niFmt("Can't create os process object: %s", strCmdLine));
      goto error;
    }

    // Initialize the file handles of the child process
    if (niFlagIs(aSpawn,eOSProcessSpawnFlags_StdFiles)) {
      // those pipes have been duped, close them since we wont use them anymore
      close(pipeStdin[PIPE_READ]);
      close(pipeStdout[PIPE_WRITE]);
      if (!sameOutAndErr) {
        close(pipeStderr[PIPE_WRITE]);
      }

      // link the other end of the pipes to a file object
      {
        niAssert(hToChildStdin >= 0);
        cString inName;
        inName << (tI32)p->GetPID() << "_STDIN_" << p->GetExePath();
        Ptr<iFile> fpIn = _CreateFilePipeWrite(hToChildStdin,inName.Chars());
        p->_SetFile(eOSProcessFile_StdIn,fpIn);
      }
      Ptr<iFile> fpOut;
      {
        niAssert(hFromChildStdout >= 0);
        cString outName;
        outName << (tI32)p->GetPID() << "_STDOUT_" << p->GetExePath();
        fpOut = _CreateFilePipeRead(hFromChildStdout,outName.Chars());
        p->_SetFile(eOSProcessFile_StdOut,fpOut);
      }
      if (sameOutAndErr) {
        p->_SetFile(eOSProcessFile_StdErr,fpOut);
      }
      else {
        niAssert(hFromChildStderr >= 0);
        cString errName;
        errName << (tI32)p->GetPID() << "_STDERR_" << p->GetExePath();
        Ptr<iFile> fpErr = _CreateFilePipeRead(hFromChildStderr,errName.Chars());
        p->_SetFile(eOSProcessFile_StdErr,fpErr);
      }
    }

    return p.GetRawAndSetNull();

 error:
    if (pipeStdin[PIPE_READ])   { close(pipeStdin[PIPE_READ]); }
    if (pipeStdin[PIPE_WRITE])  { close(pipeStdin[PIPE_WRITE]); }
    if (pipeStdout[PIPE_READ])  { close(pipeStdout[PIPE_READ]); }
    if (pipeStdout[PIPE_WRITE]) { close(pipeStdout[PIPE_WRITE]); }
    if (!sameOutAndErr) {
      if (pipeStderr[PIPE_READ])  { close(pipeStderr[PIPE_READ]); }
      if (pipeStderr[PIPE_WRITE]) { close(pipeStderr[PIPE_WRITE]); }
    }
    return NULL;
#endif
#endif // niNoProcess
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall EnumProcesses(ni::iRegex* apFilter, iOSProcessEnumSink* apSink) {
    tU32 c = 0;

#if !defined niNoProcess
    struct MyProcessFilter : public base::ProcessFilter {
      virtual bool Includes(ni::tI32 pid, ni::tI32 parent_pid) const {
        return true; // include all...
      }
    } filter;

    base::NamedProcessIterator pit(L"",&filter);
    while (1) {
      const ProcessEntry* pe = pit.NextProcessEntry();
      if (!pe) break;
      cString exePath = pe->szExeFile;
      if (apFilter && !apFilter->DoesMatch(exePath.Chars()))
        continue; // skip
      if (apSink) {
#ifdef niWindows
        if (!apSink->OnOSProcessEnumSink(pe->th32ProcessID,pe->th32ParentProcessID,exePath.Chars()))
          break;
#else
        if (!apSink->OnOSProcessEnumSink(pe->pid,pe->ppid,exePath.Chars()))
          break;
#endif
      }
      ++c;
    }
#endif

    return c;
  }

  cString __stdcall GetCwd() const {
    return agetcwd();
  }

  Ptr<tStringCMap> __stdcall GetEnvs() const {
    Ptr<tStringCMap> e = tStringCMap::Create();
    for (char **current = _GetEnviron(); *current; current++) {
      cString c = *current;
      astl::upsert(*e, c.Before("="), c.After("="));
    }
    return e;
  }

 private:
  niEndClass(cOSProcessManager);
};

//--------------------------------------------------------------------------------------------
//
//  API
//
//--------------------------------------------------------------------------------------------
namespace ni {

niExportFunc(ni::iOSProcessManager*) GetOSProcessManager() {
  static cOSProcessManager _pman;
  return &_pman;
}

}

#endif // #if niMinFeatures(15)
