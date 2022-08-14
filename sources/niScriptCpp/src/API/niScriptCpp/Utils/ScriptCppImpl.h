#ifndef __SCRIPTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
#define __SCRIPTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/Types.h>
#include <niLang/ILang.h>
#include <niLang/Utils/SmartPtr.h>
#include <niLang/Utils/QPtr.h>

/** \addtogroup niScriptCpp
 * @{
 */

//--------------------------------------------------------------------------------------------
//
// Windows implementation
//
//--------------------------------------------------------------------------------------------
#if defined niWindows && !defined niScriptCppNoGuard

#include <niLang/StringDef.h>
#include <niLang/Platforms/Win32/Win32_Redef.h>

namespace ni {

struct sScriptCppGuard
{
  const char* mGuardName;
  int     mExceptionCode;
  tIntPtr mExceptionAddr;
  bool mbHintAllowDebug;
  int  mAlwaysIgnore;

  enum eExceptionState {
    eExceptionState_PASS,
    eExceptionState_CATCH,
  };
  eExceptionState mExceptionState;

  sScriptCppGuard()
      : mGuardName("Unknown")
      , mExceptionState(eExceptionState_PASS)
      , mbHintAllowDebug(true)
      , mAlwaysIgnore(0)
  {
  }

  int RuntimeExceptionFilter() {
    if (!AmBeingDebugged()) {
      // if there's no debugger, we simply continue operating. TODO:
      // Should implement a method to ensure this can be disabled so
      // process crashes on end user machines
      return EXCEPTION_EXECUTE_HANDLER;
    }

    int result = 0;

    switch (mExceptionState) {
      case eExceptionState_PASS:
        // Let pass to debugger once, then catch it
        result = EXCEPTION_CONTINUE_SEARCH;
        mExceptionState = eExceptionState_CATCH;
        break;
      case eExceptionState_CATCH:
        // Catch it now. Reset to catch in debugger again next time.
        result = EXCEPTION_EXECUTE_HANDLER;
        mExceptionState = eExceptionState_PASS;
        break;
      default:;
        niAssertUnreachable("Unknown exception state.");
    }

    return result;
  }

  bool AmBeingDebugged()
  {
    if (::IsDebuggerPresent()) {
      return true;
    }

    BOOL bRDebugPresent = FALSE;
    CheckRemoteDebuggerPresent(GetModuleHandle(NULL), &bRDebugPresent);
    if (FALSE == bRDebugPresent) {
      return false;
    }
    else {
      return true;
    }
  }

  int ExceptionFilter(void * nativeExceptionInfo, sScriptCppGuard* apGuard)
  {
    EXCEPTION_RECORD *pRecord = ((LPEXCEPTION_POINTERS) nativeExceptionInfo)->ExceptionRecord;
    int nCode = pRecord->ExceptionCode;
    apGuard->mExceptionCode = nCode;
    apGuard->mExceptionAddr = 0;

    if (nCode == EXCEPTION_ACCESS_VIOLATION) {
      int flavour = pRecord->ExceptionInformation[0];
      switch (flavour) {
        case 0:
          apGuard->mExceptionAddr = (tIntPtr)pRecord->ExceptionInformation[1];
          break;
        case 1:
          apGuard->mExceptionAddr = (tIntPtr)pRecord->ExceptionInformation[1];
          break;
        default:
          break;
      }
    }
    else if (nCode == EXCEPTION_ILLEGAL_INSTRUCTION) {
      apGuard->mExceptionAddr = (tIntPtr)pRecord->ExceptionAddress;
    }

    if (!apGuard->mbHintAllowDebug) {
      // We don't want debugging to catch this
      return EXCEPTION_EXECUTE_HANDLER;
    }

    // Otherwise fall back
    return RuntimeExceptionFilter();
  }

  cString ToExceptionString() {
    cString o;
    o << "\nGuard: " << mGuardName;
    o << "\nException {\n  code = ";
    switch (mExceptionCode) {
      case EXCEPTION_ACCESS_VIOLATION: o << "ACCESS_VIOLATION"; break;
      case EXCEPTION_DATATYPE_MISALIGNMENT: o << "DATATYPE_MISALIGNMENT"; break;
      case EXCEPTION_BREAKPOINT: o << "BREAKPOINT"; break;
      case EXCEPTION_SINGLE_STEP: o << "SINGLE_STEP"; break;
      case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: o << "ARRAY_BOUNDS_EXCEEDED"; break;
      case EXCEPTION_FLT_DENORMAL_OPERAND: o << "FLOAT_DENORMAL_OPERAND"; break;
      case EXCEPTION_FLT_DIVIDE_BY_ZERO: o << "FLOAT_DIVIDE_BY_ZERO"; break;
      case EXCEPTION_FLT_INEXACT_RESULT: o << "FLOAT_INEXACT_RESULT"; break;
      case EXCEPTION_FLT_INVALID_OPERATION: o << "FLOAT_INVALID_OPERATION"; break;
      case EXCEPTION_FLT_OVERFLOW: o << "FLOAT_OVERFLOW"; break;
      case EXCEPTION_FLT_STACK_CHECK: o << "FLOAT_STACK_CHECK"; break;
      case EXCEPTION_FLT_UNDERFLOW: o << "FLOAT_UNDERFLOW"; break;
      case EXCEPTION_INT_DIVIDE_BY_ZERO: o << "INTEGER_DIVIDE_BY_ZERO"; break;
      case EXCEPTION_INT_OVERFLOW: o << "INTEGER_OVERFLOW"; break;
      case EXCEPTION_PRIV_INSTRUCTION: o << "PRIVILEGED_INSTRUCTION"; break;
      case EXCEPTION_IN_PAGE_ERROR: o << "IN_PAGE_ERROR"; break;
      case EXCEPTION_ILLEGAL_INSTRUCTION: o << "ILLEGAL_INSTRUCTION"; break;
      case EXCEPTION_NONCONTINUABLE_EXCEPTION: o << "NONCONTINUABLE_EXCEPTION"; break;
      case EXCEPTION_STACK_OVERFLOW: o << "STACK_OVERFLOW"; break;
      case EXCEPTION_INVALID_DISPOSITION: o << "INVALID_DISPOSITION"; break;
      case EXCEPTION_GUARD_PAGE: o << "GUARD_PAGE_VIOLATION"; break;
      case EXCEPTION_INVALID_HANDLE: o << "INVALID_HANDLE"; break;
    }
    o << niFmt(",\n  addr = %p (%d)\n}",mExceptionAddr,mExceptionAddr);
    return o;
  }
};

}

