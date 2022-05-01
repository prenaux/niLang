#ifndef __ATLDEFS_H_BA09AE67_A59D_42CB_BA43_C0CD4C586B80__
#define __ATLDEFS_H_BA09AE67_A59D_42CB_BA43_C0CD4C586B80__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#ifdef __WIN32_REDEF_H_CF655D97_FF0F_4EFC_835E_FD5B0A83720E__
#pragma message("=== You should let ATLDefs.h include Win32_Redef.h")
#endif

#include "Win32_Redef.h"

#undef GetMessage
#undef SendMessage
#undef PostMessage
#undef DispatchMessage
#undef PeekMessage
#undef CreateFile
#undef CreateWindow
#undef CreateWindowEx
#undef CreateEvent
#undef GetClassName
#undef GetCurrentThread
#undef GetModuleFileName

#ifdef _UNICODE
#define GetMessage      GetMessageW
#define SendMessage     SendMessageW
#define PostMessage     PostMessageW
#define DispatchMessage   DispatchMessageW
#define PeekMessage     PeekMessageW
#define CreateFile      CreateFileW
#define CreateWindow    CreateWindowW
#define CreateWindowEx    CreateWindowExW
#define CreateEvent     ::CreateEventW
#define GetClassName    GetClassNameW
#define GetModuleFileName   GetModuleFileNameW
#else
#define GetMessage      GetMessageA
#define SendMessage     SendMessageA
#define PostMessage     PostMessageA
#define DispatchMessage   DispatchMessageA
#define PeekMessage     PeekMessageA
#define CreateFile      CreateFileA
#define CreateWindow    CreateWindowA
#define CreateWindowEx    CreateWindowExA
#define CreateEvent     ::CreateEventA
#define GetClassName    GetClassNameA
#define GetModuleFileName   GetModuleFileNameA
#endif

#define GetCurrentThread  ::GetCurrentThread

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __ATLDEFS_H_BA09AE67_A59D_42CB_BA43_C0CD4C586B80__
