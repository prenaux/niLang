#include "Platform.h"

#if defined niPosix && !defined niNoProcess

#include "API/niLang/Utils/TimerSleep.h"
#include "API/niLang/StringDef.h"

#ifdef niOSX
#define HAS_LIBPROC
#include <libproc.h>
#endif

//--------------------------------------------------------------------------------------------
//
//  Process
//
//--------------------------------------------------------------------------------------------
namespace base {

void Process::Close() {
  pid_ = process_ = 0;
  // if the process wasn't termiated (so we waited) or the state
  // wasn't already collected w/ a wait from process_utils, we're gonna
  // end up w/ a zombie when it does finally exit.
}

void Process::Terminate(int result_code) {
  // result_code isn't supportable.
  if (!process_)
    return;
  // Wait so we clean up the zombie
  KillProcess(process_, result_code, true);
}

bool Process::is_current() const {
  return process_ == GetCurrentProcId();
}

// static
Process Process::Current() {
  return Process(GetCurrentProcId(),GetCurrentProcId());
}

}  // namspace base

//--------------------------------------------------------------------------------------------
//
//  Process Utils
//
//--------------------------------------------------------------------------------------------
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// #include "base/basictypes.h"
// #include "base/logging.h"
// #include "base/platform_thread.h"
// #include "base/process_util.h"
// #include "base/sys_info.h"
// #include "base/time.h"

const int kMicrosecondsPerSecond = 1000000;

namespace base {

int GetCurrentProcId() {
  return getpid();
}

ni::tIntPtr GetParentProcessFromPid(ni::tIntPtr dwPID) {
  // Not implemented
  return 0;
}

ni::cString GetProcessExePathFromPid(ni::tIntPtr pid) {
  int ret;
  char pathbuf[PATH_MAX] = {0};
#ifdef HAS_LIBPROC
  ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));
#else
  ret = ::readlink(niFmt("/proc/%d/exe", pid), pathbuf, sizeof(pathbuf));
#endif
  if (ret <= 0) {
    niError(niFmt("PID %d: proc_pidpath(): %s\n",
                  pid, strerror(errno)));
    return AZEROSTR;
  }
  else {
    return pathbuf;
  }
}

// Attempts to kill the process identified by the given process
// entry structure.  Ignores specified exit_code; posix can't force that.
// Returns true if this is successful, false otherwise.
bool KillProcess(ProcessHandle process_id, int exit_code, bool wait) {
  bool result = false;

  int status = kill(process_id, SIGTERM);
  if (!status && wait) {
    int tries = 60;
    // The process may not end immediately due to pending I/O
    while (tries-- > 0) {
      int pid = waitpid(process_id, &status, WNOHANG);
      if (pid == process_id) {
        result = true;
        break;
      }
      sleep(1);
    }
  }
  if (!result) {
    niAssert(0 && "Unable to terminate process.");
  }
  return result;
}

// This class wraps the c library function free() in a class that can be
// passed as a template argument to scoped_ptr_malloc below.
class ScopedPtrMallocFree {
 public:
  inline void operator()(void* x) const {
    free(x);
  }
};

// scoped_ptr_malloc<> is similar to scoped_ptr<>, but it accepts a
// second template argument, the functor used to free the object.

template<class C, class FreeProc = ScopedPtrMallocFree>
class scoped_ptr_malloc {
 public:

  // The element type
  typedef C element_type;

  // Constructor.  Defaults to intializing with NULL.
  // There is no way to create an uninitialized scoped_ptr.
  // The input parameter must be allocated with an allocator that matches the
  // Free functor.  For the default Free functor, this is malloc, calloc, or
  // realloc.
  explicit scoped_ptr_malloc(C* p = NULL): ptr_(p) {}

  // Destructor.  If there is a C object, call the Free functor.
  ~scoped_ptr_malloc() {
    free_(ptr_);
  }

  // Reset.  Calls the Free functor on the current owned object, if any.
  // Then takes ownership of a new object, if given.
  // this->reset(this->get()) works.
  void reset(C* p = NULL) {
    if (ptr_ != p) {
      free_(ptr_);
      ptr_ = p;
    }
  }

  // Get the current object.
  // operator* and operator-> will cause an assert() failure if there is
  // no current object.
  C& operator*() const {
    assert(ptr_ != NULL);
    return *ptr_;
  }

  C* operator->() const {
    assert(ptr_ != NULL);
    return ptr_;
  }

  C* get() const {
    return ptr_;
  }

