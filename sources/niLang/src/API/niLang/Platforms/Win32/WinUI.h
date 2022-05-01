#ifndef __WINUI_48978804_H__
#define __WINUI_48978804_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/Types.h>
#include <niLang/StringDef.h>
#include <niLang/Utils/UnknownImpl.h>
#include <niLang/Platforms/Win32/Win32_UTF.h>

//////////////////////////////////////////////////////////////////////
// Win32 defines
//////////////////////////////////////////////////////////////////////
#if defined _LINT && _MSC_VER >= 1600
#include <codeanalysis\warnings.h>
#pragma warning( push )
#pragma warning ( disable : ALL_CODE_ANALYSIS_WARNINGS )
#endif

#ifndef WINVER
#ifdef WINUI_USE_XP
// Windows XP level Win32 / IE / Common Controls functionality
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0600
#else
// Windows 2000 level Win32 / IE / Common Controls functionality
#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_IE 0x0500
#endif
#endif

// TODO: The check in objbase.h for including CoInitializeEx() is
//       '_WIN32_WINNT >= 0x0400' but '#define WINVER 0x0501' does
//       NOT include it. Ask for DCOM so we get it, appears to be
//       some Win32 header bug
#define _WIN32_DCOM

#include <niLang/Platforms/Win32/Win32_Redef.h>
#include <windowsx.h>
#ifdef niMSVC
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")
#endif

// Common controls
#include <commctrl.h>
#ifdef niMSVC
#pragma comment (lib, "comctl32.lib")
#pragma comment (lib, "comdlg32.lib")
#endif

// Shell
#include <shlwapi.h>
#include <shellapi.h>
#include <shlobj.h>
#ifdef niMSVC
#pragma comment (lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#endif

// Common dialog support
#include <commdlg.h>

#undef GetNextSibling
#undef GetPrevSibling

#ifndef GetWindowLongPtr
#define GetWindowLongPtr  GetWindowLong
#endif

#ifndef CF_DIBV5
#undef CF_MAX
#define CF_DIBV5            17
#define CF_MAX              18
#endif

#ifndef TBSTYLE_EX_MIXEDBUTTONS
#define TBSTYLE_EX_MIXEDBUTTONS             0x00000008
#endif

#ifndef TBSTYLE_EX_HIDECLIPPEDBUTTONS
#define TBSTYLE_EX_HIDECLIPPEDBUTTONS       0x00000010  // don't show partially obscured buttons
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL                   0x020A
#endif

#ifndef NIN_BALLOONUSERCLICK
#define NIN_BALLOONUSERCLICK (WM_USER + 5)
#endif

#if _MSC_VER < 1300
typedef int intptr_t;
#endif

extern "C" HINSTANCE  GetMsWinHInstance();

#define WINUI_IMPLEMENT_GET_HINSTANCE_STATIC(HINST) \
  extern "C" HINSTANCE  GetMsWinHInstance() {       \
    static HINSTANCE _hInst = (HINST);              \
    return _hInst;                                  \
  }

#define WINUI_IMPLEMENT_GET_HINSTANCE(HINST)    \
  extern "C" HINSTANCE  GetMsWinHInstance() {   \
    return (HINST);                             \
  }

#if defined _LINT && _MSC_VER >= 1600
#pragma warning( pop )
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
namespace WinUI {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_WinUI
 * @{
 */

inline BOOL SendMessageW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
  return (BOOL)niWin32API(SendMessage)(hWnd,Msg,wParam,lParam);
}

inline BOOL PostMessage(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam)
{
  return (BOOL)niWin32API(PostMessage)(hWnd,Msg,wParam,lParam);
}

//////////////////////////////////////////////////////////////////////
// Misc global defines
//////////////////////////////////////////////////////////////////////

#ifndef WINUI_VERSION
//! \brief Revision of the WinUI library. Check for this to avoid version conflicts
#define WINUI_VERSION MAKEWORD(1, 3)
#else
#error "Conflicting version of WinUI library already included."
#endif

#ifndef WINUI_INL
//! \brief WinUI inline mode. All global functions as well as some class members cary this attribute
#define WINUI_INL __inline
#endif

#ifdef WINUI_INCLUDE_ALL
/*! \def WINUI_INCLUDE_ALL
  \brief Define this symbol to include all of WinUI's sub libraries, which are excluded by default
*/
#define WINUI_INCLUDE_ALL // #define again for Doxygen

/*! \def WINUI_THREAD_SUPPORT
  \brief Define this symbol to include threading support
  \sa WINUI_INCLUDE_ALL
*/
#define WINUI_THREAD_SUPPORT

/*! \def WINUI_GL_SUPPORT
  \brief Define this symbol to include OpenGL support
  \sa WINUI_INCLUDE_ALL
*/
#define WINUI_GL_SUPPORT

/*! \def WINUI_NETWORK_SUPPORT
  \brief Define this symbol to include network support
  \sa WINUI_INCLUDE_ALL
*/
#define WINUI_NETWORK_SUPPORT

/*! \def WINUI_DDRAW7_SUPPORT
  \brief Define this symbol to include  DirectDraw7 support
  \sa WINUI_INCLUDE_ALL
*/
#define WINUI_DDRAW7_SUPPORT
#endif

#ifdef WINUI_THREAD_SUPPORT
// We have a user preference for threading support
#ifndef _MT
#error "Linking with multithreaded CRT required when WINUI_THREAD_SUPPORT defined."
#endif
#else
// No user preference for threading, support based on CRT
#ifdef _MT
//! \brief Enable or exclude threading support and related classes
#define WINUI_THREAD_SUPPORT
#else
//! \brief Enable or exclude threading support and related classes
#undef WINUI_THREAD_SUPPORT
#endif
#endif

// Helper macros

//! \brief Releases an IUnknown interface or general class having a Release() method
#define WINUI_RELEASE(x) { if (x) { niAssert(!IsBadReadPtr(x, sizeof(x))); (x)->Release(); (x) = NULL; } }

//! \brief Zeroes out a Win32 struct and sets the dwSize member
#define WINUI_INITSTRUCT(x) { ZeroMemory(&x, sizeof(x)); x.dwSize = sizeof(x); }

//! \brief Like the Win32 LOWORD macro, but for signed numbers
#define WINUI_SIGNED_LOWORD(L) ((__int16) ((__int32) (L) & 0xFFFF))

//! \brief Like the Win32 HIWORD macro, but for signed numbers
#define WINUI_SIGNED_HIWORD(L) ((__int16) ((__int32) (L) >> 16))

//! \brief Returns a random number between 0 and 1
#define WINUI_NORM_RAND ((float) rand() / (float) RAND_MAX)

//! \brief Clamps the value Num to the range (Min, Max)
#define WINUI_CLAMP(Num, Min, Max) (__min(Max, __max(Min, Num)))

//! \brief Deletes a pointer
#define WINUI_SAFE_DEL(x) { if (x) { niAssert(!IsBadReadPtr(x, sizeof(x))); delete x; x = NULL; } }

//! \brief Deletes an array
#define WINUI_SAFE_DEL_ARR(x) { if (x) { niAssert(!IsBadReadPtr(x, sizeof(x))); delete [] x; x = NULL; } }

//////////////////////////////////////////////////////////////////////////////////////////////
// Macros

#define WINUI_INIT_CONTROL(var,ID)    (var).AttachDlgItem(m_hWnd,ID)
#define WINUI_ADD_RADIO_STATE(var,ID) (var).AddState(m_hWnd,ID)

#define WINUI_BEGIN_WND_MSG_MAP()                                       \
  virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) \
  {                                                                     \
  niAssert(m_hWnd == NULL || hWnd == m_hWnd);                           \
  m_hWnd = hWnd;                                                        \
  switch (message) {

#define WINUI_END_WND_MSG_MAP(__base)                                   \
  default: break;                                                       \
}                                                                       \
                                                                return __base::WndProc(hWnd, message, wParam, lParam); \
}

#define WINUI_BEGIN_DLG_MSG_MAP()                                       \
  virtual INT_PTR DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) \
  {                                                                     \
  niAssert(m_hWnd == NULL || hWnd == m_hWnd);                           \
  m_hWnd = hWnd;                                                        \
  switch (message) {

#define WINUI_END_DLG_MSG_MAP(__base)                                   \
  default: break;                                                       \
}                                                                       \
                                                                return __base::DlgProc(hWnd, message, wParam, lParam); \
}

#define WINUI_BEGIN_NOTIFY_HANDLER()            \
  case WM_NOTIFY:                               \
  {                                             \
  LPNMHDR wmNMHDR = (LPNMHDR) lParam;           \
  switch (wmNMHDR->idFrom) {                    \

#define WINUI_END_NOTIFY_HANDLER()                      \
  default: break;                                       \
}                                                       \
                                                break;  \
}

#define WINUI_BEGIN_COMMAND_HANDLER()           \
  case WM_COMMAND:                              \
  {                                             \
  int wmId = LOWORD(wParam);                    \
  int wmEvent = HIWORD(wParam);
//      niPrintln(niFmt(_A("CMD >>> %d >> %d >> %d\n"),wmId,wmEvent,lParam));  \

#define WINUI_END_COMMAND_HANDLER()             \
  break;                                        \
}

#define WINUI_COMMAND_BEGIN_CTRL_HANDLER()     if (lParam) { switch (wmId) {
#define WINUI_COMMAND_CTRL_HANLDER(__id, __handler)   case __id: return __handler(wmEvent,lParam);
#define WINUI_COMMAND_END_CTRL_HANDLER()     } break; }

#define WINUI_COMMAND_BEGIN_MENU_HANDLER()     if (lParam == 0 && wmEvent == 0) { switch (wmId) {
#define WINUI_COMMAND_MENU_HANLDER(__id, __handler)   case __id: return __handler(lParam);
#define WINUI_COMMAND_END_MENU_HANDLER()     } break; }

#define WINUI_COMMAND_BEGIN_ACCEL_HANDLER()    if (lParam == 0 && wmEvent == 1){ switch (wmId) {
#define WINUI_COMMAND_ACCEL_HANLDER(__id, __handler)    case __id: return __handler(lParam);
#define WINUI_COMMAND_END_ACCEL_HANDLER()    } break; }

#define WINUI_MESSAGE_HANDLER(__message, __handler) case __message: return __handler(message, wParam, lParam);
#define WINUI_NOTIFY_HANDLER(__wmId, __handler)   case __wmId: return __handler(wmNMHDR->code);
#define WINUI_NOTIFY_HANDLER_EX(__wmId, __handler)  case __wmId: return __handler(wmNMHDR);

#define WINUI_COMMAND_OKCANCEL_HANDLERS()                               \
  if (wmId == IDOK && (lParam == 0 || lParam == (LPARAM)GetDlgItem(m_hWnd,IDOK)) && wmEvent == 0)   { EndDialog(m_hWnd, 1);  return 1; } \
  if (wmId == IDCANCEL && (lParam == 0 || lParam == (LPARAM)GetDlgItem(m_hWnd,IDCANCEL)) && wmEvent == 0) { EndDialog(m_hWnd, 0);  return 1; }

#define WINUI_COMMAND_OK_HANDLER()                                      \
  if (wmId == IDOK && (lParam == 0 || lParam == (LPARAM)GetDlgItem(m_hWnd,IDOK)) && wmEvent == 0)   { EndDialog(m_hWnd, 1);  return 1; }

#define WINUI_COMMAND_CANCEL_HANDLER()                                  \
  if (wmId == IDCANCEL && (lParam == 0 || lParam == (LPARAM)GetDlgItem(m_hWnd,IDCANCEL)) && wmEvent == 0) { EndDialog(m_hWnd, 0);  return 1; }

//////////////////////////////////////////////////////////////////////
// Warnings
//////////////////////////////////////////////////////////////////////

#pragma warning (push)

// C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#pragma warning (disable : 4290)

//////////////////////////////////////////////////////////////////////
// Debug / Error Handling
//////////////////////////////////////////////////////////////////////

//! \brief Triggers a user breakpoint
#ifdef ni64
#define WINUI_DBG_BREAK __debugbreak();
#else
#define WINUI_DBG_BREAK { __asm int 3 };
#endif

#ifdef _DEBUG
/*! \brief Similar to an assert, except the condition is still evaluated in release mode
  \sa niAssert
*/
#define WINUI_VERIFY(x) { if (!(x)) WINUI_DBG_BREAK }
#else
#define WINUI_VERIFY(x) (x)
#endif

/*! \brief WinUI base exception class
  \sa Throw(), WINUI_THROW
*/
class Exception
{
 public:
  Exception() throw() { };
  virtual ~Exception() throw() { };
  Exception(const TCHAR *pszDesc) throw() { m_strErrorDesc = pszDesc; };

  ni::cString m_strErrorDesc;  //!< Contains human readable error description
  ni::cString m_strSourceFile; //!< File in which the exception was thrown
  int m_iLine;                 //!< Line in which the exception was thrown

};

/*! \brief Throws Exception derived exception classes
  \remarks Macro WINUI_THROW fills in last two parameters automatically
  \sa Exception, WINUI_THROW
*/
template <class TException> WINUI_INL void Throw(TException& cException, const TCHAR *pszFile, int iLine)
{
  cException.m_strSourceFile = pszFile;
  cException.m_iLine = iLine;
  throw cException;
};

/*! \brief Calls Throw() with the file / line parameters filled
  \sa Throw()
*/
#define WINUI_THROW(ExceptionInstance, ExceptionType)                   \
  { Throw<ExceptionType>(ExceptionInstance, _T(__FILE__), __LINE__); }

//! \brief Returns error string for the last Win32 error
WINUI_INL ni::cString FormatLastWin32Err()
{
  ni::cString strErr;
  LPVOID lpMsgBuf = NULL;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
  strErr = (LPCTSTR) lpMsgBuf;
  LocalFree(lpMsgBuf);
  return strErr;
};

//! \brief Base Win32 exception class
class Win32Exception : public Exception
{
 public:
  Win32Exception()  { m_strErrorDesc = FormatLastWin32Err(); };
};

#ifndef WINUI_NO_LAST_ERROR_CHK
/*! \brief Calls CheckLastErr with the correct file and line parameters
  \sa CheckLastErr()
*/
#define WINUI_CHECK_LAST_ERR { WinUI::CheckLastErr(_T(__FILE__), __LINE__); }

// Optional use of exceptions
// #define WINUI_CHECK_LAST_ERR WINUI_THROW(Win32Exception(), Win32Exception)

/*! \brief Calls GetLastError() to determine if the last API call failed.
  Displays an error message dialog box in the case of failure
  \param pszFile Source file where function os being called from
  \param iLine Source line where function os being called from
  \sa WINUI_CHECK_LAST_ERR, FormatLastWin32Err()
*/
WINUI_INL void CheckLastErr(const TCHAR *pszFile, int iLine)
{
  TCHAR *pszMessage = NULL;
  ni::cString strLastErr = FormatLastWin32Err();
  pszMessage = niTMalloc(TCHAR,128 + _MAX_PATH + strLastErr.length());
  _stprintf(pszMessage, _T("The Win32 error\n%sNumber %i\n\nOccured in file\n%s\nLine %i"),
            strLastErr.c_str(), GetLastError(), pszFile, iLine);
  ::MessageBoxA(GetActiveWindow(), pszMessage, _T("WinUI Framework Error"), MB_ICONERROR);
  niSafeFree(pszMessage);
}
#else
#define WINUI_CHECK_LAST_ERR
#endif

//////////////////////////////////////////////////////////////////////
// Misc global functions
//////////////////////////////////////////////////////////////////////

/*! \brief Returns the path of the physical file the running module is implemented
  \remarks Uses static variable, not thread safe
  \return File path and name
*/
WINUI_INL const TCHAR * GetModulePath()
{
  HMODULE hModule;
  static TCHAR szPath[_MAX_PATH];
  hModule = GetModuleHandle(NULL);
  GetModuleFileNameA(hModule, szPath, _MAX_PATH);
  return (const TCHAR *) szPath;
}

WINUI_INL ni::cString LoadString(UINT id)
{
  ni::achar p[1024];
  ::LoadString(GetMsWinHInstance(), id, p, 1024*sizeof(ni::achar));
  return p;
}

/*! \brief Contains constants for all Win32 systems
  \sa GetOS()
*/
enum OSType
{
  // 9x based systems >= 0
  eWindows95 = 0,
  eWindows95_OSR2 = 1,
  eWindows98 = 2,
  eWindows98_SE = 3,
  eWindowsME = 4,
  // NT based systems >= 16
  eWindowsNT351 = 16,
  eWindowsNT4 = 17,
  eWindowsNT4_SP6 = 18,
  // 2000 based systems >= 32
  eWindows2000 = 32,
  eWindows2000_SP1 = 33,
  eWindows2000_SP2 = 34,
  eWindows2000_SP3_OrGreater = 35,
  // XP based systems >= 64
  eWindowsXP_Beta = 64,
  eWindowsXP = 65,
  eWindowsXP_SP1_OrGreater = 66,
  // .NET server or future systems >= 128
  eWindowsNETServer = 128,
  eUnknownOS_NT = 0xFFFFFFFF,
};

/*! \brief Determines the installed OS
  \return Constant describing the installed OS
  \sa OSType, IsOSNT()
*/
WINUI_INL OSType GetOS()
{
  OSVERSIONINFO OSVerInfo;
  OSVERSIONINFOEX OSVerInfoEx;
  OSVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  OSVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  GetVersionEx(&OSVerInfo);

  if (OSVerInfo.dwMajorVersion == 3)
  {
    if (OSVerInfo.dwMinorVersion == 51)
      return eWindowsNT351;
    else
      return eUnknownOS_NT;
  }
  else if (OSVerInfo.dwMajorVersion == 4)
  {
    if (OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
      if (OSVerInfo.dwMinorVersion > 0)
      {
        if (OSVerInfo.szCSDVersion[1] == 'A')
          return eWindows98_SE;
        else
          return eWindows98;
      }
      else
      {
        if (OSVerInfo.szCSDVersion[1] == 'C')
          return eWindows95_OSR2;
        else
          return eWindows95;
      }
    }
    else
    {
      if (OSVerInfo.dwMinorVersion == 0)
      {
        if (GetVersionEx((LPOSVERSIONINFO) &OSVerInfoEx))
        {
          if (OSVerInfoEx.wServicePackMajor == 6)
            return eWindowsNT4_SP6;
          else
            return eWindowsNT4;
        }
        else
          return eWindowsNT4;
      }
      else
        return eUnknownOS_NT;
    }
  }
  else if (OSVerInfo.dwMajorVersion == 5)
  {
    if (OSVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
      return eWindowsME;
    else
    {
      if (OSVerInfo.dwMinorVersion == 0)
      {
        OSVerInfoEx.wServicePackMajor = 0;
        GetVersionEx((LPOSVERSIONINFO) &OSVerInfoEx);
        if (OSVerInfoEx.wServicePackMajor == 0)
          return eWindows2000;
        else if (OSVerInfoEx.wServicePackMajor == 1)
          return eWindows2000_SP1;
        else if (OSVerInfoEx.wServicePackMajor == 2)
          return eWindows2000_SP2;
        else
          return eWindows2000_SP3_OrGreater;
      }
      else if (OSVerInfo.dwMinorVersion == 1)
      {
        if ((OSVerInfo.dwBuildNumber & 0xFFFF) < 2600)
          return eWindowsXP_Beta;
        else
        {
          OSVerInfoEx.wServicePackMajor = 0;
          GetVersionEx((LPOSVERSIONINFO) &OSVerInfoEx);
          if (OSVerInfoEx.wServicePackMajor == 0)
            return eWindowsXP;
          else
            return eWindowsXP_SP1_OrGreater;
        }
      }
      else if (OSVerInfo.dwMinorVersion == 2)
        return eWindowsNETServer;
      else
        return eUnknownOS_NT;
    }
  }
}

/*! \brief Determines if the installed OS is based on the NT kernel
  \return true if OS is NT based
  \sa GetOS()
*/
WINUI_INL bool IsOSNT() { return GetOS() >= eWindowsNT351; }

//! \brief Computes '(Col1 + Col2) / 2' with two 0xRRGGBBAA colors
WINUI_INL DWORD AverageRGBA(DWORD dwCol1, DWORD dwCol2)
{
  return (DWORD) (((((dwCol1 & 0xFF000000) >> 24) + ((dwCol2 & 0xFF000000) >> 24)) >> 1) << 24) |
      (((((dwCol1 & 0x00FF0000) >> 16) + ((dwCol2 & 0x00FF0000) >> 16)) >> 1) << 16) |
      (((((dwCol1 & 0x0000FF00) >>  8) + ((dwCol2 & 0x0000FF00) >>  8)) >> 1) <<  8) |
      (((((dwCol1 & 0x000000FF) >>  0) + ((dwCol2 & 0x000000FF) >>  0)) >> 1) <<  0);
}

/*! \brief Creates DLL version DWORD
  \sa GetDllVersion()
*/
#define WINUI_CREATE_DLL_VERSION(Major, Minor) MAKELONG(Minor, Major)

/*! \brief Obtains version information from a DLL
  \return DWORD version, use WINUI_CREATE_DLL_VERSION to create value for compare
  \sa WINUI_CREATE_DLL_VERSION
*/
WINUI_INL DWORD GetDllVersion(const TCHAR *pszDLLName)
{
  HINSTANCE hLibrary;
  DWORD dwVersion = 0;
  DLLGETVERSIONPROC pfnDLLGetVersion;
  DLLVERSIONINFO dvi;
  HRESULT hResult;

  hLibrary = LoadLibrary(pszDLLName);

  if(hLibrary != NULL)
  {
    pfnDLLGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hLibrary, "DllGetVersion");

    // Because some DLLs might not implement this function, you
    // must test for it explicitly. Depending on the particular
    // DLL, the lack of a DllGetVersion function can be a useful
    // indicator of the version
    if (pfnDLLGetVersion)
    {
      ZeroMemory(&dvi, sizeof(dvi));
      dvi.cbSize = sizeof(dvi);

      hResult = (* pfnDLLGetVersion) (&dvi);

      if (SUCCEEDED(hResult))
        dwVersion = WINUI_CREATE_DLL_VERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
    }

    FreeLibrary(hLibrary);
  }

  return dwVersion;
}

//! \brief Obtains the size of a file
WINUI_INL LONG GetFileSize(const TCHAR *pszFileName)
{
  FILE *hFile = NULL;
  LONG lPos = -1;

  hFile = _tfopen(pszFileName, _T("rb"));
  if (hFile == NULL)
    return -1;
  fseek(hFile, 0, SEEK_END);
  lPos = ftell(hFile);
  WINUI_VERIFY(fclose(hFile) == 0);

  return lPos;
}

//////////////////////////////////////////////////////////////////////
// Helper classes
//////////////////////////////////////////////////////////////////////

//! \brief Wrapper around a Win32 Font
struct Font
{
  HFONT hFont;

  Font(HFONT ahFont = NULL) : hFont(hFont) {}

  Font(int aiSize, const TCHAR* aszFace)
      : hFont(NULL)
  {
    Set(aiSize, aszFace);
  }

  void Set(int aiSize, const CHAR* aszFace)
  {
    hFont = ::CreateFontA(-(int)aiSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, aszFace);
  }

  ~Font()
  {
    ::DeleteObject(hFont);
    hFont = NULL;
  }

};

//! \brief Wrapper around a Win32 RECT
struct Rect : public RECT
{
  Rect()
  {
    left = 0;
    top = 0;
    right = 0;
    bottom = 0;
  };
  Rect(LONG _left, LONG _top, LONG _right, LONG _bottom)
  {
    left = _left;
    top = _top;
    right = _right;
    bottom = _bottom;
  };
  ~Rect() { };

  bool operator == (const Rect& rc)
  { return left == rc.left && right == rc.right && top == rc.top && bottom == rc.bottom; };

  int Width() const { return right-left; }
  int Height() const { return bottom-top; }

  void Translate(LONG x, LONG y)
  {
    left += x;
    top += y;
    right += x;
    bottom += y;
  }
};

//////////////////////////////////////////////////////////////////////
// Misc Win32 wrappers
//////////////////////////////////////////////////////////////////////

//! \brief Solid Brush wrapper
class SolidBrush {
  HBRUSH mhBrush;
 public:
  SolidBrush(int r, int g, int b, int a) {
    mhBrush = ::CreateSolidBrush(RGB(r,g,b));
  }
  ~SolidBrush() {
    if (mhBrush) {
      DeleteObject(mhBrush);
      mhBrush = NULL;
    }
  }
  HBRUSH operator * () const { return mhBrush; }
};

//! \brief Device context (DC) wrapper
class DC
{
 public:
  DC() { m_hDC = NULL; };
  DC(HWND hWnd) { m_hDC = ::GetDC(hWnd); };
  DC(HDC hDC) { m_hDC = hDC; };
  virtual ~DC() { };

