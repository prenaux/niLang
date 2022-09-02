#pragma once
#ifndef __TYPES_H_C2ADBAF2_7B9D_4C93_BE52_AB1F88D2CB54__
#define __TYPES_H_C2ADBAF2_7B9D_4C93_BE52_AB1F88D2CB54__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#ifndef __METAL_VERSION__
#  include <stdlib.h>
#  include <math.h>
#endif

//--------------------------------------------------------------------------------------------
//
//  Platform
//
//--------------------------------------------------------------------------------------------
#if defined __APPLE__ && !defined __METAL_VERSION__
// For the TARGET_* macros
#  include "TargetConditionals.h"
#endif

#define niFeatures_Light    10
#define niFeatures_Embedded 20
#define niFeatures_Full     30

#ifdef _FEATURES
#  define niFeatures _FEATURES
#elif defined niEmbedded
#  define niFeatures niFeatures_Embedded
#else
#  define niFeatures niFeatures_Full
#endif

#define niMinFeatures(REQ)  (niFeatures >= REQ)

#ifndef niCAssert
#  ifdef __cplusplus
#    define niCAssert(exp)          typedef char __C_ASSERT__[(exp)?1:-1];
#  else
#    define niCAssert(exp)
#  endif
#endif

#ifdef _DEBUG
#  define niDebug
#else
#  define niRelease
#endif

#ifdef _DEBUG
#  define niBuildDebug
#else
#  define niBuildRelease
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// < Feature Set 15, no script engine or interop
#if !niMinFeatures(15)
#  define niConfig_NoInterfaceDef
#  define niConfig_NoConstDef
#  define niConfig_NoXCALL
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// CPU Architectures

//// Metal Shader Language //////////////////////////////////////////////////////////////////////
#if defined __METAL_VERSION__

#  define niCPUArch "metalsl"
#  define ni64           1
#  define niLittleEndian 1

#  define SYNC_INT_TYPE int
#  define SYNC_READ(addr) *addr
#  define SYNC_WRITE(addr, value) *addr = value
#  define SYNC_INCREMENT(addr) (++*addr)
#  define SYNC_DECREMENT(addr) (--*addr)

//// JSCC - Emulate a 32bit CPU ///////////////////////////////////////////////////////////////
#elif defined __JSCC__

#  include <malloc.h>

#  define niCPUArch "js"
#  define ni32           1
#  define niLittleEndian 1

#  define SYNC_INT_TYPE int
#  define SYNC_READ(addr) *addr
#  define SYNC_WRITE(addr, value) *addr = value
#  define SYNC_INCREMENT(addr) (++*addr)
#  define SYNC_DECREMENT(addr) (--*addr)

//// ARM /////////////////////////////////////////////////////////////////////////////////////
#elif (defined(__AARCH64EL__) && defined(__ARM_ARCH)) || \
  defined _M_ARM || defined __arm__ || defined _ARM || defined __TARGET_ARCH_ARM

#  if (defined(__AARCH64EL__) && defined(__ARM_ARCH)) && !defined __CPU_ARM64__
#    define __CPU_ARM64__
#  endif

#  ifdef __CPU_ARM64__
#    define ni64  1
#    define niARM64 1
#  else
#    define ni32  1
#    define niARM32 1
#  endif
#  define niARM 1
#  define niLittleEndian 1

#  if defined __thumb__ || defined __TARGET_ARCH_THUMB
#    define niARM_THUMB 1
#  endif
#  if defined __ARM_NEON__
#    define niARM_NEON  1
#  endif

#  if defined __CPU_ARM64__
#    define niCPUArch "arm64"
#  elif defined _M_ARM_FP
// Assume WinRT, ARMv7 VFPv3
#    define niCPUArch "armv7a"
#  elif defined __CPU_ARMv7N__
#    define niCPUArch "armv7n"
#    if niARM_NEON != 1
#      pragma message("E/C++ Preprocessor: Invalid ARMv7n definition, should have neon instruction set enable.")
#    endif
#  elif defined __CPU_ARMv7A__ || defined __ARM_ARCH_7__ || defined __ARM_ARCH_7A__ || defined __ARM_ARCH_7R__ || defined __ARM_ARCH_7M__ || defined __ARM_ARCH_7S__ || (defined __QNX__ || defined __ARMEL__)
#    define niCPUArch "armv7a"
#  elif defined __CPU_ARMv6__ || defined __ARM_ARCH_6__ || defined __ARM_ARCH_6J__ || defined __ARM_ARCH_6K__ || defined __ARM_ARCH_6Z__ || defined __ARM_ARCH_6ZK_
#    define niCPUArch "armv6"
#  elif defined __CPU_ARMv5__
#    define niCPUArch "armv5"
#  else
#    pragma message("E/C++ Preprocessor: Unknown ARM CPU variant.")
#  endif

// Atomic, ARM, Windows
#  ifdef _MSC_VER
#    include <intrin.h>
#    define SYNC_INT_TYPE long
#    define SYNC_READ(addr) *addr
#    define SYNC_WRITE(addr, value) *addr = value
#    define SYNC_INCREMENT(addr) _InterlockedIncrement(addr)
#    define SYNC_DECREMENT(addr) _InterlockedDecrement(addr)

// Atomic, ARM, iOS
#  elif defined TARGET_OS_IPHONE
#    include <libkern/OSAtomic.h>
#    define SYNC_INT_TYPE int32_t
#    define SYNC_INCREMENT(addr) (OSAtomicIncrement32(addr))
#    define SYNC_DECREMENT(addr) (OSAtomicDecrement32(addr))
#    define SYNC_WRITE(addr,v)   (*(addr)) = (v)
#    define SYNC_READ(addr)      (*addr)

// Atomic, ARM, Android
#  elif defined ANDROID
#    define SYNC_INT_TYPE int
#    define SYNC_INCREMENT(addr) (ni_adr_atomic_inc(addr)+1)
#    define SYNC_DECREMENT(addr) (ni_adr_atomic_dec(addr)-1)
#    define SYNC_WRITE(addr,v)   ni_adr_atomic_swap(v,addr)
#    define SYNC_READ(addr)      (*addr)

// Atomic, ARM, QNX
#  elif defined __QNX__
#    define SYNC_INT_TYPE int
#    define SYNC_INCREMENT(addr) __sync_add_and_fetch(addr,1)
#    define SYNC_DECREMENT(addr) __sync_sub_and_fetch(addr,1)
#    define SYNC_WRITE(addr,v)   ni_qnx_atomic_swap(v,addr)
#    define SYNC_READ(addr)      (*addr)

#  endif

//// AMD64 / EMT64 ////////////////////////////////////////////////////////////////////////////
#elif defined _M_X64 || defined __amd64__ || defined __amd64 || defined __x86_64__ || defined __x86_64

#  define niCPUArch "x64"
#  define ni64           1
#  define niLittleEndian 1
#  define niIntelX86     1
#  define niX64          1

// Atomic, X64, Windows
#  ifdef _MSC_VER
#    include <intrin.h>
#    define SYNC_INT_TYPE long
#    define SYNC_READ(addr) *addr
#    define SYNC_WRITE(addr, value) *addr = value
#    define SYNC_INCREMENT(addr) _InterlockedIncrement(addr)
#    define SYNC_DECREMENT(addr) _InterlockedDecrement(addr)

// Atomic, X64, OSX
#  elif defined(__APPLE__) && defined(__MACH__) // OSX
#    define SYNC_INT_TYPE signed int
#    include <libkern/OSAtomic.h>
niCAssert(sizeof(SYNC_INT_TYPE) == sizeof(int32_t));
static inline SYNC_INT_TYPE _InterlockedIncrement32(SYNC_INT_TYPE* apValue) {
  return OSAtomicIncrement32((int32_t*)apValue);
}
static inline SYNC_INT_TYPE _InterlockedDecrement32(SYNC_INT_TYPE* apValue) {
  return OSAtomicDecrement32((int32_t*)apValue);
}
#    define SYNC_INCREMENT(addr) (_InterlockedIncrement32(addr))
#    define SYNC_DECREMENT(addr) (_InterlockedDecrement32(addr))
#    define SYNC_WRITE(addr,v)   (*(addr)) = (v)
#    define SYNC_READ(addr)      (*addr)

// Atomic, X64, linux
#  elif defined __linux__

#    define NI_LINUX_ATOMIC_INLINE static __inline__ __attribute__((always_inline))
NI_LINUX_ATOMIC_INLINE int ni_linux_atomic_swap(int new_value, volatile int *ptr)
{
  int old_value;
  do {
    old_value = *ptr;
  } while (__sync_val_compare_and_swap(ptr, old_value, new_value) != old_value);
  return old_value;
}
#    define SYNC_INT_TYPE int
#    define SYNC_INCREMENT(addr) __sync_add_and_fetch(addr,1)
#    define SYNC_DECREMENT(addr) __sync_sub_and_fetch(addr,1)
#    define SYNC_WRITE(addr,v)   ni_linux_atomic_swap(v,addr)
#    define SYNC_READ(addr)      (*addr)

#  endif

//// X86 //////////////////////////////////////////////////////////////////////////////////////
#elif defined _M_IX86 || defined __i386__ || defined __X86__ || defined _X86 || defined __THW_INTEL__ || defined __I86__ || defined __INTEL__

#  define niCPUArch "x86"
#  define ni32           1
#  define niLittleEndian 1
#  define niIntelX86     1
#  define niX86          1

// Atomic, X86, Windows
#  ifdef _MSC_VER
#    include <intrin.h>
#    define SYNC_INT_TYPE long
#    define SYNC_READ(addr) *addr
#    define SYNC_WRITE(addr, value) *addr = value
#    define SYNC_INCREMENT(addr) _InterlockedIncrement(addr)
#    define SYNC_DECREMENT(addr) _InterlockedDecrement(addr)

// Atomic, X86, OSX
#  elif defined(__APPLE__) && defined(__MACH__) // OSX
#    define SYNC_INT_TYPE long signed int
#    include <libkern/OSAtomic.h>
niCAssert(sizeof(SYNC_INT_TYPE) == sizeof(int32_t));
static inline SYNC_INT_TYPE _InterlockedIncrement32(SYNC_INT_TYPE* apValue) {
  return OSAtomicIncrement32((int32_t*)apValue);
}
static inline SYNC_INT_TYPE _InterlockedDecrement32(SYNC_INT_TYPE* apValue) {
  return OSAtomicDecrement32((int32_t*)apValue);
}
#    define SYNC_INCREMENT(addr) (_InterlockedIncrement32(addr))
#    define SYNC_DECREMENT(addr) (_InterlockedDecrement32(addr))
#    define SYNC_WRITE(addr,v)   (*(addr)) = (v)
#    define SYNC_READ(addr)      (*addr)

// Atomic, X86, Android
#  elif defined ANDROID
#    define SYNC_INT_TYPE int
#    include <sys/atomics.h>
#    define SYNC_INCREMENT(addr) (__atomic_inc(addr)+1)
#    define SYNC_DECREMENT(addr) (__atomic_dec(addr)-1)
#    define SYNC_WRITE(addr,v)   __atomic_swap(v,addr)
#    define SYNC_READ(addr)      (*addr)

#  endif

//// ALPHA ////////////////////////////////////////////////////////////////////////////////////
#elif defined _M_ALPHA || defined __alpha__ || defined __alpha

#  define niCPUArch "alpha"
#  define ni64           1
#  define niLittleEndian 1
#  define niAlpha        1

//// SuperH (SH4) /////////////////////////////////////////////////////////////////////////////
#elif defined __sh__

#  define niCPUArch "superh"
#  define ni32      1
#  define niSuperH  1

//// MIPS /////////////////////////////////////////////////////////////////////////////////////
#elif defined __MIPS__ || defined __mips__ || defined __mips

#  define niCPUArch "mips"
#  define ni32      1
#  define niMIPS    1

//// PowerPC //////////////////////////////////////////////////////////////////////////////////
#elif defined _M_PPC || defined __powerpc || defined __ppc__ || defined _ARCH_PPC

#  define niCPUArch "ppc"
#  define ni32      1
#  define niPPC     1

#else

#  error "No CPU Detected."

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// Default Sync implementations - UNSAFE, not Synchronized
#ifndef SYNC_INT_TYPE
#  pragma error("=== SYNC_INT_TYPE not detected")
#endif

#ifndef SYNC_READ
#  pragma message("=== SYNC_READ not defined, using default UNSAFE operation")
#  define SYNC_READ(addr) *addr
#endif

#ifndef SYNC_WRITE
#  pragma message("=== SYNC_WRITE not defined, using default UNSAFE operation")
#  define SYNC_WRITE(addr, value) *addr = value
#endif

#ifndef SYNC_INCREMENT
#  pragma message("=== SYNC_INCREMENT not defined, using default UNSAFE operation")
#  define SYNC_INCREMENT(addr) (++*addr)
#endif

#ifndef SYNC_DECREMENT
#  pragma message("=== SYNC_DECREMENT not defined, using default UNSAFE operation")
#  define SYNC_DECREMENT(addr) (--*addr)
#endif

niCAssert(sizeof(SYNC_INT_TYPE) == 4); /* Should be a 32bit integer */

#ifdef __cplusplus
namespace ni {
#endif

typedef SYNC_INT_TYPE tSyncInt;

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// Endianess
#if !defined niLittleEndian && !defined niBigEndian
#  if defined __LITTLE_ENDIAN__
#    define niLittleEndian 1
#  elif defined __BIG_ENDIAN__
#    define niBigEndian 1
#  endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// 'Computed' CPU macros
#if !defined niCPUEndian
#  if defined niLittleEndian
#    define niCPUEndian "le"
#  elif defined niBigEndian
#    define niCPUEndian "be"
#  endif
#endif

#if !defined niCPUWordSize
#  if defined ni16
#    define niCPUWordSize 16
#  elif defined ni32
#    define niCPUWordSize 32
#  elif defined ni64
#    define niCPUWordSize 64
#  endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
#ifndef niCPUArch
#  pragma message("E/C++ Preprocessor: niCPUArch not defined.")
#endif
#ifndef niCPUEndian
#  pragma message("E/C++ Preprocessor: niCPUEndian not defined.")
#endif
#ifndef niCPUWordSize
#  pragma message("E/C++ Preprocessor: niCPUWordSize not defined.")
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Platform specific headers (OS)

#ifndef niPlatformDetected
#  include "Platforms/MetalSL.h" // Metal shader language
#endif

#ifndef niPlatformDetected
#  include "Platforms/OSX.h" // OSX
#endif

#ifndef niPlatformDetected
#  include "Platforms/IOS.h" // IOS
#endif

#ifndef niPlatformDetected
#  include "Platforms/WinRT.h" // VC++ and WinRT
#endif

#ifndef niPlatformDetected
#  include "Platforms/MSVC.h" // VC++ or Intel C++ and Win32
#endif

#ifndef niPlatformDetected
#  include "Platforms/GCCW32.h" // GCC and Win32
#endif

#ifndef niPlatformDetected
#  include "Platforms/Android.h" // GCC and Android
#endif

#ifndef niPlatformDetected
#  include "Platforms/JSCC.h" // HTML5/JS
#endif

#ifndef niPlatformDetected
#  include "Platforms/Linux.h" // Linux
#endif

#ifndef niPlatformDetected
#  include "Platforms/QNX.h" // QNX
#endif

#ifndef niPlatformDetected
#  error "No Platform Detected."
#endif

// Build the SDK without any error infos nor logging
//#define _REDIST

#ifndef _REDIST
#  define niConfig_LogSourceCodeInfo
#endif

// Build the SDK without support for any graphics rendering to a window or output device
//#define niConfig_ConsoleOnly

// Try to generate the minimum size for the module def (includes the interop wrappers)
// Enabled automatically if MinCodeSize is specified
//#define niConfig_MinModuleDefSize

// Will no define the interfaces, nor enums, nor constants in the module's GetModuleDef
// This reduces the DLL size, but all features the automation will not be available.
// The automation is necessary for scripting and automated serialization.
//#define niConfig_OnlyObjectTypesIDL

#ifdef __GNUC__
#  undef NULL
// This disable 'most' (yup GCC can't get everything right...) the retarded
// 'arithmetic with NULL' crap...
#  define NULL 0
#  undef __forceinline
#  define __forceinline inline
#endif

#ifndef __forceinline
#  ifdef __GNUC__
//!!! DO NOT USE __attribute__((__always_inline__)), GCC on Android (NDK4, GCC
//!!! 4.4.0) generate crashy executables when used ... (yes GCC is awesome)
#    define __forceinline inline
#  elif defined niMSVC
// MSVC has __forceinline as a keyword
#  else
#    pragma message("E/C++ Preprocessor: __forceinline not defined.")
#  endif
#endif

#ifndef __noinline
#  ifdef __GNUC__
#    define __noinline __attribute__ ((noinline))
#  elif defined niMSVC
#    define __noinline __declspec(noinline)
#  else
#    pragma message("E/C++ Preprocessor: __noinline not defined.")
#  endif
#endif

// No platform detected
#if !defined niPlatformDetected
#  pragma message("E/C++ Preprocessor: No supported platform detected.")
#endif

#if !defined __ni_export_call_decl
#  define __ni_export_call_decl __cdecl
#endif

#ifdef __cplusplus
#  define niExternC extern "C"
#else
#  define niExternC extern
#endif

#if !defined __ni_module_export
#  ifdef niNoDLL
#    define __ni_module_export
#  elif defined __GNUC__
#    define __ni_module_export  __attribute__((visibility("default")))
#  elif defined niMSVC
#    define __ni_module_export __declspec(dllexport)
#  else
#    pragma message("E/C++ Preprocessor: __ni_module_export not defined.")
#  endif
#endif

#if !defined __ni_module_import
#  ifdef niNoDLL
#    define __ni_module_import
#  elif defined __GNUC__
#    define __ni_module_import  __attribute__((visibility("default")))
#  elif defined niMSVC
#    define __ni_module_import __declspec(dllimport)
#  else
#    pragma message("E/C++ Preprocessor: __ni_module_import not defined.")
#  endif
#endif

#define niExportFunc(TYPE) niExternC __ni_module_export TYPE __ni_export_call_decl
#define niExportFuncCPP(TYPE) extern __ni_module_export TYPE __ni_export_call_decl

#ifndef niUCharSize
#  pragma message("E/C++ Preprocessor: niUCharSize not defined in the platform header")
#endif

#ifndef niStackBufferSize
// Size of a buffer allocated on the stack 8Kb
#  define niStackBufferSize 0x2000
#endif

#ifndef niStandardTypesAlignment
#  define niStandardTypesAlignment 8
#endif

// Exceptions
#ifdef niNoExceptions
#  define niTry()
#  define niCatchAll() if(0)
#  define niCatch(X) if(0)
#  define niThrow(X) ;
#  define niThrowSpec()
#else
#  define niTry()      try
#  define niCatch(X)   catch(X)
#  define niCatchAll() catch(...)
#  define niThrow(X)   throw X
#  define niThrowSpec() throw()
#endif

#ifdef niDebug
#  define niBuildType "Debug"
#else
#  define niBuildType "Release"
#endif

#ifdef __cplusplus
#  define niNamespace(NAME,IDENT) NAME::IDENT
#  define niGlobalNamespace(IDENT)  ::IDENT
#else // __cplusplus
#  define niNamespace(NAME,IDENT) IDENT
#  define niGlobalNamespace(IDENT)  IDENT
#endif // __cplusplus

#ifndef niPP_Comma
// not a joke... needed in for example: REGISTER_EVENTCLASS_EX(CCStatusMovement, cEventClass_CCStatus<CC_COLLIDE_STATUS_MSG niPP_Comma eCollideCharacterStatus_Collide>);
#  define niPP_Comma ,
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// 'Computed' OS macros
#if defined niWindows
#  define niOS "nt"
#elif defined niOSX
#  define niOS "osx"
#elif defined niIOS
#  define niOS "ios"
#elif defined niAndroid
#  define niOS "adr"
#elif defined niLinux
#  define niOS "linux"
#elif defined niJSCC
#  define niOS "jscc"
#elif defined niMetalSL
#  define niOS "metalsl"
#elif defined niQNX
#  define niOS "qnx"
#else
#  error "E/C++ Preprocessor: Unknown OS."
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// 'Computed' Linker macro
#if defined niMSVC
#  define niLinker "vc"
#elif defined niGCC || defined niJSCC || defined niCLang
#  define niLinker "gcc"
#elif defined niMetalSL
#  define niLinker "metallib"
#else
#  error "E/C++ Preprocessor: Unknown Linker."
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// LOA macros
#define niLOA_Libs niLinker "-" niOS "-" niCPUArch
#define niLOA_Bin  niOS "-" niCPUArch

//////////////////////////////////////////////////////////////////////////////////////////////
//
// Allocate a Static objects or Singleton.
//
// The memory is never freed and the destructor is never called.
//
// This is used only for object which are create only ONCE and are
// NEVER destroyed (such as the system object, global memory
// allocators or log systems...)
//
// This allow us to not bother about the destruction order of static
// global objects and still have the program close without crashing.
//
// It WILL leak the memory, but any modern OS cleans the memory of the
// process so it should be irrelevant.
//
// In general singletons should be local to their unit (not global)
// that is why they are declare as 'static' by default.
//
#define niSingleton(TYPE,NAME,PARAMS)           \
  static TYPE* GetSingleton_##NAME() {          \
    niSingletonConstruct(TYPE,NAME,PARAMS);     \
    return s_##NAME;                            \
  }

// Construct a singleton 'in-place'
#define niSingletonConstruct(TYPE,NAME,PARAMS)              \
  static tU8 s_##NAME##_Mem[sizeof(TYPE)];                  \
  static TYPE* s_##NAME = new(s_##NAME##_Mem) TYPE PARAMS;  \

// Declare a singleton prototype (forward declaration)
#define niSingletonPrototype(TYPE,NAME)         \
  static TYPE* GetSingleton_##NAME();

