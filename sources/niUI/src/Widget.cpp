// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "UIContext.h"
#include "Widget.h"
#include "niUI_HString.h"
#include <niLang/Utils/ThreadImpl.h>
#include <niLang/Utils/UnitSnap.h>
#include "API/niUI_ModuleDef.h"

#pragma niTodo("Expose GetMessageTargetByPos in the API.")
#pragma niTodo("Make the Get*Clipped*Rect methos properties.")
#pragma niTodo("Fix the clipping issues with the scrollbar in the control center.")
#pragma niTodo("Rename Add/Remove capture, it's not a stack. Actually it should be a list but locally for one widget it's not a stack.")
#pragma niTodo("Rename Add/Remove exclusive, it's not a stack. Actually it should be a list but locally for one widget it's not a stack.")
#pragma niTodo("eUIMessage_SetText can be sent ONLY from GetText/Serialize from the widget itself, it CANT be sent by the user, need to add some flags and another message handler to secure this.")

#define MAX_PUSH_STYLE 16

#define CheckValid(RET)   if (niFlagIs(mStatus,WDGSTATUS_INVALID)) { return RET; }

static inline void _RoundRect(sRectf& rect) {
  rect.MoveTo(ni::UnitSnapf(rect.GetTopLeft()));
}
static inline void _RoundPos(sVec2f& pos) {
  pos = ni::UnitSnapf(pos);
}

