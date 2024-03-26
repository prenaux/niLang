#pragma once
#ifndef __IOSWINDOW_H_6D7C42F3_D6B8_41FD_8AF2_0641803C9F5A__
#define __IOSWINDOW_H_6D7C42F3_D6B8_41FD_8AF2_0641803C9F5A__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/IConcurrent.h>
#include <niLang/Math/MathRect.h>
#include <niLang/Utils/SinkList.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */

//! Key codes
enum eKey
{
  eKey_Unknown      = 0x00,
  eKey_Escape       = 0x01,
  eKey_n1           = 0x02,
  eKey_n2           = 0x03,
  eKey_n3           = 0x04,
  eKey_n4           = 0x05,
  eKey_n5           = 0x06,
  eKey_n6           = 0x07,
  eKey_n7           = 0x08,
  eKey_n8           = 0x09,
  eKey_n9           = 0x0A,
  eKey_n0           = 0x0B,
  eKey_Minus        = 0x0C,             //! - on main keyboard
  eKey_Equals       = 0x0D,
  eKey_BackSpace    = 0x0E,             //! backspace
  eKey_Tab          = 0x0F,
  eKey_Q            = 0x10,
  eKey_W            = 0x11,
  eKey_E            = 0x12,
  eKey_R            = 0x13,
  eKey_T            = 0x14,
  eKey_Y            = 0x15,
  eKey_U            = 0x16,
  eKey_I            = 0x17,
  eKey_O            = 0x18,
  eKey_P            = 0x19,
  eKey_LBracket     = 0x1A,
  eKey_RBracket     = 0x1B,
  eKey_Enter        = 0x1C,             //! Enter on main keyboard
  eKey_LControl     = 0x1D,
  eKey_A            = 0x1E,
  eKey_S            = 0x1F,
  eKey_D            = 0x20,
  eKey_F            = 0x21,
  eKey_G            = 0x22,
  eKey_H            = 0x23,
  eKey_J            = 0x24,
  eKey_K            = 0x25,
  eKey_L            = 0x26,
  eKey_Semicolon    = 0x27,
  eKey_Apostrophe   = 0x28,
  eKey_Grave        = 0x29,             //! accent grave
  eKey_Tilde        = 0x29,             //! accent grave
  eKey_LShift       = 0x2A,
  eKey_BackSlash    = 0x2B,
  eKey_Z            = 0x2C,
  eKey_X            = 0x2D,
  eKey_C            = 0x2E,
  eKey_V            = 0x2F,
  eKey_B            = 0x30,
  eKey_N            = 0x31,
  eKey_M            = 0x32,
  eKey_Comma        = 0x33,
  eKey_Period       = 0x34,             //! . on main keyboard
  eKey_Slash        = 0x35,             //! / on main keyboard
  eKey_RShift       = 0x36,
  eKey_NumPadStar   = 0x37,             //! * on numeric keypad
  eKey_LAlt         = 0x38,             //! left Alt
  eKey_Space        = 0x39,
  eKey_CapsLock     = 0x3A,
  eKey_F1           = 0x3B,
  eKey_F2           = 0x3C,
  eKey_F3           = 0x3D,
  eKey_F4           = 0x3E,
  eKey_F5           = 0x3F,
  eKey_F6           = 0x40,
  eKey_F7           = 0x41,
  eKey_F8           = 0x42,
  eKey_F9           = 0x43,
  eKey_F10          = 0x44,
  eKey_NumLock      = 0x45,
  eKey_Scroll       = 0x46,             //! Scroll Lock
  eKey_NumPad7      = 0x47,
  eKey_NumPad8      = 0x48,
  eKey_NumPad9      = 0x49,
  eKey_NumPadMinus  = 0x4A,             //! - on numeric keypad
  eKey_NumPad4      = 0x4B,
  eKey_NumPad5      = 0x4C,
  eKey_NumPad6      = 0x4D,
  eKey_NumPadPlus   = 0x4E,             //! + on numeric keypad
  eKey_NumPad1      = 0x4F,
  eKey_NumPad2      = 0x50,
  eKey_NumPad3      = 0x51,
  eKey_NumPad0      = 0x52,
  eKey_NumPadPeriod = 0x53,             //! . on numeric keypad
  eKey_OEM_102      = 0x56,             //! <> or \| on RT 102-key keyboard (Non-U.S.)
  eKey_F11          = 0x57,
  eKey_F12          = 0x58,
  eKey_F13          = 0x64,             //!            (NEC PC98)
  eKey_F14          = 0x65,             //!            (NEC PC98)
  eKey_F15          = 0x66,             //!            (NEC PC98)
  eKey_Kana         = 0x70,             //! (Japanese keyboard)
  eKey_ABNT_C1      = 0x73,             //! /? on Brazilian keyboard
  eKey_Convert      = 0x79,             //! (Japanese keyboard)
  eKey_NoConvert    = 0x7B,             //! (Japanese keyboard)
  eKey_Yen          = 0x7D,             //! (Japanese keyboard)
  eKey_ABNT_C2      = 0x7E,             //! Numpad . on Brazilian keyboard
  eKey_NumPadEquals = 0x8D,             //! = on numeric keypad (NEC PC98)
  eKey_Circumflex   = 0x90,             //! Previous Track (eKey_CIRCUMFLEX on Japanese keyboard)
  eKey_PrevTrack    = 0x90,             //! Previous Track (eKey_CIRCUMFLEX on Japanese keyboard)
  eKey_AT           = 0x91,             //!            (NEC PC98)
  eKey_Colon        = 0x92,             //!            (NEC PC98)
  eKey_Underline    = 0x93,             //!            (NEC PC98)
  eKey_Kanji        = 0x94,             //! (Japanese keyboard)
  eKey_Stop         = 0x95,             //!            (NEC PC98)
  eKey_AX           = 0x96,             //!            (Japan AX)
  eKey_Unlabeled    = 0x97,             //!             (J3100)
  eKey_NextTrack    = 0x99,             //! Next Track
  eKey_NumPadEnter  = 0x9C,             //! Enter on numeric keypad
  eKey_RControl     = 0x9D,
  eKey_Mute         = 0xA0,             //! Mute
  eKey_Calculator   = 0xA1,             //! Calculator
  eKey_PlayPause    = 0xA2,             //! Play / Pause
  eKey_MediaStop    = 0xA4,             //! Media Stop
  eKey_VolumeDown   = 0xAE,             //! Volume -
  eKey_VolumeUp     = 0xB0,             //! Volume +
  eKey_WebHome      = 0xB2,             //! Web home
  eKey_NumPadComma  = 0xB3,             //! , on numeric keypad (NEC PC98)
  eKey_NumPadSlash  = 0xB5,             //! / on numeric keypad
  eKey_SysRQ        = 0xB7,
  eKey_PrintScreen  = 0xB7,
  eKey_RAlt         = 0xB8,             //! right Alt
  eKey_AltGr        = 0xB8,             //! right Alt
  eKey_Pause        = 0xC5,             //! Pause
  eKey_Home         = 0xC7,             //! Home on arrow keypad
  eKey_Up           = 0xC8,             //! UpArrow on arrow keypad
  eKey_PgUp         = 0xC9,             //! PgUp on arrow keypad
  eKey_Left         = 0xCB,             //! LeftArrow on arrow keypad
  eKey_Right        = 0xCD,             //! RightArrow on arrow keypad
  eKey_End          = 0xCF,             //! End on arrow keypad
  eKey_Down         = 0xD0,             //! DownArrow on arrow keypad
  eKey_PgDn         = 0xD1,             //! PgDn on arrow keypad
  eKey_Insert       = 0xD2,             //! Insert on arrow keypad
  eKey_Delete       = 0xD3,             //! Delete on arrow keypad
  eKey_LWin         = 0xDB,             //! Left Windows key
  eKey_RWin         = 0xDC,             //! Right Windows key
  eKey_Apps         = 0xDD,             //! AppMenu key
  eKey_Power        = 0xDE,             //! System Power
  eKey_Sleep        = 0xDF,             //! System Sleep
  eKey_Wake         = 0xE3,             //! System Wake
  eKey_WebSearch    = 0xE5,             //! Web Search
  eKey_WebFavorites = 0xE6,             //! Web Favorites
  eKey_WebRefresh   = 0xE7,             //! Web Refresh
  eKey_WebStop      = 0xE8,             //! Web Stop
  eKey_WebForward   = 0xE9,             //! Web Forward
  eKey_WebBack      = 0xEA,             //! Web Back
  eKey_MyComputer   = 0xEB,             //! My Computer
  eKey_Mail         = 0xEC,             //! Mail
  eKey_MediaSelect  = 0xED,             //! Media Select
  eKey_Last         = 0xFF,             //! \internal
  //! \internal
  eKey_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Key modifier
//! \remark Starts at bit 16 so that it can be or'ed with a eKey value
enum eKeyMod
{
  eKeyMod_Control    = niBit(16),
  eKeyMod_Shift      = niBit(17),
  eKeyMod_Alt        = niBit(18),
  eKeyMod_AltGr      = niBit(19),
  eKeyMod_NumLock    = niBit(20),
  eKeyMod_CapsLock   = niBit(21),
  eKeyMod_ScrollLock = niBit(22),
  //! \internal
  eKeyMod_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Pointer buttons
enum ePointerButton
{
  ePointerButton_Bt0  = 0,
  ePointerButton_Bt1  = 1,
  ePointerButton_Bt2  = 2,
  ePointerButton_Bt3  = 3,
  ePointerButton_Bt4  = 4,
  ePointerButton_Bt5  = 5,
  ePointerButton_Bt6  = 6,
  ePointerButton_Bt7  = 7,
  ePointerButton_Last = 8,

