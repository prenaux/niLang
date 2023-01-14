#ifndef __JSCC_H_3FE46B1D_B6D1_4592_A4F2_A0A281EF5A3B__
#define __JSCC_H_3FE46B1D_B6D1_4592_A4F2_A0A281EF5A3B__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifdef __JSCC__

#ifdef _MSC_VER
#error "MSVC.h should be included before"
#endif

#define niPlatformDetected
#ifdef niPragmaPrintPlatformDetected
#pragma message("=== Detected JSCC")
#endif

#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#ifdef __cplusplus
#include <new>
#endif

#define niJSCC  1   // OS
#define niCLang 1
#ifndef niUnix
#define niUnix 1    // OS
#endif
#ifndef niPosix
#define niPosix 1   // OS-Lib
#endif
#ifndef niEmbedded
#define niEmbedded
#endif

#define niNoThreads 1
#define niNoProcess 1
#define niNoSocket 1
#define niNoDLL 1

// JSCC can run niScript & use the interop but this is disabled to minimize
// the size of the compiled output.
#define niConfig_NoInterfaceDef
#define niConfig_NoConstDef
#define niConfig_NoXCALL

#define niCPP11
#define niCPP_Lambda

#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64 1
#endif

#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64 1
#endif

#if defined _UNICODE && !defined UNICODE
#define UNICODE 1
#endif

#ifndef PATH_MAX
#define PATH_MAX  4096
#endif

#ifndef _MAX_PATH
#define _MAX_PATH PATH_MAX
#endif

// #define inline  __inline
#define __stdcall
#define __cdecl

// Typename
#define niTypename  typename

#define niBaseInterfacePadding

#define niExportJSCC(TYPE) EMSCRIPTEN_KEEPALIVE niExportFunc(TYPE)

//////////////////////////////////////////////////////////////////////////////////////////////
// Structures packing
#define niPragmaPack 0
#define niPackPush(x)
#define niPackPop()
#define niPacked(x)   __attribute__((aligned(x))) __attribute__((packed))

//////////////////////////////////////////////////////////////////////////////////////////////
// Types definition
#define niTypeCChar char
#define niTypeUChar wchar_t
#define niTypeI8    signed char
#define niTypeI16   signed short
#define niTypeU8    unsigned char
#define niTypeU16   unsigned short
#define niTypeF32   float
#define niTypeF64   double
#define niTypePtr   niTypeU8*
#define niTypeSize    size_t
#define niTypeInt     int
#define niTypeUInt    unsigned int

#define niTypeIntIsOtherType
#define niTypeI32   signed long
#define niTypeU32   unsigned long
#define niTypeI64   long long signed int
#define niTypeU64   long long unsigned int
#define niTypeIntPtr  niTypeI32
#define niTypeUIntPtr niTypeU32
#define niTypeOffset  niTypeI32

#define niUCharSize    4

#define niPragmaMinTypeInfoOn
#define niPragmaMinTypeInfoOff

//////////////////////////////////////////////////////////////////////////////////////////////
#include "Unix/UnixConfig.h"

//////////////////////////////////////////////////////////////////////////////////////////////
#define niConsoleMain()                         \
  int main(int argc, const char** argv)
#define niWindowedMain()                        \
  int main(int argc, const char** argv)

#endif // __JSCC__

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __JSCC_H_3FE46B1D_B6D1_4592_A4F2_A0A281EF5A3B__
