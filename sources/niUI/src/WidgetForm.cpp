// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetForm.h"
#include "UIContext.h"
#include "niUI_HString.h"

#define DRAG_NONE   0
#define DRAG_NORMAL   niBit(0)
#define DRAG_DOCK   niBit(1)
#define DRAG_DOCK_BEGAN niBit(2)

#define FORM_DEBUG //niPrintln

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetForm implementation.

///////////////////////////////////////////////
cWidgetForm::cWidgetForm(iWidget* apWidget)
{
  niAssert(niIsOK(apWidget));
  ZeroMembers();
  mpWidget = (cWidget*)apWidget;
  PushBeforeDock();
  UpdateStyle();
}

///////////////////////////////////////////////
cWidgetForm::~cWidgetForm()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetForm::ZeroMembers()
{
  mpWidget = NULL;
  ni::MemZero((tPtr)&mBeforeDock,sizeof(mBeforeDock));
  mhspTitle = _H(AZEROSTR);
  mfResizeBorderArea = 10.0f;
  mnDrag = DRAG_NONE;
  mResizeArea = eResizeArea_None;
  mbIsDocked = eFalse;
  mbActive = eFalse;
  mFormFrameFlags = eRectFrameFlags_Edges;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetForm::IsOK() const
{
  niClassIsOK(cWidgetForm);
  return mpWidget != NULL;
}

///////////////////////////////////////////////
iUnknown* __stdcall cWidgetForm::QueryInterface(const tUUID& aIID)
{
  return BaseImpl::QueryInterface(aIID);
}

///////////////////////////////////////////////
void __stdcall cWidgetForm::ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const
{
  BaseImpl::ListInterfaces(apLst,anFlags);
}

///////////////////////////////////////////////
tBool cWidgetForm::SetTitle(const achar *aVal)
{
  mhspTitle = _H(aVal);
  return eTrue;
}

///////////////////////////////////////////////
const achar * cWidgetForm::GetTitle() const
{
  return niHStr(mhspTitle);
}

///////////////////////////////////////////////
tBool __stdcall cWidgetForm::SetResizeBorderArea(tF32 aVal)
{
  mfResizeBorderArea = aVal;
  return eTrue;
}

///////////////////////////////////////////////
tF32 __stdcall cWidgetForm::GetResizeBorderArea() const
{
  return mfResizeBorderArea;
}

///////////////////////////////////////////////
const achar* __stdcall cWidgetForm::GetDockName() const
{
  return GetTitle();
}

///////////////////////////////////////////////
tBool __stdcall cWidgetForm::GetIsDocked() const
{
  return mbIsDocked;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetForm::OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB)
{
  niGuardObject((iWidgetSink*)this);
  mpWidget = (cWidget*)apWidget;
  switch (anMsg) {
    case eUIMessage_Destroy: {
      if (mbIsDocked) {
        QPtr<iWidgetDockingManager> ptrDockMan = mpwDockedTo;
        if (ptrDockMan.IsOK()) {
          ptrDockMan->UndockWidget(mpWidget);
        }
      }
      break;
    }
    case eUIMessage_SkinChanged:
    case eUIMessage_StyleChanged:
      {
        UpdateStyle();
        return eFalse;
      }
    case eUIMessage_Wheel:
    case eUIMessage_NCWheel:
      {
        // Dont propagate the wheel to parents further
        return eTrue;
      }
    case eUIMessage_Command:
      {
        Ptr<iWidgetCommand> ptrCmd = ni::VarQueryInterface<iWidgetCommand>(aA);
        niAssert(ptrCmd.IsOK());
        if (ptrCmd->GetSender()->GetID() == _HC(form_button_Close)) {
          if (ptrCmd->GetID() == eWidgetButtonCmd_Clicked) {
            niGuardObject(mpWidget);
            mpWidget->Destroy();
          }
        }
        //return eFalse;
        // Dont propagate the command to parents further
        return eTrue;
      };
    case eUIMessage_Cut:
    case eUIMessage_Copy:
    case eUIMessage_Paste:
      {
        // dont propagate the cut/copy/paste messages further
        return eTrue;
      };
    case eUIMessage_Submit:
      { // Dont propagate the submit to parents further
        return eTrue;
      };
    case eUIMessage_BeforeDocked:
    case eUIMessage_Undocked:
      {
        const tBool bDocked = (anMsg != eUIMessage_Undocked);
        if (bDocked != mbIsDocked) {
          mbIsDocked = bDocked;
          if (bDocked) {
            if (!mnDrag)
              PushBeforeDock();
            mpWidget->SetStyle(mpWidget->GetStyle()&~(eWidgetStyle_Free|eWidgetStyle_NoClip));
            mpWidget->SetAutoLayout(mpWidget->GetAutoLayout()&(~eWidgetAutoLayoutFlags_Size));
            mpWidget->SetZOrder(eWidgetZOrder_Bottom);
            {
              QPtr<iWidget> ptrDockedTo = aA.GetIUnknownPointer();
              mpwDockedTo = ptrDockedTo.ptr();
            }
            if (mptrButtonClose.IsOK()) {
              mptrButtonClose->SetStatus(eFalse,eFalse,eTrue);
            }
          }
          else {
            PopBeforeDock();
            mpWidget->SetZOrder(eWidgetZOrder_Top);
            mpwDockedTo.SetNull();
            if (mptrButtonClose.IsOK()) {
              mptrButtonClose->SetStatus(eTrue,eTrue,eFalse);
            }
          }
        }
        break;
      }
    case eUIMessage_NCPaint:
      {
        iCanvas* c = VarQueryInterface<iCanvas>(aB);
        if (!c) return eFalse;

        if (mbIsDocked) {
          const sColor4f& bgColor = skin.backgroundColor;
          if (bgColor.w > 0) {
            c->BlitFillAlpha(mpWidget->GetWidgetRect(),ULColorBuild(bgColor));
          }
        }
        else {
          iOverlay* frame = mbActive ? skin.normalFrame : skin.inactiveFrame;
          iFont* font = mbActive ? skin.normalFont : skin.inactiveFont;
          if (frame && c && font) {
            c->BlitOverlayFrame(
                mpWidget->GetWidgetRect(),
                frame,mFormFrameFlags);
            if (!niFlagIs(mFormFrameFlags,eRectFrameFlags_Center)) {
              const sColor4f& bgColor = skin.backgroundColor;
              if (bgColor.w > 0) {
                const sRectf clearRect = frame->ComputeFrameCenter(mpWidget->GetWidgetRect());
                c->BlitFillAlpha(clearRect,ULColorBuild(bgColor));
              }
            }
            c->BlitText(
                font,skin.titleTextFrame,
                eFontFormatFlags_CenterV,
                niHStr(mpWidget->FindLocalized(mhspTitle)));
          }
        }
        break;
      }
    case eUIMessage_Paint:
      {
        break;
      }
    case eUIMessage_Padding:
    case eUIMessage_NCSize:
      {
        UpdateClientSize();
        FitInParentRect();
        break;
      }
    case eUIMessage_Activate:
      {
        //             niDebugFmt((_A("ACTIVATE FORM: %s\n"),niHStr(mhspTitle)));
        mpWidget->SetZOrder(eWidgetZOrder_Top);
        mbActive = eTrue;
        if (mbIsDocked) {
          iWidget* p = mpWidget->GetParent();
          while (p) {
            QPtr<iWidgetTab> tab = p;
            if (tab.IsOK()) {
              tab->SetForceActivated(eTrue);
              break;
            }
            p = p->GetParent();
          }
        }
        break;
      }
    case eUIMessage_Deactivate:
      {
        if (mbIsDocked) {
          iWidget* p = mpWidget->GetParent();
          while (p) {
            QPtr<iWidgetTab> tab = p;
            if (tab.IsOK()) {
              tab->SetForceActivated(eFalse);
              break;
            }
            p = p->GetParent();
          }
        }
        mbActive = eFalse;
        break;
      }
    case eUIMessage_NCDragBegin:
      {
        // This is here so that docking is started only if moving of enought thresold,
        // so that just clicking on the title bar doesnt display the docking manager
        // overlays.
        // We can't dock the window, if we can't move it...
        if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_Move)) {
          _SetDrag(mnDrag|DRAG_NORMAL|DRAG_DOCK);
          _HandleDragDock(eFalse);
        }
        break;
      }
    case eUIMessage_NCLeftClickDown:
      {
        if (mResizeArea != eResizeArea_None ||
            (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_Move) &&
             mpWidget->GetWidgetRect().Intersect(Vec2<tF32>(aB.mV2F))))
        {
          mvPrevMousePos = mpWidget->GetUIContext()->GetCursorPosition();
          _SetDrag(DRAG_NORMAL);
          mpWidget->SetCapture(eTrue);
          PushBeforeDock();
        }
        break;
      }
    case eUIMessage_LeftClickUp:
      {
        if (!mnDrag)
          return eFalse;
        niFallthrough;
      }
    case eUIMessage_NCLeftClickUp:
      {
        if (mnDrag) {
          _HandleDragDock(eTrue);
          mpWidget->SetCapture(eFalse);
          _SetDrag(DRAG_NONE);
          apWidget->SendMessage(eUIMessage_SetCursor,aA,aB);
        }
        break;
      }
    case eUIMessage_NCMouseMove:
    case eUIMessage_MouseMove:
      {
        if (mnDrag) {
          const sVec2f absMousePos = apWidget->GetUIContext()->GetCursorPosition();
          if (mResizeArea == eResizeArea_None) {
            sVec2f delta = absMousePos-mvPrevMousePos;
            _HandleDragDock(eFalse,&absMousePos);
            sVec2f vAbsPos = apWidget->GetAbsolutePosition()+delta;
            apWidget->SetAbsolutePosition(vAbsPos);
          }
          else
          {
            const sVec2f mousepos = absMousePos;
            sRectf absRect = apWidget->GetAbsoluteRect();
            switch (mResizeArea) {
              case eResizeArea_TopLeft:
                absRect.SetTopLeft(mousepos);
                break;
              case eResizeArea_TopRight:
                absRect.SetTopRight(mousepos);
                break;
              case eResizeArea_BottomLeft:
                absRect.SetBottomLeft(mousepos);
                break;
              case eResizeArea_BottomRight:
                absRect.SetBottomRight(mousepos);
                break;
              case eResizeArea_Left:
                absRect.SetLeft(mousepos.x);
                break;
              case eResizeArea_Right:
                absRect.SetRight(mousepos.x);
                break;
              case eResizeArea_Top:
                absRect.SetTop(mousepos.y);
                break;
              case eResizeArea_Bottom:
                absRect.SetBottom(mousepos.y);
                break;
              case eResizeArea_Title:
              case eResizeArea_None:
                // Do nothing
                break;
            }
            absRect.SetLeft(ni::Min(absRect.x,mBeforeDock.mRect.z-mpWidget->GetMinimumSize().x));
            absRect.SetTop(ni::Min(absRect.y,mBeforeDock.mRect.w-mpWidget->GetMinimumSize().y));
            apWidget->SetAbsoluteRect(absRect);
          }
          mvPrevMousePos = absMousePos;
        }
      }
      break;
    case eUIMessage_SetCursor:
      {
        if (mnDrag) {
          apWidget->GetUIContext()->SetCursor(apWidget->FindSkinCursor(NULL,NULL,_H("Arrow")));
          return eTrue;
        }

        mResizeArea = eResizeArea_None;

        sVec2f vMousePos = *((sVec2f*)aB.mV2F);
        if (niFlagIs(mpWidget->GetAutoLayout(),eWidgetAutoLayoutFlags_Size)
            || mpWidget->GetClientRect().Intersect(vMousePos)) {
          return eFalse;
        }

        sVec2f vAbsMousePos = vMousePos+mpWidget->GetAbsolutePosition();
        sRectf rectAbs = mpWidget->GetAbsoluteRect();
        mvMinSize = mpWidget->GetMinimumSize();

        // top left corner
        if (sRectf(rectAbs.GetCorner(eRectCorners_TopLeft)+Vec2(-(mfResizeBorderArea/2),-(mfResizeBorderArea/2)),mfResizeBorderArea,mfResizeBorderArea).Intersect(vAbsMousePos))
        {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_ResizeTopLeftCorner)) {
            apWidget->GetUIContext()->SetCursor(skin.curResizeTL);
            mResizeArea = eResizeArea_TopLeft;
          }
        }
        // top right corner
        else if (sRectf(rectAbs.GetCorner(eRectCorners_TopRight)+Vec2(-(mfResizeBorderArea/2),-(mfResizeBorderArea/2)),mfResizeBorderArea,mfResizeBorderArea).Intersect(vAbsMousePos))
        {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_ResizeTopRightCorner)) {
            apWidget->GetUIContext()->SetCursor(skin.curResizeTR);
            mResizeArea = eResizeArea_TopRight;
          }
        }
        // bottom left corner
        else if (sRectf(rectAbs.GetCorner(eRectCorners_BottomLeft)+Vec2(-(mfResizeBorderArea/2),-(mfResizeBorderArea/2)),mfResizeBorderArea,mfResizeBorderArea).Intersect(vAbsMousePos))
        {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_ResizeBottomLeftCorner)) {
            apWidget->GetUIContext()->SetCursor(skin.curResizeTR);
            mResizeArea = eResizeArea_BottomLeft;
          }
        }
        // bottom right corner
        else if (sRectf(rectAbs.GetCorner(eRectCorners_BottomRight)+Vec2(-(mfResizeBorderArea/2),-(mfResizeBorderArea/2)),mfResizeBorderArea,mfResizeBorderArea).Intersect(vAbsMousePos))
        {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_ResizeBottomRightCorner)) {
            apWidget->GetUIContext()->SetCursor(skin.curResizeTL);
            mResizeArea = eResizeArea_BottomRight;
          }
        }
        // left edge
        else if (sRectf(rectAbs.GetLeft()-(mfResizeBorderArea/2),rectAbs.GetTop(),mfResizeBorderArea,rectAbs.GetHeight()).Intersect(vAbsMousePos))
        {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_ResizeLeftEdge)) {
            apWidget->GetUIContext()->SetCursor(skin.curResizeHz);
            mResizeArea = eResizeArea_Left;
          }
        }
        // right edge
        else if (sRectf(rectAbs.GetRight()-(mfResizeBorderArea/2),rectAbs.GetTop(),mfResizeBorderArea,rectAbs.GetHeight()).Intersect(vAbsMousePos))
        {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_ResizeRightEdge)) {
            apWidget->GetUIContext()->SetCursor(skin.curResizeHz);
            mResizeArea = eResizeArea_Right;
          }
        }
        // top edge
        else if (sRectf(rectAbs.GetLeft(),rectAbs.GetTop()-(mfResizeBorderArea/2),rectAbs.GetWidth(),mfResizeBorderArea).Intersect(vAbsMousePos))
        {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_ResizeTopEdge)) {
            apWidget->GetUIContext()->SetCursor(skin.curResizeVt);
            mResizeArea = eResizeArea_Top;
          }
        }
        // bottom edge
        else if (sRectf(rectAbs.GetLeft(),rectAbs.GetBottom()-(mfResizeBorderArea/2),rectAbs.GetWidth(),mfResizeBorderArea).Intersect(vAbsMousePos))
        {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_ResizeBottomEdge)) {
            apWidget->GetUIContext()->SetCursor(skin.curResizeVt);
            mResizeArea = eResizeArea_Bottom;
          }
        }
        else
          return eFalse;
        break;
      }
    case eUIMessage_SerializeWidget:
      {
        Ptr<iDataTable> dt = ni::VarQueryInterface<iDataTable>(aA);
        niAssert(dt.IsOK());
        if (dt.IsOK()) {
          const tU32 nFlags = aB.GetU32();
          if (niFlagIs(nFlags,eWidgetSerializeFlags_Read)) {
            if (dt->GetPropertyIndex(_A("title")) != eInvalidHandle) {
              mhspTitle = _H(dt->GetString(_A("title")));
            }

            Ptr<iDataTable> dtBeforeDock = dt->GetChild(_A("BeforeDock"));
            if (dtBeforeDock.IsOK()) {
              mBeforeDock.mRect = dtBeforeDock->GetVec4(_A("rect"));
              mBeforeDock.mnStyle = dtBeforeDock->GetInt(_A("style"));
              mBeforeDock.mDockStyle = (eWidgetDockStyle)dtBeforeDock->GetInt(_A("dock_style"));
              mBeforeDock.mbEnabled = dtBeforeDock->GetBool(_A("enabled"));
              mBeforeDock.mbVisible = dtBeforeDock->GetBool(_A("visible"));
              mBeforeDock.mbIgnoreInput = dtBeforeDock->GetBool(_A("ignore_input"));
              mBeforeDock.mZOrder = (eWidgetZOrder)dtBeforeDock->GetInt(_A("zorder"));
            }
          }
          else {
            dt->SetString(_A("title"),niHStr(mhspTitle));

            Ptr<iDataTable> dtBeforeDock = ni::CreateDataTable(_A("BeforeDock"));
            dtBeforeDock->SetVec4(_A("rect"),mBeforeDock.mRect);
            dtBeforeDock->SetInt(_A("style"),mBeforeDock.mnStyle);
            dtBeforeDock->SetInt(_A("dock_style"),mBeforeDock.mDockStyle);
            dtBeforeDock->SetInt(_A("enabled"),mBeforeDock.mbEnabled);
            dtBeforeDock->SetInt(_A("visible"),mBeforeDock.mbVisible);
            dtBeforeDock->SetInt(_A("ignore_input"),mBeforeDock.mbIgnoreInput);
            dtBeforeDock->SetInt(_A("zorder"),mBeforeDock.mZOrder);
            dt->AddChild(dtBeforeDock);
          }
        }
        break;
      }
    case eUIMessage_ContextResized:
      {
        FitInParentRect();
        break;
      }
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
void cWidgetForm::PushBeforeDock()
{
  mBeforeDock.mRect = mpWidget->GetAbsoluteRect();
  mBeforeDock.mnStyle = mpWidget->GetStyle();
  mBeforeDock.mDockStyle = mpWidget->GetDockStyle();
  mBeforeDock.mbEnabled = mpWidget->GetEnabled();
  mBeforeDock.mbVisible = mpWidget->GetVisible();
  mBeforeDock.mbIgnoreInput = mpWidget->GetIgnoreInput();
  mBeforeDock.mZOrder = mpWidget->GetZOrder();
}