  ePointerButton_Left   = ePointerButton_Bt0,
  ePointerButton_Right  = ePointerButton_Bt1,
  ePointerButton_Middle = ePointerButton_Bt2,

  //! \internal
  ePointerButton_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Pointer axis
enum ePointerAxis
{
  //! Pointer relative X axis.
  ePointerAxis_X          = 0,
  //! Pointer relative Y axis.
  ePointerAxis_Y          = 1,
  //! Pointer relative Z axis.
  ePointerAxis_Z          = 2,
  //! Pointer absolute X/Y cursor position.
  ePointerAxis_Absolute   = 3,
  //! \internal
  ePointerAxis_Last       = 4,
  //! \internal
  ePointerAxis_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Gesture states
enum eGestureState {
  //! Unknown gesture state.
  eGestureState_Unknown = 0,
  //! The gesture began.
  eGestureState_Began = 1,
  //! The gesture updated.
  eGestureState_Updated = 2,
  //! The gesture ended.
  eGestureState_Ended = 3,
  //! The gesture was cancelled.
  eGestureState_Cancelled = 4,
  //! \internal
  eGestureState_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! OS Window messages.
enum eOSWindowMessage {
  //! The close button of the window has been pressed.
  //! \param A: unused
  //! \param B: unused
  eOSWindowMessage_Close = niMessageID('_','O','W','S','O'),
  //! The window becomes active.
  //! \param A: eOSWindowSwitchReason
  //! \param B: eKeyMod modifier detected by the OS when the app is reactivated
  eOSWindowMessage_SwitchIn = niMessageID('_','O','W','S','s'),
  //! The window becomes inactive.
  //! \param A: unused
  //! \param B: unused
  eOSWindowMessage_SwitchOut = niMessageID('_','O','W','S','S'),
  //! Something has been dropped on the window.
  //! \param A: cString collection (tStringCVec) containing the list of file paths dropped.
  //! \param B: the client position of the drop.
  eOSWindowMessage_Drop = niMessageID('_','O','W','S','r'),

