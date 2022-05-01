#ifndef __WIN32_REGISTRY_H_57B1E71F_3456_466B_A32A_7CEF1C089AF5__
#define __WIN32_REGISTRY_H_57B1E71F_3456_466B_A32A_7CEF1C089AF5__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../../Types.h"
#include "../../Utils/StringIntToStr.h"
#include "Win32_UTF.h"

#ifdef niPragmaCommentLib
#pragma comment(lib,"advapi32.lib")
#endif

namespace ni { namespace Windows {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Win32
 * @{
 */

//! Get the HKey of the specified path
inline HKEY WinRegGetHKeyClass(const WCHAR* aszPath, WCHAR const** apRet) {
  const WCHAR* aszEnd = aszPath;
  while (*aszEnd && *aszEnd != '\\' && *aszEnd != '/') {
    ++aszEnd;
  }
  const ni::tU32 len = (tU32)(aszEnd-aszPath);

  if (apRet) {
    *apRet = (WCHAR*)aszEnd;
  }

  if (wcsnicmp(aszPath,L"HKEY_CLASSES_ROOT",len) == 0) {
    return HKEY_CLASSES_ROOT;
  }
  if (wcsnicmp(aszPath,L"HKEY_CURRENT_CONFIG",len) == 0) {
    return HKEY_CURRENT_CONFIG;
  }
  if (wcsnicmp(aszPath,L"HKEY_CURRENT_USER",len) == 0 ||
      wcsnicmp(aszPath,L"HKCU",len) == 0) {
    return HKEY_CURRENT_USER;
  }
  if (wcsnicmp(aszPath,L"HKEY_LOCAL_MACHINE",len) == 0 ||
      wcsnicmp(aszPath,L"HKLM",len) == 0) {
    return HKEY_LOCAL_MACHINE;
  }
  if (wcsnicmp(aszPath,L"HKEY_PERFORMANCE_DATA",len) == 0) {
    return HKEY_PERFORMANCE_DATA;
  }
  if (wcsnicmp(aszPath,L"HKEY_USERS",len) == 0) {
    return HKEY_USERS;
  }

  return 0;
}

//! Copy to a registry compatible path
inline WCHAR* WinRegPath_Copy(WCHAR* aaszOutput, const WCHAR* aaszPath) {
  WCHAR* d = aaszOutput;
  const WCHAR* p = NULL;
  if (WinRegGetHKeyClass(aaszPath,&p)) {
    if (p && *p && (*p == '\\' || *p == '/')) {
      ++p; // skip the first slash
      while (*p) {
        if (*p == '/') {
          *d++ = '\\';
        }
        else {
          *d++ = *p;
        }
        ++p;
      }
    }
  }
  *d++ = 0;
  return aaszOutput;
}

inline WCHAR* WinRegPath_GetKeyPath(WCHAR* apOut, const WCHAR* wszPath) {
  const WCHAR* bsPos = wcsrchr(wszPath,L'\\');
  if (!bsPos) {
    *apOut = 0;
    return apOut;
  }
  memcpy(apOut,wszPath,(bsPos-wszPath)*sizeof(WCHAR));
  apOut[bsPos-wszPath] = 0;
  return apOut;
}

inline WCHAR* WinRegPath_GetValueName(WCHAR* apOut, const WCHAR* wszPath) {
  const WCHAR* valueName = wcsrchr(wszPath,L'\\');
  if (!valueName) {
    *apOut = 0;
    return apOut;
  }
  ++valueName; // after the '\'
  const WCHAR* s = valueName;
  WCHAR* d = apOut;
  while (*s) {
    *d++ = *s++;
  }
  return apOut;
}

//! Get the DWORD type
inline DWORD WinRegGetType(const WCHAR* aaszType) {
  if (wcsicmp(aaszType,L"REG_SZ") == 0) {
    return REG_SZ;
  }
  if (wcsicmp(aaszType,L"REG_EXPAND_SZ") == 0) {
    return REG_EXPAND_SZ ;
  }
  if (wcsicmp(aaszType,L"REG_DWORD") == 0) {
    return REG_DWORD;
  }
  return 0;
}

//! Write a registry value
inline ni::tBool __stdcall WinRegWrite(const WCHAR* aaszPath,
                                       const WCHAR* aaszVal,
                                       const WCHAR* aaszType niDefaultParam(L"REG_SZ"))
{
  HKEY hKeyClass = WinRegGetHKeyClass(aaszPath,NULL);
  if (!hKeyClass) {
    //    niError(niFmt(_A("Can't get the key class of the path '%s'."),aaszPath));
    return ni::eFalse;
  }
  WCHAR strValuePath[AMAX_PATH] = L"";
  WinRegPath_Copy(strValuePath,aaszPath);
  if (!*strValuePath) {
    //    niError(niFmt(_A("Can't get value path of the path '%s'."),aaszPath));
    return ni::eFalse;
  }
  DWORD type = WinRegGetType(aaszType);
  if (!type) {
    //    niError(niFmt(_A("Can't get the type '%s' of the path '%s'."),aaszType,aaszPath));
    return ni::eFalse;
  }

  WCHAR strCreateKey[AMAX_PATH] = L"";
  WinRegPath_GetKeyPath(strCreateKey,strValuePath);
  HKEY hKey;
  if (niWin32API(RegCreateKey)(hKeyClass, strCreateKey, &hKey) != ERROR_SUCCESS)  {
    //    niError(niFmt(_A("Can't create/open the key '%s' (type:%s), value: '%s'."),
    //                         aaszPath,aaszType,aaszVal));
    return ni::eFalse;
  }

  WCHAR strValueName[AMAX_PATH] = L"";
  WinRegPath_GetValueName(strValueName,strValuePath);
  LONG r = -1;
  switch (type) {
    case REG_SZ:
    case REG_EXPAND_SZ: {
      r = niWin32API(RegSetValueEx)(hKey,strValueName,0,type,
                                    (BYTE*)aaszVal,
                                    (DWORD)((wcslen(aaszVal)+1)*sizeof(WCHAR)));
      break;
    }
    case REG_DWORD: {
      WCHAR* endp;
      DWORD v = wcstol(aaszVal,&endp,10);
      r = niWin32API(RegSetValueEx)(hKey,strValueName,0,type,(BYTE*)&v,sizeof(v));
      break;
    }
  }

  if (r != ERROR_SUCCESS) {
    //    niError(niFmt(_A("Can't set the key '%s' (type:%s), value: '%s'."),
    //                         aaszPath,aaszType,aaszVal));
    ::RegCloseKey(hKey);
    return ni::eFalse;
  }

  ::RegCloseKey(hKey);
  return ni::eTrue;
}
inline ni::tBool __stdcall WinRegWrite(const ni::cchar* aaszPath,
                                       const ni::cchar* aaszVal,
                                       const ni::cchar* aaszType niDefaultParam(_A("REG_SZ")))
{
  UTF16Buffer wPath, wVal, wType;
  niWin32_UTF8ToUTF16(wPath,aaszPath);
  niWin32_UTF8ToUTF16(wVal,aaszVal);
  niWin32_UTF8ToUTF16(wType,aaszType);
  return WinRegWrite(wPath.begin(),wVal.begin(),wType.begin());
}

//! Read a registry value
inline WCHAR* __stdcall WinRegRead(WCHAR* apOut, const WCHAR* aaszPath) {
  HKEY hKeyClass = WinRegGetHKeyClass(aaszPath,NULL);
  if (!hKeyClass) {
    //    niError(niFmt(_A("Can't get the key class of the path '%s'."),aaszPath));
    *apOut = 0;
    return apOut;
  }

  WCHAR strValuePath[AMAX_PATH] = L"";
  WinRegPath_Copy(strValuePath,aaszPath);
  if (!*strValuePath) {
    //    niError(niFmt(_A("Can't get value path of the path '%s'."),aaszPath));
    *apOut = 0;
    return apOut;
  }

  HKEY hKey;
  WCHAR strOpenKey[AMAX_SIZE];
  WinRegPath_GetKeyPath(strOpenKey,strValuePath);
  if (niWin32API(RegOpenKey)(hKeyClass, strOpenKey, &hKey) != ERROR_SUCCESS)  {
    //    niError(niFmt(_A("Can't open the key '%s'."),aaszPath));
    *apOut = 0;
    return apOut;
  }

  DWORD type = 0;
  WCHAR retBuffer[AMAX_SIZE];
  DWORD retSize = AMAX_SIZE;
  memset(retBuffer,0,sizeof(retBuffer));
  WCHAR strValueName[AMAX_PATH] = L"";
  WinRegPath_GetValueName(strValueName,strValuePath);
  LONG r = niWin32API(RegQueryValueEx)(hKey,strValueName,NULL,&type,(BYTE*)retBuffer,&retSize);
  if (r != ERROR_SUCCESS) {
    //    niError(niFmt(_A("Can't get the key '%s'."),aaszPath));
    ::RegCloseKey(hKey);
    *apOut = 0;
    return apOut;
  }

  switch (type) {
    case REG_SZ:
    case REG_EXPAND_SZ:
      wcscpy(apOut,retBuffer);
      break;
    case REG_DWORD: {
      UTF16Buffer wOut; wOut.Adopt(apOut);
      niCAssert(sizeof(retBuffer) >= AMAX_SIZE);
      StringIntToStr((char*)retBuffer,sizeof(retBuffer),*(ni::tU32*)(retBuffer));
      niWin32_UTF8ToUTF16(wOut,(char*)retBuffer);
      break;
    }
    default:
      //    niError(niFmt(_A("Invalid type (%d), key '%s'."),type,aaszPath));
      *apOut = 0;
      break;
  }

  ::RegCloseKey(hKey);
  return apOut;
}

inline ni::cchar* __stdcall WinRegRead(ni::cchar* apOut, const ni::cchar* aaszPath) {
  UTF16Buffer wPath;
  niWin32_UTF8ToUTF16(wPath,aaszPath);
  WCHAR wRet[AMAX_SIZE];
  WinRegRead(wRet,wPath.begin());
  UTF8Buffer uOut; uOut.Adopt(apOut);
  niWin32_UTF16ToUTF8(uOut,wRet);
  return apOut;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}}
#endif // __WIN32_REGISTRY_H_57B1E71F_3456_466B_A32A_7CEF1C089AF5__
