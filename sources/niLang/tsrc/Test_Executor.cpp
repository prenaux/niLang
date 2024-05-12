#include "stdafx.h"
#if !defined niNoThreads

#include "../src/API/niLang/Utils/ConcurrentImpl.h"
#include "../src/API/niLang/Utils/TimerSleep.h"

using namespace ni;

struct FExecutor {};

static const ni::tU32 _knFExecutorNumRun = 20;
static const tU32 _knFExecutorThreadCount =  4;

TEST_FIXTURE(FExecutor,FuncPtr) {
  struct _Local {
    static ni::SyncCounter& counter() {
      static ni::SyncCounter _counter;
      return _counter;
    }
    static Var run() {
      counter().Inc();
      ni::SleepMs(10);
      niDebugFmt(("ThreadPool, Runned FuncPtr [%d] on thread '0x%x'.",
                  counter().Get(),
                  ni::ThreadGetCurrentThreadID()));
      return 0;
    };
  };

  const ni::tU32 kNumRun = _knFExecutorNumRun;
  Ptr<iExecutor> exec = ni::GetConcurrent()->CreateExecutorThreadPool(_knFExecutorThreadCount);
  niLoop(i,kNumRun) {
    exec->Execute(Runnable(&_Local::run));
  }
  CHECK_EQUAL(0,exec->Shutdown(0));
  CHECK_EQUAL(eTrue,exec->GetIsShutdown());
  CHECK_EQUAL(eFalse,exec->GetIsTerminated());
  CHECK_EQUAL(eTrue,exec->Shutdown(eInvalidHandle));
  CHECK_EQUAL(eTrue,exec->GetIsShutdown());
  CHECK_EQUAL(eTrue,exec->GetIsTerminated());
  CHECK_EQUAL(kNumRun,_Local::counter().Get());
}

#ifdef niCPP11
TEST_FIXTURE(FExecutor,Main) {
  struct _Local {
    static ni::SyncCounter& counter() {
      static ni::SyncCounter _counter;
      return _counter;
    }
    static Var run() {
      counter().Inc();
      ni::SleepMs(10);
      niDebugFmt(("Main, Runned FuncPtr [%d] on thread '0x%x'.",
                  counter().Get(),
                  ni::ThreadGetCurrentThreadID()));
      return 0;
    };
  };

  const ni::tU32 kNumRun = _knFExecutorNumRun;
  Ptr<iExecutor> exec = ni::GetConcurrent()->GetExecutorMain();
  niLoop(i,kNumRun) {
    niExec_(exec) {
      _Local::run();
      return ni::eTrue;
    };
  }
  exec->Update(3000);
  CHECK_EQUAL(eTrue,exec->Shutdown(eInvalidHandle));
  CHECK_EQUAL(eTrue,exec->GetIsShutdown());
  CHECK_EQUAL(eTrue,exec->GetIsTerminated());
  CHECK_EQUAL(kNumRun,_Local::counter().Get());
}

struct MyCounter : public ni::ImplRC<ni::iUnknown> {
  ni::SyncCounter _v;
  void Inc() {
    _v.Inc();
  }
  ni::tI32 Get() const {
    return _v.Get();
  }
};

TEST_FIXTURE(FExecutor,Lambda) {
  const ni::tU32 kNumRun = _knFExecutorNumRun;
  Ptr<MyCounter> counter = niNew MyCounter();
  CHECK_EQUAL(1,counter->GetNumRefs());
  {
    // Here we use [=] so that the lambda copies the counter Ptr
    // (and so does a proper AddRef/Release)
    auto runMe = [=]() -> int {
      counter->Inc();
      ni::SleepMs(10);
      niDebugFmt(("ThreadPool, Runned Lambda [%d] (%d) on thread '0x%x'.",
                  counter->Get(),
                  counter->GetNumRefs(),
                  ni::ThreadGetCurrentThreadID()));
      return 0;
    };
    // runMe should have one ref to the counter, so now should
    // have 2 refs
    CHECK_EQUAL(2,counter->GetNumRefs());
    Ptr<iExecutor> exec = ni::GetConcurrent()->CreateExecutorThreadPool(_knFExecutorThreadCount);
    niLoop(i,kNumRun) {
      exec->Execute(Runnable(runMe));
    }
    CHECK_EQUAL(2+kNumRun,counter->GetNumRefs());

    CHECK_EQUAL(eFalse,exec->Shutdown(0));
    CHECK_EQUAL(eTrue,exec->GetIsShutdown());
    CHECK_EQUAL(eFalse,exec->GetIsTerminated());
    CHECK_EQUAL(eTrue,exec->Shutdown(eInvalidHandle));
    CHECK_EQUAL(eTrue,exec->GetIsShutdown());
    CHECK_EQUAL(eTrue,exec->GetIsTerminated());

    // 2 refs now, all the refs added by the runnables should have
    // been release when the executor was shutdown
    // (exec->Invalidate())
    CHECK_EQUAL(2,counter->GetNumRefs());
  }

  // now we should have only one ref...
  CHECK_EQUAL(1,counter->GetNumRefs());
  CHECK_EQUAL(kNumRun,counter->Get());
}