//////////////////////////////////////////////////////////////////////////////////////////////
// Default definitions if not defined in the platform specific headers.
#ifdef __cplusplus
namespace ni {
#endif // __cplusplus
/** \addtogroup niLang
 * @{
 */

#if !defined niLittleEndian && !niBigEndian
#  pragma message("E/C++ Preprocessor: Endianess not defined in the platform header")
#endif

#ifndef niPragmaPack
#  pragma message("E/C++ Preprocessor: niPragmaPack not defined in the platform header")
#endif

#ifndef niPackPush
#  pragma message("E/C++ Preprocessor: niPackPush not defined in the platform header")
#endif

#ifndef niPackPop
#  pragma message("E/C++ Preprocessor: niPackPop not defined in the platform header")
#endif

#ifndef niPacked
#  pragma message("E/C++ Preprocessor: niPacked attribute not defined in the platform header")
#endif

#ifndef niTypeCChar
#  pragma message("E/C++ Preprocessor: niTypeCChar not defined, fix the platform header.")
#endif

#ifndef niTypeUChar
#  pragma message("E/C++ Preprocessor: niTypeUChar not defined, fix the platform header.")
#endif

#if !defined niNoDLL && !defined niDLLSuffix
#  pragma message("E/C++ Preprocessor: Both niDLLSuffix not defined, fix the platform header.")
#endif

#ifndef niConsoleMain
#  define niConsoleMain() int __no_niConsoleMain_on_this_platform[-1];
#endif

#ifndef niWindowedMain
#  define niWindowedMain() int __no_niWindowedMain_on_this_platform[-1];
#endif

#ifndef niHidden
#  if defined niGCC || defined niCLang
#    define niHidden   __attribute__((visibility("hidden")))
#  else
#    define niHidden
#  endif
#endif

#ifndef niAligned
#  if defined(_MSC_VER)
#    define niAligned(x) __declspec(align(x))
#  else
#    define niAligned(x) __attribute__((aligned(x)))
#  endif
#endif

#define niAlignedType(t,x) t niAligned(x)

#ifdef __cplusplus
template <typename T>
inline const T AlignComputePadding(const T align, const T offset) {
  return (align - (offset % align)) % align;
}
template <typename T>
inline const T AlignOffset(const T align, const T offset) {
  return offset + ((align - (offset % align)) % align);
}
#endif

#define niAlignTo(size, alignment) (((size) + (alignment) - 1) & ~((alignment) - 1))

#ifndef niThreadLocal
#  if defined __GNUC__ || defined __SUNPRO_C  || defined __xlC__
#    define niThreadLocal __thread
#  elif __STDC_VERSION__ >= 201112L && !defined __STDC_NO_THREADS__
#    define niThreadLocal _Thread_local
#  elif defined _WIN32 && _MSC_VER >= 1400
#    define niThreadLocal __declspec(thread)
#  endif
#endif

// Detect C++11 thread_local support
#ifdef __cplusplus
#  if defined(__MINGW32__)
// mingw clang does not support non-trivial destructible types. mingw gcc is
// still broken ( https://gcc.gnu.org/bugzilla/show_bug.cgi?id=83562), new gcc
// defines __has_feature but no cxx_thread_local
#  elif (__clang__ + 0)
// clang defines _MSC_VER as the cl builds it, or masquerades as gcc4.2, so
// check clang first
#    if __has_feature(cxx_thread_local)
#      define niCPP_ThreadLocal (!(_LIBCPP_VERSION + 0)/*gnu stl, vcrt*/ || _LIBCPP_VERSION >= 4000)
#    endif
#  elif (_MSC_VER+0) >= 1900
#    define niCPP_ThreadLocal 1
#  elif (__GNUC__*100+__GNUC_MINOR__) >= 408 // can't be clang
// libstdc++(g++4.8+) and libc++4.0+(not apple). implemented in
// __cxa_thread_atexit in libc++abi, 4.0+ abi has a fallback if no
// __cxa_thread_atexit_impl (e.g. android<23)
#    define niCPP_ThreadLocal 1
#  endif
#endif

#if !defined niThreadLocal11 && defined __cplusplus
// define niThreadLocal11
#  if (niCPP_ThreadLocal+0)
#    define niThreadLocal11 thread_local
#  elif defined _WIN32 && _MSC_VER >= 1400
#    define niThreadLocal11 __declspec(thread)
#  endif
#endif

#ifdef __cplusplus
#  ifndef niTAssert
template <bool B> struct sTAssert {};
template <> struct sTAssert<true> { static void TAssert() {}; };
#    define niTAssert(__a) {const bool __b = (__a) ? true : false; sTAssert<__b>::TAssert();}
#  endif
#endif

#ifndef niTypename
#  define niTypename
#endif

#ifndef niSync
#  define niSync volatile
#endif

#ifndef niOverride
#  ifdef niCPP11
#    define niOverride override
#  else
#    define niOverride
#  endif
#endif

#ifndef niFinal
#  ifdef niCPP11
#    define niFinal final
#  else
#    define niFinal
#  endif
#endif

// Implies support for lambda "[]() { ... }" construct, the auto keyword and move semantic (&&)
#ifndef niCPP_Lambda
#  ifdef niCPP11
#    define niCPP_Lambda
#  endif
#endif

// niLambda* macros, much easier to search for than []()
#ifdef niCPP_Lambda
#  define niLambdaByVal     [=]
#  define niLambdaByRef     [&]
#  define niLambda(capture) [capture]
#else
#  define niLambdaByVal     (*no_lambda_support) // deref an unknown symbol so it should be the first error printed
#  define niLambdaByRef     (*no_lambda_support)
#  define niLambda(capture) (*no_lambda_support)
#endif

#define niImpl niOverride niFinal

#ifndef niEndMain
#  define niEndMain(x) ni::GetLang()->Exit(x); return x;
#endif

#ifndef niPluginEntryPoint
#  ifdef _LIB
#    define niPluginEntryPoint(plugname, epname)  _##epname##plugname
#  else
#    define niPluginEntryPoint(plugname, epname)  epname
#  endif
#endif

#ifndef niCountOf
#  define niCountOf(a)                                \
  ((sizeof(a) / sizeof(*(a))) /                       \
   static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif

#ifndef niCompileTypeCheck
#  define niCompileTypeCheck(TYPE)    ((void)((TYPE*)(NULL)))
#endif

#ifndef niLoopStdScope
#  define niLoopStdScope  // if(0){} else // Was needed for VC++ 6, probably not necessary anywhere anymore though
#endif

#define niLoop(IDX,COUNT)   niLoopStdScope for (niNamespace(ni,tInt) IDX = 0, __c##IDX = niNamespace(ni,tInt)(COUNT); IDX < __c##IDX; ++IDX)
#define niLoopr(IDX,COUNT)  niLoopStdScope for (niNamespace(ni,tInt) IDX = niNamespace(ni,tInt)((COUNT)-1); IDX >= 0; --IDX)
#define niLoopit(IT,ITN,C)  niLoopStdScope for (IT ITN = (C).begin(); ITN != (C).end(); ++ITN)
#define niLooprit(IT,ITN,C) niLoopStdScope for (IT ITN = (C).rbegin(); ITN != (C).rend(); ++ITN)

#if defined __cplusplus && !defined niNoPointer
template <typename T>
__forceinline bool IsOK(const T* p) {
  return (p != NULL) && (p->IsOK());
}
#  define niIsOK(x)  ni::IsOK(x)

template <typename T>
__forceinline bool IsNullPtr(const T* p) {
  return (p == NULL);
}

#  define niIsNullPtr(x) ni::IsNullPtr(x)
#  define niGetIfOK(x) (niIsOK(x) ? (x) : (NULL))
#endif

#ifndef niDeprecated
#  if defined(__cplusplus) && (__cplusplus >= 201402L)
#    define niDeprecated(since, replacement) [[deprecated("Since " #since "; use " #replacement)]]
#  elif defined(__GNUC__) || defined(__clang__)
#    define niDeprecated(since, replacement) __attribute__((deprecated))
#  elif defined(_MSC_VER)
#    define niDeprecated(since, replacement) __declspec(deprecated)
#  else
#    define niDeprecated(since, replacement)
#  endif
#endif // niDeprecated

#ifdef __cplusplus
}
#endif // __cplusplus

#if !defined niShaderLanguage
// Sets whether we should show a message box for asserts & fatal errors before
// exiting or breaking in the debugger. Defaults to 0 (disabled) and can be
// overwritten with the 'niLang.ShowAssertMessageBox' property.
niExportFunc(int)  ni_debug_get_show_assert_message_box();
niExportFunc(void) ni_debug_set_show_assert_message_box(int aShowAssertMessageBox);
// Return 1 if we should break in the debugger / crash.
niExportFunc(int)  ni_debug_assert(int expression, const char* exp, const char* file, int line, const char* func, int* alwaysignore, const char* desc);
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// ni_debug_break
#if defined niShaderLanguage
#  define ni_debug_break()

#elif defined niMSVC
#  define ni_debug_break __debugbreak

#else
#  include <signal.h>

/* Use __builtin_trap() on AArch64 iOS only */
#  if defined(__aarch64__) && defined(__APPLE__)
#    define NI_DEBUG_BREAK_PREFER_BUILTIN_TRAP_TO_SIGTRAP 1
#  else
/* gcc optimizers consider code after __builtin_trap() dead,
 * making __builtin_trap() unsuitable for breaking into the debugger */
#  endif

#  if defined(__i386__) || defined(__x86_64__)
#    define NI_HAVE_TRAP_INSTRUCTION 1
__attribute__((always_inline))
__inline__ static void ni_trap_instruction(void) {
  __asm__ volatile("int $0x03");
}
#  elif defined(__thumb__)
#    define NI_HAVE_TRAP_INSTRUCTION 1
/* FIXME: handle __THUMB_INTERWORK__ */
__attribute__((always_inline))
__inline__ static void ni_trap_instruction(void) {
  /* See 'arm-linux-tdep.c' in GDB source.
   * Both instruction sequences below work. */
#    if 1
  /* 'eabi_linux_thumb_le_breakpoint' */
  __asm__ volatile(".inst 0xde01");
#    else
  /* 'eabi_linux_thumb2_le_breakpoint' */
  __asm__ volatile(".inst.w 0xf7f0a000");
#    endif
  /* Known problem:
   * After a breakpoint hit, can't stepi, step, or continue in GDB.
   * 'step' stuck on the same instruction.
   *
   * Workaround: a new GDB command,
   * 'debugbreak-step' is defined in debugbreak-gdb.py
   * that does:
   * (gdb) set $instruction_len = 2
   * (gdb) tbreak *($pc + $instruction_len)
   * (gdb) jump   *($pc + $instruction_len)
   */
}
#  elif defined(__arm__) && !defined(__thumb__)
#    define NI_HAVE_TRAP_INSTRUCTION 1
__attribute__((always_inline))
__inline__ static void ni_trap_instruction(void) {
  /* See 'arm-linux-tdep.c' in GDB source,
   * 'eabi_linux_arm_le_breakpoint' */
  __asm__ volatile(".inst 0xe7f001f0");
  /* Has same known problem and workaround
   * as Thumb mode */
}
#  elif defined(__aarch64__) && defined(__APPLE__)
/* use __builtin_trap() on AArch64 iOS */
#  elif defined(__aarch64__)
#    define NI_HAVE_TRAP_INSTRUCTION 1
__attribute__((always_inline))
__inline__ static void ni_trap_instruction(void) {
  /* See 'aarch64-tdep.c' in GDB source,
   * 'aarch64_default_breakpoint' */
  __asm__ volatile(".inst 0xd4200000");
}
#  else
/* unknown platform, use raise(SIGTRAP) */
#  endif

__attribute__((always_inline))
__inline__ static void ni_debug_break(void) {
#  if defined NI_HAVE_TRAP_INSTRUCTION
  ni_trap_instruction();
#  elif defined NI_DEBUG_BREAK_PREFER_BUILTIN_TRAP_TO_SIGTRAP
  /* raises SIGILL on Linux x86{,-64}, to continue in gdb:
   * (gdb) handle SIGILL stop nopass
   * */
  __builtin_trap();
#  else
#    ifdef _WIN32
  /* SIGTRAP available only on POSIX-compliant operating systems
   * use builtin trap instead */
  __builtin_trap();
#    else
  raise(SIGTRAP);
#    endif
#  endif
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// Trace, Assert, ...
#ifdef __cplusplus
namespace ni {
#endif // __cplusplus

#if !defined __FUNCTION__
#  define __FUNCTION__ __func__
#endif

#ifdef niMSVC
// __PRETTY_FUNCTION__ allow us to get the name of the class, not available on MSVC
#  define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#ifndef niPrint
#  define niPrint(x)  niNamespace(ni,ni_log)(niNamespace(ni,eLogFlags_Raw)|niNamespace(ni,eLogFlags_Stdout)|niNamespace(ni,eLogFlags_NoNewLine),__FILE__,__FUNCTION__,__LINE__,x)
#endif

#ifndef niPrintln
#  define niPrintln(x)  niNamespace(ni,ni_log)(niNamespace(ni,eLogFlags_Raw)|niNamespace(ni,eLogFlags_Stdout),__FILE__,__FUNCTION__,__LINE__,x)
#endif

#if !defined niPrintFmt && defined __cplusplus
#  define niPrintFmt(FMT) {                       \
    ni::cString formatted; formatted.Format FMT;  \
    niPrintln(formatted.Chars());                 \
  }
#endif

#ifndef niAssume
#  define niAssume(x)
#endif

#define niDoAssert(exp)                                                 \
  do {                                                                  \
    static int __ignoreAssert = 0;                                      \
    if(!__ignoreAssert) {                                               \
      if(ni_debug_assert((exp)?1:0, #exp, __FILE__, __LINE__, __FUNCTION__, &__ignoreAssert, NULL)) \
      { ni_debug_break(); }                                             \
    }                                                                   \
  } while(0)

#define niDoAssertMsg(exp, desc)                                        \
  do {                                                                  \
    static int __ignoreAssert = 0;                                      \
    if(!__ignoreAssert) {                                               \
      if(ni_debug_assert((exp)?1:0, #exp, __FILE__, __LINE__, __FUNCTION__, &__ignoreAssert, desc)) \
      { ni_debug_break(); }                                             \
    }                                                                   \
  } while(0)

#define niDoAssertUnreachable(msg)                                      \
  do {                                                                  \
    static int __ignoreAssert = 0;                                      \
    if(!__ignoreAssert) {                                               \
      if(ni_debug_assert(0, msg, __FILE__, __LINE__, __FUNCTION__, &__ignoreAssert, NULL)) \
      { ni_debug_break(); }                                             \
    }                                                                   \
  } while(0)

#ifndef niAssert
#  ifdef _DEBUG
#    define niAssert(exp) niDoAssert(exp)
#  elif defined _LINT
#    define niAssert(exp) niAssume(exp)
#  else
#    define niAssert(exp)
#  endif
#endif

#ifndef niAssertMsg
#  ifdef _DEBUG
#    define niAssertMsg(exp,desc) niDoAssertMsg(exp,desc)
#  elif defined _LINT
#    define niAssertMsg(exp,desc) niAssume(exp)
#  else
#    define niAssertMsg(exp,desc)
#  endif
#endif

#ifndef niAssertUnreachable
#  ifdef _DEBUG
#    define niAssertUnreachable(msg) niDoAssertUnreachable(msg)
#  else
#    define niAssertUnreachable(msg)
#  endif
#endif

#define niDebugAssert niAssert
#define niDebugAssertMsg niAssertMsg
#define niDebugAssertUnreachable niAssertUnreachable

#ifndef niUnused
#  ifdef __cplusplus
#    define niUnused(x) (void)(x);
#  else
#    define niUnused(x)
#  endif
#endif

#ifndef niBit
#  define niBit(x)         ((niNamespace(ni,tU32))(1<<(x)))
#endif

#ifndef niBit64
#  define niBit64(x)       ((niNamespace(ni,tU64))(((niNamespace(ni,tU64))1)<<((niNamespace(ni,tU64))(x))))
#endif

// Define our own macros for writing 64-bit constants. This is less fragile
// than defining __STDC_CONSTANT_MACROS before including <stdint.h>, and it
// works on compilers that don't have it (like MSVC).
#ifdef niMSVC
#  define niU64Const(x)      (x ## UI64)
#  define niI64Const(x)      (x ## I64)
#  ifdef ni64
#    define niIntPtrConst(x) (x ## I64)
#    define niPtrPrefix      "ll"
#  else
#    define niIntPtrConst(x) (x)
#    define niPtrPrefix      ""
#  endif  // ni64
#elif defined niMingW64
#  define niU64Const(x)      (x ## ULL)
#  define niI64Const(x)      (x ## LL)
#  define niIntPtrConst(x)   (x ## LL)
#  define niPtrPrefix        "I64"
#elif defined ni64
#  if defined niOSX
#    define niU64Const(x)    (x ## ULL)
#    define niI64Const(x)    (x ## LL)
#  else
#    define niU64Const(x)    (x ## UL)
#    define niI64Const(x)    (x ## L)
#  endif
#  define niIntPtrConst(x)   (x ## L)
#  define niPtrPrefix        "l"
#else
#  define niU64Const(x)      (x ## ULL)
#  define niI64Const(x)      (x ## LL)
#  define niIntPtrConst(x)   (x)
#  define niPtrPrefix        ""
#endif

// The following macro works on both 32 and 64-bit platforms.
// Usage: instead of writing 0x1234567890123456 write niU64Const2Parts(0x12345678,90123456);
#define niU64Const2Parts(a, b) (((static_cast<ni::tU64>(a) << 32) + 0x##b##u))

#ifndef niConst
#  define niConst const
#endif
#ifndef niConstValue
#  define niConstValue static const
#endif

#ifdef __cplusplus
#  define niDefaultParam(VAL) = VAL
#  define niDefaultTemplate template <typename T>
#else // __cplusplus
#  define niDefaultParam(VAL)
#  define niDefaultTemplate
#endif // __cplusplus

#ifndef niDefaultTypeT
#  define niDefaultTypeT  niNamespace(ni,tF32)
#endif

#define niSafeFloatDiv

/**@}*/
#ifdef __cplusplus
}
#endif // __cplusplus

//--------------------------------------------------------------------------------------------
//
//  PanicAssert
//
//--------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace ni {
#endif // __cplusplus
/** \addtogroup niLang
 * @{
 */

niExportFunc(void) ni_panic_assert(
    int expression,
    const char* exp,
    const char* file,
    int line,
    const char* func,
    const char* desc);

#ifndef niPanicAssert
#define niPanicAssert(exp)                                              \
  niAssume(exp);                                                        \
  if (!(exp)) {                                                         \
    niNamespace(ni,ni_panic_assert)((exp)?1:0, #exp, __FILE__, __LINE__, __FUNCTION__, NULL); \
  }
#endif

#ifndef niPanicAssertMsg
#define niPanicAssertMsg(exp,msg)                                       \
  niAssume(exp);                                                        \
  if (!(exp)) {                                                         \
    niNamespace(ni,ni_panic_assert)((exp)?1:0, #exp, __FILE__, __LINE__, __FUNCTION__, msg); \
  }
#endif

#ifndef niPanicUnreachable
#define niPanicUnreachable(...)                                         \
  niNamespace(ni,ni_panic_assert)(0, "UNREACHABLE", __FILE__, __LINE__, __FUNCTION__, "" __VA_ARGS__);
#endif

/**@}*/
#ifdef __cplusplus
}
#endif // __cplusplus

//--------------------------------------------------------------------------------------------
//
//  Atomic types
//
//--------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace ni {
#endif // __cplusplus

/** \addtogroup niLang
 * @{
 */

#define niNull    ((void*)0)

#ifdef __cplusplus
class cString; // string class forward declaration
#endif

typedef niTypeI8      tI8;
typedef niTypeU8      tU8;
typedef niTypeI16     tI16;
typedef niTypeU16     tU16;
typedef niTypeI32     tI32;
typedef niTypeU32     tU32;
typedef niTypeI64     tI64;
typedef niTypeU64     tU64;
typedef niTypePtr     tPtr;
typedef niTypeOffset  tOffset;
typedef niTypeIntPtr  tIntPtr;
typedef niTypeUIntPtr tUIntPtr;
typedef niTypeF32     tF32;
#ifdef niTypeF64
typedef niTypeF64     tF64;
#endif
typedef tU32          tVersion;
typedef tU32          tEnum;
typedef tIntPtr       tHandle;
typedef tF32          tFloat;

typedef tI8 tBool;

#define niTrue 1
#define niFalse 0
#ifndef _DEBUG
#  define niTrueInDebug 0
#  define niFalseInDebug 1
#else
#  define niTrueInDebug 1
#  define niFalseInDebug 0
#endif

#ifdef __cplusplus
enum {
  eTrue = 1,
  eFalse = 0,
#  ifndef _DEBUG
  eTrueInDebug = 0,
  eFalseInDebug = 1,
#  else
  eTrueInDebug = 1,
  eFalseInDebug = 0,
#  endif
};
#endif // __cplusplus

typedef niTypeInt  tInt;
typedef niTypeUInt tUInt;
#if defined ni64
typedef niTypeU64  tSize;
#else
typedef niTypeU32  tSize;
#endif

niCAssert(sizeof(tBool) == 1);
niCAssert(sizeof(tI8) == 1);
niCAssert(sizeof(tU8) == 1);
niCAssert(sizeof(tI16) == 2);
niCAssert(sizeof(tU16) == 2);
niCAssert(sizeof(tI32) == 4);
niCAssert(sizeof(tU32) == 4);
niCAssert(sizeof(tI64) == 8);
niCAssert(sizeof(tU64) == 8);
niCAssert(sizeof(tF32) == 4);
#ifdef niTypeF64
niCAssert(sizeof(tF64) == 8);
#endif
#ifndef niNoPointer
niCAssert(sizeof(void*) == sizeof(tPtr));
#endif
#if defined ni64
#  ifndef niTypeIntSize
#    define niTypeIntSize 8
#  endif
#  ifndef niNoPointer
niCAssert(sizeof(void*) == 8);
#  endif
niCAssert(sizeof(tSize) == 8);
niCAssert(sizeof(tOffset) == 8);
niCAssert(sizeof(tIntPtr) == 8);
niCAssert(sizeof(tUIntPtr) == 8);
#elif defined ni32
#  ifndef niTypeIntSize
#    define niTypeIntSize 4
#  endif
#  ifndef niNoPointer
niCAssert(sizeof(void*) == 4);
#  endif
niCAssert(sizeof(tSize) == 4);
niCAssert(sizeof(tOffset) == 4);
niCAssert(sizeof(tIntPtr) == 4);
niCAssert(sizeof(tUIntPtr) == 4);
#else
#  pragma message("E/C++ Preprocessor: Machine word size not supported.")
#endif

#if niTypeIntSize == 4
niCAssert(sizeof(tInt) == 4);
niCAssert(sizeof(tUInt) == 4);
#elif niTypeIntSize == 8
niCAssert(sizeof(tInt) == 8);
niCAssert(sizeof(tUInt) == 8);
#else
#  error "No valid tInt size defined"
#endif

#if niPragmaPack
#  pragma pack(push,1)
#endif
typedef struct { tU16 w;  tU8  b; } niPacked(1)   tU24;
typedef struct { tU32 dw; tU8  b; } niPacked(1)   tU40;
typedef struct { tU32 dw; tU16 w; } niPacked(1)   tU48;
typedef struct { tU32 dw; tU16 w; tU8 b; } niPacked(1)  tU56;
niCAssert(sizeof(tU24) == 3);
niCAssert(sizeof(tU40) == 5);
niCAssert(sizeof(tU48) == 6);
niCAssert(sizeof(tU56) == 7);
#if niPragmaPack
#  pragma pack(pop)
#endif

niCAssert(((tIntPtr)-1) < 0);
niCAssert(((tUIntPtr)-1) > 0);
niCAssert(((tOffset)-1) < 0);
niCAssert(((tSize)-1) > 0);

//! Index type.
typedef tU32 tIndex;

#define niInvalidHandle 0xDEADBEEF

#if !defined niShaderLanguage
niConstValue tU32 kFeatures = niFeatures;

niConstValue tBool kTrue = niTrue;
niConstValue tBool kFalse = niFalse;

niConstValue tU32 eInvalidHandle = niInvalidHandle;
niConstValue tU32 kInvalidHandle = 0xDEADBEEF;
static inline tBool IsValidHandle(tU32 x)   { return x != eInvalidHandle; }
#endif

#define niMaxI8  0x7f                   //! max possible I8 value
#define niMinI8  -128                   //! min possible I8 value
#define niMaxU8  0xff                   //! max possible U8 value
#define niMinU8  0x00                   //! min possible U8 value
#define niMaxI16 0x7fff                 //! max possible I16 value
#define niMinI16 (-32768)               //! min possible I16 value
#define niMaxU16 0xffff                 //! max possible U16 value
#define niMinU16 0x0000                 //! min possible U16 value
#define niMaxI32 ((niNamespace(ni,tI32))0x7fffffffL) //! max possible I32 value
#define niMinI32 ((niNamespace(ni,tI32))0x80000000) //! min possible I32 value
#define niMaxU32 0xffffffffUL           //! max possible U32 value
#define niMinU32 0x00000000UL           //! min possible U32 value
#define niMaxI64 ((niNamespace(ni,tI64))0x7fffffffffffffffLL) //! max possible I64 value
#define niMinI64 ((niNamespace(ni,tI64))0x8000000000000000LL) //! min possible I64 value
#define niMaxU64 ((niNamespace(ni,tU64))0xffffffffffffffffULL) //! max possible U64 value
#define niMinU64 ((niNamespace(ni,tU64))0x0000000000000000ULL) //! min possible U64 value

#ifdef ni64
#  define niMinIntPtr niMinI64
#  define niMaxIntPtr niMaxI64
#  define niMinUIntPtr niMinU64
#  define niMaxUIntPtr niMaxU64
#else
#  define niMinIntPtr niMinI32
#  define niMaxIntPtr niMaxI32
#  define niMinUIntPtr niMinU32
#  define niMaxUIntPtr niMaxU32
#endif

#ifndef niMaxF32
#  define niMaxF32 3.402823466e+38F //! max possible F32 value
#endif
#ifndef niMinF32
#  define niMinF32 1.175494351e-38F //! min possible F32 value
#endif
#ifndef niEpsilonF32
#  define niEpsilonF32 1.192092896e-07F //! Smallest such that 1.0+niEpsilonF32 !=1.0
#endif
#ifndef niMinExpF32
#  define niMinExpF32 (-125)
#endif
#ifndef niMaxExpF32
#  define niMaxExpF32 (128)
#endif

#ifndef niMaxF64
#  define niMaxF64 1.7976931348623158e+308  //! max possible F64 value
#endif
#ifndef niMinF64
#  define niMinF64 2.2250738585072014e-308  //! min possible F64 value
#endif
#ifndef niEpsilonF64
#  define niEpsilonF64 2.2204460492503131e-016 //! Smallest such that 1.0+niEpsilonF64 !=1.0
#endif
#ifndef niMinExpF64
#  define niMinExpF64 (-1021)
#endif
#ifndef niMaxExpF64
#  define niMaxExpF64 (1024)
#endif

#define niMinVersion  0
#define niMaxVersion  0xFFFFFFFF

#if !defined niShaderLanguage

niConstValue tI8 kMaxI8 = niMaxI8;
niConstValue tI8 kMinI8 = niMinI8;
niConstValue tU8 kMaxU8 = niMaxU8;
niConstValue tU8 kMinU8 = niMinU8;
niConstValue tI16 kMaxI16 = niMaxI16;
niConstValue tI16 kMinI16 = niMinI16;
niConstValue tU16 kMaxU16 = niMaxU16;
niConstValue tU16 kMinU16 = niMinU16;
niConstValue tI32 kMaxI32 = niMaxI32;
niConstValue tI32 kMinI32 = niMinI32;
niConstValue tU32 kMaxU32 = niMaxU32;
niConstValue tU32 kMinU32 = niMinU32;
niConstValue tI64 kMaxI64 = niMaxI64;
niConstValue tI64 kMinI64 = niMinI64;
niConstValue tU64 kMaxU64 = niMaxU64;
niConstValue tU64 kMinU64 = niMinU64;
niConstValue tF32 kMaxF32 = niMaxF32;
niConstValue tF32 kMinF32 = niMinF32;
niConstValue tF32 kEpsilonF32 = niEpsilonF32;
niConstValue tF32 kMaxExpF32 = niMaxExpF32;
niConstValue tF32 kMinExpF32 = niMinExpF32;

#  ifdef niTypeF64
niConstValue tF64 kMaxF64 = niMaxF64;
niConstValue tF64 kMinF64 = niMinF64;
niConstValue tF64 kEpsilonF64 = niEpsilonF64;
niConstValue tF64 kMaxExpF64 = niMaxExpF64;
niConstValue tF64 kMinExpF64 = niMinExpF64;
#  endif

#  ifdef __cplusplus
template<typename T> T TypeMin() { ; /*error*/ }
template<typename T> T TypeMax() { ; /*error*/ }
template<typename T> T TypeEpsilon() { ; /*error*/ }
template<typename T> T TypeZeroTolerance() { ; /*error*/ }
template<typename T> T TypeMinExp() { ; /*error*/ }
template<typename T> T TypeMaxExp() { ; /*error*/ }

template<> inline tI8  TypeMin<tI8>() { return ni::kMinI8; }
template<> inline tI8  TypeMax<tI8>() { return ni::kMaxI8; }
template<> inline tI8  TypeZeroTolerance<tI8>() { return 0; }
template<> inline tU8  TypeMin<tU8>() { return ni::kMinU8; }
template<> inline tU8  TypeMax<tU8>() { return ni::kMaxU8; }
template<> inline tU8  TypeZeroTolerance<tU8>() { return 0; }
template<> inline tI16 TypeMin<tI16>() { return ni::kMinI16; }
template<> inline tI16 TypeMax<tI16>() { return ni::kMaxI16; }
template<> inline tI16 TypeZeroTolerance<tI16>() { return 0; }
template<> inline tU16 TypeMin<tU16>() { return ni::kMinU16; }
template<> inline tU16 TypeMax<tU16>() { return ni::kMaxU16; }
template<> inline tU16 TypeZeroTolerance<tU16>() { return 0; }
template<> inline tI32 TypeMin<tI32>() { return ni::kMinI32; }
template<> inline tI32 TypeMax<tI32>() { return ni::kMaxI32; }
template<> inline tI32 TypeZeroTolerance<tI32>() { return 0; }
template<> inline tU32 TypeMin<tU32>() { return ni::kMinU32; }
template<> inline tU32 TypeMax<tU32>() { return ni::kMaxU32; }
template<> inline tU32 TypeZeroTolerance<tU32>() { return 0; }
template<> inline tI64 TypeMin<tI64>() { return ni::kMinI64; }
template<> inline tI64 TypeMax<tI64>() { return ni::kMaxI64; }
template<> inline tI64 TypeZeroTolerance<tI64>() { return 0; }
template<> inline tU64 TypeMin<tU64>() { return ni::kMinU64; }
template<> inline tU64 TypeMax<tU64>() { return ni::kMaxU64; }
template<> inline tU64 TypeZeroTolerance<tU64>() { return 0; }
template<> inline tF32 TypeMin<tF32>() { return ni::kMinF32; }
template<> inline tF32 TypeMax<tF32>() { return ni::kMaxF32; }
template<> inline tF32 TypeZeroTolerance<tF32>() { return 1e-06f; }
template<> inline tF32 TypeEpsilon<tF32>() { return ni::kEpsilonF32; }
template<> inline tF32 TypeMinExp<tF32>() { return ni::kMinExpF32; }
template<> inline tF32 TypeMaxExp<tF32>() { return ni::kMaxExpF32; }
#    ifdef niTypeF64
template<> inline tF64 TypeMin<tF64>() { return ni::kMinF64; }
template<> inline tF64 TypeMax<tF64>() { return ni::kMaxF64; }
template<> inline tF64 TypeZeroTolerance<tF64>() { return 1e-08; }
template<> inline tF64 TypeEpsilon<tF64>() { return ni::kEpsilonF64; }
template<> inline tF64 TypeMinExp<tF64>() { return ni::kMinExpF64; }
template<> inline tF64 TypeMaxExp<tF64>() { return ni::kMaxExpF64; }
#    endif
#  endif // __cplusplus

#  ifdef __cplusplus

// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
// Such explicit implicit_casts are necessary in surprisingly many
// situations where C++ demands an exact type match instead of an
// argument type convertable to a target type.
//
// The From type can be inferred, so the preferred syntax for using
// implicit_cast is the same as for static_cast etc.:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
template<typename To, typename From>
inline To implicit_cast(From const &f) {
  return f;
}

// When you upcast (that is, cast a pointer from type Foo to type
// SuperclassOfFoo), it's fine to use implicit_cast<>, since upcasts
// always succeed.  When you downcast (that is, cast a pointer from
// type Foo to type SubclassOfFoo), static_cast<> isn't safe, because
// how do you know the pointer is really of type SubclassOfFoo?  It
// could be a bare Foo, or of type DifferentSubclassOfFoo.  Thus,
// when you downcast, you should use this macro.  In debug mode, we
// use dynamic_cast<> to double-check the downcast is legal (we die
// if it's not).  In normal mode, we do the efficient static_cast<>
// instead.  Thus, it's important to test in debug mode to make sure
// the cast is legal!
//    This is the only place in the code we should use dynamic_cast<>.
// In particular, you SHOULDN'T be using dynamic_cast<> in order to
// do RTTI (eg code like this:
//    if (dynamic_cast<Subclass1>(foo)) HandleASubclass1Object(foo);
//    if (dynamic_cast<Subclass2>(foo)) HandleASubclass2Object(foo);
// You should design the code some other way not to need this.
template<typename To, typename From>     // use like this: down_cast<T*>(foo);
inline To down_cast(From* f) {           // so we only accept pointers
#    if defined _DEBUG && defined _CPPRTTI
#      ifdef niMSVC
  // The try/catch allows to mix RTTI/non-RTTI build on MSVC...
#        pragma warning(push)
#        pragma warning(disable:4530)
  try
#      endif
  {
    To _to = static_cast<To>(f);
    To _dynTo = dynamic_cast<To>(f);
    niAssert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
    niAssert(_to == _dynTo);
  }
#      ifdef niMSVC
  catch (...) {
  }
#        pragma warning(pop)
#      endif
#    endif
  return static_cast<To>(f);
}

//
// The type-based aliasing rule allows the compiler to assume that pointers of
// different types (for some definition of different) never alias each other.
// Thus the following code does not work:
//
// float f = foo();
// int fbits = *(int*)(&f);
//
// The compiler 'knows' that the int pointer can't refer to f since the types
// don't match, so the compiler may cache f in a register, leaving random data
// in fbits. Using C++ style casts makes no difference, however a pointer to
// char data is assumed to alias any other pointer. This is the 'memcpy
// exception'.
//
// Bit_cast uses the memcpy exception to move the bits from a variable of one
// type of a variable of another type. Of course the end result is likely to
// be implementation dependent. Most compilers (gcc-4.2 and MSVC 2005) will
// completely optimize BitCast away.
//
// There is an additional use for BitCast. Recent gccs will warn when they see
// casts that may result in breakage due to the type-based aliasing rule. If
// you have checked that there is no breakage you can use BitCast to cast one
// pointer type to another. This confuses gcc enough that it can no longer see
// that you have cast one pointer type to another thus avoiding the warning.
//
// We need different implementations of BitCast for pointer and non-pointer
// values. We use partial specialization of auxiliary struct to work around
// issues with template functions overloading.
//
template <class TDest, class TSrc>
struct bit_cast__helper {
  niCAssert(sizeof(TDest) == sizeof(TSrc));
  static TDest cast(const TSrc& source) {
    TDest dest;
    memcpy(&dest, &source, sizeof(dest));
    return dest;
  }
};
template <class TDest, class TSrc>
struct bit_cast__helper<TDest, TSrc*> {
  static TDest cast(TSrc* source) {
    return bit_cast__helper<TDest, tUIntPtr>::
        cast(reinterpret_cast<tUIntPtr>(source));
  }
};
template <class TDest, class TSrc>
inline TDest bit_cast(const TSrc& source) {
  return bit_cast__helper<TDest, TSrc>::cast(source);
}

//! Same as bit_cast but for when the destination type is bigger than the
//! source type. The destination is initialized to zero first.
template <class TDest, class TSrc>
inline TDest bit_castz(const TSrc& source) {
  // Check that the destination type is larger than the source type, zeroing
  // the destination isnt needed if that isnt the case.
  niCAssert(sizeof(TDest) > sizeof(TSrc));
  TDest dest;
  memset(&dest, 0, sizeof(dest));
  memcpy(&dest, &source, sizeof(source));
  return dest;
}

#    define niCCast(TYPE,VAL)     ((TYPE)(VAL))
#    define niUnsafeCast(TYPE,VAL)    reinterpret_cast<TYPE>(VAL)
#    define niIUnknownCast2(IT,VAL)   ni::down_cast<ni::iUnknown*>(ni::down_cast<IT*>(VAL))
#    define niIUnknownCCast2(IT,VAL)    ni::down_cast<const ni::iUnknown*>(ni::down_cast<const IT*>(VAL))
#    define niIUnknownCast(VAL)     ni::down_cast<ni::iUnknown*>(VAL)
#    define niIUnknownCCast(VAL)      ni::down_cast<const ni::iUnknown*>(VAL)
#    define niStaticCast(TYPE,VAL)    ni::down_cast<TYPE>(VAL)
#    define niConstCast(TYPE,VAL)   const_cast<TYPE>(VAL)
#    define niConstStaticCast(CTYPE,STYPE,VAL)    ni::down_cast<STYPE>(const_cast<CTYPE>(VAL))
#    define niThis(CLASS)   niConstCast(CLASS*,this)
#    define niConstThis(CLASS)  niConstCast(const CLASS*,this)
#    define niValCast(TYPE,VAR)  ((TYPE&)(VAR))
#    define niRefCast(TYPE,VAR)  ((TYPE*)&(VAR))
#    define niRef(TYPE)     TYPE&

#  else // __cplusplus

#    define niCCast(TYPE,VAL)           ((TYPE)(VAL))
#    define niUnsafeCast(TYPE,VAL)          ((TYPE)(VAL))
#    define niIUnknownCast(VAL)           ((ni::iUnknown*)(VAL))
#    define niIUnknownCCast(VAL)            ((ni::iUnknown*)(VAL))
#    define niStaticCast(TYPE,VAL)          ((TYPE)(VAL))
#    define niConstCast(TYPE,VAL)         ((TYPE)(VAL))
#    define niConstStaticCast(CTYPE,STYPE,VAL)    ((TYPE)(VAL))
#    define niValCast(TYPE,VAR)           (*(((TYPE)*)(&(VAR))))
#    define niRefCast(TYPE,VAR)           ((TYPE*)(VAR))
#    define niRef(TYPE)     TYPE*

#    define niDefaultTemplate
#  endif // __cplusplus

static inline tU32 ftoul(tF32 f)  { return *(tU32*)((void*)(&f)); }
static inline tF32 ultof(tU32 u)  { return *(tF32*)((void*)(&u)); }
static inline tI32 ftol(tF32 f) { return *(tI32*)((void*)(&f)); }
static inline tF32 ltof(tI32 u) { return *(tF32*)((void*)(&u)); }
#  ifdef niTypeF64
static inline tU64 dtouq(tF64 f)  { return *(tU64*)((void*)(&f)); }
static inline tF64 uqtod(tU64 u)  { return *(tF64*)((void*)(&u)); }
static inline tI64 dtoq(tF64 f) { return *(tI64*)((void*)(&f)); }
static inline tF64 qtod(tI64 u) { return *(tF64*)((void*)(&u)); }
#  endif

//! UTF8/ascii character.
typedef niTypeCChar    cchar;
typedef niTypeCChar    tCChar;

//! wchar_t character.
typedef niTypeUChar    uchar;
typedef niTypeUChar    tUChar;

#  if niUCharSize == 2
//! UTF16 character.
typedef uchar  gchar;
typedef tUChar tGChar;
//! UTF32 character.
typedef tU32 xchar;
typedef tU32 tXChar;
#  elif niUCharSize == 4
//! UTF16 character.
typedef tU16 gchar;
typedef tU16 tGChar;
//! UTF32 character.
typedef uchar  xchar;
typedef tUChar tXChar;
#  elif niUCharSize == 1
//! UTF16 character.
typedef tU16 gchar;
typedef tU16 tGChar;
//! UTF32 character.
typedef tU32 xchar;
typedef tU32 tXChar;
#  else
#    pragma message("E/C++ Preprocessor: Invalid niUCharSize definition.")
#  endif

niCAssert(sizeof(cchar) == 1);
niCAssert(sizeof(gchar) == 2);
niCAssert(sizeof(xchar) == 4);
#  if niUCharSize == 4
niCAssert(sizeof(uchar) == 4);
#  elif niUCharSize == 2
niCAssert(sizeof(uchar) == 2);
#  else
niCAssert(sizeof(uchar) == 1);
#  endif

niCAssert(sizeof("X"[0]) == 1);
#  if niUCharSize != 1
niCAssert(sizeof(L"X"[0]) == sizeof(uchar));
#  endif

typedef cchar achar;
typedef cchar tAChar;
#  define _A(x) x
#  define niACharIsCChar
niCAssert(sizeof(cchar) == sizeof(achar));

niConstValue tU32 knTypeStringMaxSizeInChar = 32;

#  define niIsStringOK(str)      ((str) && *(str))
#  define niStringIsOK(str)      ((str) && *(str))

#  define AZEROSTR  _A("\0\0\0")
#  define AEOL    _A("\n")
#  define ASPACE    ((achar)(32))

// MAX Size defs
#  define AMAX_DIR  _MAX_DIR  // Maximum length of directory component
#  define AMAX_DRIVE  _MAX_DRIVE  // Maximum length of drive component
#  define AMAX_EXT  _MAX_EXT  // Maximum length of extension component
#  define AMAX_FNAME  _MAX_FNAME  // Maximum length of filename component
#  define AMAX_PATH _MAX_PATH // Maximum length of full path
#  define AMAX_SIZE (AMAX_PATH*2)   // Maximum size of a static const achar string

#  ifdef __cplusplus
// Empty string
template<class T> T* StringEmpty() {
  return (T*)"\0\0\0\0";
}

//! Empty class.
class cEmpty {};
#  endif // __cplusplus

//! UUID type, default/standard variant.
typedef struct tUUID_tag {
  niNamespace(ni,tU32)  nData1;
  niNamespace(ni,tU16)  nData2;
  niNamespace(ni,tU16)  nData3;
  niNamespace(ni,tU8)   nData4[8];
} tUUID;
//! UUID type, time based variant.
typedef struct tUUIDTime_tag {
  niNamespace(ni,tU32)  nTimeLow;
  niNamespace(ni,tU16)  nTimeMid;
  niNamespace(ni,tU16)  nTimeHiAndVersion;
  niNamespace(ni,tU8) nClockSeqReserved;
  niNamespace(ni,tU8) nClockSeqLow;
  niNamespace(ni,tU8)   nNodes[6];
} tUUIDTime;
//! UUID type, 4x64 bits variant
typedef struct tUUID32_tag {
  niNamespace(ni,tU32)  nA;
  niNamespace(ni,tU32)  nB;
  niNamespace(ni,tU32)  nC;
  niNamespace(ni,tU32)  nD;
} tUUID32;
//! UUID type, 2x64 bits variant
typedef struct tUUID64_tag {
  niNamespace(ni,tU64)  nDataHi;
  niNamespace(ni,tU64)  nDataLo;
} tUUID64;
niCAssert(sizeof(tUUID) == 16);
niCAssert(sizeof(tUUIDTime) == 16);
niCAssert(sizeof(tUUID32) == 16);
niCAssert(sizeof(tUUID64) == 16);

//! Number of bytes of a UUID
niConstValue tU32 knUUIDSize = sizeof(tUUID);

//! Zero UUID.
niConstValue tUUID kuuidZero = { 0x00000000,0x0000,0x0000, { 0,0,0,0,0,0,0,0 } };

//! Macro to init a UUID from the uuid generation tool's output (genuuid 2)
#  define niInitUUID(A,B,C,D,E,F,G,H,I,J,K) { A,B,C, { D,E,F,G,H,I,J,K } }
//! Define a UUID from the uuid the uuid generation tool's output (genuuid 2)
#  define niDefineUUID(NAME,A,B,C,D,E,F,G,H,I,J,K)  const ni::tUUID NAME = niInitUUID(A,B,C,D,E,F,G,H,I,J,K);

//! UUID compare
static __forceinline tInt UUIDCmp(const niRef(tUUID) A, const niRef(tUUID) B) {
  return memcmp(&A,&B,sizeof(A));
}
//! UUID equal
static __forceinline tBool UUIDEq(const niRef(tUUID) A, const niRef(tUUID) B) {
#  if niCPUWordSize == 32
  return
      niRefCast(tUUID32,A)->nA == niRefCast(tUUID32,B)->nA &&
      niRefCast(tUUID32,A)->nB == niRefCast(tUUID32,B)->nB &&
      niRefCast(tUUID32,A)->nC == niRefCast(tUUID32,B)->nC &&
      niRefCast(tUUID32,A)->nD == niRefCast(tUUID32,B)->nD;
#  elif niCPUWordSize == 64
  return
      niRefCast(tUUID64,A)->nDataHi == niRefCast(tUUID64,B)->nDataHi &&
      niRefCast(tUUID64,A)->nDataLo == niRefCast(tUUID64,B)->nDataLo;
#  else
#    error "E/C++ Preprocessor: CPU Word size not supported."
#  endif
}

//! Create a local UUID.
//! \remark Faster than a unique UUID, but can be used only on one machine localy.
niExportFunc(tUUID) UUIDGenLocal();
//! Create a global UUID.
niExportFunc(tUUID) UUIDGenGlobal();

#  ifdef __cplusplus
inline bool __stdcall operator == (const tUUID& A,const tUUID& B) { return UUIDEq(A,B); }
inline bool __stdcall operator != (const tUUID& A,const tUUID& B) { return !UUIDEq(A,B); }
inline bool __stdcall operator >  (const tUUID& A,const tUUID& B) { return UUIDCmp(A,B) > 0; }
inline bool __stdcall operator <  (const tUUID& A,const tUUID& B) { return UUIDCmp(A,B) < 0; }
inline bool __stdcall operator >= (const tUUID& A,const tUUID& B) { return UUIDCmp(A,B) >= 0; }
inline bool __stdcall operator <= (const tUUID& A,const tUUID& B) { return UUIDCmp(A,B) <= 0; }
#  endif

//! Type flags.
typedef enum eTypeFlags {
  eTypeFlags_Constant         = 0x00000100,
  eTypeFlags_Pointer          = 0x00000200,
  eTypeFlags_Collectable      = 0x00000400,
  eTypeFlags_Numeric          = 0x00000800,
  eTypeFlags_FUNCPROTO        = 0x00001000,
  eTypeFlags_STRING           = 0x00002000,
  eTypeFlags_TABLE            = 0x00004000,
  eTypeFlags_ARRAY            = 0x00008000,
  eTypeFlags_USERDATA         = 0x00010000,
  eTypeFlags_CLOSURE          = 0x00020000,
  eTypeFlags_NATIVECLOSURE    = 0x00040000,
  eTypeFlags_MethodOptional   = 0x00080000,
  eTypeFlags_MethodSetter     = 0x00100000,
  eTypeFlags_MethodGetter     = 0x00200000,
  eTypeFlags_MethodStatic     = 0x00400000,
  eTypeFlags_ForceDWORD       = 0xFFFFFFFF
} eTypeFlags;

//! Base types enumeration.
typedef enum eType
{
  eType_Null = 0,
  eType_IUnknown = 1,

  eType_FirstGroup4 = 0x10,
  eType_I8 = eType_FirstGroup4+0,
  eType_U8 = eType_FirstGroup4+1,
  eType_I16 = eType_FirstGroup4+2,
  eType_U16 = eType_FirstGroup4+3,
  eType_I32 = eType_FirstGroup4+4,
  eType_U32 = eType_FirstGroup4+5,
  eType_F32 = eType_FirstGroup4+6,
  eType_AChar = eType_FirstGroup4+7,
  eType_LastGroup4 = 0x1F,

  eType_FirstGroup8 = 0x20,
  eType_I64 = eType_FirstGroup8+0,
  eType_U64 = eType_FirstGroup8+1,
  eType_F64 = eType_FirstGroup8+2,
  eType_Vec2f = eType_FirstGroup8+3,
  eType_Vec2i = eType_FirstGroup8+4,
  eType_LastGroup8 = 0x2F,

  eType_FirstGroup12 = 0x30,
  eType_Vec3f = eType_FirstGroup12+0,
  eType_Vec3i = eType_FirstGroup12+1,
  eType_String = eType_FirstGroup12+2,
  eType_LastGroup12 = 0x3F,

  eType_FirstGroup16 = 0x40,
  eType_Vec4f = eType_FirstGroup16+0,
  eType_Vec4i = eType_FirstGroup16+1,
  eType_UUID = eType_FirstGroup16+2,
  eType_LastGroup16 = 0x4F,

  eType_FirstExtended = 0x50,
  eType_Matrixf = eType_FirstExtended+0,
  eType_Variant = eType_FirstExtended+1,
  eType_LastExtended = 0x7F,

  eType_Enum = eType_U32,

  // CPU word size dependant
#  ifdef ni32
  eType_Ptr = eType_U32,
  eType_Size = eType_U32,
  eType_Offset = eType_U32,
  eType_IntPtr = eType_I32,
  eType_UIntPtr = eType_U32,
#  elif defined ni64
  eType_Ptr = eType_U64,
  eType_Size = eType_U64,
  eType_Offset = eType_U64,
  eType_IntPtr = eType_I64,
  eType_UIntPtr = eType_U64,
#  else
#    error "Unknown CPU word size."
#  endif

#  if niTypeIntSize == 4
  eType_Int = eType_I32,
  eType_UInt = eType_U32,
#  elif niTypeIntSize == 8
  eType_Int = eType_I64,
  eType_UInt = eType_U64,
#  else
#    error "No valid tInt size defined"
#  endif

  eType_ASZ = eType_AChar|eTypeFlags_Constant|eTypeFlags_Pointer,
  eType_IUnknownPtr = eType_IUnknown|eTypeFlags_Pointer,

  eType_ForceDWORD = 0xFFFFFFFF
} eType;

typedef tU32  tType;

niConstValue tU32 knMaxSizeOfTypeRetByValue = 64;
niConstValue tU32 knTypeRawNumBits = 7; // number of bits without flags
niConstValue tU32 knTypeMask = 0xFF;
#  define niType(TYPE)      ((eType)(((tType)(TYPE))&knTypeMask))
#  define niTypeIsPointer(TYPE)  ((TYPE) == eType_Ptr || ((TYPE)&eTypeFlags_Pointer))

#  ifdef __cplusplus
struct Var;
#  else
typedef struct Var Var;
#  endif

//! Compare the raw data of the variant.
//! \remark This just compares the bits contained in the variant, not distinction
//!         is made between a float or an integer for example.
niExportFunc(tInt) VarCompare(niConst Var* a, niConst Var* b);
//! Get a Null variant
niExportFunc(const Var*) VarGetNull();
//! Var destructor.
niExportFunc(void) VarDestruct(Var* a);

#  ifdef __cplusplus
#    define niVarNull (*ni::VarGetNull())
#  else
#    define niVarNull VarGetNull()
#  endif

#endif // niShaderLanguage

/**@}*/
#ifdef __cplusplus
}
#endif // __cplusplus

//--------------------------------------------------------------------------------------------
//
//  Memory allocator
//
//--------------------------------------------------------------------------------------------
#if !defined niShaderLanguage
#  ifdef __cplusplus
namespace ni {
#  endif // __cplusplus

/** \addtogroup niLang
 * @{
 */

niExportFunc(void*) ni_malloc(size_t size, const char* file, int line, const char* fun);
niExportFunc(void*) ni_aligned_malloc(size_t size, size_t alignment, const char* file, int line, const char* fun);
niExportFunc(void)  ni_free(void* ptr, const char* file, int line, const char* fun);
niExportFunc(void*) ni_realloc(void* ptr, size_t size, const char* file, int line, const char* fun);
niExportFunc(void*) ni_object_alloc(size_t size, const char* file, int line, const char* fun);
niExportFunc(void) ni_object_free(void* ptr, const char* file, int line, const char* fun);

#  if defined niMalloc || defined niAlignedMalloc || defined niFree || defined niRealloc
#    error("niMalloc, niAlignedMalloc, niFree and niRealloc should not be defined by the platform header.")
#  endif

#  ifdef __cplusplus
template <typename T>
static void ni_delete(T* ptr, const char *f, int l, const char *sf)
{
	if (ptr) {
		ptr->~T();
		ni_object_free(ptr, f, l, sf);
	}
}
#  endif

#  if !defined niEmbedded && !defined _REDIST
#    define niMalloc(size)                  niNamespace(ni,ni_malloc)(size,__FILE__,__LINE__,__FUNCTION__)
#    define niAlignedMalloc(size,alignment) niNamespace(ni,ni_aligned_malloc)(size,alignment,__FILE__,__LINE__,__FUNCTION__)
#    define niFree(ptr)                     niNamespace(ni,ni_free)((void*)(ptr),__FILE__,__LINE__,__FUNCTION__)
#    define niRealloc(ptr,size)             niNamespace(ni,ni_realloc)((void*)(ptr),size,__FILE__,__LINE__,__FUNCTION__)
#    define niNew     new(__FILE__,__LINE__,__FUNCTION__)
#    define niNewM    new(__FILE__,__LINE__,"member_init")
#    define niDelete  delete
#  else
#    define niMalloc(size)                  niNamespace(ni,ni_malloc)(size,NULL,-1,NULL)
#    define niAlignedMalloc(size,alignment) niNamespace(ni,ni_aligned_malloc)(size,alignment,NULL,-1,NULL)
#    define niFree(ptr)                     niNamespace(ni,ni_free)((void*)(ptr),NULL,-1,NULL)
#    define niRealloc(ptr,size)             niNamespace(ni,ni_realloc)((void*)(ptr),size,NULL,-1,NULL)
#    define niNew    new
#    define niNewM   new
#    define niDelete delete
#  endif

// Safe memory freeing
#  ifndef niSafeDelete
#    define niSafeDelete(p)   { if(p) { niDelete p; (p)=NULL; } }
#  endif

#  ifndef niSafeDeleteArray
#    define niSafeDeleteArray(p)  { if(p) { niDelete[] p; (p)=NULL; } }
#  endif

#  ifndef niSafeRelease
#    define niSafeRelease(p)    { if(p) { (p)->Release(); (p)=NULL; } }
#  endif

#  ifndef niSafeInvalidateRelease
#    define niSafeInvalidateRelease(p)  { if(p) { (p)->Invalidate(); (p)->Release();  (p)=NULL; } }
#  endif

#  ifndef niSafeFree
#    define niSafeFree(p)     { if(p) { niFree(p);  (p)=NULL; } }
#  endif

#  define niTMalloc(TYPE,COUNT) (TYPE*)niMalloc(sizeof(TYPE)*(COUNT))

/**@}*/
#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // niShaderLanguage

//--------------------------------------------------------------------------------------------
//
//  Log
//
//--------------------------------------------------------------------------------------------
#if !defined niShaderLanguage
#  ifdef __cplusplus
namespace ni {
#  endif // __cplusplus

/** \addtogroup niLang
 * @{
 */

//! Log flags.
enum eLogFlags
{
  //! Raw type, log the message as-is without any formatting.
  eLogFlags_Raw = niBit(0),
  //! Debug log.
  eLogFlags_Debug = niBit(1),
  //! Info log.
  eLogFlags_Info = niBit(2),
  //! Warning log.
  eLogFlags_Warning = niBit(3),
  //! Error log.
  eLogFlags_Error = niBit(4),
  //! All types. (max 16 bits)
  eLogFlags_All = 0xFFFF,
  //! Format as a MSVC error message.
  eLogFlags_FormatMSVC = niBit(16),
  //! Do not call the log callback.
  eLogFlags_NoCallbackOutput = niBit(17),
  //! Do not output to the regular loggers, only the log callback can be called.
  eLogFlags_NoRegularOutput = niBit(18),
  //! If possible, do not append a new line.
  eLogFlags_NoNewLine = niBit(19),
  //! Outputs to stdout. Stderr is the default when not specified.
  eLogFlags_Stdout = niBit(20),
  //! Do not prefix with the log type.
  eLogFlags_NoLogTypePrefix = niBit(21),
  //! \internal
  eLogFlags_ForceDWORD = 0xFFFFFFFF
};

//! Log flags type.
//! \see ni::eLogFlags
typedef niTypeU32 tLogFlags;

//! Log message callback type, return false to not log in default outputs
typedef tBool (__ni_export_call_decl *tpfnLogMessage)(tLogFlags type, const tF64 afTime, const char* file, const char* func, int line, const char* msg);

niExportFunc(tU32) ni_log_set_filter(tU32 exclude);
niExportFunc(tU32) ni_log_get_filter();
niExportFunc(void) ni_log_set_callback(tpfnLogMessage);

niExportFunc(void) ni_log_system_info_once();

niExportFunc(void) ni_log(tLogFlags type, const char* file, const char* func, int line, const char* msg);

#  ifdef __cplusplus
niExportFunc(void) ni_log_format_message(cString& strOut,
                                         tU32 aType,
                                         const char* aaszFile,
                                         const ni::tU32 anLine,
                                         const char* aaszFunction,
                                         const char* aaszMsg,
                                         const tF64 afTime,
                                         const tF64 afPrevTime);
#  endif

#  define niLog_(FLAGS,MSG) ni::ni_log(FLAGS, __FILE__, __FUNCTION__, __LINE__, MSG)
#  define niLog(TYPE,MSG) niLog_(ni::eLogFlags_##TYPE, MSG)

#  define niInfo(desc) niLog(Info,desc)
#  define niTrace(desc) niLog(Debug,desc)
#  if defined _REDIST
#    define niError(desc)
#    define niWarning(desc)
#  else
#    define niError(desc)   niLog(Error,desc)
#    define niWarning(desc) niLog(Warning,desc)
#  endif // #ifdef _REDIST

#  define niCheck_(COND,MSG,RET)        if (!(COND)) { niError(MSG _A("Check '") _A(#COND) _A("' failed.")); return RET; }
#  define niCheck(COND,RET)             niCheck_(COND,_A(""),RET)
#  define niCheckIsOK_(PARAM,MSG,RET)   niCheck_(niIsOK(PARAM),MSG,RET)
#  define niCheckIsOK(COND,RET)         niCheckIsOK_(COND,_A(""),RET)
#  define niCheckSilent(COND,RET)       if (!(COND)) { return RET; }

#  ifdef niDebug
#    define niCheckAssert(COND,MSG,RET)     if (!(COND)) { niAssertMsg(COND,MSG); return RET; }
#  else
#    define niCheckAssert(COND,MSG,RET)     if (!(COND)) { return RET; }
#  endif

/**@}*/
#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // niShaderLanguage

//--------------------------------------------------------------------------------------------
//
//  Base mathematic definitions
//
//--------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace ni {
#endif // __cplusplus

/** \addtogroup niLang
 * @{
 */

#define niEpsilon0  1.0f
#define niEpsilon1  1e-1f
#define niEpsilon2  1e-2f
#define niEpsilon3  1e-3f
#define niEpsilon4  1e-4f
#define niEpsilon5  1e-5f
#define niEpsilon6  1e-6f
#define niEpsilon7  1e-7f
#define niEpsilon8  1e-8f
#define niEpsilon9  1e-9f
#define niEpsilon10 1e-10f
#define niEpsilon11 1e-11f
#define niEpsilon12 1e-12f

#define niEqualX(X, Y, eps) (niNamespace(ni,Abs)((X) - (Y)) < (eps))
#define niEqual0(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon0)
#define niEqual1(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon1)
#define niEqual2(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon2)
#define niEqual3(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon3)
#define niEqual4(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon4)
#define niEqual5(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon5)
#define niEqual6(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon6)
#define niEqual7(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon7)
#define niEqual8(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon8)
#define niEqual9(X, Y)      (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon9)
#define niEqual10(X, Y)   (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon10)
#define niEqual11(X, Y)   (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon11)
#define niEqual12(X, Y)   (niNamespace(ni,Abs)((X) - (Y)) < niEpsilon12)

// float to float
#define niRound(X) ( (X) > 0.0f ?                           \
                     ( (float) ( (int) ( (X) + 0.5f) ) ) :  \
                     ( (float) ( (int) ( (X) - 0.5f) ) ) )

#define niRound0(X, I) ( ( (X) * (I) ) > 0.0f ?                         \
                         ( ( (float) ( (int) ( (X) * (I) + 0.5f) ) ) / (I) ): \
                         ( ( (float) ( (int) ( (X) * (I) - 0.5f) ) ) / (I) ) )
#define niRound1(X) (niRound0((X), 10.0f))
#define niRound2(X) (niRound0((X), 100.0f))
#define niRound3(X) (niRound0((X), 1000.0f))
#define niRound4(X) (niRound0((X), 10000.0f))
#define niRound5(X) (niRound0((X), 100000.0f))
#define niRound6(X) (niRound0((X), 1000000.0f))

// float to int
#define niRoundI(X) ((X) > 0.0f ? (((int) ((X) + 0.5f))) : (((int) ((X) - 0.5f))))

#define niPi     3.1415926535897932384626433832795028841971693993751 // 180 deg
#define niPif    3.1415926535897932384626433832795028841971693993751f // 180 deg
#define niPi2    1.5707963267948966192313216916395 // 90 deg
#define niPi2f   1.5707963267948966192313216916395f // 90 deg
#define niPi4    0.78539816339744830961566084581975 // 45 deg
#define niPi4f   0.78539816339744830961566084581975f // 45 deg
#define ni2Pi    6.283185307179586476925286766558 // 360 deg
#define ni2Pif   6.283185307179586476925286766558f // 360 deg
#define ni1ByPi  0.31830988618379067153776752674503 // (1.0f/niPi)
#define ni1ByPif 0.31830988618379067153776752674503f // (1.0f/niPi)
#define niE      2.71828182845904523536
#define niEf     2.71828182845904523536f

#define niRad(x) ((x)*(niPi/180))
#define niDeg(x) ((x)*(180/niPi))
#define niRadf(x) (((float)(x))*(niPif/180.0f))
#define niDegf(x) (((float)(x))*(180.0f/niPif))

#if !defined niShaderLanguage
niConstValue tF32 kfE = niEf;
niConstValue tF32 kPi = niPif;
niConstValue tF32 kPi2 = niPi2f;
niConstValue tF32 kPi4 = niPi4f;
niConstValue tF32 k2Pi = ni2Pif;
niConstValue tF32 k1ByPi = ni1ByPif;
niConstValue tF32 kInfinity = niMaxF32;
niConstValue tF32 kEpsilon = niEpsilon5;
#endif

#ifndef __cplusplus
#  define T niDefaultTypeT
#endif // __cplusplus
#ifdef niTypeF64
niDefaultTemplate static inline T Num(const tF64 a) { return (T)a; }
#endif
niDefaultTemplate static inline T Abs(const T a) {  return (a < ((T)(0))) ? -a : a; }
niDefaultTemplate static inline T Rad(const T a) {  return (T)niRad(a); }
niDefaultTemplate static inline T Deg(const T a) {  return (T)niDeg(a); }
niDefaultTemplate static inline T Floor(const T a) {  return (T)floor(a); }
niDefaultTemplate static inline T Ceil(const T a) { return (T)ceil(a);  }
niDefaultTemplate static inline T Sqrt(const T a) { return (T)sqrt(a);  }
niDefaultTemplate static inline T Sqr(const T a) {  return a*a; }
niDefaultTemplate static inline T Cube(const T a) { return a*a*a; }
niDefaultTemplate static inline T Sin(const T a) {  return (T)sin(a); }
niDefaultTemplate static inline T Sinh(const T a) { return (T)sinh(a);  }
niDefaultTemplate static inline T ASin(const T a) { return (T)asin(a);  }
niDefaultTemplate static inline T Cos(const T a) {  return (T)cos(a); }
niDefaultTemplate static inline T Cosh(const T a) { return (T)cosh(a);  }
niDefaultTemplate static inline T ACos(const T a) { return (T)acos(a);  }
niDefaultTemplate static inline T Tan(const T a) {  return (T)tan(a); }
niDefaultTemplate static inline T Tanh(const T a) { return (T)tanh(a);  }
niDefaultTemplate static inline T ATan(const T a) { return (T)atan(a);  }
niDefaultTemplate static inline T ATan2(const T a, const T b) { return (T)atan2(a,b); }
niDefaultTemplate static inline T FMod(const T a, const T b) {  return (T)fmod(a,b);  }
#ifdef __cplusplus
template<> inline tI32 FMod(const tI32 a, const tI32 b) {
  return a % b;
}
template<> inline tU32 FMod(const tU32 a, const tU32 b) {
  return a % b;
}
template<> inline tI64 FMod(const tI64 a, const tI64 b) {
  return a % b;
}
template<> inline tU64 FMod(const tU64 a, const tU64 b) {
  return a % b;
}
#  ifdef niTypeIntIsOtherType
template<> inline int FMod(const tInt a, const tInt b) {
  return a % b;
}
#  endif
#endif
niDefaultTemplate static inline T Pow(const T a, const T b) { return (T)pow(a,b); }
niDefaultTemplate static inline T Exp(const T a) {  return (T)exp(a); }
niDefaultTemplate static inline T LogE(const T a) { return (T)log(a); }
niDefaultTemplate static inline T LogX(const T base, const T a) { return (T)log(a)/(T)log(base);  }
niDefaultTemplate static inline T Log10(const T a) {  return (T)log10(a); }
niDefaultTemplate static inline T Log2(const T a) { return (T)log(a)/(T)log((T)2);  }
niDefaultTemplate static inline T Hypot(const T a, const T b) { return Sqrt(a*a + b*b); }
niDefaultTemplate static inline T Sign(T a) { return (a < (T)(0)) ? (T)(-1) : (T)(1); }
#ifdef __cplusplus
template<typename T, typename S> inline T Lerp(const T& a, const T& b, const S fac) { return a + ((b-a) * fac); }
#else
niDefaultTemplate static inline T Lerp(const T a, const T b, const T fac) { return a + ((b-a) * fac); }
#endif
niDefaultTemplate static inline T Pi()  { return (T)(niPi); }
niDefaultTemplate static inline T PiTwo() { return (T)(2*niPi); }
niDefaultTemplate static inline T PiHalf(){ return (T)(niPi/2); }
#ifndef __cplusplus
#  undef T
#endif // __cplusplus

#ifdef __cplusplus
///////////////////////////////////////////////
template <typename T>
inline T FInvert(const T v) {
#  ifdef niSafeFloatDiv
  return (v == T(0)) ? T(1) : T(1)/v;
#  else
  return T(1)/v;
#  endif
}

///////////////////////////////////////////////
template <typename T, typename S>
inline T FDiv(const T n, const S d) {
#  ifdef niSafeFloatDiv
  return (d == T(0)) ? T(1) : n/T(d);
#  else
  return n/T(d);
#  endif
}
#else // __cplusplus
///////////////////////////////////////////////
static inline niDefaultTypeT FInvert(const niDefaultTypeT v) {
#  ifdef niSafeFloatDiv
  return (v == niCCast(niDefaultTypeT,0)) ? niCCast(niDefaultTypeT,1) : niCCast(niDefaultTypeT,1)/v;
#  else
  return niCCast(niDefaultTypeT,1)/v;
#  endif
}

///////////////////////////////////////////////
static inline niDefaultTypeT FDiv(const niDefaultTypeT n, const niDefaultTypeT d) {
#  ifdef niSafeFloatDiv
  return (d == niCCast(niDefaultTypeT,0)) ? niCCast(niDefaultTypeT,1) : n/niCCast(niDefaultTypeT,d);
#  else
  return n/niCCast(niDefaultTypeT,d);
#  endif
}
#endif // __cplusplus

#define niBuildColor(r,g,b,a)                                     \
  ((((a)&0xFF)<<24)|(((r)&0xFF)<<16)|(((g)&0xFF)<<8)|((b)&0xFF))

static inline tU32 ULColorBuild(tU32 r, tU32 g, tU32 b, tU32 a niDefaultParam(0)) { return ((a&0xFF)<<24) | ((r&0xFF)<<16) | ((g&0xFF)<<8) | (b&0xFF);  }
static inline tU32 ULColorBuildf(tF32 r, tF32 g, tF32 b, tF32 a niDefaultParam(0.0f)) {  return (((tU32)(a*255.0f)&0xFF)<<24) | (((tU32)(r*255.0f)&0xFF)<<16) | (((tU32)(g*255.0f)&0xFF)<<8) | ((tU32)(b*255.0f)&0xFF); }

static inline tU8  ULColorGetA(tU32 color) {  return (tU8)((color>>24)&0xFF); }
static inline tU8  ULColorGetR(tU32 color) {  return (tU8)((color>>16)&0xFF); }
static inline tU8  ULColorGetG(tU32 color) {  return (tU8)((color>>8)&0xFF);  }
static inline tU8  ULColorGetB(tU32 color) {  return (tU8)((color)&0xFF);   }
static inline tF32 ULColorGetAf(tU32 color) { return (tF32)((color>>24)&0xFF)/255.0f; }
static inline tF32 ULColorGetRf(tU32 color) { return (tF32)((color>>16)&0xFF)/255.0f; }
static inline tF32 ULColorGetGf(tU32 color) { return (tF32)((color>>8)&0xFF)/255.0f;  }
static inline tF32 ULColorGetBf(tU32 color) { return (tF32)((color)&0xFF)/255.0f;   }
static inline tU32 ULColorSetR(tU32 anColor, tU8 anV) { return ULColorBuild(anV,ULColorGetG(anColor),ULColorGetB(anColor),ULColorGetA(anColor));  }
static inline tU32 ULColorSetG(tU32 anColor, tU8 anV) { return ULColorBuild(ULColorGetR(anColor),anV,ULColorGetB(anColor),ULColorGetA(anColor));  }
static inline tU32 ULColorSetB(tU32 anColor, tU8 anV) { return ULColorBuild(ULColorGetR(anColor),ULColorGetG(anColor),anV,ULColorGetA(anColor));  }
static inline tU32 ULColorSetA(tU32 anColor, tU8 anV) { return ULColorBuild(ULColorGetR(anColor),ULColorGetG(anColor),ULColorGetB(anColor),anV);  }
static inline tU32 ULColorSetRf(tU32 anColor, tF32 anV) { return ULColorBuild(((tU32)(anV*255.0f)&0xFF),ULColorGetG(anColor),ULColorGetB(anColor),ULColorGetA(anColor));  }
static inline tU32 ULColorSetGf(tU32 anColor, tF32 anV) { return ULColorBuild(ULColorGetR(anColor),((tU32)(anV*255.0f)&0xFF),ULColorGetB(anColor),ULColorGetA(anColor));  }
static inline tU32 ULColorSetBf(tU32 anColor, tF32 anV) { return ULColorBuild(ULColorGetR(anColor),ULColorGetG(anColor),((tU32)(anV*255.0f)&0xFF),ULColorGetA(anColor));  }
static inline tU32 ULColorSetAf(tU32 anColor, tF32 anV) { return ULColorBuild(ULColorGetR(anColor),ULColorGetG(anColor),ULColorGetB(anColor),((tU32)(anV*255.0f)&0xFF));  }
static inline tU32 ULColorLerp(tU32 ca, tU32 cb, tF32 fac) {
  tF32 aa = (tF32)((ca>>24)&0xFF),  ba = (tF32)((cb>>24)&0xFF);
  tF32 ar = (tF32)((ca>>16)&0xFF),  br = (tF32)((cb>>16)&0xFF);
  tF32 ag = (tF32)((ca>>8)&0xFF), bg = (tF32)((cb>>8)&0xFF);
  tF32 ab = (tF32)(ca&0xFF),    bb = (tF32)(cb&0xFF);
  return  ((tU8)((aa + ((ba-aa) * fac)))<<24) |
      ((tU8)((ar + ((br-ar) * fac)))<<16) |
      ((tU8)((ag + ((bg-ag) * fac)))<<8)  |
      ((tU8)((ab + ((bb-ab) * fac))));
}

#if defined niShaderLanguage

typedef metal::packed_float2 float2;
typedef metal::packed_float2 sVec2f;
#  define _v2f metal::float2
niCAssert(sizeof(sVec2f) == 8);

typedef metal::packed_float3 float3;
typedef metal::packed_float3 sVec3f;
#  define _v3f metal::float3
niCAssert(sizeof(sVec3f) == 12);

typedef metal::packed_float4 float4;
typedef metal::packed_float4 sVec4f;
#  define _v4f metal::float4
niCAssert(sizeof(sVec4f) == 16);

typedef metal::float4x4 float4x4;
typedef metal::float4x4 sMatrixf;
#  define _mtx metal::float4x4
niCAssert(sizeof(sMatrixf) == 64);

#elif defined __cplusplus

template <typename T>
inline T WrapFloat(T aX, T aMin, T aMax)
{
  const T r = (aMax-aMin);
  const T t = (aX-aMin) / r;
  return aMin + r * (t-ni::Floor(t));
}

template <typename T>
inline T WrapInt(T aX, T aMin, T aMax)
{
  if (aX < aMin) {
    const T range = aMax-aMin;
    aX -= aMin;
    return ((aX%range) + range) + aMin;
  }
  else if (aX >= aMax)  {
    const T range = aMax-aMin;
    aX -= aMin;
    return (aX%range) + aMin;
  }
  return aX;
}

template <typename T>
inline T WrapRad(T aX)
{
  return WrapFloat(aX, T(0), T(ni2Pi));
}

template <typename T>
inline T WrapRad2(T aX)
{
  return WrapFloat(aX, T(-niPif), T(niPif));
}

template <typename T>
int VectorCmp(const T* A, const T* B, const int SIZE) {
  for (int i = 0; i < SIZE; ++i) {
    if (A[i] < B[i]) return -1;
    if (A[i] > B[i]) return 1;
  }
  return 0;
}
template <typename T>
bool VectorEq(const T* A, const T* B, const int SIZE) {
  return memcmp((void*)A,(void*)B,sizeof(T)*SIZE) ==0;
}

template <class T> struct sVec2;
template <class T> struct sVec3;
template <class T> struct sVec4;
template <class T> struct sMatrix;

template <class T> struct sVec2
{
  T x, y;

  inline void Set(T _x, T _y = 0) {
    x = _x; y = _y;
  }
  inline void Set(const T* apV) {
    x = apV[0];
    y = apV[1];
  }

  inline tSize    size() const { return 2; }

  inline     T* ptr()       { return &x; }
  inline const T* ptr() const { return &x; }
  inline T& operator [] (int idx) { return ptr()[idx]; }
  inline T operator [] (int idx) const { return ptr()[idx]; }

  // complex numbers
  inline      T&  re()      { return x; };
  inline const  T&  re() const  { return x; };
  inline      T&  im()      { return y; }
  inline const  T&  im() const    { return y; }
  inline      T&  E()     { return x; };
  inline const  T&  E() const { return x; };
  inline      T&  N()     { return y; }
  inline const  T&  N() const   { return y; }

  inline sVec4<T> operator [] (const cchar* idx) const;

  inline bool operator < (const sVec2<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),2) < 0;
  }
  inline bool operator <= (const sVec2<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),2) <= 0;
  }
  inline bool operator > (const sVec2<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),2) > 0;
  }
  inline bool operator >= (const sVec2<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),2) >= 0;
  }
  inline bool operator == (const sVec2<T>& aR) const {
    return VectorEq<T>(this->ptr(),aR.ptr(),2);
  }
  inline bool operator != (const sVec2<T>& aR) const {
    return !VectorEq<T>(this->ptr(),aR.ptr(),2);
  }

  inline sVec2 operator + () const {
    return *this;
  }
  inline sVec2 operator - () const {
    sVec2<T> r;
    r.x = -this->x;
    r.y = -this->y;
    return r;
  }

  inline sVec2 & operator += (const sVec2<T> & v) {
    this->x += v.x; this->y += v.y;
    return *this;
  }
  inline sVec2 & operator += (T s) {
    this->x += s; this->y += s;
    return *this;
  }
  inline sVec2 operator + (const sVec2<T> & v) const {
    sVec2 r = *this;
    r.x += v.x;
    r.y += v.y;
    return r;
  }
  inline sVec2 operator + (T s) const {
    sVec2 r = *this;
    r.x += s;
    r.y += s;
    return r;
  }

  inline sVec2 & operator -= (const sVec2<T> & v) {
    x -= v.x; y -= v.y; return *this;
  }
  inline sVec2 & operator -= (T s) {
    x -= s; y -= s; return *this;
  }
  inline sVec2 operator - (const sVec2<T> & v) const {
    sVec2 r = *this;
    r.x -= v.x;
    r.y -= v.y;
    return r;
  }
  inline sVec2 operator - (T s) const {
    sVec2 r = *this;
    r.x -= s;
    r.y -= s;
    return r;
  }

  inline sVec2 & operator *= (const sVec2<T> & v) {
    x *= v.x; y *= v.y; return *this;
  }
  inline sVec2 & operator *= (T s) {
    x *= s; y *= s; return *this;
  }
  inline sVec2 operator * (const sVec2<T> & v) const {
    sVec2 r = *this;
    r.x *= v.x;
    r.y *= v.y;
    return r;
  }
  inline sVec2 operator * (T s) const {
    sVec2 r = *this;
    r.x *= s;
    r.y *= s;
    return r;
  }

  inline sVec2 & operator /= (const sVec2<T> & v) {
    x /= v.x; y /= v.y; return *this;
  }
  inline sVec2 & operator /= (T s) {
    x /= s; y /= s; return *this;
  }
  inline sVec2 operator / (const sVec2<T> & v) const {
    sVec2 r = *this;
    r.x /= v.x;
    r.y /= v.y;
    return r;
  }
  inline sVec2 operator / (T s) const {
    sVec2 r = *this;
    r.x /= s;
    r.y /= s;
    return r;
  }

  static inline const sVec2& Zero()  { static sVec2 _v = { (T)ni::Num<T>(0),(T)ni::Num<T>(0) }; return _v; }
  static inline const sVec2& One()   { static sVec2 _v = { ni::Num<T>(1),ni::Num<T>(1) }; return _v; }
  static inline const sVec2& Epsilon() { static sVec2 _v = One()*Num<T>(1e-5f); return _v; }
  static inline const sVec2& XAxis() { static sVec2 _v = { ni::Num<T>(1),ni::Num<T>(0) }; return _v; }
  static inline const sVec2& YAxis() { static sVec2 _v = { ni::Num<T>(0),ni::Num<T>(1) }; return _v; }
  static inline const sVec2& OpXAxis() { static sVec2 _v = { ni::Num<T>(-1),ni::Num<T>(0) }; return _v; }
  static inline const sVec2& OpYAxis() { static sVec2 _v = { ni::Num<T>(0),ni::Num<T>(-1) }; return _v; }
};

