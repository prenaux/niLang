#ifndef __THREADBASEIMPL_7799222_H__
#define __THREADBASEIMPL_7799222_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "UnknownImpl.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#ifdef niNoThreads

namespace ni {

static inline tU64 ThreadGetCurrentThreadID() {
  return 0xAAAADDDD;
}
struct ThreadMutex : public cMemImpl {
  ThreadMutex() {}
  ~ThreadMutex() {}
  void __stdcall ThreadLock() const {}
  void __stdcall ThreadUnlock() const {}
};
struct AutoThreadLock {
  AutoThreadLock() = delete;
  AutoThreadLock(const ThreadMutex&) {}
  ~AutoThreadLock() {}
};

struct ThreadEvent : public cMemImpl {
  ThreadEvent(tBool abManualReset = eFalse) {
    mIsSignaled = abManualReset;
  }
  ~ThreadEvent() {
  }

  // put in non-signaled state
  void __stdcall Reset() {
    mIsSignaled = eFalse;
  }
  // put in signaled state
  void __stdcall Signal() {
    mIsSignaled = eTrue;
  }
  // wait for an event, set it back to non-signaled state when returning
  tBool __stdcall Wait(tU32 anTimeout) {
    niUnused(anTimeout);
    if (!mIsSignaled)
      return eFalse;
    mIsSignaled = eFalse;
    return eTrue;
  }

  //
  // InfiniteWait() cannot be implemented sanely in niNoThreads mode since it
  // should block and guarantee to continue only if the signal has been
  // signaled.  In fact it probably shouldn't exist at all since its a
  // deadlock factory...
  //
  // void __stdcall InfiniteWait()
  //

 private:
  tBool mIsSignaled;
};

}

#else

#ifdef niWin32
// We don't include windows.h because it pollutes the global namespace horribly.
extern "C" {

#define MY_WINBASEAPI __declspec(dllimport)
#define MY_WINAPI __stdcall

typedef struct _MY_RTL_CRITICAL_SECTION {
  void* DebugInfo;
  long LockCount;
  long RecursionCount;
  void* OwningThread;
  void* LockSemaphore;
  void* SpinCount;
} MY_CRITICAL_SECTION;

MY_WINBASEAPI ni::tU32 MY_WINAPI GetCurrentThreadId();

MY_WINBASEAPI void MY_WINAPI InitializeCriticalSection(struct _RTL_CRITICAL_SECTION* lpCriticalSection);
MY_WINBASEAPI int MY_WINAPI InitializeCriticalSectionAndSpinCount(struct _RTL_CRITICAL_SECTION* lpCriticalSection,ni::tU32 dwSpinCount);
MY_WINBASEAPI void MY_WINAPI EnterCriticalSection(struct _RTL_CRITICAL_SECTION* lpCriticalSection);
MY_WINBASEAPI void MY_WINAPI LeaveCriticalSection(struct _RTL_CRITICAL_SECTION* lpCriticalSection);
MY_WINBASEAPI int MY_WINAPI InitializeCriticalSectionAndSpinCount(struct _RTL_CRITICAL_SECTION* lpCriticalSection,ni::tU32 dwSpinCount);
MY_WINBASEAPI void MY_WINAPI DeleteCriticalSection(struct _RTL_CRITICAL_SECTION* lpCriticalSection);

// There's a wrapper here because CreateEvent's definition has been made by nut jobs
niExportFunc(void*) MyWin32CreateEvent(int bManualReset,int bInitialState);
MY_WINBASEAPI int MY_WINAPI CloseHandle(void* hObject);
MY_WINBASEAPI int MY_WINAPI SetEvent(void* hEvent);
MY_WINBASEAPI int MY_WINAPI ResetEvent(void* hEvent);

MY_WINBASEAPI void* MY_WINAPI CreateSemaphoreW(struct _SECURITY_ATTRIBUTES* lpSecurityAttr, long lInitialCount, long lMaximumCount, const wchar_t* lpName);
MY_WINBASEAPI int MY_WINAPI ReleaseSemaphore(void* hObject, long lReleaseCount, long* lpPreviousCount);

MY_WINBASEAPI ni::tU32 MY_WINAPI WaitForSingleObject(void* hHandle,ni::tU32 dwMilliseconds);

#define MY_STATUS_WAIT_0 ((ni::tU32)0x00000000L)
#define MY_WAIT_OBJECT_0 ((MY_STATUS_WAIT_0) + 0)

#define MY_INFINITE 0xffffffff
}
#elif defined niPosix
#  include "../Platforms/Unix/UnixThread.h"
#  include <pthread.h>
#  include <signal.h>
#else
#  error "No threading support on this platform."
#endif

