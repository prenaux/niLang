#include "API/niLang/Utils/ThreadImpl.h"

#ifndef niNoThreads

#ifdef niWindows
#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#include <process.h>

niExportFunc(void*) MyWin32CreateEvent(int bManualReset,int bInitialState) {
  return ::CreateEventW(NULL,bManualReset,bInitialState,NULL);
}
#endif

#include "API/niLang/STL/utils.h"
#include "API/niLang/StringDef.h"

using namespace ni;

struct ThreadImpl;

static ThreadMutex _threadListMutex;
static astl::vector<ThreadImpl*> _threadList;

struct ThreadImpl : public cIUnknownImpl<iThread>
{
  ThreadImpl() {
    mnID = eInvalidHandle;
    mHandle = NULL;
    mpfnBaseThreadProc = NULL;
    mpData = NULL;
    {
      AutoThreadLock lock(_threadListMutex);
      _threadList.push_back(this);
    }
  }
  ~ThreadImpl() {
    Close();
    {
      AutoThreadLock lock(_threadListMutex);
      bool r = astl::find_erase(_threadList,this);
      niAssert(r);
      niUnused(r);
    }
  }

  //! Starts the thread.
  tBool __stdcall Start(tpfnBaseThreadProc apfnBaseThreadProc, void* apData) {
    if (mHandle) {
      niAssertUnreachable("Thread already started.");
      return eFalse;
    }
    niAssert(apfnBaseThreadProc != NULL);
    mnID = eInvalidHandle;
    mpfnBaseThreadProc = apfnBaseThreadProc;
    mpData = apData;
#ifdef niWindows
    mHandle = _beginthreadex(NULL,0,_RunThread,(LPVOID)this,CREATE_SUSPENDED,
                             (unsigned int*)&mnID);
    if (mHandle == 0) {
      niAssertUnreachable("Can't create thread.");
      return eFalse;
    }
    ::ResumeThread((HANDLE)mHandle);
#else
    pthread_attr_t type;
    int r = pthread_attr_init(&type);
    niAssertMsg(r==0,_A("Can't init pthread attributes."));
    niUnused(r);
    pthread_attr_setdetachstate(&type,PTHREAD_CREATE_JOINABLE);
    mThreadProcCompletedEvent.Reset();
    r = pthread_create(&mHandle,&type,_RunThread,(void*)this);
    if (r != 0) {
      niAssertUnreachable("Can't create pthread.");
      return eFalse;
    }
    // wait for the ID to be set
    mThreadProcCompletedEvent.Wait();
    niAssert(mnID != eInvalidHandle);
#endif
    return eTrue;
  }

  //! Close the thread handle and allow it to be restarted. Does not
  //! garantee that the thread actually finished.
  void __stdcall Close() {
    if (mHandle) {
#ifdef niWindows
      CloseHandle((HANDLE)mHandle);
#endif
      mHandle = 0;
      mpData = NULL;
      mpfnBaseThreadProc = NULL;
    }
  }

  //! Check whether the thread has been closed.
  tBool __stdcall IsClosed() const {
    return mHandle == 0;
  }

  //! Wait for the thread to finish and then close it.
  //! \return eFalse if the timeout expired, eTrue if the thread has
  //!         been closed or is already closed.
  tBool __stdcall Join(tU32 anTimeout) {
    if (mHandle) {
      if (anTimeout == eInvalidHandle) {
#ifdef niWindows
        ::WaitForSingleObject((HANDLE)mHandle, INFINITE);
#else
        void* lpv;
        pthread_join(mHandle,&lpv);
#endif
      }
      else {
#ifdef niWindows
        if (::WaitForSingleObject((HANDLE)mHandle,anTimeout) == WAIT_TIMEOUT) {
          return eFalse;
        }
#else
        if (!mThreadProcCompletedEvent.WaitEx(anTimeout)) {
          // thread didn't complete in time...
          return eFalse;
        }
        // thread ended, join it now...
        void* lpv;
        pthread_join(mHandle,&lpv);
#endif
      }
      Close();
    }
    return ni::eTrue;
  }

  //! Kill the thread immediatly.
  void __stdcall Kill() {
    if (!mHandle) return;
#ifdef niWindows
    // warning C6258: Using TerminateThread does not allow proper thread clean up
#if defined _LINT && _MSC_VER >= 1600
#pragma warning ( disable : 6258 )
#endif
    TerminateThread((HANDLE)mHandle,eInvalidHandle);
#if defined _LINT && _MSC_VER >= 1600
#pragma warning ( default : 6258 )
#endif
#else
    pthread_kill(mHandle,SIGKILL);
#endif
  }

  //! Check whether the thread is currently running.
  tBool __stdcall GetIsAlive() const {
    if (!mHandle)
      return ni::eFalse;
#ifdef niWindows
    unsigned long code = eInvalidHandle;
    ::GetExitCodeThread((HANDLE)mHandle, &code);
    return (code == STILL_ACTIVE);
#else
    return ni::eTrue;
#endif
  }

