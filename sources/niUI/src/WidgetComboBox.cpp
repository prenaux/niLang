// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetComboBox.h"
#include "HardCodedSkin.h"
#include "API/niUI/Utils/WidgetSinkImpl.h"

#define LB_TEXT_Y_OFFSET 2
#define CBSTATUS_SETEBTEXT  niBit(0)
#define CBSTATUS_SETLBTEXT  niBit(1)
#define CBSTATUS_DONTSETDB  niBit(2)
#define CBSTATUS_SHOWING  niBit(3)
#define CBSTATUS_HIDING   niBit(4)

static _HDecl(ComboBox_Button);
static _HDecl(ComboBox_EditBox);

struct sComboBoxButtonSink : public cWidgetSinkImpl<> {
  cComboBoxWidget* _comboBox;

  sComboBoxButtonSink(cComboBoxWidget* aComboBox) : _comboBox(aComboBox) {
  }

  virtual tBool __stdcall OnMouseHover(const sVec2f& avMousePos, const sVec2f& avNCMousePos) niImpl {
    return eTrue;
  }

  virtual tBool __stdcall OnLeftClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) niImpl {
    _comboBox->_DropList();
    return eTrue;
  }
};

struct sComboBoxDroppedSink : public cWidgetSinkImpl<> {
  cComboBoxWidget* _comboBox;

  sComboBoxDroppedSink(cComboBoxWidget* aComboBox) : _comboBox(aComboBox) {
  }

  virtual tBool __stdcall OnNCRightClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) niImpl {
    return OnNCLeftClickDown(avMousePos,avNCMousePos);
  }
  virtual tBool __stdcall OnNCLeftClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) niImpl {
    _comboBox->_HideList();
    return eTrue;
  }
};

///////////////////////////////////////////////
static void _GetFlags(iHString* ahspText, astl::vector<cString>& aFlags) {
  //niPrintln(niFmt(_A("## GET FLAGS: %s\n"),niHStr(ahspText)));
  cString str = niHStr(ahspText);
  cFlagsStringTokenizer toks;
  StringTokenize(str,aFlags,&toks);
}

cComboBoxWidget::cComboBoxWidget(iWidget *pWidget)
{
  mpwDroppedBox = NULL;
  mptrDroppedSink = niNew sComboBoxDroppedSink(this);
  mnStatus = 0;
  mnNumLines = 4;
  mpWidget = pWidget;
}

cComboBoxWidget::~cComboBoxWidget()
{
}

void cComboBoxWidget::_AdjustChildrenLayout(tF32 w,tF32 h)
{
  const tF32 butW = h;
  sRectf but(w-butW,0,butW,h);
  sRectf edit(0,0,w-butW,h);
  mpwEditBox->SetRect(edit);
  mpwButton->SetRect(but);
}

