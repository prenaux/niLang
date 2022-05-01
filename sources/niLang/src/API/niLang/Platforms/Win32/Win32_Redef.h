#ifndef __WIN32_REDEF_H_CF655D97_FF0F_4EFC_835E_FD5B0A83720E__
#define __WIN32_REDEF_H_CF655D97_FF0F_4EFC_835E_FD5B0A83720E__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#ifdef _WINDOWS_
#pragma message("=== You should let Win32_Redef.h include windows.h")
#endif

#ifndef _WIN32_WINNT
#ifdef WINAPI_FAMILY
#define _WIN32_WINNT 0x0600
#else
#define _WIN32_WINNT 0x0501
#endif
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning ( disable : 4005 ) // macro redef
#if defined _LINT && _MSC_VER >= 1600
#include <codeanalysis\warnings.h>
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#endif
#endif

#include <tchar.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include <Shlobj.h>

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#include <direct.h>
#include <io.h>
#include <wchar.h>

// This header can be safely included multiple times

#undef CreateFile
#undef CreateFont
#undef CreateMutex
#undef CreateProcess
#undef CreateSemaphore
#undef CreateThread
#undef CreateThread
#undef CreateWindow
#undef CreateWindowEx
#undef DispatchMessage
#undef DrawText
#undef DrawTextEx
#undef GetCharWidth
#undef GetClassName
#undef GetCommandLine
#undef GetCurrentTime
#undef GetCurrentTime
#undef GetFileAttributes
#undef GetFileAttributes
#undef GetMessage
#undef GetModuleFileName
#undef GetObject
#undef LoadBitmap
#undef Message
#undef MessageBox
#undef PeekMessage
#undef PostMessage
#undef SendMessage
#undef SetFileAttributes
#undef SetPort
#undef max
#undef min
#undef GetUserName

#undef niWin32API
#define niWin32API(FUNC)  ::FUNC##W

#undef SNDMSG
#define SNDMSG  niWin32API(SendMessage)

#if defined __cplusplus && defined niWinDesktop
namespace ni { namespace Windows {
niExportFunc(char*) utf8_GetCommandLine();
niExportFunc(BOOL) utf8_SHGetSpecialFolderPath(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate);
niExportFunc(BOOL) utf8_SHGetFolderPath(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPSTR lpszPath);
niExportFunc(int) utf8_access(const char* aaszPath, int mode);
#define Win32GetCommandLine ni::Windows::utf8_GetCommandLine
#define Win32GetSpecialFolderPath ni::Windows::utf8_SHGetSpecialFolderPath
#define Win32GetFolderPath ni::Windows::utf8_SHGetFolderPath
}}
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIN32_REDEF_H_CF655D97_FF0F_4EFC_835E_FD5B0A83720E__
