// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/ILang.h>
#include "DataTableWriteStack.h"

using namespace ni;

#define STACK_TOP mStack.top()

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTableWriteStack implementation.

///////////////////////////////////////////////
cDataTableWriteStack::cDataTableWriteStack(iDataTable* apDT)
{
  ZeroMembers();
  mStack.push(apDT);
}

///////////////////////////////////////////////
cDataTableWriteStack::cDataTableWriteStack(const achar* aaszName)
{
  ZeroMembers();
  mStack.push(ni::CreateDataTable(aaszName));
}

///////////////////////////////////////////////
cDataTableWriteStack::~cDataTableWriteStack()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cDataTableWriteStack::ZeroMembers()
{
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cDataTableWriteStack::IsOK() const
{
  niClassIsOK(cDataTableWriteStack);
  return ni::eTrue;
}

///////////////////////////////////////////////
ni::iDataTable * cDataTableWriteStack::GetTop() const
{
  return mStack.top();
}

///////////////////////////////////////////////
const achar * cDataTableWriteStack::GetName() const
{
  if (!niIsOK(STACK_TOP)) return AZEROSTR;
  return STACK_TOP->GetName();
}

///////////////////////////////////////////////
tU32 cDataTableWriteStack::GetNumChildren() const
{
  if (!niIsOK(STACK_TOP)) return 0;
  return STACK_TOP->GetNumChildren();
}

///////////////////////////////////////////////
tBool cDataTableWriteStack::PushChild(tU32 anIndex)
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
tBool cDataTableWriteStack::PushChildFail(tU32 anIndex)
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
tBool cDataTableWriteStack::Push(const achar *aaszName)
{
  if (!niIsOK(STACK_TOP)) {
    mStack.push(NULL);
    return eFalse;
  }
  Ptr<iDataTable> dt = STACK_TOP->GetChild(aaszName);
  if (!dt.IsOK()) {
    dt = ni::CreateDataTable(aaszName);
    STACK_TOP->AddChild(dt);
  }
  mStack.push(dt);
  return dt.IsOK();
}

///////////////////////////////////////////////
tBool cDataTableWriteStack::PushFail(const achar *aaszName)
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
tBool cDataTableWriteStack::PushNew(const achar *aaszName)
{
  Ptr<iDataTable> dt = ni::CreateDataTable(aaszName);
  STACK_TOP->AddChild(dt);
  mStack.push(dt);
  return eTrue;
}

///////////////////////////////////////////////
tBool cDataTableWriteStack::PushAppend(ni::iDataTable *apDT)
{
  STACK_TOP->AddChild(apDT);
  mStack.push(apDT);
  return apDT != NULL;
}

///////////////////////////////////////////////
tBool __stdcall cDataTableWriteStack::PushEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal)
{
  if (!niIsOK(STACK_TOP)) {
    mStack.push(NULL);
    return eFalse;
  }

  Ptr<iDataTable> dt;
  for (tU32 i = 0; i < STACK_TOP->GetNumChildren(); ++i) {
    iDataTable* pDT = STACK_TOP->GetChildFromIndex(i);
    if (ni::StrEq(pDT->GetName(),aaszName) && ni::StrEq(pDT->GetString(aaszProp).Chars(),aaszVal)) {
      dt = pDT;
      break;
    }
  }

  if (!dt.IsOK()) {
    dt = ni::CreateDataTable(aaszName);
    dt->SetString(aaszProp,aaszVal);
    STACK_TOP->AddChild(dt);
  }
  mStack.push(dt);
  return dt.IsOK();
}

///////////////////////////////////////////////
tBool __stdcall cDataTableWriteStack::PushFailEx(const achar* aaszName, const achar* aaszProp, const achar* aaszVal)
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
tBool cDataTableWriteStack::Pop()
{
  if (mStack.size() == 1)
    return eFalse;
  mStack.pop();
  return eTrue;
}

///////////////////////////////////////////////
tBool cDataTableWriteStack::RemoveProperty(const achar *aaszName)
{
  if (!niIsOK(STACK_TOP)) return eFalse;
  tU32 nIndex = STACK_TOP->GetPropertyIndex(aaszName);
  if (nIndex == eInvalidHandle) return eFalse;
  return STACK_TOP->RemovePropertyFromIndex(nIndex);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetString(const achar *aaszName, const achar *v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetString(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetHString(const achar *aaszName, iHString *v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetString(aaszName,HStringGetStringEmpty(v));
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetInt(const achar *aaszName, tI64 v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetInt(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetBool(const achar *aaszName, tBool v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetBool(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetFloat(const achar *aaszName, tF64 v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetFloat(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetVec2(const achar *aaszName, const sVec2f &v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetVec2(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetVec3(const achar *aaszName, const sVec3f &v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetVec3(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetVec4(const achar *aaszName, const sVec4f &v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetVec4(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetMatrix(const achar *aaszName, const sMatrixf &v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetMatrix(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetIUnknown(const achar *aaszName, iUnknown *v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetIUnknown(aaszName,v);
}

///////////////////////////////////////////////
void cDataTableWriteStack::SetEnum(const achar *aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 v)
{
  if (!niIsOK(STACK_TOP)) return;
  STACK_TOP->SetEnum(aaszName,apEnumDef,aFlags,v);
}