void cComboBoxWidget::_UpdateStyle()
{
  // Edit box style
  tU32 editstyle = eWidgetStyle_NoMoveFocus|eWidgetStyle_HoldFocus|eWidgetStyle_NotifyParent;
  if ((mpWidget->GetStyle()&eWidgetComboBoxStyle_DropDown) ||
      (mpWidget->GetStyle()&eWidgetComboBoxStyle_ReadOnly)) {
    editstyle |= eWidgetEditBoxStyle_ReadOnly;
  }
  if (mpwEditBox->GetStyle() & eWidgetEditBoxStyle_SelectAllOnSetFocus) {
    editstyle |= eWidgetEditBoxStyle_SelectAllOnSetFocus;
  }
  if (mpwEditBox->GetStyle() & eWidgetEditBoxStyle_AutoScroll) {
    editstyle |= eWidgetEditBoxStyle_AutoScroll;
  }
  mpwEditBox->SetStyle(editstyle);

  // Button style
  QPtr<iWidgetButton> btn = mpwButton.ptr();
  if (btn.IsOK()) {
    btn->SetIcon(mpWidget->FindSkinElement(NULL,NULL,_H("ArrowDown")));
  }

  // Dropped Box style
  if (mpwDroppedBox.IsOK()) {
    tU32 lbStyle = mpwDroppedBox->GetStyle();
    niFlagOn(lbStyle,eWidgetStyle_Free);
    niFlagOn(lbStyle,eWidgetStyle_HoldFocus);
    // if menu
    QPtr<iWidgetMenu> menu = mpwDroppedBox.ptr();
    if (menu.IsOK()) {
      niFlagOn(lbStyle,eWidgetMenuStyle_NoCloseOnGroupChange);
      niFlagOn(lbStyle,eWidgetMenuStyle_NoCloseOnCheckChange);
      niFlagOn(lbStyle,eWidgetMenuStyle_NoCloseOnItemClicked);
    }
    else {
      // not a menu...
      niFlagOn(lbStyle,eWidgetStyle_NotifyParent);
      // if list box
      QPtr<iWidgetListBox> lb = mpwDroppedBox.ptr();
      if (lb.IsOK()) {
        niFlagOnIf(lbStyle,eWidgetListBoxStyle_Multiselect,
                   niFlagIs(mpWidget->GetStyle(),eWidgetComboBoxStyle_Multiselect));
        niFlagOnIf(lbStyle,eWidgetListBoxStyle_ClickAddSelection,
                   niFlagIs(mpWidget->GetStyle(),eWidgetComboBoxStyle_ClickAddSelection));
      }
    }
    mpwDroppedBox->SetStyle(lbStyle);
    _HideList();
  }
}

iUnknown* __stdcall cComboBoxWidget::QueryInterface(const tUUID& aIID)
{
  if (mpwEditBox.IsOK() && aIID == niGetInterfaceUUID(iWidgetEditBox))
    return mpwEditBox->QueryInterface(aIID);
  if (mpwDroppedBox.IsOK() && aIID != niGetInterfaceUUID(iUnknown)) {
    iUnknown* p = mpwDroppedBox->QueryInterface(aIID);
    if (p) return p;
  }
  return BaseImpl::QueryInterface(aIID);
}

void __stdcall cComboBoxWidget::ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const
{
  apLst->Add(niGetInterfaceUUID(iWidgetEditBox));
  apLst->Add(niGetInterfaceUUID(iWidgetListBox));
  BaseImpl::ListInterfaces(apLst,anFlags);
}

