#ifndef __WINRT_H_86CC04B9_A61C_4967_B623_7204B8E43D15__
#define __WINRT_H_86CC04B9_A61C_4967_B623_7204B8E43D15__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if defined _MSC_VER && defined WINAPI_FAMILY

#include <string.h>
#ifdef __cplusplus
#include <new.h>
#endif

#undef niWin32API
#define niWin32API(FUNC)  ::FUNC##W

#define niCPP11

#define niPlatformDetected
#ifdef niPragmaPrintPlatformDetected
#pragma message("=== Detected MSVC")
#endif

#define niWindows 1 // OS
#define niWin32
#define niWinRT
#define niMSVC // Compiler

#define niNoThreads
#define niNoCrashReport
#define niNoProcess
#define niNoSocket

#if defined _UNICODE && !defined UNICODE
#define UNICODE
#endif

#ifndef __cplusplus
#define inline __inline
#endif

// Typename
#define niTypename  typename

// Compiler support #pragma comment(lib,"libfile.lib")
#define niPragmaCommentLib

#define niBaseInterfacePadding    void __stdcall __dummy__() {}

#define niAssume(EXPR) __assume(EXPR)

#define niDLLSuffix ".dll"

//////////////////////////////////////////////////////////////////////////////////////////////
// Structures packing
#define niPragmaPack 1
#define niPackPush(x) pack(push,x)
#define niPackPop()  pack(pop)
#define niPacked(x)

//////////////////////////////////////////////////////////////////////////////////////////////
// Types definition
#define niTypeCChar char
#define niTypeUChar wchar_t
#define niTypeI8    signed char
#define niTypeI16   signed short
#define niTypeI32   signed long
#define niTypeI64   signed __int64
#define niTypeU8    unsigned char
#define niTypeU16   unsigned short
#define niTypeU32   unsigned long
#define niTypeU64   unsigned __int64
#define niTypeF32   float
#define niTypeF64   double
#define niTypePtr   niTypeU8*
#define niTypeSize    size_t
#define niTypeInt     int
#define niTypeUInt      unsigned int
#ifdef ni64 // 64 bits target
#define niTypeIntPtr  niTypeI64
#define niTypeUIntPtr niTypeU64
#define niTypeOffset  niTypeI64
#else
#define niTypeIntPtr  niTypeI32
#define niTypeUIntPtr niTypeU32
#define niTypeOffset  niTypeI32
#endif

#define niUCharSize    2

#define niPragmaMinTypeInfoOn   component(mintypeinfo, on)
#define niPragmaMinTypeInfoOff    component(mintypeinfo, off)

#endif // _MSC_VER

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WINRT_H_86CC04B9_A61C_4967_B623_7204B8E43D15__
