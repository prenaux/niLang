#include "API/niLang/Types.h"

#include "API/niLang/IConcurrent.h"
#include "API/niLang/Utils/ConcurrentImpl.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/Utils/MessageID.h"
#include "API/niLang/Utils/TimerSleep.h"
#include "API/niLang/STL/deque.h"

#define RUNNABLE_QUEUE_TRACE(X) //niDebugFmt(X)
#define EXECUTOR_STEAL_TRACE(X) //niDebugFmt(X)

using namespace ni;

//===========================================================================
//
//  sFutureImmediate
//
//===========================================================================
struct sFutureImmediate : public cIUnknownImpl<iFuture> {
  Var mValue;
  sFutureImmediate(const Var& aValue) : mValue(aValue) {}
  void __stdcall Cancel() {}
  tBool __stdcall GetIsDone() const { return eTrue; }
  tBool __stdcall GetIsCancelled() const { return eFalse; }
  tBool __stdcall Wait(tU32 anMs) { return eTrue; }
  Var __stdcall GetValue() const { return mValue; }
};

//===========================================================================
//
//  sFutureValue
//
//===========================================================================
struct sFutureValue : public cIUnknownImpl<iFutureValue,eIUnknownImplFlags_DontInherit1,iFuture> {
  Var mValue;
  SyncCounter mCanceled;
#if !defined niNoThreads
  mutable ThreadEvent meventSet;
  __sync_mutex();
#else
  tBool mbIsSet;
#endif

  sFutureValue()
#if !defined niNoThreads
      : meventSet(eTrue)
#endif
  {
#if !defined niNoThreads
#else
    mbIsSet = eFalse;
#endif
  }

  void __stdcall Cancel() {
    mCanceled.Set(1);
  }
  tBool __stdcall GetIsCancelled() const {
    return mCanceled.Get() != 0;
  }

  tBool __stdcall GetIsDone() const {
#if !defined niNoThreads
    return meventSet.Wait(0);
#else
    return mbIsSet;
#endif
  }
  tBool __stdcall Wait(tU32 anMs) {
#if !defined niNoThreads
    return meventSet.Wait(anMs);
#else
    return mbIsSet;
#endif
  }
  Var __stdcall GetValue() const {
    if (!GetIsDone())
      return niVarNull;
#if !defined niNoThreads
    __sync_lock();
#endif
    return mValue;
  }
  void __stdcall SetValue(const Var& aValue) {
#if !defined niNoThreads
    __sync_lock();
#endif
    mValue = aValue;
#if !defined niNoThreads
    meventSet.Signal();
#else
    mbIsSet = eTrue;
#endif
  }
  void __stdcall Reset() {
#if !defined niNoThreads
    __sync_lock();
#endif
    mCanceled.Set(0);
    mValue = niVarNull;
#if !defined niNoThreads
    meventSet.Reset();
#else
    mbIsSet = eFalse;
#endif
  }
};

//===========================================================================
//
//  sExecutorImmediate
//
//===========================================================================
struct sExecutorImmediate : public cIUnknownImpl<iExecutor> {
  tBool __stdcall Execute(iRunnable* aRunnable) {
    niAssert(niIsOK(aRunnable));
    aRunnable->Run();
    return eTrue;
  }
  Ptr<iFuture> __stdcall Submit(iRunnable* aRunnable) {
    niAssert(niIsOK(aRunnable));
    return niNew sFutureImmediate(aRunnable->Run());
  }
  tBool __stdcall Shutdown(tU32) { return eTrue; }
  tBool __stdcall ShutdownNow(tU32) { return eTrue; }
  tBool __stdcall GetIsShutdown() const { return eTrue; }
  tBool __stdcall GetIsTerminated() const { return eTrue; }
  tU32 __stdcall Update(tU32) {
    return 0;
  }
  void __stdcall InterruptUpdate() niImpl {
  }
};

//===========================================================================
//
//  RunnableQueue
//
//===========================================================================
struct RunnableQueue : public cIUnknownImpl<iRunnableQueue> {
  __sync_mutex();
  const tU32 _maxItems;
  astl::deque<Ptr<iRunnable> > _queue;
  tBool _canAdd;
#if !defined niNoThreads
  const tU64 _threadID;
  ThreadEvent _hasRunnable;
#endif

  RunnableQueue(tU64 aThreadID, tU32 aMaxItems)
      : _canAdd(eTrue)
      , _maxItems(aMaxItems)
#if !defined niNoThreads
      , _threadID(aThreadID)
      , _hasRunnable(eTrue)
#endif
  {}

  ~RunnableQueue() {
    Invalidate();
  }

  tU64 __stdcall GetThreadID() const {
#if !defined niNoThreads
    return _threadID;
#else
    return 0;
#endif
  }

  tU32 __stdcall GetSize() const {
    __sync_lock();
    return (tU32)_queue.size();
  }

  tBool __stdcall IsEmpty() const {
    __sync_lock();
    return _queue.empty();
  }

  void __stdcall Invalidate() {
    __sync_lock();
    _canAdd = eFalse;
    _queue.clear();
  }

  tBool __stdcall Add(iRunnable* aRunnable) {
    niCheckIsOK(aRunnable,eFalse);
    niAssert(niIsOK(aRunnable));
    __sync_lock();
    if (!_canAdd)
      return eFalse;
    // queue full ?
    niAssert(_queue.size() < (_maxItems-1));
    if (_queue.size() >= _maxItems) {
      niError("Too many items in RunnableQueue, item dropped.");
      return eFalse;
    }
    _queue.push_back(aRunnable);
#if !defined niNoThreads
    _hasRunnable.Signal();
#endif
    // RUNNABLE_QUEUE_TRACE(("RunnableQueue[%d], queue-end...", _queueIndex));
    return eTrue;
  }

