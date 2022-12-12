#ifndef __CONCURRENTIMPL_H_EFDFC60D_4821_40B9_AE06_54171ACE7604__
#define __CONCURRENTIMPL_H_EFDFC60D_4821_40B9_AE06_54171ACE7604__
#include "../Types.h"

#include "../IConcurrent.h"
#include "../ILang.h"
#include "../Var.h"
#include "VarCast.h"
#include "UnknownImpl.h"
#include "ThreadImpl.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! To be used with the Runnable<> template to specify a function pointer instead of a lambda.
typedef Var (__ni_export_call_decl *tpfnRunnable)();

//! To be used with the Callback0<> template to specify a function pointer instead of a lambda.
typedef Var (__ni_export_call_decl *tpfnCallback0)();

//! To be used with the Callback1<> template to specify a function pointer instead of a lambda.
typedef Var (__ni_export_call_decl *tpfnCallback1)(const Var& avarA);

//! To be used with the Callback2<> template to specify a function pointer instead of a lambda.
typedef Var (__ni_export_call_decl *tpfnCallback2)(const Var& avarA, const Var& avarB);

template <typename T>
struct sRunnable : public ni::cIUnknownImpl<iRunnable> {
  const T _lambda;
  sRunnable(const T& aLambda) : _lambda(aLambda) {}
  virtual Var __stdcall Run() { return _lambda(); }
};
template <typename T>
Nonnull<iRunnable> Runnable(const T& aLambda) {
  return ni::MakeNonnull<sRunnable<T> >(aLambda);
}


template <typename T>
struct sCallback0 : public ni::cIUnknownImpl<iCallback,eIUnknownImplFlags_DontInherit1,iRunnable> {
  const T _lambda;
  sCallback0(const T& aLambda) : _lambda(aLambda) {}
  virtual Var __stdcall Run() { return _lambda(); }
  virtual Var __stdcall RunCallback(const Var& avarA, const Var& avarB) {
    return _lambda();
  }
};
template <typename T>
Nonnull<iCallback> Callback0(const T& aLambda) {
  return ni::MakeNonnull<sCallback0<T> >(aLambda);
}

template <typename T>
struct sCallback1 : public ni::cIUnknownImpl<iCallback,eIUnknownImplFlags_DontInherit1,iRunnable> {
  const T _lambda;
  sCallback1(const T& aLambda) : _lambda(aLambda) {}
  virtual Var __stdcall Run() { return _lambda(niVarCast(niVarNull)); }
  virtual Var __stdcall RunCallback(const Var& avarA, const Var& avarB) {
    return _lambda(niVarCast(avarA));
  }
};
template <typename T>
Nonnull<iCallback> Callback1(const T& aLambda) {
  return ni::MakeNonnull<sCallback1<T> >(aLambda);
}

template <typename T>
struct sCallback2 : public ni::cIUnknownImpl<iCallback,eIUnknownImplFlags_DontInherit1,iRunnable> {
  const T _lambda;
  sCallback2(const T& aLambda) : _lambda(aLambda) {}
  virtual Var __stdcall Run() { return _lambda(niVarNull, niVarNull); }
  virtual Var __stdcall RunCallback(const Var& avarA, const Var& avarB) {
    return _lambda(niVarCast(avarA),niVarCast(avarB));
  }
};
template <typename T>
Nonnull<iCallback> Callback2(const T& aLambda) {
  return ni::MakeNonnull<sCallback2<T> >(aLambda);
}

//! To be used with the MessageHandler<> template to specify a function pointer instead of a lambda.
typedef void (__ni_export_call_decl *tpfnMessageHandler)(tU32 anMsg, const Var& avarA, const Var& avarB);

template <typename T>
struct sMessageHandler : public ni::cIUnknownImpl<iMessageHandler> {
  const tU64 _threadId;
  const T _lambda;

  sMessageHandler(const T& aLambda, const tU64 aThreadId = ni::ThreadGetCurrentThreadID())
      : _threadId(aThreadId), _lambda(aLambda) {}