template <class T> struct sVec3
{
  T x, y, z;

  inline void Set(T _x, T _y = 0, T _z = 0) {
    x = _x; y = _y; z = _z;
  }
  inline void Set(const T* apV) {
    x = apV[0]; y = apV[1]; z = apV[2];
  }

  inline tSize    size() const { return 3; }

  inline     T* ptr()       { return &x; }
  inline const T* ptr() const { return &x; }

  inline       T& operator [] (int idx) { return ptr()[idx]; }
  inline const T& operator [] (int idx) const { return ptr()[idx]; }
  inline sVec4<T> operator [] (const cchar* idx) const;

  inline bool operator < (const sVec3<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),3) < 0;
  }
  inline bool operator <= (const sVec3<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),3) <= 0;
  }
  inline bool operator > (const sVec3<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),3) > 0;
  }
  inline bool operator >= (const sVec3<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),3) >= 0;
  }
  inline bool operator == (const sVec3<T>& aR) const {
    return VectorEq<T>(this->ptr(),aR.ptr(),3);
  }
  inline bool operator != (const sVec3<T>& aR) const {
    return !VectorEq<T>(this->ptr(),aR.ptr(),3);
  }

  inline sVec3 operator + () const {
    return *this;
  }
  inline sVec3 operator - () const {
    sVec3 v = *this;
    v.x = -x; v.y = -y; v.z = -z;
    return v;
  }

  inline sVec3 & operator += (const sVec3<T> & v) {
    x += v.x; y += v.y; z += v.z; return *this;
  }
  inline sVec3 & operator += (T s) {
    x += s; y += s; z += s; return *this;
  }
  inline sVec3 operator + (const sVec3<T> & v) const {
    sVec3 r = *this;
    r.x += v.x;
    r.y += v.y;
    r.z += v.z;
    return r;
  }
  inline sVec3 operator + (T s) const {
    sVec3 r = *this;
    r.x += s;
    r.y += s;
    r.z += s;
    return r;
  }

  inline sVec3 & operator -= (const sVec3<T> & v) {
    x -= v.x; y -= v.y; z -= v.z; return *this;
  }
  inline sVec3 & operator -= (T s) {
    x -= s; y -= s; z -= s; return *this;
  }
  inline sVec3 operator - (const sVec3<T> & v) const {
    sVec3 r = *this;
    r.x -= v.x;
    r.y -= v.y;
    r.z -= v.z;
    return r;
  }
  inline sVec3 operator - (T s) const {
    sVec3 r = *this;
    r.x -= s;
    r.y -= s;
    r.z -= s;
    return r;
  }

  inline sVec3 & operator *= (const sVec3<T> & v) {
    x *= v.x; y *= v.y; z *= v.z; return *this;
  }
  inline sVec3 & operator *= (T s) {
    x *= s; y *= s; z *= s; return *this;
  }
  inline sVec3 operator * (const sVec3<T> & v) const {
    sVec3 r = *this;
    r.x *= v.x;
    r.y *= v.y;
    r.z *= v.z;
    return r;
  }
  inline sVec3 operator * (T s) const {
    sVec3 r = *this;
    r.x *= s;
    r.y *= s;
    r.z *= s;
    return r;
  }

  inline sVec3 & operator /= (const sVec3<T> & v) {
    x /= v.x; y /= v.y; z /= v.z; return *this;
  }
  inline sVec3 & operator /= (T s) {
    x /= s; y /= s; z /= s; return *this;
  }
  inline sVec3 operator / (const sVec3<T> & v) const {
    sVec3 r = *this;
    r.x /= v.x;
    r.y /= v.y;
    r.z /= v.z;
    return r;
  }
  inline sVec3 operator / (T s) const {
    sVec3 r = *this;
    r.x /= s;
    r.y /= s;
    r.z /= s;
    return r;
  }

  static inline const sVec3& Zero()    { static sVec3 _v = { Num<T>(0),Num<T>(0),Num<T>(0) }; return _v; }
  static inline const sVec3& One()     { static sVec3 _v = { Num<T>(1),Num<T>(1),Num<T>(1)}; return _v; }
  static inline const sVec3& Epsilon() { static sVec3 _v = One()*Num<T>(1e-5f); return _v; }
  static inline const sVec3& XAxis()   { static sVec3 _v = { Num<T>(1),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec3& YAxis()   { static sVec3 _v = { Num<T>(0),Num<T>(1),Num<T>(0 )}; return _v; }
  static inline const sVec3& ZAxis()   { static sVec3 _v = { Num<T>(0),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec3& OpXAxis() { static sVec3 _v = { Num<T>(-1),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec3& OpYAxis() { static sVec3 _v = { Num<T>(0),Num<T>(-1),Num<T>(0) }; return _v; }
  static inline const sVec3& OpZAxis() { static sVec3 _v = { Num<T>(0),Num<T>(0),Num<T>(-1) }; return _v; }
  static inline const sVec3& Black()   { static sVec3 _v = { Num<T>(0),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec3& White()   { static sVec3 _v = { Num<T>(1),Num<T>(1),Num<T>(1 )}; return _v; }
  static inline const sVec3& Red()     { static sVec3 _v = { Num<T>(1),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec3& Green()   { static sVec3 _v = { Num<T>(0),Num<T>(1),Num<T>(0 )}; return _v; }
  static inline const sVec3& Blue()    { static sVec3 _v = { Num<T>(0),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec3& Yellow()  { static sVec3 _v = { Num<T>(1),Num<T>(1),Num<T>(0 )}; return _v; }
  static inline const sVec3& Pink()    { static sVec3 _v = { Num<T>(1),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec3& Aqua()    { static sVec3 _v = { Num<T>(0),Num<T>(1),Num<T>(1 )}; return _v; }
  static inline const sVec3& Orange()  { static sVec3 _v = { Num<T>(1),Num<T>(0.5),Num<T>(0 )}; return _v; }
  static inline const sVec3& Gray()    { static sVec3 _v = { Num<T>(0.5),Num<T>(0.5),Num<T>(0.5 )}; return _v; }
};

template <class T>
struct sVec4
{
  T x, y, z, w;     // 4D-vector

  inline void Set(T aX, T aY = 0, T aZ = 0, T aW = 0) { x = aX; y = aY; z = aZ; w = aW; }
  inline void Set(const T* apV) { x = apV[0]; y = apV[1]; z = apV[2]; w = apV[3]; }

  inline tSize    size() const { return 4; }

  inline     T* ptr()       { return &x; }
  inline const T* ptr() const { return &x; }

  inline       T& operator [] (int idx) { return ptr()[idx]; }
  inline const T& operator [] (int idx) const { return ptr()[idx]; }

  // rectangle sides
  inline      T&  Left()      { return x; };
  inline const  T&  Left() const  { return x; };
  inline      T&  Top()     { return y; }
  inline const  T&  Top() const   { return y; }
  inline      T&  Right()     { return z; }
  inline const  T&  Right() const { return z; }
  inline      T&  Bottom()    { return w; }
  inline const  T&  Bottom() const  { return w; }

  // Rect corners / line
  inline      T&  x1()    { return x; }
  inline const  T&  x1() const  { return x; }
  inline      T&  y1()    { return y; }
  inline const  T&  y1() const  { return y; }
  inline      T&  x2()    { return z; }
  inline const  T&  x2() const  { return z; }
  inline      T&  y2()    { return w; }
  inline const  T&  y2() const  { return w; }

  // Plane
  inline      T&  A()     { return x; }
  inline const  T&  A() const { return x; }
  inline      T&  B()     { return y; }
  inline const  T&  B() const { return y; }
  inline      T&  C()     { return z; }
  inline const  T&  C() const { return z; }
  inline      T&  D()     { return w; }
  inline const  T&  D() const { return w; }

  // Plane - Normal/Distance
  inline void SetNormal(const sVec3<T>& v) { x = v.x; y = v.y; z = v.z; }
  inline sVec3<T> GetNormal() const { sVec3<T> v = {x,y,z}; return v; }
  inline void SetDist(T v) { w = v; }
  inline T GetDist() const { return w; }

  // Sphere
  inline void SetPosition(const sVec3<T>& v) { x = v.x; y = v.y; z = v.z; }
  inline sVec3<T> GetPosition() const { sVec3<T> v = {x,y,z}; return v; }
  inline void SetRadius(T v) { w = v; }
  inline T GetRadius() const { return w; }

  // Quat/DVecs
  inline      T&  ew()    { return x; }
  inline const  T&  ew() const  { return x; }
  inline      T&  ex()    { return y; }
  inline const  T&  ex() const  { return y; }
  inline      T&  ey()    { return z; }
  inline const  T&  ey() const  { return z; }
  inline      T&  ez()    { return w; }
  inline const  T&  ez() const  { return w; }

  // Matrix
  inline      T&  _11()   { return x; }
  inline const  T&  _11() const { return x; }
  inline      T&  _12()   { return y; }
  inline const  T&  _12() const { return y; }
  inline      T&  _21()   { return z; }
  inline const  T&  _21() const { return z; }
  inline      T&  _22()   { return w; }
  inline const  T&  _22() const { return w; }

  inline sVec4<T> operator [] (const cchar* idx) const;

  inline bool operator < (const sVec4<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),4) < 0;
  }
  inline bool operator <= (const sVec4<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),4) <= 0;
  }
  inline bool operator > (const sVec4<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),4) > 0;
  }
  inline bool operator >= (const sVec4<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),4) >= 0;
  }
  inline bool operator == (const sVec4<T>& aR) const {
    return VectorEq<T>(this->ptr(),aR.ptr(),4);
  }
  inline bool operator != (const sVec4<T>& aR) const {
    return !VectorEq<T>(this->ptr(),aR.ptr(),4);
  }

  inline sVec4 operator + () const {
    return *this;
  }
  inline sVec4 operator - () const {
    sVec4 r;
    r.x = -this->x;
    r.y = -this->y;
    r.z = -this->z;
    r.w = -this->w;
    return r;
  }

  inline sVec4 & operator += (const sVec4<T> & v) {
    x += v.x; y += v.y; z += v.z; w += v.w; return *this;
  }
  inline sVec4 & operator += (T s) {
    x += s; y += s; z += s; w += s; return *this;
  }
  inline sVec4 operator + (const sVec4<T> & v) const {
    sVec4 r = *this;
    r.x += v.x;
    r.y += v.y;
    r.z += v.z;
    r.w += v.w;
    return r;
  }
  inline sVec4 operator + (T s) const {
    sVec4 r = *this;
    r.x += s;
    r.y += s;
    r.z += s;
    r.w += s;
    return r;
  }

  inline sVec4 & operator -= (const sVec4<T> & v) {
    x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this;
  }
  inline sVec4 & operator -= (T s) {
    x -= s; y -= s; z -= s; w -= s; return *this;
  }
  inline sVec4 operator - (const sVec4<T> & v) const {
    sVec4 r = *this;
    r.x -= v.x;
    r.y -= v.y;
    r.z -= v.z;
    r.w -= v.w;
    return r;
  }
  inline sVec4 operator - (T s) const {
    sVec4 r = *this;
    r.x -= s;
    r.y -= s;
    r.z -= s;
    r.w -= s;
    return r;
  }

  inline sVec4 & operator *= (const sVec4<T> & v) {
    x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this;
  }
  inline sVec4 & operator *= (T s) {
    x *= s; y *= s; z *= s; w *= s; return *this;
  }
  inline sVec4 operator * (const sVec4<T> & v) const {
    sVec4 r = *this;
    r.x *= v.x;
    r.y *= v.y;
    r.z *= v.z;
    r.w *= v.w;
    return r;
  }
  inline sVec4 operator * (T s) const {
    sVec4 r = *this;
    r.x *= s;
    r.y *= s;
    r.z *= s;
    r.w *= s;
    return r;
  }

  inline sVec4 & operator /= (const sVec4<T> & v) {
    x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this;
  }
  inline sVec4 & operator /= (T s) {
    x /= s; y /= s; z /= s; w /= s; return *this;
  }
  inline sVec4 operator / (const sVec4<T> & v) const {
    sVec4 r = *this;
    r.x /= v.x;
    r.y /= v.y;
    r.z /= v.z;
    r.w /= v.w;
    return r;
  }
  inline sVec4 operator / (T s) const {
    sVec4 r = *this;
    r.x /= s;
    r.y /= s;
    r.z /= s;
    r.w /= s;
    return r;
  }

  static inline const sVec4& Zero()         { static sVec4 _v = { Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec4& One()          { static sVec4 _v = { Num<T>(1),Num<T>(1),Num<T>(1),Num<T>(1 )}; return _v; }
  static inline const sVec4& Epsilon()      { static sVec4 _v = One()*Num<T>(1e-5f); return _v; }
  static inline const sVec4& XAxis()        { static sVec4 _v = { Num<T>(1),Num<T>(0),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec4& YAxis()        { static sVec4 _v = { Num<T>(0),Num<T>(1),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec4& ZAxis()        { static sVec4 _v = { Num<T>(0),Num<T>(0),Num<T>(1),Num<T>(0 )}; return _v; }
  static inline const sVec4& WAxis()        { static sVec4 _v = { Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec4& OpXAxis()      { static sVec4 _v = { Num<T>(-1),Num<T>(0),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec4& OpYAxis()      { static sVec4 _v = { Num<T>(0),Num<T>(-1),Num<T>(0),Num<T>(0 )}; return _v; }
  static inline const sVec4& OpZAxis()      { static sVec4 _v = { Num<T>(0),Num<T>(0),Num<T>(-1),Num<T>(0 )}; return _v; }
  static inline const sVec4& OpWAxis()      { static sVec4 _v = { Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(-1) }; return _v; }
  static inline const sVec4& Black()        { static sVec4 _v = { Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec4& White()        { static sVec4 _v = { Num<T>(1),Num<T>(1),Num<T>(1),Num<T>(1 )}; return _v; }
  static inline const sVec4& Red()          { static sVec4 _v = { Num<T>(1),Num<T>(0),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec4& Green()        { static sVec4 _v = { Num<T>(0),Num<T>(1),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec4& Blue()         { static sVec4 _v = { Num<T>(0),Num<T>(0),Num<T>(1),Num<T>(1 )}; return _v; }
  static inline const sVec4& Yellow()       { static sVec4 _v = { Num<T>(1),Num<T>(1),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec4& Pink()         { static sVec4 _v = { Num<T>(1),Num<T>(0),Num<T>(1),Num<T>(1 )}; return _v; }
  static inline const sVec4& Aqua()         { static sVec4 _v = { Num<T>(0),Num<T>(1),Num<T>(1),Num<T>(1 )}; return _v; }
  static inline const sVec4& Orange()       { static sVec4 _v = { Num<T>(1),Num<T>(0.5),Num<T>(0),Num<T>(1 )}; return _v; }
  static inline const sVec4& Gray()         { static sVec4 _v = { Num<T>(0.5),Num<T>(0.5),Num<T>(0.5),Num<T>(1 )}; return _v; }
  static inline const sVec4& QuatIdentity() { static sVec4 _v = { Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(1 )}; return _v; }
};

#  ifdef __cplusplus
inline tU32 ULColorBuild(const sVec4<tF32>& aCol) {  return ULColorBuildf(aCol.x, aCol.y, aCol.z, aCol.w); }
inline tU32 ULColorBuild(const sVec3<tF32>& aCol, tF32 a = 0.0f) {  return ULColorBuildf(aCol.x, aCol.y, aCol.z, a); }
inline tU32 ULColorBuild(const sVec4<tI32>& aCol) {  return ULColorBuild(aCol.x, aCol.y, aCol.z, aCol.w); }
inline tU32 ULColorBuild(const sVec3<tI32>& aCol, tI32 a = 0) {  return ULColorBuild(aCol.x, aCol.y, aCol.z, a); }
inline tU32 ULColorBuild(const sVec4<tU8>& aCol) {  return ULColorBuild(aCol.x, aCol.y, aCol.z, aCol.w); }
inline tU32 ULColorBuild(const sVec3<tU8>& aCol, tU8 a = 0) {  return ULColorBuild(aCol.x, aCol.y, aCol.z, a); }
#  endif // __cplusplus

///////////////////////////////////////////////
template <typename T>
inline sVec2<T> Vec2(T _x, T _y = 0) { sVec2<T> v; v.Set(_x,_y); return v; }
template <typename T>
inline sVec2<T> Vec2(const T* aV) { sVec2<T> v;  v.Set(aV); return v; }
template <typename T>
inline sVec2<T> Vec2(const sVec3<T>& aV) { sVec2<T> v; v.Set(aV.x,aV.y); return v; }
template <typename T>
inline sVec2<T> Vec2(const sVec4<T>& aV) { sVec2<T> v; v.Set(aV.x,aV.y); return v; }
template <class T>
inline sVec2<T> operator * (T s, const sVec2<T> &v)
{
  sVec2<T> r = v;
  r.x *= s; r.y *= s;
  return r;
}

///////////////////////////////////////////////
template <typename T>
inline sVec3<T> Vec3(T _x, T _y = 0, T _z = 0) { sVec3<T> v; v.Set(_x,_y,_z); return v; }
template <typename T>
inline sVec3<T> Vec3(const T* aV) { sVec3<T> v;  v.Set(aV); return v; }
template <typename T>
inline sVec3<T> Vec3(const sVec2<T>& aV) { sVec3<T> v; v.Set(aV.x,aV.y,0); return v; }
template <typename T>
inline sVec3<T> Vec3(const sVec4<T>& aV) { sVec3<T> v; v.Set(aV.x,aV.y,aV.z); return v; }
template <typename T>
inline sVec3<T> Vec3(tU32 anColor) { sVec3<T> v; v.Set(ULColorGetRf(anColor),ULColorGetGf(anColor),ULColorGetBf(anColor)); return v; }

template <class T>
inline sVec3<T> operator * (T s, const sVec3<T> &v) {
  sVec3<T> r = v;
  r.x *= s; r.y *= s; r.z *= s;
  return r;
}

///////////////////////////////////////////////
template <typename T>
inline sVec4<T> Vec4(T _x, T _y = 0, T _z = 0, T _w = 0) { sVec4<T> v; v.Set(_x,_y,_z,_w); return v; }
template <typename T>
inline sVec4<T> Vec4(const T* aV) { sVec4<T> v;  v.Set(aV); return v; }
template <typename T>
inline sVec4<T> Vec4(const sVec2<T>& aV) { sVec4<T> v; v.Set(aV.x,aV.y,0,0); return v; }
template <typename T>
inline sVec4<T> Vec4(const sVec3<T>& aV) { sVec4<T> v; v.Set(aV.x,aV.y,aV.z,0); return v; }
template <typename T>
inline sVec4<T> Vec4(tU32 anColor) { sVec4<T> v; v.Set(ULColorGetRf(anColor),ULColorGetGf(anColor),ULColorGetBf(anColor),ULColorGetAf(anColor)); return v; }
template <class T>
inline sVec4<T> operator * (T s, const sVec4<T> &v)
{
  sVec4<T> r = v;
  r.x *= s; r.y *= s; r.z *= s; r.w *= s;
  return r;
}

typedef sVec2<tF32> sVec2f;
typedef sVec2<tF64> sVec2d;
typedef sVec2<tI32> sVec2i;
typedef sVec3<tF32> sVec3f;
typedef sVec3<tF64> sVec3d;
typedef sVec3<tI32> sVec3i;
typedef sVec3<tF32> sColor3f;  // [0.0, 1.0]
typedef sVec3<tF64> sColor3d;  // [0.0, 1.0]
typedef sVec3<tU8>  sColor3ub; // [0, 255]
typedef sVec3<tU16> sColor3us; // [0, 65535]
typedef sVec3<tU32> sColor3ul; // [0, 4billion]
typedef sVec4<tF32> sVec4f;
typedef sVec4<tF64> sVec4d;
typedef sVec4<tI32> sVec4i;
typedef sVec4<tF32> sColor4f;    // [0.0, 1.0]
typedef sVec4<tF64> sColor4d;    // [0.0, 1.0]
typedef sVec4<tU8>  sColor4ub; // [0, 255]
typedef sVec4<tU16> sColor4us; // [0, 65535]
typedef sVec4<tU32> sColor4ul; // [0, 4billion]
typedef sVec4<tF32> sPlanef;
typedef sVec4<tF64> sPlaned;
typedef sVec4<tF32> sSpheref;
typedef sVec4<tF64> sSphered;

typedef sVec2f float2;
typedef sVec3f float3;
typedef sVec4f float4;

typedef sVec2d double2;
typedef sVec3d double3;
typedef sVec4d double4;

typedef sVec2i int2;
typedef sVec3i int3;
typedef sVec4i int4;

/// Vec2f ///
static inline sVec2f Vec2f(tF32 x = 0, tF32 y = 0) {
  return Vec2(x,y);
}
static inline sVec2f Vec2f(tF32* v) {
  return Vec2(v[0],v[1]);
}
static inline sVec2f Vec2f(const sVec2i& v) {
  return Vec2((tF32)v[0],(tF32)v[1]);
}
static inline sVec2f Vec2f(tI32* v) {
  return Vec2((tF32)v[0],(tF32)v[1]);
}
#  define _v2f ni::Vec2f

/// Vec2i ///
static inline sVec2i Vec2i(tI32 x = 0, tI32 y = 0) {
  return Vec2(x,y);
}
static inline sVec2i Vec2i(tI32* v) {
  return Vec2(v[0],v[1]);
}
static inline sVec2i Vec2i(const sVec2f& v) {
  return Vec2((tI32)v[0],(tI32)v[1]);
}
static inline sVec2i Vec2i(tF32* v) {
  return Vec2((tI32)v[0],(tI32)v[1]);
}
#  define _v2l ni::Vec2i

/// Vec3f ///
static inline sVec3f Vec3f(tF32 x = 0, tF32 y = 0, tF32 z = 0) {
  return Vec3(x,y,z);
}
static inline sVec3f Vec3f(const tF32* v) {
  return Vec3(v[0],v[1],v[2]);
}
static inline sVec3f Vec3f(const sVec3i& v) {
  return Vec3((tF32)v[0],(tF32)v[1],(tF32)v[2]);
}
static inline sVec3f Vec3f(const tI32* v) {
  return Vec3((tF32)v[0],(tF32)v[1],(tF32)v[2]);
}
#  define _v3f ni::Vec3f

/// Vec3i ///
static inline sVec3i Vec3i(tI32 x = 0, tI32 y = 0, tI32 z = 0) {
  return Vec3(x,y,z);
}
static inline sVec3i Vec3i(const tI32* v) {
  return Vec3(v[0],v[1],v[2]);
}
static inline sVec3i Vec3i(const sVec3f& v) {
  return Vec3((tI32)v[0],(tI32)v[1],(tI32)v[2]);
}
static inline sVec3i Vec3i(const tF32* v) {
  return Vec3((tI32)v[0],(tI32)v[1],(tI32)v[2]);
}
#  define _v3l ni::Vec3i

/// Vec4f ///
static inline sVec4f Vec4f(tF32 x = 0, tF32 y = 0, tF32 z = 0, tF32 w = 0) {
  return Vec4(x,y,z,w);
}
static inline sVec4f Vec4f(const tF32* v) {
  return Vec4(v[0],v[1],v[2],v[3]);
}
static inline sVec4f Vec4f(const sVec4i& v) {
  return Vec4((tF32)v[0],(tF32)v[1],(tF32)v[2],(tF32)v[3]);
}
static inline sVec4f Vec4f(const tI32* v) {
  return Vec4((tF32)v[0],(tF32)v[1],(tF32)v[2],(tF32)v[3]);
}
#  define _v4f ni::Vec4f

/// Vec4i ///
static inline sVec4i Vec4i(tI32 x = 0, tI32 y = 0, tI32 z = 0, tI32 w = 0) {
  return Vec4(x,y,z,w);
}
static inline sVec4i Vec4i(const tI32* v) {
  return Vec4(v[0],v[1],v[2],v[3]);
}
static inline sVec4i Vec4i(const sVec4f& v) {
  return Vec4((tI32)v[0],(tI32)v[1],(tI32)v[2],(tI32)v[3]);
}
static inline sVec4i Vec4i(const tF32* v) {
  return Vec4((tI32)v[0],(tI32)v[1],(tI32)v[2],(tI32)v[3]);
}
#  define _v4l ni::Vec4i

//// Vec2 swizzling operator ////
template <typename T> inline sVec4<T> sVec2<T>::operator [] (const cchar* idx) const {
  tU32 i = 0;
  T last = Num<T>(0);
  sVec4<T> r = sVec4<T>::Zero();
  while (i < 4 && idx[i]) {
    switch (idx[i]) {
      case 'r': case 'R': case 'x': case 'X': r[i] = last = this->x; break;
      case 'g': case 'G': case 'y': case 'Y': r[i] = last = this->y; break;
      default: r[i] = last = Num<T>(0); break;
    }
    ++i;
  }
  while (i < 4) { r[i] = last; ++i; }
  return r;
}

//// Vec3 swizzling operator ////
template <typename T> inline sVec4<T> sVec3<T>::operator [] (const cchar* idx) const {
  tU32 i = 0;
  T last = Num<T>(0);
  sVec4<T> r = sVec4<T>::Zero();
  while (i < 4 && idx[i]) {
    switch (idx[i]) {
      case 'r': case 'R': case 'x': case 'X': r[i] = last = this->x; break;
      case 'g': case 'G': case 'y': case 'Y': r[i] = last = this->y; break;
      case 'b': case 'B': case 'z': case 'Z': r[i] = last = this->z; break;
      default: r[i] = last = Num<T>(0); break;
    }
    ++i;
  }
  while (i < 4) { r[i] = last; ++i; }
  return r;
}

//// Vec4 swizzling operator ////
template <typename T> inline sVec4<T> sVec4<T>::operator [] (const cchar* idx) const {
  tU32 i = 0;
  T last = Num<T>(0);
  sVec4<T> r = sVec4<T>::Zero();
  while (i < 4 && idx[i]) {
    switch (idx[i]) {
      case 'r': case 'R': case 'x': case 'X': r[i] = last = this->x; break;
      case 'g': case 'G': case 'y': case 'Y': r[i] = last = this->y; break;
      case 'b': case 'B': case 'z': case 'Z': r[i] = last = this->z; break;
      case 'a': case 'A': case 'w': case 'W': r[i] = last = this->w; break;
      default: r[i] = last = Num<T>(0); break;
    }
    ++i;
  }
  while (i < 4) { r[i] = last; ++i; }
  return r;
}

//! Quat
template <class T> struct sQuat : public sVec4<T>
{
  inline sQuat() {
  }
  inline sQuat(const sVec4<T> & q) {
    this->x = q.x; this->y = q.y; this->z = q.z; this->w = q.w;
  }
  inline explicit sQuat(T _x, T _y = 0, T _z = 0, T _w = 1) {
    this->x = _x; this->y = _y; this->z = _z; this->w = _w;
  }
  inline explicit sQuat(const T *apEl) {
    this->Set(apEl);
  }

  inline sQuat & operator += (const sVec4<T> & q) {
    this->x += q.x; this->y += q.y; this->z += q.z; this->w += q.w;
    return *this;
  }
  inline sQuat & operator -= (const sVec4<T> & q) {
    this->x -= q.x; this->y -= q.y; this->z -= q.z; this->w -= q.w;
    return *this;
  }
  inline sQuat & operator *= (const sVec4<T> & q) {
    *this = this->Mul(q);
    return *this;
  }
  inline sQuat & operator *= (T s) {
    this->x *= s; this->y *= s; this->z *= s; this->w *= s;
    return *this;
  }
  inline sQuat & operator /= (T s) {
    this->x /= s; this->y /= s; this->z /= s; this->w /= s;
    return *this;
  }

  inline sQuat operator + () const {
    return *this;
  }
  inline sQuat operator - () const {
    return sQuat<T>(-this->x, -this->y, -this->z, -this->w);
  }

  inline sQuat operator + (const sVec4<T> & q) const {
    return sQuat<T>(this->x + q.x, this->y + q.y, this->z + q.z, this->w + q.w);
  }
  inline sQuat operator - (const sVec4<T> & q) const {
    return sQuat<T>(this->x - q.x, this->y - q.y, this->z - q.z, this->w - q.w);
  }
  inline sQuat operator * (const sVec4<T> & q) const {
    return this->Mul(q);
  }
  inline sQuat operator * (T s) const {
    return sQuat<T>(this->x * s, this->y * s, this->z * s, this->w * s);
  }
  inline sQuat operator / (T s) const {
    return sQuat<T>(this->x / s, this->y / s, this->z / s, this->w / s);
  }

  static inline const sQuat& Zero() { static sQuat _v(0,0,0,0); return _v; }
  static inline const sQuat& Identity() { static sQuat _v(0,0,0,1); return _v; }

  inline sQuat<T> Mul(const sVec4<T>& Q2) const {
    sQuat<T> Out;
    T q1x, q1y, q1z, q1w;
    T q2x, q2y, q2z, q2w;
    q1x = this->x; q1y = this->y; q1z = this->z; q1w = this->w;
    q2x = Q2.x; q2y = Q2.y; q2z = Q2.z; q2w = Q2.w;

    //Out.x = q1w * q2x + q1x * q2w - q1y * q2z + q1z * q2y;
    //Out.y = q1w * q2y + q1y * q2w - q1z * q2x + q1x * q2z;
    //Out.z = q1w * q2z + q1z * q2w - q1x * q2y + q1y * q2x;
    //Out.w = q1w * q2w - q1x * q2x - q1y * q2y - q1z * q2z;

    Out.x = q2w * q1x + q2x * q1w + q2y * q1z - q2z * q1y;
    Out.y = q2w * q1y + q2y * q1w + q2z * q1x - q2x * q1z;
    Out.z = q2w * q1z + q2z * q1w + q2x * q1y - q2y * q1x;
    Out.w = q2w * q1w - q2x * q1x - q2y * q1y - q2z * q1z;
    return Out;
  }
};

template <class T>
inline sQuat<T> operator * (T s, const sQuat<T> & q)
{
  return sQuat<T>(q.x * s, q.y * s, q.z * s, q.w * s);
}

//! F32 Quat typedef
typedef sQuat<tF32> sQuatf;
//! F64 Quat typedef
typedef sQuat<tF64> sQuatd;

//! 4x4 Matrix template
template <class T> struct sMatrix
{
  //! data
  T _11, _12, _13, _14;
  T _21, _22, _23, _24;
  T _31, _32, _33, _34;
  T _41, _42, _43, _44;

  inline void Set(T e11, T e12, T e13, T e14,
                  T e21, T e22, T e23, T e24,
                  T e31, T e32, T e33, T e34,
                  T e41, T e42, T e43, T e44)
  {
    this->_11 = e11; this->_12 = e12; this->_13 = e13; this->_14 = e14;
    this->_21 = e21; this->_22 = e22; this->_23 = e23; this->_24 = e24;
    this->_31 = e31; this->_32 = e32; this->_33 = e33; this->_34 = e34;
    this->_41 = e41; this->_42 = e42; this->_43 = e43; this->_44 = e44;
  }

  inline void Set(const T* apEl) {
    T* v = ptr();
    niLoop(i,size()) {
      v[i] = apEl[i];
    }
  }

  inline tSize    size() const { return 16; }

  inline     T* ptr()       { return &_11; }
  inline const T* ptr() const { return &_11; }

  inline T& operator () (int row, int col) {
    return this->ptr()[row*4+col];
  }
  inline T operator () (int row, int col) const {
    return this->ptr()[row*4+col];
  }

  inline T & operator [] (int idx) {
    return this->ptr()[idx];
  }
  inline T operator [] (int idx) const {
    return this->ptr()[idx];
  }

  inline sMatrix<T> & operator *= (const sMatrix<T> & matB) {
    return _Mul(*this,*this,matB);
  }

  inline sMatrix<T> & operator += (const sMatrix<T> & matB) {
    this->_11 += matB._11; this->_12 += matB._12; this->_13 += matB._13; this->_14 += matB._14;
    this->_21 += matB._21; this->_22 += matB._22; this->_23 += matB._23; this->_24 += matB._24;
    this->_31 += matB._31; this->_32 += matB._32; this->_33 += matB._33; this->_34 += matB._34;
    this->_41 += matB._41; this->_42 += matB._42; this->_43 += matB._43; this->_44 += matB._44;
    return *this;
  }

  inline sMatrix<T> & operator -= (const sMatrix<T> & matB) {
    this->_11 -= matB._11; this->_12 -= matB._12; this->_13 -= matB._13; this->_14 -= matB._14;
    this->_21 -= matB._21; this->_22 -= matB._22; this->_23 -= matB._23; this->_24 -= matB._24;
    this->_31 -= matB._31; this->_32 -= matB._32; this->_33 -= matB._33; this->_34 -= matB._34;
    this->_41 -= matB._41; this->_42 -= matB._42; this->_43 -= matB._43; this->_44 -= matB._44;
    return *this;
  }

  inline sMatrix<T> & operator *= (T s) {
    this->_11 *= s; this->_12 *= s; this->_13 *= s; this->_14 *= s;
    this->_21 *= s; this->_22 *= s; this->_23 *= s; this->_24 *= s;
    this->_31 *= s; this->_32 *= s; this->_33 *= s; this->_34 *= s;
    this->_41 *= s; this->_42 *= s; this->_43 *= s; this->_44 *= s;
    return *this;
  }

  inline sMatrix<T> & operator /= (T s) {
    this->_11 /= s; this->_12 /= s; this->_13 /= s; this->_14 /= s;
    this->_21 /= s; this->_22 /= s; this->_23 /= s; this->_24 /= s;
    this->_31 /= s; this->_32 /= s; this->_33 /= s; this->_34 /= s;
    this->_41 /= s; this->_42 /= s; this->_43 /= s; this->_44 /= s;
    return *this;
  }

  inline sMatrix<T> operator + () const {
    return *this;
  }

  inline sMatrix<T> operator - () const {
    sMatrix<T> m;
    m.Set(-this->_11, -this->_12, -this->_13, -this->_14,
          -this->_21, -this->_22, -this->_23, -this->_24,
          -this->_31, -this->_32, -this->_33, -this->_34,
          -this->_41, -this->_42, -this->_43, -this->_44);
    return m;
  }

  inline sMatrix<T> operator * (const sMatrix<T> & matB) const {
    sMatrix<T> r;
    return this->_Mul(r,*this,matB);
  }

  inline sMatrix<T> operator + (const sMatrix<T> & matB) const {
    sMatrix<T> m;
    m.Set(
        this->_11 + matB._11, this->_12 + matB._12, this->_13 + matB._13, this->_14 + matB._14,
        this->_21 + matB._21, this->_22 + matB._22, this->_23 + matB._23, this->_24 + matB._24,
        this->_31 + matB._31, this->_32 + matB._32, this->_33 + matB._33, this->_34 + matB._34,
        this->_41 + matB._41, this->_42 + matB._42, this->_43 + matB._43, this->_44 + matB._44);
    return m;
  }

  inline sMatrix<T> operator - (const sMatrix<T> & matB) const {
    sMatrix<T> m;
    m.Set(
        this->_11 - matB._11, this->_12 - matB._12, this->_13 - matB._13, this->_14 - matB._14,
        this->_21 - matB._21, this->_22 - matB._22, this->_23 - matB._23, this->_24 - matB._24,
        this->_31 - matB._31, this->_32 - matB._32, this->_33 - matB._33, this->_34 - matB._34,
        this->_41 - matB._41, this->_42 - matB._42, this->_43 - matB._43, this->_44 - matB._44);
    return m;
  }

  inline sMatrix<T> operator * (T s) const {
    sMatrix<T> m;
    m.Set(
        this->_11 * s, this->_12 * s, this->_13 * s, this->_14 * s,
        this->_21 * s, this->_22 * s, this->_23 * s, this->_24 * s,
        this->_31 * s, this->_32 * s, this->_33 * s, this->_34 * s,
        this->_41 * s, this->_42 * s, this->_43 * s, this->_44 * s);
    return m;
  }

  inline sMatrix<T> operator / (T s) const {
    sMatrix<T> m;
    m.Set(
        this->_11 / s, this->_12 / s, this->_13 / s, this->_14 / s,
        this->_21 / s, this->_22 / s, this->_23 / s, this->_24 / s,
        this->_31 / s, this->_32 / s, this->_33 / s, this->_34 / s,
        this->_41 / s, this->_42 / s, this->_43 / s, this->_44 / s);
    return m;
  }

  inline bool operator < (const sMatrix<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),16) < 0;
  }
  inline bool operator <= (const sMatrix<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),16) <= 0;
  }
  inline bool operator > (const sMatrix<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),16) > 0;
  }
  inline bool operator >= (const sMatrix<T>& aR) const {
    return VectorCmp<T>(this->ptr(),aR.ptr(),16) >= 0;
  }
  inline bool operator == (const sMatrix<T>& aR) const {
    return VectorEq<T>(this->ptr(),aR.ptr(),16);
  }
  inline bool operator != (const sMatrix<T>& aR) const {
    return !VectorEq<T>(this->ptr(),aR.ptr(),16);
  }

  static inline const sMatrix& One() {
    static sMatrix _v = {
      Num<T>(1),Num<T>(1),Num<T>(1),Num<T>(1),
      Num<T>(1),Num<T>(1),Num<T>(1),Num<T>(1),
      Num<T>(1),Num<T>(1),Num<T>(1),Num<T>(1),
      Num<T>(1),Num<T>(1),Num<T>(1),Num<T>(1)
    };
    return _v;
  }
  static inline const sMatrix& Zero() {
    static sMatrix _v = {
      Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(0),
      Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(0),
      Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(0),
      Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(0)
    };
    return _v;
  }
  static inline const sMatrix& Identity() {
    static sMatrix _v = {
      Num<T>(1),Num<T>(0),Num<T>(0),Num<T>(0),
      Num<T>(0),Num<T>(1),Num<T>(0),Num<T>(0),
      Num<T>(0),Num<T>(0),Num<T>(1),Num<T>(0),
      Num<T>(0),Num<T>(0),Num<T>(0),Num<T>(1)
    };
    return _v;
  }
  static inline const sMatrix& Hermite() {
    static sMatrix _v = {
      Num<T>(2),Num<T>(-2),Num<T>(1),Num<T>(1),
      Num<T>(-3),Num<T>(3),Num<T>(-2),Num<T>(-1),
      Num<T>(0),Num<T>(0),Num<T>(1),Num<T>(0),
      Num<T>(1),Num<T>(0),Num<T>(0),Num<T>(0)
    };
    return _v;
  }
  static inline const sMatrix& Epsilon() { static sMatrix _v = One()*Num<T>(1e-5); return _v; }

  static sMatrix<T>& _Mul(sMatrix<T>& Out, const sMatrix<T>& A, const sMatrix<T>& B) {
    T a1, a2, a3, a4;
    sMatrix<T> R;

    // 1st row * 1st column
    a1 = A._11; a2 = A._12; a3 = A._13; a4 = A._14;
    R._11 = a1 * B._11 + a2 * B._21 + a3 * B._31 + a4 * B._41;
    R._12 = a1 * B._12 + a2 * B._22 + a3 * B._32 + a4 * B._42;
    R._13 = a1 * B._13 + a2 * B._23 + a3 * B._33 + a4 * B._43;
    R._14 = a1 * B._14 + a2 * B._24 + a3 * B._34 + a4 * B._44;

    // 2nd row * 2nd column
    a1 = A._21; a2 = A._22; a3 = A._23; a4 = A._24;
    R._21 = a1 * B._11 + a2 * B._21 + a3 * B._31 + a4 * B._41;
    R._22 = a1 * B._12 + a2 * B._22 + a3 * B._32 + a4 * B._42;
    R._23 = a1 * B._13 + a2 * B._23 + a3 * B._33 + a4 * B._43;
    R._24 = a1 * B._14 + a2 * B._24 + a3 * B._34 + a4 * B._44;

    // 3rd row * 3rd column
    a1 = A._31; a2 = A._32; a3 = A._33; a4 = A._34;
    R._31 = a1 * B._11 + a2 * B._21 + a3 * B._31 + a4 * B._41;
    R._32 = a1 * B._12 + a2 * B._22 + a3 * B._32 + a4 * B._42;
    R._33 = a1 * B._13 + a2 * B._23 + a3 * B._33 + a4 * B._43;
    R._34 = a1 * B._14 + a2 * B._24 + a3 * B._34 + a4 * B._44;

    // 4th row * 4th column
    a1 = A._41; a2 = A._42; a3 = A._43; a4 = A._44;
    R._41 = a1 * B._11 + a2 * B._21 + a3 * B._31 + a4 * B._41;
    R._42 = a1 * B._12 + a2 * B._22 + a3 * B._32 + a4 * B._42;
    R._43 = a1 * B._13 + a2 * B._23 + a3 * B._33 + a4 * B._43;
    R._44 = a1 * B._14 + a2 * B._24 + a3 * B._34 + a4 * B._44;

    Out = R;
    return Out;
  }

  sMatrix<T> Mul(const sMatrix<T>& M2) const {
    sMatrix<T> r;
    return this->_Mul(r,*this,M2);
  }
};

template <typename T>
inline sMatrix<T> operator * (T s, const sMatrix<T> & mat)
{
  return sMatrix<T>(
      mat._11 * s, mat._12 * s, mat._13 * s, mat._14 * s,
      mat._21 * s, mat._22 * s, mat._23 * s, mat._24 * s,
      mat._31 * s, mat._32 * s, mat._33 * s, mat._34 * s,
      mat._41 * s, mat._42 * s, mat._43 * s, mat._44 * s);
}

template <typename T>
inline sMatrix<T> Matrix(
    T e11, T e12 = 0, T e13 = 0, T e14 = 0,
    T e21 = 0, T e22 = 1, T e23 = 0, T e24 = 0,
    T e31 = 0, T e32 = 0, T e33 = 1, T e34 = 0,
    T e41 = 0, T e42 = 0, T e43 = 0, T e44 = 1)
{
  sMatrix<T> m;
  m._11 = e11; m._12 = e12; m._13 = e13; m._14 = e14;
  m._21 = e21; m._22 = e22; m._23 = e23; m._24 = e24;
  m._31 = e31; m._32 = e32; m._33 = e33; m._34 = e34;
  m._41 = e41; m._42 = e42; m._43 = e43; m._44 = e44;
  return m;
}
template <typename T>
inline sMatrix<T> Matrix(const T apEl[16]) {
  sMatrix<T> m;
  m.Set(apEl);
  return m;
}

typedef sMatrix<tF32> sMatrixf;
typedef sMatrix<tF64> sMatrixd;

typedef sMatrixf float4x4;
typedef sMatrixf double4x4;

#else // __cplusplus

typedef struct sVec2f_ {
  tF32 x, y;
} sVec2f;
typedef struct sVec2i_ {
  tI32 x, y;
} sVec2i;
typedef struct sVec3f_ {
  tF32 x, y, z;
} sVec3f;
typedef struct sVec3i_ {
  tI32 x, y, z;
} sVec3i;
typedef struct sVec4f_ {
  tF32 x, y, z, w;
} sVec4f;
typedef struct sVec4i_ {
  tI32 x, y, z, w;
} sVec4i;

//! 4x4 Matrix template
typedef struct sMatrixf_
{
  //! data
  tF32 _11, _12, _13, _14;
  tF32 _21, _22, _23, _24;
  tF32 _31, _32, _33, _34;
  tF32 _41, _42, _43, _44;
} sMatrixf;

#endif // __cplusplus

#ifdef __cplusplus
static inline sVec4f ULColorToVec4f(tU32 anColor) {
  return _v4f(
      ULColorGetRf(anColor),
      ULColorGetGf(anColor),
      ULColorGetBf(anColor),
      ULColorGetAf(anColor));
}
#endif

#ifndef niShaderLanguage
niConstValue sVec2f  kvec2fZero = { 0, 0 };
niConstValue sVec2i  kvec2lZero = { 0, 0 };
niConstValue sVec3f  kvec3fZero = { 0, 0, 0 };
niConstValue sVec3i  kvec3lZero = { 0, 0, 0 };
niConstValue sVec4f  kvec4fZero = { 0, 0, 0, 0 };
niConstValue sVec4i  kvec4lZero = { 0, 0, 0, 0 };
niConstValue sMatrixf kmtxfZero = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
niConstValue sMatrixf kmtxfIdentity = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
#endif

//! Plane type.
typedef enum ePlaneType
{
  ePlaneType_X = 0,
  ePlaneType_YZ = ePlaneType_X,
  ePlaneType_Y = 1,
  ePlaneType_XZ = ePlaneType_Y,
  ePlaneType_Z = 2,
  ePlaneType_XY = ePlaneType_Z,
  ePlaneType_NonAxial = 3,
} ePlaneType;

//! Quat slerp mode
typedef enum eQuatSlerp
{
  //! The rotation will be less than 180 degrees, default.
  eQuatSlerp_Short = 0,
  //! Rotation will be greater than 180 degrees.
  eQuatSlerp_Long = 1,
  //! No correction is applied to garantee either a short or long path when doing the slerp.
  eQuatSlerp_NoCorrection = 2,
} eQuatSlerp;

//! Rect corners.
typedef enum eRectCorners
{
  //! Top left rectangle corner.
  eRectCorners_TopLeft = niBit(0),
  //! Top right rectangle corner.
  eRectCorners_TopRight = niBit(1),
  //! Bottom right rectangle corner.
  eRectCorners_BottomRight = niBit(2),
  //! Bottom left rectangle corner.
  eRectCorners_BottomLeft = niBit(3),
  //! Top rectangle corners.
  eRectCorners_Top = eRectCorners_TopLeft|eRectCorners_TopRight,
  //! Right rectangle corners.
  eRectCorners_Right = eRectCorners_TopRight|eRectCorners_BottomRight,
  //! Bottom rectangle corners.
  eRectCorners_Bottom = eRectCorners_BottomRight|eRectCorners_BottomLeft,
  //! Left rectangle corners,
  eRectCorners_Left = eRectCorners_TopLeft|eRectCorners_BottomLeft,
  //! All rectangle corners.
  eRectCorners_All = eRectCorners_TopLeft|eRectCorners_TopRight|eRectCorners_BottomRight|eRectCorners_BottomLeft,
  //! To force the compiler to put the enumeration in DWORD.
  eRectCorners_ForceDWORD = 0xFFFFFFFF
} eRectCorners;

//! Rect edges.
typedef enum eRectEdges
{
  //! Top rectangle edge.
  eRectEdges_Top = niBit(0),
  //! Right rectangle edge.
  eRectEdges_Right = niBit(1),
  //! Bottom rectangle edge.
  eRectEdges_Bottom = niBit(2),
  //! Left rectangle edge.
  eRectEdges_Left = niBit(3),
  //! Vertical rectangle edges.
  eRectEdges_Vertical = eRectEdges_Left|eRectEdges_Right,
  //! Horizontal rectangle edges,
  eRectEdges_Horizontal = eRectEdges_Top|eRectEdges_Bottom,
  //! All rectangle edges.
  eRectEdges_All = eRectEdges_Vertical|eRectEdges_Horizontal,
  //! \internal
  eRectEdges_ForceDWORD = 0xFFFFFFFF
} eRectEdges;

//! Rect edges flags.
typedef tU32  tRectEdgesFlags;

//! Rect frame flags
enum eRectFrameFlags
{
  //! Top rectangle edge.
  eRectFrameFlags_TopEdge = niBit(0),
  //! Right rectangle edge.
  eRectFrameFlags_RightEdge = niBit(1),
  //! Bottom rectangle edge.
  eRectFrameFlags_BottomEdge = niBit(2),
  //! Left rectangle edge.
  eRectFrameFlags_LeftEdge = niBit(3),
  //! Top left corner.
  eRectFrameFlags_TopLeftCorner = niBit(4),
  //! Top right corner.
  eRectFrameFlags_TopRightCorner = niBit(5),
  //! Top left corner.
  eRectFrameFlags_BottomLeftCorner = niBit(6),
  //! Top left corner.
  eRectFrameFlags_BottomRightCorner = niBit(7),
  //! Center of the rectangle
  eRectFrameFlags_Center = niBit(8),
  //! Top side
  eRectFrameFlags_TopSide = eRectFrameFlags_TopEdge|eRectFrameFlags_TopLeftCorner|eRectFrameFlags_TopRightCorner,
  //! Bottom side
  eRectFrameFlags_BottomSide = eRectFrameFlags_BottomEdge|eRectFrameFlags_BottomLeftCorner|eRectFrameFlags_BottomRightCorner,
  //! Left side
  eRectFrameFlags_LeftSide = eRectFrameFlags_LeftEdge|eRectFrameFlags_TopLeftCorner|eRectFrameFlags_BottomLeftCorner,
  //! Right side
  eRectFrameFlags_RightSide = eRectFrameFlags_RightEdge|eRectFrameFlags_TopRightCorner|eRectFrameFlags_BottomRightCorner,
  //! Vertidcal sides
  eRectFrameFlags_VerticalSides = eRectFrameFlags_LeftSide|eRectFrameFlags_RightSide,
  //! Horizontal sides
  eRectFrameFlags_HorizontalSides = eRectFrameFlags_TopSide|eRectFrameFlags_BottomSide,
  //! All edges
  eRectFrameFlags_Edges = eRectFrameFlags_VerticalSides|eRectFrameFlags_HorizontalSides,
  //! The all frame
  //! \remark Zero mean the all frame aswell
  eRectFrameFlags_All = eRectFrameFlags_VerticalSides|eRectFrameFlags_HorizontalSides|eRectFrameFlags_Center,
  //! \internal
  eRectFrameFlags_ForceDWORD = 0xFFFFFFFF
};

//! Rect frame flags. \see ni::eRectFrameFlags
typedef tU32 tRectFrameFlags;

/**@}*/
#ifdef __cplusplus
}
#endif // __cplusplus

//--------------------------------------------------------------------------------------------
//
//  Infos
//
//--------------------------------------------------------------------------------------------
#if !defined niShaderLanguage
#  ifdef __cplusplus
namespace ni {
#  endif // __cplusplus
/** \addtogroup niLang
 * @{
 */

#  define niCopyright  "(c) 2022 The niLang Authors"
niConstValue achar* const kNiCopyright = niCopyright;

#  define niMakeVersion(a,b,c)      (((a)<<16)|((b)<<8)|(c))
#  define niGetMainVersion(ver)     (((ver)>>16)&0xFFFF)
#  define niGetSubVersion(ver)      (((ver)>>8)&0xFF)
#  define niGetPatchVersion(ver)      ((ver)&0xFF)
//lint -e{773}
#  define niVerStringConcat(ver)      _A("v")<<niGetMainVersion(ver)<<_A(".")<<niGetSubVersion(ver)<<_A(".")<<niGetPatchVersion(ver)
#  define niVerStringf(ver)       "v%d.%d.%d",niGetMainVersion(ver),niGetSubVersion(ver),niGetPatchVersion(ver)
#  define niVerAStringf(ver)        _A("v%d.%d.%d"),niGetMainVersion(ver),niGetSubVersion(ver),niGetPatchVersion(ver)
#  define niMakeVersionString(str,ver)  (str).Format(niVerAStringf(ver))
#  define niFmtVersion(ver)       niFmt(niVerAStringf(ver))

/**@}*/
#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // niShaderLanguage

//--------------------------------------------------------------------------------------------
//
//  Synchronized types
//
//--------------------------------------------------------------------------------------------
#if !defined niShaderLanguage
#  ifdef __cplusplus
namespace ni {
#  endif // __cplusplus
/** \addtogroup niLang
 * @{
 */

#  ifdef __cplusplus

//! Synchronized counter, safe across multiple threads.
struct SyncCounter {
  SyncCounter() : _counter(0) {
  }
  explicit SyncCounter(tSyncInt v) : _counter(v) {
  }
  SyncCounter(const SyncCounter& v) : _counter(v.Get()) {
  }
  __forceinline tSyncInt Inc() {
    return SYNC_INCREMENT(&_counter);
  }
  __forceinline tSyncInt Dec() {
    return SYNC_DECREMENT(&_counter);
  }
  __forceinline void Set(tSyncInt v) {
    SYNC_WRITE(&_counter,v);
  }
  __forceinline tSyncInt Get() const {
    return SYNC_READ(&_counter);
  }

 private:
  tSyncInt _counter;
};

niCAssert(sizeof(SyncCounter) == sizeof(tSyncInt));

#  endif // __cplusplus

/**@}*/
#  ifdef __cplusplus
}
#  endif // __cplusplus
#endif // niShaderLanguage

//--------------------------------------------------------------------------------------------
//
//  Base utilities
//
//--------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace ni {
#endif // __cplusplus
/** \addtogroup niLang
 * @{
 */

#define niCall2(ERRRET,A,B) (A ? (A->B) : ERRRET)
#define niCall3(ERRRET,A,B,C) (A ? (A->B ? (A->B->C) : ERRRET) : ERRRET)
#define niCall4(ERRRET,A,B,C,D) (A ? (A->B ? (A->B->C ? A->B->C->D : ERRRET) : ERRRET) : ERRRET)
#define niCall5(ERRRET,A,B,C,D,E) (A ? (A->B ? (A->B->C ? (A->B->C->D ? A->B->C->D->E : ERRRET) : ERRRET) : ERRRET) : ERRRET)

#define niEnumDef(ENUM)       GetEnumDef_##ENUM()
#define niEnumName(ENUM)      ((ENUM)0,_A(#ENUM))
#define niEnumExpr(ENUM)      niEnumDef(ENUM), 0
#define niEnumExprFull(ENUM)  niEnumDef(ENUM), ni::eEnumToStringFlags_Full
#define niFlagsExpr(ENUM)     niEnumDef(ENUM), ni::eEnumToStringFlags_Flags
#define niFlagsExprFull(ENUM) niEnumDef(ENUM), ni::eEnumToStringFlags_Flags|ni::eEnumToStringFlags_Full

#define niConvTableBegin(KEY)   switch (KEY) {
#define niConvTableItem(FROM,TO)  case FROM: return TO;
#define niConvTableEnd(DEFAULT) default: return DEFAULT; }

#define niFourCC(a,b,c,d) (((d)<<24)|((c)<<16)|((b)<<8)|(a))
#define niFourCCA(id) ((id)&0xff)
#define niFourCCB(id) (((id)>>8)&0xff)
#define niFourCCC(id) (((id)>>16)&0xff)
#define niFourCCD(id) (((id)>>24)&0xff)

#define niFourCCFmt(id)                                                 \
  niFmt(_A("%c%c%c%c"),niFourCCA(id),niFourCCB(id),niFourCCC(id),niFourCCD(id))

#define niBytesPerPixel(bpp)     (((int)(bpp)+7) >> 3)

#define niBitOn(var,x)   ((niValCast(niNamespace(ni,tU32),var)) |= niBit(x))
#define niBitOff(var,x)  ((niValCast(niNamespace(ni,tU32),var)) &= ~((niNamespace(ni,tU32))niBit(x)))
#define niFlagOn(var,x)  ((niValCast(niNamespace(ni,tU32),var)) |= ((niNamespace(ni,tU32))(x)))
#define niFlagOff(var,x) ((niValCast(niNamespace(ni,tU32),var)) &= ~((niNamespace(ni,tU32))(x)))
//! Test a single flag/bit
#define niFlagTest(var,x) (((niNamespace(ni,tU32))(x))&&((((niNamespace(ni,tU32))(var))&((niNamespace(ni,tU32))(x)))==((niNamespace(ni,tU32))(x))))
//! Test a single flag/bit
#define niFlagIs(var,x) niFlagTest(var,x)
//! Test a single flag/bit
#define niFlagIsNot(var,x)  (!niFlagTest(var,x))
#define niFlagOnIf(var,x,cond)  ((cond)?(niFlagOn(var,x)):(niFlagOff(var,x)))
#define niFlagSetOnIf(var,x,cond) ((cond)?(niFlagOn(var,x)):(0))
#define niFlagSetOffIf(var,x,cond)  ((cond)?(niFlagOff(var,x)):(0))

#define niBit64On(var,x)   ((niValCast(niNamespace(ni,tU64),var)) |= niBit64(x))
#define niBit64Off(var,x)  ((niValCast(niNamespace(ni,tU64),var)) &= ~((niNamespace(ni,tU64))niBit64(x)))
#define niFlag64On(var,x)  ((niValCast(niNamespace(ni,tU64),var)) |= ((niNamespace(ni,tU64))(x)))
#define niFlag64Off(var,x) ((niValCast(niNamespace(ni,tU64),var)) &= ~((niNamespace(ni,tU64))(x)))
#define niFlag64Test(var,x) (((niNamespace(ni,tU64))(x))&&((((niNamespace(ni,tU64))(var))&((niNamespace(ni,tU64))(x)))==((niNamespace(ni,tU64))(x))))
#define niFlag64Is(var,x)       niFlag64Test(var,x)
#define niFlag64IsNot(var,x)      (!niFlag64Test(var,x))
#define niFlag64OnIf(var,x,cond)    ((cond)?(niFlag64On(var,x)):(niFlag64Off(var,x)))
#define niFlag64SetOnIf(var,x,cond) ((cond)?(niFlag64On(var,x)):(0))
#define niFlag64SetOffIf(var,x,cond)  ((cond)?(niFlag64Off(var,x)):(0))

#define niFlagsContained(FLAGSA,FLAGSB) (((FLAGSA)&(FLAGSB))||((FLAGSA)==(FLAGSB)))

#ifdef __cplusplus

template <typename T>
inline tBool FlagsTestMask(const T flags, const T excluded, const T required) {
  if (flags&excluded) return niFalse;
  if (!required) return niTrue;
  if (!(flags&required)) return niFalse;
  return niTrue;
}
#  define niFlagsTestMask(FLAGS,EXCL,REQ) ni::FlagsTestMask(FLAGS,EXCL,REQ)

#  define niPropFlagOn(obj,var,x)   obj->Set##var(obj->Get##var()|(x))
#  define niPropFlagOff(obj,var,x)  obj->Set##var(obj->Get##var()&(~(x)))
#  define niPropFlagTest(obj,var,x) ((obj->Get##var()&((ni::tU32)x))==((ni::tU32)x))
#  define niPropFlagIs(obj,var,x)     niPropFlagTest(obj,var,x)
#  define niPropFlagIsNot(obj,var,x)      (!niPropFlagTest(obj,var,x))
#  define niPropFlagOnIf(obj,var,x,cond)    ((cond)?(niPropFlagOn(obj,var,x)):(niPropFlagOff(obj,var,x)))
#  define niPropFlagSetOnIf(obj,var,x,cond) ((cond)?(niPropFlagOn(obj,var,x)):(0))
#  define niPropFlagSetOffIf(obj,var,x,cond)  ((cond)?(niPropFlagOff(obj,var,x)):(0))

#endif // __cplusplus

#if !defined niShaderLanguage

#  ifdef __cplusplus
///////////////////////////////////////////////
template <class T>
inline void MoveArray(T* dst, T* src, tSize count)
{
  // Copying forward or backwards?
  if (dst < src) {
    for (tSize i = 0; i < count; ++i, ++dst, ++src) {
      *dst = *src;
    }
  }
  else {
    T*  s = src + count - 1;
    T*  d = dst + count - 1;
    for (tSize i = 0; i < count; ++i, --d, --s) {
      *d = *s;
    }
  }
}

///////////////////////////////////////////////
template <class T, class S>
inline void CopyArray(T* dst, const S* src, tSize count)
{
  for (tSize i = 0; i < count; ++i, ++dst, ++src) {
    *dst = (T)(*src);
  }
}

///////////////////////////////////////////////
template <class T>
inline void FillArray(T* dst, const T& src, tSize count)
{
  for (tSize i = 0; i < count; ++i, ++dst) {
    *dst = src;
  }
}

///////////////////////////////////////////////
template <class T>
inline void ZeroArray(T* dst, tSize count)
{
  FillArray(dst,T(0),count);
}
#  endif // __cplusplus

///////////////////////////////////////////////
static inline tInt MemCmp(tPtr apA, tPtr apB, tSize anSize) {
  return niGlobalNamespace(memcmp)((void*)apA,(void*)apB,anSize);
}

///////////////////////////////////////////////
static inline void MemSet(tPtr apDest, tU8 aValue, tSize anSize) {
  niGlobalNamespace(memset)((void*)apDest,aValue,anSize);
}

///////////////////////////////////////////////
static inline void MemCopy(tPtr apDest, tPtr apSrc, tSize anSize) {
  niGlobalNamespace(memcpy)((void*)apDest,(void*)apSrc,anSize);
}

///////////////////////////////////////////////
static inline void MemMove(tPtr apDest, tPtr apSrc, tSize anSize) {
  niGlobalNamespace(memmove)((void*)apDest,(void*)apSrc,anSize);
}

///////////////////////////////////////////////
static inline void MemZero(tPtr apDest, tSize anSize) {
  MemSet((tPtr)apDest,0,anSize);
}

#endif // niShaderLanguage

#ifdef __cplusplus

#  define niZeroMember(NAME) ni::MemZero((ni::tPtr)&NAME,sizeof(NAME))

///////////////////////////////////////////////
template <typename T>
int CmpByVal(const T a, const T b) {
  return ((int)(a < b)) ? -1 : ((int)(a > b));
}

///////////////////////////////////////////////
template <typename T>
int CmpByRef(const T& a, const T& b) {
  return ((int)(a < b)) ? -1 : ((int)(a > b));
}

///////////////////////////////////////////////
template<typename T,typename S,typename R>
inline T Clamp(T v, const S min, const R max) {
  if (v < min)
    v = (T)min;
  if (v > max)
    v = (T)max;
  return v;
}

///////////////////////////////////////////////
template<typename T>
inline T ClampZeroOne(T v) {
  if(v < T(0))  v = T(0);
  if(v > T(1))  v = T(1);
  return v;
}

///////////////////////////////////////////////
template<typename T, typename S>
inline T Min(const T a, const S b) {
  return (a < T(b)) ? a : T(b);
}

///////////////////////////////////////////////
template<typename T, typename S>
inline T Max(const T a, const S b) {
  return (a > T(b)) ? a : T(b);
}

///////////////////////////////////////////////
template<typename T, typename S, typename R>
inline T Max(T a, S b, R c) {
  return a > b ? (a > c ? a : c) : (b > c ? b : c);
}

///////////////////////////////////////////////
template<typename T, typename S, typename R>
inline T Min(T a, S b, R c) {
  return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

///////////////////////////////////////////////
template<typename T>
inline void Swap(T& a, T& b) {
  T c = a; a = b; b = c;
}

///////////////////////////////////////////////
template<typename T>
inline tBool FuzzyEqual(T x, T y, T e) {
  return (x-e)<y && y<(x+e);
}

///////////////////////////////////////////////
//! Blends new values into an accumulator to produce a smoothed time series.
//!
//! \param accumulator is the current value of the accumulator
//! \param newValue is the new value to accumulate
//! \param smoothRate typically made proportional to "frameTime". If
//!            smoothRate is 0 the accumulator will not change, if smoothRate
//!            is 1 the accumulator is set to the new value (no smoothing),
//!            useful values are "near zero".
//! \return the new smoothed value of the accumulator
template<typename T, typename S>
inline T BlendIntoAccumulator(const T& accumulator, const T& newValue, const S smoothRate) {
  return Lerp(accumulator,newValue,Clamp(smoothRate,(S)0,(S)1));
}

#  define niClamp(v,min,max)  ni::Min(v,min,max)
#  define niClampZeroOne(v) ni::ClampZeroOne(v)
#  define niMin(a,b)      ni::Min(a,b)
#  define niMax(a,b)      ni::Max(a,b)
#  define niMax3(a,b,c)   ni::Max(a,b,c)
#  define niMin3(a,b,c)   ni::Min(a,b,c)
#  define niSwap(TYPE,a,b)  ni::Swap(a,b)
#  define niFuzzyEqual(x,y,e) ni::FuzzyEqual(x,y,e)

#else // __cplusplus
#  define niClamp(v,min,max)  (((v)<(min))?(min):(((v)>(max))?(max):(v)))
#  define niClampZeroOne(v) niClamp(v,0,1)
#  define niMin(a,b)      (((a)<(b))?(a):(b))
#  define niMax(a,b)      (((a)>(b))?(a):(b))
#  define niMid(a,b,c)    niMax(a,niMin(b,c))
#  define niMax3(a,b,c)   ((a) > (b) ? ((a) > (c) ? (a) : (c)) : ((b) > (c) ? (b) : (c)))
#  define niMin3(a,b,c)   ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
#  define niSwap(TYPE,a,b)  { TYPE c = a; a = b; b = c; }
#  define niFuzzyEqual(x,y,e) (((x)-(e))<(y) && (y)<((x)+(e)))
#endif // __cplusplus

#if !defined niShaderLanguage
//! Get a string representation of the given type.
//! \remark Requires a buffer of 32 characters minimum.
niExportFunc(const achar*) GetTypeString(achar* aStr, tType aType);
#endif // niShaderLanguage

//--------------------------------------------------------------------------------------------
//
//  Debug
//
//--------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef _REDIST
#  define niDebugFmt(FMT)
#else
#  define niDebugFmt(FMT) {                                             \
    ni::cString debugFmt; debugFmt.Format FMT;                          \
    ni::ni_log(ni::eLogFlags_Debug,__FILE__,__FUNCTION__,__LINE__,debugFmt.Chars()); \
  }
#endif

#ifdef __cplusplus
namespace ni {
#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////////////////////////
// Fixme / Todos / Notes macros

#define niQuoteBase(x)    # x
#define niQuote(x)      niQuoteBase(x)

#define niFileLine __FILE__ "(" niQuote(__LINE__) ") : "

#define niNote(x) message(niFileLine "NOTE : " x "\n")

#define niTodoX(x)  message(niFileLine  "\n"                            \
                            "========================================================\n" \
                            "= TODO : " x "\n"                          \
                            "========================================================\n")

#define niFixmeX(x) message(niFileLine "\n"                             \
                            "========================================================\n" \
                            "= FIXME : " x "\n"                         \
                            "========================================================\n")


#define niTodo(x) message(niFileLine "TODO : " x "\n")
#define niFixme(x)  message(niFileLine "FIXME : " x "\n")

//////////////////////////////////////////////////////////////////////////////////////////////
// Run-time
#define niWarningAssert(COND) if (!(COND)) { niWarning(_A(#COND)); }
#define niWarningAssert_(COND,RET)  if (!(COND)) { niWarning(_A(#COND)); return RET; }

//////////////////////////////////////////////////////////////////////////////////////////////
// General use debugging macros

#ifdef __cplusplus

#  if defined niDebug
#    define niBeginClass(class)  niDeclareDogTag(class##Begin);
#    define niEndClass(class)    niDeclareDogTag(class##End)
#    define niClassIsOK(class)   niCheckDogTag(class##Begin); niCheckDogTag(class##End)
#  else
#    define niBeginClass(class)
#    define niEndClass(class)
#    define niClassIsOK(class)
#  endif

#  define niClassNoCopyAssign(CLASS)            \
  private:                                      \
  CLASS(const CLASS&);                          \
  CLASS& operator = (const CLASS& );

#  define niClassNoHeapAlloc(CLASS)                                     \
  public:                                                               \
  inline void* operator new(size_t anSize, void* apMem) {               \
    niUnused(anSize); return apMem;                                     \
  }                                                                     \
  inline void operator delete(void* apMem, void* apMemDum)  { niAssertUnreachable("No Heap Alloc.");  } \
 private:                                                               \
 void* operator new(size_t anSize)   { niAssertUnreachable("No Heap Alloc."); return (void*)((ni::tIntPtr)ni::eInvalidHandle); } \
 void* operator new[](size_t anSize) { niAssertUnreachable("No Heap Alloc."); return (void*)((ni::tIntPtr)ni::eInvalidHandle); } \
 void* operator new(size_t anSize, const ni::achar *aaszFile, int anLine)  { niAssertUnreachable("No Heap Alloc."); return (void*)((ni::tIntPtr)ni::eInvalidHandle); } \
 void* operator new[](size_t anSize, const ni::achar *aaszFile, int anLine)  { niAssertUnreachable("No Heap Alloc."); return (void*)((ni::tIntPtr)ni::eInvalidHandle); } \
 void  operator delete(void *apMem)  { niAssertUnreachable("No Heap Alloc.");  } \
 void  operator delete[](void *apMem)  { niAssertUnreachable("No Heap Alloc.");  } \
 void  operator delete(void *apMem, const ni::achar *aaszFile, int anLine) { niAssertUnreachable("No Heap Alloc.");  } \
 void  operator delete[](void *apMem, const ni::achar *aaszFile, int anLine) { niAssertUnreachable("No Heap Alloc.");  }

#  define niClassNoDeref(CLASS)                 \
  private:                                      \
  CLASS* operator&();

#  define niClassStrictLocal(CLASS)             \
  niClassNoCopyAssign(CLASS)                    \
  niClassNoHeapAlloc(CLASS)                     \
  niClassNoDeref(CLASS)

#  define niClassLocal(CLASS)                   \
  niClassNoCopyAssign(CLASS)                    \
  niClassNoHeapAlloc(CLASS)

/*!
  Dog tags is a technique that can be used to verify that an object has not been stepped
  on by a memory overrun. To use them you simply use the niDeclareDogTag macro to declare
  a dog tag at the begining and ending of the class definition. After this, you can check
  the validiy of an object by using the niCheckDogTag macro. If the object is not valid
  the program will assert.<br><br>

  For example :<br>
  <pre>
  class cMyClass {
  public:
    niDeclareDogTag(Begin);

    bool IsOK() const;

    void TestFunc();

    int mSomeData;

  private:
    niDeclareDogTag(End);
  }

  bool cMyClass::IsOK() const
  {
    niCheckDogTag(Begin);
    niCheckDogTag(End);
    // OR, use the facility that do exactly the same as the above macros
    niCheckDogTags();
  }
  </pre>
*/
#  ifdef _DEBUG
class cDogTag
{
 public:
  inline cDogTag()          { mThis = this; }
  inline cDogTag(const cDogTag& aTag) { mThis = this; niAssert(aTag.IsOK()); }
  inline ~cDogTag()         { niAssert(IsOK()); mThis = NULL;   }

  inline cDogTag& operator = (const cDogTag& rhs) { niAssert(IsOK() && rhs.IsOK()); return *this; }

  inline bool IsOK() const { return mThis == this; }

 private:
  const cDogTag* mThis;
};

#    define niDeclareDogTag(x)      ni::cDogTag mdogtag##x
#    define niCheckDogTag(x)      niAssert(mdogtag##x.IsOK())
#    define niCheckDogTags()      niCheckDogTag(Begin); niCheckDogTag(End)
#    define niCheckDogTagObj(obj,x) niAssert(obj.mdogtag##x.IsOK())
#    define niCheckDogTagsObj(obj)    niCheckDogTagObj(obj,Begin); niCheckDogTag(obj,End)
#  else
#    define niDeclareDogTag(x)
#    define niCheckDogTag(x)
#    define niCheckDogTags()
#    define niCheckDogTagObj(obj,x)
#    define niCheckDogTagsObj(obj)
#  endif

#endif // __cplusplus

/**@}*/
#ifdef __cplusplus
}
#endif // __cplusplus

//--------------------------------------------------------------------------------------------
//
//  Endianess
//
//--------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace ni {
#endif // __cplusplus
/** \addtogroup niLang
 * @{
 */

#define niSwap16(val) ((niNamespace(ni,tU16))((((niNamespace(ni,tU16))(val))<<8)|(((niNamespace(ni,tU16))(val))>>8)))
#define niSwap24(val) ni::niPrivateSwap24(val)
#define niSwap32(val)                                                   \
  ((niNamespace(ni,tU32))((((niNamespace(ni,tU32))(val))<<24)|          \
                          ((((niNamespace(ni,tU32))(val))<<8)&0x00FF0000)| \
                          ((((niNamespace(ni,tU32))(val))>>8)&0x0000FF00)| \
                          (((niNamespace(ni,tU32))(val))>>24)))
#define niSwap48(val) ni::niPrivateSwap48((niNamespace(ni,tU64))(val))
#define niSwap64(val) ni::niPrivateSwap64((niNamespace(ni,tU64))(val))

static inline tU64 niPrivateSwap48(tU64 val) {
  tU32 hi;
  tU16 lo;
  lo = (tU16)(val&0xFFFF);
  val >>= 16;
  hi = (tU32)(val&0xFFFFFFFF);
  val = niSwap16(lo);
  val <<= 32;
  val |= niSwap32(hi);
  return val;
}
static inline tU32 niPrivateSwap24(tU32 val) {
  tU16 hi;
  tU8 lo;
  lo = (tU8)(val&0xFF);
  val >>= 8;
  hi = (tU16)(val&0xFFFF);
  val = lo;
  val <<= 16;
  val |= niSwap16(hi);
  return val;
}
static inline tU64 niPrivateSwap64(tU64 val) {
  tU32 hi;
  tU32 lo;
  lo = (tU32)(val&0xFFFFFFFF);
  val >>= 32;
  hi = (tU32)(val&0xFFFFFFFF);
  val = niSwap32(lo);
  val <<= 32;
  val |= niSwap32(hi);
  return val;
}

#ifdef niLittleEndian
#  if !defined niShaderLanguage
niConstValue tBool kbIsBigEndian = niFalse;
niConstValue tBool kbIsLittleEndian = niTrue;
#  endif
#  define niSwapLE16(x) (x)
#  define niSwapLE24(x) (x)
#  define niSwapLE32(x) (x)
#  define niSwapLE48(x) (x)
#  define niSwapLE64(x) (x)
#  define niSwapBE16(x) niSwap16(x)
#  define niSwapBE24(x) niSwap24(x)
#  define niSwapBE32(x) niSwap32(x)
#  define niSwapBE48(x) niSwap48(x)
#  define niSwapBE64(x) niSwap64(x)
#elif defined niBigEndian
#  if !defined niShaderLanguage
niConstValue tBool kbIsBigEndian = niTrue;
niConstValue tBool kbIsLittleEndian = niFalse;
#  endif
#  define niSwapBE16(x) (x)
#  define niSwapBE24(x) (x)
#  define niSwapBE32(x) (x)
#  define niSwapBE48(x) (x)
#  define niSwapBE64(x) (x)
#  define niSwapLE16(x) niSwap16(x)
#  define niSwapLE24(x) niSwap24(x)
#  define niSwapLE32(x) niSwap32(x)
#  define niSwapLE48(x) niSwap48(x)
#  define niSwapLE64(x) niSwap64(x)
#else
#  pragma message("E/C++ Preprocessor: Endianess not defined.")
#endif

/**@}*/
#ifdef __cplusplus
}
#endif // __cplusplus

//--------------------------------------------------------------------------------------------
//
//  IUnknown
//
//--------------------------------------------------------------------------------------------
#if !defined niShaderLanguage
#  ifdef __cplusplus
namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Unknown class.
class cUnknown  {
};
//! Unknown 0 class.
class cUnknown0 {
};
//! Unknown 1 class.
class cUnknown1 {
};
//! Unknown 2 class.
class cUnknown2 {
};
//! Unknown 3 class.
class cUnknown3 {
};
//! Unknown 4 class.
class cUnknown4 {
};

struct iHString;
struct iMutableCollection;

#    define niDeclareInterfaceID_(NAME)                                   \
  inline static const ni::achar* __stdcall _GetInterfaceID() {return _A(#NAME); }

#    define niDeclareInterfaceUUID_(NAME,A,B,C,D,E,F,G,H,I,J,K)         \
  inline static const ni::tUUID& __stdcall _GetInterfaceUUID() {      \
    static const ni::tUUID uuid = niInitUUID(A,B,C,D,E,F,G,H,I,J,K);  \
    return uuid; }

#    define niDeclareBaseInterface(NAME,A,B,C,D,E,F,G,H,I,J,K)  \
 protected:                                                   \
  virtual ~NAME() {}                                          \
 public:                                                      \
  typedef NAME IUnknownBaseType;                              \
  niBaseInterfacePadding;                                     \
  niDeclareInterfaceID_(NAME);                                \
  niDeclareInterfaceUUID_(NAME,A,B,C,D,E,F,G,H,I,J,K);        \

#    ifdef _lint // to suppress warning 1516 : Data member hides inherited member 'ni::ni::iUnknown::IUnknownBaseType'
#      define niDeclareInterfaceUUID(NAME,A,B,C,D,E,F,G,H,I,J,K)
#    else
#      define niDeclareInterfaceUUID(NAME,A,B,C,D,E,F,G,H,I,J,K)  \
 protected:                                                     \
  ~NAME() {}                                                    \
 public:                                                        \
  typedef NAME IUnknownBaseType;                                \
  niDeclareInterfaceID_(NAME);                                  \
  niDeclareInterfaceUUID_(NAME,A,B,C,D,E,F,G,H,I,J,K);
#    endif

#    define niGetInterfaceID(INTERFACE) INTERFACE::IUnknownBaseType::_GetInterfaceID()
#    define niGetInterfaceUUID(INTERFACE) INTERFACE::IUnknownBaseType::_GetInterfaceUUID()

#    ifndef niBaseInterfacePadding
#      pragma message("E/C++ Preprocessor: Platform's base interface padding not defined")
#    endif

//////////////////////////////////////////////////////////////////////////////////////////////
//! IUnknown

niConstValue achar* const kiidIUnknown = _A("iUnknown");

enum eListInterfacesFlags
{
  //! List all interfaces
  eListInterfacesFlags_All = 0,
  //! Don't list 'dynamic' interfaces (interfaces in sink that extend the object).
  eListInterfacesFlags_NoDynamic = niBit(1),
  //! Only list 'dynamic' interfaces (interfaces in sink that extend the object).
  //! \remark This needs to be implemented only for objects that do have dynamic interfaces.
  eListInterfacesFlags_DynamicOnly = niBit(2),
  //! \internal
  eListInterfacesFlags_ForceDWORD = 0xFFFFFFFF
};

//! iUnknown interface
//! {NoAutomation}
//! \remark iUnknown is the base of all ni interfaces. It support reference
//!         counting and sanity check throught the IsOK() method.
struct iUnknown {
  niDeclareBaseInterface(iUnknown,0xec052841,0x18a0,0x46c1,0xad,0x8b,0xcb,0xfe,0x06,0x57,0xec,0x0e);

  //! Check if the object is valid.
  //! \return niTrue if the object can be used.
  virtual tBool __stdcall IsOK() const = 0;
  //! Signal that one more reference of the object has been created.
  //! \return the new number of reference, or eInvalidHandle if the object is not reference counted.
  virtual tI32 __stdcall AddRef() = 0;
  //! Release a reference of the object.
  //! \return the new number of reference, or eInvalidHandle if the object is not reference counted.
  /** Delete the object if there is no more reference that are used. */
  virtual tI32 __stdcall Release() = 0;
  //! Bypass any reference counting and delete this object.
  virtual void __stdcall DeleteThis() = 0;
  //! Invalidate the object.
  //! \remark This method is called when the object is not valid anymore and can't be used anymore.
  //! \remark This method has been added to make the objects garbage collector compatible.
  virtual void __stdcall Invalidate() = 0;
  //! Query an interface.
  //! \return NULL if the required interface cant be queried.
  virtual iUnknown* __stdcall QueryInterface(const tUUID& aIID) = 0;
  //! Fill a UUID list containing the UUIDs of the implemented interfaces.
  virtual void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const = 0;
  //! Set the reference counter directly.
  //! \remark This is, needless to say, dangerous and should be used only as a mean to bypass
  //!     the hard reference counting when it's required.
  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs) = 0;
  //! Get the number of references to this object.
  virtual tI32 __stdcall GetNumRefs() const = 0;
};

struct sMethodDef;

//! IDispatch interface.
//! {NoAutomation}
struct iDispatch : public iUnknown {
  niDeclareInterfaceUUID(iDispatch,0x19e1e977,0x4249,0x4104,0x80,0x87,0xa7,0xd1,0x4b,0x5a,0x26,0x21);

  //! Initialize the methods.
  virtual tBool __stdcall InitializeMethods(const sMethodDef* const* apMethods, ni::tU32 anNumMethods) = 0;
  //! Call the specified method.
  //! \return niFalse if the method call failed, else niTrue.
  virtual tBool __stdcall CallMethod(const sMethodDef* const apMethodDef, ni::tU32 anMethodIndex, const Var* apParameters, tU32 anNumParameters, Var* apRet) = 0;
};

/**@}*/
}
#  endif // __cplusplus
#endif // niShaderLanguage

//--------------------------------------------------------------------------------------------
//
//  IUnknown utilities
//
//--------------------------------------------------------------------------------------------
#if !defined niShaderLanguage
#  ifdef __cplusplus
namespace ni {
/** \addtogroup niLang
 * @{
 */

// IUnknown raw pointer, necessary for template specialization...
typedef iUnknown* tIUnknownP;

//! Query the specified interface
template <typename T>
inline T* QueryInterface(const iUnknown* p)
{
  if (p == NULL) return NULL;
  return (T*)const_cast<iUnknown*>(p)->QueryInterface(T::_GetInterfaceUUID());
}

template <typename T>
__forceinline tI32 AddRef(T* p) { return p->AddRef(); }
template <typename T>
__forceinline tI32 AddRef(const T* p) { return const_cast<T*>(p)->AddRef(); }

template <typename T>
__forceinline tI32 Release(T* p) { return p->Release(); }
template <typename T>
__forceinline tI32 Release(const T* p) { return const_cast<T*>(p)->Release(); }

//! Return false if the given instance is invalid, else true.
inline tBool IsOK(const iUnknown* pI) { return pI && pI->IsOK(); }

//! Guard an object. Prevents it from being released.
inline void GuardObject(iUnknown* apObj) {
  niAssert(apObj);
  apObj->AddRef();
}

//! Unguard an object.
inline void UnguardObject(iUnknown* apObj) {
  niAssert(apObj);
  apObj->Release();
}

//! Safely invalidate the object.
template <typename T>
inline void SafeInvalidate(T* apPtr) {
  if (!apPtr) return;
  GuardObject(apPtr);
  apPtr->Invalidate();
  UnguardObject(apPtr);
}

//! Scope bound object guard.
struct ObjectGuard
{
  iUnknown* pObj;
  ObjectGuard(iUnknown* apObj) {
    if (apObj && apObj->GetNumRefs() > 0) {
      pObj = apObj;
      GuardObject(pObj);
    }
    else {
      pObj = NULL;
    }
  }
  ~ObjectGuard() {
    if (pObj) UnguardObject(pObj);
  }
};

//! Scope bound object guard macro.
#    define niGuardObject(obj)  ni::ObjectGuard __ni_guard(obj)
//! Named scope bound object guard macro.
#    define niGuardObject_(obj,GUARDNAME) ni::ObjectGuard __ni_guard__##GUARDNAME(obj)

template <typename T>
struct ConstructorGuard {
  T* pThis;
  ConstructorGuard(T* apThis) : pThis(apThis) {
    niAssert(apThis != NULL);
    niAssert(apThis->mprotected_nNumRefs == 0);
    apThis->mprotected_nNumRefs += 1000;
  }
  ~ConstructorGuard() {
    pThis->mprotected_nNumRefs -= 1000;
    niAssert(pThis->mprotected_nNumRefs <= 0);
  }
};

//! Guards the reference count in a constructor, this is usefull if the
//! constructor stores 'this' in a smart pointer or Var (for example when
//! sending a message) ; it prevents the object from being released in the
//! constructor...
#    define niGuardConstructor(T)                     \
  ni::ConstructorGuard<T> __constructorGuard(this);

#    if !defined niConfig_NoXCALL

niConstValue tInt eVMRet_OK = 0;
niConstValue tInt eVMRet_InvalidArgCount = -1;
niConstValue tInt eVMRet_InvalidMethod = -2;
niConstValue tInt eVMRet_InvalidArg = -3;
niConstValue tInt eVMRet_InvalidRet = -4;
niConstValue tInt eVMRet_InvalidNullArg = -5;
niConstValue tInt eVMRet_OutOfMemory = -6;

#      ifdef __cplusplus
typedef tInt (__ni_export_call_decl *tpfnVMCall)(iUnknown* apThis, const Var* apParameters, tU32 anNumParameters, Var* apRet);
#      else
typedef tInt (__ni_export_call_decl *tpfnVMCall)(struct iUnknown* apThis, const Var* apParameters, tU32 anNumParameters, Var* apRet);
#      endif
#    else
#      ifdef __cplusplus
typedef tInt (__ni_export_call_decl *tpfnVMCall)(iUnknown* apThis, tIntPtr aVMHandle, tInt aStackBase);
#      else
typedef tInt (__ni_export_call_decl *tpfnVMCall)(struct iUnknown* apThis, tIntPtr aVMHandle, tInt aStackBase);
#      endif

#    endif

/**@}*/
}
#  endif
#endif // niShaderLanguage

//--------------------------------------------------------------------------------------------
//
//  File base
//
//--------------------------------------------------------------------------------------------
#if !defined niShaderLanguage
#  ifdef __cplusplus
namespace ni {
/** \addtogroup niLang
 * @{
 */

//! File attributes
typedef enum eFileAttrFlags
{
  //! File.
  eFileAttrFlags_File = niBit(0),
  //! Directory.
  eFileAttrFlags_Directory = niBit(1),
  //! Archive.
  eFileAttrFlags_Archive = niBit(3),
  //! Read only.
  eFileAttrFlags_ReadOnly = niBit(4),
  //! Hidden attribute.
  eFileAttrFlags_Hidden = niBit(5),
  //! System attribute.
  eFileAttrFlags_System = niBit(6),
  //! The file is 'virtual', aka it is not stored in the standard OS's file
  //! system, most likely it is stored within an archive or a-like.
  eFileAttrFlags_Virtual = niBit(7),
  //! Device attribute.
  eFileAttrFlags_Device = niBit(8),
  //! Fixed device attribute.
  eFileAttrFlags_DeviceFixed = eFileAttrFlags_Device|niBit(15),
  //! Removable device attribute.
  eFileAttrFlags_DeviceRemovable = eFileAttrFlags_Device|niBit(16),
  //! RAM drive device attribute.
  eFileAttrFlags_DeviceRAM = eFileAttrFlags_Device|niBit(17),
  //! Remote device attribute.
  eFileAttrFlags_DeviceRemote = eFileAttrFlags_Device|niBit(18),
  //! Floppy device attribute.
  eFileAttrFlags_DeviceFloppy = eFileAttrFlags_Device|eFileAttrFlags_DeviceRemovable|niBit(19),
  //! CDRom device attribute.
  eFileAttrFlags_DeviceCDRom = eFileAttrFlags_Device|eFileAttrFlags_DeviceRemovable|niBit(20),
  //! Reserved0 device attribute.
  eFileAttrFlags_DeviceReserved0 = eFileAttrFlags_Device|niBit(21),
  //! Reserved1 device attribute.
  eFileAttrFlags_DeviceReserved1 = eFileAttrFlags_Device|niBit(22),
  //! Reserved2 device attribute.
  eFileAttrFlags_DeviceReserved2 = eFileAttrFlags_Device|niBit(23),
  //! Reserved3 device attribute.
  eFileAttrFlags_DeviceReserved3 = eFileAttrFlags_Device|niBit(24),
  //! Reserved4 device attribute.
  eFileAttrFlags_DeviceReserved4 = eFileAttrFlags_Device|niBit(25),
  //! Reserved5 device attribute.
  eFileAttrFlags_DeviceReserved5 = eFileAttrFlags_Device|niBit(26),
  //! Reserved6 device attribute.
  eFileAttrFlags_DeviceReserved6 = eFileAttrFlags_Device|niBit(27),
  //! Reserved7 device attribute.
  eFileAttrFlags_DeviceReserved7 = eFileAttrFlags_Device|niBit(28),
  //! Reserved8 device attribute.
  eFileAttrFlags_DeviceReserved8 = eFileAttrFlags_Device|niBit(29),
  //! Reserved9 device attribute.
  eFileAttrFlags_DeviceReserved9 = eFileAttrFlags_Device|niBit(30),
  //! Reserved10 device attribute.
  eFileAttrFlags_DeviceReserved10 = eFileAttrFlags_Device|niBit(31),
  //! All.
  eFileAttrFlags_All = eFileAttrFlags_File|eFileAttrFlags_Directory,
  //! All files.
  eFileAttrFlags_AllFiles = eFileAttrFlags_File,
  //! All directories.
  eFileAttrFlags_AllDirectories = eFileAttrFlags_Directory,
  //! \internal
  eFileAttrFlags_ForceDWORD = 0xFFFFFFFF
} eFileAttrFlags;

//! File time.
typedef enum eFileTime
{
  //! Creation file time.
  eFileTime_Creation = 0,
  //! Last access file time.
  eFileTime_LastAccess = 1,
  //! Last write file time.
  eFileTime_LastWrite = 2,
  //! \internal
  eFileTime_ForceDWORD = 0xFFFFFFFF
} eFileTime;

//! File open mode.
typedef enum eFileOpenMode
{
  //! Read open mode.
  eFileOpenMode_Read = niBit(0),
  //! Write open mode.
  eFileOpenMode_Write = niBit(1),
  //! Append open mode.
  eFileOpenMode_Append = niBit(2)|eFileOpenMode_Write,
  //! Optimized form random access.
  eFileOpenMode_Random = niBit(3),
  //! \internal
  eFileOpenMode_ForceDWORD = 0xFFFFFFFF
} eFileOpenMode;

//! File attributes flags type. \see ni::eFileAttrFlags
typedef tU32 tFileAttrFlags;

/**@}*/
}
#  endif // __cplusplus
#endif // niShaderLanguage

//--------------------------------------------------------------------------------------------
//
//  SDK
//
//--------------------------------------------------------------------------------------------
#if !defined niShaderLanguage

niExportFunc(niNamespace(ni,sVec4i)*) ni_mem_get_stats(niNamespace(ni,sVec4i)* apStats);
niExportFunc(void) ni_mem_dump_report();
niExportFunc(void) ni_mem_dump_leaks();

#  ifndef niNoDLL
niExportFunc(niNamespace(ni,tIntPtr)) ni_dll_load(
    const niNamespace(ni,achar)* aName);

niExportFunc(void) ni_dll_free(
    niNamespace(ni,tIntPtr) aModule);

niExportFunc(niNamespace(ni,tPtr)) ni_dll_get_proc(
    niNamespace(ni,tIntPtr) aModule,
    const niNamespace(ni,cchar)* aProcName);
#  endif

niExportFunc(niNamespace(ni,achar)*) ni_get_exe_path(
    niNamespace(ni,achar)* buffer);

#  ifdef __cplusplus
namespace ni {
#  endif // __cplusplus

//! The current time since the begining of the app.
niExportFunc(niNamespace(ni,tF64)) TimerInSeconds();

#  ifdef __cplusplus
}
#  endif // __cplusplus

#endif // niShaderLanguage

//////////////////////////////////////////////////////////////////////////////////////////////
#endif // __TYPES_H_C2ADBAF2_7B9D_4C93_BE52_AB1F88D2CB54__