///////////////////////////////////////////////
void cWidgetForm::PopBeforeDock()
{
  mpWidget->SetDockStyle(mBeforeDock.mDockStyle);
  mpWidget->SetStyle(mBeforeDock.mnStyle);
  mpWidget->SetZOrder(mBeforeDock.mZOrder);
  mpWidget->SetEnabled(mBeforeDock.mbEnabled);
  mpWidget->SetVisible(mBeforeDock.mbVisible);
  mpWidget->SetIgnoreInput(mBeforeDock.mbIgnoreInput);
  mpWidget->SetAbsoluteRect(mBeforeDock.mRect);
}

///////////////////////////////////////////////
tBool cWidgetForm::CanDock() const
{
  return !mpWidget->GetExclusive() &&
      mpWidget->GetParent() &&
      niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_Dock);
}

///////////////////////////////////////////////
void cWidgetForm::UpdateStyle()
{
  skin.backgroundColor = mpWidget->FindSkinColor(sColor4f::Zero(),NULL,NULL,_H("Background"));

  skin.normalFrame = mpWidget->FindSkinElement(NULL,NULL,_H("Frame"));
  skin.normalFont = mpWidget->FindSkinFont(NULL,NULL,_H("Default"));
  skin.inactiveFrame = mpWidget->FindSkinElement(NULL,_H("Inactive"),_H("Frame"));
  skin.inactiveFont = mpWidget->FindSkinFont(NULL,_H("Inactive"),_H("Default"));

  skin.curResizeTL = mpWidget->FindSkinCursor(NULL,NULL,_H("ResizeTopLeft"));
  skin.curResizeTR = mpWidget->FindSkinCursor(NULL,NULL,_H("ResizeTopRight"));
  skin.curResizeHz = mpWidget->FindSkinCursor(NULL,NULL,_H("ResizeHorizontal"));
  skin.curResizeVt = mpWidget->FindSkinCursor(NULL,NULL,_H("ResizeVertical"));

  {
    Ptr<iOverlay> ptrTitleFrame = mpWidget->FindSkinElement(NULL,NULL,_H("TitleFrame"));
    skin.titleHeight = skin.normalFrame->GetFrame().y;
    if (ptrTitleFrame != mpWidget->GetUIContext()->GetErrorOverlay()) {
      skin.titleFrame = ptrTitleFrame->GetFrame();
    }
    else {
      skin.titleFrame = skin.normalFrame->GetFrame();
      skin.titleFrame.SetTop(skin.titleFrame.GetBottom());
    }
  }
  {
    Ptr<iOverlay> ptrTitleTextFrame = mpWidget->FindSkinElement(NULL,NULL,_H("TitleTextFrame"));
    if (ptrTitleTextFrame != mpWidget->GetUIContext()->GetErrorOverlay()) {
      skin.titleTextFrame = ptrTitleTextFrame->GetFrame();
    }
    else {
      skin.titleTextFrame = skin.normalFrame->GetFrame();
      skin.titleTextFrame.SetTop(skin.titleTextFrame.GetBottom());
    }
  }
  skin.titleHeight = ni::Max(skin.titleHeight-(skin.titleFrame.GetTop()+skin.titleFrame.GetBottom()),5);

  if (niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_CloseButton)) {
    Ptr<iOverlay> ptrTitleFrame = mpWidget->FindSkinElement(NULL,NULL,_H("TitleFrame"));
    if (!mptrButtonClose.IsOK()) {
      mptrButtonClose = mpWidget->GetUIContext()->CreateWidget(_A("Button"), mpWidget,
                                                             sRectf(0,0,10,10),
                                                             eWidgetStyle_NCRelative|
                                                             eWidgetStyle_DontSerialize,
                                                             _HC(form_button_Close));
    }
    mptrButtonClose->SetSkinClass(_H("ButtonClose"));
    mptrButtonClose->SetSize(Vec2(skin.titleHeight,skin.titleHeight));
    mptrButtonClose->SetMargin(
        Vec4<tF32>(0,
                      skin.titleFrame.GetTop(),
                      skin.titleFrame.GetRight(),
                      skin.titleFrame.GetBottom()));
    mptrButtonClose->SetDockStyle(eWidgetDockStyle_SnapTopRight);
    if (mbIsDocked) {
      mptrButtonClose->SetStatus(eFalse,eFalse,eTrue);
    }
    else {
      mptrButtonClose->SetStatus(eTrue,eTrue,eFalse);
    }
  }
  else {
    if (mptrButtonClose.IsOK())
      mptrButtonClose->Destroy();
    mptrButtonClose = NULL;
  }

  mpWidget->SetMinimumSize(Vec2f(200,80));
  UpdateClientSize();
}

