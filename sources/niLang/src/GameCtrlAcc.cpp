// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include "API/niLang/ILang.h"
#include "GameCtrlAcc.h"

namespace ni {

///////////////////////////////////////////////
cGameCtrlAcc::cGameCtrlAcc()
{
  ZeroMembers();
}

///////////////////////////////////////////////
cGameCtrlAcc::~cGameCtrlAcc() {
}

void cGameCtrlAcc::ZeroMembers()
{
  mvAxis = sVec3f::Zero();
}

void cGameCtrlAcc::Invalidate()
{
}

tBool __stdcall cGameCtrlAcc::IsOK() const
{
  niClassIsOK(cGameCtrlAcc);
  return eTrue;
}

tU32 __stdcall cGameCtrlAcc::GetIndex() const {
  return 0;
}

tBool __stdcall cGameCtrlAcc::GetIsConnected() const {
  return eTrue;
}

const achar*  __stdcall cGameCtrlAcc::GetName() const {
  return "Accelerometer";
}

tU32  __stdcall cGameCtrlAcc::GetNumAxis() const {
  return 3;
}

tU32  __stdcall cGameCtrlAcc::GetNumButtons() const {
  return 0;
}

tBool __stdcall cGameCtrlAcc::Update() {
  return eFalse;
}

tF32  __stdcall cGameCtrlAcc::GetButton(tU32 ulButton) {
  return 0.0f;
}
tF32  __stdcall cGameCtrlAcc::GetAxis(eGameCtrlAxis axis) {
  switch (axis) {
    case eGameCtrlAxis_LX: return mvAxis.x;
    case eGameCtrlAxis_LY: return mvAxis.y;
    case eGameCtrlAxis_LZ: return mvAxis.z;
  }
  return 0.0f;
}

///////////////////////////////////////////////
tBool __stdcall cGameCtrlAcc::UpdateFromState(const sVec4f& state)
{
  mvAxis = (sVec3f&)state;
  return eFalse;
}

}
