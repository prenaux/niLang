#ifndef __WIDGET_35017188_H__
#define __WIDGET_35017188_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "WidgetZMap.h"
#include <niLang/STL/list.h>
#include <niLang/Utils/WeakPtr.h>

struct sCacheItem;

#define USE_CACHED_ABS_RECT

#define WDGSTATUS_VISIBLE            niBit(0)
#define WDGSTATUS_ENABLED            niBit(1)
#define WDGSTATUS_IGNOREINPUT        niBit(2)
#define WDGSTATUS_UPDATINGAUTOLAYOUT niBit(3)
#define WDGSTATUS_RELATIVE           niBit(4)
#define WDGSTATUS_RELAYOUT           niBit(5)
#define WDGSTATUS_SETTEXTLOCKED      niBit(6)
#define WDGSTATUS_DUMMY              niBit(7)
#define WDGSTATUS_HASHOVERWIDGET     niBit(8)
#define WDGSTATUS_REDRAW             niBit(9)
#define WDGSTATUS_DESTROYED          niBit(10) // received the destroy message...
#define WDGSTATUS_EXCLUSIVE          niBit(11)
#define WDGSTATUS_CAPTURE_ALL        niBit(12)
#define WDGSTATUS_HIDECHILDREN       niBit(13)
#define WDGSTATUS_INVALID            niBit(14)

#ifdef USE_CACHED_ABS_RECT
#define WDGSTATUS_DIRTY_RECTABS              niBit(15)
#define WDGSTATUS_DIRTY_RECTABSCLIPPED       niBit(16)
#define WDGSTATUS_DIRTY_RECTABSCLIENTCLIPPED niBit(17)
#define WDGSTATUS_DIRTY_RECT_ANY             (niBit(15)|niBit(16)|niBit(17))
#endif

#define WDGSTATUS_FINGER_CAPTURE(FINGER) niBit(21+FINGER)

#define WDGCLICK_LEFTDOWN   niBit(0)
#define WDGCLICK_RIGHTDOWN  niBit(1)

// <z, widget>
// the map is to be sure to have all widget sorted by z
typedef astl::vector<Ptr<cWidget> > tCWidgetVec;
typedef astl::list<Ptr<cWidget> > tWidgetPtrLst;
typedef tWidgetPtrLst::iterator tWidgetPtrLstIt;
typedef tWidgetPtrLst::const_iterator tWidgetPtrLstCIt;
typedef tWidgetPtrLst::reverse_iterator tWidgetPtrLstRIt;
typedef tWidgetPtrLst::const_reverse_iterator tWidgetPtrLstCRIt;
typedef ni::SinkList<iWidgetSink> tWidgetSinkLst;

class cWidgetClass;
class cUIContext;

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidget declaration.
class cWidget : public ImplRC<iWidget,ni::eImplFlags_DontInherit1,iMessageHandler>
{
  niBeginClass(cWidget);

 public:
  inline static const ni::tUUID& __stdcall _GetInterfaceUUID() {
    return niGetInterfaceUUID(BaseType);
  }

  //! Constructor.
  cWidget(cUIContext* pUICtx, iHString* ahspClass, cWidget *pwParent, const sRectf &arectPos, tU32 anStyle, iHString* ahspID, iHString* ahspSkinClass, iHString* ahspText);
  //! Destructor.
  ~cWidget();

  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  void __stdcall Invalidate();

  //// iUnknown /////////////////////////////////
  iUnknown* __stdcall QueryInterface(const tUUID& aIID);
  void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const;
  //// iUnknown /////////////////////////////////

