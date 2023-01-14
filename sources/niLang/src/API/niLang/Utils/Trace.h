#pragma once
#ifndef __TRACE_H_0103851F_7CEF_5448_B17A_30AE16EB8F64__
#define __TRACE_H_0103851F_7CEF_5448_B17A_30AE16EB8F64__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../ILang.h"
#include "../StringDef.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

niExportFuncCPP(const char*) GetTraceFile(const char* aFile);
niExportFuncCPP(ni::cString) GetTraceFunc(const char* aPrettyFunction);

#define niTraceFile ni::GetTraceFile(__FILE__)
#define niTraceLine __LINE__
#define niTraceFunc ni::GetTraceFunc(__PRETTY_FUNCTION__).c_str()
#define niTraceSourceLoc niTraceFile,niTraceLine,niTraceFunc

#define niTraceFmt(FMT) {                                               \
    ni::cString traceFmt;                                               \
    traceFmt.append("... ");                                            \
    traceFmt.append(niTraceFunc);                                       \
    traceFmt.append(": ");                                              \
    traceFmt.CatFormat FMT;                                             \
    ni::ni_log(ni::eLogFlags_Debug, traceFmt.Chars(), niTraceFile, niTraceLine, NULL); \
  }

#define niTraceFmtIf(COND,FMT) if (COND) { niTraceFmt(FMT); }

#define niModuleTraceName_(MODULE,TRACENAME) #MODULE "." #TRACENAME
#define niModuleTraceName(MODULE) niModuleTraceName_(MODULE,Trace)

#define niModuleTraceObject_(MODULE,TRACENAME) _trace_##MODULE##TRACENAME
#define niModuleTraceObject(MODULE) niModuleTraceObject_(MODULE,Trace)

#define niModuleShouldTrace_(MODULE,TRACENAME) niModuleTraceObject_(MODULE,TRACENAME).get()
#define niModuleShouldTrace(MODULE) niModuleTraceObject(MODULE).get()

#define niModuleTrace_(MODULE,TRACENAME,FMT) niTraceFmtIf(niModuleTraceObject_(MODULE,TRACENAME).get(), FMT)
#define niModuleTrace(MODULE,FMT) niModuleTrace_(MODULE,Trace,FMT)

#define niDeclareModuleProperty(MODULE,TRACENAME,DEFAULT) static ni::sPropertyBool niModuleTraceObject_(MODULE,TRACENAME)(niModuleTraceName_(MODULE,TRACENAME), DEFAULT)
#define niDeclareModuleTrace_(MODULE,TRACENAME) niDeclareModuleProperty(MODULE,TRACENAME,ni::eFalse)
#define niDeclareModuleTrace(MODULE) niDeclareModuleTrace_(MODULE,Trace)

/*!

 Example usage of module trace macros:
 \code
 niDeclareModuleTrace(myModule);
 #define MY_TRACE(FMT) niModuleTrace(myModule,FMT)
 // Enabled it with: -DmyModule.Trace=1
 \endcode

*/

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __TRACE_H_0103851F_7CEF_5448_B17A_30AE16EB8F64__
