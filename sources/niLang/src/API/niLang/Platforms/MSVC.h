#ifndef __MSVC_30530540_H__
#define __MSVC_30530540_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifdef _MSC_VER

#include <string.h>
#ifdef __cplusplus
#include <new.h>
#endif

#undef niWin32API
#define niWin32API(FUNC)  ::FUNC##W

// MSVC++ 12.0  _MSC_VER = 1800
// MSVC++ 11.0  _MSC_VER = 1700
// MSVC++ 10.0  _MSC_VER = 1600
// MSVC++ 9.0  _MSC_VER = 1500
// MSVC++ 8.0  _MSC_VER = 1400
// MSVC++ 7.1  _MSC_VER = 1310
// MSVC++ 7.0  _MSC_VER = 1300
// MSVC++ 6.0  _MSC_VER = 1200
// MSVC++ 5.0  _MSC_VER = 1100

#if _MSC_VER >= 1700
#define niCPP11
#endif

#if _MSC_VER >= 1600
#define niCPP_Lambda
#define niOverride override
#endif

#define niPlatformDetected
#ifdef niPragmaPrintPlatformDetected
#pragma message("=== Detected MSVC")
#endif

#ifdef __JSCC__
#define niJSCC 1 // OS
#else
#define niWindows 1 // OS
#define niWin32
#define niWinDesktop
#endif
#define niMSVC // Compiler

#ifdef __INTEL_COMPILER
#define niIntelCompiler
#endif

#ifdef _MSC_FULL_VER
#define niMSVCProcPack
#endif

#if defined _UNICODE && !defined UNICODE
#define UNICODE
#endif

#if _MSC_VER < 1310
// Disables the annoying "Truncated to 255 characters" warning
#pragma warning (disable : 4786)
// Workaround a bug in the VC6 compiler.
// See http://lists.boost.org/MailArchives/boost/msg25941.php
namespace { __forceinline void dummyReallyDisable4786() {} }
#endif

#ifndef __cplusplus
#define inline __inline
#endif

// Typename
#if _MSC_VER >= 1310
#define niTypename  typename
#endif

// Compiler support #pragma comment(lib,"libfile.lib")
#define niPragmaCommentLib

#define niBaseInterfacePadding virtual void __stdcall __dummy__() {}

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
#define niTypeI32   signed int
#define niTypeI64   signed __int64
#define niTypeU8    unsigned char
#define niTypeU16   unsigned short
#define niTypeU32   unsigned int
#define niTypeU64   unsigned __int64
#define niTypeF32   float
#define niTypeF64   double
#define niTypePtr   niTypeU8*
#define niTypeSize  size_t
#define niTypeInt   int
#define niTypeUInt  unsigned int

#define niTypeIntSize 4
#define niTypeLongIsOtherType

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

//////////////////////////////////////////////////////////////////////////////////////////////
// Forward declaration of HINSTANCE so that we don't have to infect our code with windows.h
struct HINSTANCE__;
typedef struct HINSTANCE__* HINSTANCE;

#define niWindowedMain()                                  \
  niExportFunc(void) _niWinMainStartup();                 \
  niExportFunc(void) _niWinMainShutdown();                \
  struct sNiWinMainInitializer {                          \
    sNiWinMainInitializer() { _niWinMainStartup(); }      \
    ~sNiWinMainInitializer() { _niWinMainShutdown(); }    \
  } __niWinMainStart;                                     \
  int __stdcall WinMain(HINSTANCE, HINSTANCE, char*, int)

#define niConsoleMain() int main(int argc, const ni::cchar** argv)

#endif // _MSC_VER

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __MSVC_30530540_H__
