// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetCommand.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetCommand implementation.

///////////////////////////////////////////////
cWidgetCommand::cWidgetCommand()
{
  ZeroMembers();
}

///////////////////////////////////////////////
cWidgetCommand::~cWidgetCommand()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetCommand::ZeroMembers()
{
  mnID = eInvalidHandle;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetCommand::IsOK() const
{
  niClassIsOK(cWidgetCommand);
  return ni::eTrue;
}

///////////////////////////////////////////////
tBool cWidgetCommand::Copy(const ni::iWidgetCommand *apSrc)
{
  if (!niIsOK(apSrc)) return eFalse;
  SetSender(apSrc->GetSender());
  SetID(apSrc->GetID());
  SetExtra1(apSrc->GetExtra1());
  SetExtra2(apSrc->GetExtra2());
  return eTrue;
}

///////////////////////////////////////////////
ni::iWidgetCommand * cWidgetCommand::Clone() const
{
  iWidgetCommand* pNew = niNew cWidgetCommand();
  pNew->Copy(this);
  return pNew;
}

///////////////////////////////////////////////
tBool cWidgetCommand::SetSender(ni::iWidget *apSender)
{
  mptrSender = apSender;
  iWidgetCommand* c = ni::QueryInterface<iWidgetCommand>(apSender);
  niUnused(c);
  niAssert(c == NULL);
  return niIsOK(mptrSender);
}

///////////////////////////////////////////////
ni::iWidget * cWidgetCommand::GetSender() const
{
  return mptrSender;
}

///////////////////////////////////////////////
void cWidgetCommand::SetID(tU32 anID)
{
  mnID = anID;
}

///////////////////////////////////////////////
tU32 cWidgetCommand::GetID() const
{
  return mnID;
}

///////////////////////////////////////////////
void cWidgetCommand::SetExtra1(const Var &aVar)
{
  mvarExtra1 = aVar;
  iWidgetCommand* c = VarQueryInterface<iWidgetCommand>(aVar);
  niUnused(c);
  niAssert(c == NULL);
}

///////////////////////////////////////////////
const Var & cWidgetCommand::GetExtra1() const
{
  return mvarExtra1;
}

///////////////////////////////////////////////
void cWidgetCommand::SetExtra2(const Var &aVar)
{
  mvarExtra2 = aVar;
  iWidgetCommand* c = VarQueryInterface<iWidgetCommand>(aVar);
  niUnused(c);
  niAssert(c == NULL);
}

///////////////////////////////////////////////
const Var & cWidgetCommand::GetExtra2() const
{
  return mvarExtra2;
}
