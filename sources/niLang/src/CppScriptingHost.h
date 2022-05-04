#pragma once
#ifndef __CPPSCRIPTINGHOST_H_0F1C96EA_AF1D_C848_826E_8BBE4A851275__
#define __CPPSCRIPTINGHOST_H_0F1C96EA_AF1D_C848_826E_8BBE4A851275__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/Types.h"

#if !defined niWindows && !defined niOSX

#define niNoRTCpp

#else

#include "API/niLang/Utils/RTCppImpl.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/ITime.h"

#define RTCPP_HAMEXE "ham-rtcpp"
#define RTCPP_COMPILE_PROPERTY "rtcpp.compile"

struct sRTCppModuleCache {
  ni::tIntPtr hDLL = 0;
  ni::cString name;
  ni::cString path;
  ni::Ptr<ni::iTime> date;
};

typedef astl::map<ni::cString,sRTCppModuleCache> tRTCppModuleMap;

struct sRTCppStats {
  ni::tU32 _numCompiled = 0;
  ni::tU32 _numOutOfDate = 0;
  ni::tU32 _numUpToDate = 0;
};

niExportFunc(ni::tBool) RTCpp_GetCompileEnabled();
niExportFuncCPP(ni::cString) RTCpp_GetCompileModuleType();
niExportFunc(sRTCppStats*) RTCpp_GetStats();
niExportFunc(void) RTCpp_CleanupDLLs();
niExportFunc(ni::iScriptingHost*) RTCpp_CreateScriptingHost();

#endif

#endif // __CPPSCRIPTINGHOST_H_0F1C96EA_AF1D_C848_826E_8BBE4A851275__