///////////////////////////////////////////////
cWidgetForm::eResizeArea cWidgetForm::FindResizeArea(const sVec2f& aNCMousePos) const
{
  sVec2f vMousePos = aNCMousePos;
  eResizeArea resizeArea = eResizeArea_None;

  sVec2f vAbsMousePos = vMousePos+mpWidget->GetAbsolutePosition();
  sRectf rectAbs = mpWidget->GetAbsoluteRect();
  iFont* font = skin.normalFont;

  // title
  if (sRectf(rectAbs.GetCorner(eRectCorners_TopLeft)+Vec2<tF32>(1,1),rectAbs.GetWidth()-2.0f,font->GetHeight()+2).Intersect(vAbsMousePos)) {
    resizeArea = eResizeArea_Title;
  }
  // top left corner
  else if (sRectf(rectAbs.GetCorner(eRectCorners_TopLeft)+Vec2<tF32>(-(mfResizeBorderArea/2),-(mfResizeBorderArea/2)),mfResizeBorderArea,mfResizeBorderArea).Intersect(vAbsMousePos))
  {
    resizeArea = eResizeArea_TopLeft;
  }
  // top right corner
  else if (sRectf(rectAbs.GetCorner(eRectCorners_TopRight)+Vec2<tF32>(-(mfResizeBorderArea/2),-(mfResizeBorderArea/2)),mfResizeBorderArea,mfResizeBorderArea).Intersect(vAbsMousePos))
  {
    resizeArea = eResizeArea_TopRight;
  }
  // bottom left corner
  else if (sRectf(rectAbs.GetCorner(eRectCorners_BottomLeft)+Vec2<tF32>(-(mfResizeBorderArea/2),-(mfResizeBorderArea/2)),mfResizeBorderArea,mfResizeBorderArea).Intersect(vAbsMousePos))
  {
    resizeArea = eResizeArea_BottomLeft;
  }
  // bottom right corner
  else if (sRectf(rectAbs.GetCorner(eRectCorners_BottomRight)+Vec2<tF32>(-(mfResizeBorderArea/2),-(mfResizeBorderArea/2)),mfResizeBorderArea,mfResizeBorderArea).Intersect(vAbsMousePos))
  {
    resizeArea = eResizeArea_BottomRight;
  }
  // left edge
  else if (sRectf(rectAbs.GetLeft()-(mfResizeBorderArea/2),rectAbs.GetTop(),mfResizeBorderArea,rectAbs.GetHeight()).Intersect(vAbsMousePos))
  {
    resizeArea = eResizeArea_Left;
  }
  // right edge
  else if (sRectf(rectAbs.GetRight()-(mfResizeBorderArea/2),rectAbs.GetTop(),mfResizeBorderArea,rectAbs.GetHeight()).Intersect(vAbsMousePos))
  {
    resizeArea = eResizeArea_Right;
  }
  // top edge
  else if (sRectf(rectAbs.GetLeft(),rectAbs.GetTop()-(mfResizeBorderArea/2),rectAbs.GetWidth(),mfResizeBorderArea).Intersect(vAbsMousePos))
  {
    resizeArea = eResizeArea_Top;
  }
  // bottom edge
  else if (sRectf(rectAbs.GetLeft(),rectAbs.GetBottom()-(mfResizeBorderArea/2),rectAbs.GetWidth(),mfResizeBorderArea).Intersect(vAbsMousePos))
  {
    resizeArea = eResizeArea_Bottom;
  }

  return resizeArea;
}

