#pragma once
#ifndef __ICONCURRENT_H_C9263DE3_7687_4E07_A410_06B9C9F2637F__
#define __ICONCURRENT_H_C9263DE3_7687_4E07_A410_06B9C9F2637F__
#include "Types.h"

#include "Utils/SmartPtr.h"
#include "Utils/SinkList.h"
#include "Utils/MessageID.h"
#include "Var.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Runnable interface.
struct iRunnable : public iUnknown {
  niDeclareInterfaceUUID(iRunnable,0xd9ba854f,0x6310,0x4be8,0xa9,0xb7,0x0d,0xf7,0x4e,0xce,0x7b,0x35);

  virtual Var __stdcall Run() = 0;
};

//! Callback interface.
//! \remark A callback is similar to a runnable excepted that it accepts up to
//!         two parameters. If it used as a runnable both parameters are set
//!         to null.
struct iCallback : public iRunnable {
  niDeclareInterfaceUUID(iCallback,0x8ccc2096,0xa1b2,0x4bf3,0x9c,0x78,0x41,0x02,0x78,0x41,0x9f,0x4c);

  virtual Var __stdcall RunCallback(const Var& avarA, const Var& avarB) = 0;
};

//! Runnable queue interface.
struct iRunnableQueue : public iUnknown {
  niDeclareInterfaceUUID(iRunnableQueue,0x3c6398c4,0x7aea,0x4aea,0x84,0x26,0x2f,0x4c,0xc7,0x1d,0xa0,0xa9);

  //! Get the runnable queue's owner thread.
  //! {Property}
  virtual tU64 __stdcall GetThreadID() const = 0;
  //! Get the number of runnable currently in the queue.
  //! {Property}
  virtual tU32 __stdcall GetSize() const = 0;
  //! Check whether any runnable is in the queue.
  virtual tBool __stdcall IsEmpty() const = 0;
  //! Queue a runnable.
  //! \return eFalse if the queue is full, else eTrue.
  virtual tBool __stdcall Add(iRunnable* apRunnable) = 0;
  //! Retrieves, but does not remove, the head of this queue, or
  //! returns null if this queue is empty or if called from another
  //! thread than the owner thread.
  virtual Ptr<iRunnable> __stdcall Peek() = 0;
  //! Retrieves and removes the head of this queue, or returns null
  //! if this queue is empty or if called from another thread than
  //! the owner thread.
  virtual Ptr<iRunnable> __stdcall Poll() = 0;
  //! Wait for a runnable to be in the queue.
  //! \return eFalse if the called from another thread than the other thread.
  virtual tBool __stdcall WaitForRunnable(tU32 anTimeOut) = 0;
};

//! Message handler interface.
//! {DispatchWrapperCreate=iMessageHandler_CreateDispatchWrapper}
struct iMessageHandler : public iUnknown
{
  niDeclareInterfaceUUID(iMessageHandler,0xa38a4904,0x29bf,0x462a,0x9e,0x84,0xba,0xf9,0x0e,0xdd,0x6d,0xc3);
  //! Get the message handler's owner thread. That is the thread where HandleMessage will be called.
  //! {Property}{Optional}
  virtual tU64 __stdcall GetThreadID() const = 0;
  //! Called when a message should be handled.
  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& avarA, const Var& avarB) = 0;
};

//! Message handler list.
typedef SinkList<iMessageHandler> tMessageHandlerSinkLst;

struct sMessageDesc {
  Ptr<iMessageHandler> mptrHandler;
  tU32 mnMsg;
  Var  mvarA;
  Var  mvarB;
};

//! Message description interface.
struct iMessageDesc : public iUnknown
{
  niDeclareInterfaceUUID(iMessageDesc,0xeba551c4,0x82a5,0x4269,0xac,0x3e,0xa1,0x75,0xf5,0xa0,0x4f,0xfd);
  //! Get the message handler.
  //! {Property}
  virtual iMessageHandler* __stdcall GetHandler() const = 0;
  //! Get the message id.
  //! {Property}
  virtual tU32 __stdcall GetID() const = 0;
  //! Get the message parameter A.
  //! {Property}
  virtual const Var& __stdcall GetA() const = 0;
  //! Get the message parameter B.
  //! {Property}
  virtual const Var& __stdcall GetB() const = 0;
};