void cComboBoxWidget::_DropList()
{
  if (niFlagIs(mnStatus,CBSTATUS_SHOWING) || !mpwDroppedBox.IsOK() || mpwDroppedBox->GetVisible())
    return;
  niFlagOn(mnStatus,CBSTATUS_SHOWING);

  sVec2f pos = mpWidget->GetAbsolutePosition();
  sRectf rect = mpWidget->GetClientRect();

  mpwDroppedBox->SetZOrder(eWidgetZOrder_TopMost);
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetComboBoxCmd_DropShown);
  mpwDroppedBox->SetVisible(eTrue);
  sRectf screenRect = mpWidget->GetUIContext()->GetRootWidget()->GetAbsoluteRect();

  _UpdateDroppedText();
  QPtr<iWidgetMenu> menu = mpwDroppedBox.ptr();
  if (menu.IsOK()) {
    const sRectf deskRect = mpWidget->GetUIContext()->GetRootWidget()->GetAbsoluteRect();
    const sVec2f cbSize = mpWidget->GetSize();
    const sVec2f menuSize = Vec2f(menu->GetMenuWidth(),menu->GetMenuHeight());
    tF32 lbY = pos.y+rect.GetHeight()+1;
    if (lbY+menuSize.y > deskRect.w) {
      lbY = pos.y-menuSize.y;
    }
    mpwDroppedBox->SetAbsolutePosition(Vec2(pos.x+cbSize.x-menuSize.x,lbY));
    menu->Open();
  }
  else {
    QPtr<iWidgetListBox> lb = mpwDroppedBox.ptr();
    if (lb.IsOK()) {
      tF32 lineHeight = mpWidget->GetFont()->GetHeight()+LB_TEXT_Y_OFFSET;
      tU32 nNumLines = ni::Min(mnNumLines,(lb.IsOK())?lb->GetNumItems():mnNumLines);
      tF32 h = lineHeight*(nNumLines+1);
      // out of the screen list box handling
      tF32 lbY = pos.y+rect.GetHeight()+1;
      if (lbY+h > screenRect.GetBottom())
      {
        if (lbY+(lineHeight*5) < screenRect.GetBottom()) {
          // in the upper part, resize the number of lines
          nNumLines = ni::Max(1,tU32((screenRect.GetBottom()-lbY)/lineHeight)-1);
          h = lineHeight*(nNumLines+1);
        }
        else {
          tF32 newY = lbY;
          // move above the list box
          newY -= (rect.GetHeight() + 1);
          // move to the begining
          newY -= h;

          if (newY < screenRect.GetTop()) {
            tF32 diff = screenRect.GetTop()-newY;
            nNumLines = ni::Max(1,tU32((h-diff)/lineHeight)-1);
            h = lineHeight*(nNumLines+1);
            lbY = lbY - rect.GetHeight()-h;
          }
          else {
            lbY = newY;
          }
        }
      }
      mpwDroppedBox->SetRect(sRectf(pos.x,lbY,rect.GetWidth(),h));
      lb->AutoScroll();
    }
    else {
      tF32 w = mpwDroppedBox->GetSize().x;
      tF32 h = mpwDroppedBox->GetSize().y;
      tF32 lbX = pos.x;
      // out of the screen list box handling
      tF32 lbY = pos.y+rect.GetHeight()+1;
      if (lbY+h > screenRect.GetBottom())
      {
        tF32 newY = lbY;
        // move above the list box
        newY -= (rect.GetHeight() + 1);
        // move to the begining
        newY -= h;

        if (newY < screenRect.GetTop()) {
          tF32 diff = screenRect.GetTop()-newY;
          h -= diff;
          lbY = lbY - rect.GetHeight() - h;
        }
        else {
          lbY = newY;
        }
      }
      mpwDroppedBox->SetRect(sRectf(lbX,lbY,w,h));
    }
  }
  mpwDroppedBox->SetFocus();

  // clip against the screen rect
  sRectf lbAbsRect = screenRect.ClipRect(mpwDroppedBox->GetAbsoluteRect());
  mpwDroppedBox->SetAbsoluteRect(lbAbsRect);

  mpwDroppedBox->SetExclusive(eTrue);
  mpwDroppedBox->AddSink(mptrDroppedSink);

  niFlagOff(mnStatus,CBSTATUS_SHOWING);
}

void cComboBoxWidget::_HideList()
{
  if (niFlagIs(mnStatus,CBSTATUS_HIDING) || !mpwDroppedBox.IsOK())
    return;
  niFlagOn(mnStatus,CBSTATUS_HIDING);

  mpwDroppedBox->RemoveSink(mptrDroppedSink);
  mpwDroppedBox->SetExclusive(eFalse);

  if (mpwDroppedBox->GetVisible()) {
    mpWidget->SendCommand(mpWidget->GetParent(),eWidgetComboBoxCmd_DropHidden);
    QPtr<iWidgetMenu> menu = mpwDroppedBox.ptr();
    if (menu.IsOK()) {
      menu->Close();
    }
    else {
      mpwDroppedBox->SetVisible(eFalse);
    }
    _UpdateEditBoxText();
  }

  niFlagOff(mnStatus,CBSTATUS_HIDING);
}