///////////////////////////////////////////////
void cWidgetForm::UpdateClientSize()
{
  if (mbIsDocked) {
    sVec2f size = mpWidget->GetSize();
    sVec4f vPadding = mpWidget->GetPadding();
    sRectf clientRect(0,0,size.x,size.y);
    clientRect.Left() += vPadding.Left();
    clientRect.Right() -= vPadding.Right();
    clientRect.Top() += vPadding.Top();
    clientRect.Bottom() -= vPadding.Bottom();
    mpWidget->SetClientRect(clientRect);
  }
  else {
    iOverlay* frame = skin.normalFrame;
    sVec2f size = mpWidget->GetSize();
    sVec4f vPadding = mpWidget->GetPadding();
    sRectf clientRect = frame->ComputeFrameCenter(sRectf(0,0,size.x,size.y));
    clientRect.Left() += vPadding.Left();
    clientRect.Right() -= vPadding.Right();
    clientRect.Top() += vPadding.Top();
    clientRect.Bottom() -= vPadding.Bottom();
    mpWidget->SetClientRect(clientRect);
  }
}

///////////////////////////////////////////////
void cWidgetForm::FitInParentRect()
{
  mpWidget->SnapInside(mpWidget->GetParent(),0.0f);
}

///////////////////////////////////////////////
void cWidgetForm::_SetDrag(tU32 anDrag) {
  tBool bWasDockBegan = niFlagIs(mnDrag,DRAG_DOCK_BEGAN);
  mnDrag = anDrag;
  niFlagOnIf(mnDrag,DRAG_DOCK_BEGAN,bWasDockBegan);
}

