#ifndef __WIN32_UTF8_H_4FD6E97E_940A_43DC_A095_E285B2698A73__
#define __WIN32_UTF8_H_4FD6E97E_940A_43DC_A095_E285B2698A73__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Win32_Redef.h"
#include "../../StringLib.h"

namespace ni {
namespace Windows {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Win32
 * @{
 */

template <typename T>
struct UTFBuffer {
  BOOL mbAdopted;
  void* mpData;
  inline UTFBuffer() : mpData(NULL), mbAdopted(eFalse) {}
  inline ~UTFBuffer() { Free(); }
  inline T* Adopt(void* apBuffer) {
    mpData = apBuffer;
    mbAdopted = eTrue;
    return (T*)mpData;
  }
  inline T* GetNewBuffer(SIZE_T numChars) {
    if (!mbAdopted) {
      Free();
      mpData = ::HeapAlloc(::GetProcessHeap(),0,numChars*sizeof(T));
    }
    return (T*)mpData;
  }
  inline void Free() {
    if (mpData && !mbAdopted) {
      ::HeapFree(::GetProcessHeap(),0,mpData);
      mbAdopted = eFalse;
      mpData = NULL;
    }
  }

  niDeprecated(202010, data)
  inline T* begin() {
    return mpData ? (T*)mpData : StringEmpty<T>();
  }
  inline T* data() {
    return mpData ? (T*)mpData : StringEmpty<T>();
  }
};

typedef UTFBuffer<CHAR> UTF8Buffer;
typedef UTFBuffer<WCHAR> UTF16Buffer;

#define niWin32_UTF8ToUTF16(WVAR,VAR) {         \
    ni::Windows::ConvertUTF8ToUTF16(WVAR,VAR);  \
  }

#define niWin32_UTF16ToUTF8(WVAR,VAR) {         \
    ni::Windows::ConvertUTF16ToUTF8(WVAR,VAR);  \
  }

//! Converts Unicode UTF-8 text to Unicode UTF-16 (Windows default).
inline BOOL ConvertUTF8ToUTF16(UTF16Buffer& aOut, const CHAR * pszTextUTF8, size_t cchUTF8 = 0)
{
  //
  // Special case of NULL or empty input string
  //
  if ((pszTextUTF8 == NULL) || ((*pszTextUTF8 == '\0') && cchUTF8 == 0)) {
    // Return empty string
    return TRUE;
  }

  //
  // Consider CHAR's count corresponding to total input string length,
  // including end-of-string (\0) character
  //
  if (cchUTF8 == 0) {
    cchUTF8 = ni::StrSize( pszTextUTF8 );
  }

  // Consider also terminating \0
  ++cchUTF8;

  // Convert to 'int' for use with MultiByteToWideChar API
  int cbUTF8 = static_cast<int>(cchUTF8);

  //
  // Get size of destination UTF-16 buffer, in WCHAR's
  //
  int cchUTF16 = ::MultiByteToWideChar(
      CP_UTF8,                // convert from UTF-8
      0,              // error on invalid chars
      pszTextUTF8,            // source UTF-8 string
      cbUTF8,                 // total length of source UTF-8 string,
      // in CHAR's (= bytes), including end-of-string \0
      NULL,                   // unused - no conversion done in this step
      0                       // request size of destination buffer, in WCHAR's
                                       );
  if (cchUTF16 == 0) {
    return FALSE;
  }

  //
  // Allocate destination buffer to store UTF-16 string
  //
  WCHAR * pszUTF16 = aOut.GetNewBuffer(cchUTF16);

  //
  // Do the conversion from UTF-8 to UTF-16
  //
  int result = ::MultiByteToWideChar(
      CP_UTF8,                // convert from UTF-8
      0,            // error on invalid chars
      pszTextUTF8,            // source UTF-8 string
      cbUTF8,                 // total length of source UTF-8 string,
      // in CHAR's (= bytes), including end-of-string \0
      pszUTF16,               // destination buffer
      cchUTF16                // size of destination buffer, in WCHAR's
                                     );
  if (result == 0) {
    return FALSE;
  }

  return TRUE;
}

//! Converts Unicode UTF-16 (Windows default) text to Unicode UTF-8.
inline BOOL ConvertUTF16ToUTF8(UTF8Buffer& aOut, const WCHAR* pszTextUTF16, size_t cchUTF16 = 0)
{
  //
  // Special case of NULL or empty input string
  //
  if ((pszTextUTF16 == NULL) || ((*pszTextUTF16 == L'\0') && cchUTF16 == 0)) {
    // Return empty string
    return TRUE;
  }

  //
  // Consider WCHAR's count corresponding to total input string length,
  // including end-of-string (L'\0') character.
  //
  if (cchUTF16 == 0) {
    const size_t cchUTF16Max = INT_MAX - 1;
    HRESULT hr = ::StringCchLengthW( pszTextUTF16, cchUTF16Max, &cchUTF16 );
    if (FAILED(hr)) {
      return FALSE;
    }
  }

  // Consider also terminating \0
  ++cchUTF16;

  //
  // Get size of destination UTF-8 buffer, in CHAR's (= bytes)
  //
  int cbUTF8 = ::WideCharToMultiByte(
      CP_UTF8,                // convert to UTF-8
      0,            // specify conversion behavior
      pszTextUTF16,           // source UTF-16 string
      static_cast<int>( cchUTF16 ),   // total source string length, in WCHAR's,
      // including end-of-string \0
      NULL,                   // unused - no conversion required in this step
      0,                      // request buffer size
      NULL, NULL              // unused
                                     );
  if (cbUTF8 == 0) {
    return FALSE;
  }

  //
  // Allocate destination buffer for UTF-8 string
  //
  int cchUTF8 = cbUTF8; // sizeof(CHAR) = 1 byte
  CHAR * pszUTF8 = aOut.GetNewBuffer(cchUTF8);

  //
  // Do the conversion from UTF-16 to UTF-8
  //
  int result = ::WideCharToMultiByte(
      CP_UTF8,                // convert to UTF-8
      0,                      // specify conversion behavior
      pszTextUTF16,           // source UTF-16 string
      static_cast<int>( cchUTF16 ),   // total source string length, in WCHAR's,
      // including end-of-string \0
      pszUTF8,                // destination buffer
      cbUTF8,                 // destination buffer size, in bytes
      NULL, NULL              // unused
                                     );
  if (result == 0) {
    return FALSE;
  }

  // Return resulting UTF-8 string
  return TRUE;
}

niExportFunc(void) utf8_OutputDebugString(const char* aaszMsg);

#ifdef niWinDesktop
niExportFunc(char*) utf8_FixDriveLetter(char* apOut);
niExportFunc(char*) utf8_GetModuleFileName(HMODULE ahDLL, char* apOut);
niExportFunc(HMODULE) utf8_GetModuleHandle(const char* aaszPath);
niExportFuncCPP(cString) utf8_getcwd();
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}}

#endif // __WIN32_UTF8_H_4FD6E97E_940A_43DC_A095_E285B2698A73__
