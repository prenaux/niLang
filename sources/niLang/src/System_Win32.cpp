#include "API/niLang/Types.h"

#ifdef niWindows
#include "Lang.h"

#include "API/niLang/IOSWindow.h"
#include <niLang/IFile.h>
#include <niLang/ILang.h>

#include "API/niLang/Platforms/Win32/Win32_Redef.h"
#include "API/niLang/Platforms/Win32/Win32_File.h"
#include "API/niLang/Platforms/Win32/Win32_UTF.h"
#include "API/niLang/Platforms/Win32/Win32_DC.h"
#include <commdlg.h>
#include <shellapi.h>

// #define USE_IDROPTARGET
#ifdef USE_IDROPTARGET
#include "API/niLang/Platforms/Win32/Win32_DropTarget.h"
#endif

#include <niLang/Utils/CollectionImpl.h>
#include "API/niLang/Utils/StringTokenizerImpl.h"

#include <niLang/Var.h>
#include <niLang/Utils/Sync.h>
#include <niLang/STL/utils.h>
#include <niLang/Utils/Path.h>

#include "API/niLang/ILang.h"

#ifdef niJNI
#include <niLang/Utils/JNIUtils.h>
#endif

#pragma comment(lib,"version.lib")
#pragma comment(lib,"rpcrt4.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"oleaut32.lib")

using namespace ni;

// #define niWindowsOSPlatformTrace_Enabled

#ifdef niWindowsOSPlatformTrace_Enabled
#define niWindowsOSPlatformTrace(X) niPrintln(X)
#else
#define niWindowsOSPlatformTrace(X)
#endif

static const tBool _kbUseThreadedWindow = eTrue;

#if defined niGCC || _MSC_VER <= 1310
#define NO_RAWINPUT
#endif

// #define NO_POINTEREVENT

#if !defined(NO_POINTEREVENT)

#if !defined(WM_POINTERUPDATE)
#define WM_POINTERUPDATE 0x0245
#define WM_POINTERDOWN   0x0246
#define WM_POINTERUP     0x0247

#define POINTER_MESSAGE_FLAG_PRIMARY      0x00002000
#define POINTER_MESSAGE_FLAG_FIRSTBUTTON  0x00000010
#define POINTER_MESSAGE_FLAG_SECONDBUTTON 0x00000020
#define POINTER_MESSAGE_FLAG_THIRDBUTTON  0x00000040
#define POINTER_MESSAGE_FLAG_FOURTHBUTTON 0x00000080
#define POINTER_MESSAGE_FLAG_FIFTHBUTTON  0x00000100

#define GET_POINTERID_WPARAM(wParam) (LOWORD (wParam))
#define IS_POINTER_FLAG_SET_WPARAM(wParam, flag) (((DWORD)HIWORD (wParam) &(flag)) == (flag))
#define IS_POINTER_PRIMARY_WPARAM(wParam) IS_POINTER_FLAG_SET_WPARAM (wParam, POINTER_MESSAGE_FLAG_PRIMARY)
#define IS_POINTER_FIRSTBUTTON_WPARAM(wParam) IS_POINTER_FLAG_SET_WPARAM (wParam, POINTER_MESSAGE_FLAG_FIRSTBUTTON)
#define IS_POINTER_SECONDBUTTON_WPARAM(wParam) IS_POINTER_FLAG_SET_WPARAM (wParam, POINTER_MESSAGE_FLAG_SECONDBUTTON)
#define IS_POINTER_THIRDBUTTON_WPARAM(wParam) IS_POINTER_FLAG_SET_WPARAM (wParam, POINTER_MESSAGE_FLAG_THIRDBUTTON)
#define IS_POINTER_FOURTHBUTTON_WPARAM(wParam) IS_POINTER_FLAG_SET_WPARAM (wParam, POINTER_MESSAGE_FLAG_FOURTHBUTTON)
#define IS_POINTER_FIFTHBUTTON_WPARAM(wParam) IS_POINTER_FLAG_SET_WPARAM (wParam, POINTER_MESSAGE_FLAG_FIFTHBUTTON)

typedef enum tagPOINTER_INPUT_TYPE {
  PT_POINTER  = 0x00000001,
  PT_TOUCH    = 0x00000002,
  PT_PEN      = 0x00000003,
  PT_MOUSE    = 0x00000004
} POINTER_INPUT_TYPE;
#endif  // WM_POINTERUP

#define TRACE_POINTER(X) // niDebugFmt(X)

typedef BOOL (WINAPI *tpfnGetPointerType)(UINT32 pointerId, POINTER_INPUT_TYPE *pointerType);
static tpfnGetPointerType _pfnGetPointerType = NULL;

// Toggle the press and hold gesture for the given window
static bool _InitializeFingerHandler(HWND hWnd, bool abEnablePressAndHold)
{
  if (!_pfnGetPointerType) {
    TRACE_POINTER(("... _GetPointerType"));
    HMODULE hDLL = ::LoadLibraryW(L"user32.dll");
    if (hDLL) {
      TRACE_POINTER(("... _GetPointerType: user32.dll"));
      _pfnGetPointerType = (tpfnGetPointerType)::GetProcAddress(hDLL, "GetPointerType");
      if (_pfnGetPointerType) {
        TRACE_POINTER(("... _GetPointerType: user32.dll: found GetPointerType"));
      }
      else {
        TRACE_POINTER(("... _GetPointerType: user32.dll: NOT FOUND GetPointerType"));
      }
      ::FreeLibrary(hDLL);
    }
  }

  {
    // The atom identifier and Tablet PC atom
    ATOM atomID = 0;
    LPCTSTR tabletAtom = "MicrosoftTabletPenServiceProperty";

    // Get the Tablet PC atom ID
    atomID = GlobalAddAtom(tabletAtom);

    // If getting the ID failed, return false
    if (atomID == 0) {
      return false;
    }

    // Enable or disable the press and hold gesture
    if (abEnablePressAndHold) {
      // Try to enable press and hold gesture by
      // clearing the window property, return the result
      return RemoveProp(hWnd, tabletAtom);
    }
    else {
      // Try to disable press and hold gesture by
      // setting the window property, return the result
      return SetProp(hWnd, tabletAtom, (HANDLE)1);
    }
  }
}

struct sFingerHandler {
  struct sFinger {
    tU32 mIndex;
    sVec2i mPrevPos;
    sFinger(tU32 anIndex) {
      mIndex = anIndex;
      mPrevPos = Vec2i(eInvalidHandle,eInvalidHandle);
    }
  };
  typedef astl::map<tU32,sFinger> tFingerMap;
  tFingerMap mMapFingers;

  sFingerHandler() {
  }

  void Clear() {
    mMapFingers.empty();
  }

  void FingerDown(tMessageHandlerSinkLst* apMsgHandlers, tU32 anId, const sVec2i& avPos) {
    {
      tFingerMap::const_iterator checkIt = mMapFingers.find(anId);
      if (checkIt != mMapFingers.end()) {
        niWarning(niFmt("FingerDown: Pointer with id '%s' already down.", anId));
        return;
      }
    }

    tFingerMap::iterator it = astl::upsert(mMapFingers, anId, sFinger((tU32)mMapFingers.size()));
    if (apMsgHandlers) {
      const sFinger& finger = it->second;
      ni::SendMessages(apMsgHandlers,
                       eOSWindowMessage_FingerDown,
                       finger.mIndex,
                       Vec3f((tF32)avPos.x,(tF32)avPos.y,1.0f));
    }

    _DoFingerMove(apMsgHandlers,it,avPos);
  }

  void FingerUp(tMessageHandlerSinkLst* apMsgHandlers, tU32 anId, const sVec2i& avPos) {
    tFingerMap::iterator it = mMapFingers.find(anId);
    if (it == mMapFingers.end()) {
      niWarning(niFmt("FingerUp: Pointer with id '%s' not down.", anId));
      return;
    }
    _DoFingerMove(apMsgHandlers,it,avPos);
    if (apMsgHandlers) {
      const sFinger& finger = it->second;
      ni::SendMessages(apMsgHandlers,
                       eOSWindowMessage_FingerUp,
                       finger.mIndex,
                       Vec3f((tF32)avPos.x,(tF32)avPos.y,1.0f));
    }
    mMapFingers.erase(it);
  }

  void FingerMove(tMessageHandlerSinkLst* apMsgHandlers, tU32 anId, const sVec2i& avPos) {
    tFingerMap::iterator it = mMapFingers.find(anId);
    if (it == mMapFingers.end()) {
      niWarning(niFmt("FingerUp: Pointer with id '%s' not down.", anId));
      return;
    }
    _DoFingerMove(apMsgHandlers,it,avPos);
  }

  void _DoFingerMove(tMessageHandlerSinkLst* apMsgHandlers, tFingerMap::iterator it, const sVec2i& avPos) {
    sFinger& finger = it->second;
    if (apMsgHandlers) {
      if (finger.mPrevPos != avPos) {
        ni::SendMessages(apMsgHandlers,
                         eOSWindowMessage_FingerMove,
                         finger.mIndex,
                         Vec3f((tF32)avPos.x,(tF32)avPos.y,1.0f));
        if (finger.mPrevPos.x != eInvalidHandle) {
          const sVec2i vRelMove = avPos-finger.mPrevPos;
          ni::SendMessages(apMsgHandlers,
                           eOSWindowMessage_FingerRelativeMove,
                           finger.mIndex,
                           Vec3f((tF32)vRelMove.x,(tF32)vRelMove.y,1.0f));
        }
      }
      finger.mPrevPos = avPos;
    }
  }
};
#endif

static const tBool _kbDisableScreenSaver = eFalse;

enum GAMECTRL {
  GAMECTRL_NOT_INITIALIZED,
  GAMECTRL_NONE,
  GAMECTRL_XINPUT,
  GAMECTRL_DINPUT
};

static tU32 _knGameCtrlType = GAMECTRL_NOT_INITIALIZED;

tBool __stdcall XInputGameCtrls_Startup();
void __stdcall XInputGameCtrls_Shutdown();
tU32 __stdcall XInputGameCtrls_GetNumGameCtrls();
iGameCtrl* __stdcall XInputGameCtrls_GetGameCtrl(tU32 anIndex);

tBool __stdcall DInputGameCtrls_Startup();
void __stdcall DInputGameCtrls_Shutdown();
tU32 __stdcall DInputGameCtrls_GetNumGameCtrls();
iGameCtrl* __stdcall DInputGameCtrls_GetGameCtrl(tU32 anIndex);

static void _InitGameCtrls() {
  if (_knGameCtrlType != GAMECTRL_NOT_INITIALIZED)
    return;

  if (XInputGameCtrls_Startup()) {
    _knGameCtrlType = GAMECTRL_XINPUT;
    return;
  }
  else if (DInputGameCtrls_Startup()) {
    _knGameCtrlType = GAMECTRL_DINPUT;
    return;
  }

  _knGameCtrlType = GAMECTRL_NONE;
}

//////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IDB_SPLASH
#define IDB_SPLASH                      105
#endif

//
// Mouse Wheel rotation stuff, only define if we are
// on a version of the OS that does not support
// WM_MOUSEWHEEL messages.
//
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL WM_MOUSELAST+1
// Message ID for IntelliMouse wheel
#endif

#ifndef MSH_MOUSEWHEEL
#define MSH_MOUSEWHEEL _A("MSWHEEL_ROLLMSG")
#endif

#define MY_WM_UNSTICK         WM_USER+2200
niCAssert(MY_WM_UNSTICK < 0x7FFF);
#define MY_WM_SETCAPTURE      WM_USER+2201
#define MY_WM_RELEASECAPTURE  WM_USER+2202
#define MY_WM_IME_SETSTATE          WM_USER+2203 // wParam: anState
#define MY_WM_IME_ENABLE            WM_USER+2204 // wParam: abEnabled
#define MY_WM_IME_FINALIZESTRING    WM_USER+2205 // wParam: FinalizeString(abSend)
#define MY_WM_IME_DISABLE           WM_USER+2206 // none
#define MY_WM_DPICHANGED       0x02E0

static DWORD uMSH_MOUSEWHEEL = 0;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#define TIMERID_REFRESH     5501
#define TIMERID_HEARTBEAT   5502
#define REPEATED_KEYMASK  (1<<30)
#define EXTENDED_KEYMASK  (1<<24)

#ifndef VK_0
#define VK_0  ((int)'0')
#define VK_1  ((int)'1')
#define VK_2  ((int)'2')
#define VK_3  ((int)'3')
#define VK_4  ((int)'4')
#define VK_5  ((int)'5')
#define VK_6  ((int)'6')
#define VK_7  ((int)'7')
#define VK_8  ((int)'8')
#define VK_9  ((int)'9')
#define VK_A  ((int)'A')
#define VK_B  ((int)'B')
#define VK_C  ((int)'C')
#define VK_D  ((int)'D')
#define VK_E  ((int)'E')
#define VK_F  ((int)'F')
#define VK_G  ((int)'G')
#define VK_H  ((int)'H')
#define VK_I  ((int)'I')
#define VK_J  ((int)'J')
#define VK_K  ((int)'K')
#define VK_L  ((int)'L')
#define VK_M  ((int)'M')
#define VK_N  ((int)'N')
#define VK_O  ((int)'O')
#define VK_P  ((int)'P')
#define VK_Q  ((int)'Q')
#define VK_R  ((int)'R')
#define VK_S  ((int)'S')
#define VK_T  ((int)'T')
#define VK_U  ((int)'U')
#define VK_V  ((int)'V')
#define VK_W  ((int)'W')
#define VK_X  ((int)'X')
#define VK_Y  ((int)'Y')
#define VK_Z  ((int)'Z')
#endif

// These keys haven't been defined, but were experimentally determined
#define VK_SEMICOLON  0xBA
#define VK_EQUALS 0xBB
#define VK_COMMA  0xBC
#define VK_MINUS  0xBD
#define VK_PERIOD 0xBE
#define VK_SLASH  0xBF
#define VK_GRAVE  0xC0
#define VK_LBRACKET 0xDB
#define VK_BACKSLASH  0xDC
#define VK_RBRACKET 0xDD
#define VK_APOSTROPHE 0xDE
#define VK_BACKTICK 0xDF
#define VK_NUMPAD_RETURN  0xF0

#define CURSOR_CAPTURE_GRAB niBit(0)
#define CURSOR_CAPTURE_L niBit(1)
#define CURSOR_CAPTURE_R niBit(2)
#define CURSOR_CAPTURE_M niBit(3)

struct sWindowsStaticManager {
  HCURSOR mcurArrow;
  HCURSOR mcurWait;
  HCURSOR mcurResizeVt;
  HCURSOR mcurResizeHz;
  HCURSOR mcurResizeDiag;
  HCURSOR mcurHand;
  HCURSOR mcurText;
  HCURSOR mcurHelp;
  HBRUSH  mbrushClear;
  HICON   miconLogo;
  tU32    mnIconLogoWidth;
  tU32    mnIconLogoHeight;
  tU8     mVKMap[0xFF];
  tU8     mbIsDown[eKey_Last];
  tU8     mbMouseDown[3];
  tU32    mnCharPreviousKey;