///////////////////////////////////////////////
void cWidgetForm::_HandleDragDock(tBool abEnd, const sVec2f* apMousePos) {

  tBool bInputCanDock = eTrue;
  if (!niFlagIs(mpWidget->GetStyle(),eWidgetFormStyle_DockNoInputModifier)) {
    bInputCanDock = niFlagIs(mpWidget->GetUIContext()->GetInputModifiers(),
                             eUIInputModifier_Control);
    if (mnDrag&DRAG_DOCK_BEGAN) {
      if (!bInputCanDock)
        abEnd = eTrue;
    }
  }

  if (!abEnd) {
    if (bInputCanDock &&
        (!(mnDrag&DRAG_DOCK_BEGAN)) &&
        (mnDrag&DRAG_DOCK) &&
        (mResizeArea == eResizeArea_None) &&
        CanDock())
    {
      mnDrag |= DRAG_DOCK_BEGAN;
      ni::SendMessage(mpWidget->GetParent(),
                      eWidgetDockingManagerMessage_BeginMove,
                      niVarNull,niVarNull);
    }
  }

  if (apMousePos) {
    if (mnDrag & DRAG_DOCK_BEGAN) {
      ni::SendMessage(mpWidget->GetParent(),
                      eWidgetDockingManagerMessage_Move,
                      *apMousePos,
                      niVarNull);
    }
  }

  if (abEnd) {
    if (mnDrag & DRAG_DOCK_BEGAN) {
      if (mResizeArea == eResizeArea_None && CanDock()) {
        ni::SendMessage(mpWidget->GetParent(),
                        eWidgetDockingManagerMessage_EndMove,
                        mpWidget,niVarNull);
      }
    }
    mnDrag &= ~DRAG_DOCK_BEGAN;
  }
}

///
/// Title widgets
///

/////////////////////////////////////////////////////////////////
tU32 __stdcall cWidgetForm::GetNumTitleWidgets() const {
  return 0;
}


/////////////////////////////////////////////////////////////////
iWidget* __stdcall cWidgetForm::GetTitleWidget(tU32 anIndex) const {
  return NULL;
}


/////////////////////////////////////////////////////////////////
tU32 __stdcall cWidgetForm::GetTitleWidgetIndex(iWidget* apWidget) const {
  return eInvalidHandle;
}


/////////////////////////////////////////////////////////////////
iWidget* __stdcall cWidgetForm::GetTitleWidgetFromID(iHString* ahspID) const {
  return NULL;
}


/////////////////////////////////////////////////////////////////
tBool __stdcall cWidgetForm::AddTitleWidget(iWidget* apWidget, tF32 afWidth) {
  return ni::eFalse;
}

/////////////////////////////////////////////////////////////////
void __stdcall cWidgetForm::SetFormFrameFlags(tRectFrameFlags aFlags) {
  mFormFrameFlags = aFlags;
}
tRectFrameFlags __stdcall cWidgetForm::GetFormFrameFlags() const {
  return mFormFrameFlags;
}
