// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/ILang.h>
#include "DataTableReadStack.h"
#include "API/niLang/ILang.h"

using namespace ni;

#define STACK_TOP mStack.top()

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTableReadStack implementation.

///////////////////////////////////////////////
cDataTableReadStack::cDataTableReadStack(iDataTable* apDT)
{
  ZeroMembers();
  mStack.push(apDT);
}

///////////////////////////////////////////////
cDataTableReadStack::~cDataTableReadStack()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cDataTableReadStack::ZeroMembers()
{
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cDataTableReadStack::IsOK() const
{
  niClassIsOK(cDataTableReadStack);
  return ni::eTrue;
}

///////////////////////////////////////////////
ni::iDataTable * cDataTableReadStack::GetTop() const
{
  return mStack.top();
}

///////////////////////////////////////////////
const achar * cDataTableReadStack::GetName() const
{
  if (!niIsOK(STACK_TOP)) return AZEROSTR;
  return STACK_TOP->GetName();
}

///////////////////////////////////////////////
tU32 cDataTableReadStack::GetNumChildren() const
{
  if (!niIsOK(STACK_TOP)) return 0;
  return STACK_TOP->GetNumChildren();
}

///////////////////////////////////////////////
tBool cDataTableReadStack::PushChild(tU32 anIndex)
{
  if (!niIsOK(STACK_TOP)) {
    mStack.push(NULL);
    return eFalse;
  }
  Ptr<iDataTable> dt = STACK_TOP->GetChildFromIndex(anIndex);
  mStack.push(dt);
  return dt.IsOK();
}

///////////////////////////////////////////////
tBool cDataTableReadStack::PushChildFail(tU32 anIndex)
{
  if (!niIsOK(STACK_TOP)) {
    return eFalse;
  }
  Ptr<iDataTable> dt = STACK_TOP->GetChildFromIndex(anIndex);
  if (!dt.IsOK()) return eFalse;
  mStack.push(dt);
  return eTrue;
}

///////////////////////////////////////////////
tBool cDataTableReadStack::Push(const achar *aaszName)
{
  if (!niIsOK(STACK_TOP)) {
    mStack.push(NULL);
    return eFalse;
  }
  Ptr<iDataTable> dt = STACK_TOP->GetChild(aaszName);
  mStack.push(dt);
  return dt.IsOK();
}

///////////////////////////////////////////////
tBool cDataTableReadStack::PushFail(const achar *aaszName)
{
  if (!niIsOK(STACK_TOP)) return eFalse;
  Ptr<iDataTable> dt = STACK_TOP->GetChild(aaszName);
  if (!dt.IsOK()) {
    return eFalse;
  }
  mStack.push(dt);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cDataTableReadStack::PushEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal)
{
  if (!niIsOK(STACK_TOP)) {
    mStack.push(NULL);
    return eFalse;
  }

  for (tU32 i = 0; i < STACK_TOP->GetNumChildren(); ++i) {
    iDataTable* pDT = STACK_TOP->GetChildFromIndex(i);
    if (ni::StrEq(pDT->GetName(),aaszName) && ni::StrEq(pDT->GetString(aaszProp).Chars(),aaszVal)) {
      mStack.push(pDT);
      return eTrue;
    }
  }

  mStack.push(NULL);
  return eFalse;
}

///////////////////////////////////////////////
tBool __stdcall cDataTableReadStack::PushFailEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal)
{
  if (!niIsOK(STACK_TOP)) {
    return eFalse;
  }

  for (tU32 i = 0; i < STACK_TOP->GetNumChildren(); ++i) {
    iDataTable* pDT = STACK_TOP->GetChildFromIndex(i);
    if (ni::StrEq(pDT->GetName(),aaszName) && ni::StrEq(pDT->GetString(aaszProp).Chars(),aaszVal)) {
      mStack.push(pDT);
      return eTrue;
    }
  }

  return eFalse;
}

///////////////////////////////////////////////
tBool cDataTableReadStack::Pop()
{
  if (mStack.size() == 1)
    return eFalse;
  mStack.pop();
  return eTrue;
}

///////////////////////////////////////////////
tBool cDataTableReadStack::RemoveProperty(const achar *aaszName)
{
  if (!niIsOK(STACK_TOP)) return eFalse;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) return eFalse;
  return STACK_TOP->RemovePropertyFromIndex(nIndex);
}

