// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"

using namespace ni;

#ifdef niWindows
#include "API/niLang/Platforms/Win32/WinUI.h"

void _SetSystemClipboard(iDataTable* apDT) {
  niAssert(apDT && apDT->IsOK());
  tU32 nTextIndex = apDT->GetPropertyIndex(_A("text"));
  if (nTextIndex != eInvalidHandle) {
    WinUI::Clipboard cb;
    cb.SetClipboardText(apDT->GetStringFromIndex(nTextIndex).Chars());
  }
}
Ptr<iDataTable> _GetSystemClipboard(iDataTable* apExistingDT) {
  Ptr<iDataTable> dt = apExistingDT;
  if (!dt.IsOK()) {
    dt = ni::CreateDataTable(_A("Clipboard"));
    dt->SetString(_A("type"),_A("system"));
  }
  WinUI::Clipboard cb;
  cString strText;
  if (cb.GetClipboardText(strText)) {
    dt->SetString(_A("text"),strText.Chars());
  }
  return dt;
}

#elif defined niOSX || defined niIOS

niExportFunc(int) _OSXSetClipboardText(const char *text);
niExportFuncCPP(cString) _OSXGetClipboardText();

void _SetSystemClipboard(iDataTable* apDT) {
  niAssert(apDT && apDT->IsOK());
  const tU32 nTextIndex = apDT->GetPropertyIndex(_A("text"));
  if (nTextIndex != eInvalidHandle) {
    _OSXSetClipboardText(apDT->GetStringFromIndex(nTextIndex).Chars());
  }
}
Ptr<iDataTable> _GetSystemClipboard(iDataTable* apExistingDT) {
  Ptr<iDataTable> dt = apExistingDT;
  if (!dt.IsOK()) {
    dt = ni::CreateDataTable(_A("Clipboard"));
    dt->SetString(_A("type"),_A("system"));
  }
  cString strText = _OSXGetClipboardText();
  dt->SetString(_A("text"),strText.Chars());
  return dt;
}

#else
void _SetSystemClipboard(iDataTable* apDT) {
  niAssert(apDT && apDT->IsOK());
}
Ptr<iDataTable> _GetSystemClipboard(iDataTable* apExistingDT) {
  Ptr<iDataTable> dt = apExistingDT;
  if (!dt.IsOK()) {
    dt = ni::CreateDataTable(_A("Clipboard"));
    dt->SetString(_A("type"),_A("system"));
  }
  return dt;
}
#endif

///////////////////////////////////////////////
tBool __stdcall cLang::SetClipboard(eClipboardType aType, iDataTable* apDT) {
  niCheck(aType < eClipboardType_Last,eFalse);
  niCheckIsOK(apDT,eFalse);
  mptrClipboard[aType] = apDT->Clone();
  if (aType == eClipboardType_System) {
    _SetSystemClipboard(mptrClipboard[aType]);
  }
  return eTrue;
}

///////////////////////////////////////////////
iDataTable* __stdcall cLang::GetClipboard(eClipboardType aType) const {
  niCheck(aType < eClipboardType_Last,NULL);
  if (aType == eClipboardType_System) {
    niThis(cLang)->mptrClipboard[aType] = _GetSystemClipboard(mptrClipboard[aType]);
  }
  return mptrClipboard[aType];
}
