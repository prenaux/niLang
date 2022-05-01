#ifndef __RTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
#define __RTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "../Types.h"
#include "../ILang.h"
#include "../Utils/SmartPtr.h"
#include "../Utils/QPtr.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_RTCpp
 * @{
 */

//--------------------------------------------------------------------------------------------
//
// Windows implementation
//
//--------------------------------------------------------------------------------------------
#if defined niWindows && !defined niRTCppNoGuard

#include "../StringDef.h"
#include "../Platforms/Win32/Win32_Redef.h"

namespace ni {

struct RTCppGuard
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

  RTCppGuard()
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

  int ExceptionFilter(void * nativeExceptionInfo, RTCppGuard* pRTCppGuard)
  {
    EXCEPTION_RECORD *pRecord = ((LPEXCEPTION_POINTERS) nativeExceptionInfo)->ExceptionRecord;
    int nCode = pRecord->ExceptionCode;
    pRTCppGuard->mExceptionCode = nCode;
    pRTCppGuard->mExceptionAddr = 0;

    if (nCode == EXCEPTION_ACCESS_VIOLATION) {
      int flavour = pRecord->ExceptionInformation[0];
      switch (flavour) {
        case 0:
          pRTCppGuard->mExceptionAddr = (tIntPtr)pRecord->ExceptionInformation[1];
          break;
        case 1:
          pRTCppGuard->mExceptionAddr = (tIntPtr)pRecord->ExceptionInformation[1];
          break;
        default:
          break;
      }
    }
    else if (nCode == EXCEPTION_ILLEGAL_INSTRUCTION) {
      pRTCppGuard->mExceptionAddr = (tIntPtr)pRecord->ExceptionAddress;
    }

    if (!pRTCppGuard->mbHintAllowDebug) {
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

#define RTCPP_DECLARE_GUARD(NAME) ni::RTCppGuard mProtector##NAME

#define RTCPP_ENTER_GUARD(RETTYPE,NAME)         \
  RETTYPE __rtRet = (RETTYPE)0;                 \
  ni::RTCppGuard& __rtguard = mProtector##NAME; \
  if (!__rtguard.mAlwaysIgnore) {               \
  __rtguard.mGuardName = #NAME;                 \
  __try {                                       \
  auto __rtimpl = [&] () -> RETTYPE

#define RTCPP_LEAVE_GUARD()                                             \
  ; __rtRet = __rtimpl();                                               \
  } __except(__rtguard.ExceptionFilter(GetExceptionInformation(),&__rtguard)) { \
    if (ni_debug_assert(0, "RTCpp Exception", __LINE__, __FILE__,       \
                        &__rtguard.mAlwaysIgnore, __rtguard.ToExceptionString().Chars())) \
    { ni_debug_break(); }                                               \
    }                                                                   \
  }                                                                     \
                                                                        return __rtRet;


#define RTCPP_ENTER_VOID_GUARD(NAME)            \
  ni::RTCppGuard& __rtguard = mProtector##NAME; \
  if (!__rtguard.mAlwaysIgnore) {               \
  __rtguard.mGuardName = #NAME;                 \
  __try {                                       \
  auto __rtimpl = [&] () -> void

#define RTCPP_LEAVE_VOID_GUARD()                                        \
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

#ifndef RTCPP_DECLARE_GUARD
#define RTCPP_DECLARE_GUARD(NAME)
#endif

#ifndef RTCPP_ENTER_GUARD
#define RTCPP_ENTER_GUARD(RETTYPE,NAME) {
#endif

#ifndef RTCPP_LEAVE_GUARD
#define RTCPP_LEAVE_GUARD() }
#endif

#ifndef RTCPP_ENTER_VOID_GUARD
#define RTCPP_ENTER_VOID_GUARD(NAME) {
#endif

#ifndef RTCPP_LEAVE_VOID_GUARD
#define RTCPP_LEAVE_VOID_GUARD() }
#endif

#define RTCPP_METHOD_IMPL(RETTYPE,NAME,PARAMS)  \
  RTCPP_DECLARE_GUARD(NAME);                    \
  RETTYPE __stdcall NAME PARAMS niImpl {        \
  RTCPP_ENTER_GUARD(RETTYPE,NAME)

#define RTCPP_METHOD_END() RTCPP_LEAVE_GUARD(); }

#define RTCPP_VOID_METHOD_IMPL(NAME,PARAMS)     \
  RTCPP_DECLARE_GUARD(NAME);                    \
  void __stdcall NAME PARAMS niImpl {           \
  RTCPP_ENTER_VOID_GUARD(NAME)

#define RTCPP_VOID_METHOD_END() RTCPP_LEAVE_VOID_GUARD(); }

#define RTCPP_EXPORT(CATEGORY,CLASS)                        \
  niExportFunc(ni::iUnknown*) New_##CATEGORY##_##CLASS() {  \
    return niNew CLASS();                                   \
  }

#define RTCPP_IMPORT(CATEGORY,CLASS)                      \
  niExportFunc(ni::iUnknown*) New_##CATEGORY##_##CLASS();

#define RTCPP_NEW(CATEGORY,CLASS) New_##CATEGORY##_##CLASS()

/**@}*/
/**@}*/

#endif // __RTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