namespace ni {

#ifdef niWin32
static inline tU64 ThreadGetCurrentThreadID() {
  return ::GetCurrentThreadId();
}
#elif defined niOSX || defined niIOS
static inline tU64 ThreadGetCurrentThreadID() {
  uint64_t tid;
  pthread_threadid_np(NULL, &tid);
  return tid;
}
#else
static inline tU64 ThreadGetCurrentThreadID() {
  // Maybe should return a U32 (some implementation might return a 64bit pointer...)
  return (tU64)pthread_self();
}
#endif

}

//==========================================================================
//
//  Mutex
//
//==========================================================================
namespace ni {

struct ThreadMutex : public cMemImpl {
  ThreadMutex() {
#ifdef niWin32
    InitializeCriticalSectionAndSpinCount((struct _RTL_CRITICAL_SECTION*)&mCS,4000);
    //    InitializeCriticalSection(&mCS);
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
    int r = pthread_mutex_init(&mID,&attr);
    niAssertMsg(r==0,_A("Can't create pthread mutex."));
    niUnused(r);
#endif
  }
  ~ThreadMutex() {
#ifdef niWin32
    DeleteCriticalSection((struct _RTL_CRITICAL_SECTION*)&mCS);
#else
    pthread_mutex_destroy(&mID);
#endif
  }
  void __stdcall ThreadLock() const {
#ifdef niWin32
    EnterCriticalSection((struct _RTL_CRITICAL_SECTION*)&mCS);
#else
    int r = pthread_mutex_lock(&mID);
    niAssertMsg(r>=0,_A("Can't lock pthread mutex."));
    niUnused(r);
#endif
  }
  void __stdcall ThreadUnlock() const {
#ifdef niWin32
    LeaveCriticalSection((struct _RTL_CRITICAL_SECTION*)&mCS);
#else
    int r = pthread_mutex_unlock(&mID);
    niAssertMsg(r>=0,_A("Can't unlock pthread mutex."));
    niUnused(r);
#endif
  }

 private:
#ifdef niWin32
  mutable MY_CRITICAL_SECTION mCS;
#else
  mutable pthread_mutex_t mID;
#endif
};

struct AutoThreadLock {
  AutoThreadLock() = delete;
  AutoThreadLock(const ThreadMutex& v) : mValue(v) {
    mValue.ThreadLock();
  }
  ~AutoThreadLock() {
    mValue.ThreadUnlock();
  }
 private:
  const ThreadMutex& mValue;
  niClassStrictLocal(AutoThreadLock);
};

}

//==========================================================================
//
//  Event
//
//==========================================================================
namespace ni {

struct ThreadEvent : public cMemImpl {
  // create the event in non-signaled state
  ThreadEvent(tBool abManualReset = eFalse) {
#ifdef niWin32
    mHandle = MyWin32CreateEvent(abManualReset,0);
#else
    mHandle = ni::Unix::CreateEvent(NULL,abManualReset,0);
#endif
  }
  ~ThreadEvent() {
#ifdef niWin32
    ::CloseHandle(mHandle);
#else
    ni::Unix::DeleteEvent(mHandle);
#endif
  }

  // put in non-signaled state
  void __stdcall Reset() {
#ifdef niWin32
    ::ResetEvent(mHandle);
#else
    ni::Unix::ResetEvent(mHandle);
#endif
  }
  // put in signaled state
  void __stdcall Signal() {
#ifdef niWin32
    ::SetEvent(mHandle);
#else
    ni::Unix::SetEvent(mHandle);
#endif
  }
  // wait for an event, set it back to non-signaled state when returning
  tBool __stdcall Wait(tU32 anTimeout) {
#ifdef niWin32
    return WaitForSingleObject(mHandle,anTimeout) == MY_WAIT_OBJECT_0;
#else
    return ni::Unix::WaitForSingleObject(mHandle,anTimeout) == ni::Unix::WAIT_OBJECT_0;
#endif
  }
  void __stdcall InfiniteWait() {
#ifdef niWin32
    WaitForSingleObject(mHandle,MY_INFINITE);
#else
    ni::Unix::WaitForSingleObject(mHandle,ni::Unix::INFINITE);
#endif
  }

 private:
#ifdef niWin32
  void* mHandle;
#else
  ni::Unix::HANDLE mHandle;
#endif
};

}

