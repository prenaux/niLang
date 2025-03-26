// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetButton.h"
#include "niUI_HString.h"
#include <niLang/Utils/UnitSnap.h>

cButtonWidget::cButtonWidget(iWidget *pWidget) {
  mpWidget = pWidget;
  mbPressed = eFalse;
  mbChecked = eFalse;
  mvIconMargin = sVec4f::Zero();
  mvIconSize = sVec2f::Zero();
  mDrawFrameFlags = eRectFrameFlags_All;
}

cButtonWidget::~cButtonWidget() {
}

static void _UncheckSameGroup(iWidget* apSkip, iHString* ahspDummy, iHString* ahspCanvas, iWidget* parent, iHString* ahspGroup) {
  for (tU32 i = 0; i < parent->GetNumChildren(); ++i) {
    iWidget* pW = parent->GetChildFromIndex(i);
    if (pW == apSkip) continue;
    if (pW->GetClassName() == ahspDummy || pW->GetClassName() == ahspCanvas) {
      _UncheckSameGroup(apSkip,ahspDummy,ahspCanvas,pW,ahspGroup);
    }
    else {
      Ptr<iWidgetButton> ptrButton = ni::QueryInterface<iWidgetButton>(pW);
      if (ptrButton.IsOK() && ptrButton->GetGroupID() == ahspGroup) {
        ptrButton->SetCheck(eFalse);
      }
    }
  }
}

void cButtonWidget::SetIcon(iOverlay *apIcon) {
  mptrIconNormal = apIcon;
}
iOverlay* cButtonWidget::GetIcon() const {
  return mptrIconNormal;
}

void cButtonWidget::SetIconPressed(iOverlay *apIcon) {
  mptrIconPressed = apIcon;
}
iOverlay* cButtonWidget::GetIconPressed() const {
  return mptrIconPressed;
}

void cButtonWidget::SetIconHover(iOverlay *apIcon) {
  mptrIconHover = apIcon;
}
iOverlay* cButtonWidget::GetIconHover() const {
  return mptrIconHover;
}

void cButtonWidget::SetIconSize(const sVec2f& avSize) {
  if (mpWidget->GetID() == _H("Gizmo.Select")) {
    mvIconSize = Vec2f(111,111);
  }
  mvIconSize = avSize;
}
sVec2f cButtonWidget::GetIconSize() const {
  if (mpWidget->GetID() == _H("Gizmo.Select")) {
    return mvIconSize;
  }
  return mvIconSize;
}

void cButtonWidget::SetCheck(tBool abChecked)
{
  tBool bWasChecked = mbChecked;
  mbChecked = abChecked;
  if (bWasChecked != abChecked) {
    mpWidget->SendCommand(mpWidget->GetParent(),mbChecked?eWidgetButtonCmd_Checked:eWidgetButtonCmd_UnChecked);
    if (HStringIsNotEmpty(mhspGroupID) && mbChecked) {
      tHStringPtr hspDummy = _HC(Dummy);
      tHStringPtr hspCanvas = _HC(Canvas);
      iWidget* parent = mpWidget->GetParent();
      while (parent && (parent->GetClassName() == hspDummy || parent->GetClassName() == hspCanvas)) {
        parent = parent->GetParent();
      }
      if (parent) {
        _UncheckSameGroup(mpWidget,hspDummy,hspCanvas,parent,mhspGroupID);
      }
    }
  }
}
tBool __stdcall cButtonWidget::GetCheck() const {
  return mbChecked;
}

void cButtonWidget::ProcessClick(const sVec2f& avMousePos)
{
  if (mpWidget->GetStyle() & eWidgetButtonStyle_OnOff) {
    SetCheck(mbChecked?eFalse:eTrue);
  }
  else if (HStringIsNotEmpty(mhspGroupID)) {
    SetCheck(eTrue);
  }
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetButtonCmd_Clicked,avMousePos);
}

void cButtonWidget::ProcessPush(const sVec2f& avMousePos)
{
  mbPressed = eTrue;
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetButtonCmd_Pushed,avMousePos);
}

void cButtonWidget::ProcessUnPush(const sVec2f& avMousePos)
{
  mbPressed = eFalse;
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetButtonCmd_UnPushed,avMousePos);
}

