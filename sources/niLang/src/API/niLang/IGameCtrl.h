#pragma once
#ifndef __IGAMECTRL_H_E3E8B83D_D5CF_4C13_8107_8EA31A0A1300__
#define __IGAMECTRL_H_E3E8B83D_D5CF_4C13_8107_8EA31A0A1300__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

//! Game controller axis
enum eGameCtrlAxis
{
  //! Left X Axis.
  eGameCtrlAxis_LX = 0,
  //! Left Y Axis.
  eGameCtrlAxis_LY = 1,
  //! Left Z Axis.
  eGameCtrlAxis_LZ = 2,
  //! Right X Axis.
  eGameCtrlAxis_RX = 3,
  //! Right Y Axis.
  eGameCtrlAxis_RY = 4,
  //! Right Z AXis.
  eGameCtrlAxis_RZ = 5,
  //! \internal
  eGameCtrlAxis_Last = 6,
  //! \internal
  eGameCtrlAxis_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Game controller buttons.
enum eGameCtrlButton
{
  //! A
  eGameCtrlButton_A = 0,
  //! B
  eGameCtrlButton_B = 1,
  //! X
  eGameCtrlButton_X = 2,
  //! Y
  eGameCtrlButton_Y = 3,
  //! Left Block
  eGameCtrlButton_LeftBlock = 4,
  //! Right Block
  eGameCtrlButton_RightBlock = 5,
  //! Back
  eGameCtrlButton_Back = 6,
  //! Start
  eGameCtrlButton_Start = 7,
  //! Left Thumb
  eGameCtrlButton_LeftThumb = 8,
  //! Right Thumb
  eGameCtrlButton_RightThumb = 9,

  //! LeftTrigger
  eGameCtrlButton_LeftTrigger = 10,
  //! RightTrigger
  eGameCtrlButton_RightTrigger = 11,

  //! Up
  eGameCtrlButton_DPadUp  = 12,
  //! Down
  eGameCtrlButton_DPadDown = 13,
  //! Left
  eGameCtrlButton_DPadLeft = 14,
  //! Right
  eGameCtrlButton_DPadRight = 15,

  //! \internal
  eGameCtrlButton_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Game controller interface.
//! {DispatchWrapper}
struct iGameCtrl : public iUnknown
{
  niDeclareInterfaceUUID(iGameCtrl,0x14f09df7,0x3842,0x46ef,0x99,0x67,0xbb,0xc6,0x55,0x77,0x58,0xb7)

  //! Update the device states.
  //! \remark All the get methods depends on Update being called first to
  //!         get the latest value as most game controllers are polled and
  //!         not event based.
  //! \remark Return true if any state changed since the last update, eFalse otherwise.
  virtual tBool __stdcall Update() = 0;

  //! Return eTrue if the device is connected.
  //! {Property}
  virtual tBool __stdcall GetIsConnected() const = 0;

  //! Name of the input device.
  //! {Property}
  virtual const achar* __stdcall GetName() const = 0;

  //! Get the index of the game controller.
  //! {Property}
  virtual tU32 __stdcall GetIndex() const = 0;

  //! Return the number of buttons of the game controller.
  //! {Property}
  virtual tU32  __stdcall GetNumButtons() const = 0;
  //! Return the force applied on the button.
  //! {Property}
  virtual tF32 __stdcall GetButton(tU32 ulButton) = 0;

  //! Return the number of axis of the game controller.
  //! {Property}
  virtual tU32  __stdcall GetNumAxis()    const = 0;
  //! Return the position of the given axis
  //! {Property}
  virtual tF32 __stdcall GetAxis(eGameCtrlAxis axis) = 0;

  //! Get whether the game controller can vibrate.
  //! {Property}
  virtual tBool __stdcall GetCanVibrate() const = 0;
  //! Set the left & right motor vibrations.
  virtual void __stdcall Vibrate(const sVec2f& aSpeed) = 0;

  //! Get whether the controller has a battery.
  //! {Property}
  virtual tBool __stdcall GetHasBattery() const = 0;
  //! Get the battery level.
  //! {Property}
  //! \remark 0 is empty, 1.0 is full.
  virtual tF32 __stdcall GetBatteryLevel() const = 0;
};

}
#endif // __IGAMECTRL_H_E3E8B83D_D5CF_4C13_8107_8EA31A0A1300__