  Ptr<iRunnable> __stdcall Peek() {
    __sync_lock();
#if !defined niNoThreads
    niAssert(_threadID == ni::ThreadGetCurrentThreadID());
    if (_threadID != ni::ThreadGetCurrentThreadID()) {
      return NULL;
    }
#endif
    if (_queue.empty())
      return NULL;
    Ptr<iRunnable> r = _queue.front();
    return r;
  }

  Ptr<iRunnable> __stdcall Poll() {
    __sync_lock();
#if !defined niNoThreads
    niAssert(_threadID == ni::ThreadGetCurrentThreadID());
    if (_threadID != ni::ThreadGetCurrentThreadID()) {
      niError(niFmt("Polling from wrong thread, expected:%p, got: %p.",
                    _threadID, ni::ThreadGetCurrentThreadID()));
      return NULL;
    }
#endif
    if (_queue.empty())
      return NULL;
    Ptr<iRunnable> r = _queue.front();
    _queue.pop_front();
#if !defined niNoThreads
    if (_queue.empty()) {
      _hasRunnable.Reset();
    }
#endif
    return r;
  }

  tBool __stdcall WaitForRunnable(tU32 anTimeOut) {
#if defined niNoThreads
    return eFalse;
#else
    niAssert(_threadID == ni::ThreadGetCurrentThreadID());
    if (_threadID != ni::ThreadGetCurrentThreadID()) {
      return eFalse;
    }
    return _hasRunnable.Wait(anTimeOut);
#endif
  }

#if !defined niNoThreads
  Ptr<iRunnable> __stdcall _StealPoll() {
    __sync_lock();
    _hasRunnable.Reset();
    if (_queue.empty())
      return NULL;
    Ptr<iRunnable> r = _queue.front();
    _queue.pop_front();
    return r;
  }
#endif
};

//===========================================================================
//
//  MessageDesc
//
//===========================================================================
struct sMessageDescImpl : public cIUnknownImpl<iMessageDesc>
{
  sMessageDesc _desc;

  sMessageDescImpl() {
    _desc.mnMsg = 0;
  }

  sMessageDescImpl(const sMessageDesc& aDesc) {
    _desc = aDesc;
  }

  virtual iMessageHandler* __stdcall GetHandler() const niImpl {
    return _desc.mptrHandler;
  }
  virtual tU32 __stdcall GetID() const {
    return _desc.mnMsg;
  }
  virtual const Var& __stdcall GetA() const {
    return _desc.mvarA;
  }
  virtual const Var& __stdcall GetB() const {
    return _desc.mvarB;
  }
};

//===========================================================================
//
//  MessageQueue
//
//===========================================================================
// Called by MessageQueue::Invalidate
static void _Unregister_MessageQueue(struct MessageQueue* mq);

struct MessageQueue : public cIUnknownImpl<iMessageQueue> {
  __sync_mutex();
  const tU32 _maxItems;
  astl::deque<sMessageDesc> _queue;
  tBool _canAdd;
  tU64 _threadID;
  ThreadEvent _hasMessage;

  MessageQueue(tU64 aThreadID, tU32 aMaxItems)
      : _canAdd(eTrue)
      , _maxItems(aMaxItems)
      , _threadID(aThreadID)
      , _hasMessage(eTrue)
  {}
  ~MessageQueue() {
    Invalidate();
  }

  void __stdcall Invalidate() niImpl {
    __sync_lock();
    if (_canAdd) {
      _canAdd = eFalse;
      _queue.clear();
      _Unregister_MessageQueue(this);
    }
  }

  tU64 __stdcall GetThreadID() const {
    return _threadID;
  }

  tU32 __stdcall GetSize() const {
    __sync_lock();
    return (tU32)_queue.size();
  }

  tBool __stdcall IsEmpty() const {
    __sync_lock();
    return _queue.empty();
  }

  __forceinline tBool __stdcall _Add(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) {
    __sync_lock();
    if (!_canAdd)
      return eFalse;
    // queue full ?
    niAssert(_queue.size() < (_maxItems-1));
    if (_queue.size() >= _maxItems) {
      niError("Too many items in MessageQueue, item dropped.");
      return eFalse;
    }
    _queue.push_back(sMessageDesc());
    sMessageDesc& msg = _queue.back();
    msg.mptrHandler = apHandler;
    msg.mnMsg = anMsg;
    msg.mvarA = avarA;
    msg.mvarB = avarB;
    _hasMessage.Signal();
    // RUNNABLE_QUEUE_TRACE(("MessageQueue[%d], queue-end...", _queueIndex));
    return eTrue;
  }

  tBool __stdcall Add(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) {
    niCheckIsOK(apHandler,eFalse);
    return _Add(apHandler,anMsg,avarA,avarB);
  }

  tBool __stdcall Peek(sMessageDesc* apMessageDesc) {
    __sync_lock();
    niAssert(_threadID == ni::ThreadGetCurrentThreadID());
    if (_threadID != ni::ThreadGetCurrentThreadID()) {
      return eFalse;
    }
    if (_queue.empty())
      return eFalse;
    if (apMessageDesc) {
      *apMessageDesc = _queue.front();
    }
    return eTrue;
  }

  Ptr<iMessageDesc> __stdcall PeekDesc() {
    sMessageDesc desc;
    if (Peek(&desc)) {
      return niNew sMessageDescImpl(desc);
    }
    return NULL;
  }