  // Comparison operators.
  // These return whether a scoped_ptr_malloc and a plain pointer refer
  // to the same object, not just to two different but equal objects.
  // For compatibility wwith the boost-derived implementation, these
  // take non-const arguments.
  bool operator==(C* p) const {
    return ptr_ == p;
  }

  bool operator!=(C* p) const {
    return ptr_ != p;
  }

  // Swap two scoped pointers.
  void swap(scoped_ptr_malloc & b) {
    C* tmp = b.ptr_;
    b.ptr_ = ptr_;
    ptr_ = tmp;
  }

  // Release a pointer.
  // The return value is the current pointer held by this object.
  // If this object holds a NULL pointer, the return value is NULL.
  // After this operation, this object will hold a NULL pointer,
  // and will not own the object any more.
  C* release() {
    C* tmp = ptr_;
    ptr_ = NULL;
    return tmp;
  }

 private:
  C* ptr_;

  // no reason to use these: each scoped_ptr_malloc should have its own object
  template <class C2, class GP>
  bool operator==(scoped_ptr_malloc<C2, GP> const& p) const;
  template <class C2, class GP>
  bool operator!=(scoped_ptr_malloc<C2, GP> const& p) const;

  static FreeProc const free_;

  // Disallow evil constructors
  scoped_ptr_malloc(const scoped_ptr_malloc&);
  void operator=(const scoped_ptr_malloc&);
};

template<class C, class FP>
FP const scoped_ptr_malloc<C, FP>::free_ = FP();

template<class C, class FP> inline
void swap(scoped_ptr_malloc<C, FP>& a, scoped_ptr_malloc<C, FP>& b) {
  a.swap(b);
}

template<class C, class FP> inline
bool operator==(C* p, const scoped_ptr_malloc<C, FP>& b) {
  return p == b.get();
}

template<class C, class FP> inline
bool operator!=(C* p, const scoped_ptr_malloc<C, FP>& b) {
  return p != b.get();
}

// A class to handle auto-closing of DIR*'s.
class ScopedDIRClose {
 public:
  inline void operator()(DIR* x) const {
    if (x) {
      closedir(x);
    }
  }
};
typedef scoped_ptr_malloc<DIR, ScopedDIRClose> ScopedDIR;

// Sets all file descriptors to close on exec except for stdin, stdout
// and stderr.
void SetAllFDsToCloseOnExec() {
#if defined(niLinux) || defined (niQNX)
  const char fd_dir[] = "/proc/self/fd";
#elif defined(niOSX) || defined(niIOS)
  const char fd_dir[] = "/dev/fd";
#else
#error "Unknown POSIX platform."
#endif
  ScopedDIR dir_closer(opendir(fd_dir));
  DIR *dir = dir_closer.get();
  if (NULL == dir) {
    //     DLOG(ERROR) << "Unable to open " << fd_dir;
    return;
  }

  struct dirent *ent;
  while ((ent = readdir(dir))) {
    // Skip . and .. entries.
    if (ent->d_name[0] == '.')
      continue;
    int i = atoi(ent->d_name);
    // We don't close stdin, stdout or stderr.
    if (i <= STDERR_FILENO)
      continue;

    int flags = fcntl(i, F_GETFD);
    if ((flags == -1) || (fcntl(i, F_SETFD, flags | FD_CLOEXEC) == -1)) {
      //       DLOG(ERROR) << "fcntl failure.";
    }
  }
}

bool DidProcessCrash(ProcessHandle handle) {
  int status;
  if (waitpid(handle, &status, WNOHANG)) {
    // I feel like dancing!
    return false;
  }

  if (WIFSIGNALED(status)) {
    switch(WTERMSIG(status)) {
      case SIGSEGV:
      case SIGILL:
      case SIGABRT:
      case SIGFPE:
        return true;
      default:
        return false;
    }
  }

  if (WIFEXITED(status))
    return WEXITSTATUS(status) != 0;

  return false;
}