void cButtonWidget::Paint_PushButton(const sRectf& aRect, iCanvas* apCanvas)
{
  sRectf rect = aRect;
  tU32 nStyle = mpWidget->GetStyle();
  iFont* pFont = NULL;
  iOverlay* frame = NULL;
  iOverlay* pIcon = NULL;
  tBool bIsPressed;
  _GetStatusItems(&frame,&pFont,&pIcon,&bIsPressed);

  if (niFlagIsNot(nStyle,eWidgetButtonStyle_NoFrame)) {
    //    frame->DrawFrameEx(NULL,mDrawFrameFlags,rect.GetTopLeft(),rect.GetSize());
    apCanvas->BlitOverlayFrame(rect,frame,mDrawFrameFlags);
    rect = frame->ComputeFrameCenter(rect);
  }

  if (niFlagIs(nStyle,eWidgetButtonStyle_Select)) {
    const sVec4f selMargin = {2,2,2,2};
    const tU32 nColor = bIsPressed ? skin._kcolBorderColor : 0;
    if (nColor) {
      apCanvas->BlitFill(
        Rectf(aRect.GetTopLeft(),aRect.GetTopRight()+Vec2<tF32>(0,selMargin.Left())),
        nColor);
      apCanvas->BlitFill(
        Rectf(aRect.GetBottomLeft()-Vec2<tF32>(0,selMargin.Right()),aRect.GetBottomRight()),
        nColor);
      apCanvas->BlitFill(
        Rectf(aRect.GetTopLeft(),aRect.GetBottomLeft()+Vec2<tF32>(selMargin.Top(),0)),
        nColor);
      apCanvas->BlitFill(
        Rectf(aRect.GetTopRight()-Vec2<tF32>(selMargin.Bottom(),0),aRect.GetBottomRight()),
        nColor);
    }
  }

  const tBool bHorizontal = rect.GetWidth() >= rect.GetHeight();
  if (pIcon) {
    niLet destRectSize = sVec2f {
      rect.GetWidth()-mvIconMargin.x-mvIconMargin.w,
      rect.GetHeight()-mvIconMargin.y-mvIconMargin.z
    };

    // Split the 'layout' and 'draw' size, draw size preserves the aspect
    // ratio of the icon by default.
    sVec2f layoutSize = (mvIconSize != sVec2f::Zero()) ? mvIconSize : pIcon->GetSize();
    sVec2f drawSize;
    if (niFlagIs(nStyle,eWidgetButtonStyle_IconStretch)) {
      // both stretched so same size
      drawSize = layoutSize = destRectSize;
    }
    else {
      if (niFlagIs(nStyle,eWidgetButtonStyle_IconFit)) {
        // fit the layout in the destination size
        layoutSize = Rectf(0,0,layoutSize).FitInto(Rectf(0,0,destRectSize)).GetSize();
      }
      // the drawSize is always fitted in the layoutSize
      drawSize = Rectf(0,0,pIcon->GetSize()).FitInto(Rectf(0,0,layoutSize)).GetSize();
    }

    const tF32 xcenter = ni::UnitSnapf((rect.GetWidth() - drawSize.x) * 0.5f);
    const tF32 ycenter = ni::UnitSnapf((rect.GetHeight() - drawSize.y) * 0.5f);

    if (niFlagIs(nStyle,eWidgetButtonStyle_IconCenter)) {
      mrectIcon.SetTopLeft(Vec2<tF32>(rect.Left()+xcenter,rect.Top()+ycenter));
      mrectIcon.SetSize(drawSize.x,drawSize.y);
      // no text offset...
    }
    else if (niFlagIs(nStyle,eWidgetButtonStyle_IconTop)) {
      mrectIcon.SetTopLeft(Vec2<tF32>(
        rect.Left()+xcenter,
        rect.Top()+mvIconMargin.Top()));
      mrectIcon.SetSize(drawSize.x,drawSize.y);
      rect.SetTop(mrectIcon.GetTop()+layoutSize.y+mvIconMargin.Bottom());
    }
    else if (niFlagIs(nStyle,eWidgetButtonStyle_IconBottom)) {
      mrectIcon.SetTopLeft(Vec2<tF32>(
        rect.Left()+xcenter,
        rect.Bottom()-mvIconMargin.Bottom()-layoutSize.y));
      mrectIcon.SetSize(drawSize.x,drawSize.y);
      rect.SetBottom(mrectIcon.GetTop()-mvIconMargin.Top());
    }
    else if (niFlagIs(nStyle,eWidgetButtonStyle_IconRight)) {
      mrectIcon.SetTopLeft(Vec2<tF32>(
        rect.Right()-mvIconMargin.Right()-layoutSize.x,
        rect.Top()+ycenter));
      mrectIcon.SetSize(drawSize.x,drawSize.y);
      rect.SetRight(mrectIcon.GetLeft()-mvIconMargin.Left());
    }
    else /*if (niFlagIs(nStyle,eWidgetButtonStyle_IconLeft))*/ {
      mrectIcon.SetTopLeft(Vec2<tF32>(
        rect.Left()+mvIconMargin.Left(),rect.Top()+ycenter));
      mrectIcon.SetSize(drawSize.x,drawSize.y);
      rect.SetLeft(mrectIcon.GetLeft()+layoutSize.x+mvIconMargin.Right());
    }

    if (pFont) {
      sVec4f iconColor = ULColorToVec4f(pFont->GetColor());
      iconColor.w = pIcon->GetColor().w;
      pIcon->SetColor(iconColor);
    }
    apCanvas->BlitOverlay(mrectIcon,pIcon);
  }

  if (niFlagIsNot(nStyle,eWidgetButtonStyle_NoText)) {
    const achar* text = _GetText();
    if (niIsStringOK(text) && pFont) {
      tU32 oldCol = pFont->GetColor();
      tU32 col = oldCol;

      if (niFlagIs(nStyle,eWidgetButtonStyle_Select)) {
        tU32 nColor = 0;
        if (bIsPressed) {
          nColor = skin._kcolPressedBack;
        }
        if (nColor) {
          col = skin.pressedFont->GetColor();
        }
      }
      if (!mpWidget->GetEnabled()) {
        col = /*(mnDisabledColor != 0) ? mnDisabledColor : */ULColorSetAf(col,0.5f);
      }

      tFontFormatFlags fontFormat = 0;
      if ((niFlagIs(nStyle,eWidgetButtonStyle_IconCenter) && !bHorizontal) ||
          niFlagIs(nStyle,eWidgetButtonStyle_IconTop) ||
          niFlagIs(nStyle,eWidgetButtonStyle_IconBottom))
      {
        fontFormat |= eFontFormatFlags_CenterH;
        if (niFlagIs(nStyle,eWidgetButtonStyle_TextLeft)) {
          // top aligned - is the default
        }
        else if (niFlagIs(nStyle,eWidgetButtonStyle_TextRight)) {
          fontFormat |= eFontFormatFlags_Bottom;
        }
        else {
          fontFormat |= eFontFormatFlags_CenterV;
        }
      }
      else {
        fontFormat |= eFontFormatFlags_CenterV;
        if (niFlagIs(nStyle,eWidgetButtonStyle_TextLeft)) {
          // left aligned - is the default
          if (pIcon && niFlagIs(nStyle,eWidgetButtonStyle_IconLeft)) {
            rect.Left() += pFont->GetCharWidth(' ');
          }
        }
        else if (niFlagIs(nStyle,eWidgetButtonStyle_TextRight)) {
          fontFormat |= eFontFormatFlags_Right;
          if (pIcon && niFlagIs(nStyle,eWidgetButtonStyle_IconRight)) {
            rect.Right() -= pFont->GetCharWidth(' ');
          }
        }
        else {
          fontFormat |= eFontFormatFlags_CenterH;
        }
      }

      pFont->SetColor(col);
      apCanvas->BlitText(pFont,rect,fontFormat,_GetText());
      pFont->SetColor(oldCol);
    }
  }
}