//! Message queue interface.
struct iMessageQueue : public iUnknown {
  niDeclareInterfaceUUID(iMessageQueue,0xda9d184a,0xdfdb,0x49e0,0x91,0xb8,0xcf,0x17,0xae,0x02,0xca,0x1c);

  //! Get the message queue's owner thread.
  //! {Property}
  virtual tU64 __stdcall GetThreadID() const = 0;
  //! Get the number of message currently in the queue.
  //! {Property}
  virtual tU32 __stdcall GetSize() const = 0;
  //! Check whether any message is in the queue.
  virtual tBool __stdcall IsEmpty() const = 0;
  //! Queue a message.
  //! \return eFalse if the queue is full or if the message handler's thread
  //!         is not the same as the message queue's thread, else eTrue.
  virtual tBool __stdcall Add(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) = 0;
  //! Retrieves, but does not remove, the head of this queue, or returns
  //! eFalse if this queue is empty or if called from another thread than the
  //! owner thread.
  //! {NoAutomation}
  virtual tBool __stdcall Peek(sMessageDesc* apMessageDesc) = 0;
  //! Same as \see Peek but returns a new iMessageDesc object.
  virtual Ptr<iMessageDesc> __stdcall PeekDesc() = 0;
  //! Retrieves and removes the head of this queue, or returns eFalse if this
  //! queue is empty or if called from another thread than the owner thread.
  //! {NoAutomation}
  virtual tBool __stdcall Poll(sMessageDesc* apMessageDesc) = 0;
  //! Same as \see Poll but returns a new iMessageDesc object.
  virtual Ptr<iMessageDesc> __stdcall PollDesc() = 0;
  //! Retrieves, removes and call the message handler of the head of this
  //! queue, or returns eFalse if this queue is empty or if called from
  //! another thread than the owner thread.
  virtual tBool __stdcall PollAndDispatch() = 0;
  //! Wait for a message to be in the queue.
  //! \return eFalse if the called from another thread than the other thread.
  virtual tBool __stdcall WaitForMessage(tU32 anTimeOut) = 0;
};

//! Future interface.
//!
//! A Future represents the result of an asynchronous
//! computation. Methods are provided to check if the computation is
//! complete, to wait for its completion, and to retrieve the result
//! of the computation.
struct iFuture : public iUnknown
{
  niDeclareInterfaceUUID(iFuture,0xca4f4d13,0x5556,0x4177,0xbb,0x7f,0x41,0x85,0x5a,0x40,0x93,0xac);
  //! Cancel the task associated with the future.
  virtual void __stdcall Cancel() = 0;
  //! Check whether the task has been cancelled.
  //! {Property}
  virtual tBool __stdcall GetIsCancelled() const = 0;
  //! Return true if this task completed.
  //! {Property}
  //! \remark Equivalent to Wait(0).
  virtual tBool __stdcall GetIsDone() const = 0;
  //! Wait for the value to be set.
  //! \return eFalse if the value hasn't be set in the specified timeout.
  virtual tBool __stdcall Wait(tU32 anTimeOut) = 0;
  //! Return the result of the computation, returns null if the task is not
  //! completed.
  //! {Property}
  virtual Var __stdcall GetValue() const = 0;
};

//! Future value.
struct iFutureValue : public iFuture
{
  niDeclareInterfaceUUID(iFutureValue,0xb79d1f47,0x0f71,0x495a,0x8d,0x23,0x55,0x50,0xce,0x57,0x6a,0x59);
  //! Sets and signal the future.
  //! {Property}
  virtual void __stdcall SetValue(const Var& aValue) = 0;
  //! Reset the future to the unset & not-canceled state.
  virtual void __stdcall Reset() = 0;
};

//! Executes the submitted iRunnable tasks.
//!
//! This interface provides a way of decoupling task submission from
//! the mechanism of how each task will be run, including the details
//! of thread use, scheduling, etc.
//!
//! An executor is normally used instead of explicitly creating
//! threads. However the it does not strictly require the execution to
//! be asynchronous.
struct iExecutor : public iUnknown {
  niDeclareInterfaceUUID(iExecutor,0x4990b16a,0xd3cf,0x42c0,0xb9,0x33,0x5a,0x9b,0xc0,0x6b,0x65,0x5f);