  //! The client should be painted.
  //! \param A: unused
  //! \param B: unused
  eOSWindowMessage_Paint = niMessageID('_','O','W','P','p'),

  //! The window has been resized.
  //! \param A: unused
  //! \param B: unused
  eOSWindowMessage_Size = niMessageID('_','O','W','Z','z'),
  //! The widnow has been moved.
  //! \param A: unused
  //! \param B: unused
  eOSWindowMessage_Move = niMessageID('_','O','W','Z','m'),

  //! A key has been pressed.
  //! \param A: eKey code
  //! \param B: unused
  eOSWindowMessage_KeyDown = niMessageID('_','O','W','K','d'),
  //! A key has been release.
  //! \param A: eKey code
  //! \param B: unused
  eOSWindowMessage_KeyUp = niMessageID('_','O','W','K','D'),
  //! A character has been entered.
  //! \param A: 'achar' code
  //! \param B: eKey code that led to the KeyChar, 0 if none (for example when coming from IME or programmatic emulation)
  eOSWindowMessage_KeyChar = niMessageID('_','O','W','K','c'),

  //! The mouse has moved in the client area.
  //! \param A: client relative mouse position in device units
  //! \param B: unused
  eOSWindowMessage_MouseMove = niMessageID('_','O','W','I','m'),
  //! The mouse pointer has been moved.
  //! \param A: vec2 movement delta in device units.
  //! \param B: unused
  eOSWindowMessage_RelativeMouseMove = niMessageID('_','O','W','I','R'),
  //! A mouse button has been pressed in the client area.
  //! \param A: ePointerButton button ID.
  //! \param B: unused
  eOSWindowMessage_MouseButtonDown = niMessageID('_','O','W','I','d'),
  //! A mouse button has been release in the client area.
  //! \param A: ePointerButton button ID.
  //! \param B: unused
  eOSWindowMessage_MouseButtonUp = niMessageID('_','O','W','I','u'),
  //! A mouse button has been double-clicked in the client area.
  //! \param A: ePointerButton button ID.
  //! \param B: unused
  eOSWindowMessage_MouseButtonDoubleClick = niMessageID('_','O','W','I','k'),
  //! The mouse wheel has move in the client area.
  //! \param A: -1.0f if moving down, 1.0f if moving up.
  //! \param B: unused
  eOSWindowMessage_MouseWheel = niMessageID('_','O','W','I','w'),
  //! The window lost input focus.
  //! \param A: Window Handle of the window that received the focus, can be 0
  //! \param B: Process ID of the process that received the focus, can be 0
  eOSWindowMessage_LostFocus = niMessageID('_','O','W','I','f'),
  //! The window gained input focus.
  //! \param A: unused
  //! \param B: unused
  eOSWindowMessage_SetFocus = niMessageID('_','O','W','I','F'),