  void DrawLine(UINT iX1, UINT iY1, UINT iX2, UINT iY2, COLORREF clrColor)
  {
    HPEN hPen;
    HGDIOBJ hObj = NULL;

    hPen = ::CreatePen(PS_SOLID, 1, clrColor);
    hObj = SelectObject(hPen);
    ::MoveToEx(m_hDC, iX1, iY1, NULL);
    ::LineTo(m_hDC, iX2, iY2);
    SelectObject(hObj);
    ::DeleteObject(hPen);
  };

  void MoveTo(UINT iX, UINT iY) { ::MoveToEx(m_hDC, iX, iY, NULL); };

  void LineTo(UINT iX, UINT iY, COLORREF clrColor)
  {
    HPEN hPen;
    HGDIOBJ hObj = NULL;

    hPen = ::CreatePen(PS_SOLID, 1, clrColor);
    hObj = SelectObject(hPen);
    ::LineTo(m_hDC, iX, iY);
    SelectObject(hObj);
    ::DeleteObject(hPen);
  };

  void LineTo(UINT iX, UINT iY) { ::LineTo(m_hDC, iX, iY); };

  //! \remarks Caller is responsible for deleting font
  HFONT CreateAndSelectFont(const CHAR *pszFace, UINT iSize)
  {
    HFONT hFont = ::CreateFontA(-(int) iSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, pszFace);
    SelectObject(hFont);
    return hFont;
  };

  void CreateCompatible(HDC hDC) { m_hDC = ::CreateCompatibleDC(hDC); };

  void Delete()
  {
    if (m_hDC)
    {
      WINUI_VERIFY(DeleteDC(m_hDC) == TRUE);
      m_hDC = NULL;
    }
  };

  int DrawText(const TCHAR *pszString, Rect *rcBounds,
               UINT iFormat = DT_LEFT | DT_END_ELLIPSIS)
  {
#ifdef _UNICODE
    return ::DrawTextW(m_hDC, pszString, -1, rcBounds, iFormat);
#else
    return ::DrawTextA(m_hDC, pszString, -1, rcBounds, iFormat);
#endif
  };

  BOOL TextOut(const TCHAR *pszString, UINT iX, UINT iY)
  {
    return ::TextOut(m_hDC, iX, iY, pszString, (UINT)_tcslen(pszString));
  };

  BOOL SetPixel(UINT iX, UINT iY, COLORREF clrColor)
  {
    return ::SetPixelV(m_hDC, iX, iY, clrColor);
  }

  HDC GetDC() const { return m_hDC; };

  int FillRect(const Rect& rc, HBRUSH hBrush)
  { return ::FillRect(m_hDC, &rc, hBrush); };
  BOOL DrawFrameControl(Rect *pRect, UINT iType, UINT iState)
  { return ::DrawFrameControl(m_hDC, pRect, iType, iState); };
  HGDIOBJ SelectObject(HGDIOBJ hObj)
  { return ::SelectObject(m_hDC, hObj); };
  int SetBkMode(INT iMode)
  { return ::SetBkMode(m_hDC, iMode); };
  COLORREF SetTextColor(COLORREF clrColor)
  { return ::SetTextColor(m_hDC, clrColor); };
  COLORREF SetBkColor(COLORREF clrColor)
  { return ::SetBkColor(m_hDC, clrColor); };
  BOOL Ellipse(int iLeftRect, int iTopRect, int iRightRect, int iBottomRect)
  { return ::Ellipse(m_hDC, iLeftRect, iTopRect, iRightRect, iBottomRect); };

 protected:
  HDC m_hDC;

};

// \brief Bitmap wrapper
class Bitmap
{
 public:
  Bitmap() { m_hBitmap = NULL; };
  explicit Bitmap(HBITMAP hBitmap) { m_hBitmap = hBitmap; };
  virtual ~Bitmap() { this->Delete(); };

  void Delete() {
    if (m_hBitmap) {
      DeleteObject((HGDIOBJ) m_hBitmap);
      m_hBitmap = NULL;
    }
  }

  void Attach(HBITMAP hBitmap)
  {
    this->Delete();
    m_hBitmap = hBitmap;
  };

  BOOL LoadFromResource(INT iResourceID)
  {
#ifdef _UNICODE
    HBITMAP hBitmap = LoadBitmapW(GetMsWinHInstance(), MAKEINTRESOURCE(iResourceID));
#else
    HBITMAP hBitmap = LoadBitmapA(GetMsWinHInstance(), MAKEINTRESOURCE(iResourceID));
#endif
    if (hBitmap == NULL)
      return FALSE;
    Attach(hBitmap);
    return TRUE;
  };

  BOOL CreateCompatible(HDC hDC, UINT iWidth, UINT iHeight)
  {
    HBITMAP hBmp = ::CreateCompatibleBitmap(hDC, iWidth, iHeight);
    if (hBmp == NULL)
      return FALSE;
    Attach(hBmp);
    return TRUE;
  };

  BOOL Blt(HDC hDCDest, UINT iX, UINT iY)
  {
    DC hDCSrc;

    hDCSrc.CreateCompatible(hDCDest);
    hDCSrc.SelectObject((HGDIOBJ) m_hBitmap);

    if (!BitBlt(hDCDest, iX, iY, GetWidth(), GetHeight(), hDCSrc.GetDC(), 0, 0, SRCCOPY))
      return FALSE;

    hDCSrc.Delete();

    return TRUE;

  };

  UINT GetWidth()
  {
    BITMAP bmp;
    if (niWin32API(GetObject)((HGDIOBJ) m_hBitmap, sizeof(BITMAP), (LPVOID) &bmp) == 0)
      WINUI_CHECK_LAST_ERR
          return (UINT) bmp.bmWidth;
  };

  UINT GetHeight()
  {
    BITMAP bmp;
    if (niWin32API(GetObject)((HGDIOBJ) m_hBitmap, sizeof(BITMAP), (LPVOID) &bmp) == 0)
      WINUI_CHECK_LAST_ERR
          return (UINT) bmp.bmHeight;
  };

  void Detach() { m_hBitmap = NULL; };
  HBITMAP GetBitmap() const { return m_hBitmap; };

 protected:
  HBITMAP m_hBitmap;
};

//! \brief DIB wrapper
class DIB {
 public:
  DIB(HDC hdc = NULL, ni::tU32 anWidth = 0, ni::tU32 anHeight = 0, ni::tU32 anBpp = 0)
  {
    if (!hdc || !anBpp || !anWidth || !anHeight) return;
    Create(hdc,anBpp,anWidth,anHeight);
  }
  void Create(HDC hdc, ni::tU32 anWidth, ni::tU32 anHeight, ni::tU32 anBpp)
  {
    Delete();
    mnWidth = anWidth;
    mnHeight = anHeight;
    mnBpp = anBpp;
    mpBits = NULL;
    BITMAPINFOHEADER bih = {0};
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biBitCount = (WORD)anBpp;
    bih.biCompression = BI_RGB;
    bih.biPlanes = 1;
    bih.biWidth = anWidth;
    bih.biHeight = anHeight;
    mwinDC.CreateCompatible(hdc);
    mwinBmp.Attach(CreateDIBSection(hdc, (BITMAPINFO *)&bih, DIB_RGB_COLORS, (void **)&mpBits, NULL, 0x0));
    mwinDC.SelectObject(mwinBmp.GetBitmap());
  }
  ~DIB() {
    Delete();
  }

  void Delete() {
    mwinDC.Delete();
    mwinBmp.Delete();
  }

  ni::tBool __stdcall IsOK() const {
    return mwinBmp.GetBitmap() != NULL && mpBits != NULL;
  }

  HDC __stdcall GetHDC() const {
    return mwinDC.GetDC();
  }
  HBITMAP __stdcall GetHBITMAP() const {
    return mwinBmp.GetBitmap();
  }

  ni::tU32 GetWidth() const { return mnWidth; }
  ni::tU32 GetHeight() const { return mnHeight; }
  ni::tU32 GetBpp() const { return mnBpp; }
  void* GetBits() const { return mpBits; }

  WinUI::DC            mwinDC;
  WinUI::Bitmap        mwinBmp;
  void*                mpBits;
  ni::tU32      mnBpp;
  ni::tU32      mnWidth;
  ni::tU32      mnHeight;
};

//! \brief Image list wrapper
class ImageList
{
 public:
  ImageList() { m_hImgLst = NULL; };
  virtual ~ImageList()
  {
    if (m_hImgLst)
      WINUI_VERIFY(ImageList_Destroy(m_hImgLst));
  };

  bool Create(UINT iFlags = ILC_COLOR, UINT iCX = 16, UINT iCY = 16, UINT iMaxItems = 32)
  {
    niAssert(m_hImgLst == NULL);
    m_hImgLst = ImageList_Create(iCX, iCY, iFlags, 0, iMaxItems);
    if (m_hImgLst == NULL)
    {
      WINUI_CHECK_LAST_ERR
          return false;
    }
    return true;
  };

  /*! \brief Creates an image list from a bitmap
    \param iBitmapID Resource ID of the bitmap
    \param iCX Width of the individual images
    \return true in case of success
  */
  bool CreateFromBitmap(UINT iBitmapID, UINT iCX)
  {
    niAssert(m_hImgLst == NULL);
    m_hImgLst = ImageList_LoadBitmap(GetMsWinHInstance(),
                                     MAKEINTRESOURCE(iBitmapID), iCX, 32, 0x00FF00FF);
    if (m_hImgLst == NULL)
    {
      WINUI_CHECK_LAST_ERR
          return false;
    }
    return true;
  };

  bool AddImage(DWORD dwResource)
  {
    niAssert(m_hImgLst);
#ifdef _UNICODE
    HBITMAP hBmp = LoadBitmapW(GetMsWinHInstance(), MAKEINTRESOURCE(dwResource));
#else
    HBITMAP hBmp = LoadBitmapA(GetMsWinHInstance(), MAKEINTRESOURCE(dwResource));
#endif
    if (hBmp == NULL)
    {
      WINUI_CHECK_LAST_ERR
          return false;
    }
    ImageList_Add(m_hImgLst, hBmp, NULL);
    WINUI_VERIFY(DeleteObject(hBmp));
  };

  HIMAGELIST GetHandle() { return m_hImgLst; };

  inline int         GetImageCount() const { return ImageList_GetImageCount(m_hImgLst); }
  inline BOOL        SetImageCount(UINT uNewCount) { return ImageList_SetImageCount(m_hImgLst, uNewCount); }
  inline int         Add(HBITMAP hbmImage, HBITMAP hbmMask) { return ImageList_Add(m_hImgLst, hbmImage, hbmMask); }
  inline int         ReplaceIcon(int i, HICON hicon) { return ImageList_ReplaceIcon(m_hImgLst, i, hicon); }
  inline COLORREF    SetBkColor(COLORREF clrBk) { return ImageList_SetBkColor(m_hImgLst, clrBk); }
  inline COLORREF    GetBkColor() const { return ImageList_GetBkColor(m_hImgLst); }
  inline BOOL        SetOverlayImage(int iImage, int iOverlay) { return ImageList_SetOverlayImage(m_hImgLst, iImage, iOverlay); }
  inline BOOL      Draw(int i, HDC hdcDst, int x, int y, UINT fStyle) const { return ImageList_Draw(m_hImgLst, i, hdcDst, x, y, fStyle); }
  inline BOOL        Replace(int i, HBITMAP hbmImage, HBITMAP hbmMask) { return ImageList_Replace(m_hImgLst, i, hbmImage, hbmMask); }
  inline int         AddMasked(HBITMAP hbmImage, COLORREF crMask) { return ImageList_AddMasked(m_hImgLst, hbmImage, crMask); }
  inline BOOL        DrawEx(int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle) const { return ImageList_DrawEx(m_hImgLst, i, hdcDst, x, y, dx, dy, rgbBk, rgbFg, fStyle); }
  inline BOOL        DrawIndirect(IMAGELISTDRAWPARAMS* pimldp) const { return ImageList_DrawIndirect(pimldp); }
  inline BOOL        Remove(int i) { return ImageList_Remove(m_hImgLst, i); }
  inline HICON       GetIcon(int i, UINT flags) const { return ImageList_GetIcon(m_hImgLst, i, flags); }
  inline BOOL        Copy(int iDst, HIMAGELIST himlSrc, int iSrc, UINT uFlags) { return ImageList_Copy(m_hImgLst, iDst, himlSrc, iSrc, uFlags); }
  inline BOOL        BeginDrag(int iTrack, int dxHotspot, int dyHotspot) { return ImageList_BeginDrag(m_hImgLst, iTrack, dxHotspot, dyHotspot); }
  inline void        EndDrag() { ImageList_EndDrag(); }
  inline BOOL        DragEnter(HWND hwndLock, int x, int y) { return ImageList_DragEnter(hwndLock, x, y); }
  inline BOOL        DragLeave(HWND hwndLock) { return ImageList_DragLeave(hwndLock); }
  inline BOOL        DragMove(int x, int y) { return ImageList_DragMove(x, y); }
  inline BOOL        SetDragCursorImage(int iDrag, int dxHotspot, int dyHotspot) { return ImageList_SetDragCursorImage(m_hImgLst, iDrag, dxHotspot, dyHotspot); }
  inline BOOL        DragShowNolock(BOOL fShow) { return ImageList_DragShowNolock(fShow); }
  inline HIMAGELIST  GetDragImage(POINT FAR* ppt,POINT FAR* pptHotspot) { return ImageList_GetDragImage(ppt,pptHotspot); }
  inline BOOL      RemoveAll() { return Remove(-1); }
  inline HICON     ExtractIcon(int i)  { return GetIcon(i, 0); }
  inline BOOL        GetIconSize(int FAR *cx, int FAR *cy) { return ImageList_GetIconSize(m_hImgLst,cx,cy); }
  inline BOOL        SetIconSize(int cx, int cy) { return ImageList_SetIconSize(m_hImgLst,cx,cy); }
  inline BOOL        GetImageInfo(int i, IMAGEINFO FAR* pImageInfo) { return ImageList_GetImageInfo(m_hImgLst,i,pImageInfo); }
  inline HIMAGELIST  Merge(int i1, HIMAGELIST himl2, int i2, int dx, int dy) { return ImageList_Merge(m_hImgLst,i1,himl2,i2,dx,dy); }
  inline HIMAGELIST  Duplicate() { return ImageList_Duplicate(m_hImgLst); }

 protected:
  HIMAGELIST m_hImgLst;

};

//! \brief Registry access
class Registry
{
 public:
  Registry() { };
  virtual ~Registry() { };

  /*! \brief Reads a DWORD from the registry
    \param pszKey Path of the key
    \param pszValue Name of the value
    \param dwValOut Receives the read DWORD
    \param dwDefault Optional default value in case the requested value is not found
    \param hBaseKey Which root section of the registry to look in
    \return TRUE in case of success
  */
  BOOL ReadDWORD(LPCTSTR pszKey, LPCTSTR pszValue, DWORD& dwValOut,
                 DWORD dwDefault = 0, HKEY hBaseKey = HKEY_CURRENT_USER)
  {
    HKEY hKey = NULL;
    DWORD dwType;
    DWORD dwSize = sizeof(DWORD);
    dwValOut = dwDefault;
    if (RegOpenKeyEx(hBaseKey, pszKey, 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return FALSE;
    }
    if (RegQueryValueEx(hKey, pszValue, 0, &dwType, (LPBYTE) &dwValOut, &dwSize) != ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return FALSE;
    }
    if (dwType != REG_DWORD)
    {
      RegCloseKey(hKey);
      return FALSE;
    }
    RegCloseKey(hKey);
    return TRUE;
  };

  BOOL WriteDWORD(LPCTSTR pszKey, LPCTSTR pszValue, DWORD dwValue, HKEY hBaseKey = HKEY_CURRENT_USER)
  {
    HKEY hKey = NULL;
    DWORD dwDisp;
    if (RegCreateKeyEx(hBaseKey, pszKey, 0, NULL, NULL, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp) != ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return FALSE;
    }
    if (RegSetValueEx(hKey, pszValue, 0, REG_DWORD, (LPBYTE) &dwValue, sizeof(DWORD)) != ERROR_SUCCESS)
    {
      RegCloseKey(hKey);
      return FALSE;
    }
    RegCloseKey(hKey);
    return TRUE;
  };
};

/*! \brief Clipboard wrapper
  \sa ClipboardViewer
*/
class Clipboard
{
 public:
  /*! \brief Constructor, optionally opens the clipboard
    \param bOpen If true, clipboard is automatically opened
  */
  Clipboard(bool bOpen = true)
  {
    m_bClipOpened = false;
    if (bOpen) {
      WINUI_VERIFY(OpenClipboard());
    }
  };

  virtual ~Clipboard()
  {
    if (m_bClipOpened) {
      CloseClipboard();
    }
  };

  /*! \brief Opens the clipboard
    \return true in case of success
    \sa CloseClipboard()
  */
  bool OpenClipboard()
  {
    BOOL bSuccess;
    niAssert(m_bClipOpened == false);
    bSuccess = ::OpenClipboard(GetActiveWindow());
    if (bSuccess)
    {
      m_bClipOpened = true;
      return true;
    }
    return false;
  };

  bool CloseClipboard()
  {
    BOOL bSuccess;
    niAssert(m_bClipOpened == true);
    bSuccess = ::CloseClipboard();
    if (bSuccess)
    {
      m_bClipOpened = false;
      return true;
    }
    return false;
  };

  void EmptyClipboard() {
    if (!m_bClipOpened) return;
    ::EmptyClipboard();
  }

  int GetPriorityClipboardFormat() const
  {
    UINT iClipFormats [] =
        { CF_UNICODETEXT,
          CF_TEXT,
          CF_OEMTEXT,
          CF_BITMAP,
          CF_METAFILEPICT,
          CF_SYLK,
          CF_DIF,
          CF_TIFF,
          CF_DIB,
          CF_PALETTE,
          CF_PENDATA,
          CF_RIFF,
          CF_WAVE,
          CF_ENHMETAFILE,
          CF_HDROP,
          CF_LOCALE,
          CF_DIBV5
        };

    return ::GetPriorityClipboardFormat(iClipFormats, sizeof(iClipFormats) / sizeof(UINT));
  };

  SIZE_T GetClipboardDataSize() {
    if (!m_bClipOpened)
      return 0;

    int iClipFormat;
    HANDLE hClipData;

    iClipFormat = GetPriorityClipboardFormat();
    if (iClipFormat <= 0)
      return 0;

    hClipData = ::GetClipboardData((UINT) iClipFormat);
    if (hClipData == NULL) {
      WINUI_CHECK_LAST_ERR
          return 0;
    }

    return GlobalSize(hClipData);
  }

  ///////////////////////////////////////////////
  // 0 clip format == auto-detect -> goes in iClipFormat
  bool ClipboardDataReadLock(HANDLE& hClipData, LPVOID& pLock, SIZE_T& iDataSize, int& iClipFormat)
  {
    if (!m_bClipOpened)
      return false;

    if (iClipFormat == 0) {
      iClipFormat = GetPriorityClipboardFormat();
      if (iClipFormat <= 0)
        return false;
    }

    hClipData = ::GetClipboardData((UINT) iClipFormat);
    if (hClipData == NULL) {
      return false;
    }

    iDataSize = GlobalSize(hClipData);

    pLock = GlobalLock(hClipData);
    if (pLock == NULL) {
      return false;
    }

    return true;
  }
  bool ClipboardDataReadUnlock(HANDLE& hClipData) {
    if (hClipData) {
      GlobalUnlock(hClipData);
    }
    return true;
  }

#define CB_READ_OPEN(CF)                                              \
  HANDLE hClipData = NULL;                                            \
  PVOID pLock = NULL;                                                 \
  SIZE_T iDataSize = 0;                                               \
  int iClipFormat = CF;                                               \
  if (ClipboardDataReadLock(hClipData,pLock,iDataSize,iClipFormat)) {

  // Returns false if pLock is null, else true
#define CB_READ_CLOSE                           \
  ClipboardDataReadUnlock(hClipData);           \
  bRet = true;                                  \
} else { bRet = false; }

#define CB_OPEN_IFNOT_BEGIN                     \
  bool bRet = true;                             \
  BOOL __closeClip = FALSE;                     \
  if (!m_bClipOpened) {                         \
    __closeClip = TRUE;                         \
    if (!OpenClipboard())                       \
      return false;                             \
  }

#define CB_OPEN_IFNOT_END                       \
  if (__closeClip) {                            \
    CloseClipboard();                           \
  }                                             \
  return bRet;

  ///////////////////////////////////////////////
  bool GetClipboardData(PVOID pBuffer, SIZE_T& iBufLen)
  {
    CB_OPEN_IFNOT_BEGIN
        CB_READ_OPEN(0)

        if (iDataSize > iBufLen) {
          iBufLen = iDataSize;
          bRet = false;
        }
        else {
          iBufLen = iDataSize;
          memcpy(pBuffer, pLock, iDataSize);
        }

    CB_READ_CLOSE
        CB_OPEN_IFNOT_END
        }

  ///////////////////////////////////////////////
  BOOL SetClipboardData(INT Type, LPVOID pBuffer, SIZE_T iBufLen) {
    niAssert(m_bClipOpened);

    EmptyClipboard();

    BOOL bRet = FALSE;
    HGLOBAL hClipboardData;
    hClipboardData = GlobalAlloc(GMEM_MOVEABLE, iBufLen);
    if (hClipboardData) {
      void* pData = GlobalLock(hClipboardData);
      if (pData) {
        memcpy(pData,pBuffer,iBufLen);
        GlobalUnlock(hClipboardData);
        bRet = ::SetClipboardData(Type,hClipboardData) != NULL;
      }
      else {
        GlobalFree(hClipboardData);
      }
    }

    return bRet;
  }

  ///////////////////////////////////////////////
  bool SetClipboardText(const ni::cchar* aaszText, int len = -1) {
    CB_OPEN_IFNOT_BEGIN
        ni::Windows::UTF16Buffer wText;
    niWin32_UTF8ToUTF16(wText,aaszText);
    len = (int)wcslen(wText.begin());
    if (!SetClipboardData(CF_UNICODETEXT,(LPVOID)wText.data(),(len+1)*sizeof(WCHAR)))
      bRet = false;
    CB_OPEN_IFNOT_END
        }
  bool SetClipboardText(const ni::uchar* aaszText, int len = -1) {
    CB_OPEN_IFNOT_BEGIN
        if (len < 0) len = (int)wcslen(aaszText);
    if (!SetClipboardData(CF_UNICODETEXT,(LPVOID)aaszText,(len+1)*sizeof(ni::uchar)))
      bRet = false;
    CB_OPEN_IFNOT_END
        }
  bool GetClipboardText(ni::cString& astrOut) {
    CB_OPEN_IFNOT_BEGIN
    {
      CB_READ_OPEN(CF_UNICODETEXT)
          astrOut = (ni::uchar*)pLock;
      CB_READ_CLOSE
          }
    if (bRet)
      return bRet;
    {
      CB_READ_OPEN(CF_TEXT)
          astrOut = (ni::cchar*)pLock;
      CB_READ_CLOSE
          }
    CB_OPEN_IFNOT_END
        }

#undef CB_READ_OPEN
#undef CB_READ_CLOSE
#undef CB_OPEN_IFNOT_END
#undef CB_OPEN_IFNOT_BEGIN

 protected:
  bool m_bClipOpened;

};

//! \brief Menu wrapper
class Menu
{
 public:
  Menu() { m_hMenu = NULL; };
  Menu(HMENU ahMenu, BOOL bDestroy) { m_hMenu = ahMenu; mbDestroy = bDestroy; };
  virtual ~Menu() { Destroy(); };

  void Destroy()
  {
    if (m_hMenu && mbDestroy)
      WINUI_VERIFY(DestroyMenu(m_hMenu));
  };

  BOOL CreateEmpty()
  {
    Destroy();
    m_hMenu = CreateMenu();
  };

  BOOL CreateFromResource(int iResource)
  {
    Destroy();

    m_hMenu = ::LoadMenu(GetMsWinHInstance(), MAKEINTRESOURCE(iResource));
    if (!m_hMenu)
    {
      WINUI_CHECK_LAST_ERR
          return FALSE;
    }

    return TRUE;
  };

  int TrackPopupMenu(UINT iX, UINT iY, UINT iFlags, HWND hWndParent = NULL)
  {
    if (hWndParent == NULL)
      hWndParent = GetDesktopWindow();
    // Win32 Bug: You need to call SetForegroundWindow()
    //SetForegroundWindow(hWndParent);
    return ::TrackPopupMenu(m_hMenu, iFlags, iX, iY, 0, hWndParent, NULL);
  };

  HMENU GetMenu() { return m_hMenu; };

  BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem = 0, LPCTSTR lpszNewItem = NULL)
  { niAssert(::IsMenu(m_hMenu)); return ::AppendMenu(m_hMenu, nFlags, nIDNewItem, lpszNewItem); }
  BOOL AppendMenu(UINT nFlags, UINT_PTR nIDNewItem, const Bitmap* pBmp)
  { niAssert(::IsMenu(m_hMenu)); return ::AppendMenu(m_hMenu, nFlags | MF_BITMAP, nIDNewItem,
                                                     (LPCTSTR)pBmp->GetBitmap()); }
  UINT CheckMenuItem(UINT nIDCheckItem, UINT nCheck)
  { niAssert(::IsMenu(m_hMenu)); return (UINT)::CheckMenuItem(m_hMenu, nIDCheckItem, nCheck); }
  UINT EnableMenuItem(UINT nIDEnableItem, UINT nEnable)
  { niAssert(::IsMenu(m_hMenu)); return ::EnableMenuItem(m_hMenu, nIDEnableItem, nEnable); }
  BOOL SetDefaultItem(UINT uItem, BOOL fByPos)
  { niAssert(::IsMenu(m_hMenu)); return ::SetMenuDefaultItem(m_hMenu, uItem, fByPos); }
  UINT GetDefaultItem(UINT gmdiFlags, BOOL fByPos)
  { niAssert(::IsMenu(m_hMenu)); return ::GetMenuDefaultItem(m_hMenu, fByPos, gmdiFlags); }
  UINT GetMenuItemCount() const
  { niAssert(::IsMenu(m_hMenu)); return ::GetMenuItemCount(m_hMenu); }
  UINT GetMenuItemID(int nPos) const
  { niAssert(::IsMenu(m_hMenu)); return ::GetMenuItemID(m_hMenu, nPos); }
  UINT GetMenuState(UINT nID, UINT nFlags) const
  { niAssert(::IsMenu(m_hMenu)); return ::GetMenuState(m_hMenu, nID, nFlags); }
  int GetMenuString(UINT nIDItem, LPTSTR lpString, int nMaxCount, UINT nFlags) const
  { niAssert(::IsMenu(m_hMenu)); return ::GetMenuString(m_hMenu, nIDItem, lpString, nMaxCount, nFlags); }
  BOOL GetMenuItemInfo(UINT uItem, LPMENUITEMINFO lpMenuItemInfo, BOOL fByPos)
  { niAssert(::IsMenu(m_hMenu)); niAssert(lpMenuItemInfo != NULL);
    return ::GetMenuItemInfo(m_hMenu, uItem, fByPos, lpMenuItemInfo); }
  BOOL SetMenuItemInfo(UINT uItem, LPMENUITEMINFO lpMenuItemInfo, BOOL fByPos)
  { niAssert(::IsMenu(m_hMenu)); niAssert(lpMenuItemInfo != NULL);
    return ::SetMenuItemInfo(m_hMenu, uItem, fByPos, lpMenuItemInfo); }
  HMENU GetSubMenu(int nPos) const
  { niAssert(::IsMenu(m_hMenu)); return ::GetSubMenu(m_hMenu, nPos); }
  BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem)
  { niAssert(::IsMenu(m_hMenu)); return ::InsertMenu(m_hMenu, nPosition, nFlags, nIDNewItem, lpszNewItem); }
  BOOL InsertMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, const Bitmap* pBmp)
  { niAssert(::IsMenu(m_hMenu)); return ::InsertMenu(m_hMenu, nPosition, nFlags | MF_BITMAP, nIDNewItem, (LPCTSTR)pBmp->GetBitmap()); }
  BOOL InsertMenuItem(UINT uItem, LPMENUITEMINFO lpMenuItemInfo, BOOL fByPos)
  { niAssert(::IsMenu(m_hMenu)); niAssert(lpMenuItemInfo);
    ::InsertMenuItem(m_hMenu, uItem, fByPos, lpMenuItemInfo); }
  BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem)
  { niAssert(::IsMenu(m_hMenu)); return ::ModifyMenu(m_hMenu, nPosition, nFlags, nIDNewItem, lpszNewItem); }
  BOOL ModifyMenu(UINT nPosition, UINT nFlags, UINT_PTR nIDNewItem, const Bitmap* pBmp)
  { niAssert(::IsMenu(m_hMenu)); return ::ModifyMenu(m_hMenu, nPosition, nFlags | MF_BITMAP, nIDNewItem, (LPCTSTR)pBmp->GetBitmap()); }
  BOOL RemoveMenu(UINT nPosition, UINT nFlags)
  { niAssert(::IsMenu(m_hMenu)); return ::RemoveMenu(m_hMenu, nPosition, nFlags); }
  BOOL SetMenuItemBitmaps(UINT nPosition, UINT nFlags, const Bitmap* pBmpUnchecked, const Bitmap* pBmpChecked)
  { niAssert(::IsMenu(m_hMenu)); return ::SetMenuItemBitmaps(m_hMenu, nPosition, nFlags, (HBITMAP)pBmpUnchecked->GetBitmap(), (HBITMAP)pBmpChecked->GetBitmap()); }
  BOOL LoadMenu(LPCTSTR lpszResourceName)
  { m_hMenu = ::LoadMenu(GetMsWinHInstance(), lpszResourceName); return m_hMenu != NULL; }
  BOOL LoadMenu(UINT nIDResource)
  { m_hMenu = ::LoadMenu(GetMsWinHInstance(), MAKEINTRESOURCE(nIDResource)); return m_hMenu != NULL; }
  BOOL LoadMenuIndirect(const MENUTEMPLATE* lpMenuTemplate)
  { m_hMenu = ::LoadMenuIndirect(lpMenuTemplate); return m_hMenu != NULL; }
  BOOL SetMenuContextHelpId(DWORD dwContextHelpId)
  { return ::SetMenuContextHelpId(m_hMenu, dwContextHelpId); }
  DWORD GetMenuContextHelpId() const
  { return ::GetMenuContextHelpId(m_hMenu); }
  BOOL CheckMenuRadioItem(UINT nIDFirst, UINT nIDLast, UINT nIDItem, UINT nFlags)
  { return ::CheckMenuRadioItem(m_hMenu, nIDFirst, nIDLast, nIDItem, nFlags); }

 protected:
  HMENU m_hMenu;
  BOOL mbDestroy;
};

//! \brief Wrapper around the open common dialog
class OpenFile
{
 public:
  OpenFile() { m_szFileNameOut[0] = '\0'; };
  virtual ~OpenFile() { };

  bool GetFile( const TCHAR *pszDlgTitle = _T("Open"),
                const TCHAR* pszFilter = _T("All Files (*.*)\0*.*\0\0"),
                const TCHAR* pszDefExt = _T("*.*"),
                const TCHAR *pszFile = NULL,
                const TCHAR *pszInitDir = NULL,
                HWND hWndParent = NULL,
                DWORD dwFlags = 0)
  {
    OPENFILENAME ofn;
    TCHAR szInitDir[_MAX_PATH];

    if (pszInitDir == NULL)
      GetModuleFileNameA(GetModuleHandle(NULL), szInitDir, _MAX_PATH);
    else
      _tcscpy(szInitDir, pszInitDir);

    if (pszFile)
      _tcscpy(m_szFileNameOut, pszFile);

    ofn.lStructSize     = sizeof(OPENFILENAME);
    ofn.hwndOwner     = hWndParent;
    ofn.hInstance     = NULL;
    ofn.lpstrFilter     = pszFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex    = 0;
    ofn.nMaxFile      = _MAX_PATH;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle   = _MAX_PATH;
    ofn.lpstrInitialDir   = szInitDir;
    ofn.lpstrTitle      = pszDlgTitle;
    ofn.Flags       = 0;
    ofn.nFileOffset     = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt     = pszDefExt;
    ofn.lCustData     = 0L;
    ofn.lpfnHook      = NULL;
    ofn.lpTemplateName    = NULL;
    ofn.hwndOwner         = hWndParent;
    ofn.lpstrFile         = m_szFileNameOut;
    ofn.Flags           = dwFlags;

    return ::GetOpenFileName(&ofn) == TRUE;
  };

  const TCHAR* GetFileName() const { return m_szFileNameOut; };

 protected:
  TCHAR m_szFileNameOut[_MAX_PATH];

};

//! \brief Wrapper around the save common dialog
class SaveFile
{
 public:
  SaveFile() { m_szFileNameOut[0] = '\0'; };
  virtual ~SaveFile() { };

  bool GetFile( const TCHAR *pszDlgTitle = _T("Save As..."),
                const TCHAR* pszFilter = _T("All Files (*.*)\0*.*\0\0"),
                const TCHAR* pszDefExt = _T("*.*"),
                const TCHAR *pszFile = NULL,
                const TCHAR *pszInitDir = NULL,
                HWND hWndParent = NULL,
                DWORD dwFlags = 0)
  {
    OPENFILENAME ofn;
    TCHAR szInitDir[_MAX_PATH];

    if (pszInitDir == NULL)
      GetModuleFileNameA(GetModuleHandle(NULL), szInitDir, _MAX_PATH);
    else
      _tcscpy(szInitDir, pszInitDir);

    if (pszFile)
      _tcscpy(m_szFileNameOut, pszFile);

    ofn.lStructSize     = sizeof(OPENFILENAME);
    ofn.hwndOwner     = hWndParent;
    ofn.hInstance     = NULL;
    ofn.lpstrFilter     = pszFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter    = 0;
    ofn.nFilterIndex    = 0;
    ofn.nMaxFile      = _MAX_PATH;
    ofn.lpstrFileTitle    = NULL;
    ofn.nMaxFileTitle   = _MAX_PATH;
    ofn.lpstrInitialDir   = szInitDir;
    ofn.lpstrTitle      = pszDlgTitle;
    ofn.Flags       = 0;
    ofn.nFileOffset     = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt     = pszDefExt;
    ofn.lCustData     = 0L;
    ofn.lpfnHook      = NULL;
    ofn.lpTemplateName    = NULL;
    ofn.hwndOwner         = hWndParent;
    ofn.lpstrFile         = m_szFileNameOut;
    ofn.Flags           = dwFlags;

    return ::GetSaveFileName(&ofn) == TRUE;
  };

  const TCHAR* GetFileName() const { return m_szFileNameOut; };

 protected:
  TCHAR m_szFileNameOut[_MAX_PATH];

};

//! \brief Wrapper around SHBrowseForFolder()
class BrowseForFolder
{
 public:
  BrowseForFolder() { m_szPath[0] = '\0'; };
  virtual ~BrowseForFolder() { };

  BOOL Browse(const TCHAR *pszTitle = _T("Browse"), UINT iFlags = BIF_RETURNONLYFSDIRS,
              HWND hWndOwner = NULL, LPCITEMIDLIST pIDListRoot = NULL)
  {
    LPITEMIDLIST pIDList;
    BOOL bRet = FALSE;
    BROWSEINFO bi;
    LPMALLOC pIMalloc;

    bi.hwndOwner = hWndOwner;
    bi.iImage = 0;
    bi.lParam = NULL;
    bi.lpfn = NULL;
    bi.lpszTitle = pszTitle;
    bi.pidlRoot = pIDListRoot;
    bi.pszDisplayName = m_szPath;
    bi.ulFlags = iFlags;

    if ((pIDList = ::SHBrowseForFolder(&bi)) != NULL)
    {
      if (::SHGetPathFromIDList(pIDList, m_szPath))
        bRet = TRUE;

      if (SUCCEEDED(SHGetMalloc(&pIMalloc)))
      {
        pIMalloc->Free(pIDList);
        WINUI_RELEASE(pIMalloc);
      }
    }

    return bRet;
  };

  const TCHAR* GetPath() const { return m_szPath; };

 protected:
  TCHAR m_szPath[_MAX_PATH];

};

//////////////////////////////////////////////////////////////////////
// Window support
//////////////////////////////////////////////////////////////////////

//! \brief Base window class for Dialog and Window
class BaseWindow : public ni::cIUnknownImpl<ni::iUnknown>
{
 public:
  BaseWindow() { m_hWnd = NULL; m_bDestroy = TRUE; };
  //! \sa Attach()
  BaseWindow(HWND hWnd, BOOL bDestroy) { m_bDestroy = TRUE; WINUI_VERIFY(Attach(hWnd,bDestroy)); };

  virtual ~BaseWindow()
  {
    Destroy();
  };

  virtual BOOL Reshape(UINT iCX, UINT iCY)
  { return SetWindowPos(0, 0, iCX, iCY, SWP_NOZORDER | SWP_NOMOVE); };

  HWND GetHWND() const { return m_hWnd; };

  /*! \brief Attach object to an existing handle
    \return true in case of success
    \param hWnd Window handle which should be attached
    \param bDestroy
    \sa Detach(), AttachDlgItem()
    \remarks Destroies existing window
  */
  bool Attach(HWND hWnd, BOOL bDestroy)
  {
    if (!IsWindow(hWnd))
      return false;
    Destroy();
    m_bDestroy = bDestroy;
    m_hWnd = hWnd;
    return true;
  };

  void Detach() { m_hWnd = NULL; };

  //! \sa Attach(), Detach()
  bool AttachDlgItem(HWND hDlg, INT iItemID)
  { return Attach(GetDlgItem(hDlg, iItemID), FALSE); };

  void CenterOnScreen()
  {
    UINT iScreenCX = GetSystemMetrics(SM_CXFULLSCREEN);
    UINT iScreenCY = GetSystemMetrics(SM_CYFULLSCREEN);
    UINT iWndCX, iWndCY;
    Rect rcWnd;
    GetWindowRect(rcWnd);
    iWndCX = rcWnd.right - rcWnd.left;
    iWndCY = rcWnd.bottom - rcWnd.top;
    SetWindowPos(NULL, iScreenCX / 2 - iWndCX / 2, iScreenCY / 2 - iWndCY / 2, iWndCX, iWndCY);
  };

  void AddStyle(DWORD dwStyle)
  {
    DWORD dwExistingStyles = GetWindowLong(GWL_STYLE);
    dwExistingStyles |= dwStyle;
    SetWindowLong(GWL_STYLE, dwExistingStyles);
  };

  virtual void Destroy()
  {
    if (!m_bDestroy)
      return;
    if (IsCreated())
    {
      // To avoid crash when class is destroied
      ::SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR) NULL);
      WINUI_VERIFY(DestroyWindow(m_hWnd));
    }
    m_hWnd = NULL;
  };

  virtual WINUI_INL BOOL IsCreated() const { return IsWindow(m_hWnd); };

  // Wrappers
  virtual BOOL ShowWindow(int iCmdShow)
  { niAssert(IsCreated()); return ::ShowWindow(m_hWnd, iCmdShow); };
  virtual BOOL SetWindowPos(HWND hWndAfter, int iX, int iY, int iCX, int iCY, UINT iFlags = SWP_NOZORDER)
  { niAssert(IsCreated()); return ::SetWindowPos(m_hWnd, hWndAfter, iX, iY, iCX, iCY, iFlags); };
  virtual BOOL GetWindowRect(Rect& rcWnd)
  { niAssert(IsCreated()); return ::GetWindowRect(m_hWnd, &rcWnd); };
  virtual BOOL SetWindowRect(Rect rcWnd)
  { niAssert(IsCreated()); return SetWindowPos(NULL, rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top); };
  virtual BOOL GetClientRect(Rect& rcClient)
  { niAssert(IsCreated()); return ::GetClientRect(m_hWnd, &rcClient); };
  virtual LRESULT SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
  { niAssert(IsCreated()); return niWin32API(SendMessage)(m_hWnd, Msg, wParam, lParam); };
  virtual BOOL PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
  { niAssert(IsCreated()); return niWin32API(PostMessage)(m_hWnd, Msg, wParam, lParam); };
  virtual void SetParent(BaseWindow *pWnd)
  { niAssert(IsCreated()); ::SetParent(m_hWnd, (pWnd != NULL) ? pWnd->GetHWND() : NULL); };
  virtual LONG GetTextLength() const  { return ::GetWindowTextLength(m_hWnd); }
  virtual void GetText(TCHAR* aszOut, ULONG nMaxChar)  const {  ::GetWindowText(m_hWnd, aszOut, nMaxChar);  }
  virtual void SetText(const TCHAR* aszStr) { ::SetWindowText(m_hWnd, aszStr);  }
  virtual ni::cString GetText() const
  {
    ni::achar* pText;
    ni::tU32 nTexLen = GetWindowTextLength();
    if (nTexLen*sizeof(TCHAR) > 64*1024) {
      nTexLen = 4096;
    }
    pText = (ni::achar*)niMalloc(sizeof(ni::achar)*(nTexLen+1));
    GetText(pText, nTexLen+1);
    pText[nTexLen] = '\0';
    ni::cString r = pText;
    niFree(pText);
    return r;
  }
  virtual HDC GetDC()
  { niAssert(IsCreated()); return ::GetDC(m_hWnd); };
  virtual void ReleaseDC(HDC hDC)
  { niAssert(IsCreated()); ::ReleaseDC(m_hWnd, hDC); };
  virtual HWND SetCapture()
  { niAssert(IsCreated()); return ::SetCapture(m_hWnd); };
  virtual BOOL ReleaseCapture() const
  { return ::ReleaseCapture(); };
  virtual BOOL UpdateWindow()
  { niAssert(IsCreated()); return ::UpdateWindow(m_hWnd); };
  virtual DWORD GetStyle() const
  { niAssert(IsCreated()); return (DWORD) ::GetWindowLong(m_hWnd, GWL_STYLE); };
  virtual DWORD GetExStyle() const
  { niAssert(IsCreated()); return (DWORD) ::GetWindowLong(m_hWnd, GWL_EXSTYLE); };
  virtual LONG GetWindowLong(int nIndex) const
  { niAssert(IsCreated()); return ::GetWindowLong(m_hWnd, nIndex); };
  virtual LONG SetWindowLong(int nIndex, LONG dwNewLong)
  { niAssert(IsCreated()); return ::SetWindowLong(m_hWnd, nIndex, dwNewLong); };
  BOOL SendNotifyMessage(UINT message, WPARAM wParam = 0, LPARAM lParam = 0)
  { niAssert(IsCreated()); return ::SendNotifyMessage(m_hWnd, message, wParam, lParam); };
  int GetWindowTextLength() const
  { niAssert(IsCreated()); return ::GetWindowTextLength(m_hWnd); };
  void SetFont(HFONT hFont, BOOL bRedraw = TRUE)
  { niAssert(IsCreated()); SendMessage(WM_SETFONT, (WPARAM) hFont, MAKELPARAM(bRedraw, 0)); };
  HFONT GetFont() const
  { niAssert(IsCreated()); return (HFONT)niWin32API(SendMessage)(m_hWnd, WM_GETFONT, 0, 0); };
  BOOL BringWindowToTop()
  { niAssert(IsCreated()); return ::BringWindowToTop(m_hWnd); };
  BOOL GetWindowPlacement(WINDOWPLACEMENT *lpwndpl) const
  { niAssert(IsCreated()); return ::GetWindowPlacement(m_hWnd, lpwndpl); };
  BOOL SetWindowPlacement(const WINDOWPLACEMENT * lpwndpl)
  { niAssert(IsCreated()); return ::SetWindowPlacement(m_hWnd, lpwndpl); };
  BOOL ClientToScreen(LPPOINT lpPoint) const
  { niAssert(IsCreated()); return ::ClientToScreen(m_hWnd, lpPoint); };
  BOOL ClientToScreen(Rect *pRect) const
  {
    niAssert(IsCreated());
    if (!::ClientToScreen(m_hWnd, (LPPOINT) pRect))
      return FALSE;
    return ::ClientToScreen(m_hWnd, ((LPPOINT) pRect) + 1);
  };
  BOOL ScreenToClient(LPPOINT lpPoint) const
  { niAssert(IsCreated()); return ::ScreenToClient(m_hWnd, lpPoint); };
  BOOL ScreenToClient(Rect *pRect) const
  {
    niAssert(IsCreated());
    if (!::ScreenToClient(m_hWnd, (LPPOINT) pRect))
      return FALSE;
    return ::ScreenToClient(m_hWnd, ((LPPOINT) pRect) + 1);
  };
  BOOL Invalidate(BOOL bErase = TRUE)
  { niAssert(IsCreated()); return ::InvalidateRect(m_hWnd, NULL, bErase); };
  BOOL InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE)
  { niAssert(IsCreated()); return ::InvalidateRect(m_hWnd, lpRect, bErase); };
  BOOL ValidateRect(LPCRECT lpRect)
  { niAssert(IsCreated()); return ::ValidateRect(m_hWnd, lpRect); };
  void InvalidateRgn(HRGN hRgn, BOOL bErase = TRUE)
  { niAssert(IsCreated()); ::InvalidateRgn(m_hWnd, hRgn, bErase); };
  BOOL IsVisible() const
  { niAssert(IsCreated()); return ::IsWindowVisible(m_hWnd);  };
  BOOL IsEnabled() const
  { niAssert(IsCreated()); return ::IsWindowEnabled(m_hWnd); };
  BOOL Enable(BOOL bEnable = TRUE)
  { niAssert(IsCreated()); return ::EnableWindow(m_hWnd, bEnable); };
  HWND SetActiveWindow()
  { niAssert(IsCreated()); return ::SetActiveWindow(m_hWnd); };
  HWND SetFocus()
  { niAssert(IsCreated()); return ::SetFocus(m_hWnd); };
  int GetScrollPos(int nBar) const
  { niAssert(IsCreated()); return ::GetScrollPos(m_hWnd, nBar); };
  BOOL GetScrollRange(int nBar, LPINT lpMinPos, LPINT lpMaxPos) const
  { niAssert(IsCreated()); return ::GetScrollRange(m_hWnd, nBar, lpMinPos, lpMaxPos); };
  BOOL ScrollWindow(int xAmount, int yAmount, Rect *pRect = NULL, Rect *pClipRect = NULL)
  { niAssert(IsCreated()); return ::ScrollWindow(m_hWnd, xAmount, yAmount, pRect, pClipRect); };
  int ScrollWindowEx(int dx, int dy, Rect *pRectScroll, Rect *pRectClip, HRGN hRgnUpdate, Rect *pRectUpdate, UINT uFlags)
  { niAssert(IsCreated()); return ::ScrollWindowEx(m_hWnd, dx, dy, pRectScroll, pRectClip, hRgnUpdate, pRectUpdate, uFlags); };
  int ScrollWindowEx(int dx, int dy, UINT uFlags, Rect *pRectScroll = NULL, Rect *pRectClip = NULL, HRGN hRgnUpdate = NULL, Rect *pRectUpdate = NULL)
  { niAssert(IsCreated()); return ::ScrollWindowEx(m_hWnd, dx, dy, pRectScroll, pRectClip, hRgnUpdate, pRectUpdate, uFlags); };
  int SetScrollPos(int nBar, int nPos, BOOL bRedraw = TRUE)
  { niAssert(IsCreated()); return ::SetScrollPos(m_hWnd, nBar, nPos, bRedraw); };
  BOOL SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL bRedraw = TRUE)
  { niAssert(IsCreated()); return ::SetScrollRange(m_hWnd, nBar, nMinPos, nMaxPos, bRedraw); };
  BOOL ShowScrollBar(UINT nBar, BOOL bShow = TRUE)
  { niAssert(IsCreated()); return ::ShowScrollBar(m_hWnd, nBar, bShow); };
  BOOL EnableScrollBar(UINT uSBFlags, UINT uArrowFlags = ESB_ENABLE_BOTH)
  { niAssert(IsCreated()); return ::EnableScrollBar(m_hWnd, uSBFlags, uArrowFlags); };
  BOOL IsChild(HWND hWnd) const
  { niAssert(IsCreated()); return ::IsChild(m_hWnd, hWnd); };
  BOOL FlashWindow(BOOL bInvert)
  { niAssert(IsCreated()); return ::FlashWindow(m_hWnd, bInvert); };
  HICON SetIcon(HICON hIcon, BOOL bBigIcon = TRUE)
  { niAssert(IsCreated()); return (HICON) SendMessage(WM_SETICON, bBigIcon, (LPARAM) hIcon); };
  HICON GetIcon(BOOL bBigIcon = TRUE) const
  { niAssert(IsCreated()); return (HICON) niWin32API(SendMessage)(m_hWnd, WM_GETICON, bBigIcon, 0); };
  int GetWindowRgn(HRGN hRgn)
  { niAssert(IsCreated()); return ::GetWindowRgn(m_hWnd, hRgn); };
  int SetWindowRgn(HRGN hRgn, BOOL bRedraw = FALSE)
  { niAssert(IsCreated()); return ::SetWindowRgn(m_hWnd, hRgn, bRedraw); };

  BOOL SetTopMost() {
    return ::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
  }

 protected:
  HWND m_hWnd;
  BOOL m_bDestroy;
};

/*! \brief Works around the menu / control ID problem in CreateWindow()
  \sa inyWindow
*/
struct MENUOrID
{
  MENUOrID(HMENU hMenu) : m_hMenu(hMenu) { };
  MENUOrID(UINT nID) : m_hMenu((HMENU) (UINT_PTR) nID) { };
  operator HMENU () { return m_hMenu; };

 protected:
  HMENU m_hMenu;

};