  //! Returns true if this executor has been shut down.
  //! {Property}
  virtual tBool __stdcall GetIsShutdown() const = 0;

  //! Returns true if all tasks have completed following shut down.
  //! {Property}
  virtual tBool __stdcall GetIsTerminated() const = 0;

  //! Executes the given runnable at some time in the future.
  //!
  //! The runnable might execute in a new thread, a thread pool, or
  //! in the calling thread, at the discretion of the executor
  //! implementation.
  //!
  //! \param aRunnable the runnable to execute.
  //! \return false if the executor has been invalidated and cannot
  //!         execute anymore runnable.
  virtual ni::tBool __stdcall Execute(iRunnable* aRunnable) = 0;

  //! Executes the given runnable at some time in the future.
  //!
  //! The runnable might execute in a new thread, a thread pool, or
  //! in the calling thread, at the discretion of the executor
  //! implementation.
  //!
  //! \param aRunnable the runnable to execute.
  //! \return A iFuture that is set to the return value of the runnable when
  //!         it returns. NULL if the executor has been invalidated and cannot
  //!         execute anymore runnable.
  virtual Ptr<ni::iFuture> __stdcall Submit(iRunnable* aRunnable) = 0;

  //! Initiates an orderly shutdown in which previously submitted
  //! tasks are executed, but no new tasks will be accepted. Blocks
  //! until all tasks have completed execution or the timeout
  //! occurs.
  //! \return true if this executor terminated and false if
  //!         the timeout elapsed before termination.
  virtual tBool __stdcall Shutdown(tU32 anTimeOut) = 0;

  //! Attempts to cancel all actively executing tasks, halts the
  //! processing of waiting tasks. Blocks until all tasks have
  //! completed execution or the timeout occurs.
  //! \return true if this executor terminated and false if the
  //!         timeout elapsed before termination.
  //! \remark Invalidate calls ShutdownNow(eInvalidHandle).
  virtual tBool __stdcall ShutdownNow(tU32 anTimeOut) = 0;

  //! Update the executor.
  //! \param anTimeSliceInMs the suggested maximum time allocated to execute
  //!        the runnables queued in the executor, at least one runnable is
  //!        executed per update if any are queued.
  //! \return The duration (in milliseconds) of the execution of the
  //!         runnables. Always 0 for immediate and thread pool executors.
  //! \remark On multi-threaded platforms this call is superflous for all
  //!         but the cooperative executors. For the default executors
  //!         provided in iConcurrent this should be called in the main app's
  //!         for the main executor (which is always cooperative).
  virtual tU32 __stdcall Update(tU32 anTimeSliceInMs) = 0;
};

struct iConcurrent : public iUnknown {
  niDeclareInterfaceUUID(iConcurrent,0x8553d3d4,0x95f9,0x4438,0xb9,0x70,0x06,0xa9,0xf0,0x6a,0x74,0xa7);

  //! {Property}
  virtual tU64 __stdcall GetMainThreadID() const = 0;
  //! {Property}
  virtual tU64 __stdcall GetCurrentThreadID() const = 0;

  //! Create a runnable queue for the specified thread.
  virtual iRunnableQueue* __stdcall CreateRunnableQueue(tU64 aThreadID, tU32 aMaxItems) = 0;

  //! Create a cooperative executor.
  virtual iExecutor* __stdcall CreateExecutorCooperative(tU64 aThreadID, tU32 aMaxItems) = 0;

  //! Create an immediate executor.
  virtual iExecutor* __stdcall CreateExecutorImmediate() = 0;

  //! Create a threaded executor.
  //! \remark Always fails on single-threaded platforms.
  virtual iExecutor* __stdcall CreateExecutorThreadPool(tI32 aNumThreads) = 0;

  //! Get the default executor.
  //!
  //! The default executor is created with one thread for 2 logical cores, with at
  //! least one thread. It should be used for CPU intensive tasks.
  //!
  //! \remark On purely single-threaded platforms returns GetExecutorMain().
  //!
  //! {Property}
  virtual iExecutor* __stdcall GetExecutorCPU() = 0;

