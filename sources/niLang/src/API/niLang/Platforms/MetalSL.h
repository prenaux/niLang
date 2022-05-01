#pragma once
#ifndef __METAL_H_910CC61D_75DB_FF4C_B185_DD553C2DF10B__
#define __METAL_H_910CC61D_75DB_FF4C_B185_DD553C2DF10B__

// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if defined(__METAL_VERSION__)

#include <metal_stdlib>

#define niCPP11

#define niOverride override

#define niPlatformDetected
#ifdef niPragmaPrintPlatformDetected
#pragma message("=== Detected Metal Shader Language")
#endif

#define niMetalSL
#define niShaderLanguage

#define niAssume(EXPR) __assume(EXPR)

#define niNoThreads 1
#define niNoCrashReport 1
#define niNoProcess 1
#define niNoSocket 1
#define niNoDLL 1
#define niNoPointer 1
#define niNoConstVar 1
#define niNoDouble 1

// Definition types
#define inline  __inline
#define __stdcall
#define __cdecl

// Typename
#define niTypename  typename

//////////////////////////////////////////////////////////////////////////////////////////////
// Structures packing
#define niPragmaPack 0
#define niPackPush(x)
#define niPackPop()
#define niPacked(x) __attribute__((packed))

//////////////////////////////////////////////////////////////////////////////////////////////
// Types definition
#define niTypeCChar metal::int8_t
#define niTypeUChar metal::uint16_t
#define niUCharSize 2
#define niTypeI8    metal::int8_t
#define niTypeI16   metal::int16_t
#define niTypeU8    metal::uint8_t
#define niTypeU16   metal::uint16_t
#define niTypeF16   half
#define niTypeF32   float
#if! defined niNoDouble
#define niTypeF64   float
#endif
#define niTypePtr   niTypeI64
#define niTypeSize  metal::size_t
#define niTypeInt   metal::int32_t
#define niTypeUInt  metal::uint32_t

#define niTypeIntSize 4
#define niTypeI32     metal::int32_t
#define niTypeU32     metal::uint32_t
#define niTypeI64     metal::ptrdiff_t
#define niTypeU64     metal::size_t
#define niTypeIntPtr  niTypeI64
#define niTypeUIntPtr niTypeU64
#define niTypeOffset  niTypeI64

#define niPragmaMinTypeInfoOn
#define niPragmaMinTypeInfoOff

#endif // __METAL_VERSION__

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __METAL_H_910CC61D_75DB_FF4C_B185_DD553C2DF10B__