/*! \brief Default window class
  \sa Window
*/
#define WINUI_DEF_WND_CLS _T("WindowClass")

/*! \brief Window wrapper class
  \sa WINUI_BEGIN_MSG_MAP, GlobalWndProc(), Dialog
*/
class Window : public BaseWindow
{
 public:
  Window() { };
  Window(HWND hWnd, BOOL bDestroy) : BaseWindow(hWnd, bDestroy) { };

  virtual ~Window() { };

  /*! \brief Internal window procedure
    \remarks Do not override directly, use message map macros
    \sa WINUI_BEGIN_MSG_MAP
  */
  virtual LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {
    return DefWindowProc(hWnd, message, wParam, lParam);
  };

  /*! \brief Generic window creation function
    \param dwStyle Window style
    \param dwExStyle Extended window style
    \param lpClassName Registered window class name
    \param lpWindowName Window title / text
    \param pParent Parent window
    \param hMenu Menu ID for windows or control ID for control windows
    \return TRUE in case of success
    \remarks Do not change the GWL_USERDATA of the window, it is required by
    GlobalWndProc() to correctly route the messages
    \sa RegisterClass(), GlobalWndProc()
  */
  virtual BOOL Create(DWORD dwStyle = WS_VISIBLE, DWORD dwExStyle = NULL,
                      LPCTSTR lpClassName = WINUI_DEF_WND_CLS,
                      LPCTSTR lpWindowName = NULL, BaseWindow *pParent = NULL,
                      MENUOrID hMenu = (UINT) 0)
  {
    Destroy();

    if (lpClassName == NULL)
      lpClassName = WINUI_DEF_WND_CLS;

#ifdef _UNICODE
    m_hWnd = ::CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle,
                               0, 0, 100, 100, (pParent != NULL) ? pParent->GetHWND() : NULL,
                               hMenu, GetMsWinHInstance(), (LPVOID) this);
#else
    m_hWnd = ::CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle,
                               0, 0, 100, 100, (pParent != NULL) ? pParent->GetHWND() : NULL,
                               hMenu, GetMsWinHInstance(), (LPVOID) this);
#endif
    if (m_hWnd == NULL) {
      WINUI_CHECK_LAST_ERR
          return FALSE;
    }

    return TRUE;
  };
};

#pragma warning (push)
#pragma warning (disable : 4311 4312)
/*! \brief Global message procedure for all Window derived classes
  \sa Window, WINUI_BEGIN_MSG_MAP
*/
WINUI_INL LRESULT CALLBACK GlobalWndProc(HWND hWnd, UINT message,
                                         WPARAM wParam, LPARAM lParam)
{
  LPCREATESTRUCT cs;
  Window *pWnd = NULL;

  if (message == WM_CREATE)
  {
    cs = (LPCREATESTRUCT) lParam;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
  }

  if (GetWindowLongPtr(hWnd, GWLP_USERDATA) == NULL)
    return DefWindowProc(hWnd, message, wParam, lParam);

  pWnd = reinterpret_cast<Window *> (GetWindowLongPtr(hWnd, GWLP_USERDATA));
  if (!pWnd) return TRUE;

  niAssert(!IsBadReadPtr(pWnd, sizeof(Window)));

  LRESULT lRes = pWnd->WndProc(hWnd, message, wParam, lParam);

  if (message == WM_DESTROY)
  {
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) NULL);
  }

  return lRes;
};
#pragma warning (pop)

/*! \brief Window class registration function
  \return NULL in case of failure
  \sa Window
*/
WINUI_INL LPCTSTR RegisterClass(const TCHAR *pszName,
                                UINT iStyle = CS_HREDRAW | CS_VREDRAW,
                                LPCTSTR pszIconRes = NULL,
                                LPCTSTR pszSmIconRes = NULL,
                                HBRUSH hbrBackground = NULL)
{
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = iStyle;
  wcex.lpfnWndProc = (WNDPROC) GlobalWndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = GetMsWinHInstance();
  wcex.hIcon = LoadIcon(wcex.hInstance,  pszIconRes);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (hbrBackground == NULL) ? (HBRUSH) (COLOR_WINDOW + 1) : hbrBackground;
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName = pszName;
  wcex.hIconSm = LoadIcon(wcex.hInstance, pszSmIconRes);

  if (RegisterClassEx(&wcex) == NULL)
  {
    WINUI_CHECK_LAST_ERR
        return NULL;
  }

  return pszName;
}

WINUI_INL LRESULT CALLBACK GlobalDlgProc(HWND hWndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*! \brief Dialog wrapper class
  \sa WINUI_DLG_BEGIN_MSG_MAP, GlobalDlgProc(), Window
  \remarks Do not change the GWL_USERDATA of the dialog, it is required by
  GlobalDlgProc() to correctly route the messages
*/
class Dialog : public BaseWindow
{
  const TCHAR *mpszDialogTemplate;

 public:
  //! \param pszDialogTemplate Name of the dialog box template
  Dialog(const TCHAR *pszDialogTemplate) : mpszDialogTemplate(pszDialogTemplate) { };
  Dialog(UINT anDlg) : mpszDialogTemplate(MAKEINTRESOURCE(anDlg)) { };
  virtual ~Dialog() { };

  /*! \brief Create a modeless dialog box from a dialog box template resource
    \param bShow true if the dialog shoud be initially visible
    \param hParent Parent window of the dialog
    \return TRUE in case of success
  */
  BOOL Create(bool bShow, HWND hParent)
  {
    m_hWnd = CreateDialogParam(GetMsWinHInstance(), mpszDialogTemplate,  hParent, (DLGPROC)GlobalDlgProc, (LPARAM)this);
    if (m_hWnd == NULL)
    {
      WINUI_CHECK_LAST_ERR
          return FALSE;
    }

    if (bShow)
      ShowWindow(SW_SHOWNORMAL);

    return TRUE;
  };

  BOOL Create(bool bShow = true, Window *pParent = NULL)
  {
    return Create(bShow, (pParent != NULL) ? pParent->GetHWND() : NULL);
  };

  /*! \brief Creates a modal dialog box from a dialog box template resource
    \return TRUE in case of success
  */
  BOOL DoModal(HWND hParent)
  {
    m_hWnd = NULL;
    DialogBoxParam(GetMsWinHInstance(), mpszDialogTemplate, hParent, (DLGPROC)GlobalDlgProc, (LPARAM)this);
    /*if (DialogBoxParam(GetMsWinHInstance(), mpszDialogTemplate, hParent, (DLGPROC)GlobalDlgProc, (LPARAM)this) == -1)
      {
      WINUI_CHECK_LAST_ERR
      return FALSE;
      }*/
    return TRUE;
  };

  /*! \brief Creates a modal dialog box from a dialog box template resource
    \return TRUE in case of success
  */
  BOOL DoModal(Window *pParent = NULL)
  {
    return DoModal((pParent != NULL) ? pParent->GetHWND() : NULL);
  };

  /*! \brief Internal dialog procedure
    \remarks Do not override directly, use message map macros
    \sa WINUI_DLG_BEGIN_MSG_MAP
  */
  virtual INT_PTR DlgProc(HWND hWndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    return FALSE;
  };
};

#pragma warning (push)
#pragma warning (disable : 4311 4312)
/*! \brief Global message procedure for all Dialog derived classes
  \sa Dialog, WINUI_DLG_BEGIN_MSG_MAP
*/
WINUI_INL LRESULT CALLBACK GlobalDlgProc(HWND hWndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  Dialog *pDlg = NULL;

  if (uMsg == WM_INITDIALOG)
    SetWindowLongPtr(hWndDialog, GWLP_USERDATA, (LONG_PTR) lParam);

  if (GetWindowLong(hWndDialog, GWLP_USERDATA) == NULL)
    return FALSE;

  pDlg = reinterpret_cast<Dialog *> (GetWindowLongPtr(hWndDialog, GWLP_USERDATA));
  niAssert(!IsBadReadPtr(pDlg, sizeof(Dialog)));

  LRESULT lRes = pDlg->DlgProc(hWndDialog, uMsg, wParam, lParam);

  if (uMsg == WM_DESTROY)
    SetWindowLongPtr(hWndDialog, GWLP_USERDATA, (LONG) NULL);

  return lRes;
}
#pragma warning (pop)

/*! \brief Status bar control wrapper
  \sa FrameWindow
*/
class StatusBar : public Window
{
 public:
  StatusBar() { };
  virtual ~StatusBar() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, LONG_PTR iID = 0)
  {
    return Window::Create(dwStyle, NULL, STATUSCLASSNAME, NULL, pParent, (UINT) iID);
  };

 protected:

};

//! \brief Frame window wrapper class
class FrameWindow : public Window
{
 public:
  FrameWindow()
  {
    m_pStatusBar = NULL;
  };
  virtual ~FrameWindow() { }

  /*! \brief Sets the status bar control which is automatically positioned
    at the bottom of the client area
    \param pBar Status bar control
    \sa StatusBar
  */
  void SetStatusBar(Window *pBar) { m_pStatusBar = pBar; };

  WINUI_BEGIN_WND_MSG_MAP()
  WINUI_MESSAGE_HANDLER(WM_SIZE, OnSize)
  WINUI_END_WND_MSG_MAP(Window)

  LRESULT OnSize(UINT message, WPARAM wParam, LPARAM lParam)
  {
    UINT iCX = LOWORD(lParam);
    // UINT iCY = HIWORD(lParam);
    Rect rcStatus, rcClient;

    GetClientRect(rcClient);

    if (m_pStatusBar && m_pStatusBar->IsCreated())
    {
      m_pStatusBar->GetClientRect(rcStatus);
      m_pStatusBar->Reshape(iCX, rcStatus.bottom);
      m_pStatusBar->SetWindowPos(0, rcClient.bottom - rcStatus.bottom, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    return 0;
  };

 protected:
  Window *m_pStatusBar;

};

//! \brief ReBar control wrapper class
class ReBar : public Window
{
 public:
  ReBar() { };
  virtual ~ReBar() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = RBS_BANDBORDERS | RBS_DBLCLKTOGGLE | RBS_REGISTERDROP |
              RBS_VARHEIGHT | CCS_NODIVIDER | CCS_NOPARENTALIGN | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN |
              WS_CLIPSIBLINGS | WS_VISIBLE, MENUOrID hMenu = (UINT) 0)
  {
    return Window::Create(dwStyle, NULL, REBARCLASSNAME, NULL, pParent, hMenu);
  };

 protected:

};

//! \brief Tool bar control wrapper
class ToolBar : public Window
{
 public:
  ToolBar() { };
  virtual ~ToolBar() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CCS_ADJUSTABLE |
              TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, DWORD dwStyleEx = 0, MENUOrID hMenu = (UINT) 0)
  {
    BOOL bRet;
    bRet = Window::Create(dwStyle, dwStyleEx, TOOLBARCLASSNAME, NULL, pParent, hMenu);
    SendMessage(TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
    SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
    LoadStdImages();
    return bRet;
  };

  DWORD SetExtendedStyle(DWORD dwStyles)
  { return (DWORD) SendMessage(TB_SETEXTENDEDSTYLE, 0, (LPARAM) dwStyles); };

  INT LoadStdImages(INT iStdBmpID = IDB_STD_LARGE_COLOR)
  { return (INT) SendMessage(TB_LOADIMAGES, (WPARAM) iStdBmpID, (LPARAM) HINST_COMMCTRL); };

  BOOL AddButtons(TBBUTTON *pButtons, UINT iNumButtons = 1)
  {
    BOOL bRet;
    bRet = (BOOL) SendMessage(TB_ADDBUTTONS, (WPARAM) iNumButtons, (LPARAM) pButtons);
    if (!bRet)
    {
      WINUI_CHECK_LAST_ERR
          return FALSE;
    }
    SendMessage(TB_AUTOSIZE, 0, 0);
    return TRUE;
  };

 protected:

};

//! \brief Tab control wrapper
class TabControl : public Window
{
 public:
  TabControl() { };
  ~TabControl() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_CHILD | WS_VISIBLE,
              DWORD dwStyleEx = 0, MENUOrID hMenu = (UINT) 0)
  {
    BOOL bRet;
    bRet = Window::Create(dwStyle, dwStyleEx, WC_TABCONTROL, NULL, pParent, hMenu);
    return bRet;
  };

  UINT GetItemCount() { return TabCtrl_GetItemCount(m_hWnd); };

  BOOL AppendItem(LPCTSTR pszText, UINT iMask = TCIF_TEXT, int iImage = -1, LPARAM lParam = 0)
  {
    TCITEM sItem;

    sItem.mask = iMask;
    sItem.dwState = 0;
    sItem.dwStateMask = 0;
    sItem.pszText = (LPTSTR)pszText;
    sItem.cchTextMax = 0;
    sItem.iImage = iImage;
    sItem.lParam = lParam;

    return TabCtrl_InsertItem(m_hWnd, GetItemCount(), &sItem) != -1;
  };

  inline HIMAGELIST GetImageList() const { return (HIMAGELIST)niWin32API(SendMessage)(m_hWnd, TCM_GETIMAGELIST, 0, 0L); }
  inline HIMAGELIST SetImageList(HIMAGELIST himl) { return (HIMAGELIST)niWin32API(SendMessage)(m_hWnd, TCM_SETIMAGELIST, 0, (LPARAM)(UINT)(HIMAGELIST)(himl)); }
  inline int GetItemCount() const { return (int)niWin32API(SendMessage)(m_hWnd, TCM_GETITEMCOUNT, 0, 0L); }
  inline BOOL GetItem(int iItem, TC_ITEM* pitem) const { return (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_GETITEM, (WPARAM)(int)iItem, (LPARAM)(TC_ITEM FAR*)(pitem)); }
  inline BOOL SetItem(int iItem, const TC_ITEM* pitem) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_SETITEM, (WPARAM)(int)iItem, (LPARAM)(TC_ITEM FAR*)(pitem)); }
  inline int InsertItem(int iItem, const TC_ITEM* pitem) { return   (int)niWin32API(SendMessage)(m_hWnd, TCM_INSERTITEM, (WPARAM)(int)iItem, (LPARAM)(const TC_ITEM FAR*)(pitem)); }
  inline BOOL DeleteItem(int i) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_DELETEITEM, (WPARAM)(int)(i), 0L); }
  inline BOOL DeleteAllItems() { return (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_DELETEALLITEMS, 0, 0L); }
  inline BOOL GetItemRect(int i, RECT* prc) const { return (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_GETITEMRECT, (WPARAM)(int)(i), (LPARAM)(RECT FAR*)(prc)); }
  inline int GetCurSel() const { return (int)niWin32API(SendMessage)(m_hWnd, TCM_GETCURSEL, 0, 0); }
  inline int SetCurSel(int i) { return (int)niWin32API(SendMessage)(m_hWnd, TCM_SETCURSEL, (WPARAM)i, 0); }
  inline int HitTest(TC_HITTESTINFO* pinfo) { return (int)niWin32API(SendMessage)(m_hWnd, TCM_HITTEST, 0, (LPARAM)(TC_HITTESTINFO FAR*)(pinfo)); }
  inline BOOL SetItemExtra(void* cb) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_SETITEMEXTRA, (WPARAM)(cb), 0L); }
  inline int AdjustRect(BOOL bLarger, RECT* prc) { return (int)niWin32API(SendMessage)(m_hWnd, TCM_ADJUSTRECT, (WPARAM)(BOOL)bLarger, (LPARAM)(RECT FAR *)prc); }
  inline DWORD SetItemSize(int x, int y) { return (DWORD)niWin32API(SendMessage)(m_hWnd, TCM_SETITEMSIZE, 0, MAKELPARAM(x,y)); }
  inline void RemoveImage(int i) { (void)niWin32API(SendMessage)(m_hWnd, TCM_REMOVEIMAGE, i, 0L); }
  inline void SetPadding(int cx, int cy) { (void)niWin32API(SendMessage)(m_hWnd, TCM_SETPADDING, 0, MAKELPARAM(cx, cy)); }
  inline int GetRowCount() const { return (int)niWin32API(SendMessage)(m_hWnd, TCM_GETROWCOUNT, 0, 0L); }
  inline HWND GetToolTips() const { return (HWND)niWin32API(SendMessage)(m_hWnd, TCM_GETTOOLTIPS, 0, 0L); }
  inline void SetToolTips(HWND hwndTT) { (void)niWin32API(SendMessage)(m_hWnd, TCM_SETTOOLTIPS, (WPARAM)hwndTT, 0L); }
  inline int GetCurFocus() const { return (int)niWin32API(SendMessage)(m_hWnd, TCM_GETCURFOCUS, 0, 0); }
  inline int SetCurFocus(int i) { return (int)niWin32API(SendMessage)(m_hWnd,TCM_SETCURFOCUS, i, 0); }
  inline int SetMinTabWidth(int x) { return (int)niWin32API(SendMessage)(m_hWnd, TCM_SETMINTABWIDTH, 0, x); }
  inline void DeselectAll(UINT fExcludeFocus) { (void)niWin32API(SendMessage)(m_hWnd, TCM_DESELECTALL, fExcludeFocus, 0); }
  inline BOOL HighlightItem(int i, UINT fHighlight) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_HIGHLIGHTITEM, (WPARAM)i, (LPARAM)MAKELONG (fHighlight, 0)); }
  inline DWORD SetExtendedStyle(UINT dw) { return (DWORD)niWin32API(SendMessage)(m_hWnd, TCM_SETEXTENDEDSTYLE, 0, dw); }
  inline DWORD GetExtendedStyle() const { return (DWORD)niWin32API(SendMessage)(m_hWnd, TCM_GETEXTENDEDSTYLE, 0, 0); }
  inline BOOL SetUnicodeFormat(UINT fUnicode) { return  (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_SETUNICODEFORMAT, (WPARAM)(fUnicode), 0); }
  inline BOOL GetUnicodeFormat() const { return  (BOOL)niWin32API(SendMessage)(m_hWnd, TCM_GETUNICODEFORMAT, 0, 0); }

 protected:

};

/*! \brief Clipboard viewer class
  \sa Clipboard
*/
class ClipboardViewer : public Window
{
 public:
  ClipboardViewer() { m_hWndChainNext = NULL; };

  /*! \brief Creates an invisble window and registers it as a clipboard viewer
    \return TRUE in case of success
  */
  BOOL Create()
  {
    if (!Window::Create(0))
      return FALSE;
    m_hWndChainNext = SetClipboardViewer(m_hWnd);
    return TRUE;
  };

 protected:
  HWND m_hWndChainNext;

  WINUI_BEGIN_WND_MSG_MAP()
  WINUI_MESSAGE_HANDLER(WM_CHANGECBCHAIN, OnChangeCBChain)
  WINUI_MESSAGE_HANDLER(WM_DRAWCLIPBOARD, OnDrawClipboard)
  WINUI_END_WND_MSG_MAP(Window)

  LRESULT OnChangeCBChain(UINT message, WPARAM wParam, LPARAM lParam)
  {
    HWND hWndRemove = (HWND) wParam;
    HWND hWndNext = (HWND) lParam;

    if (m_hWndChainNext != NULL && hWndRemove != m_hWndChainNext)
      niWin32API(SendMessage)(m_hWndChainNext, WM_CHANGECBCHAIN, wParam, lParam);
    else if (hWndRemove == m_hWndChainNext)
      m_hWndChainNext = hWndNext;

    return 0;
  };

  LRESULT OnDrawClipboard(UINT message, WPARAM wParam, LPARAM lParam)
  {
    ClipboardDataChanged();

    if (m_hWndChainNext != NULL)
      niWin32API(SendMessage)(m_hWndChainNext, message, wParam, lParam);

    return 0;
  };

  /*! \brief Called when clipboard data changed, must be overridden
    \sa OnDrawClipboard()
  */
  virtual void ClipboardDataChanged() = 0;

};

