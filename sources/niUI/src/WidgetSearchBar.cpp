// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "WidgetSearchBar.h"

static _HDecl(SearchBar_InputBox);
static _HDecl(SearchBar_ShowBox);

//=================================================================
//
//  cSearchEngineFilePattern
//
//=================================================================
class cSearchEngineFilePattern : public cIUnknownImpl<iSearchEngine, eIUnknownImplFlags_Default>
{
 public:
  cSearchEngineFilePattern() {}
  ~cSearchEngineFilePattern() {}

  virtual void __stdcall SetCorpus(const astl::vector<tSearchItem>& avCorpus) {
    mvCorpus = avCorpus;
  }
  virtual void __stdcall Search(const achar* aSearchTerm, astl::vector<tSearchItem>& avResult) {
    if (!niStringIsOK(aSearchTerm)) {
      avResult = mvCorpus;
      return;
    }

    avResult.clear();

    StrCharIt it(aSearchTerm);
    cString searchTerm;
    searchTerm.reserve(it.compute_lengthAndSizeInBytes() * 2);
    searchTerm.appendChar('*');
    for (;;) {
      tU32 c = it.next();
      if (c == 0) {
        break;
      }
      else if (c == 32 || c == '\n' || c == '\r' || c == '\t') {
        if (searchTerm.back() != '*') {
          searchTerm.appendChar('*');
        }
      }
      else if (c == '|') {
        if (searchTerm.back() == '*') {
          searchTerm.append("|*");
        }
        else {
          searchTerm.append("*|*");
        }
      }
      else {
        searchTerm.appendChar(c);
      }
    }
    searchTerm.appendChar('*');

    Ptr<iRegex> regex = ni::CreateFilePatternRegex(searchTerm.c_str());
    if (!regex.IsOK()) {
      niWarning(niFmt("Can't create file pattern regex '%s'.", aSearchTerm));
      return;
    }

    niLoop(i,mvCorpus.size()) {
      const cString& r = mvCorpus[i].second;
      if (regex->DoesMatch(r.c_str())) {
        avResult.push_back(mvCorpus[i]);
      }
    }
  }

 private:
  astl::vector<tSearchItem> mvCorpus;
  Ptr<iRegex> mptrRegex;
};

//=================================================================
//
//  cSearchContainer
//
//=================================================================
class cSearchContainer : public cIUnknownImpl<iSearchContainer, eIUnknownImplFlags_Default>
{
  astl::vector<tSearchItem> mContainer;

 public:
  virtual void __stdcall AddResult(tU32 anIndex, const achar* aaszValue) {
    mContainer.push_back(astl::make_pair(anIndex, aaszValue));
  }
  const astl::vector<tSearchItem>& GetContainer() {
    return mContainer;
  }
};

//=================================================================
//
//  cWidgetSearchBar
//
//=================================================================
cWidgetSearchBar::cWidgetSearchBar(iWidget *apWidget)
{
  mpWidget = apWidget;
  mptrSE = niNew cSearchEngineFilePattern();
}

cWidgetSearchBar::~cWidgetSearchBar()
{
}

iUnknown* __stdcall cWidgetSearchBar::QueryInterface(const tUUID& aIID)
{
  if (aIID == niGetInterfaceUUID(iWidgetSearchBar))
    return static_cast<iWidgetSearchBar*>(this);
  if (mptrInputBox.IsOK() && aIID == niGetInterfaceUUID(iWidgetComboBox))
    return mptrInputBox->QueryInterface(aIID);
  if (mptrShowBox.IsOK() && aIID == niGetInterfaceUUID(iWidgetListBox))
    return mptrShowBox->QueryInterface(aIID);

  return BaseImpl::QueryInterface(aIID);
}

void __stdcall cWidgetSearchBar::ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const
{
  apLst->Add(niGetInterfaceUUID(iWidgetSearchBar));
  apLst->Add(niGetInterfaceUUID(iWidgetComboBox));
  apLst->Add(niGetInterfaceUUID(iWidgetListBox));
  BaseImpl::ListInterfaces(apLst,anFlags);
}

