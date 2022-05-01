// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "Platform.h"

#if defined niWinDesktop && !defined niNoProcess

#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#ifdef niMSVC
#include <winternl.h>
#endif
#include <psapi.h>
#include "API/niLang/Platforms/Win32/Win32_UTF.h"
#pragma comment(lib,"psapi.lib")
// #include <strsafe.h>

#ifdef niGCC
#include <ntsecapi.h>
#include <ddk/ntddk.h>
#include <ddk/ntapi.h>
#endif

#ifndef PROCESS_QUERY_LIMITED_INFORMATION
#define PROCESS_QUERY_LIMITED_INFORMATION  (0x1000)
#endif

//--------------------------------------------------------------------------------------------
//
//  GetProcessExePath
//
//--------------------------------------------------------------------------------------------

FARPROC WINAPI GetModuleProcAddress(LPCTSTR moduleName, LPCSTR lpProcName) {
  HMODULE h = GetModuleHandle(moduleName);
  if (!h) return NULL;
  return GetProcAddress(h,lpProcName);
}

#ifndef niMSVC
static BOOL WINAPI GetProcessExePath(DWORD anPID, WCHAR* apDestBuffer, DWORD anDestBufferSize) {
  return FALSE;
}
#else
static BOOL InPlaceTranslateDevicePathToDrivePath(WCHAR* pszFilename, DWORD anFileNameMaxSize);
typedef BOOL (WINAPI *tpfnQueryFullProcessImageName)(HANDLE hProcess, DWORD dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);
typedef BOOL (WINAPI *tpfnGetProcessImageFileName)(HANDLE hProcess, LPWSTR lpImageFileName, DWORD nSize);

/// <summary>Retrieves the file path of the executable used to launch the specified process.</summary>
/// <param name="anPID">The process identifier of the process.</param>
/// <param name="apDestBuffer">WCHAR array where to copy the file path of executable.</param>
/// <param name="anDestBufferSize">Maximum size, in characters, of the destination buffer.</param>
/// <returns>TRUE on success. FALSE if a parameter is invalid or if the function can't retrieve the executable path.</returns>
static BOOL WINAPI GetProcessExePath(DWORD anPID, WCHAR* apDestBuffer, DWORD anDestBufferSize) {
  // our function pointers
  tpfnQueryFullProcessImageName pfnQueryFullProcessImageName = NULL;
  tpfnGetProcessImageFileName pfnGetProcessImageFileName = NULL;
  // Process Handle
  HANDLE hProcess = NULL;

  // Make sure input parameters are valid.
  niAssert(apDestBuffer != NULL);
  if (!apDestBuffer) return FALSE;
  niAssert(anDestBufferSize != 0);
  if (anDestBufferSize == 0) return FALSE;

  // Set the first character of the buffer to zero so that we can use it to check
  // if we successfully retrieved the path down the line.
  *apDestBuffer = 0;

  // Entry point for 'QueryFullProcessImageName', Vista+ only
  pfnQueryFullProcessImageName = (tpfnQueryFullProcessImageName)GetModuleProcAddress(
      _T("kernel32"),
      "QueryFullProcessImageNameW");
  if (pfnQueryFullProcessImageName) {
    hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, anPID);
    if (hProcess) { // can fail due to invalid PID
      DWORD maxLen = anDestBufferSize;
      __try {
        pfnQueryFullProcessImageName(hProcess,0,apDestBuffer,&maxLen);
      }
      __finally {
      }
    }
  }
  else {
    // No QueryFullProcessImageNameW try with GetModuleHandle, works on Win2K+ (should also in Win98...)
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, anPID);
    // Can fail due to invalid PID or insufficiant privileges (PROCESS_VM_READ is a potential issue)
    if (hProcess) {
      __try {
        // this might throw an exception on WoW64 when doing cross arch queries...
        ::GetModuleFileNameExW(hProcess,0,apDestBuffer,anDestBufferSize);
      }
      __finally {
      }
    }

    // Can't open the process with these rights or can't get exePath with GetModuleFileNameEx
    // (this can happen if we're a 32bit proc and we're querying a 64bit proc, or vice versa)
    if (!*apDestBuffer) {
      if (!hProcess) {
        // Couldn't open the process, try again with the minimum rights needed by GetProcessImageFileName
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, anPID);
      }
      if (hProcess) {
        // Entry point for 'GetProcessImageFileName', WinXP+ only
        pfnGetProcessImageFileName = (tpfnGetProcessImageFileName)GetModuleProcAddress(
            _T("psapi"),
#ifdef _UNICODE
            "GetProcessImageFileNameW"
#else
            "GetProcessImageFileNameA"
#endif
                                                                                       );
        if (pfnGetProcessImageFileName) {
          __try {
            pfnGetProcessImageFileName(hProcess,apDestBuffer,anDestBufferSize);
            InPlaceTranslateDevicePathToDrivePath(apDestBuffer,anDestBufferSize);
          }
          __finally {
          }
        }
      }
    }
  }

  if (hProcess) {
    CloseHandle(hProcess);
  }
  return (*apDestBuffer) ? TRUE : FALSE;
}