  //! Finger down
  //! \param A: finger index
  //! \param B: Vec3f(clientX,clientY,pressure) in device units
  eOSWindowMessage_FingerDown = niMessageID('_','O','W','F','d'),
  //! Finger up
  //! \param A: finger index
  //! \param B: Vec3f(clientX,clientY,pressure) in device units
  eOSWindowMessage_FingerUp = niMessageID('_','O','W','F','u'),
  //! Finger moved
  //! \param A: finger index
  //! \param B: Vec3f(clientX,clientY,pressure) in device units
  eOSWindowMessage_FingerMove = niMessageID('_','O','W','F','m'),
  //! Finger relative move
  //! \param A: touchId
  //! \param B: Vec3f(rel x,rel y,pressure) in device units
  eOSWindowMessage_FingerRelativeMove = niMessageID('_','O','W','F','r'),

  //! Pinch gesture.
  //! \param A: scale
  //! \param B: eGestureState
  eOSWindowMessage_Pinch = niMessageID('_','O','W','G','p'),

  //! \internal
  eOSWindowMessage_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! OS Window style.
enum eOSWindowStyleFlags
{
  //! Regular window, with a title and the regular minimize, maximized, close buttons.
  eOSWindowStyleFlags_Regular = niBit(0),
  //! Overlay window, window without borders, buttons nor title.
  eOSWindowStyleFlags_Overlay = niBit(1),
  //! Toolbox window, smaller title bar, only with a close button, doesn't show in the taskbar.
  eOSWindowStyleFlags_Toolbox = niBit(2),
  //! Ask the window to not be resizable, no resizing borders.
  eOSWindowStyleFlags_FixedSize = niBit(3),
  //! Client area covering window.
  eOSWindowStyleFlags_ClientArea = niBit(4),
  //! Never display a title bar.
  eOSWindowStyleFlags_NoTitle = niBit(5),
  //! Full bright.
  //! \remark For mobile keep the screen on and fully bright when the window is visible.
  eOSWindowStyleFlags_FullBright = niBit(6),
  //! Fullscreen window, non-resizable overlay.
  eOSWindowStyleFlags_FullScreen = eOSWindowStyleFlags_Overlay|eOSWindowStyleFlags_FixedSize,
  //! Fullscreen window, non-resizable overlay, doesn't appear in the taskbar.
  eOSWindowStyleFlags_FullScreenNoTaskBar = eOSWindowStyleFlags_Toolbox|eOSWindowStyleFlags_FixedSize|eOSWindowStyleFlags_NoTitle,
  //! \internal
  eOSWindowStyleFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
//! OS Window style flags type. \see ni::eOSWindowStyleFlags
typedef tU32 tOSWindowStyleFlags;

//! OS Window create.
enum eOSWindowCreateFlags
{
  //! The window's message pooling is done synchronously in iOSWindow::UpdateWindow().
  eOSWindowCreateFlags_NoThread = niBit(0),
  //! For CreateWindowEx, the handle passed is owned by the window object, meaning that the OS window will be destroyed when the window object is released.
  eOSWindowCreateFlags_OwnHandle = niBit(1),
  //! For CreateWindowEx, set our window proc as the window's message handler.
  eOSWindowCreateFlags_SetWindowProc = niBit(2),
  //! \internal
  eOSWindowCreateFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
//! OS Window create flags type. \see ni::eOSWindowCreateFlags
typedef tU32 tOSWindowCreateFlags;

//! OS Window show flags.
enum eOSWindowShowFlags
{
  //! Hide the window.
  eOSWindowShowFlags_Hide = niBit(0),
  //! Show the window and activate it.
  eOSWindowShowFlags_Show = niBit(1),
  //! Show the window but dont active it.
  eOSWindowShowFlags_ShowNoActivate = niBit(2),
  //! Maximize the window.
  eOSWindowShowFlags_Maximize = niBit(3),
  //! Minimize the window.
  eOSWindowShowFlags_Minimize = niBit(4),
  //! Restore the window to it's original size (after being minimized).
  eOSWindowShowFlags_Restore = niBit(5),
  //! \internal
  eOSWindowShowFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
//! OS Window show flags type. \see ni::eOSWindowShowFlags
typedef tU32 tOSWindowShowFlags;

//! OS Window ZOrder.
enum eOSWindowZOrder
{
  //! Put the window below all others.
  eOSWindowZOrder_Bottom = 0,
  //! Normal ZOrder.
  eOSWindowZOrder_Normal = 1,
  //! Put the window on top of all others.
  eOSWindowZOrder_TopMost = 2,
  //! \internal
  eOSWindowZOrder_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! OS Cursor
enum eOSCursor
{
  //! No cursor.
  eOSCursor_None = 0,
  //! Arrow cursor.
  eOSCursor_Arrow = 1,
  //! Wait cursor.
  eOSCursor_Wait = 2,
  //! Vertical (North-South) resize cursor.
  eOSCursor_ResizeVt = 3,
  //! Horizontal (East-West) resize cursor.
  eOSCursor_ResizeHz = 4,
  //! Diagnoal resize cursor.
  eOSCursor_ResizeDiag = 5,
  //! Hand cursor.
  eOSCursor_Hand = 6,
  //! Text input cursor.
  eOSCursor_Text = 7,
  //! Help cursor.
  eOSCursor_Help = 8,
  //! Use the custom cursor \see ni::iOSWindow::InitCustomCursor.
  eOSCursor_Custom = 100,
  //! \internal
  eOSCursor_ForceDWORD niMaybeUnused = 0xFFFFFFFF,
};

//! OS Monitor flags.
enum eOSMonitorFlags
{
  //! Primary monitor.
  eOSMonitorFlags_Primary = niBit(0),
  //! \internal
  eOSMonitorFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
//! OS Monitor flags type. \see ni::eOSMonitorFlags
typedef tU32 tOSMonitorFlags;

//! OS Message box flags.
enum eOSMessageBoxFlags
{
  //! Message box with one [OK] button.
  eOSMessageBoxFlags_Ok = niBit(0),
  //! Message box with a [OK] and a [CANCEL] button.
  eOSMessageBoxFlags_OkCancel = niBit(1),
  //! Message box with a [YES] and a [NO] button.
  eOSMessageBoxFlags_YesNo = niBit(2),
  //! Message box with an information mark icon.
  eOSMessageBoxFlags_IconInfo = niBit(3),
  //! Message box with a question mark icon.
  eOSMessageBoxFlags_IconQuestion = niBit(4),
  //! Message box with a warning icon.
  eOSMessageBoxFlags_IconWarning = niBit(5),
  //! Message box with an error icon.
  eOSMessageBoxFlags_IconError = niBit(6),
  //! Message box with a hand icon.
  eOSMessageBoxFlags_IconHand = niBit(7),
  //! Message box with a stop icon.
  eOSMessageBoxFlags_IconStop = niBit(8),
  //! Message box which doesn't stay over all other windows.
  eOSMessageBoxFlags_NotTopMost = niBit(9),
  //! Message box will display a fatal error.
  //! \remark Force display of a top most UI message box.
  eOSMessageBoxFlags_FatalError = niBit(10),
  //! \internal
  eOSMessageBoxFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF,
};
//! OS Message box flags typedef. \see ni::eOSMessageBoxFlags
typedef tU32 tOSMessageBoxFlags;

//! eOSMessageBoxReturn
enum eOSMessageBoxReturn
{
  //! Message box couldn't be displayed.
  eOSMessageBoxReturn_Error = 0,
  //! Yes/Ok button pressed.
  eOSMessageBoxReturn_Yes = 1,
  //! Yes/Ok button pressed.
  eOSMessageBoxReturn_OK = 1,
  //! No/Cancel button pressed.
  eOSMessageBoxReturn_No = 2,
  //! No/Cancel button pressed.
  eOSMessageBoxReturn_Cancel = 2,
  //! \internal
  eOSMessageBoxReturn_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! OS Window switch reason.
enum eOSWindowSwitchReason
{
  eOSWindowSwitchReason_Activated = 'a',
  eOSWindowSwitchReason_Deactivated = 'A',
  eOSWindowSwitchReason_LostFocus = 'f',
  eOSWindowSwitchReason_SetFocus = 'F',
  eOSWindowSwitchReason_EnterMenu = 'm',
  eOSWindowSwitchReason_LeaveMenu = 'M',
  eOSWindowSwitchReason_Minimized = 'z',
  eOSWindowSwitchReason_User = 256,
  //! \internal
  eOSWindowSwitchReason_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! A graphics API.
struct iOSGraphicsAPI : public iUnknown
{
  niDeclareInterfaceUUID(iOSGraphicsAPI,0x0ec34553,0xc997,0x914c,0x81,0x21,0x7e,0xb3,0x16,0xd9,0xf0,0x4a);