tBool cComboBoxWidget::OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
{
  niGuardObject((iWidgetSink*)this);
  switch(nMsg) {
    case eUIMessage_SinkAttached:
      {
        mpwEditBox = mpWidget->GetUIContext()->CreateWidget(
            _A("EditBox"),mpWidget,sRectf(0,0,0,0),
            eWidgetStyle_NoMoveFocus|eWidgetStyle_HoldFocus|
            eWidgetStyle_NotifyParent,
            _HC(ComboBox_EditBox));
        mpwEditBox->SetPadding(Vec4f(1,0,1,1));
        mpwButton = mpWidget->GetUIContext()->CreateWidget(
            _A("Button"),mpWidget,sRectf(0,0,0,0),
            eWidgetButtonStyle_IconFit|eWidgetButtonStyle_IconCenter,
            _HC(ComboBox_Button));
        mpwButton->AddSink(niNew sComboBoxButtonSink(this));

        QPtr<iWidgetButton>(mpwButton)->SetIconMargin(Vec4<tF32>(3,3,3,3));
        if (niFlagIsNot(mpWidget->GetStyle(),eWidgetComboBoxStyle_NoDefaultListBox)) {
          mpWidget->GetUIContext()->CreateWidget(
              _A("ListBox"),mpWidget,sRectf(0,0,100,80),
              eWidgetStyle_Free|
              eWidgetStyle_NotifyParent|
              eWidgetStyle_HoldFocus,
              _H("ComboBox_ListBox"));
        }
        if (!mpwDroppedBox.IsOK()) {
          _SetDropped(NULL,eFalse);
        }

        // this will call UpdateStyle
        mpWidget->SetStyle(mpWidget->GetStyle()|eWidgetStyle_HoldFocus);
        break;
      };
    case eUIMessage_ChildAdded:
      {
        Ptr<iWidget> w = VarQueryInterface<iWidget>(varParam0);
        if (w.IsOK() && niFlagIsNot(w->GetStyle(),eWidgetStyle_Temp))
          _SetDropped(w,eTrue);
        break;
      }
    case eUIMessage_ChildRemoved:
      {
        Ptr<iWidget> w = VarQueryInterface<iWidget>(varParam0);
        if (w.IsOK() && niFlagIsNot(w->GetStyle(),eWidgetStyle_Temp))
          _SetDropped(w,eFalse);
        break;
      }
    case eUIMessage_TextChanged: {
      mpwEditBox->SetText(mpWidget->GetText());
      mpWidget->SetHoverText(mpWidget->GetText());
      mpwEditBox->SetHoverText(mpWidget->GetHoverText());
      _UpdateDroppedText();
      break;
    }
    case eUIMessage_SetText: {
      mpWidget->SetText(mpwEditBox->GetText());
      mpWidget->SetHoverText(mpWidget->GetText());
      mpwEditBox->SetHoverText(mpWidget->GetHoverText());
      break;
    }
    case eUIMessage_SkinChanged:
    case eUIMessage_StyleChanged:
      {
        _UpdateStyle();
        return eFalse;
      }
    case eUIMessage_Size:
      {
        sVec2f size = ((Var&)varParam0).GetVec2f();
        _AdjustChildrenLayout(size.x,size.y);
        break;
      }
    case eUIMessage_Command:
      {
        Ptr<iWidgetCommand> ptrCmd = ni::VarQueryInterface<iWidgetCommand>(varParam0);
        niAssert(ptrCmd.IsOK());
        tU32 nCmd = ptrCmd->GetID();
        iWidget *w = ptrCmd->GetSender();
        if (w->GetID() == niGetConstHString(ComboBox_EditBox)) {
          if (nCmd == eWidgetEditBoxCmd_Modified) {
            mpWidget->SendCommand(mpWidget->GetParent(),eWidgetComboBoxCmd_Modified);
          }
          if (nCmd == eWidgetEditBoxCmd_Validated) {
            mpWidget->SetText(mpwEditBox->GetText());
            mpWidget->SendCommand(mpWidget->GetParent(),eWidgetComboBoxCmd_Validated);
          }
        }
        else if (w == mpwDroppedBox) {
          QPtr<iWidgetMenu> menu = mpwDroppedBox.ptr();
          if (menu.IsOK()) {
            if (nCmd == eWidgetMenuCmd_Closed) {
              _HideList();
            }
            else {
              if (nCmd == eWidgetMenuCmd_Checked ||
                  nCmd == eWidgetMenuCmd_UnChecked ||
                  nCmd == eWidgetMenuCmd_GroupChanged)
                _UpdateEditBoxText();
            }
          }
          else {
            QPtr<iWidgetListBox> lb = mpwDroppedBox.ptr();
            if (lb.IsOK() &&
                niFlagIsNot(mnStatus,CBSTATUS_SETLBTEXT) &&
                nCmd == eWidgetListBoxCmd_SelectionChanged)
            {
              _UpdateEditBoxText();
            }
          }
        }
      }
      break;
    case eUIMessage_KeyDown:
    case eUIMessage_KeyUp:
    case eUIMessage_KeyChar:
      if (mpwDroppedBox.IsOK()) {
        mpwDroppedBox->SendMessage(nMsg,varParam0,varParam1);
      }
      break;
    case eUIMessage_MoveFocus: {
      _MoveFocus(varParam0.mBool);
      return eTrue;
    }
    case eUIMessage_SetFocus: {
      mpwEditBox->SetFocus();
      break;
    }
    case eUIMessage_LostFocus: {
      _LostFocus();
      break;
    }
    case eUIMessage_Notify:
      {
        QPtr<iMessageDesc> ptrMsg = varParam1;
        if (ptrMsg.IsOK()) {
          switch (ptrMsg->GetID()) {
            case eUIMessage_MoveFocus: {
              _MoveFocus(ptrMsg->GetA().mBool);
              break;
            }
            case eUIMessage_KeyUp: {
              if (ptrMsg->GetA().mU32 == eKey_Escape) {
                _HideList();
              }
              break;
            }
            case eUIMessage_LostFocus: {
              _LostFocus();
              break;
            }
            case eUIMessage_LeftClickDown:
              {
                iWidget *w = VarQueryInterface<iWidget>(varParam0);
                if (w->GetID() == niGetConstHString(ComboBox_EditBox)) {
                  if (mpWidget->GetStyle()&eWidgetComboBoxStyle_DropDown) {
                    if (!mpwDroppedBox->GetVisible()) {
                      _DropList();
                    }
                    else {
                      _HideList();
                    }
                  }
                }
                break;
              }
            case eUIMessage_TextChanged:
              {
                Ptr<iWidget> w = VarQueryInterface<iWidget>(varParam0);
                if (w == mpwDroppedBox) {
                  if (niFlagIsNot(mnStatus,CBSTATUS_SETEBTEXT)) {
                    _UpdateEditBoxText();
                  }
                }
                break;
              }
          }
        }
        break;
      }
  }
  return eFalse;
}