// Translate path with device name to drive letters.
// Most of the code is taken from the MSDN example "Obtaining a File Name From a File Handle" (http://msdn.microsoft.com/en-us/library/aa366789(VS.85).aspx)
static BOOL InPlaceTranslateDevicePathToDrivePath(WCHAR* pszFilename, DWORD anFileNameMaxSize) {
  WCHAR szTemp[MAX_PATH] = {0};
  if (::GetLogicalDriveStringsW(MAX_PATH-1, szTemp)) {
    WCHAR szName[MAX_PATH];
    WCHAR szDrive[3] = L" :";
    BOOL bFound = FALSE;
    WCHAR* p = szTemp;
    do {
      // Copy the drive letter to the template string
      *szDrive = *p;
      // Look up each device name
      if (::QueryDosDeviceW(szDrive, szName, MAX_PATH)) {
        size_t uNameLen = wcslen(szName);
        if (uNameLen < MAX_PATH) {
          bFound = _wcsnicmp(pszFilename, szName, uNameLen) == 0;
          if (bFound) {
            // Reconstruct pszFilename using szTempFile
            // Replace device path with DOS path
            WCHAR szTempFile[MAX_PATH];
            StringCchPrintfW(szTempFile,
                             _MAX_PATH,
                             L"%s%s",
                             szDrive,
                             pszFilename+uNameLen);
            StringCchCopyNW(pszFilename, anFileNameMaxSize, szTempFile, wcslen(szTempFile));
          }
        }
      }
      // Go to the next NULL character.
      while (*p++);
    } while (!bFound && *p); // end of string
    return bFound;
  }
  return FALSE;
}
#endif // #ifdef niMSVC

//--------------------------------------------------------------------------------------------
//
//  Process
//
//--------------------------------------------------------------------------------------------
namespace base {

void Process::Close() {
  if (!process_)
    return;
  // PIERRE: This cause crashes. In debug mode this throws an "handle invalid" exception.
  // ::CloseHandle(process_);
  process_ = NULL;
  pid_ = 0;
}

void Process::Terminate(int result_code) {
  if (!process_)
    return;
  ::TerminateProcess(process_, result_code);
}

bool Process::is_current() const {
  HANDLE curProc = ::GetCurrentProcess();
  return process_ == curProc;
}

// static
Process Process::Current() {
  return Process(::GetCurrentProcess(),::GetCurrentProcessId());
}

}  // namespace base

//--------------------------------------------------------------------------------------------
//
//  Process Utils
//
//--------------------------------------------------------------------------------------------
namespace {

// System pagesize. This value remains constant on x86/64 architectures.
const int PAGESIZE_KB = 4;

// HeapSetInformation function pointer.
typedef BOOL (WINAPI* HeapSetFn)(HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T);

}  // namespace