void cButtonWidget::Paint_CheckRadioButton(const sRectf &rect, iCanvas* apCanvas)
{
  tU32 nStyle = mpWidget->GetStyle();
  sRectf txtrect = rect;
  iFont* pFont = NULL;
  iOverlay* frame = NULL;
  tBool bIsPressed;
  _GetStatusItems(&frame,&pFont,NULL,&bIsPressed);

  tF32 xoff = 0;
  tF32 yoff = ni::UnitSnapf((rect.GetHeight() - frame->GetSize().y) * 0.5f);
  sVec2f framePos;
  if (niFlagIs(nStyle,eWidgetButtonStyle_TextRight)) {
    framePos = Vec2<tF32>(rect.Right()-frame->GetSize().x,yoff);
    txtrect.Right() -= frame->GetSize().x+4;
  }
  else {
    framePos = Vec2<tF32>(xoff,yoff);
    txtrect.Left() += frame->GetSize().x+4;
  }

  apCanvas->BlitOverlay(
      sRectf(framePos.x,framePos.y,
                  frame->GetSize().x,frame->GetSize().y),
      frame);
  if (mbChecked) {
    apCanvas->BlitOverlay(
        sRectf(framePos.x,framePos.y,
                    skin.mark->GetSize().x,skin.mark->GetSize().y),
        skin.mark);
  }

  if (!niFlagIs(nStyle,eWidgetButtonStyle_NoText) && pFont) {
    apCanvas->BlitText(pFont,txtrect,
                       eFontFormatFlags_CenterV|
                       eFontFormatFlags_ClipH|eFontFormatFlags_ClipV|
                       (niFlagIs(nStyle,eWidgetButtonStyle_TextRight)?
                        eFontFormatFlags_Right:0),
                       _GetText());
  }
}

