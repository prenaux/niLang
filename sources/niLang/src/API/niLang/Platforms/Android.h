#ifndef __ANDROID_H_EB9625B7_E285_431C_9DEE_BFA03F3B7AC2__
#define __ANDROID_H_EB9625B7_E285_431C_9DEE_BFA03F3B7AC2__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if defined ANDROID

#include <string.h>

#define niPlatformDetected
#ifdef niPragmaPrintPlatformDetected
#pragma message("=== Detected ANDROID")
#endif

#define niLinux       // OS
#define niUnix       1    // OS
#define niAndroid  1    // OS
#ifndef niPosix
#define niPosix 1          // OS-Lib
#endif
#ifdef __CLANG__
#define niCLang   1     // Compiler
#else
#define niGCC   1     // Compiler
#endif
#define niLittleEndian 1  // Endianess

#define niNoProcess
#define niCPP11

#ifndef niEmbedded
#define niEmbedded         // This is an 'embedded' platform
#endif

#if defined _UNICODE && !defined UNICODE
#define UNICODE 1
#endif

#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64 1
#endif

#ifdef __cplusplus
#include <new>
#endif

#ifndef __EXCEPTIONS
#ifndef niNoExceptions
#define niNoExceptions
#endif
#else
#ifdef niNoExceptions
#error "niNoExceptions defines although exceptions are enabled"
#endif
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
#define niPacked(x)   __attribute__((packed))

//////////////////////////////////////////////////////////////////////////////////////////////
#define niDLLSuffix   ".so"

//////////////////////////////////////////////////////////////////////////////////////////////
// Types definition
#define niTypeCChar   char
#define niTypeUChar   wchar_t
#define niTypeI8      signed char
#define niTypeI16     signed short
#define niTypeI64     signed long long
#define niTypeU8      unsigned char
#define niTypeU16     unsigned short
#define niTypeU64     unsigned long long
#define niTypeF32     float
#define niTypeF64     double
#define niTypePtr     niTypeU8*
#define niTypeSize    size_t
#define niTypeInt     int
#define niTypeUInt    unsigned int
#define niTypeIntPtr  intptr_t
#define niTypeUIntPtr uintptr_t

#if defined ni64
#define niTypeI32     int32_t
#define niTypeU32     uint32_t
#define niTypeOffset  niTypeI64
#define niTypeIntPtrIsOtherType
#else
#define niTypeI32     signed long
#define niTypeU32     unsigned long
#define niTypeOffset  niTypeI32
#define niTypeIntPtrIsOtherType
#endif

#define niTypeIntSize 4

#define niPragmaMinTypeInfoOn
#define niPragmaMinTypeInfoOff

#define niUCharSize    4
// #define niUCharIsType // Uchar is not a type...

#undef _A

//////////////////////////////////////////////////////////////////////////////////////////////

// Atomics, implemented inline here because these have been removed from
// platform-21 onward.

/* Note: atomic operations that were exported by the C library didn't
 *       provide any memory barriers, which created potential issues on
 *       multi-core devices. We now define them as inlined calls to
 *       GCC sync builtins, which always provide a full barrier.
 *
 *       NOTE: The C library still exports atomic functions by the same
 *              name to ensure ABI stability for existing NDK machine code.
 *
 *       If you are an NDK developer, we encourage you to rebuild your
 *       unmodified sources against this header as soon as possible.
 */
#define NI_ADR_ATOMIC_INLINE static __inline__ __attribute__((always_inline))

NI_ADR_ATOMIC_INLINE int ni_adr_atomic_cmpxchg(int old_value, int new_value, volatile int* ptr)
{
    /* We must return 0 on success */
    return __sync_val_compare_and_swap(ptr, old_value, new_value) != old_value;
}

NI_ADR_ATOMIC_INLINE int ni_adr_atomic_swap(int new_value, volatile int *ptr)
{
    int old_value;
    do {
        old_value = *ptr;
    } while (__sync_val_compare_and_swap(ptr, old_value, new_value) != old_value);
    return old_value;
}

NI_ADR_ATOMIC_INLINE int ni_adr_atomic_dec(volatile int *ptr)
{
  return __sync_fetch_and_sub (ptr, 1);
}

NI_ADR_ATOMIC_INLINE int ni_adr_atomic_inc(volatile int *ptr)
{
  return __sync_fetch_and_add (ptr, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////
#include "Unix/UnixConfig.h"

#endif // __unix__

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __ANDROID_H_EB9625B7_E285_431C_9DEE_BFA03F3B7AC2__
