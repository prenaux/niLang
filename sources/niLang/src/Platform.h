// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROCESS_H_
#define BASE_PROCESS_H_

#include "API/niLang/Types.h"

#ifndef niNoProcess

#include "API/niLang/IOSProcess.h"

#if defined(niWindows)
#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#include <tlhelp32.h>
#elif defined(niPosix)
#include <dirent.h>
#include <limits.h>
#include <sys/types.h>
#endif

#include "API/niLang/STL/string.h"
#include "API/niLang/STL/vector.h"

#if defined(niWindows)
typedef PROCESSENTRY32W ProcessEntry;
#elif defined(niPosix)
// TODO(port): we should not rely on a Win32 structure.
struct ProcessEntry {
  int pid;
  int ppid;
  char szExeFile[NAME_MAX + 1];
};
#endif

#if defined(niOSX) || defined niIOS
struct kinfo_proc;
#endif

namespace base {
//--------------------------------------------------------------------------------------------
//
//  Process
//
//--------------------------------------------------------------------------------------------

// ProcessHandle is a platform specific type which represents the underlying OS
// handle to a process.
#if defined(niWindows)
typedef HANDLE ProcessHandle;
#elif defined(niPosix)
// On POSIX, our ProcessHandle will just be the PID.
typedef int ProcessHandle;
#endif

class Process {
 public:
  Process() : process_(0), last_working_set_size_(0), pid_() {}
  explicit Process(ProcessHandle handle, int pid) :
      process_(handle), pid_(pid), last_working_set_size_(0) {}

  // A handle to the current process.
  static Process Current();

  // Get/Set the handle for this process. The handle will be 0 if the process
  // is no longer running.
  ProcessHandle handle() const { return process_; }
  int pid() const { return pid_; }
  void set_handle(ProcessHandle handle, int pid) { process_ = handle; pid_ = pid; }

  // Is the this process the current process.
  bool is_current() const;

  // Close the process handle. This will not terminate the process.
  void Close();

  // Terminates the process with extreme prejudice. The given result code will
  // be the exit code of the process. If the process has already exited, this
  // will do nothing.
  void Terminate(int result_code);

