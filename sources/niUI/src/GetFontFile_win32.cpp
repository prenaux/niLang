// GetFontFile.cpp
//
// Copyright (C) 2001 Hans Dietrich
//
// This software is released into the public domain.
// You are free to use it in any way you like.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#if defined niWindows && !defined niEmbedded
#include <niLang/Platforms/Win32/Win32_Redef.h>
#include <shlobj.h>

#include "GetFontFile.h"

#pragma comment(lib,"Advapi32.lib")

///////////////////////////////////////////////////////////////////////////////
//
// GetWindowsPlatform()
//
// Purpose: Get Windows platform
//
// Returns: int - platform
// 0 UNKNOWN
// 1 Win/95, Win/98, Win/ME
// 2 Win/NT, Win/2000, Win/XP, Win/VISTA
// 3 Win/CE
static int GetWindowsPlatform(void) {
  OSVERSIONINFO osinfo;
  memset(&osinfo, 0, sizeof(OSVERSIONINFO));
  osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (GetVersionEx(&osinfo)) {
    switch (osinfo.dwPlatformId) {
      case 1: return 1;
      case 2: return 2;
      case 3: return 3;
    }
  }
  return 0;
}

static cString GetFontRegistryPath() {
  const int nPlatform = GetWindowsPlatform();
  switch (nPlatform) {
    case 1: // Windows 95
      return _T("Software\\Microsoft\\Windows\\CurrentVersion\\Fonts");
    default:
    case 2: // Windows NT
      return _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
  }
}

static LONG GetNextNameValue(HKEY key, LPCTSTR subkey, LPTSTR szName, LPTSTR szData)
{
  static HKEY hkey = NULL;
  static DWORD dwIndex = 0;
  LONG retval;

  if (subkey == NULL && szName == NULL && szData == NULL) {
    if (hkey)
      RegCloseKey(hkey);
    hkey = NULL;
    return ERROR_SUCCESS;
  }

  if (subkey && subkey[0] != 0) {
    retval = RegOpenKeyEx(key, subkey, 0, KEY_READ, &hkey);
    if (retval != ERROR_SUCCESS)
      return retval;
    dwIndex = 0;
  }
  else {
    dwIndex++;
  }

  niAssert(szName != NULL && szData != NULL);

  *szName = 0;
  *szData = 0;

  TCHAR szValueName[MAX_PATH];
  ::ZeroMemory(szValueName,sizeof(szValueName));
  DWORD dwValueNameSize = sizeof(szValueName)-1;
  BYTE szValueData[MAX_PATH];
  ::ZeroMemory(szValueData,sizeof(szValueData));
  DWORD dwValueDataSize = sizeof(szValueData)-1;
  DWORD dwType = 0;

  retval = RegEnumValue(hkey, dwIndex, szValueName, &dwValueNameSize, NULL,
                        &dwType, szValueData, &dwValueDataSize);
  if (retval == ERROR_SUCCESS) {
    ni::StrCpy(szName, cString(szValueName).Chars());
    ni::StrCpy(szData, cString((achar*)szValueData).Chars());
  }

  return retval;
}

///////////////////////////////////////////////
tBool GetAllFontFiles(tFontFileLst& aLst) {
  _TCHAR szName[2 * MAX_PATH];
  _TCHAR szData[2 * MAX_PATH];
  ::ZeroMemory(szName,sizeof(szName));
  ::ZeroMemory(szData,sizeof(szData));

  cString strFont = GetFontRegistryPath();
  while (GetNextNameValue(HKEY_LOCAL_MACHINE, strFont.Chars(), szName, szData) == ERROR_SUCCESS) {
    sFontFile item;
    item.strFile = szData;
    if (!item.strFile.EndsWithI(_A(".ttf"))) {
      strFont.clear();  // this will get next value, same key
      continue;
    }
    item.strDisp = szName;
    if (item.strDisp.EndsWith(_A(" (TrueType)"))) {
      item.strDisp = item.strDisp.RBefore(_A(" (TrueType)"));
    }
    aLst.push_back(item);
    strFont.clear();  // this will get next value, same key
  }

  //     aLst.sort();
  return !aLst.empty();
}

///////////////////////////////////////////////
cString GetFontsDirectory() {
  TCHAR szFontDirPath[MAX_PATH] = {0};
  SHGetFolderPath(NULL, CSIDL_WINDOWS, NULL, 0, szFontDirPath);
  ni::StrCat(szFontDirPath,_A("\\Fonts"));
  return szFontDirPath;
}

#endif
