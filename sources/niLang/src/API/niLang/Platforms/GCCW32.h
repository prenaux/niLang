#ifndef __GCCW32_H_14585117_6F68_4C74_BD6A_C24BF22A5F1E__
#define __GCCW32_H_14585117_6F68_4C74_BD6A_C24BF22A5F1E__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifdef __MINGW32__

#define niPlatformDetected

#if defined niPragmaPrintPlatformDetected || defined _COMPILER_MSVC_10_X86
#pragma message("=== Detected MingW")
#endif

#define niWindows   // OS
#define niGCC     // Compiler
#define niMingW32   // Compiler, MingW32 (tested with GCC 3.4.5)

#if defined _UNICODE && !defined UNICODE
#define UNICODE
#endif

#ifndef _WIN32_DCOM
#define _WIN32_DCOM
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#ifndef _WIN32_IE
#define _WIN32_IE   0x0501
#endif

#define __STDC_LIMIT_MACROS // For WCHAR_MIN and WCHAR_MAX in stdint.
#define __STDC_CONSTANT_MACROS  // For UINT??_C macros to avoid using L and UL suffixes on constants.

// Required headers
#include <string.h>
#ifdef __cplusplus
#include <new>  // placement new
#endif

// Typename
#define niTypename  typename

// Compiler support #pragma comment(lib,"libfile.lib")
//#define niPragmaCommentLib

#define niBaseInterfacePadding

//////////////////////////////////////////////////////////////////////////////////////////////
// Structures packing
#define niPragmaPack 0
#define niPackPush(x)
#define niPackPop()
#define niPacked(x) __attribute__((packed))

//////////////////////////////////////////////////////////////////////////////////////////////
#define niDLLSuffix        ".dll"

//////////////////////////////////////////////////////////////////////////////////////////////
// Types definition
#define niTypeCChar char
#define niTypeUChar wchar_t
#define niTypeI8    signed char
#define niTypeI16   signed short
#define niTypeI32   signed long
#define niTypeI64   signed long long
#define niTypeU8    unsigned char
#define niTypeU16   unsigned short
#define niTypeU32   unsigned long
#define niTypeU64   unsigned long long
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

//////////////////////////////////////////////////////////////////////////////////////////////
// Main - Entry point
#define niWindowedMain() int APIENTRY WinMain(HINSTANCE ahInstance, HINSTANCE, LPTSTR, int)
#define niConsoleMain() int main(int argc, const char** argv)

#endif // __MINGW32__

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __GCCW32_H_14585117_6F68_4C74_BD6A_C24BF22A5F1E__