namespace base {

int GetCurrentProcId() {
  return ::GetCurrentProcessId();
}

ProcessHandle GetCurrentProcessHandle() {
  return ::GetCurrentProcess();
}

ProcessHandle OpenProcessHandle(int pid) {
  return OpenProcess(
      SYNCHRONIZE | PROCESS_DUP_HANDLE |
      PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION,
      FALSE, pid);
}

void CloseProcessHandle(ProcessHandle process) {
  CloseHandle(process);
}

// Helper for GetProcId()
static bool _GetProcIdViaGetProcessId(base::ProcessHandle process, DWORD* id) {
  // Dynamically get a pointer to GetProcessId().
  typedef DWORD (WINAPI *GetProcessIdFunction)(HANDLE);
  static GetProcessIdFunction GetProcessIdPtr = NULL;
  static bool initialize_get_process_id = true;
  if (initialize_get_process_id) {
    initialize_get_process_id = false;
    HMODULE kernel32_handle = ::GetModuleHandleW(L"kernel32.dll");
    if (!kernel32_handle) {
      niAssertUnreachable("Can't get handle of kernel32.dll.");
      return false;
    }
    GetProcessIdPtr = reinterpret_cast<GetProcessIdFunction>(
        GetProcAddress(kernel32_handle, "GetProcessId"));
  }
  if (!GetProcessIdPtr)
    return false;
  // Ask for the process ID.
  *id = (*GetProcessIdPtr)(process);
  return true;
}

// Helper for GetProcId()
static bool _GetProcIdViaNtQueryInformationProcess(base::ProcessHandle process, DWORD* id) {
  // Dynamically get a pointer to NtQueryInformationProcess().
  typedef NTSTATUS (WINAPI *NtQueryInformationProcessFunction)(
      HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
  static NtQueryInformationProcessFunction NtQueryInformationProcessPtr = NULL;
  static bool initialize_query_information_process = true;
  if (initialize_query_information_process) {
    initialize_query_information_process = false;
    // According to nsylvain, ntdll.dll is guaranteed to be loaded, even though
    // the Windows docs seem to imply that you should LoadLibrary() it.
    HMODULE ntdll_handle = GetModuleHandle(_A("ntdll.dll"));
    if (!ntdll_handle) {
      niAssertUnreachable("Can't get handle of ntdll.dll.");
      return false;
    }
    NtQueryInformationProcessPtr =
        reinterpret_cast<NtQueryInformationProcessFunction>(
            GetProcAddress(ntdll_handle, "NtQueryInformationProcess"));
  }
  if (!NtQueryInformationProcessPtr)
    return false;
  // Ask for the process ID.
  PROCESS_BASIC_INFORMATION info;
  ULONG bytes_returned;
  NTSTATUS status = (*NtQueryInformationProcessPtr)(process,
                                                    ProcessBasicInformation,
                                                    &info, sizeof info,
                                                    &bytes_returned);
  if (!SUCCEEDED(status) || (bytes_returned != (sizeof info)))
    return false;

  *id = static_cast<DWORD>(info.UniqueProcessId);
  return true;
}

int GetProcIdFromHandle(ProcessHandle process) {
  // Get a handle to |process| that has PROCESS_QUERY_INFORMATION rights.
  HANDLE current_process = GetCurrentProcess();
  HANDLE process_with_query_rights;
  if (DuplicateHandle(current_process, process, current_process,
                      &process_with_query_rights, PROCESS_QUERY_INFORMATION,
                      false, 0)) {
    // Try to use GetProcessId(), if it exists.  Fall back on
    // NtQueryInformationProcess() otherwise (< Win XP SP1).
    DWORD id;
    bool success =
        _GetProcIdViaGetProcessId(process_with_query_rights, &id) ||
        _GetProcIdViaNtQueryInformationProcess(process_with_query_rights, &id);
    CloseHandle(process_with_query_rights);
    if (success)
      return id;
  }

  // We're screwed.
  niAssertUnreachable("Unreachable.");
  return 0;
}

bool LaunchApp(const ni::achar* aaszCmdLine,
               HANDLE hStdin, HANDLE hStdout, HANDLE hStderr,
               const ni::achar* aaszWorkDir,
               const WCHAR* apEnv,
               ProcessHandle* process_handle,
               ni::tOSProcessSpawnFlags aSpawnFlags)
{
  int createFlags = GetPriorityClass(GetCurrentProcess());
  createFlags |= CREATE_UNICODE_ENVIRONMENT;
  if (niFlagIs(aSpawnFlags,ni::eOSProcessSpawnFlags_Detached)) {
    createFlags |= DETACHED_PROCESS;
  }
  else {
    // Indicate that no console window should be created for child console
    // processes. Without this our windowed apps popup an empty console window
    // every time they run a command line tool (such as the package exporter
    // or shader compiler).
    createFlags |= CREATE_NO_WINDOW;
  }

  PROCESS_INFORMATION process_info;
  ni::MemZero((ni::tPtr)&process_info,sizeof(process_info));

  STARTUPINFOW startup_info;
  ni::MemZero((ni::tPtr)&startup_info,sizeof(startup_info));
  startup_info.cb = sizeof(startup_info);

  if (hStdin && hStdout && hStderr) {
    startup_info.dwFlags |= STARTF_USESTDHANDLES;
    startup_info.hStdInput = hStdin;
    startup_info.hStdOutput = hStdout;
    startup_info.hStdError = hStderr;
  }

  ni::Windows::UTF16Buffer wCmdLine;
  niWin32_UTF8ToUTF16(wCmdLine,aaszCmdLine);
  ni::Windows::UTF16Buffer wWorkDir;
  niWin32_UTF8ToUTF16(wWorkDir,aaszWorkDir);

  if (niFlagIs(aSpawnFlags,ni::eOSProcessSpawnFlags_SandBox)) {
    BOOL tryRegular = TRUE;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    HANDLE hNewToken = NULL;
    SID_IDENTIFIER_AUTHORITY MLAuthority = SECURITY_MANDATORY_LABEL_AUTHORITY;
    PSID pIntegritySid = NULL;
    TOKEN_MANDATORY_LABEL tml = { 0 };

    // Open the primary access token of the process.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY |
                          TOKEN_ADJUST_DEFAULT | TOKEN_ASSIGN_PRIMARY, &hToken))
    {
      dwError = GetLastError();
      niDebugFmt(("OpenProcessToken failed: 0x%x",dwError));
      goto Cleanup;
    }

    // Duplicate the primary token of the current process.
    if (!DuplicateTokenEx(hToken, 0, NULL, SecurityImpersonation,
                          TokenPrimary, &hNewToken))
    {
      dwError = GetLastError();
      niDebugFmt(("DuplicateTokenEx failed: 0x%x",dwError));
      goto Cleanup;
    }

    // Create the low integrity SID.
    if (!AllocateAndInitializeSid(&MLAuthority, 1, SECURITY_MANDATORY_LOW_RID,
                                  0, 0, 0, 0, 0, 0, 0, &pIntegritySid))
    {
      dwError = GetLastError();
      niDebugFmt(("AllocateAndInitializeSid failed: 0x%x",dwError));
      goto Cleanup;
    }

    tml.Label.Attributes = SE_GROUP_INTEGRITY;
    tml.Label.Sid = pIntegritySid;

    // Set the integrity level in the access token to low.
    if (!SetTokenInformation(hNewToken, TokenIntegrityLevel, &tml,
                             (sizeof(tml) + GetLengthSid(pIntegritySid))))
    {
      dwError = GetLastError();
      niDebugFmt(("SetTokenInformation failed: 0x%x",dwError));
      goto Cleanup;
    }

    // Create the new process at the Low integrity level.
    tryRegular = FALSE;
    if (!::CreateProcessAsUserW(
            hNewToken, NULL, wCmdLine.begin(), NULL, NULL,
            FALSE, createFlags, (LPVOID)apEnv,
            niStringIsOK(aaszWorkDir) ? wWorkDir.begin() : NULL,
            &startup_info, &process_info))
    {
      dwError = GetLastError();
      niDebugFmt(("CreateProcessAsUser failed: 0x%x",dwError));
      goto Cleanup;
    }

 Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken) {
      CloseHandle(hToken);
      hToken = NULL;
    }
    if (hNewToken) {
      CloseHandle(hNewToken);
      hNewToken = NULL;
    }
    if (pIntegritySid) {
      FreeSid(pIntegritySid);
      pIntegritySid = NULL;
    }