//! \brief Splitter window control
class Splitter : public Window
{
 public:
  /*! \brief Creates a slitter window control
    \param pParent Parent window
    \param pFirstPan Window above / left of the splitter
    \param pSecondPan Window below / right of the splitter
    \param bVertical true if splitter should be vertical, false for horizontal
    \return TRUE in case of success
  */
  BOOL Create(BaseWindow *pParent, Window *pFirstPan = NULL,
              Window *pSecondPan = NULL, bool bVertical = false)
  {
    if (!Window::Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN))
      return FALSE;
    m_bVertical = bVertical;
    m_bDragging = false;
    SetFirstPan(pFirstPan);
    SetSecondPan(pSecondPan);
    CenterSplitter();
  };

  void SetFirstPan(Window *pWnd)
  {
    m_pFirstPan = pWnd;
    if (m_pFirstPan != NULL)
    {
      m_pFirstPan->SetParent(this);
      m_pFirstPan->AddStyle(WS_CHILD);
    }
  };

  void SetSecondPan(Window *pWnd)
  {
    m_pSecondPan = pWnd;
    if (m_pSecondPan)
    {
      m_pSecondPan->SetParent(this);
      m_pSecondPan->AddStyle(WS_CHILD);
    }
  };

  bool Reshape()
  {
    Rect rc;
    GetClientRect(rc);
    return Reshape(rc.right, rc.bottom) == TRUE;
  };

  //! \brief Centers the splitter inside the parent window
  void CenterSplitter()
  {
    HWND hWndParent = NULL;

    if (IsCreated())
    {
      hWndParent = GetParent(m_hWnd);
      if (IsWindow(hWndParent))
      {
        Rect rc;
        ::GetClientRect(hWndParent, &rc);
        m_iSplitterPos = m_bVertical ? rc.bottom / 2 : rc.right / 2;
      }
    }
  };

  BOOL Reshape(UINT iCX, UINT iCY)
  {
    const int iHlfSplitterWdh = SPLITTER_WIDTH / 2;
    int iEndUpper, iEndLeft;

    if (m_pFirstPan == NULL || m_pSecondPan == NULL)
      return false;

    SetWindowPos(NULL, 0, 0, iCX, iCY, SWP_NOZORDER | SWP_NOMOVE);

    if (m_bVertical)
    {
      iEndUpper = ReverseYAxis(m_iSplitterPos) - iHlfSplitterWdh;
      m_pFirstPan->Reshape(iCX, iEndUpper);
      m_pFirstPan->SetWindowPos(0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
      m_pSecondPan->Reshape(iCX, iCY - (iEndUpper + iHlfSplitterWdh) - 2);
      m_pSecondPan->SetWindowPos(0, iEndUpper + iHlfSplitterWdh * 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }
    else
    {
      iEndLeft = m_iSplitterPos - iHlfSplitterWdh;
      m_pFirstPan->Reshape(iEndLeft, iCY);
      m_pFirstPan->SetWindowPos(0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
      m_pSecondPan->Reshape(iCX - (iEndLeft + iHlfSplitterWdh) - 2, iCY);
      m_pSecondPan->SetWindowPos(NULL, iEndLeft + iHlfSplitterWdh * 2, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    }

    return true;
  };

  void SetSplitterPos(UINT iPos) { m_iSplitterPos = iPos; };
  UINT GetSplitterPos() const { return m_iSplitterPos; };

 protected:
  WINUI_BEGIN_WND_MSG_MAP()
  WINUI_MESSAGE_HANDLER(WM_PAINT, OnPaint)
  WINUI_MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
  WINUI_MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
  WINUI_MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
  WINUI_MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
  WINUI_END_WND_MSG_MAP(Window)

  enum { SPLITTER_WIDTH = 3 };

  void ObtainSplitterRect(Rect &rcOut)
  {
    const int iHlfSplitterWdh = SPLITTER_WIDTH / 2;

    GetClientRect(rcOut);
    if (m_bVertical)
    {
      rcOut.top = ReverseYAxis(m_iSplitterPos) - iHlfSplitterWdh - 2;
      rcOut.bottom = ReverseYAxis(m_iSplitterPos) + iHlfSplitterWdh + 2;
    }
    else
    {
      rcOut.left = m_iSplitterPos - iHlfSplitterWdh;
      rcOut.right = m_iSplitterPos + iHlfSplitterWdh;
    }
  };

  void EnableMouseTracking()
  {
    TRACKMOUSEEVENT evt;
    evt.cbSize = sizeof(TRACKMOUSEEVENT);
    evt.dwFlags = TME_HOVER | TME_LEAVE;
    evt.hwndTrack = m_hWnd;
    evt.dwHoverTime = 1;
    _TrackMouseEvent(&evt);
  };

  LRESULT OnMouseLeave(UINT message, WPARAM wParam, LPARAM lParam)
  {
    HCURSOR hCur = LoadCursor(NULL, IDC_ARROW);
    SetCursor(hCur);

    return 0;
  };

  LRESULT OnMouseMove(UINT message, WPARAM wParam, LPARAM lParam)
  {
    HCURSOR hCur;
    int xPos, yPos;

    EnableMouseTracking();

    hCur = LoadCursor(NULL, m_bVertical ? IDC_SIZENS : IDC_SIZEWE);
    niAssert(hCur != NULL);
    SetCursor(hCur);

    if (m_bDragging)
    {
      xPos = WINUI_SIGNED_LOWORD(lParam);
      yPos = WINUI_SIGNED_HIWORD(lParam);
      m_iSplitterPos = m_bVertical ? ReverseYAxis(yPos) : xPos;
      Reshape();
    }

    return 0;
  };

  LRESULT OnLButtonDown(UINT message, WPARAM wParam, LPARAM lParam)
  {
    m_bDragging = true;
    SetCapture();

    return 0;
  };

  LRESULT OnPaint(UINT message, WPARAM wParam, LPARAM lParam)
  {
    Rect rc;
    PAINTSTRUCT ps;
    DC cDC(m_hWnd);

    ::BeginPaint(m_hWnd, &ps);
    GetClientRect(rc);
    cDC.FillRect(rc, GetSysColorBrush(COLOR_BTNFACE));
    ObtainSplitterRect(rc);
    cDC.DrawFrameControl(&rc, DFC_BUTTON, DFCS_BUTTONPUSH);
    ::EndPaint(m_hWnd, &ps);

    return 0;
  };

  LRESULT OnLButtonUp(UINT message, WPARAM wParam, LPARAM lParam)
  {
    int xPos, yPos;

    m_bDragging = false;
    WINUI_VERIFY(ReleaseCapture());
    xPos = LOWORD(lParam);
    yPos = HIWORD(lParam);
    m_iSplitterPos = m_bVertical ? ReverseYAxis(yPos) : xPos;
    Reshape();

    return 0;
  };

  UINT ReverseYAxis(UINT iY)
  {
    Rect rc;
    WINUI_VERIFY(GetClientRect(rc));
    return rc.bottom - iY;
  };

  Window *m_pFirstPan;
  Window *m_pSecondPan;
  bool m_bVertical;
  bool m_bDragging;
  UINT m_iSplitterPos;

};

#if _MSC_VER >= 1300 && defined NIIF_INFO
//////////////////////////////////////////////////////////////////////////////////////////////
//! \brief Tray notify icon wrapper
class NotifyIcon : public Window
{
 public:
  NotifyIcon()
  {
    m_pContextMenu = NULL;
    memset(&m_sIcon, 0, sizeof(NOTIFYICONDATA));
  };

  //! \brief Removes the icon from the tray bar
  virtual ~NotifyIcon()
  {
    if (m_hWnd)
      WINUI_VERIFY(Shell_NotifyIcon(NIM_DELETE, &m_sIcon));
  };

  /*! \brief Creates a notify icon and puts in the tray bar
    \param iIconResourceID ID of an icon resource
    \param pContextMenu Optional pointer to a menu which is displayed when the
    user clicks the right mouse button over the icon
    \return TRUE in case of success
  */
  BOOL Create(int iIconResourceID, Menu *pContextMenu = NULL)
  {
    if (!Window::Create(0))
      return FALSE;

    m_pContextMenu = pContextMenu;

    memset(&m_sIcon, 0, sizeof(NOTIFYICONDATA));
    m_sIcon.cbSize = sizeof(NOTIFYICONDATA);
    m_sIcon.hWnd = m_hWnd;
    m_sIcon.uID = 0;
    m_sIcon.uFlags = NIF_ICON | NIF_MESSAGE;
    m_sIcon.uCallbackMessage = _WMWINUI_NOTIFY_ICON;
    m_sIcon.hIcon = LoadIcon(GetMsWinHInstance(), MAKEINTRESOURCE(iIconResourceID));

    if (!Shell_NotifyIcon(NIM_ADD, &m_sIcon))
      return FALSE;

    return TRUE;
  };

  /*! \brief Shows a ballon tooltip
    \return TRUE in case of success
  */
  BOOL ShowBalloon(const TCHAR *pszText, const TCHAR *pszTitle, UINT iTimeOut, DWORD dwInfoFlags = NIIF_INFO)
  {
    NOTIFYICONDATA sIcon = GetIconData();

    _tcsncpy(sIcon.szInfo, pszText, 256);
    _tcsncpy(sIcon.szInfoTitle, pszTitle, 64);
    sIcon.uTimeout = iTimeOut;
    sIcon.uFlags = NIF_INFO;
    sIcon.dwInfoFlags = dwInfoFlags;

    if (!Shell_NotifyIcon(NIM_MODIFY, &sIcon))
      return FALSE;

    return TRUE;
  };

  NOTIFYICONDATA GetIconData() const { return m_sIcon; };

 protected:
  NOTIFYICONDATA m_sIcon;
  Menu *m_pContextMenu;

  enum { _WMWINUI_NOTIFY_ICON = WM_USER + 1 };

  WINUI_BEGIN_WND_MSG_MAP()
  WINUI_MESSAGE_HANDLER(_WMWINUI_NOTIFY_ICON, OnNotifyIcon)
  WINUI_END_WND_MSG_MAP(Window)

  /*! \brief Handles user input for the notify icon, override to specialize behavior
    \sa OnContextMenuAction(), OnIconDblClick()
  */
  virtual LRESULT OnNotifyIcon(UINT message, WPARAM wParam, LPARAM lParam)
  {
    UINT iID;
    UINT iMouseMsg;
    POINT ptCursor;
    INT iSelectionID;

    WINUI_VERIFY(GetCursorPos(&ptCursor));

    iID = (UINT) wParam;
    iMouseMsg = (UINT) lParam;

    switch (iMouseMsg)
    {
      case WM_RBUTTONUP:
        if (m_pContextMenu)
        {
          iSelectionID = m_pContextMenu->TrackPopupMenu(ptCursor.x, ptCursor.y, TPM_RETURNCMD, GetHWND());
          return OnContextMenuAction(iSelectionID);
        }
        break;

      case WM_LBUTTONDBLCLK:
        return OnIconDblClick();
        break;

      case NIN_BALLOONUSERCLICK:
        return OnBalloonClick();
        break;

      default:
        break;
    };

    return 0;
  };

  /*! \brief Called when user selects an entry in the icon's context menu, override
    \param iSelectionID Resource ID of the selected menu entry
    \sa OnNotifyIcon(), Create()
  */
  virtual LRESULT OnContextMenuAction(INT iSelectionID)
  {
    return 0;
  };

  //! \brief Called when the user double clicks the icon, override
  virtual LRESULT OnIconDblClick()
  {
    return 0;
  };

  //! \brief Called when the user clicks on a balloon tooltip, override
  virtual LRESULT OnBalloonClick()
  {
    return 0;
  };
};
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// Controls
//////////////////////////////////////////////////////////////////////////////////////////////

//! \brief Static control wrapper
class Static : public Window
{
 public:
  Static() { };
  virtual ~Static() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_VISIBLE | WS_CHILD,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    return Window::Create(dwStyle, dwExStyle, _T("STATIC"), lpWindowName, pParent, (HMENU) iID);
  };

  HICON SetIcon(HICON hIcon) { return ((HICON)(UINT)(DWORD)niWin32API(SendMessage)(m_hWnd, STM_SETICON, (WPARAM)(HICON)(hIcon), 0L)); }
  HICON GetIcon() const { return ((HICON)(UINT)(DWORD)niWin32API(SendMessage)(m_hWnd, STM_GETICON, 0L, 0L)); }

 protected:

};

//! \brief Button control wrapper
class Button : public Window
{
 public:
  Button() { };
  virtual ~Button() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_VISIBLE | WS_CHILD,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    return Window::Create(dwStyle, dwExStyle, _T("BUTTON"),
                          lpWindowName, pParent, (HMENU) iID);
  };

  inline BOOL IsChecked() const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, BM_GETCHECK, 0L, 0L)); }
  inline ULONG GetCheck() const { return ((ULONG)niWin32API(SendMessage)(m_hWnd, BM_GETCHECK, 0L, 0L)); }
  inline void SetCheck(BOOL bChecked)  { ((void)niWin32API(SendMessage)(m_hWnd, BM_SETCHECK, (WPARAM)(int)(bChecked), 0L)); }

  inline ULONG GetState() const { return ((ULONG)(DWORD)niWin32API(SendMessage)(m_hWnd, BM_GETSTATE, 0L, 0L)); }
  inline void SetState(ULONG aState) { ((void)niWin32API(SendMessage)(m_hWnd, BM_SETSTATE, (WPARAM)(int)(aState), 0L)); }

  inline void SetStyle(ULONG aStyle, BOOL bRedraw = TRUE) { ((void)niWin32API(SendMessage)(m_hWnd, BM_SETSTYLE, (WPARAM)LOWORD(aStyle), MAKELPARAM(bRedraw,0))); }

 protected:
};

//! \brief Radio button wrapper
class Radio
{
 public:
  Radio() { mState = 0; };
  virtual ~Radio() { };

  inline void AddState(HWND hWndParent, INT nControlID)
  {
    mvButtons.push_back(WinUI::Button());
    mvButtons.back().AttachDlgItem(hWndParent, nControlID);
  }

  inline ULONG GetNumState() const
  {
    return (ULONG)mvButtons.size();
  }

  inline void SetState(ULONG aState)
  {
    if (aState >= GetNumState()) return;
    if (mState < GetNumState())
    {
      mvButtons[mState].SetCheck(0);
    }
    mvButtons[aState].SetCheck(1);
    mState = aState;
  }

  inline ULONG GetState() const
  {
    for (mState = 0; mState < mvButtons.size(); ++mState)
      if (mvButtons[mState].GetCheck())
        return mState;
    mState = 0;
    return mState;
  }

 private:
  typedef astl::vector<WinUI::Button> tButtonVec;
  typedef tButtonVec::iterator    tButtonVecIt;
  typedef tButtonVec::const_iterator  tButtonVecCIt;
  tButtonVec  mvButtons;
  mutable ULONG mState;
};

//! \brief Combo box control wrapper
class ComboBox : public Window
{
 public:
  ComboBox() { };
  virtual ~ComboBox() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_VISIBLE | WS_CHILD,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    return Window::Create(dwStyle, dwExStyle, _T("COMBOBOX"),
                          lpWindowName, pParent, (HMENU) iID);
  };

  inline int LimitText(int cchLimit)  {  return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_LIMITTEXT, (WPARAM)(int)(cchLimit), 0L)); }
  inline int GetEditSel()       const { return ((DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETEDITSEL, 0L, 0L)); }
  inline int SetEditSel(int ichStart, int ichEnd) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_SETEDITSEL, 0L, MAKELPARAM((ichStart), (ichEnd)))); }
  inline int GetCount() const   { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETCOUNT, 0L, 0L)); }
  inline int ResetContent()     { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_RESETCONTENT, 0L, 0L)); }
  inline int AddString(LPCTSTR lpsz) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_ADDSTRING, 0L, (LPARAM)(LPCTSTR)(lpsz))); }
  inline int InsertString(int index, LPCTSTR lpsz)  { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(lpsz))); }
  inline int AddItemData(void* data) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_ADDSTRING, 0L, (LPARAM)(data))); }
  inline int InsertItemData(int index, void* data)  { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(data))); }
  inline int DeleteString(int index) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_DELETESTRING, (WPARAM)(int)(index), 0L)); }
  inline int GetLBTextLen(int index) const  { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETLBTEXTLEN, (WPARAM)(int)(index), 0L)); }
  inline int GetLBText(int index, LPTSTR lpszBuffer) const  { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETLBTEXT, (WPARAM)(int)(index), (LPARAM)(LPTSTR)(lpszBuffer))); }
  inline ni::cString GetLBText(int index) const {
    int len = GetLBTextLen(index);
    LPTSTR pszStr = (LPTSTR)niMalloc(len*sizeof(ni::achar));
    GetLBText(index, pszStr);
    ni::cString r = pszStr;
    niFree(pszStr);
    return r;
  }
  inline void* GetItemData(int index) const { return ((void*)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETITEMDATA, (WPARAM)(int)(index), 0L)); }
  inline int SetItemData(int index, void* data) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_SETITEMDATA, (WPARAM)(int)(index), (LPARAM)(data))); }
  inline int FindString(int indexStart, LPCTSTR lpszFind) const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind))); }
  inline int FindItemData(int indexStart, void* data) const   { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data))); }
  inline int GetCurSel() const    { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETCURSEL, 0L, 0L)); }
  inline int SetCurSel(int index) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_SETCURSEL, (WPARAM)(int)(index), 0L)); }
  inline int SelectString(int indexStart, LPCTSTR lpszSelect) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszSelect))); }
  inline int SelectItemData(int indexStart, void* data)     { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_SELECTSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data))); }
  inline int Dir(UINT attrs, LPCTSTR lpszFileSpec)  { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_DIR, (WPARAM)(UINT)(attrs), (LPARAM)(LPCTSTR)(lpszFileSpec))); }
  inline int ShowDropdown(UINT fShow)       { return ((BOOL)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_SHOWDROPDOWN, (WPARAM)(BOOL)(fShow), 0L)); }
  inline int FindStringExact(int indexStart, LPCTSTR lpszFind) const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_FINDSTRINGEXACT, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind))); }
  inline int GetDroppedState() const  { return ((BOOL)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETDROPPEDSTATE, 0L, 0L)); }
  inline void GetDroppedControlRect(RECT* lprc)   { ((void)niWin32API(SendMessage)(m_hWnd, CB_GETDROPPEDCONTROLRECT, 0L, (LPARAM)(RECT *)(lprc))); }
  inline int GetItemHeight() const  { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETITEMHEIGHT, 0L, 0L)); }
  inline int SetItemHeight(int index, int cyItem) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_SETITEMHEIGHT, (WPARAM)(int)(index), (LPARAM)(int)cyItem)); }
  inline int GetExtendedUI() const  { return ((UINT)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_GETEXTENDEDUI, 0L, 0L)); }
  inline int SetExtendedUI(UINT flags) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, CB_SETEXTENDEDUI, (WPARAM)(UINT)(flags), 0L)); }

 protected:

};

//! \brief List view control wrapper
class Header : public Window
{
 public:
  Header() { };
  Header(HWND hWnd, BOOL bDestroy) : Window(hWnd, bDestroy) { };
  virtual ~Header() { };

  BOOL Create(DWORD dwStyle = WS_VISIBLE | WS_CHILD | LVS_REPORT, Window *pParent = NULL,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    return Window::Create(dwStyle, dwExStyle, WC_HEADER,
                          lpWindowName, pParent, (HMENU) iID);
  };

  int GetItemCount() const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, HDM_GETITEMCOUNT, 0, 0L); }
  int InsertItem(int nPos, HDITEM* phdi)
  { niAssert(::IsWindow(m_hWnd)); return (int)niWin32API(SendMessage)(m_hWnd, HDM_INSERTITEM, nPos, (LPARAM)phdi); }
  BOOL DeleteItem(int nPos)
  { niAssert(::IsWindow(m_hWnd)); return (BOOL)niWin32API(SendMessage)(m_hWnd, HDM_DELETEITEM, nPos, 0L); }
  BOOL GetItem(int nPos, HDITEM* pHeaderItem) const
  { niAssert(::IsWindow(m_hWnd)); return (BOOL)niWin32API(SendMessage)(m_hWnd, HDM_GETITEM, nPos, (LPARAM)pHeaderItem); }
  BOOL SetItem(int nPos, HDITEM* pHeaderItem)
  { niAssert(::IsWindow(m_hWnd)); return (BOOL)niWin32API(SendMessage)(m_hWnd, HDM_SETITEM, nPos, (LPARAM)pHeaderItem); }
  BOOL Layout(HDLAYOUT* pHeaderLayout)
  { niAssert(::IsWindow(m_hWnd)); return (BOOL)niWin32API(SendMessage)(m_hWnd, HDM_LAYOUT, 0, (LPARAM)pHeaderLayout); }
  BOOL SetOrderArray(int iCount, LPINT piArray)
  { niAssert(::IsWindow(m_hWnd)); return (BOOL) niWin32API(SendMessage)(m_hWnd, HDM_SETORDERARRAY, (WPARAM)iCount, (LPARAM)piArray); }
  BOOL GetOrderArray(LPINT piArray, int iCount)
  { niAssert(::IsWindow(m_hWnd));
    int nCount = iCount;
    if (nCount < 0)
    {
      niAssert(FALSE);
      return FALSE;
    }
    return (BOOL)niWin32API(SendMessage)(m_hWnd, HDM_GETORDERARRAY, (WPARAM) nCount, (LPARAM) piArray);
  }
};

//! \brief List view control wrapper
class ListBox : public Window
{
 public:
  ListBox() { };
  virtual ~ListBox() { };

  BOOL Create(DWORD dwStyle = WS_VISIBLE | WS_CHILD | LVS_REPORT, Window *pParent = NULL,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    return Window::Create(dwStyle, dwExStyle, WC_LISTVIEW,
                          lpWindowName, pParent, (HMENU) iID);
  };

  void SetViewStyle(DWORD dwView)
  {
    DWORD dwStyle = GetWindowLong(GWL_STYLE);
    if ((dwStyle & LVS_TYPEMASK) != dwView)
      SetWindowLong(GWL_STYLE, (dwStyle & ~LVS_TYPEMASK) | dwView);
  };

  bool InsertColumn(UINT iColIdx, const TCHAR* pszColumnHeading, int nFormat, INT iWidth = -1, INT aSubItem = -1)
  {
    LVCOLUMN col;
    col.mask =  LVCF_TEXT;
    col.pszText = (TCHAR*)pszColumnHeading;
    if (iWidth >= 0)
    {
      col.mask |= LVCF_WIDTH;
      col.cx = iWidth;
    }
    if (LVCF_SUBITEM)
    {
      col.mask |= LVCF_SUBITEM;
      col.iSubItem = aSubItem;
    }
    return ListView_InsertColumn(m_hWnd,iColIdx, &col) != -1;
  };

  //! Get the number of item in the list box.
  //! \return is the number of items in the list box, or LB_ERR if an error occurs.
  inline int GetCount() const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETCOUNT, 0L, 0L)); }
  //! Reset the content of the list box.
  inline void ResetContent()  { niWin32API(SendMessage)(m_hWnd, LB_RESETCONTENT, 0L, 0L); }

  inline int InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask,
                        int nImage, LPARAM lParam)
  {
    niAssert(::IsWindow(m_hWnd));
    LVITEM item;
    item.mask = nMask;
    item.iItem = nItem;
    item.iSubItem = 0;
    item.pszText = (LPTSTR)lpszItem;
    item.state = nState;
    item.stateMask = nStateMask;
    item.iImage = nImage;
    item.lParam = lParam;
    return InsertItem(&item);
  }
  int InsertItem(const LVITEM* pItem)
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)pItem); }
  int InsertItem(int nItem, LPCTSTR lpszItem)
  { niAssert(::IsWindow(m_hWnd)); return InsertItem(LVIF_TEXT, nItem, lpszItem, 0, 0, 0, 0); }
  int InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
  { niAssert(::IsWindow(m_hWnd)); return InsertItem(LVIF_TEXT|LVIF_IMAGE, nItem, lpszItem, 0, 0, nImage, 0); }
  int InsertItem(int nItem, void* pData)
  { niAssert(::IsWindow(m_hWnd)); return InsertItem(LVIF_PARAM, nItem, NULL, 0, 0, 0, (LPARAM)pData); }

  //! Delete the item at the specified index.
  //! \return The return value is a count of the strings remaining in the list.
  //!     The return value is LB_ERR if the index is greater than the number of items in the list.
  inline int DeleteItem(int index) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_DELETESTRING, (WPARAM)(int)(index), 0L)); }

  inline int GetTextLen(int index) const  { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETTEXTLEN, (WPARAM)(int)(index), 0L)); }
  inline int GetText(int index, const TCHAR* lpszBuffer) const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETTEXT, (WPARAM)(int)(index), (LPARAM)(LPCTSTR)(lpszBuffer))); }
  inline int FindString(int indexStart, const TCHAR* lpszFind) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind))); }
  inline int FindItemData(int indexStart, void* data) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_FINDSTRING, (WPARAM)(int)(indexStart), (LPARAM)(data))); }
  inline int SetSel(int fSelect, int index)     { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_SETSEL, (WPARAM)(BOOL)(fSelect), (LPARAM)(index))); }
  inline int SelItemRange(int fSelect, int first, int last)    { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_SELITEMRANGE, (WPARAM)(BOOL)(fSelect), MAKELPARAM((first), (last)))); }
  inline int GetCurSel()  { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LVM_GETSELECTIONMARK, 0L, 0L)); }
  inline int SetCurSel(int index)           { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_SETCURSEL, (WPARAM)(int)(index), 0L)); }
  inline int GetSel(int index) const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETSEL, (WPARAM)(int)(index), 0L)); }
  inline int GetSelCount() const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETSELCOUNT, 0L, 0L)); }
  inline int GetTopIndex() const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETTOPINDEX, 0L, 0L)); }
  inline int GetSelItems(int cItems, int* lpItems) const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETSELITEMS, (WPARAM)(int)(cItems), (LPARAM)(int *)(lpItems))); }
  inline int SetTopIndex(int indexTop)      { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_SETTOPINDEX, (WPARAM)(int)(indexTop), 0L)); }
  inline void SetColumnWidth(int cxColumn)   { ((void)niWin32API(SendMessage)(m_hWnd, LB_SETCOLUMNWIDTH, (WPARAM)(int)(cxColumn), 0L)); }
  inline int GetHorizontalExtent() const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETHORIZONTALEXTENT, 0L, 0L)); }
  inline void SetHorizontalExtent(int cxExtent)     { ((void)niWin32API(SendMessage)(m_hWnd, LB_SETHORIZONTALEXTENT, (WPARAM)(int)(cxExtent), 0L)); }
  inline int SetTabStops(int cTabs, int* lpTabs) { return ((BOOL)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_SETTABSTOPS, (WPARAM)(int)(cTabs), (LPARAM)(int *)(lpTabs))); }
  inline int GetItemRect(int index, RECT* lprc)   { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETITEMRECT, (WPARAM)(int)(index), (LPARAM)(RECT *)(lprc))); }
  inline int SetCaretIndex(int index)       { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_SETCARETINDEX, (WPARAM)(int)(index), 0L)); }
  inline int GetCaretIndex() const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETCARETINDEX, 0L, 0L)); }
  inline int FindStringExact(int indexStart, const TCHAR* lpszFind) { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_FINDSTRINGEXACT, (WPARAM)(int)(indexStart), (LPARAM)(LPCTSTR)(lpszFind))); }
  inline int SetItemHeight(int index, int cy)   { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_SETITEMHEIGHT, (WPARAM)(int)(index), MAKELPARAM((cy), 0))); }
  inline int GetItemHeight(int index)  const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_GETITEMHEIGHT, (WPARAM)(int)(index), 0L)); }
  inline int Dir(UINT attrs, const TCHAR* lpszFileSpec)   { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, LB_DIR, (WPARAM)(UINT)(attrs), (LPARAM)(LPCTSTR)(lpszFileSpec))); }
  inline BOOL DeleteColumn(int nCol)
  { niAssert(::IsWindow(m_hWnd)); return (BOOL) niWin32API(SendMessage)(m_hWnd, LVM_DELETECOLUMN, nCol, 0); }
  inline int GetColumnWidth(int nCol) const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, LVM_GETCOLUMNWIDTH, nCol, 0); }

  inline int GetItemCount() const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, LVM_GETITEMCOUNT, 0, 0L); }
  inline BOOL DeleteAllItems()
  { niAssert(::IsWindow(m_hWnd)); return (BOOL) niWin32API(SendMessage)(m_hWnd, LVM_DELETEALLITEMS, 0, 0L); }
  inline void SetItemCount(int nItems)
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, LVM_SETITEMCOUNT, nItems, 0); }
  inline BOOL SortItems(PFNLVCOMPARE pfnCompare, DWORD_PTR dwData)
  { niAssert(::IsWindow(m_hWnd)); niAssert((GetStyle() & LVS_OWNERDATA)==0); return (BOOL) niWin32API(SendMessage)(m_hWnd, LVM_SORTITEMS, dwData, (LPARAM)pfnCompare); }
  inline BOOL SetItemState(int nItem, LVITEM* pItem)
  { niAssert(::IsWindow(m_hWnd)); return (BOOL) niWin32API(SendMessage)(m_hWnd, LVM_SETITEMSTATE, nItem, (LPARAM)pItem); }
  inline UINT GetItemState(int nItem, UINT nMask) const
  { niAssert(::IsWindow(m_hWnd)); return (UINT) niWin32API(SendMessage)(m_hWnd, LVM_GETITEMSTATE, nItem, nMask); }

  inline BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
                      int nImage, UINT nState, UINT nStateMask, LPARAM lParam)
  {
    niAssert((nMask & LVIF_INDENT) == 0);
    return SetItem(nItem, nSubItem, nMask, lpszItem, nImage, nState, nStateMask, lParam, 0);
  }

  inline BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
                      int nImage, UINT nState, UINT nStateMask, LPARAM lParam, int nIndent)
  {
    niAssert(::IsWindow(m_hWnd));
    niAssert((GetStyle() & LVS_OWNERDATA)==0);
    LVITEM lvi;
    lvi.mask = nMask;
    lvi.iItem = nItem;
    lvi.iSubItem = nSubItem;
    lvi.stateMask = nStateMask;
    lvi.state = nState;
    lvi.pszText = (LPTSTR) lpszItem;
    lvi.iImage = nImage;
    lvi.lParam = lParam;
    lvi.iIndent = nIndent;
    return (BOOL)niWin32API(SendMessage)(m_hWnd, LVM_SETITEM, 0, (LPARAM)&lvi);
  }

  inline BOOL SetItemState(int nItem, UINT nState, UINT nStateMask)
  {
    niAssert(::IsWindow(m_hWnd));
    LVITEM lvi;
    lvi.stateMask = nStateMask;
    lvi.state = nState;
    return (BOOL)niWin32API(SendMessage)(m_hWnd, LVM_SETITEMSTATE, nItem, (LPARAM)&lvi);
  }

  inline BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
  {
    niAssert(::IsWindow(m_hWnd));
    niAssert((GetStyle() & LVS_OWNERDATA)==0);
    LVITEM lvi;
    lvi.iSubItem = nSubItem;
    lvi.iItem = nItem;
    lvi.pszText = (LPTSTR) lpszText;
    return (BOOL)niWin32API(SendMessage)(m_hWnd, LVM_SETITEMTEXT, nItem, (LPARAM)&lvi);
  }

  inline ni::cString GetItemText(int nItem, int nSubItem) const
  {
    niAssert(::IsWindow(m_hWnd));
    LVITEM lvi;
    memset(&lvi, 0, sizeof(LVITEM));
    lvi.iSubItem = nSubItem;
    lvi.iItem = nItem;
    ni::cString str;
    int nLen = 128;
    int nRes;
    do
    {
      nLen *= 2;
      lvi.cchTextMax = nLen;
      str.resize(nLen);
      lvi.pszText = str.data();
      nRes = (int)niWin32API(SendMessage)(m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)&lvi);
    } while (nRes == nLen-1);
    return str;
  }

  inline int GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const
  {
    niAssert(::IsWindow(m_hWnd));
    LVITEM lvi;
    memset(&lvi, 0, sizeof(LVITEM));
    lvi.iItem = nItem;
    lvi.iSubItem = nSubItem;
    lvi.cchTextMax = nLen;
    lvi.pszText = lpszText;
    return (int)niWin32API(SendMessage)(m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem, (LPARAM)&lvi);
  }

  inline BOOL SetItemData(int nItem, void* pData)
  { niAssert(::IsWindow(m_hWnd)); return SetItem(nItem, 0, LVIF_PARAM, NULL, 0, 0, 0, (LPARAM)pData); }

  inline void* GetItemData(int nItem) const
  {
    niAssert(::IsWindow(m_hWnd));
    LVITEM lvi;
    memset(&lvi, 0, sizeof(LVITEM));
    lvi.mask = LVIF_PARAM;
    lvi.iItem = nItem;
    niWin32API(SendMessage)(m_hWnd, LVM_GETITEM, (WPARAM)0, (LPARAM)&lvi);
    return (void*)lvi.lParam;
  }

  HWND GetHeaderCtrl()
  {
    niAssert(::IsWindow(m_hWnd));
    return (HWND)niWin32API(SendMessage)(m_hWnd, LVM_GETHEADER, 0, 0);
  }
};