//==========================================================================
//
//  Semaphore
//
//==========================================================================
#ifdef niWin32
// #include <windows.h>
#elif defined niOSX || defined niIOS
#include <mach/mach.h>
#elif defined niLinux || defined niQNX
#include <semaphore.h>
#else
#error "ThreadSem: Platform not supported"
#endif

namespace ni {

struct ThreadSem : public cMemImpl {
#ifdef niWin32
  void* _winSem;
#elif defined niOSX || defined niIOS
  semaphore_t _osxSem;
#elif defined niLinux
  sem_t _linSem;
#endif

  ThreadSem(tI32 initialCount = 0) {
    niAssert(initialCount >= 0);
#ifdef niWin32
    _winSem = CreateSemaphoreW(NULL, initialCount, ni::TypeMax<long>(), NULL);
#elif defined niOSX || defined niIOS
    semaphore_create(mach_task_self(), &_osxSem, SYNC_POLICY_FIFO, initialCount);
#elif defined niLinux
    sem_init(&_linSem, 0, initialCount);
#endif
  }

  ~ThreadSem() {
#ifdef niWin32
    CloseHandle(_winSem);
#elif defined niOSX || defined niIOS
    semaphore_destroy(mach_task_self(), _osxSem);
#elif defined niLinux
    sem_destroy(&_linSem);
#endif
  }

  // Decrements the semaphore. If the resulting value is less than zero, it
  // waits for a signal from a thread that increments the semaphore by calling
  // Signal() before returning.
  void InfiniteWait() {
#ifdef niWin32
    WaitForSingleObject(_winSem, MY_INFINITE);
#elif defined niOSX || defined niIOS
    semaphore_wait(_osxSem);
#elif defined niLinux
    // http://stackoverflow.com/questions/2013181/gdb-causes-sem-wait-to-fail-with-eintr-error
    int rc;
    do {
      rc = sem_wait(&_linSem);
    } while (rc == -1 && errno == EINTR);
#endif
  }

  // Increments the counting semaphore. If the previous value was less than
  // zero, it wakes one of the threads that are waiting in Wait() before
  // returning.
  void Signal(tU32 count = 1) {
#ifdef niWin32
    ReleaseSemaphore(_winSem, count, NULL);
#elif defined niOSX || defined niIOS
    while (count-- > 0) {
      semaphore_signal(_osxSem);
    }
#elif defined niLinux
    while (count-- > 0) {
      sem_post(&_linSem);
    }
#endif
  }
};

}

//==========================================================================
//
//  Thread
//
//==========================================================================
namespace ni {

//! Thread procedure type.
//! {NoAutomation}
typedef tIntPtr (__ni_export_call_decl *tpfnBaseThreadProc)(void* apData);

struct iThread : public iUnknown {
  niDeclareInterfaceUUID(iThread,0x9cac2fc1,0xfe06,0x4ade,0x86,0x6e,0x5c,0xfc,0x3d,0xb7,0xca,0xff);
  //! Starts the thread.
  virtual tBool __stdcall Start(tpfnBaseThreadProc apfnBaseThreadProc, void* apData) = 0;
  //! Close the thread handle and allow it to be restarted. Does not
  //! garantee that the thread actually finished.
  virtual void __stdcall Close() = 0;
  //! Check whether the thread has been closed.
  virtual tBool __stdcall IsClosed() const = 0;
  //! Wait for the thread to finish and then close it.
  //! \return eFalse if the timeout expired, eTrue if the thread has
  //!         been closed or is already closed.
  virtual tBool __stdcall Join(tU32 anTimeout) = 0;
  //! Kill the thread immediatly.
  virtual void __stdcall Kill() = 0;
  //! Check whether the thread is currently running.
  virtual tBool __stdcall GetIsAlive() const = 0;
  //! Return the thread's id.
  virtual tU64 __stdcall GetThreadID() const = 0;
  //! Set the thread's priority.
  virtual void __stdcall SetPriority(tI32 aPriority) = 0;
  //! Get the thread's priority.
  virtual tI32 __stdcall GetPriority() const = 0;
};

}

niExportFunc(ni::iThread*) ni_create_thread();
niExportFunc(ni::tU32) ni_get_num_threads();
niExportFunc(ni::iThread*) ni_get_thread(ni::tU32 anIndex);
niExportFunc(void) ni_join_all_threads();

#ifdef niWin32
#undef MY_WINBASEAPI
#undef MY_WINAPI
#undef MY_STATUS_WAIT_0
#undef MY_WAIT_OBJECT_0
#undef MY_INFINITE
#endif

#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
#endif // __THREADBASEIMPL_7799222_H__