///////////////////////////////////////////////
cString cDataTableReadStack::GetString(const achar *aaszName) const
{
  return GetStringDefault(aaszName,AZEROSTR);
}

///////////////////////////////////////////////
cString cDataTableReadStack::GetStringDefault(const achar *aaszName, const achar *v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetStringFromIndex(nIndex);
}

///////////////////////////////////////////////
tHStringPtr cDataTableReadStack::GetHString(const achar *aaszName) const
{
  return GetHStringDefault(aaszName,_H(AZEROSTR));
}

///////////////////////////////////////////////
tHStringPtr cDataTableReadStack::GetHStringDefault(const achar *aaszName, iHString *v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return _H(STACK_TOP->GetStringFromIndex(nIndex));
}

///////////////////////////////////////////////
tI64 cDataTableReadStack::GetInt(const achar *aaszName) const
{
  return GetIntDefault(aaszName,0);
}

///////////////////////////////////////////////
tI64 cDataTableReadStack::GetIntDefault(const achar *aaszName, tI64 v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetIntFromIndex(nIndex);
}

///////////////////////////////////////////////
tBool cDataTableReadStack::GetBool(const achar *aaszName) const
{
  return GetBoolDefault(aaszName,eFalse);
}

///////////////////////////////////////////////
tBool cDataTableReadStack::GetBoolDefault(const achar *aaszName, tBool v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetBoolFromIndex(nIndex);
}

///////////////////////////////////////////////
tF64 cDataTableReadStack::GetFloat(const achar *aaszName) const
{
  return GetFloatDefault(aaszName,0.0f);
}

///////////////////////////////////////////////
tF64 cDataTableReadStack::GetFloatDefault(const achar *aaszName, tF64 v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetFloatFromIndex(nIndex);
}

///////////////////////////////////////////////
sVec2f cDataTableReadStack::GetVec2(const achar *aaszName) const
{
  return GetVec2Default(aaszName,sVec2f::Zero());
}

///////////////////////////////////////////////
sVec2f cDataTableReadStack::GetVec2Default(const achar *aaszName, const sVec2f &v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetVec2FromIndex(nIndex);
}

///////////////////////////////////////////////
sVec3f cDataTableReadStack::GetVec3(const achar *aaszName) const
{
  return GetVec3Default(aaszName,sVec3f::Zero());
}

///////////////////////////////////////////////
sVec3f cDataTableReadStack::GetVec3Default(const achar *aaszName, const sVec3f &v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetVec3FromIndex(nIndex);
}

///////////////////////////////////////////////
sVec4f cDataTableReadStack::GetVec4(const achar *aaszName) const
{
  return GetVec4Default(aaszName,sVec4f::Zero());
}

///////////////////////////////////////////////
sVec4f cDataTableReadStack::GetVec4Default(const achar *aaszName, const sVec4f &v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetVec4FromIndex(nIndex);
}

///////////////////////////////////////////////
sVec3f cDataTableReadStack::GetCol3(const achar *aaszName) const
{
  return GetVec3Default(aaszName,sVec3f::One());
}

///////////////////////////////////////////////
sVec4f cDataTableReadStack::GetCol4(const achar *aaszName) const
{
  return GetVec4Default(aaszName,sVec4f::One());
}

///////////////////////////////////////////////
sMatrixf cDataTableReadStack::GetMatrix(const achar *aaszName) const
{
  return GetMatrixDefault(aaszName,sMatrixf::Identity());
}

///////////////////////////////////////////////
sMatrixf cDataTableReadStack::GetMatrixDefault(const achar *aaszName, const sMatrixf &v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetMatrixFromIndex(nIndex);
}

///////////////////////////////////////////////
iUnknown * cDataTableReadStack::GetIUnknown(const achar *aaszName) const
{
  return GetIUnknownDefault(aaszName,NULL);
}

///////////////////////////////////////////////
iUnknown * cDataTableReadStack::GetIUnknownDefault(const achar *aaszName, iHString *v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetIUnknownFromIndex(nIndex);
}

///////////////////////////////////////////////
tU32 cDataTableReadStack::GetEnum(const achar *aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const
{
  return GetEnumDefault(aaszName,apEnumDef,aFlags,0);
}

///////////////////////////////////////////////
tU32 cDataTableReadStack::GetEnumDefault(const achar *aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 v) const
{
  if (!niIsOK(STACK_TOP)) return v;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) {
    return v;
  }
  return STACK_TOP->GetEnumFromIndex(nIndex,apEnumDef,aFlags);
}