tBool cButtonWidget::OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
{
  niGuardObject((iWidgetSink*)this);
  switch(nMsg) {
    case eUIMessage_NCSize:
      niFallthrough;
    case eUIMessage_Paint:{
      iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
      if (c) {
        const sRectf rect = Rectf(0,0,mpWidget->GetClientSize());
        const tBool bOver = apWidget->GetIsMouseOver();
        const tU32 style = mpWidget->GetStyle();
        mrectIcon = rect;
        if (niFlagIs(style,eWidgetButtonStyle_CheckBox) ||
            niFlagIs(style,eWidgetButtonStyle_RadioButton))
        {
          Paint_CheckRadioButton(rect,c);
        }
        else {
          Paint_PushButton(rect,c);
        }
      }
      return eFalse;
    }
    case eUIMessage_KeyDown: {
      const tU32 key = varParam0.mU32;
      if (key == eKey_Enter || key == eKey_NumPadEnter)
        ProcessPush(sVec2f::Zero());
      break;
    }
    case eUIMessage_KeyUp: {
      const tU32 key = varParam0.mU32;
      if ((key == eKey_Enter || key == eKey_NumPadEnter) && (mbPressed)) {
        ProcessUnPush(sVec2f::Zero());
        ProcessClick(sVec2f::Zero());
      }
      break;
    }
    case eUIMessage_LeftClickDown: {
      const sVec2f& vMousePos = ((Var&)varParam0).GetVec2f();
      mpWidget->SetCapture(eTrue);
      ProcessPush(vMousePos);
      break;
    }
    case eUIMessage_NCLeftClickUp:
    case eUIMessage_LeftClickUp: {
      if (mbPressed) {
        // get the states before processing unpush otherwise will
        // always be false since ProcessUnPush resets mbPressed
        // and co.
        const tBool canClick = (mbPressed != 2);
        const tBool isOver = mpWidget->GetIsMouseOver() ||
            (mpWidget->GetStyle()&eWidgetButtonStyle_Sticky);

        mpWidget->SetCapture(eFalse);
        const sVec2f& vMousePos = ((Var&)varParam0).GetVec2f();
        ProcessUnPush(vMousePos);
        if (isOver && canClick)
          ProcessClick(vMousePos);
      }
      break;
    }
    case eUIMessage_LeftDoubleClick:
      {
        if (mbPressed) {
          mbPressed = 2; // Click should not be sent after double click
        }
        break;
      }
    case eUIMessage_Visible: {
      if (!mpWidget->GetVisible()) {
        // if we hide the button we make sure that its pressed state become false,
        // since the widget won't receive the 'left click up' event anymore.
        mbPressed = eFalse;
      }
      break;
    }
    case eUIMessage_SerializeWidget: {
      QPtr<iDataTable> ptrDT = varParam0;
      if (ptrDT.IsOK()) {
        tU32 nFlags = varParam1.mU32;
        if (nFlags & eWidgetSerializeFlags_Write) {
          ptrDT->SetString(_A("group_id"),niHStr(mhspGroupID));
          ptrDT->SetInt(_A("checked"),GetCheck());
        }
        else if (nFlags & eWidgetSerializeFlags_Read) {
          tU32 nIndex = ptrDT->GetPropertyIndex(_A("group_id"));
          if (nIndex != eInvalidHandle)
            mhspGroupID = _H(ptrDT->GetStringFromIndex(nIndex));
          nIndex = ptrDT->GetPropertyIndex(_A("checked"));
          if (nIndex != eInvalidHandle) {
            SetCheck(ptrDT->GetBoolFromIndex(nIndex));
          }
          nIndex = ptrDT->GetPropertyIndex(_A("icon_path"));
          if (nIndex != eInvalidHandle) {
            tHStringPtr path = _H(ptrDT->GetStringFromIndex(nIndex));
            Ptr<iOverlay> o = mpWidget->GetUIContext()->GetGraphics()->CreateOverlayResource(path);
            if (o.IsOK()) {
              o->SetBlendMode(eBlendMode_Translucent);
              o->SetFiltering(eTrue);
              this->SetIcon(o);
            }
          }
          nIndex = ptrDT->GetPropertyIndex(_A("icon_size"));
          if (nIndex != eInvalidHandle && mptrIconNormal.IsOK()) {
            mptrIconNormal->SetSize(ptrDT->GetVec2FromIndex(nIndex));
          }
        }
      }
      break;
    }
    case eUIMessage_StyleChanged:
    case eUIMessage_SkinChanged: {
      InitSkin();
      return eTrue;
    }
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cButtonWidget::SetGroupID(iHString* aVal)
{
  mhspGroupID = aVal;
  return eTrue;
}

///////////////////////////////////////////////
iHString* __stdcall cButtonWidget::GetGroupID() const
{
  return mhspGroupID;
}

///////////////////////////////////////////////
void cButtonWidget::InitSkin() {

  tHStringPtr hspSkinClass = NULL;
  tU32 style = mpWidget->GetStyle();
  if (niFlagIs(style,eWidgetButtonStyle_CheckBox)) {
    hspSkinClass = _H("CheckBox");
  }
  else if (niFlagIs(style,eWidgetButtonStyle_RadioButton)) {
    hspSkinClass = _H("RadioButton");
  }
  else if (niFlagIs(style,eWidgetButtonStyle_TabButton)) {
    hspSkinClass = _H("TabButton");
  }
  else {
    // nothing use default skin class
  }
  skin.normalFrame  = mpWidget->FindSkinElement (hspSkinClass,NULL,_H("Frame"));
  skin.normalFont   = mpWidget->FindSkinFont  (hspSkinClass,NULL,_H("Default"));
  skin.pressedFrame = mpWidget->FindSkinElement (hspSkinClass,_H("Pressed"),_H("Frame"));
  skin.pressedFont  = mpWidget->FindSkinFont  (hspSkinClass,_H("Pressed"),_H("Default"));
  skin.hoverFrame   = mpWidget->FindSkinElement (hspSkinClass,_H("Hover"),_H("Frame"));
  skin.hoverFont    = mpWidget->FindSkinFont  (hspSkinClass,_H("Hover"),_H("Default"));

  skin.mark = mpWidget->FindSkinElement(hspSkinClass,NULL,_H("Mark"));

  skin._kcolBorderColor = ULColorBuild(mpWidget->FindSkinColor(Vec4<tF32>(1,1,1,1),hspSkinClass,_H("Pressed"),_H("Border")));
  skin._kcolPressedBack = ULColorBuild(mpWidget->FindSkinColor(Vec4<tF32>(0,0,0,1),hspSkinClass,_H("Pressed"),_H("Background")));
}

///////////////////////////////////////////////
void __stdcall cButtonWidget::SetIconMargin(const sVec4f& avMargin) {
  mvIconMargin = avMargin;
}
sVec4f __stdcall cButtonWidget::GetIconMargin() const {
  return mvIconMargin;
}

///////////////////////////////////////////////
void cButtonWidget::_GetStatusItems(iOverlay** appFrame, iFont** appFont, iOverlay** appIcon, tBool* apbIsPressed) const
{
  tBool bOver = mpWidget->GetIsMouseOver();

  if (apbIsPressed) *apbIsPressed = ni::eFalse;

  if (mbChecked ||
      (mbPressed &&
       (bOver || niFlagIs(mpWidget->GetStyle(),eWidgetButtonStyle_Sticky))))
  {
    if (apbIsPressed) *apbIsPressed = ni::eTrue;
    if (appFrame) *appFrame = skin.pressedFrame;
    if (appFont) *appFont = skin.pressedFont;
    if (appIcon) *appIcon = mptrIconPressed;
  }
  else if (bOver) {
    if (appFrame) *appFrame = skin.hoverFrame;
    if (appFont) *appFont = skin.hoverFont;
    if (appIcon) *appIcon = mptrIconHover;
  }
  else {
    if (appFrame) *appFrame = skin.normalFrame;
    if (appFont) *appFont = skin.normalFont;
    if (appIcon) *appIcon = mptrIconNormal;
  }
  if (appIcon && !*appIcon) {
    *appIcon = mptrIconNormal;
  }
}