void cWidgetSearchBar::_InitLayout() {
  mptrInputBox = mpWidget->GetUIContext()->CreateWidget(
      _A("EditBox"),
      mpWidget,
      sRectf(0,0,0,36),
      eWidgetStyle_HoldFocus|eWidgetStyle_NotifyParent,
      _H((_A("SearchBar_InputBox"))));
  mptrInputBox->SetDockStyle(eWidgetDockStyle_DockTop);

  mptrShowBox = mpWidget->GetUIContext()->CreateWidget(
      _A("ListBox"),
      mpWidget,
      sRectf(0,0,0,0),
      eWidgetStyle_HoldFocus|eWidgetStyle_NotifyParent,
      _H(_A("SearchBar_ShowBox")));
  mptrShowBox->SetDockStyle(eWidgetDockStyle_DockFill);

  mptrInputBox->SetFocus();
}

void cWidgetSearchBar::_AdjustChildrenLayout(tF32 w, tF32 h)
{
  if (mptrInputBox.IsOK() && mptrShowBox.IsOK()) {
    Ptr<iFont> ptrFont = mptrInputBox->GetFont();
    tF32 fFontH = ptrFont->GetHeight();
    fFontH *= 1.5f;
    mptrInputBox->SetSize(Vec2(0.0f,fFontH));
    /*sRectf input(0,0,w,fFontH);
      sRectf show(0,fFontH,w,h-fFontH);
      mptrInputBox->SetRect(input);
      mptrShowBox->SetRect(show);*/
  }
}

void cWidgetSearchBar::_MoveFocus(tBool abToPrevious)
{
  if (mpWidget) {
    mpWidget->MoveFocus(abToPrevious);
  }
}

void cWidgetSearchBar::_LostFocus()
{
}

void cWidgetSearchBar::_UpdateEditBoxText()
{
}

void cWidgetSearchBar::SetSearchable(iSearchable* aS)
{
  mptrSearchable = aS;
  Ptr<cSearchContainer> ptrCon = niNew cSearchContainer();
  mptrSearchable->GetCorpus(ptrCon);

  // improve perf, use asyn mode
  const astl::vector<tSearchItem>& vCorpus = ptrCon->GetContainer();
  mptrSE->SetCorpus(vCorpus);
  QPtr<iWidgetListBox> ptrListBox = mptrShowBox.ptr();
  ptrListBox->ClearItems();
  niLoop(i,vCorpus.size()) {
    ptrListBox->AddItem(vCorpus[i].second.Chars());
  }

  mptrInputBox->SendCommand(mpWidget, eWidgetEditBoxCmd_Modified);

  //  mvResult = vCorpus;
  //  ptrListBox->SetSelected(0);
}