  virtual const achar* __stdcall GetName() const = 0;
};

//! OS Window interface.
struct iOSWindow : public iUnknown
{
  niDeclareInterfaceUUID(iOSWindow,0x5f7dfadd,0x64b7,0x43f6,0xa4,0x27,0x41,0x56,0xcc,0x9b,0x2b,0x1a)

  //! Get the parent window if any.
  //! {Property}
  //! \remark This is valid only if the window has been created
  //!         explicitly with a parent window. To retrieve the 'OS'
  //!         parent window use GetParentHandle instead.
  virtual iOSWindow* __stdcall GetParent() const = 0;

  //! Set a child window that should always be set to cover the window's client area.
  //! {Property}
  virtual void __stdcall SetClientAreaWindow(tIntPtr aHandle) = 0;
  //! Get the client area window of this window.
  //! {Property}
  virtual tIntPtr __stdcall GetClientAreaWindow() const = 0;

  //! Get the process that created this window.
  //! {Property}
  virtual tIntPtr __stdcall GetPID() const = 0;

  //! Get the os handle of the window.
  //! {Property}
  virtual tIntPtr __stdcall GetHandle() const = 0;
  //! Get whether the handle of the window is owned (aka will be destroyed) by the window object.
  //! {Property}
  virtual tBool __stdcall GetIsHandleOwned() const = 0;

