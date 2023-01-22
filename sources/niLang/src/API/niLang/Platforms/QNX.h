// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#pragma once
#ifndef __QNX_H_AEF452DS649__
#define __QNX_H_AEF452DS649__
#if defined(__QNX__)


#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include <limits.h>
#include <alloca.h>
#include <sys/types.h>

#ifdef __cplusplus
#include <new>
#endif

#define niCPP11
#define niCPP_Lambda

#define niOverride override

#define niPlatformDetected
#ifdef niPragmaPrintPlatformDetected
#pragma message("=== Detected QNX")
#endif

#define niQNX       // OS
#define niUnix	 1		// OS
#ifndef niPosix
#  define niPosix 1   // OS-Lib
#endif
#ifdef __clang__
#define niCLang 1
#else
#define niGCC   1
#endif

#define niQNXDesktop 1

#ifndef __USE_LARGEFILE64
#  define __USE_LARGEFILE64 1
#endif

#ifndef PATH_MAX
#define PATH_MAX _POSIX_PATH_MAX
#endif

#ifndef NAME_MAX
#define NAME_MAX PATH_MAX
#endif

#define _MAX_PATH	(PATH_MAX-1)

// Compiler support #pragma comment(lib,"libfile.lib")
#define niPragmaCommentLib

#define niDLLSuffix ".so"

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

#ifdef ni64 // 64 bits target
#define niTypeIntSize 4
// #define niTypeIntIsOtherType
#define niTypeI32   signed int
#define niTypeU32   unsigned int
#define niTypeI64   long signed int
#define niTypeU64   long unsigned int
#define niTypeIntPtr  niTypeI64
#define niTypeUIntPtr niTypeU64
#define niTypeOffset  niTypeI64
#else
#define niTypeIntIsOtherType
#define niTypeI32   signed long
#define niTypeU32   unsigned long
#define niTypeI64   long long signed int
#define niTypeU64   long long unsigned int
#define niTypeIntPtr  niTypeI32
#define niTypeUIntPtr niTypeU32
#define niTypeOffset  niTypeI32
#endif

#define niUCharSize    4

#define niPragmaMinTypeInfoOn
#define niPragmaMinTypeInfoOff

#ifdef niCLang
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#endif

#define NI_QNX_ATOMIC_INLINE static __inline__ __attribute__((always_inline))
NI_QNX_ATOMIC_INLINE int ni_qnx_atomic_swap(int new_value, volatile int *ptr)
{
  int old_value;
  do {
    old_value = *ptr;
  } while (__sync_val_compare_and_swap(ptr, old_value, new_value) != old_value);
  return old_value;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Main - Entry point
#define niConsoleMain()                         \
  int main(int argc, const char** argv)

#define niWindowedMain()                        \
  int main(int argc, const char** argv)

#endif // QNX

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __QNX_H_AEF452DS649__