//! \brief Tree view control wrapper
class TreeView : public Window
{
 public:
  TreeView() { };
  virtual ~TreeView() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_VISIBLE | WS_CHILD |
              TVS_HASLINES | TVS_HASBUTTONS, LONG_PTR iID = 0, DWORD dwExStyle = NULL,
              LPCTSTR lpWindowName = NULL)
  {
    return Window::Create(dwStyle, dwExStyle, WC_TREEVIEW,
                          lpWindowName, pParent, (HMENU) iID);
  };

  /*! \brief Adds an item into the tree control
    \param pszItemDesc String which is displayed for the item
    \param iUserData User data stored with the item
    \param iImage Zero based index into the image list of the control
    \param hParent parent of the item to be inserted. The root of the three
    is taken when this is set to NULL
    \return Handle of the item added
    \sa SetImageList()
    \remarks Uses static variable to add items sequential, not thread safe
  */
  HTREEITEM InsertItem(const TCHAR* pszItemDesc, LPARAM iUserData = NULL,
                       HTREEITEM hParent = NULL, UINT iImage = 0)
  {
    TVITEM sItem;
    TVINSERTSTRUCT sInsert;
    static HTREEITEM hPrev = (HTREEITEM) TVI_FIRST;

    memset(&sItem, 0, sizeof(TVITEM));
    sItem.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    sItem.pszText = (TCHAR*)pszItemDesc;
    sItem.lParam = iUserData;
    sItem.iImage = iImage;
    sItem.iSelectedImage = iImage;
    sInsert.item = sItem;
    sInsert.hInsertAfter = hPrev;

    if (hParent == NULL)
      sInsert.hParent = TVI_ROOT;
    else
      sInsert.hParent = hParent;

    hPrev = (HTREEITEM) SendMessage(TVM_INSERTITEM, 0, (LPARAM) &sInsert);

    return hPrev;
  };

  BOOL SetItemText(HTREEITEM hItem, const TCHAR* aszText, ni::tI32 anNumChars = -1)
  {
    TVITEM item;
    memset(&item, 0, sizeof(TVITEM));
    item.mask = TVIF_TEXT;
    item.hItem = hItem;
    item.pszText = (LPTSTR)aszText;
    item.cchTextMax = anNumChars>0?anNumChars:ni::StrLen(aszText);
    return SetItem(&item);
  }

  BOOL GetItemText(HTREEITEM hItem, ni::achar* aaszBuffer, ni::tI32 anMaxChars) const
  {
    TVITEM item;
    memset(&item, 0, sizeof(TVITEM));
    item.mask = TVIF_TEXT;
    item.hItem = hItem;
    item.pszText = aaszBuffer;
    item.cchTextMax = anMaxChars;
    return GetItem(&item);
  }

  ni::cString GetItemText(HTREEITEM hItem) const
  {
    ni::achar aszText[4096];
    if (!GetItemText(hItem, aszText, 4096)) return ni::cString();
    return aszText;
  }

  BOOL SetItemData(HTREEITEM hItem, void* pData)
  {
    TVITEM item;
    memset(&item, 0, sizeof(TVITEM));
    item.mask = TVIF_PARAM;
    item.lParam = (LPARAM)pData;
    return SetItem(&item);
  }

  void* GetItemData(HTREEITEM hItem) const
  {
    TVITEM item;
    memset(&item, 0, sizeof(TVITEM));
    item.mask = TVIF_PARAM;
    if (!GetItem(&item)) return NULL;
    return (void*)item.lParam;
  }

  void Expand(HTREEITEM hItem)
  { TreeView_Expand(m_hWnd, hItem, TVE_EXPAND); };

  HTREEITEM GetSelectedItem() { return TreeView_GetSelection(m_hWnd); };

  void SetImageList(HIMAGELIST hLst)
  { TreeView_SetImageList(m_hWnd, hLst, TVSIL_NORMAL); };

  LPARAM GetItemUserData(HTREEITEM hItem)
  {
    TVITEM tvItem;
    BOOL bRet;

    tvItem.hItem = hItem;
    tvItem.mask = TVIF_HANDLE;

    bRet = TreeView_GetItem(m_hWnd, &tvItem);
    niAssert(bRet);

    return tvItem.lParam;
  };

  inline BOOL DeleteItem(HTREEITEM hitem)   { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)(HTREEITEM)(hitem)); }
  inline BOOL DeleteAllItems()          { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT); }
  inline BOOL Expand(HTREEITEM hitem, UINT code)  { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_EXPAND, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem)); }
  inline BOOL GetItemRect(HTREEITEM hitem, RECT* prc, UINT code) const { return (*(HTREEITEM FAR *)prc = (hitem), (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_GETITEMRECT, (WPARAM)(code), (LPARAM)(RECT FAR*)(prc))); }
  inline UINT GetCount() const { return (UINT)niWin32API(SendMessage)(m_hWnd, TVM_GETCOUNT, 0, 0); }
  inline UINT GetIndent() const { return (UINT)niWin32API(SendMessage)(m_hWnd, TVM_GETINDENT, 0, 0); }
  inline BOOL SetIndent(int indent)     { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_SETINDENT, (WPARAM)indent, 0); }
  inline HIMAGELIST GetImageList(int iImage) { return (HIMAGELIST)niWin32API(SendMessage)(m_hWnd, TVM_GETIMAGELIST, iImage, 0); }
  inline HIMAGELIST SetImageList(HIMAGELIST himl, int iImage) { return (HIMAGELIST)niWin32API(SendMessage)(m_hWnd, TVM_SETIMAGELIST, iImage, (LPARAM)(UINT)(HIMAGELIST)(himl)); }
  inline HTREEITEM GetNextItem(HIMAGELIST hitem, UINT code) const { return (HTREEITEM)niWin32API(SendMessage)(m_hWnd, TVM_GETNEXTITEM, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem)); }
  inline HTREEITEM GetChild(HIMAGELIST hitem)     const { return GetNextItem(hitem, TVGN_CHILD); }
  inline HTREEITEM GetNextSibling(HIMAGELIST hitem) const { return GetNextItem(hitem, TVGN_NEXT); }
  inline HTREEITEM GetPrevSibling(HIMAGELIST hitem) const { return GetNextItem(hitem, TVGN_PREVIOUS); }
  inline HTREEITEM GetParent(HIMAGELIST hitem)    const { return GetNextItem(hitem, TVGN_PARENT); }
  inline HTREEITEM GetFirstVisible()          const { return GetNextItem(NULL,  TVGN_FIRSTVISIBLE); }
  inline HTREEITEM GetNextVisible(HIMAGELIST hitem) const { return GetNextItem(hitem, TVGN_NEXTVISIBLE); }
  inline HTREEITEM GetPrevVisible(HIMAGELIST hitem) const { return GetNextItem(hitem, TVGN_PREVIOUSVISIBLE); }
  inline HTREEITEM GetSelection()           const { return GetNextItem(NULL,  TVGN_CARET); }
  inline HTREEITEM GetDropHilight()         const { return GetNextItem(NULL,  TVGN_DROPHILITE); }
  inline HTREEITEM GetRoot()              const { return GetNextItem(NULL,  TVGN_ROOT); }
  inline HTREEITEM GetLastVisible()         const { return GetNextItem(NULL,  TVGN_LASTVISIBLE); }
  inline BOOL Select(HIMAGELIST hitem, UINT code)   { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_SELECTITEM, (WPARAM)code, (LPARAM)(HTREEITEM)(hitem)); }
  inline BOOL SelectItem(HIMAGELIST hitem)      { return Select(hitem, TVGN_CARET); }
  inline BOOL SelectDropTarget(HIMAGELIST hitem)    { return Select(hitem, TVGN_DROPHILITE); }
  inline BOOL SelectSetFirstVisible(HIMAGELIST hitem) { return Select(hitem, TVGN_FIRSTVISIBLE); }
  inline BOOL GetItem(TV_ITEM* pitem) const { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_GETITEM, 0, (LPARAM)(TV_ITEM FAR*)(pitem)); }
  inline BOOL SetItem(const TV_ITEM* pitem) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_SETITEM, 0, (LPARAM)(const TV_ITEM FAR*)(pitem)); }
  inline HWND EditLabel(HTREEITEM hitem)    { return (HWND)niWin32API(SendMessage)(m_hWnd, TVM_EDITLABEL, 0, (LPARAM)(HTREEITEM)(hitem)); }
  inline HWND GetEditControl()  const { return (HWND)niWin32API(SendMessage)(m_hWnd, TVM_GETEDITCONTROL, 0, 0); }
  inline UINT GetVisibleCount() const { return (UINT)niWin32API(SendMessage)(m_hWnd, TVM_GETVISIBLECOUNT, 0, 0); }
  inline HTREEITEM HitTest(TV_HITTESTINFO* pHitInfos) { return (HTREEITEM)niWin32API(SendMessage)(m_hWnd, TVM_HITTEST, 0, (LPARAM)(LPTV_HITTESTINFO)(pHitInfos)); }
  inline HIMAGELIST CreateDragImage(HTREEITEM hitem) { return (HIMAGELIST)niWin32API(SendMessage)(m_hWnd, TVM_CREATEDRAGIMAGE, 0, (LPARAM)(HTREEITEM)(hitem)); }
  inline BOOL SortChildren(HTREEITEM hitem, BOOL recurse) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_SORTCHILDREN, (WPARAM)recurse, (LPARAM)(HTREEITEM)(hitem)); }
  inline BOOL EnsureVisible(HTREEITEM hitem) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_ENSUREVISIBLE, 0, (LPARAM)(HTREEITEM)(hitem)); }
  inline BOOL SortChildrenCB(TV_SORTCB* psort, BOOL recurse) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_SORTCHILDRENCB, (WPARAM)recurse,  (LPARAM)(LPTV_SORTCB)(psort)); }
  inline BOOL EndEditLabelNow(UINT fCancel) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_ENDEDITLABELNOW, (WPARAM)fCancel, 0); }
  inline HWND SetToolTips(HWND hwndTT) { return (HWND)niWin32API(SendMessage)(m_hWnd, TVM_SETTOOLTIPS, (WPARAM)hwndTT, 0); }
  inline HWND GetToolTips() const { return (HWND)niWin32API(SendMessage)(m_hWnd, TVM_GETTOOLTIPS, 0, 0); }
  inline BOOL GetISearchString(HWND hwndTV, const TCHAR* lpsz) const { return (BOOL)niWin32API(SendMessage)((hwndTV), TVM_GETISEARCHSTRING, 0, (LPARAM)(LPTSTR)lpsz); }
  inline BOOL SetInsertMark(HTREEITEM hItem, UINT fAfter) { return (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_SETINSERTMARK, (WPARAM) (fAfter), (LPARAM) (hItem)); }
  inline BOOL SetUnicodeFormat(UINT fUnicode) { return  (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_SETUNICODEFORMAT, (WPARAM)(fUnicode), 0); }
  inline BOOL GetUnicodeFormat() const { return  (BOOL)niWin32API(SendMessage)(m_hWnd, TVM_GETUNICODEFORMAT, 0, 0); }
  inline int SetItemHeight(int iHeight) { return (int)niWin32API(SendMessage)(m_hWnd, TVM_SETITEMHEIGHT, (WPARAM)iHeight, 0); }
  inline int GetItemHeight() const { return (int)niWin32API(SendMessage)(m_hWnd, TVM_GETITEMHEIGHT, 0, 0); }
  inline COLORREF SetBkColor(COLORREF clr) { return (COLORREF)niWin32API(SendMessage)(m_hWnd, TVM_SETBKCOLOR, 0, (LPARAM)clr); }
  inline COLORREF SetTextColor(COLORREF clr) { return (COLORREF)niWin32API(SendMessage)(m_hWnd, TVM_SETTEXTCOLOR, 0, (LPARAM)clr); }
  inline COLORREF GetBkColor() const { return (COLORREF)niWin32API(SendMessage)(m_hWnd, TVM_GETBKCOLOR, 0, 0); }
  inline COLORREF GetTextColor() const { return (COLORREF)niWin32API(SendMessage)(m_hWnd, TVM_GETTEXTCOLOR, 0, 0); }
  inline UINT SetScrollTime(UINT uTime) { return (UINT)niWin32API(SendMessage)(m_hWnd, TVM_SETSCROLLTIME, uTime, 0); }
  inline UINT GetScrollTime() { return (UINT)niWin32API(SendMessage)(m_hWnd, TVM_GETSCROLLTIME, 0, 0); }
  inline COLORREF SetInsertMarkColor(COLORREF clr) { return (COLORREF)niWin32API(SendMessage)(m_hWnd, TVM_SETINSERTMARKCOLOR, 0, (LPARAM)clr); }
  inline COLORREF GetInsertMarkColor() const { return (COLORREF)niWin32API(SendMessage)(m_hWnd, TVM_GETINSERTMARKCOLOR, 0, 0); }

 protected:
};

//! \brief Progress bar control wrapper
class ProgressBar : public Window
{
 public:
  ProgressBar() { };
  virtual ~ProgressBar() { };

  BOOL Create(DWORD dwStyle = WS_VISIBLE | WS_CHILD, Window *pParent = NULL,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    BOOL bSuccess = Window::Create(dwStyle, dwExStyle, PROGRESS_CLASS,
                                   lpWindowName, pParent, (HMENU) iID);
    if (!bSuccess)
      return FALSE;
    SetRange(0, 100);
    return TRUE;
  };

  inline void SetPos(int pos) { ((void)niWin32API(SendMessage)(m_hWnd, PBM_SETPOS, (WPARAM)(int)(pos), 0L)); };
  inline int GetPos() const { ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, PBM_GETPOS, 0L, 0L)); }

  inline void SetRange(int min, int max) { ((void)niWin32API(SendMessage)(m_hWnd, PBM_SETRANGE, 0L, MAKELPARAM(min,max))); };
  inline int GetRangeMin() const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, PBM_GETRANGE, TRUE, 0L)); }
  inline int GetRangeMax() const { return ((int)(DWORD)niWin32API(SendMessage)(m_hWnd, PBM_GETRANGE, FALSE, 0L)); }

 protected:

};

//! \brief Slider bar control wrapper
class Slider : public Window
{
 public:
  Slider() { };
  virtual ~Slider() { };

  BOOL Create(DWORD dwStyle = WS_VISIBLE | WS_CHILD, Window *pParent = NULL,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    BOOL bSuccess = Window::Create(dwStyle, dwExStyle, TRACKBAR_CLASS, lpWindowName, pParent, (HMENU) iID);
    if (!bSuccess)
      return FALSE;
    return TRUE;
  };

  int GetLineSize() const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_GETLINESIZE, 0, 0l); }
  int SetLineSize(int nSize)
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_SETLINESIZE, 0, nSize); }
  int GetPageSize() const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_GETPAGESIZE, 0, 0l); }
  int SetPageSize(int nSize)
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_SETPAGESIZE, 0, nSize); }
  int GetRangeMax() const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_GETRANGEMAX, 0, 0l); }
  int GetRangeMin() const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_GETRANGEMIN, 0, 0l); }
  void SetRangeMin(int nMin, BOOL bRedraw)
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_SETRANGEMIN, bRedraw, nMin); }
  void SetRangeMax(int nMax, BOOL bRedraw)
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_SETRANGEMAX, bRedraw, nMax); }
  void ClearSel(BOOL bRedraw)
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_CLEARSEL, bRedraw, 0l); }
  void GetChannelRect(LPRECT lprc) const
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_GETCHANNELRECT, 0, (LPARAM)lprc); }
  void GetThumbRect(LPRECT lprc) const
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_GETTHUMBRECT, 0, (LPARAM)lprc); }
  int GetPos() const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_GETPOS, 0, 0l); }
  void SetPos(int nPos)
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_SETPOS, TRUE, nPos); }
  void ClearTics(BOOL bRedraw)
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_CLEARTICS, bRedraw, 0l); }
  UINT GetNumTics() const
  { niAssert(::IsWindow(m_hWnd)); return (UINT) niWin32API(SendMessage)(m_hWnd, TBM_GETNUMTICS, 0, 0l); }
  DWORD* GetTicArray() const
  { niAssert(::IsWindow(m_hWnd)); return (DWORD*) niWin32API(SendMessage)(m_hWnd, TBM_GETPTICS, 0, 0l); }
  int GetTic(int nTic) const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_GETTIC, nTic, 0L); }
  int GetTicPos(int nTic) const
  { niAssert(::IsWindow(m_hWnd)); return (int) niWin32API(SendMessage)(m_hWnd, TBM_GETTICPOS, nTic, 0L); }
  BOOL SetTic(int nTic)
  { niAssert(::IsWindow(m_hWnd)); return (BOOL)niWin32API(SendMessage)(m_hWnd, TBM_SETTIC, 0, nTic); }
  void SetTicFreq(int nFreq)
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_SETTICFREQ, nFreq, 0L); }
  HWND GetBuddy(BOOL fLocation) const
  { niAssert(::IsWindow(m_hWnd)); return (HWND)niWin32API(SendMessage)(m_hWnd, TBM_GETBUDDY, fLocation, 0l); }
  HWND SetBuddy(HWND hWndBuddy, BOOL fLocation)
  { niAssert(::IsWindow(m_hWnd)); return (HWND)niWin32API(SendMessage)(m_hWnd, TBM_SETBUDDY, fLocation, (LPARAM)hWndBuddy); }
  HWND GetToolTips() const
  { niAssert(::IsWindow(m_hWnd)); return (HWND)niWin32API(SendMessage)(m_hWnd, TBM_GETTOOLTIPS, 0, 0L); }
  void SetToolTips(HWND hWnd)
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, TBM_SETTOOLTIPS, (WPARAM)hWnd, 0L); }
  int SetTipSide(int nLocation)
  { niAssert(::IsWindow(m_hWnd)); return (int)niWin32API(SendMessage)(m_hWnd, TBM_SETTIPSIDE, nLocation, 0L); }
  void GetRange(int& nMin, int& nMax) const
  {
    niAssert(::IsWindow(m_hWnd));
    nMin = GetRangeMin();
    nMax = GetRangeMax();
  }
  void SetRange(int nMin, int nMax, BOOL bRedraw)
  {
    SetRangeMin(nMin, bRedraw);
    SetRangeMax(nMax, bRedraw);
  }
  void GetSelection(int& nMin, int& nMax) const
  {
    niAssert(::IsWindow(m_hWnd));
    nMin = int(niWin32API(SendMessage)(m_hWnd, TBM_GETSELSTART, 0, 0L));
    nMax = int(niWin32API(SendMessage)(m_hWnd, TBM_GETSELEND, 0, 0L));
  }
  void SetSelection(int nMin, int nMax)
  {
    niAssert(::IsWindow(m_hWnd));
    niWin32API(SendMessage)(m_hWnd, TBM_SETSELSTART, 0, (LPARAM)nMin);
    niWin32API(SendMessage)(m_hWnd, TBM_SETSELEND, 0, (LPARAM)nMax);
  }
};

/*! \brief Edit control wrapper
  \sa RichtEdit
*/
class Edit : public Window
{
 public:
  Edit() { };
  virtual ~Edit() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_VISIBLE | WS_CHILD,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    return Window::Create(dwStyle, dwExStyle, _T("EDIT"),
                          lpWindowName, pParent, (HMENU) iID);
  };

  void AppendTextLine(const TCHAR *pszText) {
    SendMessage(EM_SETSEL, -1, -1);
    SendMessage(EM_REPLACESEL, 0, (LPARAM) pszText);
    SendMessage(EM_SCROLLCARET, 0, 0);
  }
  void AppendText(const TCHAR *pszText)
  {
    TCHAR tmp[1024];
    TCHAR* t;
    const TCHAR* p = pszText;
    t = tmp; *t = 0;
    while (*p) {
      if (*p == '\n') {
        if (tmp[0] != 0) {
          *t++ = '\n'; *t = 0;
          AppendTextLine(tmp);
          t = tmp; *t = 0;
        }
      }
      else if (*p == '\r') {
      }
      else {
        if (t-tmp < 1024) {
          *t = *p;
          ++t;
        }
      }
      ++p;
    }
    if (tmp[0] != 0) {
      *t = 0;
      AppendTextLine(tmp);
    }
  };

  void GetTextMetrics(TEXTMETRIC *pTM)
  {
    HDC hDC;
    HFONT hFont;
    hDC = GetDC();
    hFont = (HFONT) SendMessage(WM_GETFONT, 0, 0L);
    if (hFont != NULL)
      SelectObject(hDC, hFont);
    ::GetTextMetrics(hDC, pTM);
    ReleaseDC(hDC);
  };

  UINT LineFromChar(int iChar)
  { return (UINT) SendMessage(EM_LINEFROMCHAR, iChar, 0); };

  UINT GetFirstVisibleLine()
  { return (UINT) SendMessage(EM_GETFIRSTVISIBLELINE, 0, 0); };

  UINT SetSel(DWORD adwBeg, DWORD adwEnd)
  { return (UINT)niWin32API(SendMessage)(m_hWnd, EM_SETSEL, adwBeg, adwEnd); };
  UINT GetSel(DWORD* apBeg, DWORD* apEnd)
  { return (UINT)niWin32API(SendMessage)(m_hWnd, EM_GETSEL, (DWORD)apBeg, (DWORD)apEnd); };

  BOOL Undo()
  { niAssert(::IsWindow(m_hWnd)); return (BOOL)niWin32API(SendMessage)(m_hWnd, EM_UNDO, 0, 0); }
  void Clear()
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, WM_CLEAR, 0, 0); }
  void Copy()
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, WM_COPY, 0, 0); }
  void Cut()
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, WM_CUT, 0, 0); }
  void Paste()
  { niAssert(::IsWindow(m_hWnd)); niWin32API(SendMessage)(m_hWnd, WM_PASTE, 0, 0); }

  BOOL IsReadOnly() const
  { niAssert(IsCreated()); return (GetStyle()&ES_READONLY)==ES_READONLY; };
  LRESULT SetReadOnly(BOOL bEnable = TRUE)
  { niAssert(IsCreated()); return niWin32API(SendMessage)(m_hWnd, EM_SETREADONLY, bEnable, 0); };

 protected:
};