  //! Activate the window if the application is in the foreground.
  virtual void __stdcall ActivateWindow() = 0;
  //! Get whether the window is active or not.
  //! {Property}
  virtual tBool __stdcall GetIsActive() const = 0;
  //! Send a switch in message and set the window in active state.
  //! \param anReason is an id that can be used to identify why the switch occurred, \see ni::eOSWindowSwitchReason
  //! \remark Return eTrue if the active state changed and the SwitchIn message has been sent, else eFalse.
  virtual tBool __stdcall SwitchIn(tU32 anReason) = 0;
  //! Send a switch out message and set the window in inactive state.
  //! \param anReason is an id that can be used to identify why the switch occurred, \see ni::eOSWindowSwitchReason
  //! \remark Return eTrue if the active state changed and the SwitchOut message has been sent, else eFalse.
  virtual tBool __stdcall SwitchOut(tU32 anReason) = 0;

  //! Set the window's title.
  //! {Property}
  virtual void __stdcall SetTitle(const achar* aaszTitle) = 0;
  //! Get the window's title.
  //! {Property}
  virtual const achar* __stdcall GetTitle() const = 0;

  //! Set the window's style.
  //! {Property}
  virtual void __stdcall SetStyle(tOSWindowStyleFlags aStyle) = 0;
  //! Get the window's style.
  //! {Property}
  virtual tOSWindowStyleFlags __stdcall GetStyle() const = 0;

  //! Set the window shown status.
  //! {Property}
  virtual void __stdcall SetShow(tOSWindowShowFlags aStyle) = 0;
  //! Get the window shown status.
  //! {Property}
  virtual tOSWindowShowFlags __stdcall GetShow() const = 0;

  //! Set the window's ZOrder.
  //! {Property}
  virtual void __stdcall SetZOrder(eOSWindowZOrder aZOrder) = 0;
  //! Get the window's ZOrder.
  //! {Property}
  virtual eOSWindowZOrder __stdcall GetZOrder() const = 0;

  //! Set the window's size.
  //! {Property}
  virtual void __stdcall SetSize(const sVec2i& avSize) = 0;
  //! Get the window's size.
  //! {Property}
  virtual sVec2i __stdcall GetSize() const = 0;
  //! Set the window's position.
  //! {Property}
  virtual void __stdcall SetPosition(const sVec2i& avPos) = 0;
  //! Get the window's position.
  //! {Property}
  virtual sVec2i __stdcall GetPosition() const = 0;
  //! Set the window's rectangle.
  //! {Property}
  virtual void __stdcall SetRect(const sRecti& aRect) = 0;
  //! Get the window's rectangle.
  //! {Property}
  virtual sRecti __stdcall GetRect() const = 0;

  //! Set the window's client size.
  //! {Property}
  virtual void __stdcall SetClientSize(const sVec2i& avSize) = 0;
  //! Get the window's client size.
  //! {Property}
  //! \remark The client size can be different of the window's size. For
  //!         example on retina displays with some window manager, such as on
  //!         macOS, the client size is larger because it is always specified
  //!         in pixels. GetContentsScale can be use to retrieve the "virtual
  //!         units" to "pixel size" ratio.
  virtual sVec2i __stdcall GetClientSize() const = 0;

  //! Get the contents' scale factor.
  //! {Property}
  virtual tF32 __stdcall GetContentsScale() const = 0;

  //! Get the window's message handlers.
  //! {Property}
  virtual tMessageHandlerSinkLst* __stdcall GetMessageHandlers() const = 0;

  //! Update the window's message pump and others.
  //! \remark abBlockingMessage specifies whether UpdateWindow should wait for at least one os window message/event
  //!     before returning.
  //! \return eTrue if any window message has been processed, else eFalse if the UpdateWindow didnt do anything.
  //!     In general the application's update should be called only when UpdateWindow return eFalse.
  virtual tBool __stdcall UpdateWindow(tBool abBlockingMessages) = 0;

  //! Center the window in the middle of the current screen/parent window.
  virtual void __stdcall CenterWindow() = 0;

  //! Get whether window closing has been requested.
  //! {Property}
  virtual tBool __stdcall GetRequestedClose() const = 0;
  //! Set the close requested flags.
  //! {Property}
  virtual void __stdcall SetRequestedClose(tBool abRequested) = 0;