  sWindowsStaticManager() {
    mnCharPreviousKey = 0;

    mcurArrow = ::LoadCursor(NULL,IDC_ARROW);
    mcurWait = ::LoadCursor(NULL,IDC_WAIT);
    mcurHand = ::LoadCursor(NULL,IDC_HAND);
    mcurText = ::LoadCursor(NULL,IDC_IBEAM);
    mcurHelp = ::LoadCursor(NULL,IDC_HELP);
    mcurResizeVt = ::LoadCursor(NULL,IDC_SIZENS);
    mcurResizeHz = ::LoadCursor(NULL,IDC_SIZEWE);
    mcurResizeDiag = ::LoadCursor(NULL,IDC_SIZENESW);

    static const tU32 _knWindowClearColor = ULColorBuild(255,255,255,255);
    if (ULColorGetA(_knWindowClearColor) != 0) {
      mbrushClear = ::CreateSolidBrush(
          RGB(ULColorGetR(_knWindowClearColor),
              ULColorGetG(_knWindowClearColor),
              ULColorGetB(_knWindowClearColor)));
    }
    else {
      mbrushClear = NULL;
    }
    mnIconLogoWidth = 512;
    mnIconLogoHeight = 512;
    miconLogo = (HICON)::LoadImage(::GetModuleHandle(0),
                                   MAKEINTRESOURCE(101),
                                   IMAGE_ICON,
                                   mnIconLogoWidth,
                                   mnIconLogoHeight,
                                   LR_LOADTRANSPARENT);

    memset(mbMouseDown,0,sizeof(mbMouseDown));

    memset(mbIsDown,0,sizeof(mbIsDown));
    for(tU32 i = 0; i < 0xFF; ++i)
      mVKMap[i] = eKey_Unknown;

    mVKMap[VK_BACK] = eKey_BackSpace;
    mVKMap[VK_TAB] = eKey_Tab;
    mVKMap[VK_RETURN] = eKey_Enter;
    mVKMap[VK_NUMPAD_RETURN] = eKey_NumPadEnter;
    mVKMap[VK_PAUSE] = eKey_Pause;
    mVKMap[VK_ESCAPE] = eKey_Escape;
    mVKMap[VK_SPACE] = eKey_Space;
    mVKMap[VK_APOSTROPHE] = eKey_Apostrophe;
    mVKMap[VK_COMMA] = eKey_Comma;
    mVKMap[VK_MINUS] = eKey_Minus;
    mVKMap[VK_PERIOD] = eKey_Period;
    mVKMap[VK_SLASH] = eKey_Slash;
    mVKMap[VK_0] = eKey_n0;
    mVKMap[VK_1] = eKey_n1;
    mVKMap[VK_2] = eKey_n2;
    mVKMap[VK_3] = eKey_n3;
    mVKMap[VK_4] = eKey_n4;
    mVKMap[VK_5] = eKey_n5;
    mVKMap[VK_6] = eKey_n6;
    mVKMap[VK_7] = eKey_n7;
    mVKMap[VK_8] = eKey_n8;
    mVKMap[VK_9] = eKey_n9;
    mVKMap[VK_SEMICOLON] = eKey_Semicolon;
    mVKMap[VK_EQUALS] = eKey_Equals;
    mVKMap[VK_LBRACKET] = eKey_LBracket;
    mVKMap[VK_BACKSLASH] = eKey_BackSlash;
    mVKMap[VK_RBRACKET] = eKey_RBracket;
    mVKMap[VK_GRAVE] = eKey_Grave;
    mVKMap[VK_A] = eKey_A;
    mVKMap[VK_B] = eKey_B;
    mVKMap[VK_C] = eKey_C;
    mVKMap[VK_D] = eKey_D;
    mVKMap[VK_E] = eKey_E;
    mVKMap[VK_F] = eKey_F;
    mVKMap[VK_G] = eKey_G;
    mVKMap[VK_H] = eKey_H;
    mVKMap[VK_I] = eKey_I;
    mVKMap[VK_J] = eKey_J;
    mVKMap[VK_K] = eKey_K;
    mVKMap[VK_L] = eKey_L;
    mVKMap[VK_M] = eKey_M;
    mVKMap[VK_N] = eKey_N;
    mVKMap[VK_O] = eKey_O;
    mVKMap[VK_P] = eKey_P;
    mVKMap[VK_Q] = eKey_Q;
    mVKMap[VK_R] = eKey_R;
    mVKMap[VK_S] = eKey_S;
    mVKMap[VK_T] = eKey_T;
    mVKMap[VK_U] = eKey_U;
    mVKMap[VK_V] = eKey_V;
    mVKMap[VK_W] = eKey_W;
    mVKMap[VK_X] = eKey_X;
    mVKMap[VK_Y] = eKey_Y;
    mVKMap[VK_Z] = eKey_Z;
    mVKMap[VK_DELETE] = eKey_Delete;

    mVKMap[VK_NUMPAD0] = eKey_NumPad0;
    mVKMap[VK_NUMPAD1] = eKey_NumPad1;
    mVKMap[VK_NUMPAD2] = eKey_NumPad2;
    mVKMap[VK_NUMPAD3] = eKey_NumPad3;
    mVKMap[VK_NUMPAD4] = eKey_NumPad4;
    mVKMap[VK_NUMPAD5] = eKey_NumPad5;
    mVKMap[VK_NUMPAD6] = eKey_NumPad6;
    mVKMap[VK_NUMPAD7] = eKey_NumPad7;
    mVKMap[VK_NUMPAD8] = eKey_NumPad8;
    mVKMap[VK_NUMPAD9] = eKey_NumPad9;
    mVKMap[VK_DECIMAL] = eKey_NumPadPeriod;
    mVKMap[VK_DIVIDE] = eKey_NumPadSlash;
    mVKMap[VK_MULTIPLY] = eKey_NumPadStar;
    mVKMap[VK_SUBTRACT] = eKey_NumPadMinus;
    mVKMap[VK_ADD] = eKey_NumPadPlus;

    mVKMap[VK_UP] = eKey_Up;
    mVKMap[VK_DOWN] = eKey_Down;
    mVKMap[VK_RIGHT] = eKey_Right;
    mVKMap[VK_LEFT] = eKey_Left;
    mVKMap[VK_INSERT] = eKey_Insert;
    mVKMap[VK_HOME] = eKey_Home;
    mVKMap[VK_END] = eKey_End;
    mVKMap[VK_PRIOR] = eKey_PgUp;
    mVKMap[VK_NEXT] = eKey_PgDn;

    mVKMap[VK_F1] = eKey_F1;
    mVKMap[VK_F2] = eKey_F2;
    mVKMap[VK_F3] = eKey_F3;
    mVKMap[VK_F4] = eKey_F4;
    mVKMap[VK_F5] = eKey_F5;
    mVKMap[VK_F6] = eKey_F6;
    mVKMap[VK_F7] = eKey_F7;
    mVKMap[VK_F8] = eKey_F8;
    mVKMap[VK_F9] = eKey_F9;
    mVKMap[VK_F10] = eKey_F10;
    mVKMap[VK_F11] = eKey_F11;
    mVKMap[VK_F12] = eKey_F12;
    mVKMap[VK_F13] = eKey_F13;
    mVKMap[VK_F14] = eKey_F14;
    mVKMap[VK_F15] = eKey_F15;

    mVKMap[VK_NUMLOCK] = eKey_NumLock;
    mVKMap[VK_CAPITAL] = eKey_CapsLock;
    mVKMap[VK_SCROLL] = eKey_Scroll;
    mVKMap[VK_RSHIFT] = eKey_RShift;
    mVKMap[VK_LSHIFT] = eKey_LShift;
    mVKMap[VK_RCONTROL] = eKey_RControl;
    mVKMap[VK_LCONTROL] = eKey_LControl;
    mVKMap[VK_RMENU] = eKey_RAlt;
    mVKMap[VK_LMENU] = eKey_LAlt;
    mVKMap[VK_RWIN] = eKey_RWin;
    mVKMap[VK_LWIN] = eKey_LWin;

#ifdef VK_PRINT
    mVKMap[VK_PRINT] = eKey_PrintScreen;
#endif
    mVKMap[VK_SNAPSHOT] = eKey_PrintScreen;
    mVKMap[VK_CANCEL] = eKey_Pause;
    mVKMap[VK_APPS] = eKey_Apps;
  }

  void Activate(tU32& anKeyMod) {
    niFlagOnIf(anKeyMod,eKeyMod_Control,
               (GetKeyState(VK_LCONTROL) & 0x8000) ||
               (GetKeyState(VK_RCONTROL) & 0x8000));
    niFlagOnIf(anKeyMod,eKeyMod_Alt,
               (GetKeyState(VK_LMENU) & 0x8000) ||
               (GetKeyState(VK_RMENU) & 0x8000));
    niFlagOnIf(anKeyMod,eKeyMod_Shift,
               (GetKeyState(VK_SHIFT) & 0x8000));
    niFlagOnIf(anKeyMod,eKeyMod_NumLock,
               (GetKeyState(VK_NUMLOCK) & 0x8000));
    niFlagOnIf(anKeyMod,eKeyMod_CapsLock,
               (GetKeyState(VK_CAPITAL) & 0x8000));
    niFlagOnIf(anKeyMod,eKeyMod_ScrollLock,
               (GetKeyState(VK_SCROLL) & 0x8000));
  }

  __forceinline tBool AcceptKey(ni::tIntPtr wParam, ni::tIntPtr lParam) {
    return eTrue;
    //         (wParam == VK_DELETE ||
    //          wParam == VK_RETURN ||
    //          wParam == VK_TAB ||
    //          wParam == VK_BACK)
    //             || (!(HIWORD(lParam)&KF_REPEAT));
  }