  //// ni::iWidget /////////////////////////////
  void __stdcall Destroy();
  ni::iGraphics* __stdcall GetGraphics() const niImpl;
  ni::iGraphicsContext* __stdcall GetGraphicsContext() const niImpl;
  ni::iUIContext * __stdcall GetUIContext() const;
  iHString* __stdcall GetClassName() const;
  void __stdcall SetID(iHString* ahspID);
  iHString* __stdcall GetID() const;
  tBool __stdcall SetStyle(tU32 anStyle);
  tU32 __stdcall GetStyle() const;
  tBool __stdcall _CheckSelfParent(ni::iWidget *apParent) const;
  tBool __stdcall DoSetParent(ni::iWidget *apParent);
  void __stdcall SetParent(ni::iWidget *apParent);
  ni::iWidget * __stdcall GetParent() const;
  void __stdcall SetZOrder(eWidgetZOrder aZOrder);
  eWidgetZOrder __stdcall GetZOrder() const;
  void __stdcall SetZOrderAbove(iWidget* apWidget);
  tU32 __stdcall GetDrawOrder() const;
  void __stdcall SetPosition(const sVec2f &avPos);
  sVec2f __stdcall GetPosition() const;
  void __stdcall SetSize(const sVec2f &avSize);
  sVec2f __stdcall GetSize() const;
  void __stdcall SetMinimumSize(sVec2f avMinSize);
  sVec2f __stdcall GetMinimumSize() const;
  void __stdcall SetMaximumSize(sVec2f avMaxSize);
  sVec2f __stdcall GetMaximumSize() const;
  void __stdcall SetRect(const sRectf &aRect);
  sRectf __stdcall GetWidgetRect() const;
  sRectf __stdcall GetDockFillRect() const;
  sRectf __stdcall GetDockFillRectNC() const;
  sRectf __stdcall GetRect() const;
  void __stdcall SetClientPosition(const sVec2f &avPos);
  sVec2f __stdcall GetClientPosition() const;
  void __stdcall SetClientSize(const sVec2f &avSize);
  sVec2f __stdcall GetClientSize() const;
  void __stdcall SetClientRect(const sRectf &aRect);
  sRectf __stdcall GetClientRect() const;
  sRectf __stdcall ComputeFitRect(const sRectf& aRect) const;
  void __stdcall SetFitRect(const sRectf& aRect);
  void __stdcall SetFitSize(const sVec2f avSize);
  void __stdcall SetAbsolutePosition(const sVec2f &avPos);
  sVec2f __stdcall GetAbsolutePosition() const;
  void __stdcall SetAbsoluteRect(const sRectf& aRect);
  sRectf __stdcall GetAbsoluteRect() const;
  sRectf __stdcall GetClippedRect() const;
  sRectf __stdcall GetAbsoluteClippedRect() const;
  sRectf __stdcall GetClippedClientRect() const;
  sRectf __stdcall GetAbsoluteClippedClientRect() const;
  void __stdcall SetRelativePosition(const sVec2f& avPos);
  sVec2f __stdcall GetRelativePosition() const;
  void __stdcall SetRelativeSize(const sVec2f& avSize);
  sVec2f __stdcall GetRelativeSize() const;
  void __stdcall SetRelativeRect(const sRectf& aRect);
  sRectf __stdcall GetRelativeRect() const;
  void __stdcall SetPadding(const sVec4f& aRect);
  sVec4f __stdcall GetPadding() const;
  tBool __stdcall GetHasPadding() const;
  void __stdcall SetMargin(const sVec4f& aRect);
  sVec4f __stdcall GetMargin() const;
  tBool __stdcall GetHasMargin() const;
  tBool __stdcall SetFocus();
  tBool __stdcall MoveFocus(tBool abToPrevious);
  tBool __stdcall GetHasFocus() const;
  tBool __stdcall GetDraggingSource() const;
  tBool __stdcall GetDragging() const;
  tBool __stdcall GetDraggingFingerSource(tU32 anFinger) const;
  tBool __stdcall GetDraggingFinger(tU32 anFinger) const;
  void __stdcall Redraw();
  void __stdcall SetCapture(tBool abEnable);
  tBool __stdcall GetCapture() const;
  void __stdcall SetExclusive(tBool abEnable);
  tBool __stdcall GetExclusive() const;
  void __stdcall SetInputSubmitFlags(tU32 aSubmitFlags);
  tU32 __stdcall GetInputSubmitFlags() const;
  void __stdcall SetVisible(tBool abVisible);
  tBool __stdcall GetVisible() const;
  void __stdcall SetEnabled(tBool abEnabled);
  tBool __stdcall GetEnabled() const;
  void __stdcall SetIgnoreInput(tBool abIgnoreInput);
  tBool __stdcall GetIgnoreInput() const;
  void __stdcall SetHideChildren(tBool abHideChildren);
  tBool __stdcall GetHideChildren() const;
  void __stdcall SetStatus(tBool abVisible, tBool abEnabled, tBool abIgnoreInput);
  tBool __stdcall GetIsMouseOver() const;
  tBool __stdcall GetIsPressed() const;
  tBool __stdcall GetIsNcMouseOver() const;
  tBool __stdcall GetIsNcPressed() const;
  tBool __stdcall GetIsFingerOver(tU32 anFinger) const;
  tBool __stdcall GetIsFingerPressed(tU32 anFinger) const;
  tBool __stdcall GetIsNcFingerOver(tU32 anFinger) const;
  tBool __stdcall GetIsNcFingerPressed(tU32 anFinger) const;
  void __stdcall SetTimer(tU32 anID, tF32 afTime);
  tF32 __stdcall GetTimer(tU32 anID) const;
  tBool __stdcall AddSink(ni::iWidgetSink *apSink);
  tBool __stdcall AddPostSink(iWidgetSink* apSink);
  iWidgetSink* __stdcall AddClassSink(const achar* aaszClassName);
  iWidgetSink* __stdcall AddClassPostSink(const achar* aaszClassName);
  void __stdcall RemoveSink(ni::iWidgetSink *apSink);
  void __stdcall _SendDestroyMessage();
  void __stdcall InvalidateChildren();
  tU32 __stdcall GetNumChildren() const;
  tU32 __stdcall GetChildIndex(iWidget* apWidget) const;
  ni::iWidget * __stdcall GetChildFromIndex(tU32 anIndex) const;
  ni::iWidget * __stdcall GetChildFromID(iHString* ahspID) const;
  tU32 __stdcall GetNumChildrenDrawn() const;
  ni::iWidget* __stdcall GetChildFromDrawOrder(tU32 anDrawOrder) const;
  iWidget* __stdcall FindWidget(iHString* ahspID) const;
  iWidget* __stdcall FindWidgetByPos(const sVec2f& avPos) const;
  void __stdcall SetSkin(iHString* ahspSkin);
  ni::iHString* __stdcall GetSkin() const;
  void __stdcall SetSkinClass(iHString* ahspSkinClass);
  iHString* __stdcall GetSkinClass() const;
  iFont*   __stdcall FindSkinFont(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const;
  iOverlay* __stdcall FindSkinCursor(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const;
  iOverlay* __stdcall FindSkinElement(iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const;
  sColor4f __stdcall FindSkinColor(const sColor4f& aDefault, iHString* ahspSkinClass, iHString* ahspState, iHString* ahspName) const;

  void __stdcall SetText(iHString* ahspText);
  void __stdcall RetrieveText(tBool abSerialize);
  iHString* __stdcall GetText() const;
  iHString* __stdcall GetLocalizedText() const;
  void __stdcall SetHoverText(iHString* ahspText);
  iHString* __stdcall GetHoverText() const;
  tBool __stdcall SetFont(iFont *apFont);
  iFont * __stdcall GetFont() const;
  tBool __stdcall SetDockStyle(eWidgetDockStyle aStyle);
  eWidgetDockStyle __stdcall GetDockStyle() const;
  tBool __stdcall ApplyDockStyle(eWidgetDockStyle aStyle);
  tBool __stdcall Place(const sRectf& aRect, eWidgetDockStyle aStyle, const sVec4f& avMargin);
  tBool __stdcall SetContextMenu(iWidget* apMenu);
  iWidget* __stdcall GetContextMenu() const;
  tBool __stdcall SendMessage(tU32 aMsg, const Var& avarA, const Var& avarB);
  tBool __stdcall SendCommand(iWidget* apDest, tU32 anCommand, const Var& avarExtra1, const Var& avarExtra2);
  tBool __stdcall BroadcastMessage(tU32 aMsg, const Var& avarA, const Var& avarB);

  void __stdcall SetCanvas(iCanvas* apCanvas);
  iCanvas* __stdcall GetCanvas() const;

  tBool __stdcall SerializeLayout(iDataTable* apDT, tWidgetSerializeFlags anFlags);
  tBool __stdcall SerializeChildren(iDataTable* apDT, tWidgetSerializeFlags anFlags);
  void __stdcall SetAutoLayout(tWidgetAutoLayoutFlags aFlags);
  tWidgetAutoLayoutFlags __stdcall GetAutoLayout() const;

  void __stdcall ComputeAutoLayout(tWidgetAutoLayoutFlags aFlags);

  ni::iWidget* __stdcall CreateDefaultHoverWidget(ni::iHString* ahspHoverText);
  tBool __stdcall ShowHoverWidget(ni::iWidget* apWidget, const sVec2f& avAbsPos);
  void __stdcall ResetHoverWidget(tBool abRestart);
  ni::iWidget* __stdcall GetHoverWidget() const;

  tBool __stdcall SnapInside(iWidget* apContainer, tF32 afSnapMargin);
  tBool __stdcall PopAt(iWidget* apContainer, const sVec2f& avAbsPos, tF32 afSnapMargin);

  void __stdcall Layout(tBool abChildren);
  void __stdcall Relayout(const achar* aaszReason);

  void __stdcall SetLocale(iHString* ahspLocale);
  iHString* __stdcall GetLocale() const;
  iHString* __stdcall GetActiveLocale() const;
  iHString* __stdcall FindLocalized(iHString* ahspText) const;

  void __stdcall SetFingerCapture(tU32 anFinger, tBool abEnable);
  tBool __stdcall GetFingerCapture(tU32 anFinger) const;
  //// ni::iWidget /////////////////////////////

  tBool MapToClientRect(const sVec2f &pos,sVec2f &clientPos) const;
  void AddChild(cWidget *pChild);
  void RemoveChild(cWidget *pChild);


  cWidget* GetMessageTargetByPos(const sVec2f &pos,
                                 tBool (*apfnExcludeWidget)(const cWidget* w, tIntPtr apUserData),
                                 tIntPtr apUserData) const;

  tBool __stdcall HasChild(const iWidget* apW, tBool abRecursive) const;
  tBool __stdcall HasParent(const iWidget* apW) const;
  void UpdateParentAutoLayout(const achar* aaszReason);
  tU32 GetChildDrawOrder(const iWidget* apW) const;
  void _ComputeRectSize(const sRectf& newRect, tBool abForceSendMessage, const sVec2f& avNewSize);
  void ClearSinks();
  void ApplyRelativeRect();
  void ChildChangedDockStyle(cWidget* apChild);
  void NotifyChangedDockStyle();
  void ChildChangedRelative(cWidget* apChild);
  void NotifyChangedRelative();

  iWidgetZMap* _GetOwnerZMap() const;

  void _UpdateMinMaxSize(sVec2f newMin, sVec2f newMax);
  void _CheckCaptureAndExclusive();

  tU64 __stdcall GetThreadID() const { return mnThreadID; }
  void __stdcall HandleMessage(const tU32 anMsg, const Var& avarA, const Var& avarB);

 public:
  const tU64 mnThreadID;

  tHStringPtr mhspID;
  tHStringPtr mhspText;
  tHStringPtr mhspClass;
  tHStringPtr mhspSkin;
  tHStringPtr mhspSkinClass;

  cUIContext* mpUICtx;
  Nonnull<tWidgetSinkLst> mlstSinks;
  sRectf mClientRect;
  sRectf mRect;
  sRectf mrectRelative;
  WeakPtr<cWidget> mpwParent;
  tCWidgetVec mvecClipChildren;
  tCWidgetVec mvecFreeChildren;
  cWidgetZMap mZMap;
  tU32 mnStyle;
  sColor4f mcolBackground;
  tU32 mStatus;
  tU16 mClick;
  Ptr<iFont> mptrFont;
  eWidgetDockStyle mDockStyle;
  eWidgetZOrder mZOrder;
  sVec2f mvMinSize,mvMaxSize;
  sRectf mrectDockFillClient;
  sRectf mrectDockFillNonClient;
  Ptr<iWidget> mpwContextMenu;
  sVec4f mvMargin;
  sVec4f mvPadding;
  tWidgetAutoLayoutFlags mnAutoLayout;
  tU32 mnInputSubmitFlags;
  tHStringPtr mhspHoverText;

  struct sContextMenu {
    sRectf     mRect;
    Ptr<iWidget>  mpwMenu;
    QPtr<iWidgetMenu> mptrMenu;
    tBool       mbNonClient;
    tBool       mbPushed;
    sContextMenu(sRectf aRect, iWidget* apWidget, tBool abNonClient) {
      mRect = aRect;
      mpwMenu = apWidget;
      mptrMenu = apWidget;
      mbNonClient = abNonClient;
      mbPushed = eFalse;
    }
    tBool IsOK() const {
      return niIsOK(mptrMenu);
    }
    tBool Down(sVec2f avPos) {
      mbPushed = mRect.Intersect(avPos);
      return mbPushed;
    }
    tBool Up(sVec2f avPos) {
      if (!mbPushed) {
        return eFalse;
      }
      mbPushed = eFalse;
      return mRect.Intersect(avPos);
    }
  };
  astl::vector<sContextMenu> mvContextMenus;

  Ptr<iCanvas>  mptrCanvas;

#ifdef USE_CACHED_ABS_RECT
  mutable sRectf mrectAbs;
  mutable sRectf mrectAbsClipped;
  mutable sRectf mrectAbsClientClipped;
  void _SetMoved();
#endif

  tHStringPtr mhspLocale;
  tHStringPtr mhspLocalizedText;

  niEndClass(cWidget);
};

tBool Callback_ExcludeWidget_IgnoreInputNotDragDest(const cWidget* w, tIntPtr apUserData);
tBool Callback_ExcludeWidget_IgnoreInput(const cWidget* w, tIntPtr apUserData);
tBool Callback_ExcludeWidget_UserDataAndIgnoreInput(const cWidget* w, tIntPtr apUserData);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __WIDGET_35017188_H__