    if (ERROR_SUCCESS != dwError) {
      if (tryRegular) {
        niDebugFmt(("Fallback to spawn regular process."));
        if (process_info.hProcess) {
          CloseHandle(process_info.hProcess);
          process_info.hProcess = NULL;
        }
        if (process_info.hThread) {
          CloseHandle(process_info.hThread);
          process_info.hThread = NULL;
        }
      }
      else {
        // Make sure that the error code is set for failure.
        SetLastError(dwError);
        return false;
      }
    }
  }

  if (!process_info.hProcess) {
    if (!::CreateProcessW(NULL, // application name
                          wCmdLine.begin(),
                          NULL, // process attrs
                          NULL, // thread attrs
                          TRUE, // inheritHandles ?
                          createFlags,
                          (LPVOID)apEnv,  // environment
                          niStringIsOK(aaszWorkDir) ? wWorkDir.begin() : NULL,  // current directory
                          &startup_info,
                          &process_info))
    {
      return false;
    }
  }

  // Handles must be closed or they will leak
  if (process_info.hThread) {
    CloseHandle(process_info.hThread);
  }

  // If the caller wants the process handle, we won't close it.
  if (process_handle) {
    *process_handle = process_info.hProcess;
  }
  else if (process_info.hProcess) {
    CloseHandle(process_info.hProcess);
  }
  return true;
}