  tBool __stdcall Poll(sMessageDesc* apMessageDesc) {
    __sync_lock();
    niAssert(_threadID == ni::ThreadGetCurrentThreadID());
    if (_threadID != ni::ThreadGetCurrentThreadID()) {
      niError(niFmt("Polling from wrong thread, expected:%p, got: %p.",
                    _threadID, ni::ThreadGetCurrentThreadID()));
      return eFalse;
    }
    if (_queue.empty())
      return eFalse;
    if (apMessageDesc) {
      *apMessageDesc = _queue.front();
    }
    _queue.pop_front();
    if (_queue.empty()) {
      _hasMessage.Reset();
    }
    return eTrue;
  }

  Ptr<iMessageDesc> __stdcall PollDesc() {
    sMessageDesc desc;
    if (Poll(&desc)) {
      return niNew sMessageDescImpl(desc);
    }
    return NULL;
  }

  tBool __stdcall PollAndDispatch() {
    sMessageDesc msg;
    if (!Poll(&msg)) {
      return eFalse;
    }
    niAssert(msg.mptrHandler.IsOK());
    msg.mptrHandler->HandleMessage(msg.mnMsg, msg.mvarA, msg.mvarB);
    return eTrue;
  }

  tBool __stdcall WaitForMessage(tU32 anTimeOut) {
    niAssert(_threadID == ni::ThreadGetCurrentThreadID());
    if (_threadID != ni::ThreadGetCurrentThreadID()) {
      return eFalse;
    }
    return _hasMessage.Wait(anTimeOut);
  }
};

//===========================================================================
//
//  sFutureRunnable
//
//===========================================================================
struct ExecutorCooperative;

struct sFutureCooperativeRunnable : public cIUnknownImpl<iFuture,eIUnknownImplFlags_Default,iRunnable> {
  __sync_mutex();
  Ptr<ExecutorCooperative> mptrExecutor;
  Ptr<iRunnable> mptrRunnable;
  Var mRet;
  tBool mbIsDone;

  sFutureCooperativeRunnable(ExecutorCooperative* apExecutor, iRunnable* apRunnable) {
    niAssert(niIsOK(apRunnable));
    mptrExecutor = apExecutor;
    mptrRunnable = apRunnable;
    mbIsDone = eFalse;
  }
  ~sFutureCooperativeRunnable() {
    Invalidate();
  }

  void __stdcall Cancel() {
    __sync_lock();
    mptrRunnable = NULL;
    mptrExecutor = NULL;
  }

  tBool __stdcall GetIsCancelled() const {
    return !mptrRunnable.IsOK();
  }

  tBool __stdcall GetIsDone() const {
    return mbIsDone;
  }

  Var __stdcall GetValue() const {
    if (!GetIsDone())
      return niVarNull;
    return mRet;
  }

  Var __stdcall Run() {
    Ptr<iRunnable> toRun = mptrRunnable;
    if (toRun.IsOK()) {
      mRet = toRun->Run();
    }
    mbIsDone = eTrue;
    return mRet;
  }

  tBool __stdcall Wait(tU32 anMs);
};

//===========================================================================
//
//  ExecutorCooperative
//
//===========================================================================
struct ExecutorCooperative : public cIUnknownImpl<iExecutor> {
  ni::SyncCounter    _shutdownMode; // 0, continue ; 1, shutdown ; 2, shutdown now
  Ptr<RunnableQueue> _queue;
  tBool              _interruptedUpdate = eFalse;

  ExecutorCooperative(tU64 aThreadID, tU32 aMaxItems)
  {
    RUNNABLE_QUEUE_TRACE(("ExecutorCooperative, created for thread %d, with max %d items in queue",
                          aThreadID,aMaxItems));
    _queue = niNew RunnableQueue(aThreadID,aMaxItems);
  }

  ~ExecutorCooperative() {
    Invalidate();
  }

  tBool __stdcall IsOK() const {
    return eTrue;
  }

  void __stdcall Invalidate() {
    ShutdownNow(eInvalidHandle);
  }

  ni::tBool __stdcall Execute(iRunnable* aRunnable) {
    niCheckIsOK(aRunnable,eFalse);
    if (_shutdownMode.Get() != 0)
      return eFalse;
    return _queue->Add(aRunnable);
  }

  Ptr<iFuture> __stdcall Submit(iRunnable* aRunnable) {
    niCheckIsOK(aRunnable,NULL);
    if (_shutdownMode.Get() != 0)
      return NULL;
    Ptr<sFutureCooperativeRunnable> futureRun = niNew sFutureCooperativeRunnable(this,aRunnable);
    if (!_queue->Add(futureRun))
      return NULL;
    return Ptr<iFuture>(futureRun);
  }

  tBool __stdcall Shutdown(tU32 anTimeOut) {
    if (_shutdownMode.Get() == 0) {
      _shutdownMode.Set(1);
      Update(anTimeOut);
      _queue->Invalidate();
    }
    return eTrue;
  }

  tBool __stdcall ShutdownNow(tU32 anTimeOut) {
    if (_shutdownMode.Get() != 2) {
      _shutdownMode.Set(2);
      _queue->Invalidate();
    }
    return eTrue;
  }

  tBool __stdcall GetIsShutdown() const {
    return _shutdownMode.Get() != 0;
  }

  tBool __stdcall GetIsTerminated() const {
    return _shutdownMode.Get() != 0;
  }