#define SCRIPTCPP_DECLARE_GUARD(NAME) ni::sScriptCppGuard mProtector##NAME

#define SCRIPTCPP_ENTER_GUARD(RETTYPE,NAME)         \
  RETTYPE __rtRet = (RETTYPE)0;                 \
  ni::sScriptCppGuard& __rtguard = mProtector##NAME; \
  if (!__rtguard.mAlwaysIgnore) {               \
  __rtguard.mGuardName = #NAME;                 \
  __try {                                       \
  auto __rtimpl = [&] () -> RETTYPE

#define SCRIPTCPP_LEAVE_GUARD()                                             \
  ; __rtRet = __rtimpl();                                               \
  } __except(__rtguard.ExceptionFilter(GetExceptionInformation(),&__rtguard)) { \
    if (ni_debug_assert(0, "RTCpp Exception", __LINE__, __FILE__,       \
                        &__rtguard.mAlwaysIgnore, __rtguard.ToExceptionString().Chars())) \
    { ni_debug_break(); }                                               \
    }                                                                   \
  }                                                                     \
                                                                        return __rtRet;


#define SCRIPTCPP_ENTER_VOID_GUARD(NAME)            \
  ni::sScriptCppGuard& __rtguard = mProtector##NAME; \
  if (!__rtguard.mAlwaysIgnore) {               \
  __rtguard.mGuardName = #NAME;                 \
  __try {                                       \
  auto __rtimpl = [&] () -> void

#define SCRIPTCPP_LEAVE_VOID_GUARD()                                        \
  ; __rtimpl();                                                         \
  } __except(__rtguard.ExceptionFilter(GetExceptionInformation(),&__rtguard)) { \
    if (ni_debug_assert(0, "RTCpp Exception", __LINE__, __FILE__,       \
                        &__rtguard.mAlwaysIgnore, __rtguard.ToExceptionString().Chars())) \
    { ni_debug_break(); }                                               \
    }                                                                   \
  }

#endif

//--------------------------------------------------------------------------------------------
//
// General macros
//
//--------------------------------------------------------------------------------------------

#ifndef SCRIPTCPP_DECLARE_GUARD
#define SCRIPTCPP_DECLARE_GUARD(NAME)
#endif

#ifndef SCRIPTCPP_ENTER_GUARD
#define SCRIPTCPP_ENTER_GUARD(RETTYPE,NAME) {
#endif

#ifndef SCRIPTCPP_LEAVE_GUARD
#define SCRIPTCPP_LEAVE_GUARD() }
#endif

#ifndef SCRIPTCPP_ENTER_VOID_GUARD
#define SCRIPTCPP_ENTER_VOID_GUARD(NAME) {
#endif

#ifndef SCRIPTCPP_LEAVE_VOID_GUARD
#define SCRIPTCPP_LEAVE_VOID_GUARD() }
#endif

#define SCRIPTCPP_METHOD_IMPL(RETTYPE,NAME,PARAMS)  \
  SCRIPTCPP_DECLARE_GUARD(NAME);                    \
  RETTYPE __stdcall NAME PARAMS niImpl {        \
  SCRIPTCPP_ENTER_GUARD(RETTYPE,NAME)

#define SCRIPTCPP_METHOD_END() SCRIPTCPP_LEAVE_GUARD(); }

#define SCRIPTCPP_VOID_METHOD_IMPL(NAME,PARAMS)     \
  SCRIPTCPP_DECLARE_GUARD(NAME);                    \
  void __stdcall NAME PARAMS niImpl {           \
  SCRIPTCPP_ENTER_VOID_GUARD(NAME)

#define SCRIPTCPP_VOID_METHOD_END() SCRIPTCPP_LEAVE_VOID_GUARD(); }

#define SCRIPTCPP_EXPORT_CLASS(CATEGORY,NAME,CLASS)     \
  niExportFunc(ni::iUnknown*) New_##CATEGORY##_##NAME(  \
    const ni::Var&,const ni::Var&)                      \
  {                                                     \
    return niNew CLASS();                               \
  }

#define SCRIPTCPP_EXPORT_FUNC(CATEGORY,NAME,CONSTRUCTOR)               \
  niExportFunc(ni::iUnknown*) New_##CATEGORY##_##NAME(                 \
    const ni::Var&,const ni::Var&)                                      \
  {                                                                     \
    ni::Ptr<ni::iUnknown> scriptCppObjectInstance = CONSTRUCTOR().ptr(); \
    niCheck(scriptCppObjectInstance.IsOK(),NULL);                       \
    return scriptCppObjectInstance.GetRawAndSetNull();                  \
  }

#define SCRIPTCPP_EXPORT(CATEGORY,CLASS)  \
  SCRIPTCPP_EXPORT_CLASS(CATEGORY,CLASS,CLASS)

/**@}*/

#endif // __SCRIPTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