  //! Get the IO executor.
  //!
  //! The IO executor is created with 2 threads per logical core. It should
  //! be used for IO bound tasks.
  //!
  //! \remark On purely single-threaded platforms returns GetExecutorMain().
  //!
  //! {Property}
  virtual iExecutor* __stdcall GetExecutorIO() = 0;

  //! Get the cooperative executor bound to the main thread.
  //! {Property}
  //! \remark All the runnable submitted to this executor are garanteed to be executed in the main thread.
  virtual iExecutor* __stdcall GetExecutorMain() = 0;

  //! Creates a new thread and run the runnable in it.
  virtual Ptr<iFuture> __stdcall ThreadRun(iRunnable* apRunnable) = 0;

  //! Create a future value object.
  virtual iFutureValue* __stdcall CreateFutureValue() = 0;

  //! Create a message desc object.
  virtual Ptr<iMessageDesc> __stdcall CreateMessageDesc(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) = 0;

  //! Create a message queue for the specified thread.
  //! \remark Only one message queue can be created per thread, subsequent calls to create a queue for the same thread will fail.
  virtual Ptr<iMessageQueue> __stdcall CreateMessageQueue(tU64 anThreadID, tU32 aMaxItems) = 0;

  //! Get the message queue associated with the specified thread.
  //! {Property}
  virtual Ptr<iMessageQueue> __stdcall GetMessageQueue(tU64 anThreadID) const = 0;

  //! Send a message to the specified message handler.
  //! \remark If the handler's thread is the same as the current thread then
  //!         the handler is called immediatly. Otherwise the message queue's
  //!         of the handler's thread is retrieved and the message is pushed
  //!         in the queue.
  //! \return eTrue if the message as been added to the handler's thread
  //!         message queue or if the handler has been called
  //!         immediatly. Otherwise returns eFalse.
  virtual tBool __stdcall SendMessage(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) = 0;

  //! Queue a message in the messge queue of the message handler's thread.
  //! \return eTrue if the message as been added to the handler's thread
  //!         message queue. Otherwise returns eFalse.
  virtual tBool __stdcall QueueMessage(iMessageHandler* apHandler, tU32 anMsg, const Var& avarA, const Var& avarB) = 0;
};

niExportFunc(iConcurrent*) GetConcurrent();

niExportFunc(ni::iUnknown*) New_niLang_Concurrent(const ni::Var&,const ni::Var&);

///////////////////////////////////////////////
inline tBool __stdcall SendMessage(iMessageHandler* apMT, tU32 anID, const Var& avarA = (Var&)niVarNull, const Var& avarB = (Var&)niVarNull) {
  return GetConcurrent()->SendMessage(apMT,anID,avarA,avarB);
}

///////////////////////////////////////////////
inline tBool __stdcall SendMessages(tMessageHandlerSinkLst* apMT, tU32 anID, const Var& avarA = (Var&)niVarNull, const Var& avarB = (Var&)niVarNull) {
  if (!apMT) return eFalse;
  niLoopSink(iMessageHandler,it,apMT) {
    if (!GetConcurrent()->SendMessage(it->_Value(),anID,avarA,avarB))
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
inline tBool __stdcall QueueMessage(iMessageHandler* apMT, tU32 anID, const Var& avarA = (Var&)niVarNull, const Var& avarB = (Var&)niVarNull) {
  return GetConcurrent()->QueueMessage(apMT,anID,avarA,avarB);
}

///////////////////////////////////////////////
inline tBool __stdcall QueueMessages(tMessageHandlerSinkLst* apMT, tU32 anID, const Var& avarA = (Var&)niVarNull, const Var& avarB = (Var&)niVarNull) {
  if (!apMT) return eFalse;
  niLoopSink(iMessageHandler,it,apMT) {
    if (!GetConcurrent()->QueueMessage(it->_Value(),anID,avarA,avarB))
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
inline Ptr<iMessageQueue> GetOrCreateMessageQueue(tU64 anThreadID, tU32 aMaxItems = ~0) {
  Ptr<iMessageQueue> mq = ni::GetConcurrent()->GetMessageQueue(anThreadID);
  if (mq.IsOK())
    return mq;
  mq = ni::GetConcurrent()->CreateMessageQueue(anThreadID,aMaxItems);
  return mq;
}

/**@}*/
}
#endif // __ICONCURRENT_H_C9263DE3_7687_4E07_A410_06B9C9F2637F__