  ni::tU32 __stdcall Update(tU32 anTimeSliceMs) {
    ExecutorCooperative* _this = this;

    Ptr<RunnableQueue> queue = _this->_queue;

    int numExecuted = 0;
    const tF64 timeSlice = ((tF64)anTimeSliceMs) / 1000.0;
    const tF64 timerStart = ni::TimerInSeconds();
    tF64 timerElapsed = 0;

    _this->_interruptedUpdate = eFalse;
    for (;;) {
      Ptr<iRunnable> r = queue->Poll();
      if (!r.IsOK() || (_this->_shutdownMode.Get() == 2)) {
        RUNNABLE_QUEUE_TRACE(("ExecutorCooperative::Update(): break, empty queue."));
        break;
      }

      r->Run();
      ++numExecuted;

      if (_this->_interruptedUpdate) {
        break;
      }

      if (_this->_shutdownMode.Get() != 0) {
        break;
      }

      timerElapsed = ni::TimerInSeconds() - timerStart;
      if (timerElapsed >= timeSlice) {
        RUNNABLE_QUEUE_TRACE(("ExecutorCooperative::Update(): break, time slice exceeded."));
        break;
      }
    }

    if (numExecuted > 0) {
      RUNNABLE_QUEUE_TRACE(("ExecutorCooperative::Update(%d, slice:%g, elapsed: %g): executed %d runnable in %d ms.",
                            anTimeSliceMs, timeSlice, timerElapsed, numExecuted, timerElapsed));
    }

    return (tU32)((ni::TimerInSeconds() - timerStart) * 1000.0);
  }

  void __stdcall InterruptUpdate() niImpl {
    this->_interruptedUpdate = eTrue;
  }
};

tBool __stdcall sFutureCooperativeRunnable::Wait(tU32 anMs) {
  tU32 execTime = 0;
  while ((!mbIsDone) && (execTime < anMs)) {
    Ptr<ExecutorCooperative> executor;
    {
      __sync_lock();
      if (!mptrRunnable.IsOK() || !mptrExecutor.IsOK())
        break;
      executor = mptrExecutor;
    }
    // 0ms time slice, only one runnable executed per loop
    execTime += executor->Update(0);
  }
  return mbIsDone;
}

//===========================================================================
//
//  sFutureRunnable
//
//===========================================================================
#if !defined niNoThreads

struct sFutureRunnable : public cIUnknownImpl<iFuture,eIUnknownImplFlags_Default,iRunnable> {
  Var mRet;
  Ptr<iRunnable> mptrRunnable;
  mutable ThreadEvent meventRan;

  sFutureRunnable(iRunnable* apRunnable)
      : meventRan(eTrue)
  {
    niAssert(niIsOK(apRunnable));
    mptrRunnable = apRunnable;
  }

  void __stdcall Cancel() {
    mptrRunnable = NULL;
  }
  tBool __stdcall GetIsCancelled() const {
    return !mptrRunnable.IsOK();
  }
  tBool __stdcall GetIsDone() const {
    return meventRan.Wait(0);
  }
  tBool __stdcall Wait(tU32 anMs) {
    return meventRan.Wait(anMs);
  }
  Var __stdcall GetValue() const {
    if (!GetIsDone()) return niVarNull;
    return mRet;
  }

  Var __stdcall Run() {
    {
      Ptr<iRunnable> toRun = mptrRunnable;
      if (toRun.IsOK()) {
        mRet = toRun->Run();
      }
    }
    meventRan.Signal();
    return mRet;
  }
};

#endif

//===========================================================================
//
//  ExecutorThreadPool
//
//===========================================================================
#if !defined niNoThreads

static tpfnConcurrentThreadCallback _pfnConcurrentThreadStart = NULL;
static tpfnConcurrentThreadCallback _pfnConcurrentThreadEnd = NULL;

niExportFunc(void) ConcurrentSetThreadStartEndCallbacks(tpfnConcurrentThreadCallback apfnStart, tpfnConcurrentThreadCallback apfnEnd) {
  _pfnConcurrentThreadStart = apfnStart;
  _pfnConcurrentThreadEnd = apfnEnd;
}

struct ExecutorThreadPool : public cIUnknownImpl<iExecutor> {
  tSyncInt         _doneCount;
  ni::SyncCounter  _shutdownMode; // 0, continue ; 1, shutdown ; 2, shutdown now
  ThreadEvent      _event;
  const ni::tU32  _numThreads;
  struct sThread {
    Ptr<iThread>       thread;
    Ptr<RunnableQueue> queue;
    ni::SyncCounter    isBusy;
    ni::SyncCounter    numExecuted;
  };
  sThread*         _threads;

  ExecutorThreadPool(ni::tU32 anNumThreads)
      : _numThreads(ni::Clamp<ni::tU32>(anNumThreads,1,64))
      , _event(eTrue)
  {
    RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool, created with %d queues",_numThreads));
    SYNC_WRITE(&_doneCount,0);
    _threads = new sThread[_numThreads];
    niLoop(i,_numThreads) {
      SYNC_WRITE(&_doneCount,i);
      _threads[i].thread = ni_create_thread();
      _threads[i].thread->Start(_Run,(void*)this);
      niPanicAssertMsg(_event.Wait(5000), "_Run should initialize the current queue.");
      _event.Reset();
    }
    SYNC_WRITE(&_doneCount,0);
  }

  ~ExecutorThreadPool() {
    RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool %p, destructor.",(tIntPtr)this));
    Invalidate();
    if (_threads) {
      delete[] _threads;
      _threads = NULL;
    }
  }

  tBool __stdcall IsOK() const {
    return eTrue;
  }

  void __stdcall Invalidate() {
    RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool %p, invalidate.",(tIntPtr)this));
    if (_shutdownMode.Get() != 2) {
      // Auto shutdown, not great in general, SleepMs to prevent race
      // conditions with IsAlive on Windows because all the threads are
      // terminated without fully running if this is collected when the app
      // terminates.
      ni::SleepMs(10);
    }
    ShutdownNow(eInvalidHandle);
  }