  virtual tU64 __stdcall GetThreadID() const {
    return _threadId;
  }
  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& avarA, const Var& avarB) {
    _lambda(anMsg,niVarCast(avarA),niVarCast(avarB));
  }
};

template <typename T>
Nonnull<iMessageHandler> MessageHandler(const T& aLambda, const tU64 anThreadID = ni::ThreadGetCurrentThreadID()) {
  return ni::MakeNonnull<sMessageHandler<T> >(aLambda, anThreadID);
}

inline tBool IOExecute(iRunnable* apRunnable) {
  return ni::GetConcurrent()->GetExecutorIO()->Execute(apRunnable);
}

inline tBool CPUExecute(iRunnable* apRunnable) {
  return ni::GetConcurrent()->GetExecutorCPU()->Execute(apRunnable);
}

inline tBool MainExecute(iRunnable* apRunnable) {
  return ni::GetConcurrent()->GetExecutorMain()->Execute(apRunnable);
}

inline Ptr<iFuture> IOSubmit(iRunnable* apRunnable) {
  return ni::GetConcurrent()->GetExecutorIO()->Submit(apRunnable);
}

inline Ptr<iFuture> CPUSubmit(iRunnable* apRunnable) {
  return ni::GetConcurrent()->GetExecutorCPU()->Submit(apRunnable);
}

inline Ptr<iFuture> MainSubmit(iRunnable* apRunnable) {
  return ni::GetConcurrent()->GetExecutorMain()->Submit(apRunnable);
}

#ifdef niCPP_Lambda

struct sConcurrentExecute {
  iExecutor* _executor;
  sConcurrentExecute(iExecutor* apExecutor) : _executor(apExecutor) {
  }
  template <typename TReleaseFun>
  inline tBool operator << (TReleaseFun&& afunRelease) {
    if (!_executor)
      return afunRelease();
    return _executor->Execute(ni::Runnable(afunRelease));
  }
};
#define niExec_(OBJ,...) ni::sConcurrentExecute(OBJ) << [__VA_ARGS__]() -> ni::tBool
#define niExec(KIND,...) ni::sConcurrentExecute(ni::GetConcurrent()->GetExecutor##KIND()) << [__VA_ARGS__]() -> ni::tBool

struct sConcurrentSubmit {
  iExecutor* _executor;
  sConcurrentSubmit(iExecutor* apExecutor) : _executor(apExecutor) {
  }
  template <typename TReleaseFun>
  inline Ptr<iFuture> operator << (TReleaseFun&& afunRelease) {
    if (!_executor) {
      Nonnull<iFutureValue> val { ni::GetConcurrent()->CreateFutureValue() };
      val->SetValue(afunRelease());
      return val;
    }
    return _executor->Submit(ni::Runnable(afunRelease));
  }
};
#define niSubmit_(OBJ,...) ni::sConcurrentSubmit(OBJ) << [__VA_ARGS__]() -> ni::tBool
#define niSubmit(KIND,...) ni::sConcurrentSubmit(ni::GetConcurrent()->GetExecutor##KIND()) << [__VA_ARGS__]() -> ni::tBool

struct sMessageHandlerBuilder {
  Ptr<iMessageHandler> _messageHandler;
  template <typename TReleaseFun>
  inline Ptr<iMessageHandler> operator << (TReleaseFun&& afunRelease) {
    _messageHandler = ni::MessageHandler(afunRelease);
    return _messageHandler;
  }
};

#define niMessageHandler(...) ni::sMessageHandlerBuilder() << [__VA_ARGS__](ni::tU32 anMsg, const ni::Var& avarA, const ni::Var& avarB)

#endif

#if !defined niNoThreads
typedef void (*tpfnConcurrentThreadCallback)(ni::tU64 threadID);
niExportFunc(void) ConcurrentSetThreadStartEndCallbacks(tpfnConcurrentThreadCallback apfnStart, tpfnConcurrentThreadCallback apfnEnd);
#endif

/**@}*/
/**@}*/
}
#endif // __CONCURRENTIMPL_H_EFDFC60D_4821_40B9_AE06_54171ACE7604__