// iWidgetSink interface
tBool __stdcall cWidgetSearchBar::OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
{
  niGuardObject((iWidgetSink*)this);
  switch(nMsg) {
    case eUIMessage_SinkAttached: {
      _InitLayout();
      mpWidget->SetStyle(mpWidget->GetStyle()|eWidgetStyle_HoldFocus);
      mptrInputBox->SetFocus();
      break;
    }
    case eUIMessage_Visible: {
      break;
    }
    case eUIMessage_ChildAdded: {
      break;
    }
    case eUIMessage_ChildRemoved: {
      break;
    }
    case eUIMessage_TextChanged: {
      mptrInputBox->SetText(mpWidget->GetText());
      break;
    }
    case eUIMessage_SetText: {
      mpWidget->SetText(mptrInputBox->GetText());
      break;
    }
    case eUIMessage_SkinChanged:
    case eUIMessage_StyleChanged: {
      return eFalse;
    }
    case eUIMessage_Size: {
      sVec2f size = ((Var&)varParam0).GetVec2f();
      _AdjustChildrenLayout(size.x,size.y);
      break;
    }
    case eUIMessage_Command: {
      Ptr<iWidgetCommand> ptrCmd = ni::VarQueryInterface<iWidgetCommand>(varParam0);
      niAssert(ptrCmd.IsOK());
      tU32 nCmd = ptrCmd->GetID();
      Ptr<iWidget> ptrW = ptrCmd->GetSender();
      if (ptrW->GetID() == niGetConstHString(SearchBar_InputBox)){
        if (nCmd == eWidgetEditBoxCmd_Modified) {
          Ptr<iHString> strText = ptrW->GetText();
          //niDebugFmt((_A("# %s"), strText));

          mvResult.clear();
          mptrSE->Search(strText->GetChars(), mvResult);

          QPtr<iWidgetListBox> ptrListBox = mptrShowBox.ptr();
          ptrListBox->ClearItems();

          niLoop(i,mvResult.size()) {
            ptrListBox->AddItem(mvResult[i].second.Chars());
          }
          if (!mvResult.empty())
            QPtr<iWidgetListBox>(mptrShowBox)->SetSelected(0);
        }
      }
      else if (ptrW->GetID() == niGetConstHString(SearchBar_ShowBox)) {
        if (nCmd == eWidgetListBoxCmd_SelectionChanged) {
          QPtr<iWidgetListBox> ptrListBox = mptrShowBox.ptr();
          tU32 nPos = ptrListBox->GetSelected();
          if (nPos < mvResult.size()) {
            mptrSearchable->OnSelect(mvResult[nPos].first);
          }
        }
      }
      break;
    }
    case eUIMessage_MoveFocus: {
      _MoveFocus(varParam0.mBool);
      return eTrue;
    }
    case eUIMessage_SetFocus: {
      mptrInputBox->SetFocus();
      break;
    }
    case eUIMessage_LostFocus: {
      _LostFocus();
      break;
    }
    case eUIMessage_Notify: {
      QPtr<iMessageDesc> ptrMsg = varParam1;
      if (ptrMsg.IsOK()) {
        switch (ptrMsg->GetID()) {
          case eUIMessage_MoveFocus: {
            _MoveFocus(ptrMsg->GetA().mBool);
            break;
          }
          case eUIMessage_KeyDown: {
            // we forward event only if it's coming from the input box since the list box already handles it itself
            const tU32 key = ptrMsg->GetA().mU32;
            _OnKeyDown(key,varParam0.mpIUnknown);
            break;
          }
          case eUIMessage_LostFocus: {
            _LostFocus();
            break;
          }
          case eUIMessage_LeftClickDown: {
            //            iWidget *w = VarQueryInterface<iWidget>(varParam0);
            break;
          }
          case eUIMessage_NCLeftClickUp:
          case eUIMessage_LeftClickUp: {
            break;
          }
          case eUIMessage_LeftDoubleClick: {
            _OnKeyDown(eKey_Enter,0);
            break;
          }
          case eUIMessage_TextChanged: {
            //Ptr<iWidget> w = VarQueryInterface<iWidget>(varParam0);
            //  if (w == mptrInputBox) {
            //    //do google
            //  }
            break;
          }
        }
      }
      break;
    }
  }
  return eFalse;
}

void cWidgetSearchBar::_OnKeyDown(tU32 key, iUnknown* apOriginalWidget)
{
  QPtr<iWidgetListBox> ptrListBox = mptrShowBox.ptr();
  niAssert(ptrListBox.IsOK());
  QPtr<iWidgetEditBox> ptrEditBox = mptrInputBox.ptr();
  niAssert(ptrEditBox.IsOK());

  switch(key) {
    case eKey_Up: {
      if ((apOriginalWidget != mptrShowBox.ptr()) && ptrListBox->GetSelected() > 0) {
        ptrListBox->SetSelected(ptrListBox->GetSelected()-1);
        ptrListBox->AutoScroll();
        ptrEditBox->MoveCursorEnd(eTrue);
      }
      break;
    }
    case eKey_Down: {
      if ((apOriginalWidget != mptrShowBox.ptr()) && ptrListBox->GetSelected() < ptrListBox->GetNumItems()-1) {
        ptrListBox->SetSelected(ptrListBox->GetSelected()+1);
        ptrListBox->AutoScroll();
        ptrEditBox->MoveCursorEnd(eTrue);
      }
      break;
    }
    case eKey_Enter:
    case eKey_NumPadEnter:{
      tU32 nPos = ptrListBox->GetSelected();
      if (nPos < mvResult.size()) {
        mptrSearchable->OnSelect(mvResult[nPos].first);
      }
      _Suicide();
      break;
    }
    case eKey_Escape: {
      _Suicide();
      break;
    }
  }
}

void cWidgetSearchBar::_Suicide()
{
  //ni::PostMessage(mpWidget->GetParent()->GetMessageTarget(), niMessageID('_','_','m','y','D'), varParam0, varParam1);
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetSearchBarCmd_End);
}