  tU32 __stdcall _GetMostAvailableQueue() const {
    tU32 mostAvailQueueCost = ni::TypeMax<tU32>();
    tInt index = 0;
    niLoop(i, _numThreads) {
      tU32 qCost = _threads[i].queue->GetSize() + ((_threads[i].isBusy.Get() != 0)?1:0);
      if (qCost < mostAvailQueueCost) {
        mostAvailQueueCost = qCost;
        index = i;
      }
    }
    return index;
  }

  tU32 __stdcall _GetLeastAvailableQueue() const {
    tU32 leastAvailQueueCost = 0;
    tInt index = _numThreads-1;
    niLoopr(i, _numThreads) {
      tU32 qCost = _threads[i].queue->GetSize() + ((_threads[i].isBusy.Get() != 0)?1:0);
      if (qCost > leastAvailQueueCost) {
        leastAvailQueueCost = qCost;
        index = i;
      }
    }
    return index;
  }

  Ptr<iRunnable> __stdcall _StealRunnable(const tU64 aThreadID) {
    const tU32 leastAvailQueue = _GetLeastAvailableQueue();
    niUnused(leastAvailQueue);
    EXECUTOR_STEAL_TRACE(("ExecutorThreadPool, thread %p trying to steal task from queue %d with %d tasks.", aThreadID, leastAvailQueue, _threads[leastAvailQueue].queue->GetSize()));
    Ptr<iRunnable> r = _threads[_GetLeastAvailableQueue()].queue->_StealPoll();
    if (r.IsOK()) {
      EXECUTOR_STEAL_TRACE(("ExecutorThreadPool, thread %p stole task from queue %d.", aThreadID, leastAvailQueue));
      return r;
    }
    return NULL;
  }

  ni::tBool __stdcall Execute(iRunnable* aRunnable) {
    niCheckIsOK(aRunnable,eFalse);
    if (_shutdownMode.Get() != 0)
      return eFalse;
    tU32 mostAvailableQueue = _GetMostAvailableQueue();
    // RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool, Execute queued in queue %d",mostAvailableQueue));
    niAssert(mostAvailableQueue < _numThreads);
    niAssert(_threads[mostAvailableQueue].queue.IsOK());
    ni::tBool r = _threads[mostAvailableQueue].queue->Add(aRunnable);
    return r;
  }

  Ptr<iFuture> __stdcall Submit(iRunnable* aRunnable) {
    niCheckIsOK(aRunnable,NULL);
    if (_shutdownMode.Get() != 0)
      return NULL;
    tU32 mostAvailableQueue = _GetMostAvailableQueue();
    // RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool, Submit queued in queue %d",mostAvailableQueue));
    Ptr<sFutureRunnable> futureRun = niNew sFutureRunnable(aRunnable);
    niAssert(mostAvailableQueue < _numThreads);
    niAssert(_threads[mostAvailableQueue].queue.IsOK());
    if (!_threads[mostAvailableQueue].queue->Add(futureRun))
      return NULL;
    return Ptr<iFuture>(futureRun);
  }

  tBool __stdcall Shutdown(tU32 anTimeOut) {
    if (_shutdownMode.Get() == 0) {
      _shutdownMode.Set(1);
    }
    if (_threads) {
      niLoop(i,_numThreads) {
        RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool, Shutdown signal queue %d",i));
        if (!_threads[i].thread->GetIsAlive()) {
          _IncDoneCountForShutdown();
        }
        else {
          _threads[i].queue->_hasRunnable.Signal();
        }
      }
    }
    RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool, Shutdown waiting for queues"));
    return _event.Wait(anTimeOut);
  }

  tBool __stdcall ShutdownNow(tU32 anTimeOut) {
    if (_shutdownMode.Get() == 2) {
      // Already shutdown...
      return eTrue;
    }
    _shutdownMode.Set(2);
    if (_threads) {
      niLoop(i,_numThreads) {
        // RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool, ShutdownNow signal queue %d, thread is alive %d, is busy %d.", i, _threads[i].thread->GetIsAlive(), _threads[i].isBusy.Get()));
        if (!_threads[i].thread->GetIsAlive()) {
          _IncDoneCountForShutdown();
        }
        else {
          _threads[i].queue->_hasRunnable.Signal();
        }
      }
    }
    RUNNABLE_QUEUE_TRACE(("ExecutorThreadPool, ShutdownNow waiting for queues"));
    return _event.Wait(anTimeOut);
  }

  tBool __stdcall GetIsShutdown() const {
    return _shutdownMode.Get() != 0;
  }

  tBool __stdcall GetIsTerminated() const {
    return niThis(ExecutorThreadPool)->_event.Wait(0);
  }

  tU32 __stdcall Update(tU32) {
    return 0;
  }

  void __stdcall InterruptUpdate() niImpl {
  }

  void _IncDoneCountForShutdown() {
    const int doneCount = SYNC_INCREMENT(&this->_doneCount);
    if (doneCount == (tI32)this->_numThreads) {
      RUNNABLE_QUEUE_TRACE(("... SIGNAL DONE ..."));
      this->_event.Signal();
    }
  }