  //! Set the current OS cursor.
  //! {Property}
  virtual void __stdcall SetCursor(eOSCursor aCursor) = 0;
  //! Get the current OS cursor.
  //! {Property}
  virtual eOSCursor __stdcall GetCursor() const = 0;
  //! Initialize the custom cursor.
  //! \param aID is an id provided by the user that should be used to identify the current
  //!        custom cursor type.
  //! \param anWidth is the width of the cursor data
  //! \param anHeight is the heigh of the cursor data
  //! \param anHotSpotX
  //! \param anHotSpotY
  //! \param apData is the data of the cursor, should be a U32[w*h] array. If NULL
  //!        InitCustomCursor returns eTrue if the specified custom cursor size is
  //!        supported, else eFalse.
  //! \return eTrue if the specified cursor size is supported, eFalse otherwise.
  //! \remark If eOSCursor_Custom is used as cursor type and no custom cursor as been
  //!         successfully initialized the Arrow cursor is used as fallback.
  virtual tBool __stdcall InitCustomCursor(tIntPtr aID, tU32 anWidth, tU32 anHeight, tU32 anHotSpotX, tU32 anHotSpotY, const tU32* apData) = 0;
  //! Get the custom cursor user id.
  //! {Property}
  //! \remark The id is always 0 when custom cursor initialization with valid apData failed.
  virtual tIntPtr __stdcall GetCustomCursorID() const = 0;
  //! Set the cursor position.
  //! {Property}
  //! \remark The cursor position is in client coordinate.
  virtual void __stdcall SetCursorPosition(const sVec2i& avCursorPos) = 0;
  //! Get the last recorded cursor position.
  //! {Property}
  //! \remark The cursor position is in client coordinate.
  virtual sVec2i __stdcall GetCursorPosition() const = 0;
  //! Set the cursor capture.
  //! {Property}
  //! \remark When enabled only the RelativeMouseMove message will be sent,
  //!         the absolute MouseMove message isn't sent while the cursor is
  //!         captured.
  //! \remark The actual absolute position of the cursor isn't defined when
  //!         disabling cursor capture. While captured, on Windows the cursor
  //!         is constrained to the window's boundaries; on macOS the cursor
  //!         is fixed to the middle of the active window. It is recommended
  //!         that you set the cursor position to a known consistent position
  //!         when disabling cursor capture.
  //! \remark Since the absolute cursor position is not relevant while
  //!         captured, it is recommended to also hide the cursor with
  //!         SetCursor(eOSCursor_None) when capture is enabled.
  virtual void __stdcall SetCursorCapture(tBool abCapture) = 0;
  //! Get the cursor capture state.
  //! {Property}
  virtual tBool __stdcall GetCursorCapture() const = 0;
  //! Get whether the mouse cursor is in the client area of the window.
  //! {Property}
  virtual tBool __stdcall GetIsCursorOverClient() const = 0;

  //! Try to close the window (this is equivalent to pressing the close button on the window).
  virtual void __stdcall TryClose() = 0;

  //! Set whether the window is a drop target.
  //! {Property}
  virtual void __stdcall SetDropTarget(tBool abDropTarget) = 0;
  //! Get whether the window is a drop target.
  //! {Property}
  virtual tBool __stdcall GetDropTarget() const = 0;

  //! Explicitly clear the window's content.
  virtual void __stdcall Clear() = 0;

  //! Force redraw of the full window
  virtual tBool __stdcall RedrawWindow() = 0;

  //! Get the monitor index the window is on.
  //! {Property}
  virtual tU32 __stdcall GetMonitor() const = 0;

  //! Set the window to fullsreen, set to eInvalidHandle to return to windowed.
  //! {Property}
  virtual tBool __stdcall SetFullScreen(tU32 anMonitor) = 0;
  //! Get the monitor on which the window is fullscreend, eInvalidHandle if it's not fullscreen.
  //! {Property}
  virtual tU32 __stdcall GetFullScreen() const = 0;

  //! Get whether the window is currently minimized.
  //! {Property}
  virtual tBool __stdcall GetIsMinimized() const = 0;
  //! Get whether the window is currently maximzed.
  //! {Property}
  virtual tBool __stdcall GetIsMaximized() const = 0;

  //! Set focus on this window.
  virtual void __stdcall SetFocus() = 0;
  //! Get whether the window has focus.
  //! {Property}
  virtual tBool __stdcall GetHasFocus() const = 0;

  //! Set the rate of the refresh timer in seconds.
  //! {Property}
  //! \remark By default windows are purely event driven so they won't
  //!         be refershed/painted unless a message is sent to it for that
  //!         purpose. UpdateWindow() waits for messages to arrive before
  //!         continuing. The refersh timer allows UpdateWindow to continue
  //!         at the specified time interval if no message is received.
  //! \remark Pass a value <= 0 to disable the refresh timer.
  virtual void __stdcall SetRefreshTimer(tF32 afTime) = 0;
  //! Get the rate of the refresh timer.
  //! {Property}
  virtual tF32 __stdcall GetRefreshTimer() const = 0;