  //! Return the thread's id.
  tU64 __stdcall GetThreadID() const {
    return mnID;
  }

  //! Set the thread's priority.
  void __stdcall SetPriority(tI32 aPriority) {
#ifdef niWindows
    switch (aPriority) {
      case 0: ::SetThreadPriority((HANDLE)mHandle, THREAD_PRIORITY_LOWEST); break;
      case 1: ::SetThreadPriority((HANDLE)mHandle, THREAD_PRIORITY_BELOW_NORMAL); break;
      case 2: ::SetThreadPriority((HANDLE)mHandle, THREAD_PRIORITY_IDLE); break;
      case 3: ::SetThreadPriority((HANDLE)mHandle, THREAD_PRIORITY_NORMAL); break;
      case 4: ::SetThreadPriority((HANDLE)mHandle, THREAD_PRIORITY_ABOVE_NORMAL); break;
      case 5: ::SetThreadPriority((HANDLE)mHandle, THREAD_PRIORITY_HIGHEST);  break;
      case 6: ::SetThreadPriority((HANDLE)mHandle, THREAD_PRIORITY_TIME_CRITICAL);  break;
    }
#endif
  }

  //! Get the thread's priority.
  tI32 __stdcall GetPriority() const {
#ifdef niWindows
    int priority = ::GetThreadPriority((HANDLE)mHandle);
    if (priority <= THREAD_PRIORITY_BELOW_NORMAL) {
      return 0;
    }
    else if(priority <= THREAD_PRIORITY_BELOW_NORMAL) {
      return 1;
    }
    else if(priority <= THREAD_PRIORITY_IDLE) {
      return 2;
    }
    else if(priority <= THREAD_PRIORITY_NORMAL) {
      return 3;
    }
    else if(priority <= THREAD_PRIORITY_ABOVE_NORMAL) {
      return 4;
    }
    else if(priority <= THREAD_PRIORITY_HIGHEST) {
      return 5;
    }
    else if(priority <= THREAD_PRIORITY_TIME_CRITICAL) {
      return 6;
    }
    niAssertUnreachable("Unknown Windows thread priority.");
#endif
    return 3;
  }

 private:
#ifdef niWindows
  tIntPtr mHandle;
#else
  pthread_t mHandle;
  ThreadEvent mThreadProcCompletedEvent;
#endif
  tU64 mnID;
  tpfnBaseThreadProc mpfnBaseThreadProc;
  void* mpData;

#ifdef niWindows
  static UINT WINAPI _RunThread(LPVOID data)
#else
  static void* _RunThread(void* data)
#endif
  {
    tIntPtr r;
#if !defined niWindows
    { // WeakPtr scope BEGIN. The scope is important to not leak the WeakPtr
      // since pthread_exit will exit this function without calling any
      // destructor.
      WeakPtr<ThreadImpl> wImpl = (ThreadImpl*)data;
#endif
      {
        ThreadImpl* pBase = (ThreadImpl*)data;
        niAssert(pBase->IsOK());
#ifdef niWindows
        niAssert(pBase->mnID == ThreadGetCurrentThreadID());
#else
        pBase->mnID = ThreadGetCurrentThreadID();
        pBase->mThreadProcCompletedEvent.Signal(); // signal for Start
#endif
        niAssert(pBase->mpfnBaseThreadProc != NULL);
        r = pBase->mpfnBaseThreadProc(pBase->mpData);
      }
#ifdef niWindows
      return (UINT)r;
#else
      QPtr<ThreadImpl> impl = wImpl;
      if (impl.IsOK()) {
        impl->mThreadProcCompletedEvent.Signal(); // signal for Join
      }
    } // WeakPtr scope end
    pthread_exit((void*)r);
    return (void*)r;
#endif
  }
};

niExportFunc(iThread*) ni_create_thread() {
  return niNew ThreadImpl();
}
niExportFunc(tU32) ni_get_num_threads() {
  AutoThreadLock lock(_threadListMutex);
  return _threadList.size();
}
niExportFunc(iThread*) ni_get_thread(tU32 anIndex) {
  AutoThreadLock lock(_threadListMutex);
  niAssert(anIndex < _threadList.size());
  if (anIndex >= _threadList.size()) return NULL;
  return _threadList[anIndex];
}
niExportFunc(void) ni_join_all_threads() {
  astl::vector<Ptr<ThreadImpl> > allThreads;
  {
    AutoThreadLock lock(_threadListMutex);
    niLoop(i,_threadList.size()) {
      allThreads.push_back(_threadList[i]);
    }
  }
  if (!allThreads.empty()) {
    niWarning(niFmt("Joining '%d' left over thread.",allThreads.size()));
    niLoop(i,allThreads.size()) {
      niWarning(niFmt("Joining thread '%p'",allThreads[i]->GetThreadID()));
      if (!allThreads[i]->Join(3000)) {
        niWarning(niFmt("Joining thread '%p' timed out, killing it.",allThreads[i]->GetThreadID()));
        allThreads[i]->Kill();
      }
    }
    niWarning("Joined all threads.");
  }
}

#endif