enum {
  MarginLeft = niBit(7),
  MarginRight = niBit(8),
  MarginTop = niBit(9),
  MarginBottom = niBit(10),
  MarginAll = MarginLeft|MarginRight|MarginTop|MarginBottom
};
// Margin moves the widget, never change its size
static inline sRectf& _ApplyMargin(sRectf& rect, const sVec4f& vMargin, tU32 flags) {
  // apply horizontal margin...
  if (niFlagIs(flags,MarginLeft)) {
    rect.x += vMargin.x;
    rect.z += vMargin.x;
  }
  if (niFlagIs(flags,MarginRight)) {
    rect.x -= vMargin.z;
    rect.z -= vMargin.z;
  }
  // apply vertical margin...
  if (niFlagIs(flags,MarginTop)) {
    rect.y += vMargin.y;
    rect.w += vMargin.y;
  }
  if (niFlagIs(flags,MarginBottom)) {
    rect.y -= vMargin.w;
    rect.w -= vMargin.w;
  }
  return rect;
}
// Padding changes the size of the widget
static inline sRectf& _ApplyPadding(sRectf& rect, const sVec4f& vPadding, tU32 flags) {
  if (niFlagIs(flags,MarginLeft)) {
    rect.x += vPadding.x;
  }
  if (niFlagIs(flags,MarginRight)) {
    rect.z -= vPadding.z;
  }
  if (niFlagIs(flags,MarginTop)) {
    rect.y += vPadding.y;
  }
  if (niFlagIs(flags,MarginBottom)) {
    rect.w -= vPadding.w;
  }
  return rect;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidget implementation.

///////////////////////////////////////////////
cWidget::cWidget(cUIContext* pUICtx, iHString* ahspClass, cWidget *pwParent, const sRectf &arectPos, tU32 anStyle, iHString* ahspID, iHString* ahspSkinClass, iHString* ahspText)
    : mpUICtx(pUICtx)
    , mnThreadID(ni::ThreadGetCurrentThreadID())
    , mlstSinks(tWidgetSinkLst::Create())
{
  niGuardConstructor(cWidget);

  mrectRelative = sRectf(0,0);
  mvMargin = sVec4f::Zero();
  mvPadding = sVec4f::Zero();
  mhspID = NULL;
  mStatus = 0;
  mZOrder = (eWidgetZOrder)eInvalidHandle;
  mDockStyle = eWidgetDockStyle_None;
  mvMinSize = _kvMinSize;
  mvMaxSize = _kvMaxSize;
  mnAutoLayout = eWidgetAutoLayoutFlags_Dock|eWidgetAutoLayoutFlags_Relative;
  mhspHoverText = _H("");

  mhspSkinClass = ahspSkinClass;
  mhspText = ahspText?ahspText:_H("").ptr();
  mhspClass = ahspClass;
  mnStyle = anStyle;
  mStatus = WDGSTATUS_ENABLED|WDGSTATUS_VISIBLE;
  if (GetClassName() == _H("Dummy"))
    mStatus |= WDGSTATUS_DUMMY;
  mClick = 0;
  mRect = arectPos;
  _RoundRect(mRect);
  mClientRect = arectPos;
  mpwParent = pwParent;
  mrectDockFillClient = mRect;
  mrectDockFillNonClient = mRect;
  mptrCanvas = NULL;
  mnInputSubmitFlags = pUICtx->GetDefaultInputSubmitFlags();
#ifdef USE_CACHED_ABS_RECT
  mrectAbs = mRect;
  mrectAbsClipped = mRect;
  mrectAbsClientClipped = mClientRect;
#endif
  SetID(ahspID);
  if (!_CheckSelfParent(pwParent)) {
    mStatus = WDGSTATUS_INVALID;
    niError(_A("Invalid parent set."));
  }
  mpUICtx->RegisterWidget(this);

#ifdef USE_CACHED_ABS_RECT
  _SetMoved(); // Make sure the initial update is done
#endif

  // at the end so that the parent is already set
  mhspLocalizedText = FindLocalized(mhspText);

  SetZOrder(eWidgetZOrder_Top);
}

///////////////////////////////////////////////
cWidget::~cWidget()
{
  niGuardConstructor(cWidget);
  Invalidate();
}

///////////////////////////////////////////////
void cWidget::Destroy()
{
  Invalidate();
}

///////////////////////////////////////////////
void cWidget::Invalidate()
{
  CheckValid(;);
  niGuardObject(this);

  //     niDebugFmt((_A("# INVALIDATE WIDGET: %p '%s' (%s)"),
  //               this,niHStr(mhspID),niHStr(mhspClass)));

  if (!niFlagIs(mStatus,WDGSTATUS_DESTROYED)) {
    // if we didnt receive the destroy message yet, we broadcast a
    // save config message to this widget and all its children...
    BroadcastMessage(eUIMessage_SaveConfig,niVarNull,niVarNull);
    // Send the destroy message to this and to all children
    _SendDestroyMessage();
  }

  mStatus = WDGSTATUS_INVALID;
  ClearSinks();

  // Invalidate all children
  mpwContextMenu = NULL;
  InvalidateChildren();

  // Detach completly from the parent
  DoSetParent(NULL);
  // Invalidate target and unregister
  mpUICtx->InvalidateTarget(this);
  mpUICtx->UnregisterWidget(this);

  // Final touch...
  mpUICtx = NULL;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidget::IsOK() const
{
  niClassIsOK(cWidget);
  CheckValid(ni::eFalse);
  return mStatus != WDGSTATUS_INVALID;
}

///////////////////////////////////////////////
iUnknown* __stdcall cWidget::QueryInterface(const tUUID& aIID)
{
  if (aIID == niGetInterfaceUUID(iUnknown))
    return static_cast<iWidget*>(this);
  if (aIID == niGetInterfaceUUID(iWidget))
    return static_cast<iWidget*>(this);
  if (aIID == niGetInterfaceUUID(iMessageHandler))
    return static_cast<iWidget*>(this);
  Ptr<tWidgetSinkLst::tImmutableCollection> coll = mlstSinks->_ImmutableCollection();
  if (coll.IsOK()) {
    iUnknown *ret;
    for(tWidgetSinkLst::const_reverse_iterator itr = coll->rbegin(); itr != coll->rend(); ++itr) {
      if((ret = (*itr)->QueryInterface(aIID))!=NULL)
        return ret;
    }
  }
  return NULL;
}

///////////////////////////////////////////////
void __stdcall cWidget::ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const
{
  if (!niFlagIs(anFlags,eListInterfacesFlags_NoDynamic)) {
    Ptr<tWidgetSinkLst::tImmutableCollection> coll = mlstSinks->_ImmutableCollection();
    if (coll.IsOK()) {
      for (tWidgetSinkLst::const_reverse_iterator itr = coll->rbegin(); itr != coll->rend(); ++itr) {
        (*itr)->ListInterfaces(apLst,anFlags);
      }
    }
  }
  if (!niFlagIs(anFlags,eListInterfacesFlags_DynamicOnly)) {
    apLst->Add(niGetInterfaceUUID(iWidget));
    apLst->Add(niGetInterfaceUUID(iMessageHandler));
    apLst->Add(niGetInterfaceUUID(iUnknown));
  }
}

///////////////////////////////////////////////
ni::iGraphics* __stdcall cWidget::GetGraphics() const {
  CheckValid(NULL);
  return mpUICtx->mptrGraphics;
}

///////////////////////////////////////////////
ni::iGraphicsContext* __stdcall cWidget::GetGraphicsContext() const {
  CheckValid(NULL);
  return mpUICtx->mptrGraphicsContext;
}

///////////////////////////////////////////////
ni::iUIContext * cWidget::GetUIContext() const {
  CheckValid(NULL);
  return mpUICtx;
}

///////////////////////////////////////////////
iHString* __stdcall cWidget::GetClassName() const
{
  return mhspClass;
}

///////////////////////////////////////////////
void cWidget::SetID(iHString* ahspID)
{
  if (!ahspID) {
    mhspID = _H(AZEROSTR);
  }
  else {
    mhspID = ahspID;
  }
}

///////////////////////////////////////////////
iHString* cWidget::GetID() const
{
  return mhspID;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::SetStyle(tU32 anStyle)
{
  if (anStyle == mnStyle) return eTrue;
  const tU32 nOldStyle = mnStyle;
  // free style changed, so update the owner zmap
  const tBool bZMapUpdate = niFlagIs(anStyle,eWidgetStyle_Free) != niFlagIs(nOldStyle,eWidgetStyle_Free);
  if (bZMapUpdate) {
    // remove of the current zmap, before the style flag is changed as
    // _GetOwnerZMap checks the current style
    iWidgetZMap* pOwnerZMap = _GetOwnerZMap();
    if (pOwnerZMap) {
      pOwnerZMap->RemoveOfZMap(this);
    }
  }
  // change the style
  mnStyle = anStyle;
  // set the new zmap if necessary
  if (bZMapUpdate) {
    QPtr<cWidget> ptrParent(mpwParent);
    if (ptrParent.IsOK()) {
      Ptr<iWidget> ptrChild = this;
      tBool bRes = astl::find_erase(ptrParent->mvecClipChildren,this);
      if (!bRes) {
        bRes = astl::find_erase(ptrParent->mvecFreeChildren,this);
      }
      if (bRes) {
        if (niFlagIs(mnStyle,eWidgetStyle_Free)) {
          ptrParent->mvecFreeChildren.push_back(this);
        }
        else {
          ptrParent->mvecClipChildren.push_back(this);
        }
      }
      UpdateParentAutoLayout("Parent-ChangedStyle-ZMapUpdate");
      mpUICtx->UpdateFreeWidgets();
    }
  }

#ifdef USE_CACHED_ABS_RECT
  if (niFlagIs(mnStyle,eWidgetStyle_NoClip) != niFlagIs(nOldStyle,eWidgetStyle_NoClip)) {
    // make sure the clip rects are updated if NoClip style changed...
    mStatus |= WDGSTATUS_DIRTY_RECT_ANY;
  }
#endif

  this->SendMessage(eUIMessage_StyleChanged,nOldStyle,niVarNull);
  return eTrue;
}

///////////////////////////////////////////////
tU32 cWidget::GetStyle() const
{
  return mnStyle;
}

///////////////////////////////////////////////
tBool cWidget::_CheckSelfParent(ni::iWidget *apParent) const {
  if (apParent) {
    if (!niIsOK(apParent)) {
      niError(_A("Trying to set an invalid parent directly."));
      return eFalse;
    }
    if (apParent == this) {
      niError(niFmt(_A("[%s] Trying to set itself as direct parent."),
                    niHStr(GetID())));
      return eFalse;
    }

    iWidget* p = apParent;
    while (p) {
      if (p == this) {
        niError(niFmt(_A("[%s] Trying to set itself as indirect parent."),
                      niHStr(GetID())));
        return eFalse;
      }
      if (!niIsOK(apParent)) {
        niError(_A("Trying to set an invalid parent indirectly."));
        return eFalse;
      }
      p = p->GetParent();
    }
  }
  return eTrue;
}


///////////////////////////////////////////////
// Set the parent without any prior check
tBool cWidget::DoSetParent(ni::iWidget *apParent)
{
  niGuardObject(this);
  if (apParent && apParent->GetUIContext() != this->GetUIContext()) {
    niError("Can't set as parent a widget from another UI context.");
    return eFalse;
  }

  if (!_CheckSelfParent(apParent)) {
    apParent = NULL;
  }

  QPtr<cWidget> ptrOldParent(mpwParent);
  // Has to be done here, as in RemoveChild the ZMap will already be linked to the new parent
  iWidgetZMap* pOwnerZMap = _GetOwnerZMap();
  if (pOwnerZMap) {
    pOwnerZMap->RemoveOfZMap(this);
  }
  mpwParent = static_cast<cWidget*>(apParent);
  if (ptrOldParent.IsOK()) {
    ptrOldParent->RemoveChild(this);
  }
  if (apParent) {
    cWidget* parent = static_cast<cWidget*>(apParent);
    parent->AddChild(this);
  }

#ifdef USE_CACHED_ABS_RECT
  _SetMoved(); // make sure we notify that the widget has moved
#endif

  if (HStringIsEmpty(mhspLocale) && apParent && !HStringIsEmpty(apParent->GetLocale())) {
    BroadcastMessage(eUIMessage_LocaleChanged,niVarNull,niVarNull);
  }
  return eTrue;
}

///////////////////////////////////////////////
void cWidget::SetParent(ni::iWidget *apParent)
{
  CheckValid(;);
  WeakPtr<cWidget> wpNewParent((cWidget*)apParent);
  if (wpNewParent == mpwParent)
    return;
  DoSetParent(apParent);
}

///////////////////////////////////////////////
ni::iWidget * cWidget::GetParent() const
{
  QPtr<cWidget> ptrParent(mpwParent);
  return ptrParent.ptr();
}

///////////////////////////////////////////////
void __stdcall cWidget::SetZOrder(eWidgetZOrder aZOrder)
{
  const eWidgetZOrder wasZOrder = aZOrder;
  mZOrder = aZOrder;
  iWidgetZMap* pOwnerZMap = _GetOwnerZMap();
  if (pOwnerZMap) {
    pOwnerZMap->SetZOrder(this,aZOrder);
    if (wasZOrder != mZOrder || !niFlagIs(mStatus,WDGSTATUS_UPDATINGAUTOLAYOUT)) {
      UpdateParentAutoLayout("Parent-ZOrder");
    }
  }
}

///////////////////////////////////////////////
eWidgetZOrder __stdcall cWidget::GetZOrder() const
{
  return mZOrder;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetZOrderAbove(iWidget* apWidget)
{
  iWidgetZMap* pOwnerZMap = _GetOwnerZMap();
  if (pOwnerZMap) {
    pOwnerZMap->SetZOrderAbove(this,(cWidget*)apWidget);
    UpdateParentAutoLayout("Parent-ZOrder-Above");
  }
}

///////////////////////////////////////////////
tU32 __stdcall cWidget::GetDrawOrder() const
{
  QPtr<cWidget> ptrParent(mpwParent);
  if (!ptrParent.IsOK())
    return 0;
  return ptrParent->GetChildDrawOrder(this);
}

///////////////////////////////////////////////
void cWidget::SetPosition(const sVec2f &avPos)
{
  sVec2f pos = avPos; _RoundPos(pos);
#ifdef USE_CACHED_ABS_RECT
  if (pos == mRect.GetTopLeft()) return;
  mRect.MoveTo(pos);
  _SetMoved();
#else
  mRect.MoveTo(pos);
#endif
}

///////////////////////////////////////////////
sVec2f cWidget::GetPosition() const
{
  return *(sVec2f*)&mRect.x;
}

///////////////////////////////////////////////
void cWidget::SetSize(const sVec2f &avSize)
{
  CheckValid(;);
  sVec2f size = avSize; _RoundPos(size);
  const sVec2f vPrevSize = mRect.GetSize();
  sRectf newRect = mRect;
  newRect.SetSize(size.x,size.y);
  _ComputeRectSize(newRect,eTrue,vPrevSize);
}

///////////////////////////////////////////////
sVec2f cWidget::GetSize() const
{
  return mRect.GetSize();
}

///////////////////////////////////////////////
void __stdcall cWidget::SetMinimumSize(sVec2f avMinSize)
{
  _UpdateMinMaxSize(avMinSize,mvMaxSize);
}

///////////////////////////////////////////////
sVec2f __stdcall cWidget::GetMinimumSize() const
{
  return mvMinSize;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetMaximumSize(sVec2f avMaxSize)
{
  _UpdateMinMaxSize(mvMinSize,avMaxSize);
}

///////////////////////////////////////////////
sVec2f __stdcall cWidget::GetMaximumSize() const
{
  return mvMaxSize;
}

///////////////////////////////////////////////
void cWidget::SetRect(const sRectf &aRect)
{
  CheckValid(;);
  const sVec2f vPrevSize = mRect.GetSize();
  if (aRect == mRect) return;
  _ComputeRectSize(aRect,eTrue,vPrevSize);
}

///////////////////////////////////////////////
sRectf cWidget::GetRect() const
{
  return mRect;
}

///////////////////////////////////////////////
sRectf __stdcall cWidget::GetWidgetRect() const
{
  return sRectf(0,0,mRect.GetWidth(),mRect.GetHeight());
}

///////////////////////////////////////////////
sRectf __stdcall cWidget::GetDockFillRect() const
{
  return mrectDockFillClient;
}

///////////////////////////////////////////////
sRectf __stdcall cWidget::GetDockFillRectNC() const
{
  return mrectDockFillNonClient;
}

///////////////////////////////////////////////
void cWidget::SetClientPosition(const sVec2f &avPos) {
  sRectf newRect = mClientRect;
  sVec2f pos = avPos;
  _RoundPos(pos);
  newRect.MoveTo(pos);
  this->SetClientRect(newRect);
}
sVec2f cWidget::GetClientPosition() const {
  return *(sVec2f*)&mClientRect.x;
}
void cWidget::SetClientSize(const sVec2f &avSize) {
  sRectf newRect = mClientRect;
  sVec2f size = avSize;
  _RoundPos(size);
  newRect.SetSize(size.x,size.y);
  this->SetClientRect(newRect);
}
sVec2f cWidget::GetClientSize() const {
  return mClientRect.GetSize();
}

///////////////////////////////////////////////
void cWidget::SetClientRect(const sRectf &aNewRect) {
  sRectf newRect = aNewRect;
  if (mClientRect == newRect)
    return;
  const sVec2f vPrevSize = mClientRect.GetSize();
  mClientRect = newRect;
#ifdef USE_CACHED_ABS_RECT
  _SetMoved();
#endif
  _RoundRect(mClientRect);
  this->SendMessage(eUIMessage_Size,mClientRect.GetSize(),vPrevSize);
}
sRectf cWidget::GetClientRect() const {
  return mClientRect;
}

///////////////////////////////////////////////
sRectf __stdcall cWidget::ComputeFitRect(const sRectf& aRect) const
{
  sRectf newRect = aRect;
  sRectf margins;
  margins.Left() = mClientRect.Left();
  margins.Right() = mRect.GetSize().x-mClientRect.GetSize().x-mClientRect.Left();
  margins.Top() = mClientRect.Top();
  margins.Bottom() = mRect.GetSize().y-mClientRect.GetSize().y-mClientRect.Top();
  newRect.Left() -= margins.Left();
  newRect.Right() += margins.Right();
  newRect.Top() -= margins.Top();
  newRect.Bottom() += margins.Bottom();
  _RoundRect(newRect);
  return newRect;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetFitRect(const sRectf& aRect)
{
  SetAbsoluteRect(ComputeFitRect(aRect));
}

///////////////////////////////////////////////
void __stdcall cWidget::SetFitSize(const sVec2f avSize)
{
  sRectf newRect = ComputeFitRect(sRectf(0,0,avSize.x,avSize.y));
  newRect.MoveTo(GetAbsolutePosition());
  SetAbsoluteRect(newRect);
}

///////////////////////////////////////////////
void cWidget::SetAbsolutePosition(const sVec2f &avPos)
{
  QPtr<cWidget> ptrParent(mpwParent);
  if (ptrParent.IsOK() && niFlagIsNot(GetStyle(),eWidgetStyle_Free)) {
    sVec2f pos = avPos-ptrParent->GetAbsolutePosition();
    if (niFlagIsNot(GetStyle(),eWidgetStyle_NCRelative)) {
      pos -= ptrParent->GetClientPosition();
    }
    SetPosition(pos);
  }
  else {
    SetPosition(avPos);
  }
}

///////////////////////////////////////////////
sVec2f cWidget::GetAbsolutePosition() const
{
  CheckValid(sVec2f::Zero());
  niProf(Widget_GetAbsPos);

#ifdef USE_CACHED_ABS_RECT
  if (niFlagIs(mStatus,WDGSTATUS_DIRTY_RECTABS)) {
    mrectAbs = mRect;
#endif
    QPtr<cWidget> ptrParent(mpwParent);
    if (ptrParent.IsOK() && niFlagIsNot(GetStyle(),eWidgetStyle_Free)) {
      sVec2f pos = GetPosition()+ptrParent->GetAbsolutePosition();
      if (niFlagIsNot(GetStyle(),eWidgetStyle_NCRelative)) {
        pos += ptrParent->GetClientPosition();
      }
#ifdef USE_CACHED_ABS_RECT
      mrectAbs.MoveTo(pos);
#else
      return pos;
#endif
    }

#ifdef USE_CACHED_ABS_RECT
    niFlagOff(mStatus,WDGSTATUS_DIRTY_RECTABS);
  }
  return mrectAbs.GetTopLeft();
#else
  return GetPosition();
#endif
}

///////////////////////////////////////////////
void __stdcall cWidget::SetAbsoluteRect(const sRectf& aRect)
{
  SetAbsolutePosition(aRect.GetTopLeft());
  SetSize(aRect.GetSize());
}

///////////////////////////////////////////////
sRectf __stdcall cWidget::GetAbsoluteRect() const
{
  return GetWidgetRect()+GetAbsolutePosition();
}

///////////////////////////////////////////////
sRectf cWidget::GetClippedRect() const
{
  sRectf rect = GetAbsoluteClippedRect();
  rect = sRectf(rect.GetTopLeft() - GetAbsolutePosition(),rect.GetWidth(),rect.GetHeight());
  rect = GetAbsoluteClippedRect()-GetAbsolutePosition();
  return rect;
}

///////////////////////////////////////////////
sRectf cWidget::GetAbsoluteClippedRect() const
{
  CheckValid(sRectf::Null());
#ifdef USE_CACHED_ABS_RECT
  if (niFlagIs(mStatus,WDGSTATUS_DIRTY_RECTABSCLIPPED)) {
#endif
    sRectf parrect;
    const tWidgetStyleFlags style = GetStyle();
    if (niFlagIs(style,eWidgetStyle_Free) || niFlagIs(style,eWidgetStyle_NoClip)) {
      parrect = mpUICtx->GetRootWidget()->GetAbsoluteRect();
    }
    else {
      QPtr<cWidget> ptrParent(mpwParent);
      if (niFlagIs(style,eWidgetStyle_NCRelative)) {
        parrect = ptrParent.IsOK()?ptrParent->GetAbsoluteClippedRect():mRect;
      }
      else {
        parrect = ptrParent.IsOK()?ptrParent->GetAbsoluteClippedClientRect():mRect;
      }
    }
#ifdef USE_CACHED_ABS_RECT
    mrectAbsClipped = parrect.ClipRect(GetAbsoluteRect());
    niFlagOff(mStatus,WDGSTATUS_DIRTY_RECTABSCLIPPED);
  }
  return mrectAbsClipped;
#else
#endif
}

///////////////////////////////////////////////
sRectf __stdcall cWidget::GetClippedClientRect() const
{
  return GetAbsoluteClippedClientRect()-GetAbsolutePosition();
}

///////////////////////////////////////////////
sRectf __stdcall cWidget::GetAbsoluteClippedClientRect() const
{
  CheckValid(sRectf::Null());
#ifdef USE_CACHED_ABS_RECT
  if (niFlagIs(mStatus,WDGSTATUS_DIRTY_RECTABSCLIENTCLIPPED)) {
#endif
    sRectf parrect;
    const tWidgetStyleFlags style = GetStyle();
    if (niFlagIs(style,eWidgetStyle_Free) || niFlagIs(style,eWidgetStyle_NoClip)) {
      parrect = mpUICtx->GetRootWidget()->GetAbsoluteRect();
    }
    else {
      QPtr<cWidget> ptrParent(mpwParent);
      if (niFlagIs(style,eWidgetStyle_NCRelative)) {
        parrect = ptrParent.IsOK()?ptrParent->GetAbsoluteClippedRect():mRect;
      }
      else {
        parrect = ptrParent.IsOK()?ptrParent->GetAbsoluteClippedClientRect():mRect;
        sRectf absRect = GetAbsoluteRect();
        if (parrect.Left() < absRect.Left()) parrect.Left() = absRect.Left();
        if (parrect.Right() > absRect.Right()) parrect.Right() = absRect.Right();
        if (parrect.Top() < absRect.Top()) parrect.Top() = absRect.Top();
        if (parrect.Bottom() > absRect.Bottom()) parrect.Bottom() = absRect.Bottom();
      }
    }

#ifdef USE_CACHED_ABS_RECT
    mrectAbsClientClipped = parrect.ClipRect(GetClientRect()+GetAbsolutePosition());
    niFlagOff(mStatus,WDGSTATUS_DIRTY_RECTABSCLIENTCLIPPED);
  }
  return mrectAbsClientClipped;
#else
  return parrect.ClipRect(GetClientRect()+GetAbsolutePosition());
#endif
}

///////////////////////////////////////////////
void __stdcall cWidget::SetRelativePosition(const sVec2f& avPos)
{
  CheckValid(;);
  if (mrectRelative.GetTopLeft() == avPos) return;
  mrectRelative.SetCorner(eRectCorners_TopLeft,avPos);
  NotifyChangedRelative();
  ApplyRelativeRect();
}

///////////////////////////////////////////////
sVec2f __stdcall cWidget::GetRelativePosition() const
{
  return *(sVec2f*)&mrectRelative.x;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetRelativeSize(const sVec2f& avSize)
{
  CheckValid(;);
  if (mrectRelative.GetSize() == avSize) return;
  mrectRelative.SetSize(avSize.x,avSize.y);
  NotifyChangedRelative();
  ApplyRelativeRect();
}

///////////////////////////////////////////////
sVec2f __stdcall cWidget::GetRelativeSize() const
{
  return mrectRelative.GetSize();
}

///////////////////////////////////////////////
void __stdcall cWidget::SetRelativeRect(const sRectf& aRect)
{
  CheckValid(;);
  if (mrectRelative == aRect) return;
  mrectRelative = aRect;
  NotifyChangedRelative();
  ApplyRelativeRect();
}

///////////////////////////////////////////////
sRectf __stdcall cWidget::GetRelativeRect() const
{
  return mrectRelative;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetPadding(const sVec4f& aRect)
{
  if (aRect == mvPadding) return;
  mvPadding = aRect;
  this->SendMessage(eUIMessage_Padding,niVarNull,niVarNull);
}
sVec4f __stdcall cWidget::GetPadding() const
{
  return mvPadding;
}
tBool __stdcall cWidget::GetHasPadding() const
{
  return mvPadding.Left() != 0 ||
      mvPadding.Right() != 0 ||
      mvPadding.Top() != 0 ||
      mvPadding.Bottom() != 0;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetMargin(const sVec4f& aRect)
{
  if (aRect == mvMargin) return;
  mvMargin = aRect;
  this->SendMessage(eUIMessage_Margin,niVarNull,niVarNull);
}
sVec4f __stdcall cWidget::GetMargin() const
{
  return mvMargin;
}
tBool __stdcall cWidget::GetHasMargin() const
{
  return mvMargin.Left() != 0 ||
      mvMargin.Right() != 0 ||
      mvMargin.Top() != 0 ||
      mvMargin.Bottom() != 0;
}

///////////////////////////////////////////////
tBool cWidget::SetFocus()
{
  CheckValid(eFalse);
  _UIFocusTrace(niFmt(_A("### UICONTEXT this->SetFocus(): %p (ID:%s)."),(tIntPtr)this,this->GetID()));
  mpUICtx->SetFocusInput(this,eFalse);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::MoveFocus(tBool abToPrevious) {
  CheckValid(eFalse);
  return mpUICtx->MoveFocus(this,abToPrevious);
}

///////////////////////////////////////////////
tBool cWidget::GetHasFocus() const
{
  CheckValid(eFalse);
  return mpUICtx->_IsFocusInput(this);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetDraggingSource() const
{
  CheckValid(eFalse);
  return GetDraggingFingerSource(mpUICtx->GetPrimaryFingerID());
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetDragging() const
{
  CheckValid(eFalse);
  return GetDraggingFinger(mpUICtx->GetPrimaryFingerID());
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetDraggingFingerSource(tU32 anFinger) const
{
  CheckValid(eFalse);
  return mpUICtx->IsFingerDrag(anFinger,this);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetDraggingFinger(tU32 anFinger) const
{
  CheckValid(eFalse);
  return mpUICtx->IsFingerDragging(anFinger);
}

///////////////////////////////////////////////
void cWidget::SetCapture(tBool abEnable) {
  CheckValid(;);
  if (GetCapture() == abEnable)
    return;
  if (abEnable) {
    if (!GetVisible() || !GetEnabled())
      return; // can capture only if enabled and visible
    mpUICtx->_AddCaptureAll(this);
  }
  else {
    mpUICtx->_RemoveCaptureAll(this,eFalse);
  }
  niFlagOnIf(mStatus,WDGSTATUS_CAPTURE_ALL,abEnable);
}
tBool cWidget::GetCapture() const {
  CheckValid(eFalse);
  niAssert(mpUICtx->_IsCaptureAll(this) ==
           niFlagIs(mStatus,WDGSTATUS_CAPTURE_ALL));
  return niFlagIs(mStatus,WDGSTATUS_CAPTURE_ALL);
}

///////////////////////////////////////////////
void cWidget::SetFingerCapture(tU32 anFinger, tBool abEnable) {
  CHECK_FINGER(anFinger,;);
  CheckValid(;);
  if (GetFingerCapture(anFinger) == abEnable)
    return;
  if (abEnable) {
    if (!GetVisible() || !GetEnabled())
      return; // can capture only if enabled and visible
    mpUICtx->_AddFingerCapture(anFinger,this);
  }
  else {
    mpUICtx->_RemoveFingerCapture(anFinger,this,eFalse);
  }
  niFlagOnIf(mStatus,WDGSTATUS_FINGER_CAPTURE(anFinger),abEnable);
}
tBool cWidget::GetFingerCapture(tU32 anFinger) const {
  CHECK_FINGER(anFinger,eFalse);
  CheckValid(eFalse);
  niAssert(mpUICtx->_IsFingerCapture(anFinger,this) == niFlagIs(mStatus,WDGSTATUS_FINGER_CAPTURE(anFinger)));
  return niFlagIs(mStatus,WDGSTATUS_FINGER_CAPTURE(anFinger));
}

///////////////////////////////////////////////
void cWidget::SetExclusive(tBool abEnable) {
  CheckValid(;);
  if (GetExclusive() == abEnable)
    return;

  if (abEnable) {
    if (!GetVisible() || !GetEnabled())
      return; // can exclusive only if enabled and visible
    mpUICtx->_AddExclusive(this);
  }
  else {
    mpUICtx->_RemoveExclusive(this,eFalse);
  }
  niFlagOnIf(mStatus,WDGSTATUS_EXCLUSIVE,abEnable);
}
tBool cWidget::GetExclusive() const {
  CheckValid(eFalse);
  niAssert(mpUICtx->_IsExclusive(this) == niFlagIs(mStatus,WDGSTATUS_EXCLUSIVE));
  return niFlagIs(mStatus,WDGSTATUS_EXCLUSIVE);
}

///////////////////////////////////////////////
void cWidget::_CheckCaptureAndExclusive() {
  CheckValid(;);
  if (!niFlagIs(mStatus,WDGSTATUS_VISIBLE) || !niFlagIs(mStatus,WDGSTATUS_ENABLED)) {
    niLoop(i,knNumFingers) {
      if (this->GetFingerCapture(i)) {
        this->SetFingerCapture(i,eFalse);
      }
    }
    if (this->GetCapture()) {
      this->SetCapture(eFalse);
    }
    if (this->GetExclusive()) {
      this->SetExclusive(eFalse);
    }

    QPtr<cWidget> inputTarget(mpUICtx->GetInputMessageTarget());
    if (inputTarget == this) {
      QPtr<cWidget> ptrParent(mpwParent);
      _UIFocusTrace(niFmt(_A("### UICONTEXT _CheckCaptureAndExclusive: %p (ID:%s)."),(tIntPtr)ptrParent.ptr(),ptrParent->GetID()));
      mpUICtx->SetFocusInput(ptrParent,eFalse);
    }
  }
}

///////////////////////////////////////////////
void cWidget::SetVisible(tBool abVisible) {
  CheckValid(;);
  if (GetVisible() == abVisible) return;
  niFlagOnIf(mStatus,WDGSTATUS_VISIBLE,abVisible);
  _CheckCaptureAndExclusive();
  this->SendMessage(eUIMessage_Visible,tU32(abVisible),niVarNull);
  UpdateParentAutoLayout("Parent-SetVisible");
}
tBool cWidget::GetVisible() const {
  return niFlagIs(mStatus,WDGSTATUS_VISIBLE);
}

///////////////////////////////////////////////
void cWidget::SetEnabled(tBool abEnabled) {
  CheckValid(;);
  if (GetEnabled() == abEnabled) return;
  niFlagOnIf(mStatus,WDGSTATUS_ENABLED,abEnabled);
  _CheckCaptureAndExclusive();
  this->SendMessage(eUIMessage_Enabled,tU32(abEnabled),niVarNull);
}
tBool cWidget::GetEnabled() const {
  return niFlagIs(mStatus,WDGSTATUS_ENABLED);
}

///////////////////////////////////////////////
void __stdcall cWidget::SetInputSubmitFlags(tU32 aSubmitFlags) {
  mnInputSubmitFlags = aSubmitFlags;
}
tU32 __stdcall cWidget::GetInputSubmitFlags() const {
  return mnInputSubmitFlags;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetIgnoreInput(tBool abIgnoreInput)
{
  niFlagOnIf(mStatus,WDGSTATUS_IGNOREINPUT,abIgnoreInput);
}
tBool __stdcall cWidget::GetIgnoreInput() const
{
  return niFlagIs(mStatus,WDGSTATUS_IGNOREINPUT);
}

///////////////////////////////////////////////
void __stdcall cWidget::SetHideChildren(tBool abHideChildren)
{
  niFlagOnIf(mStatus,WDGSTATUS_HIDECHILDREN,abHideChildren);
}
tBool __stdcall cWidget::GetHideChildren() const
{
  return niFlagIs(mStatus,WDGSTATUS_HIDECHILDREN);
}

///////////////////////////////////////////////
void __stdcall cWidget::SetStatus(tBool abVisible, tBool abEnabled, tBool abIgnoreInput)
{
  SetVisible(abVisible);
  SetEnabled(abEnabled);
  SetIgnoreInput(abIgnoreInput);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetIsMouseOver() const {
  CheckValid(eFalse);
  return GetIsFingerOver(mpUICtx->GetPrimaryFingerID());
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetIsNcMouseOver() const {
  CheckValid(eFalse);
  return GetIsNcFingerOver(mpUICtx->GetPrimaryFingerID());
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetIsPressed() const {
  return GetIsMouseOver() && niFlagIs(mClick,WDGCLICK_LEFTDOWN);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetIsNcPressed() const {
  return GetIsNcMouseOver() && niFlagIs(mClick,WDGCLICK_LEFTDOWN);
}


///////////////////////////////////////////////
static tBool __stdcall _IsFingerOver(const cWidget* apWidget, const sRectf& aRect, tU32 anFinger) {
  CHECK_FINGER(anFinger,eFalse);
  cUIContext* pCtx = apWidget->mpUICtx;

  const cUIContext::sFinger& f = pCtx->GET_FINGER(anFinger);

  {
    QPtr<cWidget> captureAll(pCtx->mpwCaptureAll);
    if (captureAll.IsOK() && captureAll.ptr() != apWidget)
      return eFalse;
    QPtr<cWidget> captureFinger(f.mpwCapture);
    if (captureFinger.IsOK() && captureFinger.ptr() != apWidget)
      return eFalse;
  }

  if (aRect.Intersect(f.GetPosition()))
  {
    QPtr<cWidget> topWidget(pCtx->mpwTopWidget);
    if (topWidget.IsOK() && !topWidget->HasChild(apWidget,eTrue)) {
      return eFalse;
    }

    QPtr<cWidget> hoverWidget(f.mpwHover);
    if (hoverWidget.IsOK() &&
        hoverWidget.ptr() != apWidget &&
        !hoverWidget->HasChild(apWidget,eTrue) &&
        !hoverWidget->HasParent(apWidget))
    {
      return eFalse;
    }

    return eTrue;
  }

  return eFalse;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetIsFingerOver(tU32 anFinger) const {
  CheckValid(eFalse);
  CHECK_FINGER(anFinger,eFalse);
  sRectf r = GetClientRect();
  r += GetAbsolutePosition();
  return _IsFingerOver(this,r,anFinger);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetIsNcFingerOver(tU32 anFinger) const {
  CheckValid(eFalse);
  CHECK_FINGER(anFinger,eFalse);
  const sRectf r = GetAbsoluteRect();
  return _IsFingerOver(this,r,anFinger);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetIsFingerPressed(tU32 anFinger) const {
  CheckValid(eFalse);
  return GetIsFingerOver(anFinger) && mpUICtx->GetFingerDown(anFinger);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::GetIsNcFingerPressed(tU32 anFinger) const {
  CheckValid(eFalse);
  return GetIsNcFingerOver(anFinger) && mpUICtx->GetFingerDown(anFinger);
}

///////////////////////////////////////////////
void cWidget::SetTimer(tU32 anID, tF32 afTime) {
  CheckValid(;);
  mpUICtx->SetTimer(this,anID,afTime);
}

///////////////////////////////////////////////
tF32 cWidget::GetTimer(tU32 anID) const {
  return mpUICtx->GetTimer(this,anID);
}

///////////////////////////////////////////////
tBool cWidget::AddSink(ni::iWidgetSink *apSink)
{
  CheckValid(eFalse);
  if (!niIsOK(apSink)) return eFalse;
  Ptr<ni::iWidgetSink> ptrSink = apSink;
  niAssert(!mlstSinks->Contains(apSink));
  niCheck(!mlstSinks->Contains(apSink),eFalse);
  niGuardObject(this);
  niCheck(mlstSinks->AddSink(apSink),eFalse);
  apSink->OnWidgetSink(this,eUIMessage_SinkAttached,niVarNull,niVarNull);
  CheckValid(eFalse); // in case the previous call destroy the widget...
  apSink->OnWidgetSink(this,eUIMessage_SkinChanged,niVarNull,niVarNull);
  CheckValid(eFalse); // in case the previous call destroy the widget...
  this->SendMessage(eUIMessage_NCSize,mRect.GetSize(),mRect.GetSize());
  CheckValid(eFalse); // in case the previous call destroy the widget...
  this->SendMessage(eUIMessage_Size,mClientRect.GetSize(),mClientRect.GetSize());
  CheckValid(eFalse); // in case the previous call destroy the widget...
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::AddPostSink(iWidgetSink* apSink)
{
  CheckValid(eFalse);
  if (!niIsOK(apSink)) return eFalse;
  Ptr<ni::iWidgetSink> ptrSink = apSink;
  niAssert(!mlstSinks->Contains(apSink));
  niCheck(!mlstSinks->Contains(apSink),eFalse);
  niGuardObject(this);
  niCheck(mlstSinks->AddFrontSink(apSink),eFalse);
  apSink->OnWidgetSink(this,eUIMessage_SinkAttached,niVarNull,niVarNull);
  CheckValid(eFalse); // in case the previous call destroy the widget...
  apSink->OnWidgetSink(this,eUIMessage_SkinChanged,niVarNull,niVarNull);
  CheckValid(eFalse); // in case the previous call destroy the widget...
  this->SendMessage(eUIMessage_NCSize,mRect.GetSize(),mRect.GetSize());
  CheckValid(eFalse); // in case the previous call destroy the widget...
  this->SendMessage(eUIMessage_Size,mClientRect.GetSize(),mClientRect.GetSize());
  CheckValid(eFalse); // in case the previous call destroy the widget...
  return eTrue;
}

///////////////////////////////////////////////
iWidgetSink* __stdcall cWidget::AddClassSink(const achar* aaszClassName)
{
  Ptr<iWidgetSink> ptrSink = mpUICtx->CreateWidgetSink(aaszClassName,this);
  if (!ptrSink.IsOK()) {
    niError(niFmt(_A("Can't create class '%s' sink."),aaszClassName));
    return NULL;
  }
  if (!AddSink(ptrSink)) {
    niError(niFmt(_A("Can't add class '%s' sink."),aaszClassName));
    return NULL;
  }
  return ptrSink.GetRawAndSetNull();
}

///////////////////////////////////////////////
iWidgetSink* __stdcall cWidget::AddClassPostSink(const achar* aaszClassName)
{
  Ptr<iWidgetSink> ptrSink = mpUICtx->CreateWidgetSink(aaszClassName,this);
  if (!ptrSink.IsOK()) {
    niError(niFmt(_A("Can't create class '%s' sink."),aaszClassName));
    return NULL;
  }
  if (!AddPostSink(ptrSink)) {
    niError(niFmt(_A("Can't post add class '%s' sink."),aaszClassName));
    return NULL;
  }
  return ptrSink.GetRawAndSetNull();
}

///////////////////////////////////////////////
void cWidget::RemoveSink(ni::iWidgetSink *apSink)
{
  CheckValid(;);
  Ptr<ni::iWidgetSink> ptrSink = apSink;
  Ptr<tWidgetSinkLst::sIterator> it = (tWidgetSinkLst::sIterator*)mlstSinks->Find(apSink);
  if (it.IsOK()) {
    it->_Value()->OnWidgetSink(this,eUIMessage_SinkDetached,niVarNull,niVarNull);
    mlstSinks->RemoveSink(apSink);
  }
}

///////////////////////////////////////////////
void __stdcall cWidget::_SendDestroyMessage()
{
  if (GetNumChildren()) {
    tWidgetPtrLst lstW;
    for (tCWidgetVec::iterator itr=mvecClipChildren.begin(); itr != mvecClipChildren.end(); ++itr) {
      lstW.push_back(*itr);
    }
    for (tCWidgetVec::iterator itr=mvecFreeChildren.begin(); itr != mvecFreeChildren.end(); ++itr) {
      lstW.push_back(*itr);
    }
    for (tWidgetPtrLstIt itL = lstW.begin(); itL != lstW.end(); ++itL) {
      cWidget* w = (*itL);
      w->_SendDestroyMessage();
    }
  }
  if (!niFlagIs(this->mStatus,WDGSTATUS_DESTROYED)) {
    this->SendMessage(eUIMessage_Destroy,niVarNull,niVarNull);
    niFlagOn(this->mStatus,WDGSTATUS_DESTROYED);
  }
}

///////////////////////////////////////////////
void __stdcall cWidget::InvalidateChildren()
{
  if (GetNumChildren()) {
    tWidgetPtrLst lstW;
    for (tCWidgetVec::iterator itr=mvecClipChildren.begin(); itr != mvecClipChildren.end(); ++itr) {
      lstW.push_back(*itr);
    }
    for (tCWidgetVec::iterator itr=mvecFreeChildren.begin(); itr != mvecFreeChildren.end(); ++itr) {
      lstW.push_back(*itr);
    }
    for (tWidgetPtrLstIt itL = lstW.begin(); itL != lstW.end(); ++itL) {
      (*itL)->Invalidate();
    }
  }
  mZMap.Clear();
  // niAssert(mvecClipChildren.empty());
  // niAssert(mvecFreeChildren.empty());
}

///////////////////////////////////////////////
tU32 cWidget::GetNumChildren() const
{
  return (tU32)mvecClipChildren.size()+(tU32)mvecFreeChildren.size();
}

///////////////////////////////////////////////
tU32 __stdcall cWidget::GetChildIndex(iWidget* apWidget) const
{
  niLoop(i,GetNumChildren()) {
    if (GetChildFromIndex(i) == apWidget)
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
ni::iWidget * cWidget::GetChildFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumChildren())
    return NULL;
  if (anIndex < mvecClipChildren.size())
    return mvecClipChildren[anIndex];
  return mvecFreeChildren[anIndex-mvecClipChildren.size()];
}

///////////////////////////////////////////////
ni::iWidget * cWidget::GetChildFromID(iHString* ahspID) const
{
  for(tCWidgetVec::const_iterator mitr = mvecClipChildren.begin();
      mitr !=  mvecClipChildren.end(); ++mitr)
  {
    if ((*mitr)->GetID() == ahspID)
      return (*mitr);
  }

  for(tCWidgetVec::const_iterator vitr = mvecFreeChildren.begin();
      vitr != mvecFreeChildren.end(); ++vitr)
  {
    if ((*vitr)->GetID() == ahspID)
      return (*vitr);
  }

  return NULL;
}

///////////////////////////////////////////////
tU32 __stdcall cWidget::GetNumChildrenDrawn() const {
  CheckValid(0);
  return mZMap.GetNumDrawable();
}
ni::iWidget* __stdcall cWidget::GetChildFromDrawOrder(tU32 anDrawOrder) const
{
  CheckValid(NULL);
  return mZMap.GetFromDrawOrder(anDrawOrder);
}

///////////////////////////////////////////////
static iWidget* _FindWidget(iWidget* apWidget, iHString* ahspID) {
  if (apWidget->GetID() == ahspID)
    return apWidget;
  for (tU32 i = 0; i < apWidget->GetNumChildren(); ++i) {
    iWidget* pW = _FindWidget(apWidget->GetChildFromIndex(i),ahspID);
    if (pW != NULL) return pW;
  }
  return NULL;
}

///////////////////////////////////////////////
iWidget* __stdcall cWidget::FindWidget(iHString* ahspID) const
{
  if (HStringIsEmpty(ahspID)) return NULL;
  return _FindWidget(const_cast<cWidget*>(this),ahspID);
}

///////////////////////////////////////////////
iWidget* __stdcall cWidget::FindWidgetByPos(const sVec2f& avPos) const
{
  return GetMessageTargetByPos(avPos,Callback_ExcludeWidget_IgnoreInput,0);
}

///////////////////////////////////////////////
void __stdcall cWidget::SetSkin(iHString* ahspSkin) {
  mhspSkin = ahspSkin;
  SendMessage(eUIMessage_SkinChanged,niVarNull,niVarNull);
}

///////////////////////////////////////////////
iHString* __stdcall cWidget::GetSkin() const {
  tHStringPtr hspSkin = mhspSkin;
  if (mpwParent.IsOK() && HStringIsEmpty(hspSkin)) {
    QPtr<cWidget> ptrParent(mpwParent);
    if (ptrParent.IsOK()) {
      hspSkin = ptrParent->GetSkin();
    }
  }
  return hspSkin;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetSkinClass(iHString* ahspSkinClass)
{
  if (ahspSkinClass == mhspSkinClass)
    return;
  if (ni::HStringIsNotEmpty(ahspSkinClass) && ahspSkinClass != mhspClass) {
    mhspSkinClass = ahspSkinClass;
  }
  else {
    mhspSkinClass = NULL;
  }
  SendMessage(eUIMessage_SkinChanged,niVarNull,niVarNull);
}

///////////////////////////////////////////////
iHString* __stdcall cWidget::GetSkinClass() const
{
  return (HStringIsNotEmpty(mhspSkinClass)) ? mhspSkinClass : mhspClass;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetText(iHString* ahspText) {
  if (mhspText == ahspText)
    return;
  mhspText = niGetIfOK(ahspText);
  if (!mhspText.IsOK()) {
    mhspText = _H("");
    mhspLocalizedText = mhspText;
  }
  else {
    mhspLocalizedText = this->FindLocalized(mhspText);
  }
  if (niFlagIsNot(mStatus,WDGSTATUS_SETTEXTLOCKED)) { // calling from get text message ?
    niFlagOn(mStatus,WDGSTATUS_SETTEXTLOCKED);
    SendMessage(eUIMessage_TextChanged,niVarNull,niVarNull);
    niFlagOff(mStatus,WDGSTATUS_SETTEXTLOCKED);
    this->Redraw();
  }
}

///////////////////////////////////////////////
void __stdcall cWidget::RetrieveText(tBool abSerialize)
{
  // ask the widget to update the text
  niFlagOn(mStatus,WDGSTATUS_SETTEXTLOCKED);
  SendMessage(eUIMessage_SetText,abSerialize,niVarNull);
  niFlagOff(mStatus,WDGSTATUS_SETTEXTLOCKED);
}

///////////////////////////////////////////////
iHString* __stdcall cWidget::GetText() const {
  if (niFlagIsNot(mStatus,WDGSTATUS_SETTEXTLOCKED)) { // calling from text changed message ?
    niThis(cWidget)->RetrieveText(eFalse);
  }
  return mhspText;
}

///////////////////////////////////////////////
iHString* __stdcall cWidget::GetLocalizedText() const {
  if (niFlagIsNot(mStatus,WDGSTATUS_SETTEXTLOCKED)) { // calling from text changed message ?
    niThis(cWidget)->RetrieveText(eFalse);
  }
  return mhspLocalizedText;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetHoverText(iHString* ahspText)
{
  if (mhspHoverText != ahspText) {
    mhspHoverText = ahspText;
    ResetHoverWidget(eTrue);
    SendMessage(eUIMessage_HoverTextChanged,niVarNull,niVarNull);
  }
}

///////////////////////////////////////////////
iHString* __stdcall cWidget::GetHoverText() const
{
  return mhspHoverText;
}

///////////////////////////////////////////////
tBool cWidget::SetFont(iFont *apFont)
{
  mptrFont = niGetIfOK(apFont);
  if (!mptrFont.IsOK()) {
    mptrFont = FindSkinFont(NULL,NULL,_H("Default"));
  }
  if (mptrFont.IsOK()) {
    mptrFont = mptrFont->CreateFontInstance(NULL);
  }
  SendMessage(eUIMessage_FontChanged,niVarNull,niVarNull);
  return mptrFont.IsOK();
}

///////////////////////////////////////////////
iFont * cWidget::GetFont() const
{
  return mptrFont;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::SetDockStyle(eWidgetDockStyle aStyle)
{
  mDockStyle = aStyle;
  NotifyChangedDockStyle();
  return eTrue;
}

///////////////////////////////////////////////
eWidgetDockStyle __stdcall cWidget::GetDockStyle() const
{
  return mDockStyle;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::ApplyDockStyle(eWidgetDockStyle aStyle)
{
  // Make sure that the widget has been layed out, twice because the child
  // widgets might have to be layed out before this widget can compute its
  // size.
  niLoop(i,2) {
    this->ComputeAutoLayout(this->GetAutoLayout());
  }
  return Place(sRectf::Null(),aStyle,GetMargin());
}

///////////////////////////////////////////////
tBool __stdcall cWidget::Place(const sRectf& aRect, eWidgetDockStyle aStyle, const sVec4f& avMargin)
{
  QPtr<cWidget> ptrParent(mpwParent);
  if (!ptrParent.IsOK())
    return eFalse;
  eWidgetDockStyle prevDockStyle = GetDockStyle();
  sVec4f prevMargin = GetMargin();
  if (aRect != sRectf::Null()) {
    const sVec2f parentSize = ptrParent->GetSize();
    sRectf rect = aRect;
    if (aRect.Left() <= 1.0f) {
      rect.Left() = aRect.Left()*parentSize.x;
    }
    if (aRect.Top() <= 1.0f) {
      rect.Top() = aRect.Top()*parentSize.y;
    }
    if (aRect.GetWidth() <= 1.0f) {
      rect.SetWidth(aRect.GetWidth()*parentSize.x);
    }
    if (aRect.GetHeight() <= 1.0f) {
      rect.SetHeight(aRect.GetHeight()*parentSize.y);
    }
    SetRect(rect);
  }
  SetMargin(avMargin);
  SetDockStyle(aStyle);
  ptrParent->ComputeAutoLayout(ptrParent->GetAutoLayout());
  SetMargin(prevMargin);
  SetDockStyle(prevDockStyle);
  return eTrue;
}

///////////////////////////////////////////////
cWidget* cWidget::GetMessageTargetByPos(
    const sVec2f &pos,
    tBool (*apfnExcludeWidget)(const cWidget* w, tIntPtr apUserData),
    tIntPtr apUserData) const
{
  if (!GetVisible())
    return NULL;

  if (apfnExcludeWidget && apfnExcludeWidget(this,apUserData))
    return NULL;

  cWidget* ret = mZMap.GetTargetByPos(pos,apfnExcludeWidget,apUserData);
  if (ret) return ret;

  if (niFlagIsNot(mStatus,WDGSTATUS_DUMMY)) { // Dummies never get any input messages
    // checks if is in this window
    sRectf rect = GetAbsoluteClippedRect();
    if (rect.Intersect(pos)) {
      // is this window
      return const_cast<cWidget*>(this);
    }
  }

  return NULL;
}

///////////////////////////////////////////////
void cWidget::AddChild(cWidget *pChild)
{
  CheckValid(;);
  niAssert(niIsOK(pChild));
  if (niFlagIs(pChild->GetStyle(),eWidgetStyle_Free)) {
    mvecFreeChildren.push_back(pChild);
  }
  else {
    mvecClipChildren.push_back(pChild);
  }
  pChild->SetZOrder(pChild->GetZOrder()); // this updates the layout
  mpUICtx->UpdateFreeWidgets();
  this->SendMessage(eUIMessage_ChildAdded,pChild,niVarNull);
}

///////////////////////////////////////////////
void cWidget::RemoveChild(cWidget *pChild)
{
  //CheckValid(;);
  Ptr<iWidget> ptrChild = pChild; // to make sure a reference exist while removing, avoid getting a dangling pointer.
  tBool bRes = astl::find_erase(mvecClipChildren,pChild);
  if (!bRes) {
    bRes = astl::find_erase(mvecFreeChildren,pChild);
  }
  if (bRes) {
    iWidgetZMap* pOwnerZMap = pChild->_GetOwnerZMap();
    if (pOwnerZMap) {
      pOwnerZMap->RemoveOfZMap(pChild);
    }
    UpdateParentAutoLayout("Parent-RemoveChild");
    if (mpUICtx)
      mpUICtx->UpdateFreeWidgets();
    this->SendMessage(eUIMessage_ChildRemoved,pChild,niVarNull);
  }
}

///////////////////////////////////////////////
void cWidget::HandleMessage(const tU32 msg, const Var& A, const Var& B) {
  if (niFlagIs(mStatus,WDGSTATUS_INVALID))
    return;

  cWidget* pThis = this;
  niGuardObject(pThis);

  tBool bHandled = eFalse;
  // Behavior if the widget is not enabled
  if (!pThis->GetEnabled()) {
#pragma niNote("If a new UI message is added, it should be handled properly here.")
    tU8  msgType = niMessageID_GetCharD(msg);
    if (msg == eUIMessage_SetCursor) {
      _UIInputMMTrace(niFmt(_A("### cWidget: SetCursor: ActivateStandardCursor: %p (ID:%s)."),(tIntPtr)this,niHStr(this->GetID())));
      pThis->mpUICtx->ActivateStandardCursor(eUIStandardCursor_Arrow);
      return;
    }
    else {
      switch (msgType) {
        case 'C':
        case 'I':
        case 'A':
          return;
      }
    }
  }

  // Reset the hover widget
  if (niFlagIs(mStatus,WDGSTATUS_HASHOVERWIDGET)) {
    switch (msg) {
      case eUIMessage_KeyDown: {
        const tU32 key = A.mU32;
        if (key == eKey_LControl ||
            key == eKey_RControl ||
            key == eKey_LShift ||
            key == eKey_RShift ||
            key == eKey_LAlt ||
            key == eKey_RAlt)
          break;
        niFallthrough;
      }
      case eUIMessage_FingerDown:
      case eUIMessage_RightClickDown:
      case eUIMessage_LeftClickDown:
      case eUIMessage_LostFocus:
      case eUIMessage_MouseLeave: {
        pThis->ResetHoverWidget(eFalse);
        break;
      }
    }
  }

  // Message that have to be pre-handled
  switch (msg) {
    case eUIMessage_Destroy: {
      break;
    }
    case eUIMessage_Layout:
      {
        if (niFlagIs(pThis->mStatus,WDGSTATUS_UPDATINGAUTOLAYOUT))
          return; // updating currently we don't want to
        // update again, otherwise we'll get
        // into an infinite message loop
        niFlagOn(pThis->mStatus,WDGSTATUS_UPDATINGAUTOLAYOUT);
        break;
      }
    case eUIMessage_Timer:
      {
        const tU32 timerId = A.GetU32();
        if (timerId == eWidgetSystemTimer_Hover) {
          sVec2f mousePos = pThis->GetUIContext()->GetCursorPosition();
          mousePos -= pThis->GetAbsolutePosition();
          pThis->SendMessage(eUIMessage_MouseHover,mousePos-pThis->GetClientPosition(),mousePos);
          pThis->SetTimer(eWidgetSystemTimer_Hover,-1);
        }
        else if (timerId == eWidgetSystemTimer_NCHover) {
          sVec2f mousePos = pThis->GetUIContext()->GetCursorPosition();
          mousePos -= pThis->GetAbsolutePosition();
          pThis->SendMessage(eUIMessage_NCMouseHover,mousePos-pThis->GetClientPosition(),mousePos);
          pThis->SetTimer(eWidgetSystemTimer_NCHover,-1);
        }
        break;
      }
    case eUIMessage_Size: {
      pThis->Redraw();
      break;
    }
    case eUIMessage_LocaleChanged: {
      if (HStringIsNotEmpty(pThis->mhspText)) {
        pThis->mhspLocalizedText = pThis->FindLocalized(pThis->mhspText);
      }
      else {
        pThis->mhspLocalizedText = pThis->mhspText;
      }
      break;
    }
  }

  if (niFlagIsNot(pThis->GetStyle(),eWidgetStyle_NoClick)) {
    // Click messsages
    switch(msg) {
      case eUIMessage_NCLeftClickDown:
      case eUIMessage_LeftClickDown: {
        niFlagOn(pThis->mClick,WDGCLICK_LEFTDOWN);
        break;
      }
      case eUIMessage_NCLeftClickUp:
      case eUIMessage_LeftClickUp: {
        if (niFlagIs(pThis->mClick,WDGCLICK_LEFTDOWN)) {
          tBool bIsClient = pThis->GetClippedRect().Intersect(A.GetVec2f());
          pThis->SendMessage(bIsClient?eUIMessage_LeftClick:eUIMessage_NCLeftClick,A,B);
          niFlagOff(pThis->mClick,WDGCLICK_LEFTDOWN);
        }
        break;
      }
      case eUIMessage_NCLeftDoubleClick:
      case eUIMessage_LeftDoubleClick: {
        niFlagOff(pThis->mClick,WDGCLICK_LEFTDOWN); // cancel the next click msg on Up
        break;
      }
      case eUIMessage_NCRightClickDown:
      case eUIMessage_RightClickDown: {
        niFlagOn(pThis->mClick,WDGCLICK_RIGHTDOWN);
        break;
      }
      case eUIMessage_NCRightClickUp:
      case eUIMessage_RightClickUp: {
        if (niFlagIs(pThis->mClick,WDGCLICK_RIGHTDOWN)) {
          tBool bIsClient = pThis->GetClippedRect().Intersect(A.GetVec2f());
          pThis->SendMessage(bIsClient?eUIMessage_RightClick:eUIMessage_NCRightClick,A,B);
          niFlagOff(pThis->mClick,WDGCLICK_RIGHTDOWN);
        }
        break;
      }
      case eUIMessage_NCRightDoubleClick:
      case eUIMessage_RightDoubleClick: {
        niFlagOn(pThis->mClick,WDGCLICK_RIGHTDOWN); // cancel the next click msg on Up
        break;
      }
    }
  }

  Ptr<iMessageDesc> msgDesc;

  // Submit message
  if (pThis->mnInputSubmitFlags) {
    tBool bSendSubmit = eFalse;
    switch(msg) {
      case eUIMessage_LeftClick: {
        if (niFlagIs(pThis->mnInputSubmitFlags,eUIInputSubmitFlags_LeftClick))
          bSendSubmit = eTrue;
        break;
      }
      case eUIMessage_LeftDoubleClick: {
        if (niFlagIs(pThis->mnInputSubmitFlags,eUIInputSubmitFlags_LeftDoubleClick))
          bSendSubmit = eTrue;
        break;
      }
      case eUIMessage_RightClick: {
        if (niFlagIs(pThis->mnInputSubmitFlags,eUIInputSubmitFlags_RightClick))
          bSendSubmit = eTrue;
        break;
      }
      case eUIMessage_RightDoubleClick: {
        if (niFlagIs(pThis->mnInputSubmitFlags,eUIInputSubmitFlags_RightDoubleClick))
          bSendSubmit = eTrue;
        break;
      }
    }
    if (bSendSubmit) {
      if (!msgDesc.IsOK()) {
        msgDesc = ni::GetConcurrent()->CreateMessageDesc(pThis,msg,A,B);
      }
      pThis->SendMessage(eUIMessage_Submit,pThis,msgDesc.ptr());
    }
  }

  Ptr<tWidgetSinkLst::tImmutableCollection> coll = pThis->mlstSinks->_ImmutableCollection();
  if (coll.IsOK()) {
    tWidgetSinkLst::const_reverse_iterator ritr = coll->rbegin();
    while (ritr != coll->rend()) {
      bHandled = (*ritr)->OnWidgetSink(pThis, msg, A, B);
      // In case this widget is invalidated by this message handler
      if (niFlagIs(mStatus,WDGSTATUS_INVALID))
        return;
      if (bHandled)
        break;
      ++ritr;
    }
  }
  if (!bHandled)
  {
    switch (msg) {
      case eUIMessage_SkinChanged: {
        pThis->SetFont(NULL);
        break;
      }
      case eUIMessage_MouseHover: {
        Ptr<iWidget> w = pThis->GetChildFromID(_HC(__ID_HoverWidget__));
        if (!w.IsOK()) {
          // No hover widget created or shown, create the
          // default hover label... Hover text is localized
          // with the system's locale.
          tHStringPtr hspHoverText =
              ni::HStringIsNotEmpty(pThis->mhspHoverText) ?
              pThis->mhspHoverText->GetLocalized() :
              NULL;
          if (ni::HStringIsNotEmpty(hspHoverText)) {
            w = pThis->CreateDefaultHoverWidget(hspHoverText);
          }
        }
        pThis->ShowHoverWidget(w,B.GetVec2f()+pThis->GetAbsolutePosition());
        break;
      }
      case eUIMessage_Paint:
        break;
      case eUIMessage_Size:
        break;
      case eUIMessage_Padding:
      case eUIMessage_NCSize:
        {
          sRectf rect = ni::Rectf(0,0,mRect.GetWidth(),mRect.GetHeight());
          if (pThis->mvPadding != sVec4f::Zero()) {
            rect.x += pThis->mvPadding.x;
            rect.y += pThis->mvPadding.y;
            rect.z -= pThis->mvPadding.z;
            rect.w -= pThis->mvPadding.w;
          }
          pThis->SetClientRect(rect);
          break;
        }
      case eUIMessage_Activate:
        break;
      case eUIMessage_SetCursor: {
        _UIInputMMTrace(niFmt(_A("### cWidget: SetCursor: ActivateStandardCursor: %p (ID:%s)."),(tIntPtr)this,niHStr(this->GetID())));
        pThis->mpUICtx->ActivateStandardCursor(eUIStandardCursor_Arrow);
        break;
      }
      case eUIMessage_LostFocus:
        break;
      case eUIMessage_SetFocus:
        break;
      case eUIMessage_LeftClickDown:
        break;
      case eUIMessage_LeftClickUp:
        break;
      case eUIMessage_RightClickDown:
        break;
      case eUIMessage_RightClickUp:
        pThis->SendMessage(eUIMessage_ContextMenu,(tU32)eTrue,A);
        break;
      case eUIMessage_ContextMenu:
        {
          if (pThis->mpwContextMenu.IsOK()) {
            sVec2f vAbsMousePos = {0,0};
            if (A.mBool) {
              vAbsMousePos = B.GetVec2f();
            }
            else {
            }
            vAbsMousePos += pThis->GetAbsolutePosition()+pThis->GetClientPosition();
            QPtr<iWidgetMenu> ptrMenu = pThis->mpwContextMenu.ptr();
            pThis->mpwContextMenu->SetAbsolutePosition(vAbsMousePos);
            ptrMenu->Open();
          }
        }
        break;
      case eUIMessage_NCMouseMove:
      case eUIMessage_MouseMove:
        break;
      case eUIMessage_NCWheel:
      case eUIMessage_Wheel:
        { // look for a child vertical scroll bar and send it the message
          tBool bSent = eFalse;
          for (tCWidgetVec::const_iterator mitr = pThis->mvecClipChildren.begin(); mitr !=  pThis->mvecClipChildren.end(); ++mitr)
          {
            iWidget* pWidget = (*mitr);
            if (!QPtr<iWidgetScrollBar>(pWidget).IsOK())
              continue;
            if (niFlagIsNot(pWidget->GetStyle(),eWidgetScrollBarStyle_Horz) &&
                niFlagIsNot(pWidget->GetStyle(),eWidgetScrollBarStyle_Slider)) {
              pWidget->SendMessage(msg,A,B);
              bSent = eTrue;
            }
          }
          if (!bSent) {
            for (tCWidgetVec::const_iterator vitr = pThis->mvecFreeChildren.begin();
                 vitr != pThis->mvecFreeChildren.end(); ++vitr)
            {
              iWidget* pWidget = (*vitr);
              if (!QPtr<iWidgetScrollBar>(pWidget).IsOK())
                continue;
              if (niFlagIsNot(pWidget->GetStyle(),eWidgetScrollBarStyle_Horz) &&
                  niFlagIsNot(pWidget->GetStyle(),eWidgetScrollBarStyle_Slider))
              {
                pWidget->SendMessage(msg,A,B);
                bSent = eTrue;
              }
            }
          }
          {
            if (!bSent) {
              QPtr<cWidget> ptrThisParent(pThis->mpwParent);
              if (ptrThisParent.IsOK()) {
                ptrThisParent->SendMessage(msg,A,B);
              }
            }
          }
          break;
        }
      case eUIMessage_Layout:
        pThis->ComputeAutoLayout(A.mU32);
        break;
      case eUIMessage_SerializeLayout:
        pThis->SerializeLayout(ni::VarQueryInterface<iDataTable>(A),B.mU32);
        break;
      case eUIMessage_SerializeWidget:
        break;
      case eUIMessage_SerializeChildren:
        pThis->SerializeChildren(ni::VarQueryInterface<iDataTable>(A),B.mU32);
        break;
      case eUIMessage_Cancel:
      case eUIMessage_Submit:
      case eUIMessage_Cut:
      case eUIMessage_Copy:
      case eUIMessage_Paste:
      case eUIMessage_Command:
      case eUIMessage_Notify:
        { // Send the message to the first parent
          QPtr<cWidget> ptrThisParent(pThis->mpwParent);
          if (ptrThisParent.IsOK()) {
            ptrThisParent->SendMessage(msg,A,B);
          }
          break;
        }
      case eUIMessage_MoveFocus: {
        pThis->MoveFocus(A.mBool);
        break;
      }
      default:
        break;
    }
  }

  // post process of messages
  switch (msg) {
    case eUIMessage_ChildAdded:
      pThis->Relayout("Msg_ChildAdded");
      break;
    case eUIMessage_ChildRemoved:
      pThis->Relayout("Msg_ChildRemoved");
      break;
    case eUIMessage_Layout: {
      niFlagOff(pThis->mStatus,WDGSTATUS_UPDATINGAUTOLAYOUT);
      break;
    }
    case eUIMessage_NCSize: {
      if (A.GetVec2f() != B.GetVec2f()) {
        pThis->UpdateParentAutoLayout("Parent-Msg_NCSize");
      }
      break;
    }
    case eUIMessage_Margin: {
      pThis->UpdateParentAutoLayout("Parent-Msg_Margin");
      break;
    }
    case eUIMessage_Visible: {
      pThis->UpdateParentAutoLayout("Parent-Msg_Visible");
      break;
    }
    case eUIMessage_Size: {
      if (A.GetVec2f() != B.GetVec2f()) {
        pThis->Relayout("Msg_Size");
      }
      break;
    }
  }

  // Notify message
  if (niFlagIs(pThis->GetStyle(),eWidgetStyle_NotifyParent) &&
      (msg != eUIMessage_Notify))
  {
    QPtr<cWidget> ptrThisParent(pThis->mpwParent);
    if (ptrThisParent.IsOK()) {
      if (!msgDesc.IsOK()) {
        msgDesc = ni::GetConcurrent()->CreateMessageDesc(pThis,msg,A,B);
      }
      ptrThisParent->SendMessage(eUIMessage_Notify,pThis,msgDesc.ptr());
    }
  }
}

///////////////////////////////////////////////
tBool cWidget::MapToClientRect(const sVec2f &pos,sVec2f &clientPos) const
{
  clientPos = pos - mClientRect.GetTopLeft();
  return mClientRect.Intersect(pos);
}

///////////////////////////////////////////////
tU32 cWidget::GetChildDrawOrder(const iWidget* apW) const
{
  CheckValid(eInvalidHandle);
  return mZMap.GetDrawOrder(apW);
}

///////////////////////////////////////////////
tBool cWidget::HasParent(const iWidget* apW) const {
  QPtr<cWidget> p = mpwParent;
  while (p.IsOK()) {
    if (p == apW)
      return eTrue;
    p = p->mpwParent;
  }
  return eFalse;
}
tBool cWidget::HasChild(const iWidget* apW, tBool abRecursive) const
{
  tCWidgetVec::const_iterator it;
  for (it = mvecClipChildren.begin(); it != mvecClipChildren.end(); ++it) {
    if (*it == apW) return eTrue;
    if (abRecursive) {
      if ((*it)->HasChild(apW,abRecursive))
        return eTrue;
    }
  }
  for (it = mvecFreeChildren.begin(); it != mvecFreeChildren.end(); ++it) {
    if (*it == apW) return eTrue;
    if (abRecursive) {
      if ((*it)->HasChild(apW,abRecursive))
        return eTrue;
    }
  }
  return eFalse;
}

///////////////////////////////////////////////
void __stdcall cWidget::SetAutoLayout(tWidgetAutoLayoutFlags aFlags)
{
  if (aFlags == mnAutoLayout) return;
  mnAutoLayout = aFlags;
  this->Relayout("AutoLayout");
}

///////////////////////////////////////////////
tWidgetAutoLayoutFlags __stdcall cWidget::GetAutoLayout() const
{
  return mnAutoLayout;
}

///////////////////////////////////////////////
void cWidget::ComputeAutoLayout(tWidgetAutoLayoutFlags aFlags)
{
  if (!aFlags || mZMap.IsEmpty())
    return;

  if (niFlagIs(aFlags,eWidgetAutoLayoutFlags_Relative)) {
    /*for (wdgIt = wdgList.begin(); wdgIt != wdgList.end(); ++wdgIt) {
      iWidget* pChild = *wdgIt;
      static_cast<cWidget*>(pChild)->ApplyRelativeRect();
      }*/
    for (tCWidgetVec::iterator itr=mvecClipChildren.begin(); itr != mvecClipChildren.end(); ++itr) {
      (*itr)->ApplyRelativeRect();
    }
    for (tCWidgetVec::iterator itr=mvecFreeChildren.begin(); itr != mvecFreeChildren.end(); ++itr) {
      (*itr)->ApplyRelativeRect();
    }
  }

  if (niFlagIs(aFlags,eWidgetAutoLayoutFlags_Dock)) {
    tF32 clGridX = 0.0f, clGridY = 0.0f, clGridLineHeight = 0.0f;
    tU32 clGridLineCount = 0;
    sVec2f clGridSize = GetClientSize();

    tF32 ncGridX = 0.0f, ncGridY = 0.0f, ncGridLineHeight = 0.0f;
    tU32 ncGridLineCount = 0;
    sVec2f ncGridSize = GetSize();

    mrectDockFillClient = GetClientRect();
    mrectDockFillClient.MoveTo(Vec2<tF32>(0,0));

    const sRectf orgRectDockFillNonClient = GetRect();
    mrectDockFillNonClient = orgRectDockFillNonClient;
    mrectDockFillNonClient.MoveTo(Vec2<tF32>(0,0));

    astl::list<cWidget*>::const_iterator wdgIt;
    astl::list<cWidget*> wdgList;
    mZMap.GetReverseList(wdgList);
    for (wdgIt = wdgList.begin(); wdgIt != wdgList.end(); ++wdgIt) {
      iWidget* pChild = *wdgIt;
      if (!pChild->GetVisible())
        continue;
      eWidgetDockStyle dockStyle = pChild->GetDockStyle();

      if (niFlagIs(pChild->GetStyle(),eWidgetStyle_Free)) {
        switch (dockStyle) {
          case eWidgetDockStyle_SnapLeft:
          case eWidgetDockStyle_SnapRight:
          case eWidgetDockStyle_SnapTop:
          case eWidgetDockStyle_SnapBottom:
          case eWidgetDockStyle_SnapLeftCenter:
          case eWidgetDockStyle_SnapRightCenter:
          case eWidgetDockStyle_SnapTopCenter:
          case eWidgetDockStyle_SnapBottomCenter:
          case eWidgetDockStyle_SnapTopLeft:
          case eWidgetDockStyle_SnapTopRight:
          case eWidgetDockStyle_SnapBottomLeft:
          case eWidgetDockStyle_SnapBottomRight:
          case eWidgetDockStyle_SnapCenter:
            break;
          default:
            dockStyle = eWidgetDockStyle_None;
            break;
        }
      }

      if (dockStyle == eWidgetDockStyle_None) {
        continue;
      }
      else if (dockStyle == eWidgetDockStyle_Grid) {
        const sVec4f vMargin = pChild->GetMargin();
        const sVec2f chSize = pChild->GetSize();
        if (niFlagIs(pChild->GetStyle(),eWidgetStyle_NCRelative)) {
          // if new line
          if (ncGridLineCount && ((ncGridX+chSize.x+vMargin.x) > ncGridSize.x)) {
            ncGridLineCount = 0;
            ncGridLineHeight = 0.0f;
            ncGridY += ncGridLineHeight;
            ncGridX = 0.0f;
          }
          pChild->SetPosition(Vec2<tF32>(ncGridX+vMargin.x,ncGridY+vMargin.y));
          ncGridX += chSize.x + vMargin.x + vMargin.z;
          ncGridLineHeight = ni::Max(ncGridLineHeight,chSize.y+vMargin.y+vMargin.w);
          ++ncGridLineCount;  // line item count
          // niDebugFmt(("... NCGrid: %s, to %s, now at %s", pChild->GetID(), Vec2<tF32>(clGridX,clGridY), pChild->GetRect()));
        }
        else {
          // if new line
          if (clGridLineCount && ((clGridX+chSize.x+vMargin.x) > clGridSize.x)) {
            clGridY += clGridLineHeight;
            clGridX = 0.0f;
            clGridLineCount = 0;
            clGridLineHeight = 0.0f;
          }
          pChild->SetPosition(Vec2<tF32>(clGridX+vMargin.x,clGridY+vMargin.y));
          clGridX += chSize.x + vMargin.x + vMargin.z;
          clGridLineHeight = ni::Max(clGridLineHeight,chSize.y+vMargin.y+vMargin.w);
          ++clGridLineCount;  // line item count
          // niDebugFmt(("... Grid: %s, to %s, now at %s", pChild->GetID(), Vec2<tF32>(clGridX,clGridY), pChild->GetRect()));
        }
      }
      else {
        enum {
          SnapLeft = niBit(0),
          SnapRight = niBit(1),
          SnapTop = niBit(2),
          SnapBottom = niBit(3),
          SnapCenterH = niBit(4),
          SnapCenterV = niBit(5),
          Dock = niBit(6)
        };
        tU32 sides = 0;
        sRectf rectThis;
        sRectf* rectFill;

        if (niFlagIs(pChild->GetStyle(),eWidgetStyle_Free)) {
          rectThis = mpUICtx->GetRootWidget()->GetRect();
          rectFill = &mrectDockFillNonClient;
        }
        else if (niFlagIs(pChild->GetStyle(),eWidgetStyle_NCRelative)) {
          rectThis = GetRect();
          rectFill = &mrectDockFillNonClient;
        }
        else {
          rectThis = GetClientRect();
          rectFill = &mrectDockFillClient;
        }

        const sVec4f vMargin = pChild->GetMargin();
        sRectf rectChild = pChild->GetRect();
        switch (dockStyle) {
          case eWidgetDockStyle_SnapLeft:     sides |= SnapLeft; break;
          case eWidgetDockStyle_SnapRight:    sides |= SnapRight; break;
          case eWidgetDockStyle_SnapTop:      sides |= SnapTop; break;
          case eWidgetDockStyle_SnapBottom:   sides |= SnapBottom; break;
          case eWidgetDockStyle_SnapTopLeft:    sides |= SnapTop; sides |= SnapLeft; break;
          case eWidgetDockStyle_SnapTopRight:   sides |= SnapTop; sides |= SnapRight; break;
          case eWidgetDockStyle_SnapBottomLeft: sides |= SnapBottom; sides |= SnapLeft; break;
          case eWidgetDockStyle_SnapBottomRight:  sides |= SnapBottom; sides |= SnapRight; break;
          case eWidgetDockStyle_SnapLeftCenter: sides |= SnapLeft; sides |= SnapCenterV; break;
          case eWidgetDockStyle_SnapRightCenter:  sides |= SnapRight; sides |= SnapCenterV; break;
          case eWidgetDockStyle_SnapTopCenter:  sides |= SnapTop; sides |= SnapCenterH; break;
          case eWidgetDockStyle_SnapBottomCenter: sides |= SnapBottom; sides |= SnapCenterH; break;
          case eWidgetDockStyle_SnapCenter:   sides |= SnapCenterH; sides |= SnapCenterV; break;
          case eWidgetDockStyle_SnapCenterH:   sides |= SnapCenterH; break;
          case eWidgetDockStyle_SnapCenterV:   sides |= SnapCenterV; break;
          case eWidgetDockStyle_DockLeft:
            {
              rectChild.SetHeight(rectFill->GetHeight());
              rectChild.MoveTo(rectFill->GetTopLeft());
              _ApplyMargin(rectChild,vMargin,MarginLeft);
              rectFill->SetLeft(ni::Min(rectFill->GetRight(),rectChild.GetRight()+vMargin.z));
              break;
            }
          case eWidgetDockStyle_DockRight:
            {
              rectChild.SetHeight(rectFill->GetHeight());
              rectChild.MoveTo(Vec2<tF32>(rectFill->GetRight()-rectChild.GetWidth(),rectFill->GetTop()));
              _ApplyMargin(rectChild,vMargin,MarginRight);
              rectFill->SetRight(ni::Max(rectFill->GetLeft(),rectChild.GetLeft()-vMargin.x));
              break;
            }
          case eWidgetDockStyle_DockTop:
            {
              rectChild.SetWidth(rectFill->GetWidth());
              rectChild.MoveTo(rectFill->GetTopLeft());
              _ApplyMargin(rectChild,vMargin,MarginTop);
              rectFill->SetTop(ni::Min(rectFill->GetBottom(),rectChild.GetBottom()+vMargin.w));
              break;
            }
          case eWidgetDockStyle_DockBottom:
            {
              rectChild.SetWidth(rectFill->GetWidth());
              rectChild.MoveTo(Vec2<tF32>(rectFill->GetLeft(),
                                             rectFill->GetBottom()-rectChild.GetHeight()));
              _ApplyMargin(rectChild,vMargin,MarginBottom);
              rectFill->SetBottom(ni::Max(rectFill->GetTop(),rectChild.GetTop()-vMargin.y));
              break;
            }
          case eWidgetDockStyle_DockFill:
            {
              rectChild = *rectFill;
              // with DockFill the margin is applied as a form of padding
              _ApplyPadding(rectChild,vMargin,MarginAll);
              break;
            }
          case eWidgetDockStyle_DockFillWidth:
            {
              rectChild.SetLeft(rectFill->GetLeft());
              rectChild.SetRight(rectFill->GetRight());
              // with DockFill the margin is applied as a form of padding
              _ApplyPadding(rectChild,vMargin,MarginLeft|MarginRight);
              break;
            }
          case eWidgetDockStyle_DockFillHeight:
            {
              rectChild.SetTop(rectFill->GetTop());
              rectChild.SetBottom(rectFill->GetBottom());
              // with DockFill the margin is applied as a form of padding
              _ApplyPadding(rectChild,vMargin,MarginTop|MarginBottom);
              break;
            }
          case eWidgetDockStyle_DockFillOverlay:
            {
              rectChild = rectThis;
              rectChild.MoveTo(Vec2<tF32>(0,0));
              // with DockFill the margin is applied as a form of padding
              _ApplyPadding(rectChild,vMargin,MarginAll);
              break;
            };
          default:
            niAssertMsg(0,_A("Unreachable code."));
            break;
        }

        if (sides) {
          sVec2f vPos = pChild->GetPosition();
          if (niFlagIs(sides,SnapLeft)) {
            vPos.x = vMargin.Left();
          }
          else if (niFlagIs(sides,SnapRight)) {
            vPos.x = rectThis.GetWidth()-pChild->GetSize().x-vMargin.Right();
          }
          else if (niFlagIs(sides,SnapCenterH)) {
            vPos.x = (rectThis.GetWidth()/2)-(pChild->GetSize().x/2);
          }
          if (niFlagIs(sides,SnapTop)) {
            vPos.y = vMargin.Top();
          }
          else if (niFlagIs(sides,SnapBottom)) {
            vPos.y = rectThis.GetHeight()-pChild->GetSize().y-vMargin.Bottom();
          }
          else if (niFlagIs(sides,SnapCenterV)) {
            vPos.y = (rectThis.GetHeight()/2)-(pChild->GetSize().y/2);
          }
          rectChild.MoveTo(vPos);
        }

        if (rectChild != pChild->GetRect()) {
          //                     niDebugFmt((_A("LAYOUT %s in %s: from %s to %s [THIS:%s] [FILL:%s]\n"),
          //                               niHStr(pChild->GetID()),
          //                               niHStr(GetID()),
          //                               _ASZ(pChild->GetRect()),
          //                               _ASZ(rectChild),
          //                               _ASZ(rectThis),
          //                               _ASZ(*rectFill)));
          pChild->SetRect(rectChild);
          //                     niDebugFmt((_A("NEW RECT %s\n"),
          //                               _ASZ(pChild->GetRect())));
        }
      }
    }
  } // if (abDock)

  if (niFlagIs(aFlags,eWidgetAutoLayoutFlags_Size)) {
    tBool canComputeW = eTrue;
    tBool canComputeH = eTrue;
    switch (mDockStyle) {
      case eWidgetDockStyle_DockLeft:
      case eWidgetDockStyle_DockRight:
      case eWidgetDockStyle_DockFillHeight:
        canComputeH = eFalse;
        break;
      case eWidgetDockStyle_DockTop:
      case eWidgetDockStyle_DockBottom:
      case eWidgetDockStyle_DockFillWidth:
        canComputeW = eFalse;
        break;
      case eWidgetDockStyle_DockFill:
      case eWidgetDockStyle_DockFillOverlay:
        canComputeW = eFalse;
        canComputeH = eFalse;
        break;
      default:
        break;
    }
    if (canComputeH || canComputeW) {
      sVec2f vNewSize = {0,0};
      for (tCWidgetVec::iterator itW = mvecClipChildren.begin(); itW != mvecClipChildren.end(); ++itW) {
        iWidget* pW = *itW;
        if (niFlagIs(pW->GetStyle(),eWidgetStyle_NCRelative))
          continue;
        const sRectf rect = pW->GetRect();
        const sVec4f vMargin = pW->GetMargin();
        tF32 right = rect.Right()+vMargin.Right();
        if (right > vNewSize.x) vNewSize.x = right;
        tF32 bottom = rect.Bottom()+vMargin.Bottom();
        if (bottom > vNewSize.y) vNewSize.y = bottom;
        // niDebugFmt(("... Size?: %s, now at %s, with margin %s, newSize: %s, visible?: %d",
        // pW->GetID(),
        // rect,vMargin,
        // vNewSize,
        // pW->GetVisible()));
      }
      if (vNewSize.x > 0 && vNewSize.y > 0) {
        sRectf newRect = ComputeFitRect(sRectf(0,0,vNewSize.x,vNewSize.y));
        newRect.MoveTo(GetAbsolutePosition());
        if (canComputeH && canComputeW) {
          // niDebugFmt((".... BeforeFitSize [%s]: %s, %s",
          // this->GetID(),this->GetAbsoluteRect(),
          // mDockStyle));
        }
        else if (canComputeW) {
          // niDebugFmt((".... BeforeFitSizeW [%s]: %s, %s",
          // this->GetID(),this->GetAbsoluteRect(),
          // mDockStyle));
          newRect.SetHeight(GetSize().y);
        }
        else if (canComputeH) {
          // niDebugFmt((".... BeforeFitSizeH [%s]: %s, %s",
          // this->GetID(),this->GetAbsoluteRect(),
          // mDockStyle));
          newRect.SetWidth(GetSize().x);
        }
        SetAbsoluteRect(newRect);
        // niDebugFmt((".... AfterFitSize [%s]: %s, %s",
        // this->GetID(),this->GetAbsoluteRect(),
        // mDockStyle));
      }
    }
  }
}

///////////////////////////////////////////////
void cWidget::UpdateParentAutoLayout(const achar* aaszReason)
{
  CheckValid(;);
  QPtr<cWidget> ptrParent(mpwParent);
  if (ptrParent.IsOK() && niFlagIsNot(ptrParent->mStatus,WDGSTATUS_UPDATINGAUTOLAYOUT)) {
    ptrParent->Relayout(aaszReason);
  }
}

///////////////////////////////////////////////
void cWidget::ApplyRelativeRect()
{
  QPtr<cWidget> ptrParent(mpwParent);
  if (!ptrParent.IsOK() || niFlagIsNot(mStatus,WDGSTATUS_RELATIVE))
    return;

  sRectf newRect = mRect;
  sVec2f vParentSize;

  if (niFlagIs(ptrParent->GetStyle(),eWidgetStyle_Free)) {
    vParentSize = mpUICtx->GetRootWidget()->GetSize();
  }
  else {
    vParentSize = niFlagIs(ptrParent->GetStyle(),eWidgetStyle_NCRelative)?
        ptrParent->GetSize():ptrParent->GetClientSize();
  }
  if (mrectRelative.Left() > niEpsilon5)
    newRect.Left() = UnitSnapf(mrectRelative.Left()*vParentSize.x);
  if (mrectRelative.Top() > niEpsilon5)
    newRect.Top() = UnitSnapf(mrectRelative.Top()*vParentSize.y);

  if (mrectRelative.GetWidth() > niEpsilon5)
    newRect.SetWidth(UnitSnapf(mrectRelative.GetWidth()*vParentSize.x));
  else if (mrectRelative.GetWidth() < -niEpsilon5)
    newRect.SetWidth(UnitSnapf(vParentSize.x+mrectRelative.GetWidth()));

  if (mrectRelative.GetHeight() > niEpsilon5)
    newRect.SetHeight(UnitSnapf(mrectRelative.GetHeight()*vParentSize.y));
  else if (mrectRelative.GetHeight() < -niEpsilon5)
    newRect.SetHeight(UnitSnapf(vParentSize.y+mrectRelative.GetHeight()));

  SetRect(newRect);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::SetContextMenu(iWidget* apMenu)
{
  if (apMenu == mpwContextMenu)
    return eTrue;
  if (!QPtr<iWidgetMenu>(apMenu).IsOK())
    mpwContextMenu = NULL;
  else
    mpwContextMenu = apMenu;
  this->SendMessage(eUIMessage_ContextMenuChanged,niVarNull,niVarNull);
  return mpwContextMenu.IsOK();
}

///////////////////////////////////////////////
iWidget* __stdcall cWidget::GetContextMenu() const
{
  return mpwContextMenu;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::SendMessage(tU32 aMsg, const Var& avarA, const Var& avarB)
{
  CheckValid(eFalse);
  if (mnThreadID == ni::ThreadGetCurrentThreadID()) {
    this->HandleMessage(aMsg,avarA,avarB);
    return eTrue;
  }
  else {
    return ni::SendMessage(this,aMsg,avarA,avarB);
  }
}

///////////////////////////////////////////////
tBool __stdcall cWidget::BroadcastMessage(tU32 aMsg, const Var& avarA, const Var& avarB)
{
  CheckValid(eFalse);
  for (tCWidgetVec::iterator itr=mvecClipChildren.begin(); itr != mvecClipChildren.end(); ++itr) {
    (*itr)->BroadcastMessage(aMsg,avarA,avarB);
  }
  for (tCWidgetVec::iterator itr=mvecFreeChildren.begin(); itr != mvecFreeChildren.end(); ++itr) {
    (*itr)->BroadcastMessage(aMsg,avarA,avarB);
  }
  return this->SendMessage(aMsg,avarA,avarB);
}

///////////////////////////////////////////////
tBool __stdcall cWidget::SendCommand(iWidget* apDest, tU32 anCommand, const Var& avarExtra1, const Var& avarExtra2)
{
  CheckValid(eFalse);
  if (!niIsOK(apDest)) return eFalse;
  Ptr<iWidgetCommand> ptrCmd = mpUICtx->CreateWidgetCommand();
  ptrCmd->SetSender(this);
  ptrCmd->SetID(anCommand);
  ptrCmd->SetExtra1(avarExtra1);
  ptrCmd->SetExtra2(avarExtra2);
  return mpUICtx->SendCommand(apDest,ptrCmd);
}

///////////////////////////////////////////////
void cWidget::ClearSinks()
{
  Ptr<tWidgetSinkLst::tImmutableCollection> coll = mlstSinks->_ImmutableCollection();
  if (coll.IsOK()) {
    for (tWidgetSinkLst::const_reverse_iterator itS = coll->rbegin(); itS != coll->rend(); ++itS) {
      (*itS)->OnWidgetSink(NULL,eUIMessage_SinkDetached,niVarNull,niVarNull);
    }
    mlstSinks->Clear();
  }
}

///////////////////////////////////////////////
tBool __stdcall cWidget::SerializeLayout(iDataTable* apDT, tWidgetSerializeFlags anFlags)
{
  CheckValid(eFalse);

  if (!niIsOK(apDT)) {
    niError(_A("Invalid data table."));
    return eFalse;
  }

  if (niFlagIs(anFlags,eWidgetSerializeFlags_Read)) {
    tU32 nPropertyIndex;
    nPropertyIndex = apDT->GetPropertyIndex(_A("style"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetStyle(apDT->GetEnumFromIndex(
          nPropertyIndex,
          niFlagsExpr(eWidgetStyle)|ni::eEnumToStringFlags_GlobalSearch));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("auto_layout"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetAutoLayout(apDT->GetEnumFromIndex(nPropertyIndex,niFlagsExpr(eWidgetAutoLayoutFlags)));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("zorder"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetZOrder((eWidgetZOrder)apDT->GetEnumFromIndex(nPropertyIndex,niEnumExpr(eWidgetZOrder)));
    }
    // rect
    nPropertyIndex = apDT->GetPropertyIndex(_A("rect"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetRect(apDT->GetVec4FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("position"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetPosition(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("size"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetSize(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("x"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetPosition();
      v.x = f;
      this->SetPosition(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("y"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetPosition();
      v.y = f;
      this->SetPosition(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("width"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetSize();
      v.x = f;
      this->SetSize(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("height"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetSize();
      v.y = f;
      this->SetSize(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("minimum_size"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetMinimumSize(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("maximum_size"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetMaximumSize(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    // relative rect
    nPropertyIndex = apDT->GetPropertyIndex(_A("relative_rect"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetRelativeRect(apDT->GetVec4FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("relative_position"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetRelativePosition(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("relative_size"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetRelativeSize(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("relative_x"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetRelativePosition();
      v.x = f;
      this->SetRelativePosition(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("relative_y"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetRelativePosition();
      v.y = f;
      this->SetRelativePosition(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("relative_width"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetRelativeSize();
      v.x = f;
      this->SetRelativeSize(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("relative_height"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetRelativeSize();
      v.y = f;
      this->SetRelativeSize(v);
    }
    // client rect
    nPropertyIndex = apDT->GetPropertyIndex(_A("client_rect"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetClientRect(apDT->GetVec4FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("client_position"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetClientPosition(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("client_size"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetClientSize(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("client_x"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetClientPosition();
      v.x = f;
      this->SetClientPosition(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("client_y"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetClientPosition();
      v.y = f;
      this->SetClientPosition(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("client_width"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetClientSize();
      v.x = f;
      this->SetClientSize(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("client_height"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetClientSize();
      v.y = f;
      this->SetClientSize(v);
    }
    // padding
    nPropertyIndex = apDT->GetPropertyIndex(_A("padding"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetPadding(apDT->GetVec4FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("padding_h"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetPadding();
      v.Left() = f;
      v.Right() = f;
      this->SetPadding(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("padding_left"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetPadding();
      v.Left() = f;
      this->SetPadding(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("padding_right"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetPadding();
      v.Right() = f;
      this->SetPadding(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("padding_v"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetPadding();
      v.Top() = f;
      v.Bottom() = f;
      this->SetPadding(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("padding_top"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetPadding();
      v.Top() = f;
      this->SetPadding(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("padding_bottom"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetPadding();
      v.Bottom() = f;
      this->SetPadding(v);
    }
    // margin
    nPropertyIndex = apDT->GetPropertyIndex(_A("margin"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetMargin(apDT->GetVec4FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("margin_h"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetMargin();
      v.Left() = f;
      v.Right() = f;
      this->SetMargin(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("margin_left"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetMargin();
      v.Left() = f;
      this->SetMargin(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("margin_right"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetMargin();
      v.Right() = f;
      this->SetMargin(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("margin_v"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetMargin();
      v.Top() = f;
      v.Bottom() = f;
      this->SetMargin(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("margin_top"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetMargin();
      v.Top() = f;
      this->SetMargin(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("margin_bottom"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec4f v = this->GetMargin();
      v.Bottom() = f;
      this->SetMargin(v);
    }
    // fit rect
    nPropertyIndex = apDT->GetPropertyIndex(_A("fit_rect"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetFitRect(apDT->GetVec4FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("fit_size"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetFitSize(apDT->GetVec2FromIndex(nPropertyIndex));
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("fit_width"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetClientSize();
      v.x = f;
      this->SetFitSize(v);
    }
    nPropertyIndex = apDT->GetPropertyIndex(_A("fit_height"));
    if (nPropertyIndex != eInvalidHandle) {
      tF32 f = apDT->GetFloatFromIndex(nPropertyIndex);
      sVec2f v = this->GetClientSize();
      v.y = f;
      this->SetFitSize(v);
    }
    // input submit flags
    nPropertyIndex = apDT->GetPropertyIndex(_A("input_submit_flags"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetInputSubmitFlags(apDT->GetEnumFromIndex(nPropertyIndex,niFlagsExpr(eUIInputSubmitFlags)));
    }
    // exclusive - disabled... should be set by the API...
    //    nPropertyIndex = apDT->GetPropertyIndex(_A("exclusive"));
    //    if (nPropertyIndex != eInvalidHandle) {
    //      this->SetExclusive(apDT->GetIntFromIndex(nPropertyIndex)?eTrue:eFalse);
    //    }
    // locale
    nPropertyIndex = apDT->GetPropertyIndex(_A("locale"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetLocale(_H(apDT->GetStringFromIndex(nPropertyIndex)));
    }
    // visible
    nPropertyIndex = apDT->GetPropertyIndex(_A("visible"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetVisible(apDT->GetIntFromIndex(nPropertyIndex)?eTrue:eFalse);
    }
    // enabled
    nPropertyIndex = apDT->GetPropertyIndex(_A("enabled"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetEnabled(apDT->GetIntFromIndex(nPropertyIndex)?eTrue:eFalse);
    }
    // ignore input
    nPropertyIndex = apDT->GetPropertyIndex(_A("ignore_input"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetIgnoreInput(apDT->GetIntFromIndex(nPropertyIndex)?eTrue:eFalse);
    }
    // dock style
    nPropertyIndex = apDT->GetPropertyIndex(_A("dock_style"));
    if (nPropertyIndex != eInvalidHandle) {
      this->SetDockStyle((eWidgetDockStyle)apDT->GetEnumFromIndex(nPropertyIndex,niFlagsExpr(eWidgetDockStyle)));
    }
    // text
    nPropertyIndex = apDT->GetPropertyIndex(_A("text"));
    if (nPropertyIndex != eInvalidHandle) {
      SetText(_H(apDT->GetStringFromIndex(nPropertyIndex)));
    }
    // hover_text
    nPropertyIndex = apDT->GetPropertyIndex(_A("hover_text"));
    if (nPropertyIndex != eInvalidHandle) {
      SetHoverText(_H(apDT->GetStringFromIndex(nPropertyIndex)));
    }
    // skin
    nPropertyIndex = apDT->GetPropertyIndex(_A("skin"));
    if (nPropertyIndex != eInvalidHandle) {
      SetSkin(_H(apDT->GetStringFromIndex(nPropertyIndex)));
    }
    // skin class
    nPropertyIndex = apDT->GetPropertyIndex(_A("skin_class"));
    if (nPropertyIndex != eInvalidHandle) {
      SetSkinClass(_H(apDT->GetStringFromIndex(nPropertyIndex)));
    }
    // font
    nPropertyIndex = apDT->GetPropertyIndex(_A("font"));
    if (nPropertyIndex != eInvalidHandle) {
      cString strFont = apDT->GetStringFromIndex(nPropertyIndex);
      if (!ni::StrEq(niHStr(this->GetFont()->GetName()),strFont.Chars())) {
        this->SetFont(mpUICtx->mptrGraphics->LoadFont(_H(strFont)));
      }
    }
    // font size
    nPropertyIndex = apDT->GetPropertyIndex(_A("font_size"));
    if (nPropertyIndex != eInvalidHandle) {
      tI32 fontSize = apDT->GetIntFromIndex(nPropertyIndex);
      if (fontSize >= 2 && fontSize <= 256) {
        this->GetFont()->SetSizeAndResolution(sVec2f::Zero(),fontSize,mpUICtx->mfContentsScale);
      }
    }
  }
  else {
    apDT->SetEnum(_A("style"),
                  niFlagsExprFull(eWidgetStyle)|ni::eEnumToStringFlags_GlobalSearch,
                  this->GetStyle());
    apDT->SetEnum(_A("auto_layout"), niFlagsExpr(eWidgetAutoLayoutFlags), this->GetAutoLayout());
    apDT->SetEnum(_A("zorder"),niEnumExpr(eWidgetZOrder),this->GetZOrder());
    apDT->SetVec2(_A("position"),this->GetPosition());
    apDT->SetVec2(_A("size"),this->GetSize());
    apDT->SetVec2(_A("minimum_size"),this->GetMinimumSize());
    apDT->SetVec2(_A("maximum_size"),this->GetMaximumSize());
    apDT->SetVec2(_A("relative_position"),this->GetRelativePosition());
    apDT->SetVec2(_A("relative_size"),this->GetRelativeSize());
    apDT->SetVec4(_A("padding"),this->GetPadding());
    apDT->SetVec4(_A("margin"),this->GetMargin());
    apDT->SetEnum(_A("input_submit_flags"),niFlagsExpr(eUIInputSubmitFlags),this->GetInputSubmitFlags());
    //    apDT->SetInt(_A("exclusive"),this->GetExclusive()); // disabled - should be set byt he api
    apDT->SetInt(_A("visible"),this->GetVisible());
    apDT->SetInt(_A("enabled"),this->GetEnabled());
    apDT->SetInt(_A("ignore_input"),this->GetIgnoreInput());
#pragma niTodo("Skin serialization")
    RetrieveText(eTrue);
    apDT->SetString(_A("text"),niHStr(mhspText));
    apDT->SetString(_A("hover_text"),niHStr(mhspHoverText));
    apDT->SetString(_A("font"),niHStr(this->GetFont()->GetName()));
    apDT->SetInt(_A("font_size"),this->GetFont()->GetResolution());
    apDT->SetString(_A("skin"),niHStr(this->GetSkin()));
    if (HStringIsNotEmpty(mhspSkinClass) && mhspSkinClass != mhspClass)
      apDT->SetString(_A("skin_class"),niHStr(this->GetSkinClass()));

    apDT->SetEnum(_A("dock_style"),niEnumExpr(eWidgetDockStyle),this->GetDockStyle());
    if (this->GetContextMenu()) {
      iWidget* pMenu = this->GetContextMenu();
      if (pMenu->GetParent() != this) {
        apDT->SetString(_A("context_menu_parent_id"),pMenu->GetParent()?HStringGetStringEmpty(pMenu->GetParent()->GetID()):AZEROSTR);
      }
      apDT->SetString(_A("context_menu_id"),HStringGetStringEmpty(pMenu->GetID()));
    }
    if (niFlagIs(anFlags,eWidgetSerializeFlags_PropertyBox)) {
      apDT->SetMetadata(_A("exclusive"),_H("bool"));
      apDT->SetMetadata(_A("visible"),_H("bool"));
      apDT->SetMetadata(_A("enabled"),_H("bool"));
      apDT->SetMetadata(_A("ignore_input"),_H("bool"));
      apDT->SetMetadata(_A("auto_layout"),_H("flags[*eWidgetAutoLayoutFlags]"));
      apDT->SetMetadata(_A("zorder"),_H("enum[*eWidgetZOrder]"));
      apDT->SetMetadata(_A("dock_style"),_H("enum[*eWidgetDockStyle]"));
      apDT->SetMetadata(_A("input_submit_flags"),_H("flags[*eUIInputSubmitFlags]"));
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::SerializeChildren(iDataTable* apDT, tWidgetSerializeFlags anFlags)
{
  if (!niIsOK(apDT)) {
    niError(_A("Invalid data table."));
    return eFalse;
  }
  tBool isChildrenTable = ni::StrEq(apDT->GetName(),_A("Children"));

  QPtr<iRegex> ptrRegex = apDT->GetIUnknown(_A("_regex_filter"));
  if (niFlagIs(anFlags,eWidgetSerializeFlags_Read)) {
    for (tU32 i = 0; i < apDT->GetNumChildren(); ++i) {
      Ptr<iWidget> ptrWidget = NULL;
      iDataTable* pChildDT = apDT->GetChildFromIndex(i);

      cString strClass = pChildDT->GetString(_A("class"));

      tBool isWidgetTable = ni::StrEq(pChildDT->GetName(),_A("Widget"));
      if (!isWidgetTable) {
        if (strClass.empty() && mpUICtx->HasWidgetSinkClass(pChildDT->GetName())) {
          strClass = pChildDT->GetName();
        }
        else if (!isChildrenTable) {
          // not a 'Children' table, allow other tables
          continue;
        }
        else {
          niError(niFmt(_A("Invalid child data table '%s'."),pChildDT->GetName()));
          return eFalse;
        }
      }

      if (ptrRegex.IsOK() && !ptrRegex->DoesMatch(strClass.Chars()))
        continue;

      tHStringPtr hspID = _H(pChildDT->GetString(_A("id")));
      if (HStringIsNotEmpty(hspID)) {
        ptrWidget = this->GetChildFromID(hspID);
        if (ptrWidget.IsOK() && niFlagIs(ptrWidget->GetStyle(),eWidgetStyle_DontSerialize))
          continue;
      }
      if (!ptrWidget.IsOK()) {
        ptrWidget = mpUICtx->CreateWidget(strClass.Chars(),this,sRectf(0,0),0,hspID);
      }
      if (!niIsOK(ptrWidget)) {
        niError(niFmt(_A("Can't create child widget '%s' (class:%s), from data table %d."),HStringGetStringEmpty(hspID),strClass.Chars(),i));
        return eFalse;
      }
      if (!mpUICtx->SerializeWidget(ptrWidget,pChildDT,(anFlags&(~eWidgetSerializeFlags_NoRoot)),ptrRegex)) {
        niError(niFmt(_A("Can't serialize read widget '%s' (class:%s), from data table %d."),HStringGetStringEmpty(hspID),strClass.Chars(),i));
        return eFalse;
      }
    }
  }
  else {
    if (!isChildrenTable) {
      niError(_A("Writting outside of a children data table."));
      return eFalse;
    }
    for (tU32 i = 0; i < this->GetNumChildren(); ++i) {
      iWidget* pWidget = this->GetChildFromIndex(i);
      if (ptrRegex.IsOK() && !ptrRegex->DoesMatch(niHStr(pWidget->GetClassName())))
        continue;
      if (niFlagIs(pWidget->GetStyle(),eWidgetStyle_DontSerialize))
        continue;
      Ptr<iDataTable> ptrNewDT = ni::CreateDataTable(AZEROSTR);
      mpUICtx->SerializeWidget(pWidget,ptrNewDT,(anFlags&(~eWidgetSerializeFlags_NoRoot)),ptrRegex);
      apDT->AddChild(ptrNewDT);
    }
  }

  // link the context menu afterward, as it might get created as a child widget
  if (niFlagIs(anFlags,eWidgetSerializeFlags_Read)) {
    tU32 nPropertyIndex = apDT->GetPropertyIndex(_A("context_menu_id"));
    if (nPropertyIndex != eInvalidHandle) {
      tHStringPtr hspMenuID = _H(apDT->GetStringFromIndex(nPropertyIndex));
      nPropertyIndex = apDT->GetPropertyIndex(_A("context_menu_parent_id"));
      if (nPropertyIndex != eInvalidHandle) {
        tHStringPtr hspMenuParentID = _H(apDT->GetStringFromIndex(nPropertyIndex));
        iWidget* pwMenuParent = mpUICtx->GetRootWidget()->FindWidget(hspMenuParentID);
        if (!pwMenuParent) {
          niWarning(niFmt(_A("Can't find context menu parent widget '%s'."),hspMenuParentID));
        }
        else {
          iWidget* pwMenu = pwMenuParent->GetChildFromID(hspMenuID);
          if (!pwMenuParent) {
            niWarning(niFmt(_A("Can't find context menu '%s' in it's parent '%s'."),hspMenuID,hspMenuParentID));
          }
          else {
            this->SetContextMenu(pwMenu);
          }
        }
      }
      else {
        iWidget* pwMenu = this->GetChildFromID(hspMenuID);
        if (!pwMenu) {
          niWarning(niFmt(_A("Can't find context menu '%s'."),hspMenuID));
        }
        else {
          this->SetContextMenu(pwMenu);
        }
      }
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
void cWidget::ChildChangedDockStyle(cWidget* apChild)
{
  tBool childDocked = (apChild)?(apChild->mDockStyle != eWidgetDockStyle_None):eFalse;
  if (!childDocked) {
    for (tU32 i = 0; i < GetNumChildren(); ++i) {
      iWidget* pChild = GetChildFromIndex(i);
      if (pChild->GetDockStyle() != eWidgetDockStyle_None) {
        childDocked = eTrue;
        break;
      }
    }
  }
  niFlagOnIf(mnAutoLayout,eWidgetAutoLayoutFlags_Dock,childDocked);
}

///////////////////////////////////////////////
void cWidget::ChildChangedRelative(cWidget* apChild)
{
  tBool childRel = (apChild)?niFlagIs(apChild->mStatus,WDGSTATUS_RELATIVE):eFalse;
  if (!childRel) {
    for (tU32 i = 0; i < GetNumChildren(); ++i) {
      cWidget* pChild = static_cast<cWidget*>(GetChildFromIndex(i));
      if (niFlagIs(pChild->mStatus,WDGSTATUS_RELATIVE)) {
        childRel = eTrue;
        break;
      }
    }
  }
  niFlagOnIf(mnAutoLayout,eWidgetAutoLayoutFlags_Relative,childRel);
}

///////////////////////////////////////////////
void cWidget::NotifyChangedDockStyle()
{
  QPtr<cWidget> ptrParent(mpwParent);
  if (ptrParent.IsOK()) {
    ptrParent->ChildChangedDockStyle(this);
    if (GetVisible()) {
      UpdateParentAutoLayout("Parent-ChangedDockStyle");
    }
  }
}

///////////////////////////////////////////////
void cWidget::NotifyChangedRelative()
{
  niFlagOnIf(mStatus,WDGSTATUS_RELATIVE,
             fabsf(mrectRelative.Left()) > niEpsilon5 || fabsf(mrectRelative.Right()) > niEpsilon5 ||
             fabsf(mrectRelative.Top()) > niEpsilon5 || fabsf(mrectRelative.Bottom()) > niEpsilon5);
  QPtr<cWidget> ptrParent(mpwParent);
  if (ptrParent.IsOK()) {
    ptrParent->ChildChangedRelative(this);
    if (GetVisible()) {
      UpdateParentAutoLayout("Parent-ChangedRelative");
    }
  }
}

///////////////////////////////////////////////
iWidgetZMap* cWidget::_GetOwnerZMap() const
{
  if (mnStyle & eWidgetStyle_Free) {
    return &mpUICtx->mFreeWidgets;
  }
  QPtr<cWidget> ptrParent(mpwParent);
  if (ptrParent.IsOK()) {
    return &ptrParent->mZMap;
  }
  return NULL;
}

///////////////////////////////////////////////
iFont*   __stdcall cWidget::FindSkinFont(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const {
  return mpUICtx->FindSkinFont(
      GetSkin(),ni::HStringIsEmpty(ahspSkinClass)?GetSkinClass():ahspSkinClass,ahspState,ahspName);
}

///////////////////////////////////////////////
iOverlay* __stdcall cWidget::FindSkinCursor(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const {
  return mpUICtx->FindSkinCursor(
      GetSkin(),ni::HStringIsEmpty(ahspSkinClass)?GetSkinClass():ahspSkinClass,ahspState,ahspName);
}

///////////////////////////////////////////////
iOverlay* __stdcall cWidget::FindSkinElement(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const {
  return mpUICtx->FindSkinElement(
      GetSkin(),ni::HStringIsEmpty(ahspSkinClass)?GetSkinClass():ahspSkinClass,ahspState,ahspName);
}

///////////////////////////////////////////////
sColor4f __stdcall cWidget::FindSkinColor(const sColor4f& aDefault, iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const {
  return mpUICtx->FindSkinColor(
      aDefault,GetSkin(),ni::HStringIsEmpty(ahspSkinClass)?GetSkinClass():ahspSkinClass,ahspState,ahspName);
}

///////////////////////////////////////////////
iWidget* __stdcall cWidget::CreateDefaultHoverWidget(ni::iHString* ahspHoverText) {
  CheckValid(NULL);

  Ptr<iWidget> w = mpUICtx->CreateWidget(
      _A("Button"),
      this,
      sRectf(0,0,100,80),
      eWidgetStyle_Free|eWidgetStyle_Temp|eWidgetButtonStyle_TextLeft,
      _HC(__ID_HoverWidget__));
  w->SetSkinClass(_H("HoverLabel"));
  w->SetIgnoreInput(eTrue);
  w->SetLocale(ni::GetLang()->GetDefaultLocale());
  w->SetText(ahspHoverText);

  iOverlay* frame = w->FindSkinElement(w->GetSkinClass(),NULL,_H("Frame"));
  iFont* font = w->GetFont();
  if (font) {
    const tF32 extraWidth = font->GetCharWidth(' ') * 2.0f;
    sRectf rect(0,0,0,0);
    rect = font->ComputeTextSize(rect,niHStr(w->GetLocalizedText()),0);
    if (frame) {
      const sVec4f& vFrame = frame->GetFrame();
      w->SetSize(
          rect.GetSize()+
          Vec2(
              vFrame.Left()+vFrame.Right()+extraWidth,
              vFrame.Top()+vFrame.Bottom()));
    }
    else {
      w->SetSize(
          rect.GetSize()+
          Vec2(font->GetMaxCharWidth()*3.0f,font->GetMaxCharHeight()));
    }
  }

  return w.GetRawAndSetNull();
}

///////////////////////////////////////////////
tBool __stdcall cWidget::ShowHoverWidget(ni::iWidget* apWidget, const sVec2f& avAbsPos) {
  CheckValid(eFalse);

  // first we remove the previous hover widget if there was one
  if (niFlagIs(mStatus,WDGSTATUS_HASHOVERWIDGET)) {
    Ptr<iWidget> w = GetChildFromID(_HC(__ID_HoverWidget__));
    if (w.IsOK() && w.ptr() != apWidget) {
      w->Invalidate();
    }
    niFlagOff(mStatus,WDGSTATUS_HASHOVERWIDGET);
  }

  if (!niIsOK(apWidget))
    return eFalse;

  apWidget->SetParent(this);
  if (apWidget->GetParent() != this)
    return eFalse;

  // make sure our widget has the HoverWidget id and style
  apWidget->SetID(_HC(__ID_HoverWidget__));
  apWidget->SetStyle(apWidget->GetStyle()|eWidgetStyle_Free|eWidgetStyle_Temp);

  niFlagOn(this->mStatus,WDGSTATUS_HASHOVERWIDGET);
  sVec2f vCurPos = avAbsPos;
  if (mpUICtx->GetCursor()) {
    vCurPos += mpUICtx->GetCursor()->GetSize();
  }
  apWidget->PopAt(NULL,vCurPos,0);
  apWidget->SetZOrder(eWidgetZOrder_Overlay);
  return eTrue;
}

///////////////////////////////////////////////
void __stdcall cWidget::ResetHoverWidget(tBool abRestart)
{
  CheckValid(;);
  if (niFlagIs(mStatus,WDGSTATUS_HASHOVERWIDGET)) {
    Ptr<iWidget> w = GetChildFromID(_HC(__ID_HoverWidget__));
    if (w.IsOK()) {
      w->Invalidate();
    }
    niFlagOff(mStatus,WDGSTATUS_HASHOVERWIDGET);

    if (abRestart && mpUICtx->_CanHover()) {
      SetTimer(eWidgetSystemTimer_Hover,mpUICtx->mfHoverDelay);
      SetTimer(eWidgetSystemTimer_NCHover,-1);
    }
    else {
      SetTimer(eWidgetSystemTimer_Hover,-1);
      SetTimer(eWidgetSystemTimer_NCHover,-1);
    }
  }
}

///////////////////////////////////////////////
ni::iWidget* __stdcall cWidget::GetHoverWidget() const {
  CheckValid(NULL);
  if (niFlagIs(mStatus,WDGSTATUS_HASHOVERWIDGET)) {
    return GetChildFromID(_HC(__ID_HoverWidget__));
  }
  return NULL;
}

///////////////////////////////////////////////
void __stdcall cWidget::Redraw() {
  CheckValid(;);
  mpUICtx->_RedrawWidget(this);
}

//////////////////////////////////////////////
void cWidget::_ComputeRectSize(const sRectf& aNewRect, tBool abForceSendMessage, const sVec2f& vPrevSize)
{
  sRectf newRect = aNewRect;
  sVec2f vSize = newRect.GetSize();
  VecMinimize(vSize,vSize,mvMaxSize);
  VecMaximize(vSize,vSize,mvMinSize);
  newRect.SetSize(vSize.x,vSize.y);
  _RoundRect(newRect);
  //     niDebugFmt((_A("[%s] COMPUTE RECT SIZE: %s => %s [SIZE:%s-m:%s-M:%s]\n"),
  //               niHStr(mhspID),
  //               _ASZ(mRect),
  //               _ASZ(newRect),
  //               _ASZ(vSize),
  //               _ASZ(mvMinSize),
  //               _ASZ(mvMaxSize)));
  if (newRect != mRect/* || abForceSendMessage*/) {
    // const sVec2f vPrevSize = mRect.GetSize();
    mRect = newRect;
#ifdef USE_CACHED_ABS_RECT
    _SetMoved();
#endif
    this->SendMessage(eUIMessage_NCSize,mRect.GetSize(),vPrevSize);
  }
}

#ifdef USE_CACHED_ABS_RECT
///////////////////////////////////////////////
void cWidget::_SetMoved()
{
  if (!niFlagIs(mStatus,WDGSTATUS_DIRTY_RECTABS) ||
      !niFlagIs(mStatus,WDGSTATUS_DIRTY_RECTABSCLIPPED) ||
      !niFlagIs(mStatus,WDGSTATUS_DIRTY_RECTABSCLIENTCLIPPED))
  {
    mStatus |= WDGSTATUS_DIRTY_RECT_ANY;
    niLoop(i,mvecClipChildren.size()) {
      mvecClipChildren[i]->_SetMoved();
    }
  }
}
#endif

///////////////////////////////////////////////
void cWidget::_UpdateMinMaxSize(sVec2f newMin, sVec2f newMax)
{
  // Clamp the min/max values in a valid range
  VecMaximize(newMin,newMin,_kvMinSize);
  VecMinimize(newMax,newMax,_kvMaxSize);

  // check that the max value is valid (at least bigger than the min size)
  if (newMax.x <= newMin.x) newMax.x = _kvMaxSize.x;
  if (newMax.y <= newMin.y) newMax.y = _kvMaxSize.y;

  // if min/max changed do the required updates
  if (newMin != mvMinSize || newMax != mvMaxSize) {
    mvMinSize = newMin;
    mvMaxSize = newMax;
    _ComputeRectSize(mRect,eFalse,mRect.GetSize());
  }
}

///////////////////////////////////////////////
void __stdcall cWidget::SetCanvas(iCanvas* apCanvas) {
  if (apCanvas != mptrCanvas) {
    this->Redraw();
  }
  mptrCanvas = apCanvas;
}
iCanvas* __stdcall cWidget::GetCanvas() const {
  return mptrCanvas;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::SnapInside(iWidget* apContainer, tF32 afSnapMargin) {
  CheckValid(eFalse);
  if (!apContainer && this->GetUIContext())
    apContainer = this->GetUIContext()->GetRootWidget();
  niCheckSilent(niIsOK(apContainer),eFalse);

  const sVec2f vSize = this->GetSize();
  sVec2f vAbsPos = this->GetAbsolutePosition();
  sRectf rectDesk = apContainer->GetAbsoluteClippedClientRect();

  // horizontal
  if (vAbsPos.x <= rectDesk.Left()+afSnapMargin) {
    vAbsPos.x = rectDesk.Left();
  }
  else if (vAbsPos.x+vSize.x >= rectDesk.Right()-afSnapMargin) {
    vAbsPos.x = rectDesk.Right()-vSize.x;
  }
  // vertical
  if (vAbsPos.y <= rectDesk.Top()+afSnapMargin) {
    vAbsPos.y = rectDesk.Top();
  }
  else if (vAbsPos.y+vSize.y >= rectDesk.Bottom()-afSnapMargin) {
    vAbsPos.y = rectDesk.Bottom()-vSize.y;
  }

  this->SetAbsolutePosition(vAbsPos);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidget::PopAt(iWidget* apContainer, const sVec2f& avAbsPos, tF32 afSnapMargin) {
  CheckValid(eFalse);
  if (!apContainer && this->GetUIContext())
    apContainer = this->GetUIContext()->GetRootWidget();
  niCheckSilent(niIsOK(apContainer),eFalse);
  sRectf rect = this->GetWidgetRect();
  sVec2f vSize = apContainer->GetSize();
  sVec2f vPos = avAbsPos;
  if (vPos.x+rect.GetWidth() >= vSize.x) {
    // move the box on the left of the cursor
    vPos.x = ni::Max(0.0f,avAbsPos.x - rect.GetWidth() - 2.0f);
  }
  if (vPos.y+rect.GetHeight() >= vSize.y) {
    // move the box on the top of the cursor
    vPos.y = ni::Max(0.0f,avAbsPos.y - rect.GetHeight() - 2.0f);
  }
  this->SetAbsolutePosition(avAbsPos);
  return SnapInside(apContainer,afSnapMargin);
}

///////////////////////////////////////////////
void cWidget::Layout(tBool abChildren)
{
  niAssert(niFlagIsNot(mStatus,WDGSTATUS_INVALID));
  niCheck(niFlagIsNot(mStatus,WDGSTATUS_INVALID),;);

  if (niFlagIs(mStatus,WDGSTATUS_RELAYOUT)) {
    niFlagOff(mStatus,WDGSTATUS_RELAYOUT);
    SendMessage(eUIMessage_Layout,mnAutoLayout,niVarNull);
  }

  if (abChildren) {
    niLoop(i,mvecFreeChildren.size()) {
      mvecFreeChildren[i]->Layout(abChildren);
    }

    niLoop(i,mvecClipChildren.size()) {
      mvecClipChildren[i]->Layout(abChildren);
    }
  }
}

///////////////////////////////////////////////
void __stdcall cWidget::Relayout(const achar* aaszReason) {
  if (mpUICtx && !niFlagIs(mStatus,WDGSTATUS_RELAYOUT)) {
    ++mpUICtx->mnRelayoutCount;
    if (PRINT_RELAYOUT) {
      niDebugFmt(("W/Relayout [%s]: %s (%d)",aaszReason,mhspID,mpUICtx->mnRelayoutCount));
    }
    niFlagOn(mStatus,WDGSTATUS_RELAYOUT);
  }
}

///////////////////////////////////////////////
tBool Callback_ExcludeWidget_IgnoreInputNotDragDest(const cWidget* w, tIntPtr apUserData) {
  return (w->GetIgnoreInput() && niFlagIsNot(w->GetStyle(),eWidgetStyle_DragDestination));
}
tBool Callback_ExcludeWidget_IgnoreInput(const cWidget* w, tIntPtr apUserData) {
  return w->GetIgnoreInput();
}
tBool Callback_ExcludeWidget_UserDataAndIgnoreInput(const cWidget* w, tIntPtr apUserData) {
  return ((tIntPtr)w == apUserData) || w->GetIgnoreInput();
}

///////////////////////////////////////////////
void __stdcall cWidget::SetLocale(iHString* ahspLocale) {
  tHStringPtr newLocale = HStringIsNotEmpty(ahspLocale) ? ahspLocale : NULL;
  if (newLocale != mhspLocale) {
    mhspLocale = newLocale;
    BroadcastMessage(eUIMessage_LocaleChanged,niVarNull,niVarNull);
  }
}
iHString* __stdcall cWidget::GetLocale() const {
  return mhspLocale;
}
iHString* __stdcall cWidget::GetActiveLocale() const {
  Ptr<cWidget> w = this;
  while (w.IsOK()) {
    if (HStringIsNotEmpty(w->mhspLocale))
      return w->mhspLocale;
    w = QPtr<cWidget>(w->mpwParent);
  }
  return ni::GetLang()->GetDefaultLocale();
}
iHString* __stdcall cWidget::FindLocalized(iHString* ahspText) const {
  if (HStringIsEmpty(ahspText))
    return ahspText;
  return ahspText->GetLocalizedEx(this->GetActiveLocale());
}