  //! Get the parent window handle if any.
  //! {Property}
  //! \remark This returns the 'OS' parent window regardless of
  //!         whether the window has been created explicitly with a
  //!         parent window.
  virtual tIntPtr __stdcall GetParentHandle() const = 0;
  //! Check whether the specified window is a parent window of this window.
  //! \returns 0 if not, 1 if its the direct parent, 2 if its the parent of the parent, etc...
  virtual tU32 __stdcall IsParentWindow(tIntPtr aHandle) const = 0;

  //! Attach a graphics API to the window.
  //! \remark This is OS and graphics driver dependant.
  //! \remark Only one graphics API can be attached to a window during its lifetime.
  virtual tBool __stdcall AttachGraphicsAPI(iOSGraphicsAPI* apAPI) = 0;
  //! Get the graphics API attached to the window.
  //! {Property}
  virtual iOSGraphicsAPI* __stdcall GetGraphicsAPI() const = 0;
};

//! Generic window.
struct iOSWindowGeneric : public iUnknown {
  niDeclareInterfaceUUID(iOSWindowGeneric,0x18265b02,0xe973,0x45f3,0xbc,0x74,0x0d,0xf8,0xd0,0x60,0x8e,0xa2);

  //! Send an input string to the window.
  //! \remark Send a series of eOSWindowMessage_KeyChar messages.
  virtual void __stdcall GenericInputString(const achar* aaszString) = 0;
  //! Send a message to the window.
  virtual void __stdcall GenericSendMessage(tU32 anMsg, const Var& avarA, const Var& avarB) = 0;

  //! Set the window's content scale.
  //! {Property}
  virtual void __stdcall SetContentsScale(tF32 afContentsScale) = 0;
};

niExportFunc(iOSWindow*) CreateGenericWindow();
niExportFunc(iUnknown*) New_niLang_OSWindowGeneric(const Var& avarA, const Var& avarB);

//! Windows window's sink.
//! {NoAutomation}
struct iOSWindowWindowsSink : public iUnknown {
  niDeclareInterfaceUUID(iOSWindowWindowsSink,0xf667e760,0xbdd5,0x4b43,0xbd,0xd0,0x86,0x77,0x1c,0x86,0x8b,0xaf);

  //! Called when a message is received by the window, return TRUE to override the default implementation.
  virtual tIntPtr __stdcall OnOSWindowWindowsSink_WndProc(tIntPtr hWnd, tU32 message, tIntPtr wParam, tIntPtr lParam) = 0;
};

//! Windows window sink list.
typedef SinkList<iOSWindowWindowsSink> tOSWindowWindowsSinkList;

//! Windows window.
//! {NoAutomation}
struct iOSWindowWindows : public iUnknown {
  niDeclareInterfaceUUID(iOSWindowWindows,0x02a97e41,0x8797,0x4679,0x93,0x8a,0x85,0x0b,0x13,0x5a,0xa8,0x48);

  //! Get the window's sink list.
  virtual tOSWindowWindowsSinkList* __stdcall GetWindowsWindowSinkList() const = 0;

  //! Translate a windows KEYDOWN/KEYUP message to a eKey code.
  virtual ni::tU32 __stdcall WindowsTranslateKey(ni::tU32 wParam, ni::tU32 lParam, ni::tBool abDown) = 0;

  //! Default window proc
  virtual tIntPtr __stdcall WndProc(tIntPtr hWnd, tU32 message, tIntPtr wParam, tIntPtr lParam) = 0;
};

//! OSX window.
//! {NoAutomation}
struct iOSWindowOSX : public iUnknown {
  niDeclareInterfaceUUID(iOSWindowOSX,0x460e12e2,0x86d4,0xe849,0x87,0xfe,0x8a,0x35,0xa9,0xb4,0xba,0x80);

  //! Get the window's NS window.
  virtual void* __stdcall GetNSWindow() const = 0;
};

//! QNX window.
//! {NoAutomation}
struct iOSWindowQNX : public iUnknown {
  niDeclareInterfaceUUID(iOSWindowQNX,0x73b61729,0x589d,0xf04c,0x9f,0x7e,0xfd,0x3d,0x19,0x49,0x23,0x86);

  //! Get the native screen handle
  virtual void* __stdcall GetScreenHandle() const = 0;
};

//! Linux window.
//! {NoAutomation}
struct iOSWindowLinux : public iUnknown {
  niDeclareInterfaceUUID(iOSWindowLinux,0x662a125b,0x3a03,0x0244,0x9f,0x3b,0x49,0x18,0xd6,0x1f,0x3b,0x44);

  //! Get the native screen handle
  virtual void* __stdcall GetScreenHandle() const = 0;
};


/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IOSWINDOW_H_6D7C42F3_D6B8_41FD_8AF2_0641803C9F5A__