  ni::eKey TranslateKey(ni::tU32 wParam, ni::tU32 lParam, ni::tBool abDown)
  {
    if (wParam >= 0xFF)
      return ni::eKey_Unknown;

    switch (wParam)
    {
      case VK_RETURN:
        if (lParam&EXTENDED_KEYMASK)
          wParam = VK_NUMPAD_RETURN;
        break;
      case VK_CONTROL:
        if (lParam&EXTENDED_KEYMASK)
          wParam = VK_RCONTROL;
        else
          wParam = VK_LCONTROL;
        break;
      case VK_SHIFT:
        wParam = VK_LSHIFT;
#if 0
        // Disabled this... there's basically no sane robust way to differentiate the Left
        // and Right Shift in Windows, this solution breaks when You Alt-Shift-Tab back for
        // example...
        if (abDown) {
          if (!mPrevShiftStates[0] && (GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
            mPrevShiftStates[0] = eTrue;
          }
          else if (!mPrevShiftStates[1] && (GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
            wParam = VK_RSHIFT;
            mPrevShiftStates[1] = eTrue;
          } else {
            /* Huh? */
          }
        }
        else {
          if (mPrevShiftStates[0] && !(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
            wParam = VK_LSHIFT;
            mPrevShiftStates[0] = eFalse;
          }
          else if (mPrevShiftStates[1] && !(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
            wParam = VK_RSHIFT;
            mPrevShiftStates[1] = eFalse;
          }
          else {
            /* Huh? */
          }
        }
#endif
        break;
      case VK_MENU:
        if (lParam&EXTENDED_KEYMASK)
          wParam = VK_RMENU;
        else
          wParam = VK_LMENU;
        break;
    }

    return (ni::eKey)mVKMap[wParam];
  }
};

// The name of the custom window message that the browser uses to tell the
// plugin process to paint a window.
#define NI_PaintMessageName L"NI_CustomPaint"

#if 0
static BOOL CALLBACK EnumChildProcRedrawAsync(HWND hwnd, LPARAM lparam) {
  // SendMessage gets the message across much quicker than PostMessage, since it
  // doesn't get queued.  When the plugin thread calls PeekMessage or other
  // Win32 APIs, sent messages are dispatched automatically.
  static UINT msg = RegisterWindowMessage(NI_PaintMessageName);
  SendNotifyMessage(hwnd, msg, 0, 0);
  return TRUE;
}

static void RedrawWindowChildrenAsync(HWND hWnd) {
  EnumChildWindows(hWnd, EnumChildProcRedrawAsync, 0);
}

static BOOL CALLBACK EnumChildProcRedrawSync(HWND hwnd, LPARAM lparam) {
  ::RedrawWindow(hwnd, NULL, NULL,
                 RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
  return TRUE;
}

static void RedrawWindowChildrenSync(HWND hWnd) {
  EnumChildWindows(hWnd, EnumChildProcRedrawSync, 0);
}
#endif

static const
WCHAR* _wszDefaultWindowClass = L"TSWindowClass";
static LRESULT CALLBACK _StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#pragma niNote("For IME to work properly the _StaticWndProc has to be wired when we register the window class, not later... otherwise it doesn't initialize properly")

static tIntPtr      _nLastErr = 0;
static cString      _strLastErr;

static tIntPtr __stdcall _GetLastErrorCode() {
  tIntPtr currErr = ::GetLastError();
  if (currErr != _nLastErr) {
    _nLastErr = currErr;
    _strLastErr.Clear();
  }
  return _nLastErr;
}
static const achar* _GetLastErrorMessage() {
  tIntPtr currErr = ::GetLastError();
  if (currErr != _nLastErr || _strLastErr.IsEmpty()) {
    if (currErr == S_OK) {
      _nLastErr = currErr;
      _strLastErr = _A("OK");
    }
    else {
      LPVOID lpMsgBuf = NULL;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, (DWORD)currErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
      _strLastErr = (LPCTSTR)lpMsgBuf;
      LocalFree(lpMsgBuf);
    }
  }
  return _strLastErr.Chars();
}

#if defined _DEBUG
#define TRACE_SET_DPI_AWARE(X) niDebugFmt(X)
#else
#define TRACE_SET_DPI_AWARE(X)
#endif

#define MY_WINAPI_FUNCPTR(RETTYPE, NAME, PARAMS) \
  typedef RETTYPE (WINAPI *tpfn##NAME) PARAMS; \
  static tpfn##NAME _pfn##NAME = NULL;

#define MY_WINAPI_INIT(HDLL, NAME) \
  _pfn##NAME = (tpfn##NAME)::GetProcAddress(HDLL, #NAME); \
  TRACE_SET_DPI_AWARE(("... WINAPI_INIT Loaded " #NAME ": %p", (tIntPtr)_pfn##NAME));

typedef enum MY_PROCESS_DPI_AWARENESS {
  MY_PROCESS_DPI_UNAWARE = 0,
  MY_PROCESS_SYSTEM_DPI_AWARE = 1,
  MY_PROCESS_PER_MONITOR_DPI_AWARE = 2
} MY_PROCESS_DPI_AWARENESS;

typedef enum MY_DPI_AWARENESS
{
  MY_DPI_AWARENESS_INVALID = -1,
  MY_DPI_AWARENESS_UNAWARE = 0,
  MY_DPI_AWARENESS_SYSTEM_AWARE,
  MY_DPI_AWARENESS_PER_MONITOR_AWARE
} MY_DPI_AWARENESS;

#define DPI_AWARENESS_CONTEXT_UNAWARE              ((HANDLE)-1)
#define DPI_AWARENESS_CONTEXT_SYSTEM_AWARE         ((HANDLE)-2)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE    ((HANDLE)-3)
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE)-4)
#define DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED    ((HANDLE)-5)

static const char* _GetDpiAwarenessContextString(HANDLE h) {
  if (h == DPI_AWARENESS_CONTEXT_UNAWARE) { return "DPI_AWARENESS_CONTEXT_UNAWARE"; }
  else if (h == DPI_AWARENESS_CONTEXT_SYSTEM_AWARE) { return "DPI_AWARENESS_CONTEXT_SYSTEM_AWARE"; }
  else if (h == DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE) {return "DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE"; }
  else if (h == DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) {return "DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2"; }
  else if (h == DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED) {return "DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED"; }
  else if (h == (HANDLE)eInvalidHandle) { return "DPI_AWARENESS_CONTEXT_INVALIDHANDLE"; }
  return "DPI_AWARENESS_CONTEXT_UNKNOWN";
}

// shcore.dll
MY_WINAPI_FUNCPTR(HRESULT, SetProcessDpiAwareness, (MY_PROCESS_DPI_AWARENESS value));
// shcore.dll
MY_WINAPI_FUNCPTR(HRESULT, GetScaleFactorForMonitor, (HMONITOR hMon, DWORD *pScale)); // Win8 only...
// user32.dll
MY_WINAPI_FUNCPTR(HRESULT, SetProcessDPIAware, (VOID));
// user32.dll
MY_WINAPI_FUNCPTR(HANDLE, GetThreadDpiAwarenessContext, ());
// user32.dll
MY_WINAPI_FUNCPTR(MY_DPI_AWARENESS, GetAwarenessFromDpiAwarenessContext, (HANDLE value));
// user32.dll
MY_WINAPI_FUNCPTR(UINT, GetDpiForSystem, ());
// user32.dll
MY_WINAPI_FUNCPTR(UINT, GetDpiForWindow, (HWND hWnd));
// user32.dll
MY_WINAPI_FUNCPTR(HANDLE, SetThreadDpiAwarenessContext, (HANDLE dpiContext));
// user32.dll
MY_WINAPI_FUNCPTR(BOOL, SetProcessDpiAwarenessContext, (HANDLE dpiContext));
// user32.dll
MY_WINAPI_FUNCPTR(BOOL, EnableNonClientDpiScaling, (HWND hwnd));

static HMODULE _hDLLShcore = NULL;
static HMODULE _hDLLUser32 = NULL;

static void _SetThreadDpiAwareness() {
  HANDLE awareness = (HANDLE)eInvalidHandle;
  if (_pfnSetThreadDpiAwarenessContext) {
    awareness = _pfnSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  }
  TRACE_SET_DPI_AWARE(("... _SetThreadDpiAwareness: threadId: %p, awareness: %s, error: %s",
                       (tU32)::GetCurrentThreadId(),
                       _GetDpiAwarenessContextString(awareness),
                       _GetLastErrorMessage()));
}

static tBool _SetDPIAware() {
  static tBool _bDidSet = eFalse;
  if (_bDidSet) {
    return eTrue;
  }
  _GetLastErrorCode();

  HMODULE _hDLLShcore = ::LoadLibraryW(L"shcore.dll");
  TRACE_SET_DPI_AWARE(("... _SetDPIAware: hDLLShcore: %p", (tIntPtr)_hDLLShcore));

  HMODULE _hDLLUser32 = ::LoadLibraryW(L"user32.dll");
  TRACE_SET_DPI_AWARE(("... _SetDPIAware: hDLLUser32: %p", (tIntPtr)_hDLLUser32));

  if (_hDLLShcore) {
    MY_WINAPI_INIT(_hDLLShcore, SetProcessDpiAwareness);
    MY_WINAPI_INIT(_hDLLShcore, GetScaleFactorForMonitor);
  }

  if (_hDLLUser32) {
    MY_WINAPI_INIT(_hDLLUser32, GetAwarenessFromDpiAwarenessContext);
    MY_WINAPI_INIT(_hDLLUser32, GetThreadDpiAwarenessContext);
    MY_WINAPI_INIT(_hDLLUser32, GetDpiForSystem);
    MY_WINAPI_INIT(_hDLLUser32, GetDpiForWindow);
    MY_WINAPI_INIT(_hDLLUser32, SetThreadDpiAwarenessContext);
    MY_WINAPI_INIT(_hDLLUser32, SetProcessDpiAwarenessContext);
    MY_WINAPI_INIT(_hDLLUser32, EnableNonClientDpiScaling);
  }

  if (_pfnSetProcessDpiAwarenessContext && _pfnSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
    TRACE_SET_DPI_AWARE(("... _SetDPIAware: SetProcessDpiAwarenessContext: succeeded"));
  }
  else {
    TRACE_SET_DPI_AWARE(("... _SetDPIAware: SetProcessDpiAwarenessContext: failed: %s", _GetLastErrorMessage()));
  }

  if (_pfnSetProcessDpiAwareness && (_pfnSetProcessDpiAwareness(MY_PROCESS_PER_MONITOR_DPI_AWARE) == S_OK)) {
    TRACE_SET_DPI_AWARE(("... _SetDPIAware: SetProcessDpiAwareness: succeeded"));
  }
  else {
    TRACE_SET_DPI_AWARE(("... _SetDPIAware: SetProcessDpiAwareness: failed: %s", _GetLastErrorMessage()));
  }

  if (_pfnSetProcessDPIAware && _pfnSetProcessDPIAware()) {
    TRACE_SET_DPI_AWARE(("... _SetDPIAware: SetProcessDPIAware: succeeded"));
  }
  else {
    TRACE_SET_DPI_AWARE(("... _SetDPIAware: SetProcessDPIAware: failed: %s", _GetLastErrorMessage()));
  }

  _SetThreadDpiAwareness();

  return eTrue;
}

static UINT _GetDpiForWindow(HWND hWnd) {
  UINT uDpi = 96;
  if (_pfnGetAwarenessFromDpiAwarenessContext &&
      _pfnGetThreadDpiAwarenessContext &&
      _pfnGetDpiForSystem &&
      _pfnGetDpiForWindow)
  {
      // Determine the DPI to use, according to the DPI awareness mode
      MY_DPI_AWARENESS dpiAwareness = _pfnGetAwarenessFromDpiAwarenessContext(_pfnGetThreadDpiAwarenessContext());
      switch (dpiAwareness)
      {
        // Scale the window to the system DPI
      case MY_DPI_AWARENESS_SYSTEM_AWARE:
        uDpi = _pfnGetDpiForSystem();
        break;
        // Scale the window to the monitor DPI
      case MY_DPI_AWARENESS_PER_MONITOR_AWARE:
        uDpi = _pfnGetDpiForWindow(hWnd);
        break;
      }

    }
  return uDpi;
}

static float _GetContentsScaleFromDpi(UINT dpi) {
  if (dpi > 96) {
    return (float)dpi / 96.0f;
  }
  return 1.0f;
}

static tBool _RegisterDefaultWindowClass(HINSTANCE hInst) {
  static tBool _bRegistered = eFalse;
  if (_bRegistered) return eTrue;

  WORD iconID = 0;
  char exePath[AMAX_PATH];
  ni::Windows::UTF16Buffer wExePath;
  niWin32_UTF8ToUTF16(wExePath,ni_get_exe_path(exePath));
  HANDLE hAppIcon = ::ExtractAssociatedIconW(hInst,wExePath.data(),&iconID);

  WNDCLASSEXW wcex;
  memset(&wcex,0,sizeof(wcex));
  wcex.cbSize = sizeof(WNDCLASSEXW);
  wcex.style      = /*CS_HREDRAW | CS_VREDRAW | CS_OWNDC |*/ CS_DBLCLKS;
  wcex.lpfnWndProc  = _StaticWndProc;
  wcex.cbClsExtra   = 0;
  wcex.cbWndExtra   = 0;
  wcex.hInstance    = hInst;
  wcex.hIcon      = (HICON)hAppIcon;
  wcex.hCursor    = NULL;
  wcex.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wcex.lpszMenuName = NULL;
  wcex.lpszClassName  = _wszDefaultWindowClass;
  wcex.hIconSm    = NULL;
  if (!RegisterClassExW(&wcex)) {
    return eFalse;
  }

  _bRegistered = eTrue;
  return eTrue;
}
static inline void _GetWindowStyle(tOSWindowStyleFlags aStyle, LONG_PTR& style, LONG_PTR& exstyle) {
  exstyle &= ~WS_EX_TOOLWINDOW;
  if (niFlagIs(aStyle,eOSWindowStyleFlags_ClientArea)) {
    style = WS_CHILD|WS_VISIBLE;
    style &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION | WS_SIZEBOX);
  }
  else {
    style = WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    if (niFlagIs(aStyle,eOSWindowStyleFlags_Overlay)) {
      style |= WS_POPUP;
    }
    else if (niFlagIs(aStyle,eOSWindowStyleFlags_Toolbox)) {
      style |= WS_POPUPWINDOW|WS_CAPTION;
      style &= ~WS_SYSMENU;
      exstyle |= WS_EX_TOOLWINDOW;
    }
    else {
      style |= WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
    }
    if (niFlagIs(aStyle,eOSWindowStyleFlags_NoTitle)) {
      style &= ~WS_CAPTION;
    }
    if (niFlagIsNot(aStyle,eOSWindowStyleFlags_FixedSize)) {
      style |= WS_SIZEBOX;
    }
    else {
      style &= ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
    }
  }
}

///////////////////////////////////////////////
static HINSTANCE _GetHInstance() {
  static HINSTANCE _hInstance = NULL;
	if (!_hInstance) {
		char exePath[AMAX_PATH];
		_hInstance = ni::Windows::utf8_GetModuleHandle(ni_get_exe_path(exePath));
	}
  return _hInstance;
}

///////////////////////////////////////////////
static HWND _CreateWindow(HWND ahWndParent, tOSWindowStyleFlags aStyle) {
	HINSTANCE hInst = _GetHInstance();
  _RegisterDefaultWindowClass(hInst);

  LONG_PTR style = 0, exstyle = 0;
  _GetWindowStyle(aStyle,style,exstyle);

  HWND hWnd = niWin32API(CreateWindowEx)(
      exstyle,
      _wszDefaultWindowClass,
      L"Default",
      style,
      -1000,-1000,0,0,
      ahWndParent, NULL, hInst, NULL);
  if (!hWnd) {
    niError(_A("Can't create application's windows."));
    return NULL;
  }

  (_pfnEnableNonClientDpiScaling && _pfnEnableNonClientDpiScaling(hWnd));

#if !defined NO_POINTEREVENT
  // Disable the "hold" and press for the right click emulation if we handle pointer events
  _InitializeFingerHandler(hWnd,false);
#endif

  return hWnd;
}

class cOSWindowWindows;
static void _RegisterWindow(cOSWindowWindows* apWin);
static void _UnregisterWindow(cOSWindowWindows* apWin);
static void _UnstickWindows();

//--------------------------------------------------------------------------------------------
//
//  Window implementation
//
//--------------------------------------------------------------------------------------------
class cThreadedWindow : public ni::cIUnknownImpl<ni::iUnknown>
{
 public:
  cThreadedWindow(cOSWindowWindows* apOwner, HWND ahWndParent, tOSWindowStyleFlags aStyle) {
    mpOwner = apOwner;
    mhWndParent = ahWndParent;
    mhWnd = NULL;
    mStyle = aStyle;
    mbRequestedClose = eFalse;
    mThread = ni_create_thread();
    mThread->Start(_ThreadProc,(void*)this);
    meventSetup.Wait();
#ifdef _DEBUG
    niDebugFmt(("D/cThreadedWindow using thread '%d'",mThread->GetThreadID()));
#endif
  }
  ~cThreadedWindow() {
    Invalidate();
  }
  void __stdcall Invalidate() {
  }

  void __stdcall SignalMessage() {
    meventHandledMsg.Signal();
  }
  void __stdcall WaitForMessage() {
    // We put a timeout of 1sec to ensure that the application never
    // deadlock no matter what is its state.
    meventHandledMsg.WaitEx(1000);
  }
  void __stdcall SetRequestedClose(tBool abRequestedClose) {
    mbRequestedClose = abRequestedClose;
  }
  tBool __stdcall GetRequestedClose() const {
    return mbRequestedClose;
  }

  HWND __stdcall GetHWND() const {
    return mhWnd;
  }

  virtual tU32  __stdcall Run() {
    mhWnd = _CreateWindow(mhWndParent,mStyle);
    if (!mhWnd) {
      return eInvalidHandle;
    }
    _SetThreadDpiAwareness();

    meventSetup.Signal();

    MSG msg;
    while (1) {
      tBool doBreak = mbRequestedClose;
      tBool bRet = niWin32API(GetMessage)(&msg, NULL, 0, 0);
      if (bRet == -1) {
        doBreak = eTrue;
      }
      else if (bRet) {
        SignalMessage();
        switch (msg.message) {
          case WM_DESTROY:
          doBreak = eTrue;
          break;
        }
        {
          ::TranslateMessage(&msg);
          niWin32API(DispatchMessage)(&msg);
        }
      }
      if (doBreak)
        break;
    }

    BOOL r = ::DestroyWindow(mhWnd);
    return r;
  }

  virtual void __stdcall EndThread() {
    // Post a message so that GetMessage returns if its waiting.
    niWin32API(PostMessage)(mhWnd,WM_DESTROY,0,0);
    mThread->Join(eInvalidHandle);
  }

  static tIntPtr _ThreadProc(void* apData) {
    return ((cThreadedWindow*)apData)->Run();
  }

 public:
  cOSWindowWindows* mpOwner;
  HWND mhWnd;
  HWND mhWndParent;
  tOSWindowStyleFlags mStyle;
  ni::ThreadEvent meventSetup;
  ni::ThreadEvent meventHandledMsg;
  tBool mbRequestedClose;
  Ptr<iThread> mThread;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cOSWindowWindows declaration.

//! Description
class cOSWindowWindows : public ni::cIUnknownImpl<ni::iOSWindow,
                                                  ni::eIUnknownImplFlags_Default,
                                                  ni::iOSWindowWindows>
{
  niBeginClass(cOSWindowWindows);

 public:
  ///////////////////////////////////////////////
  cOSWindowWindows(iOSWindow* apParent)
      : mptrParentWindow(apParent),
        mlstSinks(tOSWindowWindowsSinkList::Create())
  {
    mHandle = NULL;
    mhClientAreaWnd = NULL;
    mbOwnedHandle = eFalse;
    mbDestroyed = eFalse;
    mbIsActive = eFalse;
    mbIsResizing = eFalse;
    mbRequestedClose = eFalse;
    mbDropTarget = eFalse;
    mbBackgroundUpdate = eFalse;
#ifdef USE_IDROPTARGET
    mpDropTarget = NULL;
#endif
    mPrevWindowProc = NULL;
    mfRefreshTimer = -1;
    mvPrevMousePos = ni::Vec2<tI32>(eInvalidHandle,eInvalidHandle);
    mvRawInputAbsRelPos = ni::Vec2<tI32>(eInvalidHandle,eInvalidHandle);
    mbMouseOverClient = eFalse;
    mbHasFocus = eFalse;
    mhCursor = NULL;
    mhCustomCursor = NULL;
    mnCustomCursorID = 0;
    mCursorCapture = 0;
    mrectCursorClip = sRecti::Null();
    mptrMT = tMessageHandlerSinkLst::Create();
    mfContentsScale = 1.0f;
    SetCursor(eOSCursor_Arrow);
    _RegisterWindow(this);
  }

  ///////////////////////////////////////////////
  ~cOSWindowWindows() {
    Invalidate();
    _UnregisterWindow(this);
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (mHandle) {
      mCursorCapture = 0;
      // _UpdateCursorCapture(); // commented out cause it causes Invalidate to deadlock in some circumstances and its most likely unnecessary...
      mbRequestedClose = eTrue;
      niWin32API(SetWindowLongPtr)(mHandle,GWLP_USERDATA,NULL);
      // Dont clear the window proc, otherwise the app will crash, _StaticWndProc takes care of the NULL UserData
      //niWin32API(SetWindowLongPtr)(mHandle,GWLP_WNDPROC,NULL);
      HWND toDestroy = mHandle;
      mHandle = NULL;
      if (mbOwnedHandle) {
        if (mptrWindowThread.IsOK()) {
          mptrWindowThread->EndThread();
          mptrWindowThread = NULL;
        }
        else {
          // Post a message so that GetMessage returns if its
          // in use.
          niWin32API(PostMessage)(toDestroy,WM_DESTROY,0,0);
          if (!mbDestroyed) {
            // This is necessary otherwise a GetMessage could block
            // indefinitly.
            ::DestroyWindow(toDestroy);
            mbDestroyed = eTrue;
          }
        }
        mbOwnedHandle = eFalse;
      }
      mbIsActive = eFalse;
      if (mptrMT.IsOK()) {
        mptrMT->Invalidate();
        mptrMT = NULL;
      }

      if (mhCustomCursor) {
        ::DestroyCursor(mhCustomCursor);
        mnCustomCursorID = 0;
      }

      mhClientAreaWnd = NULL;
      mptrParentWindow = NULL;
#ifdef USE_IDROPTARGET
      niSafeRelease(mpDropTarget);
#endif

      _UnstickWindows();
    }
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cOSWindowWindows);
    return mHandle != NULL && ::IsWindow(mHandle);
  }

  ///////////////////////////////////////////////
  tOSWindowWindowsSinkList* __stdcall GetWindowsWindowSinkList() const {
    return mlstSinks.ptr();
  }

  ///////////////////////////////////////////////
  iOSWindow* __stdcall GetParent() const {
    return mptrParentWindow;
  }
  tIntPtr __stdcall GetParentHandle() const {
    return (tIntPtr)::GetParent(mHandle);
  }
  tIntPtr __stdcall GetPID() const {
    DWORD pid = 0;
    if (mHandle) {
      ::GetWindowThreadProcessId(mHandle,&pid);
    }
    return (tIntPtr)pid;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall IsParentWindow(tIntPtr aHandle) const {
    tBool isParent = eFalse;
    tU32 c = 0;
    HWND hParent = mHandle;
    while (1) {
      ++c;
      hParent = ::GetParent(hParent);
      if (!hParent)
        break;
      if (hParent == (HWND)aHandle) {
        isParent = eTrue;
        break;
      }
    }
    return isParent ? c : 0;
  }


  ///////////////////////////////////////////////
  void __stdcall SetClientAreaWindow(tIntPtr aHandle) {
    mhClientAreaWnd = aHandle;
    if (::IsWindow((HWND)mhClientAreaWnd)) {
      if (::GetWindowLong((HWND)mhClientAreaWnd,GWL_STYLE) & WS_CHILD) {
        sRecti rect;
        ::GetClientRect(mHandle,(LPRECT)&rect);
        rect.MoveTo(sVec2i::Zero());
        ::SetWindowPos((HWND)mhClientAreaWnd,NULL,
                       rect.GetLeft(),rect.GetTop(),
                       rect.GetWidth(),rect.GetHeight(),
                       SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
      }
    }
    else {
      mhClientAreaWnd = NULL;
    }
  }
  tIntPtr __stdcall GetClientAreaWindow() const {
    return mhClientAreaWnd;
  }

  ///////////////////////////////////////////////
  void _UpdateContentsScale(const char* aReason, UINT dpi) {
    const tF32 wasContentsScale = mfContentsScale;
    mfContentsScale = _GetContentsScaleFromDpi(dpi ? dpi : _GetDpiForWindow(mHandle));
    TRACE_SET_DPI_AWARE(("... _UpdateContentsScale (%s): %p, contentsScale: %g, wasContentsScale: %g",
                         aReason,
                         (tIntPtr)mHandle,
                         mfContentsScale,
                         wasContentsScale));
    if (mfContentsScale != wasContentsScale) {
      // TODO: Send window message
    }
  }

  ///////////////////////////////////////////////
  tBool _SetWindowHandle(HWND aHWND, tBool abOwnHandle, tBool abSetWindowProc, cThreadedWindow* apWindowThread) {
    if (!::IsWindow(aHWND))
      return eFalse;

    mHandle = aHWND;
    if (!mHandle)
      return eFalse;
    _UpdateContentsScale("SetWindowHandle", 0);

    mptrWindowThread = apWindowThread;
    if (abSetWindowProc) {
#ifdef niMSVC
#pragma warning (disable : 4244)  //  conversion from 'LONG_PTR' to 'LONG', possible loss of data --- SetWindowLongPtr isnt declared to work warning-free with the Win64 checks
#endif
      niWin32API(SetWindowLongPtr)(mHandle,GWLP_USERDATA,(LONG_PTR)(this));
      ((LONG_PTR&)mPrevWindowProc) = niWin32API(GetWindowLongPtr)(mHandle,GWLP_WNDPROC);
      if (mPrevWindowProc != _StaticWndProc) {
        // wire in _StaticWndProc
        niWin32API(SetWindowLongPtr)(mHandle,GWLP_WNDPROC,(LONG_PTR)(_StaticWndProc));
      }
      else {
        // already _StaticWndProc, keep it as-is
        mPrevWindowProc = NULL;
      }
#ifdef niMSVC
#pragma warning (default : 4244)
#endif
    }
    mbOwnedHandle = abOwnHandle;
    mbDestroyed = eFalse;

#ifndef NO_RAWINPUT
    // Set mouse raw input
    if (1) {
      RAWINPUTDEVICE Rid[1];
      Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
      Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
      Rid[0].dwFlags = 0; //RIDEV_INPUTSINK; // Must be zero otherwise the function fails on some versions of windows
      Rid[0].hwndTarget = NULL;
      mbUseWMInput = !!RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
      // niDebugFmt((_A("USE_WM_INPUT: %d"),mbUseWMInput));
    }
#else
    mbUseWMInput = eFalse;
#endif

    niWin32API(PostMessage)(mHandle,MY_WM_IME_DISABLE,0,0);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetHandle() const {
    return (tIntPtr)mHandle;
  }
  virtual tBool __stdcall GetIsHandleOwned() const {
    return mbOwnedHandle;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall ActivateWindow() {
    niCheckSilent(mHandle!=NULL,;);
    if (!GetParent()) {
      ::SetForegroundWindow(mHandle);
      ::SetActiveWindow(mHandle);
    }
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsActive() const {
    return mbIsActive;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SwitchIn(tU32 anReason) {
    if (mbIsActive == eTrue)
      return eFalse;
    mbIsActive = eTrue;
    tU32 newKeyMod = 0;
    mStaticManager.Activate(newKeyMod);
    mStaticManager.mnCharPreviousKey = 0;
    _UpdateCursorCapture();
    _UpdateCursorClip();
    _UpdateCursor();
    ::SetCursor(mhCursor);
    _PostMessage(eOSWindowMessage_SwitchIn,anReason,newKeyMod);
    RedrawWindow();
    return eTrue;
  }
  virtual tBool __stdcall SwitchOut(tU32 anReason) {
    if (mbIsActive == eFalse)
      return eFalse;
    // make sure no capture sticks to the window...
    niLoop(i,niCountOf(mStaticManager.mbMouseDown)) {
      if (mStaticManager.mbMouseDown[i]) {
        mStaticManager.mbMouseDown[i] = eFalse;
        _PostMessage(eOSWindowMessage_MouseButtonUp,i);
      }
    }
    // Make sure the keys are unstuck no matter what...
    niLoop(i,niCountOf(mStaticManager.mbIsDown)) {
      if (mStaticManager.mbIsDown[i]) {
        mStaticManager.mbIsDown[i] = eFalse;
        _PostMessage(eOSWindowMessage_KeyUp,i);
      }
    }
    mStaticManager.mnCharPreviousKey = 0;
    mbIsActive = eFalse;
    _UpdateCursorCapture();
    _UpdateCursorClip();
    _UpdateCursor();
    _PostMessage(eOSWindowMessage_SwitchOut,anReason);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTitle(const achar* aaszTitle) {
    niCheckSilent(mHandle!=NULL,;);
    mstrTitle = aaszTitle;
    ni::Windows::UTF16Buffer wTitle;
    niWin32_UTF8ToUTF16(wTitle,mstrTitle.Chars());
    niWin32API(SetWindowText)(mHandle,wTitle.data());
  }
  virtual const achar* __stdcall GetTitle() const {
    return mstrTitle.Chars();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetStyle(tOSWindowStyleFlags aStyle) {
    niCheckSilent(mHandle!=NULL,;);
    if (!aStyle) return;
    WINDOWPLACEMENT wp;
    wp.length = sizeof(wp);
    ::GetWindowPlacement(mHandle,&wp);
    LONG_PTR style = GetWindowLongPtr(mHandle, GWL_STYLE);
    LONG_PTR exstyle = GetWindowLongPtr(mHandle, GWL_EXSTYLE);
    _GetWindowStyle(aStyle,style,exstyle);

#ifdef niMSVC
#pragma warning (disable : 4244)  //  conversion from 'LONG_PTR' to 'LONG', possible loss of data --- SetWindowLongPtr isnt declared to work warning-free with the Win64 checks
#endif
    niWin32API(SetWindowLongPtr)(mHandle,GWL_STYLE,style);
    niWin32API(SetWindowLongPtr)(mHandle,GWL_EXSTYLE,exstyle);
#ifdef niMSVC
#pragma warning (default : 4244)
#endif

    //    SetRect(GetRect()); // update the window rect to fit the current style
    ::SetWindowPlacement(mHandle,&wp);
  }
  virtual tOSWindowStyleFlags __stdcall GetStyle() const {
    niCheckSilent(mHandle!=NULL,eOSWindowStyleFlags_ForceDWORD);
    tOSWindowStyleFlags flags = 0;
    LONG_PTR style = GetWindowLongPtr(mHandle, GWL_STYLE);
    LONG_PTR exstyle = GetWindowLongPtr(mHandle, GWL_EXSTYLE);
    if (!(style & WS_SIZEBOX)) {
      flags |= eOSWindowStyleFlags_FixedSize;
    }
    else {
      style &= ~WS_SIZEBOX;
    }
    if (exstyle & WS_EX_TOOLWINDOW) {
      flags |= eOSWindowStyleFlags_Toolbox;
    }
    else if (style & WS_POPUP) {
      flags |= eOSWindowStyleFlags_Overlay;
    }
    else if (style & WS_CHILD) {
      flags |= eOSWindowStyleFlags_ClientArea;
    }
    else /*if (style & WS_SYSMENU)*/ {
      flags |= eOSWindowStyleFlags_Regular;
    }
    return flags;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetShow(tOSWindowShowFlags aShow) {
    niCheckSilent(mHandle!=NULL,;);
    int cmd = 0;
    if (niFlagIs(aShow,eOSWindowShowFlags_Hide)) {
      cmd |= SW_HIDE;
    }
    else if (niFlagIs(aShow,eOSWindowShowFlags_Restore)) {
      cmd |= SW_RESTORE;
    }
    else {
      if (niFlagIs(aShow,eOSWindowShowFlags_Maximize)) {
        if (niFlagIs(aShow,eOSWindowShowFlags_Show)) {
          cmd |= SW_SHOWMAXIMIZED;
        }
        else {
          cmd |= SW_MAXIMIZE;
        }
      }
      else if (niFlagIs(aShow,eOSWindowShowFlags_Minimize)) {
        if (niFlagIs(aShow,eOSWindowShowFlags_ShowNoActivate)) {
          cmd |= SW_SHOWMINNOACTIVE;
        }
        else if (niFlagIs(aShow,eOSWindowShowFlags_Show)) {
          cmd |= SW_SHOWMINIMIZED;
        }
        else {
          cmd |= SW_MINIMIZE;
        }
      }
      else {
        if (niFlagIs(aShow,eOSWindowShowFlags_Show)) {
          cmd |= SW_SHOWNORMAL;
        }
        else if (niFlagIs(aShow,eOSWindowShowFlags_ShowNoActivate)) {
          cmd |= SW_SHOWNOACTIVATE;
        }
      }
    }
    if (cmd) {
      ShowWindow(mHandle,cmd);
    }
  }
  virtual tOSWindowShowFlags __stdcall GetShow() const {
    niCheckSilent(mHandle!=NULL,0);
    WINDOWPLACEMENT wp;
    wp.length = sizeof(wp);
    ::GetWindowPlacement(mHandle,&wp);
    //         niDebugFmt((_A("## GETWINDPLACEMENT(%d): %d\n"),
    //                   r,wp.showCmd));
    tOSWindowShowFlags cmd = 0;
    switch (wp.showCmd) {
      case SW_SHOW:
      case SW_SHOWNORMAL:
        cmd |= eOSWindowShowFlags_Show;
        break;
      case SW_SHOWMINIMIZED:
        cmd |= eOSWindowShowFlags_Show|eOSWindowShowFlags_Minimize;
        break;
      case SW_SHOWMAXIMIZED: // SW_MAXIMIZE has the same value...
        cmd |= eOSWindowShowFlags_Show|eOSWindowShowFlags_Maximize;
        break;
      case SW_SHOWNOACTIVATE:
        cmd |= eOSWindowShowFlags_ShowNoActivate;
        break;
      case SW_HIDE:
        cmd |= eOSWindowShowFlags_Hide;
        break;
      case SW_RESTORE:
        cmd |= eOSWindowShowFlags_Restore;
        break;
      case SW_MINIMIZE:
        cmd |= eOSWindowShowFlags_Minimize;
        break;
    }
    return cmd;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetZOrder(eOSWindowZOrder aZOrder) {
    niCheckSilent(mHandle!=NULL,;);
  }
  virtual eOSWindowZOrder __stdcall GetZOrder() const {
    niCheckSilent(mHandle!=NULL,eOSWindowZOrder_Normal);
    return eOSWindowZOrder_Normal;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetSize(const sVec2i& avSize) {
    sRecti rect = GetRect();
    rect.SetSize(avSize);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetSize() const {
    return GetRect().GetSize();
  }
  virtual void __stdcall SetPosition(const sVec2i& avPos) {
    sRecti rect = GetRect();
    rect.SetTopLeft(avPos);
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetPosition() const {
    return GetRect().GetTopLeft();
  }
  virtual void __stdcall SetRect(const sRecti& aRect) {
    niCheckSilent(mHandle!=NULL,;);
    sRecti newRect = aRect;
    if (GetParent()) {
      if (::GetWindowLong(mHandle,GWL_STYLE) & WS_CHILD) {
        ScreenToClient(mHandle,(LPPOINT)&newRect.x);
        ScreenToClient(mHandle,(LPPOINT)&newRect.z);
      }
    }
    SetWindowPos(mHandle,NULL,
                 newRect.GetLeft(),newRect.GetTop(),
                 newRect.GetWidth(),newRect.GetHeight(),
                 SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
  }
  virtual sRecti __stdcall GetRect() const {
    niCheckSilent(mHandle!=NULL,sRecti::Null());
    sRecti rect;
    ::GetWindowRect(mHandle,(LPRECT)&rect);
    return rect;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetClientSize(const sVec2i& avSize) {
    sRecti rect = GetRect();
    rect.SetSize(avSize);
    AdjustWindowRectEx( (RECT*)&rect, GetWindowLong(mHandle,GWL_STYLE),
                        GetMenu(mHandle) != NULL,
                        GetWindowLong(mHandle,GWL_EXSTYLE));
    SetRect(rect);
  }
  virtual sVec2i __stdcall GetClientSize() const {
    niCheckSilent(mHandle!=NULL,sVec2i::Zero());
    sRecti rect;
    ::GetClientRect(mHandle,(LPRECT)&rect);
    return rect.GetSize();
  }

  /////////////////////////////////////////////////////////////////
  virtual tF32 __stdcall GetContentsScale() const niImpl {
    return mfContentsScale;
  }

  ///////////////////////////////////////////////
  void __stdcall _DoClear() {
    if (!mStaticManager.mbrushClear && !mStaticManager.miconLogo)
      return;

    ni::Windows::DC dc(::GetDC(mHandle));
    sRecti rect;
    ::GetClientRect(mHandle,(RECT*)&rect);

    // background clear
    dc.FillRect((RECT&)rect, mStaticManager.mbrushClear);

    if (mStaticManager.miconLogo) {
      ::DrawIconEx(dc.GetDC(),
                   rect.GetLeft()+((rect.GetWidth()-mStaticManager.mnIconLogoWidth)/2),
                   rect.GetTop()+((rect.GetHeight()-mStaticManager.mnIconLogoHeight)/2),
                   mStaticManager.miconLogo,
                   mStaticManager.mnIconLogoWidth,
                   mStaticManager.mnIconLogoHeight,
                   0, NULL, DI_NORMAL | DI_COMPAT);
    }
  }

  ///////////////////////////////////////////////
  virtual tMessageHandlerSinkLst* __stdcall GetMessageHandlers() const {
    return mptrMT;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall UpdateWindow(tBool abBlockingMessages) {
    niCheckSilent(mHandle!=NULL,eFalse);
    if (mfRefreshTimer == 0) {
      abBlockingMessages = eFalse;
    }

    if (mptrWindowThread.IsOK()) {
      if (abBlockingMessages && mptrWindowThread.IsOK()) {
        mptrWindowThread->WaitForMessage();
      }
    }
    else {
      // this way feels better, it make sure all input messages are processed in one shot...
      tU32 numProcessMessages = 0;
      if (abBlockingMessages) {
        WaitMessage();
      }
      MSG msg;
      while (niWin32API(PeekMessage)(&msg, NULL, 0, 0, PM_REMOVE)) {
        ::TranslateMessage(&msg);
        niWin32API(DispatchMessage)(&msg);
        ++numProcessMessages;
        if (numProcessMessages > 100)
          break;
      }
    }

    sVec2i curPos;
    ::GetCursorPos((LPPOINT)&curPos);
    ::ScreenToClient(mHandle,(LPPOINT)&curPos);
    sRecti clientRect;
    ::GetClientRect(mHandle,(LPRECT)&clientRect);
    mbMouseOverClient = clientRect.Intersect(curPos);

    _PostMessage(eOSWindowMessage_Paint);
    return eTrue;
  }

  virtual tBool __stdcall RedrawWindow() {
    ::InvalidateRect(mHandle,NULL,FALSE);
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall CenterWindow()
  {
    const tU32 nMonitor = GetMonitor();
    if (nMonitor == eInvalidHandle) {
      niWarning(niFmt("Can't center window: can't get monitor of window %x.", (tIntPtr)mHandle));
      return;
    }
    const sRecti monitorRect = ni::GetLang()->GetMonitorRect(nMonitor);
    const sRecti rect = CenterRect(monitorRect, this->GetSize());
    // niDebugFmt(("... CenterWindow: (%d) %s -> %s", nMonitor, monitorRect, rect));
    SetRect(rect);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetRequestedClose() const {
    return mbRequestedClose;
  }
  virtual void __stdcall SetRequestedClose(tBool abRequested) {
    mbRequestedClose = abRequested;
    if (mptrWindowThread.IsOK())
      mptrWindowThread->SetRequestedClose(abRequested);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCursor(eOSCursor aCursor) {
    switch (aCursor) {
      case eOSCursor_None:
        mhCursor = NULL;
        break;
      case eOSCursor_Wait:
        mhCursor = mStaticManager.mcurWait;
        break;
      case eOSCursor_Hand:
        mhCursor = mStaticManager.mcurHelp;
        break;
      case eOSCursor_Text:
        mhCursor = mStaticManager.mcurText;
        break;
      case eOSCursor_Help:
        mhCursor = mStaticManager.mcurHelp;
        break;
      case eOSCursor_ResizeHz:
        mhCursor = mStaticManager.mcurResizeHz;
        break;
      case eOSCursor_ResizeVt:
        mhCursor = mStaticManager.mcurResizeVt;
        break;
      case eOSCursor_ResizeDiag:
        mhCursor = mStaticManager.mcurResizeDiag;
        break;
      case eOSCursor_Custom:
        if (mhCustomCursor) {
          mhCursor = mhCustomCursor;
          break;
        }
      default:
      case eOSCursor_Arrow:
        mhCursor = mStaticManager.mcurArrow;
        break;
    }
    if (mbMouseOverClient) {
      // release the capture before changing the cursor otherwise
      // windows refuses to change the cursor
      const tU32 wasCapture = mCursorCapture;
      if (wasCapture) { mCursorCapture = 0; _UpdateCursorCapture(); }
      ::SetCursor(mhCursor);
      if (wasCapture) { mCursorCapture = wasCapture; _UpdateCursorCapture(); }
    }
    _UpdateCursor();
  }
  virtual eOSCursor __stdcall GetCursor() const {
    if (!mhCursor)
      return eOSCursor_None;
    if (mhCustomCursor && mhCursor == mhCustomCursor)
      return eOSCursor_Custom;
    if (mhCursor == mStaticManager.mcurWait)
      return eOSCursor_Wait;
    if (mhCursor == mStaticManager.mcurResizeHz)
      return eOSCursor_ResizeHz;
    if (mhCursor == mStaticManager.mcurResizeVt)
      return eOSCursor_ResizeVt;
    if (mhCursor == mStaticManager.mcurResizeDiag)
      return eOSCursor_ResizeDiag;
    if (mhCursor == mStaticManager.mcurHand)
      return eOSCursor_Hand;
    if (mhCursor == mStaticManager.mcurText)
      return eOSCursor_Text;
    if (mhCursor == mStaticManager.mcurHelp)
      return eOSCursor_Help;
    return eOSCursor_Arrow;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCursorPosition(const sVec2i& avCursorPos) {
    POINT p = { avCursorPos.x, avCursorPos.y };
    ::ClientToScreen(mHandle, &p);
    ::SetCursorPos(p.x,p.y);
  }
  virtual sVec2i __stdcall GetCursorPosition() const {
    return mvPrevMousePos;
  }

  ///////////////////////////////////////////////
  void __stdcall _SetCursorCapture(const tU32 anCaptureFlag, tBool abCapture) {
    if (abCapture == niFlagIs(mCursorCapture,anCaptureFlag))
      return;
    niFlagOnIf(mCursorCapture, anCaptureFlag, abCapture);
    _UpdateCursorCapture();
  }
  void __stdcall SetCursorCapture(tBool abCapture) {
    _SetCursorCapture(CURSOR_CAPTURE_GRAB, abCapture);
  }
  tBool __stdcall GetCursorCapture() const {
    return niFlagIs(mCursorCapture,CURSOR_CAPTURE_GRAB);
  }
  void __stdcall _UpdateCursorCapture() {
    if (!mHandle)
      return;
    if (mbIsActive && mCursorCapture) {
      niWin32API(SendMessage)(mHandle,MY_WM_SETCAPTURE,0,0);
    }
    else {
      niWin32API(SendMessage)(mHandle,MY_WM_RELEASECAPTURE,0,0);
    }
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCursorClip(const sRecti& aRect) {
    if (mrectCursorClip == aRect) return;
    mrectCursorClip = aRect;
    _UpdateCursorClip();
  }
  virtual sRecti __stdcall GetCursorClip() const {
    return mrectCursorClip;
  }
  void __stdcall _UpdateCursorClip() {
    if (mbIsActive && mrectCursorClip != sRecti::Null()) {
      RECT rc = (RECT&)mrectCursorClip;
      ::ClientToScreen(mHandle,(LPPOINT)&rc.left);
      ::ClientToScreen(mHandle,(LPPOINT)&rc.right);
      ::ClipCursor(&rc);
    }
    else {
      ::ClipCursor(NULL);
    }
  }

  ///////////////////////////////////////////////
  tBool __stdcall InitCustomCursor(tIntPtr aID, tU32 anWidth, tU32 anHeight, tU32 anPivotX, tU32 anPivotY, const tU32* apData) {
    if (aID == mnCustomCursorID)
      return eTrue;
    if (anWidth != 32 || anHeight != 32) {
      if (apData)
        mnCustomCursorID = 0;
      return eFalse;
    }
    if (!apData) // we're just checking size support...
      return eTrue;

    BITMAPV5HEADER bi;
    ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
    bi.bV5Size   = sizeof(BITMAPV5HEADER);
    bi.bV5Width  = 32;
    bi.bV5Height = 32;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    // The following mask specification specifies a supported 32 BPP
    // alpha format for Windows XP.
    bi.bV5RedMask   =  0x00FF0000;
    bi.bV5GreenMask =  0x0000FF00;
    bi.bV5BlueMask  =  0x000000FF;
    bi.bV5AlphaMask =  0xFF000000;

    // Create the DIB section with an alpha channel.
    HDC hdc = GetDC(mHandle);
    niCheck(hdc!=NULL,eFalse);
    void *lpBits = NULL;
    HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS,
                                       (void **)&lpBits, NULL, (DWORD)0);
    ReleaseDC(mHandle,hdc);
    niCheck(hBitmap!=NULL,eFalse);
    niCheck(lpBits!=NULL,eFalse);

    // Create an empty mask bitmap.
    HBITMAP hMonoBitmap = CreateBitmap(32,32,1,1,NULL);
    if (!hMonoBitmap) {
      DeleteObject(hBitmap);
      niCheck(hMonoBitmap!=NULL,eFalse);
    }

    // Set the alpha values for each pixel in the cursor so that
    // the complete cursor is semi-transparent.
    const tU32* pSrc = apData;
    DWORD* lpdwPixel = (DWORD *)lpBits;
    for (DWORD y = 0; y < 32; ++y) {
      for (DWORD x = 0; x < 32; ++x) {
        tU32 c = *(pSrc+(31-y)*32+x);
        tU8 r = ULColorGetR(c);
        tU8 g = ULColorGetG(c);
        tU8 b = ULColorGetB(c);
        tU8 a = ULColorGetA(c);
        *lpdwPixel = ULColorBuild(b,g,r,a);
        ++lpdwPixel;
      }
    }

    ICONINFO ii;
    ii.fIcon = FALSE; // FALSE for cursor
    ii.xHotspot = anPivotX;
    ii.yHotspot = anPivotY;
    ii.hbmMask = hMonoBitmap;
    ii.hbmColor = hBitmap;

    // Create the alpha cursor with the alpha DIB section.
    mhCustomCursor = CreateIconIndirect(&ii);

    DeleteObject(hBitmap);
    DeleteObject(hMonoBitmap);

    mnCustomCursorID = aID;
    return mhCustomCursor != NULL;
  }
  tIntPtr __stdcall GetCustomCursorID() const {
    return mnCustomCursorID;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsCursorOverClient() const {
    return mbMouseOverClient;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall TryClose() {
    if (!mHandle)
      return;

    if (GetRequestedClose()) {
      // already requested close...
    }
    else {
      if (!mptrMT.IsOK() || mptrMT->IsEmpty()) {
        this->SetRequestedClose(ni::eTrue);
      }
      else {
        _PostMessage(eOSWindowMessage_Close);
      }
    }
  }


  ///////////////////////////////////////////////
  virtual void __stdcall SetDropTarget(tBool abDropTarget) {
    if (!mHandle) return;
    if (mbDropTarget == abDropTarget) return;
    mbDropTarget = abDropTarget;
#ifndef USE_IDROPTARGET
    ::DragAcceptFiles(mHandle,abDropTarget);
#else
    if (mbDropTarget) {
      mpDropTarget = niNew cWin32DropTarget(mHandle);
      mpDropTarget->AddRef();
      HRESULT hr = ::RegisterDragDrop(mHandle,mpDropTarget);
      if (FAILED(hr)) {
        niWarning("Can't initialize drop target '%s'.");
      }
      else {
        FORMATETC ftetc={0};
        ftetc.cfFormat = CF_TEXT;
        ftetc.dwAspect = DVASPECT_CONTENT;
        ftetc.lindex = -1;
        ftetc.tymed = TYMED_HGLOBAL;
        mpDropTarget->AddSuportedFormat(ftetc);
        ftetc.cfFormat=CF_HDROP;
        mpDropTarget->AddSuportedFormat(ftetc);
      }
    }
    else {
      if (mpDropTarget) {
        ::RevokeDragDrop(mHandle);
      }
      niSafeRelease(mpDropTarget);
    }
#endif
  }
  virtual tBool __stdcall GetDropTarget() const {
    return mbDropTarget;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetBackgroundUpdate(tBool abBackgroundUpdate) {
    mbBackgroundUpdate = abBackgroundUpdate;
  }
  virtual tBool __stdcall GetBackgroundUpdate() const {
    return mbBackgroundUpdate;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetMonitor() const {
    HMONITOR mon = MonitorFromWindow(mHandle,MONITOR_DEFAULTTOPRIMARY);
    return ni::GetLang()->GetMonitorIndex((tIntPtr)mon);
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetFullScreen(tU32 anMonitor) {
    if (mFullScreen.setOnMonitor == anMonitor)
      return eTrue;  // Nothing to do.

    mbIsResizing = eTrue;
    // Toggle fullscreen mode.
    mFullScreen.setOnMonitor = anMonitor;
    if (mFullScreen.setOnMonitor != eInvalidHandle) {
      // Set to fullscreen
      ::GetWindowPlacement(mHandle,&mFullScreen.wp);
      mFullScreen.style = GetWindowLong(mHandle, GWL_STYLE);
      mFullScreen.ex_style = GetWindowLong(mHandle, GWL_EXSTYLE);
      mFullScreen.window_rect = this->GetRect();

      // Set new window style and size.
      ::SetWindowLong(mHandle, GWL_STYLE,
                      mFullScreen.style & ~(WS_CAPTION | WS_THICKFRAME));
      ::SetWindowLong(mHandle, GWL_EXSTYLE,
                      mFullScreen.ex_style & ~(WS_EX_DLGMODALFRAME |
                                               WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE |
                                               WS_EX_STATICEDGE));

      HMONITOR hMonitor = (HMONITOR)ni::GetLang()->GetMonitorHandle(anMonitor);
      if (!hMonitor) {
        hMonitor = MonitorFromWindow(mHandle, MONITOR_DEFAULTTOPRIMARY);
      }
      MONITORINFO monitor_info;
      monitor_info.cbSize = sizeof(monitor_info);
      ::GetMonitorInfo(hMonitor,&monitor_info);
      const sRecti& newRect = (const sRecti&)monitor_info.rcMonitor;
      ::SetWindowPos(mHandle, NULL,
                     newRect.GetLeft(), newRect.GetTop(),
                     newRect.GetWidth(), newRect.GetHeight(),
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

      // these force proper refresh and focus...
      ::ShowWindow(mHandle,SW_HIDE);
      ::ShowWindow(mHandle,SW_SHOW);
    }
    else {
      // these force proper refresh and focus...
      ::ShowWindow(mHandle,SW_HIDE);
      ::ShowWindow(mHandle,SW_SHOW);

      // Reset original window style and size.  The multiple window size/moves
      // here are ugly, but if SetWindowPos() doesn't redraw, the taskbar won't be
      // repainted.  Better-looking methods welcome.
      ::SetWindowLong(mHandle, GWL_STYLE, mFullScreen.style);
      ::SetWindowLong(mHandle, GWL_EXSTYLE, mFullScreen.ex_style);
      ::SetWindowPlacement(mHandle,&mFullScreen.wp);
      ::SetWindowPos(mHandle, NULL,
                     mFullScreen.window_rect.GetLeft(),
                     mFullScreen.window_rect.GetTop(),
                     mFullScreen.window_rect.GetWidth(),
                     mFullScreen.window_rect.GetHeight(),
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    }
    mbIsResizing = eFalse;

    // ALL of this is needed, otherwise Windows won't always update the taskbar...
    RedrawWindow();
    ::UpdateWindow(mHandle);
    ::SetForegroundWindow(mHandle);
    ::SetActiveWindow(mHandle);
    if (mptrWindowThread.IsOK())
      mptrWindowThread->WaitForMessage();

    _UnstickWindows();
    return eTrue;
  }

  tU32 __stdcall GetFullScreen() const {
    return mFullScreen.setOnMonitor;
  }
  tBool __stdcall GetIsMaximized() const {
    if (!mHandle) return eFalse;
    return !!::IsZoomed(mHandle);
  }
  tBool __stdcall GetIsMinimized() const {
    if (!mHandle) return eFalse;
    return !!::IsIconic(mHandle);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFocus() {
    ::SetFocus(mHandle);
  }
  virtual tBool __stdcall GetHasFocus() const {
    return mbHasFocus;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Clear() {
    _DoClear();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetRefreshTimer(tF32 afRefreshTimer) {
    if (afRefreshTimer <= 0) {
      if (afRefreshTimer < -niEpsilon5)
        mfRefreshTimer = -1;
      else
        mfRefreshTimer = 0.0;
      ::KillTimer(mHandle,TIMERID_REFRESH);
    }
    else if (mfRefreshTimer != afRefreshTimer) {
      mfRefreshTimer = afRefreshTimer;
      ::SetTimer(mHandle,TIMERID_REFRESH,(tU32)(mfRefreshTimer*1000.0f),NULL);
    }
  }
  virtual tF32 __stdcall GetRefreshTimer() const {
    return mfRefreshTimer;
  }

  ///////////////////////////////////////////////
  ni::tU32 __stdcall WindowsTranslateKey(ni::tU32 wParam, ni::tU32 lParam, ni::tBool abDown) {
    return mStaticManager.TranslateKey(wParam,lParam,abDown);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall AttachGraphicsAPI(iOSGraphicsAPI* apAPI) {
    mptrAttachedGraphicsAPI = apAPI;
    return mptrAttachedGraphicsAPI.IsOK();
  }
  virtual iOSGraphicsAPI* __stdcall GetGraphicsAPI() const {
    return mptrAttachedGraphicsAPI;
  }

 public:
  Ptr<iOSGraphicsAPI>                     mptrAttachedGraphicsAPI;
  Ptr<iOSWindow>                          mptrParentWindow;
  tIntPtr                                 mhClientAreaWnd;
  cString                                 mstrTitle;
  tBool                                   mbOwnedHandle;
  tBool                                   mbDestroyed;
  HWND                                    mHandle;
  Ptr<cThreadedWindow>                    mptrWindowThread;
  Ptr<tMessageHandlerSinkLst>             mptrMT;
  tBool                                   mbIsActive;
  tBool                                   mbIsResizing;
  tBool                                   mbRequestedClose;
  tBool                                   mbDropTarget;
  tBool                                   mbBackgroundUpdate;
  Nonnull<tOSWindowWindowsSinkList>      mlstSinks;
#ifdef USE_IDROPTARGET
  cWin32DropTarget*                       mpDropTarget;
#endif
  sWindowsStaticManager                   mStaticManager;
  tF32                                    mfRefreshTimer;
  sVec2i                                  mvPrevMousePos;
  sVec2i                                  mvRawInputAbsRelPos;
  tBool                                   mbMouseOverClient;
  tBool                                   mbHasFocus;
  tBool                                   mbUseWMInput;
  HCURSOR                                 mhCursor;
  HCURSOR                                 mhCustomCursor;
  tIntPtr                                 mnCustomCursorID;
  tU32                                    mCursorCapture;
  sRecti                                  mrectCursorClip;
#if !defined NO_POINTEREVENT
  sFingerHandler                          mFingerHandler;
#endif
  tF32                                    mfContentsScale;

  friend class cOSPlatformWindows;

  LRESULT (CALLBACK *mPrevWindowProc)(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

  struct sFullScreen {
    tU32  setOnMonitor;
    DWORD style;
    DWORD ex_style;
    sRecti  window_rect;
    WINDOWPLACEMENT wp;
    sFullScreen() {
      setOnMonitor = eInvalidHandle;
      style = 0;
      ex_style = 0;
      window_rect = sRecti::Null();
      wp.length = sizeof(wp);
    }
  } mFullScreen;

  ///////////////////////////////////////////////
  static void _ShowCursor() {
    int newCounter;
    do {
      newCounter = ::ShowCursor(TRUE);
    } while (newCounter < 0);
  }
  static void _HideCursor() {
    int newCounter;
    do {
      newCounter = ::ShowCursor(FALSE);
    } while (newCounter > 0);
  }
  static int _CursorCounter() {
    int newCounter = ::ShowCursor(TRUE);
    newCounter = ::ShowCursor(FALSE);
    return newCounter;
  }
  static void _UpdateCursor() {
    POINT p;
    GetCursorPos(&p);
    SetCursorPos(p.x,p.y);
  }

  ///////////////////////////////////////////////
  __forceinline tBool _CanSendMouseMessage() const {
    return eTrue;
  }
  tBool _PostMessage(eOSWindowMessage aMsg, const Var& avarA = niVarNull, const Var& avarB = niVarNull) {
    return SendMessages(mptrMT,aMsg,avarA,avarB);
  }
  void _SendMouseMove(const sVec2i& avMousePos) {
    if (avMousePos == mvPrevMousePos)
      return;
    if (!mbUseWMInput && (mvPrevMousePos.x != eInvalidHandle)) {
      const sVec2i vRelMove = avMousePos-mvPrevMousePos;
      _PostMessage(eOSWindowMessage_RelativeMouseMove,vRelMove);
    }

    // dont send mouse move when we've grabbed the cursor
    if (niFlagIsNot(mCursorCapture,CURSOR_CAPTURE_GRAB)) {
      _PostMessage(eOSWindowMessage_MouseMove,avMousePos);
    }
    mvPrevMousePos = avMousePos;
  }
  void _HandleMouseMove(LPARAM lParam) {
    sVec2i vMousePos;
    vMousePos.x = ((int)(short)LOWORD(lParam));
    vMousePos.y = ((int)(short)HIWORD(lParam));
    _SendMouseMove(vMousePos);
  }
  void _HandleNCMouseMove(LPARAM lParam) {
    sVec2i vMousePos;
    vMousePos.x = ((int)(short)LOWORD(lParam));
    vMousePos.y = ((int)(short)HIWORD(lParam));
    ::ScreenToClient(mHandle,(LPPOINT)&vMousePos);
    _SendMouseMove(vMousePos);
  }
  void _SendMouseWheel(const tF32 afDelta) {
    _PostMessage(eOSWindowMessage_MouseWheel,afDelta);
  }

  ///////////////////////////////////////////////
  tIntPtr __stdcall WndProc(tIntPtr hWnd, tU32 message, tIntPtr wParam, tIntPtr lParam)
  {
    tIntPtr sinkRet = 0;
    if (mlstSinks.IsOK()) {
      niCallSinkRetTest_(OSWindowWindowsSink,
                         mlstSinks,
                         WndProc,
                         (hWnd,message,wParam,lParam),
                         sinkRet,sinkRet == 0);
    }
    if (sinkRet != 0)
      return sinkRet;

    switch (message)
    {
      case WM_CREATE: {
        ::ShowCursor(TRUE);
        break;
      }
      case WM_SETCURSOR: {
        //        niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_SETCURSOR(%d) - wParam:%d:%d:%d - lParam:%d:%d:%d (%d)"),
        //                _CursorCounter(),
        //                wParam,LOWORD(wParam),HIWORD(wParam),
        //                lParam,LOWORD(lParam),HIWORD(lParam),
        //                GetCapture() == hWnd));
        mbMouseOverClient = (LOWORD(lParam) == 1);
        if (mbMouseOverClient) {
          ::SetCursor(mhCursor);
          _HideCursor();
          _ShowCursor();
          return 0;
        }
        break;
      }
      case WM_DESTROY:
        {
          HWND ca = (HWND)GetClientAreaWindow();
          if (ca && ::IsWindow(ca)) {
            ::DestroyWindow(ca);
            SetClientAreaWindow(NULL);
          }
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_DESTROY - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));

          mbDestroyed = eTrue; // in destroy message handler so mark as destroyed...
          Invalidate();
          break;
        }

      case WM_CLOSE:
        {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_CLOSE - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));

          this->TryClose();
          if (mptrWindowThread.IsOK())
            return 0; // Keep the window, the thread will stop when GetRequestedClose is true
          else {
            if (this->GetRequestedClose())
              break; // let the default window proc close the window
            return 0;
          }
        }

      case WM_SHOWWINDOW:
        {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_SHOWWINDOW[%p] - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         mHandle,
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          //             niDebugFmt((_A("=== WINMSG === WM_SHOWWINDOW[%p] - wParam:%d:%d:%d - lParam:%d:%d:%d"),
          //                          mHandle,
          //                          wParam,LOWORD(wParam),HIWORD(wParam),
          //                          lParam,LOWORD(lParam),HIWORD(lParam)));
          break;
        }

      case WM_WINDOWPOSCHANGING: {
        break;
      }

      case WM_WINDOWPOSCHANGED: {
        // WE CANNOT USE THIS, WE HAVE TO HANDLE WM_SIZE/MOVE otherwise
        // we'll have issues with the window when its embedded in an ActiveX control.
        // It is likely that the ActiveX relies on WM_SIZE/MOVE as-well...
        //             return 0; // WM_SIZE and WM_MOVE are not sent...
        break;
      }

      case WM_SIZE:
      case WM_MOVE: {
        //             niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_SHOWWINDOW[%p] - wParam:%d:%d:%d - lParam:%d:%d:%d"),
        //                                              mHandle,
        //                                              wParam,LOWORD(wParam),HIWORD(wParam),
        //                                              lParam,LOWORD(lParam),HIWORD(lParam)));

        // Handle move and size change here...

        HWND ca = (HWND)GetClientAreaWindow();
        const tBool hasClientAreaWindow = (ca && ::IsWindow(ca));

        if (hasClientAreaWindow) {
          if (::GetWindowLong(ca,GWL_STYLE) & WS_CHILD) {
            sRecti rect;
            ::GetClientRect(mHandle,(LPRECT)&rect);
            rect.MoveTo(sVec2i::Zero());
            ::SetWindowPos(ca,NULL,
                           rect.GetLeft(),rect.GetTop(),
                           rect.GetWidth(),rect.GetHeight(),
                           SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
            // Win32 UpdateWindow is synchronous it fucks up
            // the message queue and create app freezes so its
            // disabled, and should never be used...
            // ::UpdateWindow(ca);
          }
        }
        _HideCursor();
        _ShowCursor();
        _UpdateCursor();

        if (message == WM_MOVE) {
          _PostMessage(eOSWindowMessage_Move);
        }
        if (message == WM_SIZE) {
          _PostMessage(eOSWindowMessage_Size);
        }
        if (mptrWindowThread.IsOK()) {
          mptrWindowThread->SignalMessage();
        }

        break;
      }

      case WM_ENTERSIZEMOVE:
      case WM_EXITSIZEMOVE:
      case WM_ACTIVATE:
        {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_ACTIVATE - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          mvPrevMousePos.x = mvPrevMousePos.y = eInvalidHandle;
          mvRawInputAbsRelPos.x = mvRawInputAbsRelPos.y = eInvalidHandle;
          mbIsResizing = (message == WM_ENTERSIZEMOVE);
          if (!GetParent())
          {
            if (message == WM_ENTERSIZEMOVE ||
                (message == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE))
            {
              this->SwitchOut(eOSWindowSwitchReason_Deactivated);
            }
            else {
              this->SwitchIn(eOSWindowSwitchReason_Activated);
            }
          }
          _UnstickWindows();
          _UpdateContentsScale("WM_ACTIVATE", 0);
          break;
        }
      case WM_ENTERMENULOOP:
        {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_ENTERMENULOOP - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          this->SwitchOut(eOSWindowSwitchReason_EnterMenu);
          return 0;
        }

      case WM_EXITMENULOOP:
        {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_EXITMENULOOP - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          this->SwitchOut(eOSWindowSwitchReason_LeaveMenu);
          RedrawWindow();
          return 0;
        }

      case WM_INITMENUPOPUP:
        {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_INITMENUPOPUP - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          return 0;
        }

      case WM_MENUSELECT:
        {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_MENUSELECT - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          return 0;
        }

      case WM_POWERBROADCAST:
        {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_POWERBROADCAST - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          return BROADCAST_QUERY_DENY;
        }

      case WM_ERASEBKGND:
        {
          HWND ca = (HWND)GetClientAreaWindow();
          if (ca && ::IsWindow(ca)) {
            ::InvalidateRect(ca,NULL,FALSE);
          }
          else {
            if (mptrWindowThread.IsOK())
              mptrWindowThread->SignalMessage();
            if (!mbIsResizing) {
              _DoClear();
              return 1;
            }
          }
          return 0;
        }

      case WM_NCPAINT: {
        //             if (!mbIsResizing) {
        //                 _PostMessage(eOSWindowMessage_NCPaint);
        //             }
        break;
      }

      case WM_PAINT: {
        break;
      }

      case WM_KILLFOCUS: {
        HWND hwndTo = wParam ? (HWND)wParam : GetForegroundWindow();
        DWORD pid = 0;
        GetWindowThreadProcessId(hwndTo,&pid);
        mbHasFocus = eFalse;
        _PostMessage(eOSWindowMessage_LostFocus,(tIntPtr)hwndTo,(tIntPtr)pid);
        _UnstickWindows();
        if (GetParent()) {
          this->SwitchOut(eOSWindowSwitchReason_LostFocus);
        }
        break;
      }

      case WM_SETFOCUS: {
        if (GetParent()) {
          this->SwitchIn(eOSWindowSwitchReason_SetFocus);
          ActivateWindow();
          RedrawWindow();
        }
        HWND ca = (HWND)GetClientAreaWindow();
        if (ca && ::IsWindow(ca)) {
          ::SetFocus(ca);
        }
        else {
          mbHasFocus = eTrue;
          _PostMessage(eOSWindowMessage_SetFocus);
        }
        _UnstickWindows();
        break;
      }

      case WM_SYSCOMMAND: {
        if (_kbDisableScreenSaver &&
            ((wParam & 0xfff0) == SC_MONITORPOWER || (wParam & 0xfff0) == SC_SCREENSAVE))
        {
          return 0;
        }
        else if ((wParam & 0xfff0) == SC_KEYMENU) {
          /* Prevent Windows from intercepting the ALT key.
             (Disables opening menus via the ALT key.) */
          return 0;
        }
        break;
      }

      case WM_SYSKEYDOWN: {
        if (mStaticManager.AcceptKey(wParam,lParam)) {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_SYSKEYDOWN - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));

          {
            tU32 key = mStaticManager.TranslateKey((tU32)wParam,(tU32)lParam,eTrue);
            mStaticManager.mbIsDown[key] = eTrue;
            mStaticManager.mnCharPreviousKey = key;
            _PostMessage(eOSWindowMessage_KeyDown,key);
          }
        }
        break;
      }
      case WM_KEYDOWN: {
        if (mStaticManager.AcceptKey(wParam,lParam)) {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_KEYDOWN - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          tU32 key = mStaticManager.TranslateKey((tU32)wParam,(tU32)lParam,eTrue);
          mStaticManager.mbIsDown[key] = eTrue;
          mStaticManager.mnCharPreviousKey = key;
          _PostMessage(eOSWindowMessage_KeyDown,key);
        }
        break;
      }
      case WM_SYSKEYUP: {
        niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_SYSKEYUP - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                       wParam,LOWORD(wParam),HIWORD(wParam),
                                       lParam,LOWORD(lParam),HIWORD(lParam)));
        tU32 key = mStaticManager.TranslateKey((tU32)wParam,(tU32)lParam,eFalse);
        mStaticManager.mbIsDown[key] = eFalse;
        mStaticManager.mnCharPreviousKey = 0;
        _PostMessage(eOSWindowMessage_KeyUp,key);
        break;
      }
      case WM_KEYUP: {
        niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_KEYUP - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                       wParam,LOWORD(wParam),HIWORD(wParam),
                                       lParam,LOWORD(lParam),HIWORD(lParam)));
        tU32 key = mStaticManager.TranslateKey((tU32)wParam,(tU32)lParam,eFalse);
        mStaticManager.mbIsDown[key] = eFalse;
        mStaticManager.mnCharPreviousKey = 0;
        _PostMessage(eOSWindowMessage_KeyUp,key);
        break;
      }

      case WM_CHAR: {
        niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_CHAR - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                       wParam,LOWORD(wParam),HIWORD(wParam),
                                       lParam,LOWORD(lParam),HIWORD(lParam)));
        tU32 charCode = (tU32)wParam;
        _PostMessage(eOSWindowMessage_KeyChar,charCode,mStaticManager.mnCharPreviousKey);
        mStaticManager.mnCharPreviousKey = 0;
        break;
      }

      case WM_IME_CHAR: {
        niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_IME_CHAR - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                       wParam,LOWORD(wParam),HIWORD(wParam),
                                       lParam,LOWORD(lParam),HIWORD(lParam)));
        tU32 charCode = (tU32)wParam;
        mStaticManager.mnCharPreviousKey = 0;
        _PostMessage(eOSWindowMessage_KeyChar,charCode,0);
        return 0;
      }

      case WM_NCMOUSEMOVE:
      case WM_MOUSEMOVE: {
        if (_CanSendMouseMessage()) {
          _HandleMouseMove(lParam);
        }
        break;
      }
      case WM_LBUTTONDOWN: {
        if (_CanSendMouseMessage()) {
          niWindowsOSPlatformTrace(niFmt(_A("=== WINMSG === WM_LBUTTONDOWN - wParam:%d:%d:%d - lParam:%d:%d:%d"),
                                         wParam,LOWORD(wParam),HIWORD(wParam),
                                         lParam,LOWORD(lParam),HIWORD(lParam)));
          this->SetFocus();
          mStaticManager.mbMouseDown[ePointerButton_Left] = eTrue;
          _SetCursorCapture(CURSOR_CAPTURE_L, eTrue);
          _PostMessage(eOSWindowMessage_MouseButtonDown,(ni::tU32)ePointerButton_Left);
        }
        break;
      }
      case WM_NCLBUTTONUP:
      case WM_LBUTTONUP: {
        if (_CanSendMouseMessage()) {
          _HandleMouseMove(lParam);
          mStaticManager.mbMouseDown[ePointerButton_Left] = eFalse;
          _PostMessage(eOSWindowMessage_MouseButtonUp,(ni::tU32)ePointerButton_Left);
          _SetCursorCapture(CURSOR_CAPTURE_L, eFalse);
        }
        break;
      }
      case WM_LBUTTONDBLCLK: {
        if (_CanSendMouseMessage()) {
          _HandleMouseMove(lParam);
          //  Double-clicking the left mouse button actually
          //  generates a sequence of four messages: WM_LBUTTONDOWN,
          //  WM_LBUTTONUP, WM_LBUTTONDBLCLK, and WM_LBUTTONUP.
          // So, we add a down as-well, cause we want, down/up, down, dblclick, up
          _PostMessage(eOSWindowMessage_MouseButtonDown,(ni::tU32)ePointerButton_Left);
          _PostMessage(eOSWindowMessage_MouseButtonDoubleClick,(ni::tU32)ePointerButton_Left);
        }
        break;
      }
      case WM_RBUTTONDOWN: {
        if (_CanSendMouseMessage()) {
          this->SetFocus();
          _HandleMouseMove(lParam);
          mStaticManager.mbMouseDown[ePointerButton_Right] = eTrue;
          _SetCursorCapture(CURSOR_CAPTURE_R, eTrue);
          _PostMessage(eOSWindowMessage_MouseButtonDown,(ni::tU32)ePointerButton_Right);
        }
        break;
      }
      case WM_NCRBUTTONUP:
      case WM_RBUTTONUP: {
        if (_CanSendMouseMessage()) {
          _HandleMouseMove(lParam);
          mStaticManager.mbMouseDown[ePointerButton_Right] = eFalse;
          _PostMessage(eOSWindowMessage_MouseButtonUp,(ni::tU32)ePointerButton_Right);
          _SetCursorCapture(CURSOR_CAPTURE_R, eFalse);
        }
        break;
      }
      case WM_RBUTTONDBLCLK: {
        if (_CanSendMouseMessage()) {
          _HandleMouseMove(lParam);
          // see Left button handling for details as to why we have one more down posted msg here
          _PostMessage(eOSWindowMessage_MouseButtonDown,(ni::tU32)ePointerButton_Right);
          _PostMessage(eOSWindowMessage_MouseButtonDoubleClick,(ni::tU32)ePointerButton_Right);
        }
        break;
      }
      case WM_MBUTTONDOWN: {
        if (_CanSendMouseMessage()) {
          this->SetFocus();
          _HandleMouseMove(lParam);
          mStaticManager.mbMouseDown[ePointerButton_Middle] = eTrue;
          _SetCursorCapture(CURSOR_CAPTURE_M, eTrue);
          _PostMessage(eOSWindowMessage_MouseButtonDown,(ni::tU32)ePointerButton_Middle);
        }
        break;
      }
      case WM_NCMBUTTONUP:
      case WM_MBUTTONUP: {
        if (_CanSendMouseMessage()) {
          _HandleMouseMove(lParam);
          mStaticManager.mbMouseDown[ePointerButton_Middle] = eFalse;
          _SetCursorCapture(CURSOR_CAPTURE_M, eFalse);
          _PostMessage(eOSWindowMessage_MouseButtonUp,(ni::tU32)ePointerButton_Middle);
        }
        break;
      }
      case WM_MBUTTONDBLCLK: {
        if (_CanSendMouseMessage()) {
          _HandleMouseMove(lParam);
          // see Left button handling for details as to why we have one more down posted msg here
          _PostMessage(eOSWindowMessage_MouseButtonDown,(ni::tU32)ePointerButton_Middle);
          _PostMessage(eOSWindowMessage_MouseButtonDoubleClick,(ni::tU32)ePointerButton_Middle);
        }
        break;
      }

      case WM_MOUSEWHEEL: {
        this->SetFocus();
        if (_CanSendMouseMessage()) {
          _HandleNCMouseMove(lParam);
        }
        if (!mbUseWMInput) {
          tF32 v = (tF32)((short)HIWORD(wParam));
          v /= (tF32)WHEEL_DELTA;
          _SendMouseWheel(v);
          // niDebugFmt(("... WM_MOUSEWHEEL: %f", v));
        }
        return 0;
      }

#ifndef NO_RAWINPUT
      case WM_INPUT: {
        // niDebugFmt((_A("WM_INPUT: %d"),mbUseWMInput));
        if (mbUseWMInput) {
          BYTE lpb[128];
          UINT dwSize = 0;
          GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize,
                          sizeof(RAWINPUTHEADER));
          if (dwSize > 0 && dwSize < sizeof(lpb)) {
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT,
                                lpb, &dwSize, sizeof(RAWINPUTHEADER)) != (UINT)-1)
            {
              RAWINPUT* raw = (RAWINPUT*)lpb;
              if (raw->header.dwType == RIM_TYPEMOUSE) {
                if (_CanSendMouseMessage()) {
                  if (niFlagIs(raw->data.mouse.usFlags, MOUSE_MOVE_ABSOLUTE)) {
                    const ni::tBool isVirtualDesktop = niFlagIs(raw->data.mouse.usFlags, MOUSE_VIRTUAL_DESKTOP);

                    const ni::tI32 width = GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
                    const ni::tI32 height = GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);

                    const ni::tI32 absoluteX = ni::tI32((raw->data.mouse.lLastX / 65535.0f) * width);
                    const ni::tI32 absoluteY = ni::tI32((raw->data.mouse.lLastY / 65535.0f) * height);

                    const sVec2i vMousePos = Vec2(absoluteX,absoluteY);
                    if (mvRawInputAbsRelPos.x != eInvalidHandle) {
                      const sVec2i vRelMove = vMousePos-mvRawInputAbsRelPos;
                      _PostMessage(eOSWindowMessage_RelativeMouseMove,vRelMove);
                    }
                    mvRawInputAbsRelPos = vMousePos;
                  }
                  else if (raw->data.mouse.lLastX != 0 || raw->data.mouse.lLastY != 0) {
                    ni::tI32 xPosRelative = raw->data.mouse.lLastX;
                    ni::tI32 yPosRelative = raw->data.mouse.lLastY;
                    // niDebugFmt((_A("WM_INPUT_MOUSEMOVEREL[%d]: %d,%d"),raw->data.mouse.usFlags,xPosRelative,yPosRelative));
                    _PostMessage(eOSWindowMessage_RelativeMouseMove,
                                 Vec2(xPosRelative,yPosRelative));
                  }
                }
                if (raw->data.mouse.usButtonFlags&RI_MOUSE_WHEEL) {
                  this->SetFocus();
                  tF32 v = (tF32)(SHORT)raw->data.mouse.usButtonData;
                  v /= (tF32)WHEEL_DELTA;
                  // niDebugFmt((_A("WM_INPUT_MOUSEWHEEL: %d, %f"),raw->data.mouse.usButtonData,v));
                  _SendMouseWheel(v);
                }
              }
            }
            else {
              // niDebugFmt(("WM_INPUT GetRawInputData Error"));
            }
          }
          else {
            // niDebugFmt(("WM_INPUT Invalid dwSize: %d", dwSize));
          }
        }
        break;
      }
#endif

#if !defined NO_POINTEREVENT
      case WM_POINTERDOWN:
      case WM_POINTERUP:
      case WM_POINTERUPDATE: {
        POINTER_INPUT_TYPE pointerType = PT_TOUCH;
        UINT32 pointerId = GET_POINTERID_WPARAM(wParam);
        if ((!_pfnGetPointerType || _pfnGetPointerType(pointerId, &pointerType))
            && (pointerType == PT_TOUCH || pointerType == PT_PEN))
        {
          POINT p;
          p.x = ((int)(short)LOWORD(lParam));
          p.y = ((int)(short)HIWORD(lParam));
          ::ScreenToClient(mHandle,(LPPOINT)&p);
          TRACE_POINTER((
              "=== WINMSG === WM_POINTER%s[%d:%d]: %d,%d",
              (message == WM_POINTERDOWN) ? "DOWN" : ((message == WM_POINTERUP) ? "UP" : "UPDATE"),
              (tU32)pointerType,
              GET_POINTERID_WPARAM(wParam),
              p.x, p.y));
          switch (message) {
            case WM_POINTERDOWN: {
              mFingerHandler.FingerDown(mptrMT,pointerId,Vec2i(p.x,p.y));
              break;
            }
            case WM_POINTERUP: {
              mFingerHandler.FingerUp(mptrMT,pointerId,Vec2i(p.x,p.y));
              break;
            }
            case WM_POINTERUPDATE: {
              mFingerHandler.FingerMove(mptrMT,pointerId,Vec2i(p.x,p.y));
              break;
            }
          }

          // So that windows doesn't send any WM_MOUSEMOVE, WM_LBUTTONDOWN, etc... for the touch input
          return 0;
        }
        break;
      }
#endif

      case WM_DROPFILES:
        {
          HDROP hDrop = (HDROP)wParam;
          UINT nNumFiles = DragQueryFile(hDrop,0xFFFFFFFF,NULL,0);
          if (nNumFiles) {
            sVec2i dropPos;
            DragQueryPoint(hDrop,(LPPOINT)&dropPos);
            achar szFileName[2048] = {0,0,0,0,0,0,0,0};
            Ptr<tStringCVec> vFiles = tStringCVec::Create();
            vFiles->Reserve(nNumFiles);
            niLoop(i,nNumFiles) {
              DragQueryFile(hDrop,i,szFileName,niCountOf(szFileName));
              if (szFileName[0]) {
                vFiles->Add(szFileName);
              }
            }
            _PostMessage(eOSWindowMessage_Drop,vFiles.ptr(),dropPos);
          }
          DragFinish(hDrop);
          break;
        }

      case MY_WM_SETCAPTURE: {
        if (mptrWindowThread.IsOK()) {
          RECT rc;
          ::GetClientRect(mHandle,&rc);
          ::ClientToScreen(mHandle,(LPPOINT)&rc.left);
          ::ClientToScreen(mHandle,(LPPOINT)&rc.right);
          ::ClipCursor(&rc);
        }
        else {
          ::SetCapture((HWND)hWnd);
        }
        break;
      }
      case MY_WM_RELEASECAPTURE: {
        if (mptrWindowThread.IsOK()) {
          ::ClipCursor(NULL);
        }
        else {
          ::ReleaseCapture();
        }
        break;
      }
      case MY_WM_DPICHANGED: {
        // TODO: We actually never receive this message... Windows just won't
        //       cooperate, maybe its the fact that there's an OpenGL context
        //       attached to the window?
        UINT uDpi = HIWORD(wParam);
        _UpdateContentsScale("WM_DPICHANGED", uDpi);
        break;
      }
      default:
        if (message == uMSH_MOUSEWHEEL) {
          tF32 v = (tF32)(((int)wParam < 0) ? -1 : +1);
          _SendMouseWheel(v);
          //
          // Do other wheel stuff...
          //
          break;
        }
        break;
    }

    if (mPrevWindowProc) {
      return niWin32API(CallWindowProc)(mPrevWindowProc,(HWND)hWnd,message,wParam,lParam);
    }
    else {
      return niWin32API(DefWindowProc)((HWND)hWnd,message,wParam,lParam);
    }
  }

  niEndClass(cOSWindowWindows);
};

static LRESULT CALLBACK _StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  cOSWindowWindows* pWnd = reinterpret_cast<cOSWindowWindows*>(
      (tIntPtr)GetWindowLongPtr(hWnd,GWLP_USERDATA));
  if (pWnd) {
    return pWnd->WndProc((tIntPtr)hWnd, message, wParam, lParam);
  }
  else {
    return niWin32API(DefWindowProc)(hWnd,message,wParam,lParam);
  }
}

//--------------------------------------------------------------------------------------------
//
//  cLang implementation
//
//--------------------------------------------------------------------------------------------
static BOOL CALLBACK _MonitorEnumProc(
    HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

struct sMonitor {
  tIntPtr   mHandle;
  cString   mstrName;
  sRecti mRect;
  tOSMonitorFlags mFlags;
};
static astl::vector<sMonitor> _vMonitors;
static astl::vector<cOSWindowWindows*> _vWindows;
static ni::tBool _bInitializedGameCtrls = eFalse;

void cLang::_PlatformStartup() {
  _SetDPIAware();
#ifdef USE_IDROPTARGET
  ::OleInitialize(NULL);
#endif
  uMSH_MOUSEWHEEL = RegisterWindowMessage(MSH_MOUSEWHEEL);
}

///////////////////////////////////////////////
static void _EnumMonitors() {
  static ni::tBool _bEnumedDisplays = eFalse;
  if (!_bEnumedDisplays) {
    _bEnumedDisplays = eTrue;
    ::EnumDisplayMonitors(NULL,NULL,_MonitorEnumProc,0);
  }
}

tU32 __stdcall cLang::GetNumMonitors() const {
  _EnumMonitors();
  return (tU32)_vMonitors.size();
}
tU32 __stdcall cLang::GetMonitorIndex(tIntPtr aHandle) const {
  _EnumMonitors();
  niLoop(i,_vMonitors.size()) {
    if (_vMonitors[i].mHandle == aHandle)
      return (tU32)i;
  }
  return eInvalidHandle;
}
tIntPtr __stdcall cLang::GetMonitorHandle(tU32 anIndex) const {
  _EnumMonitors();
  niCheckSilent(anIndex<_vMonitors.size(),eInvalidHandle);
  return _vMonitors[anIndex].mHandle;
}
const achar* __stdcall cLang::GetMonitorName(tU32 anIndex) const {
  _EnumMonitors();
  niCheckSilent(anIndex<_vMonitors.size(),NULL);
  return _vMonitors[anIndex].mstrName.Chars();
}
sRecti __stdcall cLang::GetMonitorRect(tU32 anIndex) const {
  _EnumMonitors();
  niCheckSilent(anIndex<_vMonitors.size(),sRecti::Null());
  return _vMonitors[anIndex].mRect;
}
tOSMonitorFlags __stdcall cLang::GetMonitorFlags(tU32 anIndex) const {
  _EnumMonitors();
  niCheckSilent(anIndex<_vMonitors.size(),0);
  return _vMonitors[anIndex].mFlags;
}

///////////////////////////////////////////////
iOSWindow* __stdcall cLang::CreateWindow(iOSWindow* apParent, const achar* aaszTitle, const sRecti& aRect, tOSWindowCreateFlags aCreate, tOSWindowStyleFlags aStyle) {
  iOSWindow* caParent = NULL;
  tOSWindowStyleFlags caStyle = 0;
  if (aStyle&eOSWindowStyleFlags_ClientArea) {
    // We have to do this so that a ClientArea window in a hosted process still
    // gets the IME initialized for it.
    caParent = apParent;
    caStyle = aStyle;
    apParent = NULL;
    // Use _Regular, _Overlay creates a wierd border issue when embedded later on...
    aStyle = eOSWindowStyleFlags_Regular|eOSWindowStyleFlags_NoTitle;
  }

  HWND parent = niIsOK(apParent) ? (HWND)apParent->GetHandle() : NULL;

  Ptr<cOSWindowWindows> ptrWindow = niNew cOSWindowWindows(down_cast<iOSWindow*>(apParent));
  if (niFlagIsNot(aCreate,eOSWindowCreateFlags_NoThread)) {
    Ptr<cThreadedWindow> ptrWindowThread = niNew cThreadedWindow(ptrWindow,parent,aStyle);
    if (!ptrWindowThread.IsOK()) {
      niError(_A("Can't create the new OS window's thread."));
      return NULL;
    }
    if (!ptrWindowThread->GetHWND()) {
      niError(_A("Can't create get the new OS window."));
      return NULL;
    }
    if (!ptrWindow->_SetWindowHandle(ptrWindowThread->GetHWND(),eTrue,eTrue,ptrWindowThread)) {
      niError(_A("Can't setup the window's handle."));
      return NULL;
    }
  }
  else {
    HWND hWnd = _CreateWindow(parent,aStyle);
    if (!hWnd) {
      niError(_A("Can't create the new OS window."));
      return NULL;
    }
    if (!ptrWindow->_SetWindowHandle(hWnd,eTrue,eTrue,NULL)) {
      niError(_A("Can't setup the window's handle."));
      ::DestroyWindow(hWnd);
      return NULL;
    }
  }

  ptrWindow->SetTitle(aaszTitle);
  if (aRect != sRecti::Null()) {
    ptrWindow->SetRect(aRect);
  }

  if (caParent) {
    ptrWindow->SetStyle(caStyle);
    ::SetParent(
         (HWND)ptrWindow->GetHandle(),
         (HWND)caParent->GetHandle());

    // This, again (isn't Windows wonderfull ?), is needed so that the
    // child window has not title bar on some XP systems...
    HWND ca = (HWND)ptrWindow->GetHandle();
    sRecti rect;
    ::GetClientRect(ca,(LPRECT)&rect);
    rect.Inflate(sVec2i::One());
    ::SetWindowPos(ca,NULL,
                   rect.GetLeft(),rect.GetTop(),
                   rect.GetWidth(),rect.GetHeight(),
                   SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
    rect.Inflate(-sVec2i::One());
    ::SetWindowPos(ca,NULL,
                   rect.GetLeft(),rect.GetTop(),
                   rect.GetWidth(),rect.GetHeight(),
                   SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
  }

  return ptrWindow.GetRawAndSetNull();
}

///////////////////////////////////////////////
iOSWindow* __stdcall cLang::CreateWindowEx(tIntPtr aOSWindowHandle, tOSWindowCreateFlags aCreate) {
  Ptr<cOSWindowWindows> ptrWindow = niNew cOSWindowWindows(NULL);

  if (!ptrWindow->_SetWindowHandle(
          (HWND)aOSWindowHandle,
          niFlagIs(aCreate,eOSWindowCreateFlags_OwnHandle),
          niFlagIs(aCreate,eOSWindowCreateFlags_SetWindowProc),
          NULL))
  {
    niError(_A("Can't setup the window's handle."));
    if (niFlagIs(aCreate,eOSWindowCreateFlags_OwnHandle)) {
      ::DestroyWindow((HWND)aOSWindowHandle);
    }
    return NULL;
  }

  achar wndTitle[4096] = {0};
  ::GetWindowText((HWND)aOSWindowHandle,wndTitle,niCountOf(wndTitle));
  ptrWindow->mstrTitle = wndTitle;

  return ptrWindow.GetRawAndSetNull();
}

///////////////////////////////////////////////
eOSMessageBoxReturn __stdcall cLang::MessageBox(iOSWindow* apParent, const achar* aaszTitle, const achar* aaszText, tOSMessageBoxFlags aFlags)
{
  HWND parent = niIsOK(apParent) ? (HWND)apParent->GetHandle() : NULL;
  UINT type = MB_SETFOREGROUND|MB_TASKMODAL;

  if (niFlagIs(aFlags,eOSMessageBoxFlags_OkCancel)) {
    type |= MB_OKCANCEL;
  }
  else if (niFlagIs(aFlags,eOSMessageBoxFlags_YesNo)) {
    type |= MB_YESNO;
  }
  else /*if (niFlagIs(aFlags,eOSMessageBoxFlags_Ok))*/ {
    type |= MB_OK;
  }

  if (niFlagIs(aFlags,eOSMessageBoxFlags_IconInfo)) {
    type |= MB_ICONINFORMATION;
  }
  else if (niFlagIs(aFlags,eOSMessageBoxFlags_IconQuestion)) {
    type |= MB_ICONQUESTION;
  }
  else if (niFlagIs(aFlags,eOSMessageBoxFlags_IconWarning)) {
    type |= MB_ICONWARNING;
  }
  else if (niFlagIs(aFlags,eOSMessageBoxFlags_IconError)) {
    type |= MB_ICONERROR;
  }
  else if (niFlagIs(aFlags,eOSMessageBoxFlags_IconHand)) {
    type |= MB_ICONHAND;
  }
  else if (niFlagIs(aFlags,eOSMessageBoxFlags_IconStop)) {
    type |= MB_ICONSTOP;
  }

  if (niFlagIsNot(aFlags,eOSMessageBoxFlags_NotTopMost)) {
    type |= MB_TOPMOST;
  }

  cString strTitle = niIsStringOK(aaszTitle)?aaszTitle:"Message";
  cString strText = niIsStringOK(aaszText)?aaszText:_A("");

  ni::Windows::UTF16Buffer wText;
  niWin32_UTF8ToUTF16(wText,strText.Chars());
  ni::Windows::UTF16Buffer wTitle;
  niWin32_UTF8ToUTF16(wTitle,strTitle.Chars());

  int ret = ::MessageBoxW(parent,wText.data(),wTitle.data(),type);
  switch (ret) {
    case IDABORT:
    case IDCANCEL:
    case IDNO:
      return eOSMessageBoxReturn_No;
#ifdef IDCONTINUE
    case IDCONTINUE:
#endif
#ifdef IDRETRY
    case IDRETRY:
#endif
#ifdef IDTRYAGAIN
    case IDTRYAGAIN:
#endif
    case IDIGNORE:
    case IDOK:
    case IDYES:
      return eOSMessageBoxReturn_Yes;
  }

  return eOSMessageBoxReturn_Error;
}

struct sOpenFileNameDialog {
  OPENFILENAMEW ofn;
  WCHAR wszFileNameOut[_MAX_PATH] = {0};
  ni::Windows::UTF16Buffer wTitle;
  ni::Windows::UTF16Buffer wDefExt;
  ni::Windows::UTF16Buffer wFilter;
  ni::Windows::UTF16Buffer wInitDir;

  sOpenFileNameDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir)
  {
    ZeroMemory(wszFileNameOut,sizeof(wszFileNameOut));
    ZeroMemory(&ofn,sizeof(ofn));
    ofn.lStructSize   = sizeof(ofn);
    ofn.nMaxFile      = _MAX_PATH;
    ofn.nMaxFileTitle = _MAX_PATH;
    ofn.lpstrFile     = wszFileNameOut;
    ofn.Flags         = 0;

    if (aParent) {
      ofn.hwndOwner = (HWND)aParent->GetHandle();
    }

    if (niStringIsOK(aTitle)) {
      niWin32_UTF8ToUTF16(wTitle,aTitle);
      ofn.lpstrTitle = wTitle.data();
    }

    if (niStringIsOK(aFilter)) {
      cString filter;
      tBool allowAllExts = eFalse;
      astl::vector<cString> exts;
      StringSplit(_ASTR(aFilter), ";", &exts);
      niLoop(i, exts.size()) {
        if (exts[i].IEq("*")) {
          allowAllExts = eTrue;
          filter.append("All Files");
          filter.appendChar(0);
          filter.append("*.*");
          filter.appendChar(0);
        }
        else {
          if (wDefExt.mpData == NULL) {
            niWin32_UTF8ToUTF16(wDefExt,exts[i].Chars());
            ofn.lpstrDefExt = wDefExt.data();
          }
          filter.append(exts[i].Chars());
          filter.appendChar(0);
          filter.append("*.");
          filter.append(exts[i].Chars());
          filter.appendChar(0);
        }
      }
      filter.appendChar(0);
      ni::Windows::ConvertUTF8ToUTF16(wFilter,filter.Chars(),filter.size());
      ofn.lpstrFilter = wFilter.data();
    }

    if (niStringIsOK(aInitDir)) {
      niWin32_UTF8ToUTF16(wInitDir,aInitDir);
    }
    else {
      niWin32_UTF8ToUTF16(wInitDir,GetProperty("ni.dirs.home").Chars());
    }
    ofn.lpstrInitialDir = wInitDir.data();
  }

  cString GetFileName() const {
    return wszFileNameOut;
  }
};

///////////////////////////////////////////////
cString __stdcall cLang::OpenFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  sOpenFileNameDialog ofn(aParent, aTitle, aFilter, aInitDir);
  if (::GetOpenFileNameW(&ofn.ofn) != TRUE) {
    return "";
  }
  return cPath(ofn.GetFileName()).GetPath();
}

///////////////////////////////////////////////
cString __stdcall cLang::SaveFileDialog(iOSWindow* aParent, const achar* aTitle, const achar* aFilter, const achar* aInitDir) {
  sOpenFileNameDialog ofn(aParent, aTitle, aFilter, aInitDir);
  if (::GetSaveFileNameW(&ofn.ofn) != TRUE) {
    return "";
  }
  return cPath(ofn.GetFileName()).GetPath();
}

///////////////////////////////////////////////
INT CALLBACK _BrowseForFolderCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) {
  if (uMsg == BFFM_INITIALIZED) {
    ::SendMessageW(hwnd, BFFM_SETSELECTIONW, TRUE, pData);
  }
  return 0;
}

///////////////////////////////////////////////
cString __stdcall cLang::PickDirectoryDialog(iOSWindow* aParent, const achar* aTitle, const achar* aInitDir) {
  BOOL bRet = FALSE;

  BROWSEINFOW bi;
  ZeroMemory(&bi,sizeof(bi));
  bi.iImage = 0;
  bi.ulFlags = BIF_USENEWUI; //BIF_RETURNONLYFSDIRS;

  WCHAR szOutPath[_MAX_PATH] = {0};
  bi.pszDisplayName = szOutPath;

  if (aParent) {
    bi.hwndOwner = (HWND)aParent->GetHandle();
  }

  ni::Windows::UTF16Buffer wTitle;
  niWin32_UTF8ToUTF16(wTitle,aTitle);
  bi.lpszTitle = wTitle.data();

#if 0
  // Note: This doesn't seem to work (anymore?) on Windows 10. Disabled since
  // I can't verify that it works and I don't want it to risk this crashing at
  // a later point.
  ni::Windows::UTF16Buffer wInitDir;
  if (niStringIsOK(aInitDir)) {
    niWin32_UTF8ToUTF16(wInitDir,aInitDir);
    bi.lpfn = _BrowseForFolderCallbackProc;
    bi.lParam = (LPARAM)wInitDir.data();
  }
#endif

  PIDLIST_ABSOLUTE pIDList = ::SHBrowseForFolderW(&bi);
  if (pIDList != NULL) {
    if (::SHGetPathFromIDListW(pIDList, szOutPath))
      bRet = TRUE;

    LPMALLOC pIMalloc = NULL;
    if (SUCCEEDED(SHGetMalloc(&pIMalloc)) && pIMalloc) {
      pIMalloc->Free(pIDList);
      pIMalloc->Release();
    }
  }

  if (bRet) {
    cString r = szOutPath;
    StrMakeStdPath(r.data());
    return r;
  }

  return "";
}

///////////////////////////////////////////////
static ThreadMutex _mutexWindows;
static void _RegisterWindow(cOSWindowWindows* apWin) {
  AutoThreadLock lock(_mutexWindows);
  _vWindows.push_back(apWin);
}
static void _UnregisterWindow(cOSWindowWindows* apWin) {
  AutoThreadLock lock(_mutexWindows);
  astl::find_erase(_vWindows,apWin);
}
static void _UnstickWindows() {
  AutoThreadLock lock(_mutexWindows);
  niLoop(i,_vWindows.size()) {
    cOSWindowWindows* pWin = _vWindows[i];
    if (pWin->GetHandle()) {
      niWin32API(PostMessage)((HWND)pWin->GetHandle(),MY_WM_UNSTICK,0,0);
    }
  }
}

///////////////////////////////////////////////
ni::tU32 __stdcall cLang::GetNumGameCtrls() const {
  _InitGameCtrls();
  switch (_knGameCtrlType) {
    case GAMECTRL_XINPUT:
      return XInputGameCtrls_GetNumGameCtrls();
    case GAMECTRL_DINPUT:
      return DInputGameCtrls_GetNumGameCtrls();
  }
  return 0;
}
iGameCtrl* __stdcall cLang::GetGameCtrl(tU32 anIndex) const {
  _InitGameCtrls();
  switch (_knGameCtrlType) {
    case GAMECTRL_XINPUT:
      return XInputGameCtrls_GetGameCtrl(anIndex);
    case GAMECTRL_DINPUT:
      return DInputGameCtrls_GetGameCtrl(anIndex);
  }
  return NULL;
}

///////////////////////////////////////////////
static BOOL CALLBACK _MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
  MONITORINFOEX monInfos;
  ni::MemZero((tPtr)&monInfos,sizeof(monInfos));
  monInfos.cbSize = sizeof(monInfos);
  if (GetMonitorInfo(hMonitor,&monInfos)) {
    sMonitor mon;
    ni::MemZero((tPtr)&mon,sizeof(mon));
    mon.mHandle = (tIntPtr)hMonitor;
    mon.mRect = (sRecti&)monInfos.rcMonitor;
    niFlagOnIf(mon.mFlags, eOSMonitorFlags_Primary, (monInfos.dwFlags & MONITORINFOF_PRIMARY));
    mon.mstrName = monInfos.szDevice;
    _vMonitors.push_back(mon);
    //      niDebugFmt((_A("Found monitor : %s, %p, (%d,%d,%d,%d)\n"),mon.mstrName.Chars(),mon.mHandle,mon.mRect.GetLeft(),mon.mRect.GetTop(),mon.mRect.GetWidth(),mon.mRect.GetHeight()));
  }
  return TRUE;
}

static STICKYKEYS mStartupStickyKeys;
static TOGGLEKEYS mStartupToggleKeys;
static FILTERKEYS mStartupFilterKeys;
void _AllowAccessibilityShortcutKeys( bool bAllowKeys )
{
  if (bAllowKeys) {
    // Restore StickyKeys/etc to original state and enable Windows key
    SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &mStartupStickyKeys, 0);
    SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &mStartupToggleKeys, 0);
    SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &mStartupFilterKeys, 0);
  }
  else {
    // Disable StickyKeys/etc shortcuts but if the accessibility feature is on,
    // then leave the settings alone as its probably being usefully used
    STICKYKEYS skOff = mStartupStickyKeys;
    if ((skOff.dwFlags & SKF_STICKYKEYSON) == 0) {
      // Disable the hotkey and the confirmation
      skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
      skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;

      SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
    }

    TOGGLEKEYS tkOff = mStartupToggleKeys;
    if ((tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0) {
      // Disable the hotkey and the confirmation
      tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
      tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;

      SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
    }

    FILTERKEYS fkOff = mStartupFilterKeys;
    if ((fkOff.dwFlags & FKF_FILTERKEYSON) == 0) {
      // Disable the hotkey and the confirmation
      fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
      fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;

      SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
    }
  }
}
#endif