 private:
  ProcessHandle process_;
  int pid_;
  size_t last_working_set_size_;
};

//--------------------------------------------------------------------------------------------
//
//  Process Utils
//
//--------------------------------------------------------------------------------------------
// A minimalistic but hopefully cross-platform set of exit codes.
// Do not change the enumeration values or you will break third-party
// installers.
enum {
  PROCESS_END_NORMAL_TERMINATON = 0,
  PROCESS_END_KILLED_BY_USER    = 1,
  PROCESS_END_PROCESS_WAS_HUNG  = 2
};

// Returns the id of the current process.
int GetCurrentProcId();

// Get the exe path of the specified process.
ni::cString GetProcessExePathFromPid(ni::tIntPtr pid);

// Get the parent process of the specified process.
ni::tIntPtr GetParentProcessFromPid(ni::tIntPtr pid);

#if defined(niPosix)
// Sets all file descriptors to close on exec except for stdin, stdout
// and stderr.
void SetAllFDsToCloseOnExec();
#endif

// For platforms where ProcessHandle != PID
#ifdef niWindows
#define niProcessProcessHandleNotPID
ProcessHandle GetCurrentProcessHandle();
// Converts a PID to a process handle. This handle must be closed by
// CloseProcessHandle when you are done with it.
ProcessHandle OpenProcessHandle(int pid);
// Closes the process handle opened by OpenProcessHandle.
void CloseProcessHandle(ProcessHandle process);
// on other OS's the process handle is the same as the PID
int GetProcIdFromHandle(ProcessHandle process);
#endif

#if defined(niWindows)
// Runs the given application name with the given command line. Normally, the
// first command line argument should be the path to the process, and don't
// forget to quote it.
//
// If wait is true, it will block and wait for the other process to finish,
// otherwise, it will just continue asynchronously.
//
// Example (including literal quotes)
//  cmdline = "c:\windows\explorer.exe" -foo "c:\bar\"
//
// If process_handle is non-NULL, the process handle of the launched app will be
// stored there on a successful launch.
// NOTE: In this case, the caller is responsible for closing the handle so
//       that it doesn't leak!
//
// hStdin, hStdout, hStderr specify the standard pipes to be used by the process,
// they must all be specified if at all.
//
// aaszWorkDir is the starting working directory.
//
// apEnv defines the environment variables that the process will inherit in the form
// ENV0=VAL0\0ENVN=VALN\0\0.
//
// An environment block can contain either Unicode or ANSI
// characters. If the environment block pointed to by lpEnvironment
// contains Unicode characters, be sure that dwCreationFlags includes
// CREATE_UNICODE_ENVIRONMENT. If this parameter is NULL and the
// environment block of the parent process contains Unicode
// characters, you must also ensure that dwCreationFlags includes
// CREATE_UNICODE_ENVIRONMENT.
//

bool LaunchApp(const ni::achar* aaszCmdLine,
               HANDLE hStdin, HANDLE hStdout, HANDLE hStderr,
               const ni::achar* aaszWorkDir,
               const WCHAR* apEnv,
               ProcessHandle* process_handle,
               ni::tOSProcessSpawnFlags aSpawnFlags);
#elif defined(niPosix)
// Runs the application specified in argv[0] with the command line argv.
// Before launching all FDs open in the parent process will be marked as
// close-on-exec.  |fds_to_remap| defines a mapping of src fd->dest fd to
// propagate FDs into the child process.
//
// As above, if wait is true, execute synchronously. The pid will be stored
// in process_handle if that pointer is non-null.
//
// Note that the first argument in argv must point to the filename,
// and must be fully specified.
typedef astl::vector<astl::pair<int, int> > file_handle_mapping_vector;
bool LaunchApp(const astl::vector<astl::string>& argv,
               const file_handle_mapping_vector& fds_to_remap,
               ProcessHandle* process_handle,
               const astl::vector<ni::cString>& aEnvs);
#endif

// Used to filter processes by process ID.
class ProcessFilter {
 public:
  // Returns true to indicate set-inclusion and false otherwise.  This method
  // should not have side-effects and should be idempotent.
  virtual bool Includes(ni::tI32 pid, ni::tI32 parent_pid) const = 0;
  virtual ~ProcessFilter() { }
};

// Returns the number of processes on the machine that are running from the
// given executable name.  If filter is non-null, then only processes selected
// by the filter will be counted.
int GetProcessCount(const astl::string& executable_name,
                    const ProcessFilter* filter);

// Attempts to kill all the processes on the current machine that were launched
// from the given executable name, ending them with the given exit code.  If
// filter is non-null, then only processes selected by the filter are killed.
// Returns false if all processes were able to be killed off, false if at least
// one couldn't be killed.
bool KillProcesses(const astl::string& executable_name, int exit_code,
                   const ProcessFilter* filter);

// Attempts to kill the process identified by the given process
// entry structure, giving it the specified exit code. If |wait| is true, wait
// for the process to be actually terminated before returning.
// Returns true if this is successful, false otherwise.
bool KillProcess(ProcessHandle process, int exit_code, bool wait);
#if defined(niWindows)
bool KillProcessById(DWORD process_id, int exit_code, bool wait);
#endif

// Get the termination status (exit code) of the process and return true if the
// status indicates the process crashed.  It is an error to call this if the
// process hasn't terminated yet.
bool DidProcessCrash(ProcessHandle handle);

// Waits for process to exit. In POSIX systems, if the process hasn't been
// signaled then puts the exit code in |exit_code|; otherwise it's considered
// a failure. On Windows |exit_code| is always filled. Returns true on success,
// and closes |handle| in any case.
bool WaitForExitCode(ProcessHandle handle, int* exit_code, int wait_milliseconds);

// Wait for all the processes based on the named executable to exit.  If filter
// is non-null, then only processes selected by the filter are waited on.
// Returns after all processes have exited or wait_milliseconds have expired.
// Returns true if all the processes exited, false otherwise.
bool WaitForProcessesToExit(const astl::string& executable_name,
                            int wait_milliseconds,
                            const ProcessFilter* filter);

// Wait for a single process to exit. Return true if it exited cleanly within
// the given time limit.
bool WaitForSingleProcess(ProcessHandle handle,
                          int wait_milliseconds);

// Waits a certain amount of time (can be 0) for all the processes with a given
// executable name to exit, then kills off any of them that are still around.
// If filter is non-null, then only processes selected by the filter are waited
// on.  Killed processes are ended with the given exit code.  Returns false if
// any processes needed to be killed, true if they all exited cleanly within
// the wait_milliseconds delay.
bool CleanupProcesses(const astl::string& executable_name,
                      int wait_milliseconds,
                      int exit_code,
                      const ProcessFilter* filter);

// This class provides a way to iterate through the list of processes
// on the current machine that were started from the given executable
// name.  To use, create an instance and then call NextProcessEntry()
// until it returns false.
class NamedProcessIterator {
 public:
  NamedProcessIterator(const astl::string& executable_name,
                       const ProcessFilter* filter);
  ~NamedProcessIterator();

  // If there's another process that matches the given executable name,
  // returns a const pointer to the corresponding PROCESSENTRY32.
  // If there are no more matching processes, returns NULL.
  // The returned pointer will remain valid until NextProcessEntry()
  // is called again or this NamedProcessIterator goes out of scope.
  const ProcessEntry* NextProcessEntry();

 private:
  // Determines whether there's another process (regardless of executable)
  // left in the list of all processes.  Returns true and sets entry_ to
  // that process's info if there is one, false otherwise.
  bool CheckForNextProcess();

  bool IncludeEntry();

  // Initializes a PROCESSENTRY32 data structure so that it's ready for
  // use with Process32First/Process32Next.
  void InitProcessEntry(ProcessEntry* entry);

  astl::string executable_name_;

#if defined(niWindows)
  HANDLE snapshot_;
  bool started_iteration_;
#elif defined(niOSX) || defined niIOS
  astl::vector<kinfo_proc> kinfo_procs_;
  size_t index_of_kinfo_proc_;
#elif defined(niPosix)
  DIR *procfs_dir_;
#endif
  ProcessEntry entry_;
  const ProcessFilter* filter_;

  niClassNoCopyAssign(NamedProcessIterator);
};

}  // namespace base

#endif // niNoProcess

#endif  // BASE_PROCESS_H_