  static ni::tIntPtr _Run(void* apData) {
    ExecutorThreadPool* _this = (ExecutorThreadPool*)apData;
    const tU64 threadID = ni::ThreadGetCurrentThreadID();
    const int queueIndex = SYNC_READ(&_this->_doneCount);
    Ptr<RunnableQueue> queue = niNew RunnableQueue(threadID,~0);
    sThread* thread = &_this->_threads[queueIndex];
    thread->queue = queue;
    _this->_event.Signal();
    RUNNABLE_QUEUE_TRACE(("RunnableQueue[%d], thread started.",queueIndex));

    if (_pfnConcurrentThreadStart) {
      _pfnConcurrentThreadStart(threadID);
    }

    for (;;) {
      RUNNABLE_QUEUE_TRACE(("RunnableQueue[%d], wait...", queueIndex));
      queue->WaitForRunnable(eInvalidHandle);
      queue->_hasRunnable.Reset();
      if (_this->_shutdownMode.Get() == 2) {
        RUNNABLE_QUEUE_TRACE(("RunnableQueue[%d], skipped %d runnable in the shutdown.",
                              queueIndex, queue->GetSize()));
        queue->Invalidate();
        break;
      }
      else if (_this->_shutdownMode.Get() == 1) {
        RUNNABLE_QUEUE_TRACE(("RunnableQueue[%d], shutdown 1, %d runnable.",
                              queueIndex, queue->GetSize()));
      }

      thread->isBusy.Set(1);
      for (;;) {
        RUNNABLE_QUEUE_TRACE(("RunnableQueue[%d], unqueue...", queueIndex));
        Ptr<iRunnable> r = queue->Poll();
        if (!r.IsOK() || (_this->_shutdownMode.Get() == 2)) {
          if (_this->_shutdownMode.Get() == 2) {
            break;
          }

          // Try to steal from another thread
          r = _this->_StealRunnable(threadID);
          if (!r.IsOK()) {
            break;
          }
        }
        RUNNABLE_QUEUE_TRACE(("RunnableQueue[%d], run...", queueIndex));
        r->Run();
        thread->numExecuted.Inc();
      }
      thread->isBusy.Set(0);

      if (_this->_shutdownMode.Get() != 0) {
        RUNNABLE_QUEUE_TRACE(("RunnableQueue[%d], shutdown %d", _this->_shutdownMode.Get()));
        break;
      }
    }

    if (_pfnConcurrentThreadEnd) {
      _pfnConcurrentThreadEnd(threadID);
    }

    _this->_IncDoneCountForShutdown();
    RUNNABLE_QUEUE_TRACE(
        ("RunnableQueue[%d], thread ended, executed %d runnable. %d thread closed.",
         queueIndex, thread->numExecuted.Get(), _this->_doneCount));
    return 0;
  }
};

iExecutor* New_ExecutorThreadPool(tI32 anNumThreads) {
  if (anNumThreads <= 0) {
    anNumThreads = _ASTR(ni::GetLang()->GetProperty("ni.cpu.count")).Long();
  }
  niAssert(anNumThreads >= 1 && anNumThreads < 0xFFFF);
  return niNew ExecutorThreadPool(anNumThreads);
}
#endif

//===========================================================================
//
//  ThreadRun
//
//===========================================================================
#if !defined niNoThreads

struct sThreadRun : public cIUnknownImpl<iFuture> {
  Ptr<iThread> mThread;
  Ptr<iRunnable> mptrRunnable;
  ThreadEvent meventStarted;
  Var mRet;
  SyncCounter mIsStarted;

  sThreadRun(iRunnable* apRunnable) {
    mptrRunnable = apRunnable;
    mThread = ni_create_thread();
    // This is not correct, the _ThreadProc holds a reference to this and could
    // release it before the holder takes a reference to the object
    // _Start();
  }
  ~sThreadRun() {
#if 0
    if (mThread->GetIsAlive()) {
      niWarning(
        niFmt("ThreadRun future %p destroyed in thread '%d' while its thread '%d' is still alive.",
              (void*)this,
              ni::ThreadGetCurrentThreadID(),
              mThread->GetThreadID()));
    }
#endif
    // Close only closes the thread handle it doesnt stop the thread.
    mThread->Close();
  }

  __forceinline void _Start() {
    if (mIsStarted.Get() == 0 && mIsStarted.Inc() == 1) {
      mThread->Start(_ThreadProc,(void*)this);
      niPanicAssertMsg(meventStarted.Wait(5000), "_ThreadProc should have started.");
    }
  }

  tI32 __stdcall AddRef() {
    tI32 r = BaseImpl::AddRef();
    // Start only on the first add ref, we must make sure their is a valid
    // root reference to the object before we can start it to avoid it
    // being released by the _ThreadProc before we hold it in our program
    _Start();
    return r;
  }

  void __stdcall Cancel() { }
  tBool __stdcall GetIsCancelled() const { return eFalse; }
  tBool __stdcall GetIsDone() const { return niThis(sThreadRun)->Wait(0); }
  tBool __stdcall Wait(tU32 anMs) {
    // niDebugFmt(("Waiting for %p in %d",(void*)this,ni::ThreadGetCurrentThreadID()));
    tBool b = mThread->Join(anMs);
    // niDebugFmt(("... Done Waiting for %p in %d",(void*)this,ni::ThreadGetCurrentThreadID()));
    return b;
  }
  Var __stdcall GetValue() const {
    if (!GetIsDone()) return niVarNull;
    return mRet;
  }

  static tIntPtr _ThreadProc(void* apData) {
    const tU64 threadID = ni::ThreadGetCurrentThreadID();
    if (_pfnConcurrentThreadStart) {
      _pfnConcurrentThreadStart(threadID);
    }
    // niDebugFmt(("Future %p thread start in %d",(void*)apData,threadID));
    {
      Ptr<sThreadRun> _this = (sThreadRun*)apData;
      niAssert(_this->mThread->GetThreadID() == threadID);
      _this->meventStarted.Signal();
      if (_this->mptrRunnable.IsOK()) {
        _this->mRet = _this->mptrRunnable->Run();
      }
    }
    // niDebugFmt(("Future %p thread end in %d",(void*)apData,threadID));
    if (_pfnConcurrentThreadEnd) {
      _pfnConcurrentThreadEnd(threadID);
    }
    return 0;
  }
};
#endif