// Attempts to kill the process identified by the given process
// entry structure, giving it the specified exit code.
// Returns true if this is successful, false otherwise.
bool KillProcessById(DWORD process_id, int exit_code, bool wait) {
  HANDLE process = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE,
                               FALSE,  // Don't inherit handle
                               process_id);
  if (!process)
    return false;

  bool ret = KillProcess(process, exit_code, wait);
  CloseHandle(process);
  return ret;
}

bool KillProcess(ProcessHandle process, int exit_code, bool wait) {
  bool result = (TerminateProcess(process, exit_code) != FALSE);
  if (result && wait) {
    // The process may not end immediately due to pending I/O
    if (WAIT_OBJECT_0 != ::WaitForSingleObject(process, 60 * 1000)) {
      niError(niFmt(_A("Error waiting for process exit: %d"), GetLastError()));
    }
  } else if (!result) {
    niError(niFmt(_A("Unable to terminate process: %d"), GetLastError()));
  }
  return result;
}

bool DidProcessCrash(ProcessHandle handle) {
  DWORD exitcode = 0;
  if (!::GetExitCodeProcess(handle, &exitcode)) {
    //    niAssert(0);
    return false;
  }

  if (exitcode == STILL_ACTIVE) {
    // The process is likely not dead or it used 0x103 as exit code.
    //    niAssert(0);
    return false;
  }

  // Warning, this is not generic code; it heavily depends on the way
  // the rest of the code kills a process.

  if (exitcode == PROCESS_END_NORMAL_TERMINATON ||
      exitcode == PROCESS_END_KILLED_BY_USER ||
      exitcode == PROCESS_END_PROCESS_WAS_HUNG ||
      exitcode == 0xC0000354 ||   // STATUS_DEBUGGER_INACTIVE.
      exitcode == 0xC000013A ||   // Control-C/end session.
      exitcode == 0x40010004) {   // Debugger terminated process/end session.
    return false;
  }

  // All other exit codes indicate crashes.
  return true;
}

NamedProcessIterator::NamedProcessIterator(const astl::string& executable_name,
                                           const ProcessFilter* filter)
    : executable_name_(executable_name),
      started_iteration_(false),
      filter_(filter)
{
  snapshot_ = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
}

NamedProcessIterator::~NamedProcessIterator() {
  CloseHandle(snapshot_);
}


const ProcessEntry* NamedProcessIterator::NextProcessEntry() {
  bool result = false;
  do {
    result = CheckForNextProcess();
  } while (result && !IncludeEntry());

  if (result) {
    return &entry_;
  }

  return NULL;
}

bool NamedProcessIterator::CheckForNextProcess() {
  InitProcessEntry(&entry_);

  if (!started_iteration_) {
    started_iteration_ = true;
    return !!Process32FirstW(snapshot_, &entry_);
  }

  return !!Process32NextW(snapshot_, &entry_);
}

bool NamedProcessIterator::IncludeEntry() {
  ni::Windows::UTF8Buffer wExeFile;
  niWin32_UTF16ToUTF8(wExeFile,entry_.szExeFile);
  return
      (executable_name_.empty() ||
       stricmp(executable_name_.c_str(), wExeFile.begin()) == 0) &&
      (!filter_ || filter_->Includes(entry_.th32ProcessID,
                                     entry_.th32ParentProcessID));
}

void NamedProcessIterator::InitProcessEntry(ProcessEntry* entry) {
  memset(entry, 0, sizeof(*entry));
  entry->dwSize = sizeof(*entry);
}

typedef struct _tagMY_PROCESS_BASIC_INFORMATION
{
  PVOID ExitStatus;
  PVOID PebBaseAddress;
  PVOID AffinityMask;
  PVOID BasePriority;
  PVOID UniqueProcessId;
  ULONG_PTR InheritedFromUniqueProcessId;
}   MY_PROCESS_BASIC_INFORMATION;
niCAssert(sizeof(MY_PROCESS_BASIC_INFORMATION) == sizeof(PROCESS_BASIC_INFORMATION));
typedef NTSTATUS (WINAPI *tpfnNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
                                                         );