//! \brief Rich edit control wrapper
class RichEdit : public Edit
{
 public:
  RichEdit() { };
  virtual ~RichEdit() { };

  BOOL Create(BaseWindow *pParent, DWORD dwStyle = WS_VISIBLE | WS_CHILD,
              LONG_PTR iID = 0, DWORD dwExStyle = NULL, LPCTSTR lpWindowName = NULL)
  {
    return Window::Create(dwStyle, dwExStyle, _T("RICHEDIT_CLASS"),
                          lpWindowName, pParent, (HMENU) iID);
  };

 protected:

};

//////////////////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////////////////

/*! \brief Inititalize common controls
  \return TRUE in case of success
  \remarks Automatically called by Initialize()
  \sa Initialize()
*/
WINUI_INL BOOL InitCommonControls()
{
  INITCOMMONCONTROLSEX ctrls;
  WINUI_INITSTRUCT(ctrls)

      ctrls.dwICC = ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_DATE_CLASSES |
      ICC_HOTKEY_CLASS | ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES | ICC_PAGESCROLLER_CLASS |
      ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS |
      ICC_USEREX_CLASSES | ICC_WIN95_CLASSES;

  if (!InitCommonControlsEx(&ctrls))
    return FALSE;

  return TRUE;
}

/*! \brief Main initialization function
  \return TRUE in case of success
  \remarks Automatically called by WINUI_DECLARE
  \sa WINUI_DECLARE
*/
WINUI_INL BOOL Initialize()
{
  if (!RegisterClass(WINUI_DEF_WND_CLS))
    return FALSE;
  if (!InitCommonControls())
    return FALSE;

  return TRUE;
}

//////////////////////////////////////////////////////////////////////
// Message loops
//////////////////////////////////////////////////////////////////////

/*! \brief Call to process messages until WM_QUIT is being received
  \sa MainLoopPeek()
  \param hAccelerators Accelerator table which should be used
  \param hWndAccel Handle to the window whose messages are to be translated
*/
WINUI_INL int MainLoop(HACCEL hAccelerators = NULL, HWND hWndAccel = NULL)
{
  MSG msg;
  while (niWin32API(GetMessage)(&msg, NULL, 0, 0))
  {
    if (hAccelerators == NULL || hWndAccel == NULL ||
        TranslateAccelerator(hWndAccel, hAccelerators, &msg) == 0)
    {
      TranslateMessage(&msg);
      niWin32API(DispatchMessage)(&msg);
    }
  }
  return (int) msg.wParam;
}

/*! \brief Call to process all queued messages
  \sa MainLoop()
  \param hAccelerators Accelerator table which should be used
  \param hWndAccel Handle to the window whose messages are to be translated
*/
WINUI_INL void MainLoopPeek(HACCEL hAccelerators = NULL, HWND hWndAccel = NULL)
{
  MSG msg;
  while (niWin32API(PeekMessage)(&msg, NULL, 0, 0, PM_NOREMOVE))
  {
    if (niWin32API(GetMessage)(&msg, NULL, 0, 0))
    {
      if (hAccelerators == NULL || hWndAccel == NULL ||
          TranslateAccelerator(hWndAccel, hAccelerators, &msg) == 0)
      {
        TranslateMessage(&msg);
        niWin32API(DispatchMessage)(&msg);
      }
    }
    else
      return;
  }
}

//////////////////////////////////////////////////////////////////////
// Application base class
//////////////////////////////////////////////////////////////////////

/*! \brief WinUI application base class
  \remarks Do not instantiate, declare using WINUI_DECLARE
  \sa WINUI_DECLARE
*/
class App
{
 public:
  App()
  {
  };
  virtual ~App()
  {
  };

  /*! \brief Entry point for the application's code, override
    \return Exit code of the application
  */
  virtual int Main(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow)
  { return WinUI::MainLoop(); };
};

/*! \brief Declares WinMain() and instantiates an App derived type
  \param AppDerivedType custom App derived class of you application
  \remarks If the macro is not used, the applications needs to provide its own WinMain() and
  call Initialize() manually
  \sa App, Initialize()
*/
#define WINUI_DECLARE(AppDerivedType)                                   \
  static HINSTANCE _WinUI_hInst = NULL;                                 \
  HINSTANCE GetWin32HInstance() { return _WinUI_hInst; }                \
  int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) \
  {                                                                     \
    _WinUI_hInst = hInstance;                                           \
    if (!WinUI::Initialize()) return -1;                                \
    AppDerivedType cApp;                                                \
    return cApp.Main(hInstance, lpCmdLine, nCmdShow);                   \
  }

//////////////////////////////////////////////////////////////////////
// Warnings
//////////////////////////////////////////////////////////////////////

#pragma warning (pop)

//////////////////////////////////////////////////////////////////////////////////////////////
// Registry
//////////////////////////////////////////////////////////////////////////////////////////////

struct RegVal
{
  DWORD dwType;

  union
  {
    DWORD dwNumber;
    LPBYTE pbyteData;
    LPTSTR pszString;
  };
  union
  {
    DWORD dwSize;
    DWORD dwLength;
  };
};

struct cRegVal : public RegVal
{
  /* constructor - automatically initializes registry value data */
  cRegVal ();

  /* destructor - automatically frees registry value data */
  ~cRegVal ();

  /* initializes registry value data */
  void Init ();

  /* frees registry value data */
  void Free ();

  /* get a number */
  bool GetNumber (DWORD *pdwNumber) const;

  /* get binary data */
  bool GetBinary (LPBYTE pbyteData, DWORD dwSize) const;

  /* get new binary data */
  bool GetNewBinary (LPBYTE *pbyteData, DWORD *pdwSize) const;

  /* get a string */
  bool GetString (LPTSTR pszString, DWORD dwLength) const;

  /* get a new string */
  bool GetNewString (LPTSTR *pszString, DWORD *pdwLength) const;

  /* get an array of strings */
  bool GetStringArr (LPTSTR pszStrings[], DWORD dwCount) const;

  /* get a new array of strings */
  bool GetNewStringArr (LPTSTR **pszStrings, DWORD *pdwCount) const;

  /* set a number */
  void SetNumber (DWORD dwNumber);

  /* set binary data */
  bool SetBinary (const LPBYTE pbyteData, DWORD dwSize);

  /* set a string */
  bool SetString (LPCTSTR pszString);

  /* set an array of strings */
  bool SetStringArr (const LPCTSTR pszStrings[], DWORD dwCount);
};

struct cReg
{
  HKEY hKey;

  /* constructor - automatically initializes registry data */
  cReg ();

  /* destructor - automatically frees registry data */
  ~cReg ();

  /* connect to remote computer registry */
  HKEY Connect (HKEY hNewKey, LPCTSTR pszRemote);

  /* connect to registry key */
  HKEY Open (HKEY hNewKey = NULL);

  /* open computer registry */
  HKEY Open (HKEY hNewKey, LPCTSTR pszSubKey, DWORD dwRights);

  /* close computer registry */
  void Close ();

  /* create computer registry */
  HKEY Create (HKEY hNewKey, LPCTSTR pszSubKey, DWORD dwRights);

  /* load data of any type */
  bool LoadVal (LPCTSTR pszValName, RegVal *pValData);

  /* load data of any type from subkey */
  bool LoadVal (LPCTSTR pszSubKey, LPCTSTR pszValName, RegVal *pValData);

  /* load a number */
  bool LoadNumber (LPCTSTR pszValName, DWORD *pdwNumber);

  /* load a number from subkey */
  bool LoadNumber (LPCTSTR pszSubKey, LPCTSTR pszValName, DWORD *pdwNumber);

  /* load binary data */
  bool LoadBinary (LPCTSTR pszValName, LPBYTE pbyteData, DWORD dwSize);

  /* load binary data from subkey */
  bool LoadBinary (LPCTSTR pszSubKey, LPCTSTR pszValName, LPBYTE pbyteData, DWORD dwSize);

  /* load new binary data */
  bool LoadNewBinary (LPCTSTR pszValName, LPBYTE *pbyteData, DWORD *pdwSize);

  /* load new binary data from subkey */
  bool LoadNewBinary (LPCTSTR pszSubKey, LPCTSTR pszValName, LPBYTE *pbyteData, DWORD *pdwSize);

  /* load a string */
  bool LoadString (LPCTSTR pszValName, LPTSTR pszString, DWORD dwLength);

  /* load a string from subkey */
  bool LoadString (LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR pszString, DWORD dwLength);

  /* load a new string */
  bool LoadNewString (LPCTSTR pszValName, LPTSTR *pszString, DWORD *pdwLength);

  /* load a new string from subkey */
  bool LoadNewString (LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR *pszString, DWORD *pdwLength);

  /* load an array of strings */
  bool LoadStringArr (LPCTSTR pszValName, LPTSTR pszStrings[], DWORD dwCount);

  /* load an array of strings from subkey */
  bool LoadStringArr (LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR pszStrings[], DWORD dwCount);

  /* load a new array of strings */
  bool LoadNewStringArr (LPCTSTR pszValName, LPTSTR **pszStrings, DWORD *pdwCount);

  /* load a new array of strings from subkey */
  bool LoadNewStringArr (LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR **pszStrings, DWORD *pdwCount);

  /* load a string */
  bool LoadString (LPCTSTR pszValName, ni::cString &sString);

  /* load a string from subkey */
  bool LoadString (LPCTSTR pszSubKey, LPCTSTR pszValName, ni::cString &sString);

  /* store data of any type */
  bool SaveVal (LPCTSTR pszValName, const RegVal *pValData);

  /* store data of any type to subkey */
  bool SaveVal (LPCTSTR pszSubKey, LPCTSTR pszValName, const RegVal *pValData);

  /* store a number */
  bool SaveNumber (LPCTSTR pszValName, DWORD dwNumber);

  /* store a number to subkey */
  bool SaveNumber (LPCTSTR pszSubKey, LPCTSTR pszValName, DWORD dwNumber);

  /* store binary data */
  bool SaveBinary (LPCTSTR pszValName, const LPBYTE pbyteData, DWORD dwSize);

  /* store binary data to subkey */
  bool SaveBinary (LPCTSTR pszSubKey, LPCTSTR pszValName, const LPBYTE pbyteData, DWORD dwSize);

  /* store a string */
  bool SaveString (LPCTSTR pszValName, LPCTSTR pszString);

  /* store a string to subkey */
  bool SaveString (LPCTSTR pszSubKey, LPCTSTR pszValName, LPCTSTR pszString);

  /* store an array of strings */
  bool SaveStringArr (LPCTSTR pszValName, const LPCTSTR pszStrings[], DWORD dwCount);

  /* store an array of strings to subkey */
  bool SaveStringArr (LPCTSTR pszSubKey, LPCTSTR pszValName, const LPCTSTR pszStrings[], DWORD dwCount);

  /* delete the given value or key in the registry with all of its subkeys */
  bool DeleteKey (LPCTSTR pszValName);

  /* delete the given value or key in the registry with all of its subkeys in subkey */
  bool DeleteKey (LPCTSTR pszSubKey, LPCTSTR pszValName);

  /* delete all of subkeys in the key */
  bool DeleteSubKeys ();

  /* check wether the given key has other subkeys and/or values */
  bool HasEntries (DWORD *pdwSubKeyCount, DWORD *pdwValueCount);

  /* check wether the given key has other subkeys and/or values in subkey */
  bool HasEntries (LPCTSTR pszSubKey, DWORD *pdwSubKeyCount, DWORD *pdwValueCount);

  /* walks to the first value */
  bool FindFirstValue (LPCTSTR &ppszValue, RegVal *pValData);

  /* walks to the next value */
  bool FindNextValue (LPCTSTR &ppszValue, RegVal *pValData);

  /* closes registry walking */
  void FindClose ();
};

/* initializes registry value data */
inline void
RegValInit (RegVal *pValData)
{
  niAssert (pValData);
  pValData->dwType = REG_NONE;
}

/* frees registry value data */
inline void
RegValFree (RegVal *pValData)
{
  niAssert (pValData);
  if (pValData->dwType == REG_SZ || pValData->dwType == REG_EXPAND_SZ
      || pValData->dwType == REG_LINK || pValData->dwType == REG_MULTI_SZ
      || pValData->dwType == REG_BINARY)
  {
    free (pValData->pbyteData);
    pValData->dwType = REG_NONE;
  }
}

/* get a number */
inline bool
RegValGetNumber (const RegVal *pValData, DWORD *pdwNumber)
{
  niAssert (pValData &&pdwNumber);
  if (pValData->dwType == REG_DWORD)
  {
    *pdwNumber = pValData->dwNumber;
    return true;
  }
  return false;
}

/* get binary data */
inline bool
RegValGetBinary (const RegVal *pValData, LPBYTE pbyteData, DWORD dwSize)
{
  niAssert (pValData &&pbyteData);
  if (pValData->dwType == REG_BINARY &&dwSize >= pValData->dwSize)
  {
    memcpy (pbyteData, pValData->pbyteData, pValData->dwSize);
    return true;
  }
  return false;
}

/* get new binary data */
inline bool
RegValGetNewBinary (const RegVal *pValData, LPBYTE *pbyteData, DWORD *pdwSize)
{
  niAssert (pValData &&pbyteData);
  if (pValData->dwType == REG_BINARY)
  {
    LPBYTE pbyteNewData = (LPBYTE) malloc (pValData->dwSize);
    if (pbyteNewData)
    {
      *pbyteData = pbyteNewData;
      *pdwSize = pValData->dwSize;
      memcpy (pbyteNewData, pValData->pbyteData, pValData->dwSize);
      return true;
    }
  }
  return false;
}

/* get a new string */
inline bool
RegValGetNewString (const RegVal *pValData, LPTSTR *pszString, DWORD *pdwLength)
{
  niAssert (pValData &&pszString);
  if (pValData->dwType == REG_SZ || pValData->dwType == REG_EXPAND_SZ
      || pValData->dwType == REG_LINK || pValData->dwType == REG_MULTI_SZ)
  {
    LPTSTR pszNewString = (LPTSTR) malloc (pValData->dwLength + 1);
    if (pszNewString)
    {
      *pszString = pszNewString;
      if (pdwLength)
      {
        *pdwLength = pValData->dwLength;
      }
      memcpy (pszNewString, pValData->pszString, pValData->dwLength);
      pszNewString [pValData->dwLength] = _T ('\0');
      return true;
    }
  }
  return false;
}

/* get a string */
inline bool
RegValGetString (const RegVal *pValData, LPTSTR pszString, DWORD dwLength)
{
  niAssert (pValData &&pszString);
  if ((pValData->dwType == REG_SZ || pValData->dwType == REG_EXPAND_SZ
       || pValData->dwType == REG_LINK || pValData->dwType == REG_MULTI_SZ)
      && dwLength >= pValData->dwLength)
  {
    memcpy (pszString, pValData->pszString, pValData->dwLength);
    pszString [pValData->dwLength] = _T ('\0');
    return true;
  }
  return false;
}

/* get an array of strings */
inline bool
RegValGetStringArr (const RegVal *pValData, LPTSTR pszStrings[], DWORD dwCount)
{
  niAssert (pValData);
  if (pValData->dwType == REG_MULTI_SZ)
  {
    LPCTSTR pszString;
    size_t dwRealCount = 0, dwLength;
    for (pszString = pValData->pszString; *pszString; pszString += dwLength)
    {
      dwLength = _tcslen (pszString) + 1;
      dwRealCount++;
    }
    if (dwCount >= dwRealCount)
    {
      LPTSTR *pszDstString = pszStrings;
      for (pszString = pValData->pszString; *pszString; pszString += dwLength, pszDstString++)
      {
        dwLength = _tcslen (pszString) + 1;
        LPTSTR pszNewString = (LPTSTR) malloc (dwLength);
        *pszDstString = pszNewString;
        if (pszNewString)
        {
          while ((*pszNewString = (BYTE) *pszString) != _T ('\0'))
          {
            pszNewString++;
            pszString++;
          }
        }
        else
        {
          while (*pszString)
          {
            pszString++;
          }
        }
      }
      return true;
    }
  }
  return false;
}

/* get a new array of strings */
inline bool
RegValGetNewStringArr (const RegVal *pValData, LPTSTR **pszStrings, DWORD *pdwCount)
{
  niAssert (pValData);
  if (pValData->dwType == REG_MULTI_SZ)
  {
    LPTSTR pszString;
    size_t dwRealCount = 0, dwLength;
    for (pszString = pValData->pszString; *pszString; pszString += dwLength)
    {
      dwLength = _tcslen (pszString) + 1;
      dwRealCount++;
    }
    LPTSTR *pszNewStrings = (LPTSTR *) malloc (dwRealCount *sizeof (LPTSTR));
    if (pszNewStrings)
    {
      *pszStrings = pszNewStrings;
      *pdwCount = (DWORD)dwRealCount;
      for (pszString = pValData->pszString; *pszString; pszString += dwLength, pszNewStrings++)
      {
        dwLength = _tcslen (pszString) + 1;
        LPTSTR pszNewString = (LPTSTR) malloc (dwLength);
        *pszNewStrings = pszNewString;
        if (pszNewString)
        {
          while ((*pszNewString = (BYTE) *pszString) != _T ('\0'))
          {
            pszNewString++;
            pszString++;
          }
        }
        else
        {
          while (*pszString)
          {
            pszString++;
          }
        }
      }
      return true;
    }
  }
  return false;
}

/* set a number */
inline void
RegValSetNumber (RegVal *pValData, DWORD dwNumber)
{
  niAssert (pValData);
  pValData->dwType = REG_DWORD;
  pValData->dwNumber = dwNumber;
}

/* set binary data */
inline bool
RegValSetBinary (RegVal *pValData, const LPBYTE pbyteData, DWORD dwSize)
{
  niAssert (pValData &&pbyteData);
  pValData->pbyteData = (LPBYTE) malloc (dwSize);
  if (pValData->pbyteData)
  {
    pValData->dwSize = dwSize;
    pValData->dwType = REG_BINARY;
    memcpy (pValData->pbyteData, pbyteData, dwSize);
    return true;
  }
  pValData->dwType = REG_NONE;
  return false;
}

/* set a string */
inline bool
RegValSetString (RegVal *pValData, LPCTSTR pszString)
{
  niAssert (pValData &&pszString);
  DWORD dwLength = (DWORD)(_tcslen (pszString) + 1);
  pValData->pszString = (LPTSTR) malloc (dwLength);
  if (pValData->pszString)
  {
    pValData->dwLength = dwLength;
    pValData->dwType = REG_SZ;
    memcpy (pValData->pbyteData, pszString, dwLength);
    return true;
  }
  pValData->dwType = REG_NONE;
  return false;
}

inline bool
RegValSetStringArr (RegVal *pValData, const LPCTSTR pszStrings[], DWORD dwCount)
{
  niAssert (pValData &&pszStrings);
  DWORD i, dwSize = 1;
  if (dwCount) {
    for (i = 0; i < dwCount; i++) {
      dwSize += (DWORD)(_tcslen (pszStrings[i]) + 1);
    }
  }
  else
  {
    dwSize++;
  }
  pValData->pbyteData = (LPBYTE) malloc (dwSize);
  if (pValData->pbyteData)
  {
    pValData->dwSize = dwSize;
    pValData->dwType = REG_MULTI_SZ;
    LPBYTE pbyteData = pValData->pbyteData;
    if (dwCount)
    {
      for (i = 0; i < dwCount; i++)
      {
        LPCTSTR pszString = pszStrings[i];
        while ((*pbyteData++ = (BYTE) *pszString) != _T ('\0'))
        {
          pszString++;
        }
      }
    }
    else
    {
      *pbyteData++ = _T ('\0');
    }
    *pbyteData = _T ('\0');
    return true;
  }
  pValData->dwType = REG_NONE;
  return false;
}

/* connect to remote computer registry */
inline HKEY
RegConnect (HKEY hKey, LPCTSTR pszRemote)
{
  HKEY hSubKey;
  if (RegConnectRegistry (pszRemote, hKey, &hSubKey) == ERROR_SUCCESS)
  {
    return hSubKey;
  }
  return NULL;
}

/* open computer registry */
inline HKEY
RegOpen (HKEY hKey, LPCTSTR pszSubKey, DWORD dwRights)
{
  HKEY hSubKey;
  if (RegOpenKeyEx (hKey, pszSubKey, 0, dwRights, &hSubKey) == ERROR_SUCCESS)
  {
    return hSubKey;
  }
  return NULL;
}

/* create computer registry */
inline HKEY
RegCreate (HKEY hKey, LPCTSTR pszSubKey, DWORD dwRights)
{
  HKEY hSubKey;
  DWORD dwDisposition;
  if (RegCreateKeyEx (hKey, pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE, dwRights,
                      NULL, &hSubKey, &dwDisposition) == ERROR_SUCCESS)
  {
    return hSubKey;
  }
  return NULL;
}

/* close computer registry */
inline void
RegClose (HKEY hKey)
{
  if (hKey)
    RegCloseKey (hKey);
}