//===========================================================================
//
//  Concurrent factory implementation
//
//===========================================================================

struct sConcurrent : public cIUnknownImpl<iConcurrent> {
  const tU64 _mainThreadID;
  Ptr<iExecutor> _mainExecutor;
  __sync_mutex_(mqs);
  typedef astl::map<tU64,MessageQueue*> tMQMap;
  tMQMap _mqs;

#if !defined niNoThreads
  Ptr<iExecutor> _cpuExecutor;
  Ptr<iExecutor> _ioExecutor;
#endif

  sConcurrent() : _mainThreadID(ni::ThreadGetCurrentThreadID()) {
  }
  ~sConcurrent() {
    Invalidate();
  }

  virtual void __stdcall Invalidate() niImpl {
    {
      __sync_lock_(mqs);
      if (!_mqs.empty()) {
        niWarning(niFmt("Concurrent still has %d message queues.", _mqs.size()));
      }
    }

#if !defined niNoThreads
    if (_cpuExecutor.IsOK()) {
      _cpuExecutor->Invalidate();
      _cpuExecutor = NULL;
    }

    if (_ioExecutor.IsOK()) {
      _ioExecutor->Invalidate();
      _ioExecutor = NULL;
    }
#endif

    if (_mainExecutor.IsOK()) {
      _mainExecutor->Invalidate();
      _mainExecutor = NULL;
    }
  }

  virtual tU64 __stdcall GetMainThreadID() const {
    return _mainThreadID;
  }
  virtual tU64 __stdcall GetCurrentThreadID() const {
    return ni::ThreadGetCurrentThreadID();
  }

  virtual Ptr<iFuture> __stdcall ThreadRun(iRunnable* apRunnable) {
#if defined niNoThreads
    return NULL;
#else
    niCheckIsOK(apRunnable,NULL);
    return niNew sThreadRun(apRunnable);
#endif
  }

  virtual iRunnableQueue* __stdcall CreateRunnableQueue(tU64 aThreadID, tU32 aMaxItems) {
    return niNew RunnableQueue(aThreadID,aMaxItems);
  }

  virtual iExecutor* __stdcall CreateExecutorImmediate() {
    return niNew sExecutorImmediate();
  }

  virtual iExecutor* __stdcall CreateExecutorCooperative(tU64 aThreadID, tU32 aMaxItems) {
    return niNew ExecutorCooperative(aThreadID,aMaxItems);
  }

  virtual iExecutor* __stdcall CreateExecutorThreadPool(tI32 anNumThreads) {
#if defined niNoThreads
    return NULL;
#else
    return New_ExecutorThreadPool(anNumThreads);
#endif
  }

  virtual iExecutor* __stdcall GetExecutorMain() {
    if (!_mainExecutor.IsOK()) {
      _mainExecutor = CreateExecutorCooperative(_mainThreadID,~0);
    }
    return _mainExecutor;
  }

  virtual iExecutor* __stdcall GetExecutorCPU() {
#if defined niNoThreads
    return GetExecutorMain();
#else
    if (!_cpuExecutor.IsOK()) {
      _cpuExecutor = CreateExecutorThreadPool(
          ni::Max(1,
                  _ASTR(ni::GetLang()->GetProperty("ni.cpu.count")).Long()/2));
    }
    return _cpuExecutor;
#endif
  }

  virtual iExecutor* __stdcall GetExecutorIO() {
#if defined niNoThreads
    return GetExecutorMain();
#else
    if (!_ioExecutor.IsOK()) {
      _ioExecutor = CreateExecutorThreadPool(
          ni::Max(1,
                  _ASTR(ni::GetLang()->GetProperty("ni.cpu.count")).Long()*2));
    }
    return _ioExecutor;
#endif
  }

  virtual iFutureValue* __stdcall CreateFutureValue() {
    return niNew sFutureValue();
  }

  virtual Ptr<iMessageDesc> __stdcall CreateMessageDesc(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) niImpl {
    sMessageDescImpl* pDesc = niNew sMessageDescImpl();
    sMessageDesc& desc = pDesc->_desc;
    desc.mptrHandler = apHandler;
    desc.mnMsg = anMsg;
    desc.mvarA = avarA;
    desc.mvarB = avarB;
    return pDesc;
  }

  virtual Ptr<iMessageQueue> __stdcall CreateMessageQueue(tU64 anThreadID, tU32 aMaxItems) niImpl {
    __sync_lock_(mqs);
    if (_mqs.find(anThreadID) != _mqs.end()) {
      niError(niFmt("MessageQueue already created for thread '%d'.", anThreadID));
      return NULL;
    }
    Ptr<MessageQueue> mq = niNew MessageQueue(anThreadID,aMaxItems);
    niAssert(niIsOK(mq));
    astl::upsert(_mqs,mq->_threadID,mq.ptr());
    return (Ptr<iMessageQueue>&)mq;
  }

  virtual Ptr<iMessageQueue> __stdcall GetMessageQueue(tU64 anThreadID) const niImpl {
    __sync_lock_(mqs);
    tMQMap::const_iterator it = _mqs.find(anThreadID);
    if (it == _mqs.end())
      return NULL;
    return it->second;
  }