ni::tIntPtr GetParentProcessFromPid(ni::tIntPtr dwPID) {

  // Entry point for 'NtQueryInformationProcess'
  tpfnNtQueryInformationProcess pfnNtQueryInformationProcess;
  pfnNtQueryInformationProcess = (tpfnNtQueryInformationProcess)GetModuleProcAddress(
      _T("ntdll"),"NtQueryInformationProcess");
  niAssert(pfnNtQueryInformationProcess != NULL);
  if (!pfnNtQueryInformationProcess)
    return 0;

  // get process handle
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,dwPID);
  if (!hProcess) // could fail due to invalid PID or insufficiant privileges
    return (0);

  // gather information
  MY_PROCESS_BASIC_INFORMATION    pbi;
  ULONG                           ulRetLen = 0;
  NTSTATUS ntStatus = pfnNtQueryInformationProcess(
      hProcess,
      ProcessBasicInformation,
      (void*)&pbi,
      sizeof(PROCESS_BASIC_INFORMATION),
      &ulRetLen);

  // copy PID on success
  DWORD dwParentPID = 0;
  if (ntStatus == 0)
    dwParentPID = pbi.InheritedFromUniqueProcessId;

  CloseHandle(hProcess);
  return dwParentPID;
}

ni::cString GetProcessExePathFromPid(ni::tIntPtr pid) {
  WCHAR exePath[AMAX_PATH] = {0};
  GetProcessExePath(pid,exePath,AMAX_PATH);
  return exePath;
}

int GetProcessCount(const astl::string& executable_name,
                    const ProcessFilter* filter) {
  int count = 0;

  NamedProcessIterator iter(executable_name, filter);
  while (iter.NextProcessEntry())
    ++count;
  return count;
}

bool KillProcesses(const astl::string& executable_name, int exit_code,
                   const ProcessFilter* filter) {
  bool result = true;
  NamedProcessIterator iter(executable_name, filter);
  for (;;) {
    const ProcessEntry* entry = iter.NextProcessEntry();
    if (!entry)
      break;
    if (!KillProcessById((*entry).th32ProcessID, exit_code, true))
      result = false;
  }

  return result;
}

bool WaitForProcessesToExit(const astl::string& executable_name,
                            int wait_milliseconds,
                            const ProcessFilter* filter) {
  bool result = true;
  DWORD start_time = GetTickCount();
  NamedProcessIterator iter(executable_name, filter);
  for (;;) {
    const ProcessEntry* entry = iter.NextProcessEntry();
    if (!entry)
      break;
    DWORD remaining_wait = eastl::max(0, wait_milliseconds -
                                      static_cast<int>(GetTickCount() - start_time));
    HANDLE process = OpenProcess(SYNCHRONIZE,
                                 FALSE,
                                 entry->th32ProcessID);
    DWORD wait_result = ::WaitForSingleObject(process, remaining_wait);
    CloseHandle(process);
    result = result && (wait_result == WAIT_OBJECT_0);
  }

  return result;
}

bool WaitForSingleProcess(ProcessHandle handle, int wait_milliseconds) {
  bool retval = ::WaitForSingleObject(handle, wait_milliseconds) == WAIT_OBJECT_0;
  return retval;
}

bool WaitForExitCode(ProcessHandle handle, int* exit_code, int wait_milliseconds) {
  if (::WaitForSingleObject(handle, wait_milliseconds) != WAIT_OBJECT_0) {
    return false;
  }
  DWORD temp_code;  // Don't clobber out-parameters in case of failure.
  if (!::GetExitCodeProcess(handle, &temp_code))
    return false;
  if (exit_code) {
    *exit_code = temp_code;
  }
  return true;
}

bool CleanupProcesses(const astl::string& executable_name,
                      int wait_milliseconds,
                      int exit_code,
                      const ProcessFilter* filter) {
  bool exited_cleanly = WaitForProcessesToExit(executable_name,
                                               wait_milliseconds,
                                               filter);
  if (!exited_cleanly)
    KillProcesses(executable_name, exit_code, filter);
  return exited_cleanly;
}

}  // namespace base

#endif
