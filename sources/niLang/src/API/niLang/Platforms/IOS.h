#pragma once
#ifndef __IOS_H_437DE32C_09C2_40C5_AD83_5E275AEB67AC__
#define __IOS_H_437DE32C_09C2_40C5_AD83_5E275AEB67AC__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if defined TARGET_OS_IPHONE || defined TARGET_IPHONE_SIMULATOR || defined __IOS__

#define niPlatformDetected

#ifndef niLib
#define niLib
#endif

#define niIOS
#define niUnix  1
#ifndef niPosix
#define niPosix 1
#endif
#ifdef __clang__
#define niCLang 1
#else
#define niGCC   1
#endif
#ifdef IOSMAC
#define niIOSMac
#endif

#define niNoProcess

#define niCPP11

#ifndef niEmbedded
#define niEmbedded
#endif

#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64 1
#endif

#include <stdint.h>
#include <string.h>
#ifdef __cplusplus
#include <new>
#endif

#define _vsnprintf  vsnprintf
#define _vsnwprintf vswprintf

#ifndef _MAX_PATH
#define _MAX_PATH PATH_MAX
#endif

// Definition types
#define inline  __inline
#define __stdcall
#define __cdecl

// Typename
#define niTypename  typename

// Compiler support #pragma comment(lib,"libfile.lib")
//#define niPragmaCommentLib

#define niBaseInterfacePadding  // no padding for GCC

//////////////////////////////////////////////////////////////////////////////////////////////
// Structures packing
#define niPragmaPack 0
#define niPackPush(x)
#define niPackPop()
#define niPacked(x)	__attribute__((packed))

//////////////////////////////////////////////////////////////////////////////////////////////
#define niDLLSuffix        ".dylib"

//////////////////////////////////////////////////////////////////////////////////////////////
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
#include "Unix/UnixConfig.h"

//////////////////////////////////////////////////////////////////////////////////////////////
#define niConsoleMain()                         \
  int main(int argc, const char** argv)
#define niWindowedMain()                        \
  int main(int argc, const char** argv)

#endif

#endif // __IOS_H_437DE32C_09C2_40C5_AD83_5E275AEB67AC__