  virtual tBool __stdcall SendMessage(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) niImpl {
    niCheckIsOK(apHandler,eFalse);
    const tU64 handlerThreadID = apHandler->GetThreadID();
    if (handlerThreadID == ni::eInvalidHandle ||
        handlerThreadID == ni::ThreadGetCurrentThreadID())
    {
      // handle the message...
      apHandler->HandleMessage(anMsg,avarA,avarB);
      return eTrue;
    }
    else {
      return _QueueMessage(handlerThreadID,apHandler,anMsg,avarA,avarB);
    }
  }

  //! Queue a message in the messge queue of the message handler's thread.
  //! \return eTrue if the message as been added to the handler's thread
  //!         message queue. Otherwise returns eFalse.
  virtual tBool __stdcall QueueMessage(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) niImpl {
    niCheckIsOK(apHandler,eFalse);
    const tU64 handlerThreadID = apHandler->GetThreadID();
    return _QueueMessage(handlerThreadID,apHandler,anMsg,avarA,avarB);
  }

  inline tBool __stdcall _QueueMessage(
      const tU64 handlerThreadID,
      iMessageHandler* apHandler,
      tU32 anMsg, const Var& avarA, const Var& avarB)
  {
    __sync_lock_(mqs);
    tMQMap::iterator it = _mqs.find(handlerThreadID);
    if (it == _mqs.end()) {
      niError(niFmt("No message queue for thread '%d' to queue message '%s'.",
                    handlerThreadID,
                    MessageID_ToString(anMsg),
                    avarA, avarB));
      return eFalse;
    }
    // queue the message
    return it->second->_Add(apHandler,anMsg,avarA,avarB);
  }

  void _Unregister_MessageQueue(MessageQueue* mq) {
    __sync_lock_(mqs);
    niAssert(niIsOK(mq));
    niAssert(_mqs.find(mq->_threadID) != _mqs.end());
    astl::map_erase(_mqs,mq->_threadID);
  }

  Ptr<MessageQueue> _Find_MessageQueue(tU64 anThreadID) {
    __sync_lock_(mqs);
    tMQMap::const_iterator it = _mqs.find(anThreadID);
    if (it == _mqs.end())
      return NULL;
    return it->second;
  }
};

static sConcurrent* _GetConcurrent() {
  static Ptr<sConcurrent> _ptrConcurrent;
  if (!_ptrConcurrent.IsOK()) {
    _ptrConcurrent = niNew sConcurrent();
  }
  return _ptrConcurrent;
}

static void _Unregister_MessageQueue(MessageQueue* mq) {
  _GetConcurrent()->_Unregister_MessageQueue(mq);
}

namespace ni {

niExportFunc(iConcurrent*) GetConcurrent() {
  return _GetConcurrent();
}

niExportFunc(iUnknown*) New_niLang_Concurrent(const Var& avarA, const Var& avarB) {
  return _GetConcurrent();
}

}

//===========================================================================
//
//  MessageHandler DispatchWrapper
//
//===========================================================================

static const ni::sParameterDef iMessageHandler_HandleMessage_Parameters[3] = {
	{ "anMsg", ni::eTypeFlags_Constant|ni::eType_U32, NULL, "const tU32" },
	{ "avarA", ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer, NULL, "const Var&" },
	{ "avarB", ni::eTypeFlags_Constant|ni::eType_Variant|ni::eTypeFlags_Pointer, NULL, "const Var&" }
};
static const ni::sMethodDef iMessageHandler_HandleMessage = {
	"HandleMessage",
	0|ni::eType_Null, NULL, "void",
	3, iMessageHandler_HandleMessage_Parameters
#if !defined niConfig_NoXCALL
  , NULL
#endif
};
static const ni::sMethodDef* Methods_iMessageHandler[] = {
  &iMessageHandler_HandleMessage
};

struct niHidden iMessageHandler_DispatchWrapper : public ImplAggregate<ni::iMessageHandler>
{
  const tU64 _threadID;

  iMessageHandler_DispatchWrapper(ni::iDispatch* apDispatch)
      : _threadID(ni::ThreadGetCurrentThreadID())
  {
    mprotected_pAggregateParent = apDispatch;
  }

  ~iMessageHandler_DispatchWrapper() {
  }

  ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    if (aIID == niGetInterfaceUUID(iMessageHandler))
      return this;
    return mprotected_pAggregateParent->QueryInterface(aIID);
  }
  void __stdcall ListInterfaces(ni::iMutableCollection* apLst, ni::tU32 anFlags) const {
    apLst->Add(niGetInterfaceUUID(iMessageHandler));
    mprotected_pAggregateParent->ListInterfaces(apLst,anFlags);
  }

  // Method (0): GetThreadID
  tU64 __stdcall GetThreadID() const {
    return _threadID;
  }

  // Method (1): HandleMessage
  void __stdcall HandleMessage(const tU32 anMsg, const Var& avarA, const Var& avarB)
  {
    niAssert(ni::ThreadGetCurrentThreadID() == _threadID);
    ni::Var _params_[3];
    _params_[0] = anMsg;
    _params_[1] = avarA;
    _params_[2] = avarB;
    ((iDispatch*)mprotected_pAggregateParent)->CallMethod(
        &iMessageHandler_HandleMessage, 0,
        _params_, niCountOf(_params_), NULL);
  }
};

namespace ni {
ni::iUnknown* iMessageHandler_CreateDispatchWrapper(ni::iDispatch* apDispatch) {
  niAssert(niIsOK(apDispatch));
  if (!apDispatch->InitializeMethods(Methods_iMessageHandler,niCountOf(Methods_iMessageHandler))) {
    niError(niFmt("Can't initialize dispatch methods for iMessageHandler."));
    return NULL;
  }
  return niNew iMessageHandler_DispatchWrapper(apDispatch);
}
}
