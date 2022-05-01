#ifndef __OSACCDRIVER_H_5E908DA1_9935_DF11_973D_83A5CB5A39F2__
#define __OSACCDRIVER_H_5E908DA1_9935_DF11_973D_83A5CB5A39F2__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/Types.h>
#include <niLang/Utils/UnknownImpl.h>
#include "API/niLang/IGameCtrl.h"

namespace ni {

class cGameCtrlAcc : public cIUnknownImpl<iGameCtrl>
{
  niBeginClass(cGameCtrlAcc);

 public:
  cGameCtrlAcc();
  ~cGameCtrlAcc();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;
  void __stdcall Invalidate();

  //// iGameCtrl /////////////////////////////
  tU32 __stdcall GetIndex() const niImpl;
  tBool __stdcall GetIsConnected() const niImpl;
  const achar*  __stdcall GetName() const niImpl;
  tU32  __stdcall GetNumAxis() const niImpl;
  tU32  __stdcall GetNumButtons() const niImpl;
  tBool __stdcall Update() niImpl;
  tF32  __stdcall GetButton(tU32 ulButton) niImpl;
  tF32  __stdcall GetAxis(eGameCtrlAxis axis) niImpl;
  tBool __stdcall GetCanVibrate() const niImpl { return eFalse; }
  void __stdcall Vibrate(const sVec2f& aSpeed) niImpl {}
  tBool __stdcall GetHasBattery() const { return eFalse; }
  tF32 __stdcall GetBatteryLevel() const { return 1.0f; }
  //// iGameCtrl /////////////////////////////

  tBool __stdcall UpdateFromState(const sVec4f& state);

 private:
  sVec3f            mvAxis;

  niEndClass(cGameCtrlAcc);
};

cGameCtrlAcc* __stdcall GetGenericGameCtrlAcc();

}
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __OSACCDRIVER_H_5E908DA1_9935_DF11_973D_83A5CB5A39F2__