namespace {

int WaitpidWithTimeout(ProcessHandle handle, int wait_milliseconds,
                       bool* success, int* exit_code)
{
  // This POSIX version of this function only guarantees that we wait no less
  // than |wait_milliseconds| for the proces to exit.  The child process may
  // exit sometime before the timeout has ended but we may still block for
  // up to 0.25 seconds after the fact.
  //
  // waitpid() has no direct support on POSIX for specifying a timeout, you can
  // either ask it to block indefinitely or return immediately (WNOHANG).
  // When a child process terminates a SIGCHLD signal is sent to the parent.
  // Catching this signal would involve installing a signal handler which may
  // affect other parts of the application and would be difficult to debug.
  //
  // Our strategy is to call waitpid() once up front to check if the process
  // has already exited, otherwise to loop for wait_milliseconds, sleeping for
  // at most 0.25 secs each time using usleep() and then calling waitpid().
  //
  // usleep() is speced to exit if a signal is received for which a handler
  // has been installed.  This means that when a SIGCHLD is sent, it will exit
  // depending on behavior external to this function.
  //
  // This function is used primarilly for unit tests, if we want to use it in
  // the application itself it would probably be best to examine other routes.
  int status = -1;
  pid_t ret_pid = waitpid(handle, &status, (wait_milliseconds == (int)ni::eInvalidHandle) ? 0 : WNOHANG);
  const ni::tF64 waitSeconds = (ni::tF64)wait_milliseconds / 1000.0;

  // If the process hasn't exited yet, then sleep and try again.
  //   Time wakeup_time = Time::Now() + TimeDelta::FromMilliseconds(wait_milliseconds);
  ni::tF64 wakeupTime = ni::TimerInSeconds() + waitSeconds;
  while (ret_pid == 0) {
    ni::tF64 now = ni::TimerInSeconds();
    if (now > wakeupTime)
      break;

    // Guaranteed to be non-negative!
    ni::tF64 sleep_time = (wakeupTime - now);
    // Don't sleep for more than 0.25 secs at a time.
    if (sleep_time > 0.25) {
      sleep_time = 0.25;
    }

    // usleep() will return 0 and set errno to EINTR on receipt of a signal
    // such as SIGCHLD.
    ni::SleepMs(sleep_time*1000.0);
    ret_pid = waitpid(handle, &status, WNOHANG);
  }

  if (success) {
    *success = (ret_pid != -1);
  }

  if (exit_code && WIFEXITED(status)) {
    *exit_code = WEXITSTATUS(status);
  }

  return status;
}

}  // namespace

bool WaitForSingleProcess(ProcessHandle handle, int wait_milliseconds) {
  bool waitpid_success;
  int status = WaitpidWithTimeout(handle, wait_milliseconds, &waitpid_success, NULL);
  if (status != -1) {
    niAssert(waitpid_success);
    return WIFEXITED(status);
  } else {
    return false;
  }
}

bool WaitForExitCode(ProcessHandle handle, int* exit_code, int wait_milliseconds) {
  bool waitpid_success;
  int status = WaitpidWithTimeout(handle, wait_milliseconds, &waitpid_success, exit_code);
  if (status != -1) {
    niAssert(waitpid_success);
    return WIFEXITED(status);
  } else {
    return false;
  }
}

bool CrashAwareSleep(ProcessHandle handle, int wait_milliseconds) {
  bool waitpid_success;
  int status = WaitpidWithTimeout(handle, wait_milliseconds, &waitpid_success, NULL);
  if (status != -1) {
    niAssert(waitpid_success);
    return !(WIFEXITED(status) || WIFSIGNALED(status));
  } else {
    // If waitpid returned with an error, then the process doesn't exist
    // (which most probably means it didn't exist before our call).
    return waitpid_success;
  }
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
  const ProcessEntry* entry;

  NamedProcessIterator iter(executable_name, filter);
  while ((entry = iter.NextProcessEntry()) != NULL)
    result = KillProcess((*entry).pid, exit_code, true) && result;

  return result;
}

bool WaitForProcessesToExit(const astl::string& executable_name,
                            int wait_milliseconds,
                            const ProcessFilter* filter) {
  bool result = false;

  // TODO(port): This is inefficient, but works if there are multiple procs.
  // TODO(port): use waitpid to avoid leaving zombies around

  const ni::tF64 waitSeconds = (ni::tF64)wait_milliseconds / 1000.0;
  ni::tF64 endTime = ni::TimerInSeconds() + waitSeconds;
  do {
    NamedProcessIterator iter(executable_name, filter);
    if (!iter.NextProcessEntry()) {
      result = true;
      break;
    }
    ni::SleepMs(100);
  } while ((ni::TimerInSeconds() - endTime) > 0.0);

  return result;
}

bool CleanupProcesses(const astl::string& executable_name,
                      int wait_milliseconds,
                      int exit_code,
                      const ProcessFilter* filter) {
  bool exited_cleanly =
      WaitForProcessesToExit(executable_name, wait_milliseconds,
                             filter);
  if (!exited_cleanly)
    KillProcesses(executable_name, exit_code, filter);
  return exited_cleanly;
}

}  // namespace base

#endif