///////////////////////////////////////////////
void __stdcall cComboBoxWidget::SetNumLines(tU32 anNum)
{
  mnNumLines = anNum?anNum:4;
}

///////////////////////////////////////////////
tU32 __stdcall cComboBoxWidget::GetNumLines() const
{
  return mnNumLines;
}

///////////////////////////////////////////////
void cComboBoxWidget::_LostFocus()
{
  if (!mpWidget->GetHasFocus() &&
      (!mpwDroppedBox.IsOK() || !mpwDroppedBox->GetHasFocus()) &&
      (!mpwEditBox->GetHasFocus()) &&
      (!mpwButton->GetHasFocus()))
  {
    _HideList();
  }
}

///////////////////////////////////////////////
void cComboBoxWidget::_UpdateEditBoxText()
{
  if (!mpwDroppedBox.IsOK()) return;
  niFlagOn(mnStatus,CBSTATUS_SETEBTEXT);
  QPtr<iWidgetMenu> menu = mpwDroppedBox.ptr();
  if (menu.IsOK()) {
    cString o;
    tBool bPrefixWithPipe = eFalse;
    niLoop(i,menu->GetNumItems()) {
      iWidgetMenuItem* item = menu->GetItem(i);
      if (!item || !item->GetSelected()) continue;
      if (bPrefixWithPipe) o << "|";
      o << item->GetName();
      bPrefixWithPipe = eTrue;
    }
    mpwEditBox->SetText(_H(o));
  }
  else {
    mpwEditBox->SetText(mpwDroppedBox->GetText());
  }
  QPtr<iWidgetEditBox> eb = mpwEditBox;
  if (eb.IsOK()) {
    eb->MoveCursorHome(eFalse);
    eb->SetSelection(sVec4i::Zero());
  }
  niFlagOff(mnStatus,CBSTATUS_SETEBTEXT);
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetListBoxCmd_SelectionChanged);
}

