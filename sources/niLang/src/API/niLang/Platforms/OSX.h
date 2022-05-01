#ifndef __OSX_0983457873465_H__
#define __OSX_0983457873465_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if defined(__APPLE_CC__) && defined(__APPLE__) && defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && !defined __IOS__ && !defined __IPHONE_OS_VERSION_MIN_REQUIRED

#include <string.h>
#include <stdint.h>
#ifdef __cplusplus
#include <new>
#endif

#define niCPP11
#define niCPP_Lambda

#define niOverride override

#define niPlatformDetected
#ifdef niPragmaPrintPlatformDetected
#pragma message("=== Detected OSX")
#endif

#define niOSX		    // OS
#define niUnix	 1		// OS
#ifndef niPosix
#  define niPosix 1        // OS-Lib
#endif
#ifdef __clang__
#define niCLang 1
#else
#define niGCC   1
#endif

#ifndef __USE_LARGEFILE64
#  define __USE_LARGEFILE64 1
#endif

#ifndef _MAX_PATH
#  include <limits.h>
#  define _MAX_PATH	(PATH_MAX-1)
#endif

// Compiler support #pragma comment(lib,"libfile.lib")
#define niPragmaCommentLib

#define niAssume(EXPR) //__assume(EXPR)

#define niDLLSuffix ".dylib"

#define _vsnprintf  vsnprintf
#define _vsnwprintf vswprintf

// Definition types
#define inline  __inline
#define __stdcall
#define __cdecl

// Typename
#define niTypename  typename

// Compiler does not support #pragma comment(lib,"libfile.lib")
//#define niPragmaCommentLib

#define niBaseInterfacePadding  // no padding for GCC

//////////////////////////////////////////////////////////////////////////////////////////////
// Structures packing
#define niPragmaPack 0
#define niPackPush(x)
#define niPackPop()
#define niPacked(x) __attribute__((packed))

//////////////////////////////////////////////////////////////////////////////////////////////
// Types definition
#define niTypeCChar char
#define niTypeUChar wchar_t
#define niTypeI8    int8_t
#define niTypeI16   int16_t
#define niTypeI32   int32_t
#define niTypeI64   int64_t
#define niTypeU8    uint8_t
#define niTypeU16   uint16_t
#define niTypeU32   uint32_t
#define niTypeU64   uint64_t
#define niTypeF32   float
#define niTypeF64   double
#define niTypePtr   niTypeU8*
#define niTypeSize  size_t
#define niTypeInt   int
#define niTypeUInt  unsigned int
#define niTypeIntPtr  intptr_t
#define niTypeUIntPtr uintptr_t
#define niTypeOffset  ssize_t

#define niTypeIntSize  4
#define niUCharSize    4

#define niTypeIntPtrIsOtherType

#define niPragmaMinTypeInfoOn
#define niPragmaMinTypeInfoOff

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wstrict-prototypes"

//////////////////////////////////////////////////////////////////////////////////////////////
// Main - Entry point
#define niConsoleMain()                         \
  int main(int argc, const char** argv)

#define niWindowedMain()                            \
  int main(int argc, const char** argv)

#endif // OSX

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __OSX_0983457873465_H__