/* load data of any type */
inline bool
RegLoadVal (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, RegVal *pValData)
{
  niAssert (pValData);
  HKEY hSubKey = pszSubKey ? RegOpen (hKey, pszSubKey, KEY_READ) : hKey;
  if (hSubKey)
  {
    DWORD dwType, dwSize;
    if (RegQueryValueEx (hSubKey, pszValName, 0, &dwType, NULL, &dwSize) == ERROR_SUCCESS)
    {
      if (dwType == REG_DWORD)
      {
        niAssert (dwSize == sizeof (DWORD));
        DWORD dwNumber;
        if (RegQueryValueEx (hSubKey, pszValName, 0, NULL, (LPBYTE) &dwNumber, &dwSize) == ERROR_SUCCESS)
        {
          niAssert (dwSize == sizeof (DWORD));
          RegValFree (pValData);
          pValData->dwType = dwType;
          pValData->dwNumber = dwNumber;
          if (hSubKey != hKey)
            RegClose (hSubKey);
          return true;
        }
      }
      else if (dwType == REG_SZ || dwType == REG_EXPAND_SZ || dwType == REG_LINK
               || dwType == REG_MULTI_SZ || dwType == REG_BINARY)
      {
        LPBYTE pbyteData = (LPBYTE) malloc (dwSize);
        if (pbyteData)
        {
          if (RegQueryValueEx (hSubKey, pszValName, 0, NULL, pbyteData, &dwSize) == ERROR_SUCCESS)
          {
            RegValFree (pValData);
            pValData->dwType = dwType;
            pValData->pbyteData = pbyteData;
            pValData->dwLength = dwSize;
            if (hSubKey != hKey)
              RegClose (hSubKey);
            return true;
          }
          free (pbyteData);
        }
      }
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* load a number */
inline bool
RegLoadNumber (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, DWORD *pdwNumber)
{
  niAssert (pdwNumber);
  HKEY hSubKey = pszSubKey ? RegOpen (hKey, pszSubKey, KEY_READ) : hKey;
  if (hSubKey)
  {
    DWORD dwType, dwSize;
    if (RegQueryValueEx (hSubKey, pszValName, 0, &dwType, NULL, &dwSize) == ERROR_SUCCESS)
    {
      if (dwType == REG_DWORD)
      {
        niAssert (dwSize == sizeof (DWORD));
        if (RegQueryValueEx (hSubKey, pszValName, 0, NULL, (LPBYTE) pdwNumber, &dwSize) == ERROR_SUCCESS)
        {
          niAssert (dwSize == sizeof (DWORD));
          if (hSubKey != hKey)
            RegClose (hSubKey);
          return true;
        }
      }
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* load binary data */
inline bool
RegLoadBinary (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, LPBYTE pbyteData, DWORD dwSize)
{
  niAssert (pbyteData);
  HKEY hSubKey = pszSubKey ? RegOpen (hKey, pszSubKey, KEY_READ) : hKey;
  if (hSubKey)
  {
    DWORD dwType, dwRealSize;
    if (RegQueryValueEx (hSubKey, pszValName, 0, &dwType, NULL, &dwRealSize) == ERROR_SUCCESS)
    {
      if (dwType == REG_BINARY &&dwSize >= dwRealSize)
      {
        if (RegQueryValueEx (hSubKey, pszValName, 0, NULL, pbyteData, &dwRealSize) == ERROR_SUCCESS)
        {
          if (hSubKey != hKey)
            RegClose (hSubKey);
          return true;
        }
      }
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* load new binary data */
inline bool
RegLoadNewBinary (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, LPBYTE *pbyteData, DWORD *pdwSize)
{
  niAssert (pbyteData);
  HKEY hSubKey = pszSubKey ? RegOpen (hKey, pszSubKey, KEY_READ) : hKey;
  if (hSubKey)
  {
    DWORD dwType, dwRealSize;
    if (RegQueryValueEx (hSubKey, pszValName, 0, &dwType, NULL, &dwRealSize) == ERROR_SUCCESS)
    {
      if (dwType == REG_BINARY)
      {
        LPBYTE pbyteNewData = (LPBYTE) malloc (dwRealSize);
        if (pbyteNewData)
        {
          if (RegQueryValueEx (hSubKey, pszValName, 0, NULL, pbyteNewData, &dwRealSize) == ERROR_SUCCESS)
          {
            *pbyteData = pbyteNewData;
            *pdwSize = dwRealSize;
            if (hSubKey != hKey)
              RegClose (hSubKey);
            return true;
          }
          free (pbyteNewData);
        }
      }
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* load a string */
inline bool
RegLoadString (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR pszString, DWORD dwLength)
{
  niAssert (pszString);
  HKEY hSubKey = pszSubKey ? RegOpen (hKey, pszSubKey, KEY_READ) : hKey;
  if (hSubKey)
  {
    DWORD dwType, dwRealLength;
    if (RegQueryValueEx (hSubKey, pszValName, 0, &dwType, NULL, &dwRealLength) == ERROR_SUCCESS)
    {
      if ((dwType == REG_SZ || dwType == REG_EXPAND_SZ || dwType == REG_LINK
           || dwType == REG_MULTI_SZ) &&dwLength >= dwRealLength)
      {
        if (RegQueryValueEx (hSubKey, pszValName, 0, NULL, (LPBYTE) pszString, &dwRealLength) == ERROR_SUCCESS)
        {
          if (hSubKey != hKey)
            RegClose (hSubKey);
          return true;
        }
      }
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* load a new string */
inline bool
RegLoadNewString (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR *pszString, DWORD *pdwLength)
{
  niAssert (pszString);
  HKEY hSubKey = pszSubKey ? RegOpen (hKey, pszSubKey, KEY_READ) : hKey;
  if (hSubKey)
  {
    DWORD dwType, dwRealLength;
    if (RegQueryValueEx (hSubKey, pszValName, 0, &dwType, NULL, &dwRealLength) == ERROR_SUCCESS)
    {
      if (dwType == REG_SZ || dwType == REG_EXPAND_SZ || dwType == REG_LINK
          || dwType == REG_MULTI_SZ)
      {
        LPTSTR pszNewString = (LPTSTR) malloc (dwRealLength);
        if (pszNewString)
        {
          if (RegQueryValueEx (hSubKey, pszValName, 0, NULL, (LPBYTE) pszNewString, &dwRealLength) == ERROR_SUCCESS)
          {
            *pszString = pszNewString;
            if (pdwLength)
            {
              *pdwLength = dwRealLength;
            }
            if (hSubKey != hKey)
              RegClose (hSubKey);
            return true;
          }
          free (pszNewString);
        }
      }
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* load an array of strings */
inline bool
RegLoadStringArr (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR pszStrings[], DWORD dwCount)
{
  RegVal Value;
  if (RegLoadVal (hKey, pszSubKey, pszValName, &Value))
  {
    if (RegValGetStringArr (&Value, pszStrings, dwCount))
    {
      RegValFree (&Value);
      return true;
    }
    RegValFree (&Value);
  }
  return false;
}

/* load a new array of strings */
inline bool
RegLoadNewStringArr (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR **pszStrings, DWORD *pdwCount)
{
  RegVal Value;
  if (RegLoadVal (hKey, pszSubKey, pszValName, &Value))
  {
    if (RegValGetNewStringArr (&Value, pszStrings, pdwCount))
    {
      RegValFree (&Value);
      return true;
    }
    RegValFree (&Value);
  }
  return false;
}

/* store data of any type */
inline bool RegSaveVal (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, const RegVal *pValData)
{
  niAssert (pValData);
  HKEY hSubKey = pszSubKey ? RegCreate (hKey, pszSubKey, KEY_WRITE) : hKey;
  if (hSubKey)
  {
    LONG lResult;
    if (pValData->dwType == REG_DWORD)
    {
      lResult = RegSetValueEx (hSubKey, pszValName, 0, pValData->dwType, (LPBYTE) &pValData->dwNumber, sizeof (DWORD));
    }
    else if (pValData->dwType == REG_SZ || pValData->dwType == REG_EXPAND_SZ
             || pValData->dwType == REG_LINK || pValData->dwType == REG_MULTI_SZ
             || pValData->dwType == REG_BINARY)
    {
      lResult = RegSetValueEx (hSubKey, pszValName, 0, pValData->dwType, pValData->pbyteData, pValData->dwSize);
    }
    else
    {
      lResult = ERROR_BAD_FORMAT;
    }
    if (lResult == ERROR_SUCCESS)
    {
      if (hSubKey != hKey)
        RegClose (hSubKey);
      return true;
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* store a number */
inline bool
RegSaveNumber (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, DWORD dwNumber)
{
  HKEY hSubKey = pszSubKey ? RegCreate (hKey, pszSubKey, KEY_WRITE) : hKey;
  if (hSubKey)
  {
    if (RegSetValueEx (hSubKey, pszValName, 0, REG_DWORD, (LPBYTE) &dwNumber, sizeof (DWORD)) == ERROR_SUCCESS)
    {
      if (hSubKey != hKey)
        RegClose (hSubKey);
      return true;
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* store binary data */
inline bool
RegSaveBinary (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, const LPBYTE pbyteData, DWORD dwSize)
{
  HKEY hSubKey = pszSubKey ? RegCreate (hKey, pszSubKey, KEY_WRITE) : hKey;
  if (hSubKey)
  {
    if (RegSetValueEx (hSubKey, pszValName, 0, REG_BINARY, pbyteData, dwSize) == ERROR_SUCCESS)
    {
      if (hSubKey != hKey)
        RegClose (hSubKey);
      return true;
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* store a string */
inline bool
RegSaveString (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, LPCTSTR pszString)
{
  HKEY hSubKey = pszSubKey ? RegCreate (hKey, pszSubKey, KEY_WRITE) : hKey;
  if (hSubKey)
  {
    if (RegSetValueEx(hSubKey, pszValName, 0, REG_SZ, (LPBYTE)pszString, (DWORD)(_tcslen(pszString) + 1)) == ERROR_SUCCESS)
    {
      if (hSubKey != hKey)
        RegClose (hSubKey);
      return true;
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* store an array of strings */
inline bool
RegSaveStringArr (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName, const LPCTSTR pszStrings[], DWORD dwCount)
{
  RegVal Value;
  if (RegValSetStringArr (&Value, pszStrings, dwCount))
  {
    if (RegSaveVal (hKey, pszSubKey, pszValName, &Value))
    {
      RegValFree (&Value);
      return true;
    }
    RegValFree (&Value);
  }
  return false;
}

/* delete all subkeys in the given key */
inline bool
RegDeleteSubKeys (HKEY hKey)
{
  DWORD dwSubKeyCnt, dwMaxSubKey;
  if (RegQueryInfoKey (hKey, NULL, NULL, 0, &dwSubKeyCnt, &dwMaxSubKey,
                       NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
  {
    if (dwSubKeyCnt)
    {
      LPTSTR pszKeyName = (LPTSTR) malloc (dwMaxSubKey += 1);
      if (pszKeyName)
      {
        do
        {
          if (RegEnumKey (hKey, --dwSubKeyCnt, pszKeyName, dwMaxSubKey) == ERROR_SUCCESS)
          {
            HKEY hSubKey = RegOpen (hKey, pszKeyName, KEY_READ | KEY_WRITE);
            if (hSubKey)
            {
              if (RegDeleteSubKeys (hSubKey))
              {
                RegClose (hSubKey);
                if (RegDeleteKey (hKey, pszKeyName) != ERROR_SUCCESS)
                {
                  free (pszKeyName);
                  return false;
                }
              }
              else
              {
                RegClose (hSubKey);
                free (pszKeyName);
                return false;
              }
            }
            else
            {
              free (pszKeyName);
              return false;
            }
          }
          else
          {
            free (pszKeyName);
            return false;
          }
        }
        while (dwSubKeyCnt);
        free (pszKeyName);
      }
      else
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

/* delete the given value or key in the registry with all of its subkeys */
inline bool
RegDeleteKey (HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValName)
{
  HKEY hSubKey = pszSubKey ? RegOpen (hKey, pszSubKey, KEY_READ | KEY_WRITE) : hKey;
  if (hSubKey)
  {
    if (pszValName)
    {
      if (RegDeleteValue (hSubKey, pszValName) == ERROR_SUCCESS)
      {
        if (hSubKey != hKey)
          RegClose (hSubKey);
        return true;
      }
    }
    else
    {
      if (RegDeleteSubKeys(hSubKey)) {
        if (hSubKey != hKey) {
          RegClose(hSubKey);
        }
        if (pszSubKey != NULL) {
          return RegDeleteKey(hKey, pszSubKey) == ERROR_SUCCESS;
        }
        return true;
      }
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  return false;
}

/* check wether the given key has other subkeys and/or values */
inline bool
RegHasEntries (HKEY hKey, LPCTSTR pszSubKey, DWORD *pdwSubKeyCount, DWORD *pdwValueCount)
{
  HKEY hSubKey = pszSubKey ? RegOpen (hKey, pszSubKey, KEY_READ) : hKey;
  if (hSubKey)
  {
    if (RegQueryInfoKey (hSubKey, NULL, NULL, 0, pdwSubKeyCount, NULL, NULL, pdwValueCount, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
      if (hSubKey != hKey)
        RegClose (hSubKey);
      return true;
    }
    if (hSubKey != hKey)
      RegClose (hSubKey);
  }
  *pdwSubKeyCount = *pdwValueCount = 0;
  return false;
}

/* constructor - automatically initializes registry value data */
inline cRegVal::cRegVal ()
{
  Init ();
}

/* destructor - automatically frees registry value data */
inline cRegVal::~cRegVal ()
{
  Free ();
}

/* initializes registry value data */
inline void cRegVal::Init ()
{
  RegValInit (this);
}

/* frees registry value data */
inline void cRegVal::Free ()
{
  RegValFree (this);
}

/* get a number */
inline bool cRegVal::GetNumber (DWORD *pdwNumber) const
{
  return RegValGetNumber (this, pdwNumber);
}

/* get binary data */
inline bool cRegVal::GetBinary (LPBYTE pbyteData, DWORD dwSize) const
{
  return RegValGetBinary (this, pbyteData, dwSize);
}

/* get new binary data */
inline bool cRegVal::GetNewBinary (LPBYTE *pbyteData, DWORD *pdwSize) const
{
  return RegValGetNewBinary (this, pbyteData, pdwSize);
}

/* get a string */
inline bool cRegVal::GetString (LPTSTR pszString, DWORD dwLength) const
{
  return RegValGetString (this, pszString, dwLength);
}

/* get a new string */
inline bool cRegVal::GetNewString (LPTSTR *pszString, DWORD *pdwLength) const
{
  return RegValGetNewString (this, pszString, pdwLength);
}

/* get an array of strings */
inline bool cRegVal::GetStringArr (LPTSTR pszStrings[], DWORD dwCount) const
{
  return RegValGetStringArr (this, pszStrings, dwCount);
}

/* get a new array of strings */
inline bool cRegVal::GetNewStringArr (LPTSTR **pszStrings, DWORD *pdwCount) const
{
  return RegValGetNewStringArr (this, pszStrings, pdwCount);
}

/* set a number */
inline void cRegVal::SetNumber (DWORD dwNumber)
{
  RegValSetNumber (this, dwNumber);
}

/* set binary data */
inline bool cRegVal::SetBinary (const LPBYTE pbyteData, DWORD dwSize)
{
  return RegValSetBinary (this, pbyteData, dwSize);
}

/* set a string */
inline bool cRegVal::SetString (LPCTSTR pszString)
{
  return RegValSetString (this, pszString);
}

/* set an array of strings */
inline bool cRegVal::SetStringArr (const LPCTSTR pszStrings[], DWORD dwCount)
{
  return RegValSetStringArr (this, pszStrings, dwCount);
}

/* constructor - automatically initializes registry data */
inline cReg::cReg ()
{
  Open ();
}

/* destructor - automatically frees registry data */
inline cReg::~cReg ()
{
  Close ();
}

/* connect to remote computer registry */
inline HKEY cReg::Connect (HKEY hNewKey, LPCTSTR pszRemote)
{
  return hKey = RegConnect (hNewKey, pszRemote);
}

/* connect to registry key */
inline HKEY cReg::Open (HKEY hNewKey /*= NULL*/)
{
  return hKey = hNewKey;
}

/* open computer registry */
inline HKEY cReg::Open (HKEY hNewKey, LPCTSTR pszSubKey, DWORD dwRights)
{
  return hKey = RegOpen (hNewKey, pszSubKey, dwRights);
}

/* close computer registry */
inline void cReg::Close ()
{
  if (hKey != NULL) // MAB 8 Nov 1999 - added NULL test
  {
    RegClose (hKey);
    //*** MIPO 07-12-1999 - After Closing the Key, hKey must by NULL ****
    // RegClose - dont do that - it must be done manualy
    hKey = NULL;
  }
}

/* create computer registry */
inline HKEY cReg::Create (HKEY hNewKey, LPCTSTR pszSubKey, DWORD dwRights)
{
  return hKey = RegCreate (hNewKey, pszSubKey, dwRights);
}

/* load data of any type */
inline bool cReg::LoadVal (LPCTSTR pszValName, RegVal *pValData)
{
  return RegLoadVal (hKey, NULL, pszValName, pValData);
}

/* load data of any type from subkey */
inline bool cReg::LoadVal (LPCTSTR pszSubKey, LPCTSTR pszValName, RegVal *pValData)
{
  return RegLoadVal (hKey, pszSubKey, pszValName, pValData);
}

/* load a number */
inline bool cReg::LoadNumber (LPCTSTR pszValName, DWORD *pdwNumber)
{
  return RegLoadNumber (hKey, NULL, pszValName, pdwNumber);
}

/* load a number from subkey */
inline bool cReg::LoadNumber (LPCTSTR pszSubKey, LPCTSTR pszValName, DWORD *pdwNumber)
{
  return RegLoadNumber (hKey, pszSubKey, pszValName, pdwNumber);
}

/* load binary data */
inline bool cReg::LoadBinary (LPCTSTR pszValName, LPBYTE pbyteData, DWORD dwSize)
{
  return RegLoadBinary (hKey, NULL, pszValName, pbyteData, dwSize);
}

/* load binary data from subkey */
inline bool cReg::LoadBinary (LPCTSTR pszSubKey, LPCTSTR pszValName, LPBYTE pbyteData, DWORD dwSize)
{
  return RegLoadBinary (hKey, pszSubKey, pszValName, pbyteData, dwSize);
}

/* load new binary data */
inline bool cReg::LoadNewBinary (LPCTSTR pszValName, LPBYTE *pbyteData, DWORD *pdwSize)
{
  return RegLoadNewBinary (hKey, NULL, pszValName, pbyteData, pdwSize);
}

/* load new binary data from subkey */
inline bool cReg::LoadNewBinary (LPCTSTR pszSubKey, LPCTSTR pszValName, LPBYTE *pbyteData, DWORD *pdwSize)
{
  return RegLoadNewBinary (hKey, pszSubKey, pszValName, pbyteData, pdwSize);
}

/* load a string */
inline bool cReg::LoadString (LPCTSTR pszValName, LPTSTR pszString, DWORD dwLength)
{
  return RegLoadString (hKey, NULL, pszValName, pszString, dwLength);
}

/* load a string from subkey */
inline bool cReg::LoadString (LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR pszString, DWORD dwLength)
{
  return RegLoadString (hKey, pszSubKey, pszValName, pszString, dwLength);
}

/* load a new string */
inline bool cReg::LoadNewString (LPCTSTR pszValName, LPTSTR *pszString, DWORD *pdwLength)
{
  return RegLoadNewString (hKey, NULL, pszValName, pszString, pdwLength);
}

/* load a new string from subkey */
inline bool cReg::LoadNewString (LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR *pszString, DWORD *pdwLength)
{
  return RegLoadNewString (hKey, pszSubKey, pszValName, pszString, pdwLength);
}

/* load an array of strings */
inline bool cReg::LoadStringArr (LPCTSTR pszValName, LPTSTR pszStrings[], DWORD dwCount)
{
  return RegLoadStringArr (hKey, NULL, pszValName, pszStrings, dwCount);
}

/* load an array of strings from subkey */
inline bool cReg::LoadStringArr (LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR pszStrings[], DWORD dwCount)
{
  return RegLoadStringArr (hKey, pszSubKey, pszValName, pszStrings, dwCount);
}

/* load a new array of strings */
inline bool cReg::LoadNewStringArr (LPCTSTR pszValName, LPTSTR **pszStrings, DWORD *pdwCount)
{
  return RegLoadNewStringArr (hKey, NULL, pszValName, pszStrings, pdwCount);
}

/* load a new array of strings from subkey */
inline bool cReg::LoadNewStringArr (LPCTSTR pszSubKey, LPCTSTR pszValName, LPTSTR **pszStrings, DWORD *pdwCount)
{
  return RegLoadNewStringArr (hKey, pszSubKey, pszValName, pszStrings, pdwCount);
}

/* load a string */
inline bool cReg::LoadString (LPCTSTR pszValName, ni::cString &sString)
{
  ni::achar aaszTmp[4096];
  if (!RegLoadString (hKey, NULL, pszValName, aaszTmp, 4096)) return false;
  sString = aaszTmp;
  return true;
}

/* load a string from subkey */
inline bool cReg::LoadString (LPCTSTR pszSubKey, LPCTSTR pszValName, ni::cString &sString)
{
  ni::achar aaszTmp[4096];
  if (!RegLoadString (hKey, pszSubKey, pszValName, aaszTmp, 4096)) return false;
  sString = aaszTmp;
  return true;
}

/* store data of any type */
inline bool cReg::SaveVal (LPCTSTR pszValName, const RegVal *pValData)
{
  return RegSaveVal (hKey, NULL, pszValName, pValData);
}

/* store data of any type to subkey */
inline bool cReg::SaveVal (LPCTSTR pszSubKey, LPCTSTR pszValName, const RegVal *pValData)
{
  return RegSaveVal (hKey, pszSubKey, pszValName, pValData);
}

/* store a number */
inline bool cReg::SaveNumber (LPCTSTR pszValName, DWORD dwNumber)
{
  return RegSaveNumber (hKey, NULL, pszValName, dwNumber);
}

/* store a number to subkey */
inline bool cReg::SaveNumber (LPCTSTR pszSubKey, LPCTSTR pszValName, DWORD dwNumber)
{
  return RegSaveNumber (hKey, pszSubKey, pszValName, dwNumber);
}

/* store binary data */
inline bool cReg::SaveBinary (LPCTSTR pszValName, const LPBYTE pbyteData, DWORD dwSize)
{
  return RegSaveBinary (hKey, NULL, pszValName, pbyteData, dwSize);
}

/* store binary data to subkey */
inline bool cReg::SaveBinary (LPCTSTR pszSubKey, LPCTSTR pszValName, const LPBYTE pbyteData, DWORD dwSize)
{
  return RegSaveBinary (hKey, pszSubKey, pszValName, pbyteData, dwSize);
}

/* store a string */
inline bool cReg::SaveString (LPCTSTR pszValName, LPCTSTR pszString)
{
  return RegSaveString (hKey, NULL, pszValName, pszString);
}

/* store a string to subkey */
inline bool cReg::SaveString (LPCTSTR pszSubKey, LPCTSTR pszValName, LPCTSTR pszString)
{
  return RegSaveString (hKey, pszSubKey, pszValName, pszString);
}

/* store an array of strings */
inline bool cReg::SaveStringArr (LPCTSTR pszValName, const LPCTSTR pszStrings[], DWORD dwCount)
{
  return RegSaveStringArr (hKey, NULL, pszValName, pszStrings, dwCount);
}

/* store an array of strings to subkey */
inline bool cReg::SaveStringArr (LPCTSTR pszSubKey, LPCTSTR pszValName, const LPCTSTR pszStrings[], DWORD dwCount)
{
  return RegSaveStringArr (hKey, pszSubKey, pszValName, pszStrings, dwCount);
}

/* delete the given value or key in the registry with all of its subkeys */
inline bool cReg::DeleteKey (LPCTSTR pszValName)
{
  return RegDeleteKey (hKey, NULL, pszValName);
}

/* delete the given value or key in the registry with all of its subkeys in subkey */
inline bool cReg::DeleteKey (LPCTSTR pszSubKey, LPCTSTR pszValName)
{
  return RegDeleteKey (hKey, pszSubKey, pszValName);
}

/* delete all of subkeys in the key */
inline bool cReg::DeleteSubKeys ()
{
  return RegDeleteSubKeys (hKey);
}

/* check wether the given key has other subkeys and/or values */
inline bool cReg::HasEntries (DWORD *pdwSubKeyCount, DWORD *pdwValueCount)
{
  return RegHasEntries (hKey, NULL, pdwSubKeyCount, pdwValueCount);
}

/* check wether the given key has other subkeys and/or values in subkey */
inline bool cReg::HasEntries (LPCTSTR pszSubKey, DWORD *pdwSubKeyCount, DWORD *pdwValueCount)
{
  return RegHasEntries (hKey, pszSubKey, pdwSubKeyCount, pdwValueCount);
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
inline BOOL SetClassesRootRegKey(const ni::achar* lpszKey, const ni::achar* lpszValue, const ni::achar* lpszValueName = NULL)
{
  if (lpszValueName == NULL)
  {
    if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
                      lpszValue, lstrlen(lpszValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
    {
      return FALSE;
    }
    return TRUE;
  }
  else
  {
    HKEY hKey;

    if(::RegCreateKey(HKEY_CLASSES_ROOT, lpszKey, &hKey) == ERROR_SUCCESS)
    {
      LONG lResult = ::RegSetValueEx(hKey, lpszValueName, 0, REG_SZ,
                                     (CONST BYTE*)lpszValue, (lstrlen(lpszValue) + 1) * sizeof(TCHAR));

      if(::RegCloseKey(hKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS)
        return TRUE;
    }
    return FALSE;
  }
}

///////////////////////////////////////////////
inline const ni::achar* RegisterApp(const ni::achar* aszAppName, const ni::achar* aszExt, const ni::achar* aszDocDesc)
{
  ni::cString strCmdLine = niWin32API(GetCommandLine)();
  ni::tU32 lExeLen = 0;
  achar aszExe[1024] = {0};
  ni::StrGetCommandPath(aszExe,niCountOf(aszExe),strCmdLine.Chars(),&lExeLen);
  if (niStringIsOK(aszExe)) {
    ni::cString strBuffer;
    ni::cString strBuffer2;
    SetClassesRootRegKey(strBuffer2.Format(_A(".%s"), aszExt),  strBuffer.Format(_A("%s.Document"), aszAppName));
    SetClassesRootRegKey(strBuffer2.Format(_A(".%s\\ShellNew"), aszExt), _A(""), _A("NullFile"));

    if (aszDocDesc)
    {
      SetClassesRootRegKey(strBuffer2.Format(_A("%s.Document"), aszAppName),
                           aszDocDesc);
    }

    SetClassesRootRegKey(strBuffer2.Format(_A("%s.Document\\DefaultIcon"), aszAppName),
                         strBuffer. Format(_A("%s,0"), aszExe));

    SetClassesRootRegKey(strBuffer2.Format(_A("%s.Document\\shell\\open\\command"), aszAppName),
                         strBuffer. Format(_A("%s \"%%1\""), aszExe));
  }
  const ni::achar* pRet = strCmdLine.Chars()+lExeLen;
  while (*pRet && ni::StrIsSpace(*pRet)) { ++pRet; }
  return pRet;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Utility dialogs

// cAboutDlg dialog used for App About
class cAboutDlg : public WinUI::Dialog
{
 public:
  cAboutDlg(ULONG anAboutBoxID, BOOL abCenter = TRUE) : WinUI::Dialog(anAboutBoxID), mbCenter(abCenter) {}

  virtual int OnInitDialog(int nCtrlID, WPARAM nCtrlMsg, LPARAM lParam)
  {
    if (mbCenter)
      CenterOnScreen();
    return 0;
  }

  WINUI_BEGIN_DLG_MSG_MAP()
  WINUI_MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
  WINUI_BEGIN_COMMAND_HANDLER()
  WINUI_COMMAND_OKCANCEL_HANDLERS()
  WINUI_END_COMMAND_HANDLER()
  WINUI_END_DLG_MSG_MAP(Dialog)

  private:
  BOOL  mbCenter;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __WINUI_48978804_H__