///////////////////////////////////////////////
void cComboBoxWidget::_UpdateDroppedText()
{
  if (!mpwDroppedBox.IsOK()) return;
  niFlagOn(mnStatus,CBSTATUS_SETLBTEXT);
  QPtr<iWidgetMenu> menu = mpwDroppedBox.ptr();
  if (menu.IsOK()) {
    astl::vector<cString> vFlags;
    _GetFlags(mpwEditBox->GetText(),vFlags);
    niLoop(i,menu->GetNumItems()) {
      menu->GetItem(i)->SetSelected(eFalse);
    }
    niLoop(i,vFlags.size()) {
      iWidgetMenuItem* item = menu->GetItemFromName(vFlags[i].Chars());
      if (item) item->SetSelected(eTrue);
    }
  }
  else {
    mpwDroppedBox->SetText(mpwEditBox->GetText());
  }
  niFlagOff(mnStatus,CBSTATUS_SETLBTEXT);
}

///////////////////////////////////////////////
void cComboBoxWidget::_MoveFocus(tBool abToPrevious) {
  if (mpWidget) {
    mpWidget->MoveFocus(abToPrevious);
  }
}

///////////////////////////////////////////////
tBool cComboBoxWidget::_SetDropped(iWidget* apDB, tBool abAdd)
{
  if (niFlagIs(mnStatus,CBSTATUS_DONTSETDB))
    return eFalse;

  if (apDB &&
      ((apDB == mpwDroppedBox && abAdd) ||
       apDB->GetID() == niGetConstHString(ComboBox_EditBox) ||
       apDB->GetID() == niGetConstHString(ComboBox_Button)))
    return eFalse;

  if (mpwDroppedBox.IsOK()) {
    Ptr<iWidget> toDestroy = mpwDroppedBox;
    // make sure the dropped sink has been removed
    mpwDroppedBox->RemoveSink(mptrDroppedSink);
    mpwDroppedBox = NULL;
    niFlagOn(mnStatus,CBSTATUS_DONTSETDB);
    // destroy only if its a child widget
    if (mpWidget->GetChildIndex(toDestroy) != eInvalidHandle) {
      toDestroy->Destroy();
    }
    niFlagOff(mnStatus,CBSTATUS_DONTSETDB);
  }

  if (abAdd) {
    mpwDroppedBox = apDB;
  }
  if (!abAdd || !mpwDroppedBox.IsOK()) {
    mpwDroppedBox = mpWidget->GetUIContext()->CreateWidget(
        _A("Label"),mpWidget,
        sRectf(0,0,100,80),
        eWidgetStyle_Free);
    mpwDroppedBox->SetText(_H(niFmt(_A("No Drop Widget Set -%s- "),
                                     niHStr(mpWidget->GetID()))));
  }

  _UpdateStyle();
  return eTrue;
}

///////////////////////////////////////////////
iWidget* __stdcall cComboBoxWidget::GetDroppedWidget() const {
  return mpwDroppedBox;
}
tBool __stdcall cComboBoxWidget::SetDroppedWidget(iWidget* apWidget) {
  return _SetDropped(apWidget,eTrue);
}