TEST_FIXTURE(FExecutor,TaskStealing) {
  AUTO_WARNING_MODE();

  astl::vector<Ptr<iFuture> > done;
  Ptr<iExecutor> executor = GetConcurrent()->CreateExecutorThreadPool(2);
  tU32 taskSeqCounter = 0;
  tU64 taskThread1 = eInvalidHandle+1;
  tU64 taskSeq1 = eInvalidHandle;
  tU64 taskThread2 = eInvalidHandle+2;
  tU64 taskSeq2 = eInvalidHandle;
  tU64 taskThread3 = eInvalidHandle+3;
  tU64 taskSeq3 = eInvalidHandle;
  tU64 taskThread4 = eInvalidHandle+4;
  tU64 taskSeq4 = eInvalidHandle;
  tU64 taskThread5 = eInvalidHandle+5;
  tU64 taskSeq5 = eInvalidHandle;

  // should go in queue 0
  done.push_back(niSubmit_(executor,&taskThread1,&taskSeq1,&taskSeqCounter) {
    taskThread1 = ni::ThreadGetCurrentThreadID();
    taskSeq1 = taskSeqCounter++;
    niDebugFmt(("... Task1 Start: %p", ni::ThreadGetCurrentThreadID()));
    ni::SleepMs(200);
    niDebugFmt(("... Task1 Done"));
    return ni::eTrue;
  });
  // should go in queue 1
  done.push_back(niSubmit_(executor,done,&taskThread2,&taskSeq2,&taskSeqCounter) {
    taskThread2 = ni::ThreadGetCurrentThreadID();
    taskSeq2 = taskSeqCounter++;
    niDebugFmt(("... Task2 Start: %p", ni::ThreadGetCurrentThreadID()));
    ni::SleepMs(100);
    niDebugFmt(("... Task2 Done"));
    return ni::eTrue;
  });
  // should go in queue 0
  done.push_back(niSubmit_(executor,done,&taskThread3,&taskSeq3,&taskSeqCounter) {
    taskThread3 = ni::ThreadGetCurrentThreadID();
    taskSeq3 = taskSeqCounter++;
    niDebugFmt(("... Task3 Start: %p", ni::ThreadGetCurrentThreadID()));
    ni::SleepMs(100);
    niDebugFmt(("... Task3 Done"));
    return ni::eTrue;
  });
  // should go in queue 1
  done.push_back(niSubmit_(executor,done,&taskThread4,&taskSeq4,&taskSeqCounter) {
    taskThread4 = ni::ThreadGetCurrentThreadID();
    taskSeq4 = taskSeqCounter++;
    niDebugFmt(("... Task4 Start: %p", ni::ThreadGetCurrentThreadID()));
    ni::SleepMs(50);
    niDebugFmt(("... Task4 Done"));
    return ni::eTrue;
  });
  // should steal Task3 from queue 0
  // should go in queue 0
  done.push_back(niSubmit_(executor,done,&taskThread5,&taskSeq5,&taskSeqCounter) {
    taskThread5 = ni::ThreadGetCurrentThreadID();
    taskSeq5 = taskSeqCounter++;
    niDebugFmt(("... Task5 Start: %p", ni::ThreadGetCurrentThreadID()));
    ni::SleepMs(100);
    niDebugFmt(("... Task5 Done"));
    return ni::eTrue;
  });

  niLoop(i,done.size()) {
    done[i]->Wait(eInvalidHandle);
  }
  niDebugFmt(("... All tasks completed"));

  // task 1 & 2 must run on different thread
  CHECK_NOT_EQUAL(taskThread1,taskThread2);
  // task 1 & 5 should run on the same thread
  CHECK_EQUAL(taskThread1,taskThread5);
  // task 2, 4 & 3 should run on the same thread - in that order, task3 is
  // stolen from the first queue
  CHECK_EQUAL(taskThread2,taskThread3);
  CHECK_EQUAL(taskThread2,taskThread4);
  // Run order should be task 1 (q0), 2 (q1), 4 (q1), 3 (q1 stolen from q0), 5
  CHECK_EQUAL(5,taskSeqCounter);
  CHECK_EQUAL(0,taskSeq1);
  CHECK_EQUAL(1,taskSeq2);
  CHECK_EQUAL(2,taskSeq4);
  CHECK_EQUAL(3,taskSeq3);
  CHECK_EQUAL(4,taskSeq5);
}
#endif
#endif
