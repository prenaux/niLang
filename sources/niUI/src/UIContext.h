#ifndef __UICONTEXT_15354543_H__
#define __UICONTEXT_15354543_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Graphics.h"
#include "Widget.h"
#include "WidgetZMap.h"
#include <niLang/Utils/TimerManager.h>
#include <niLang/Utils/WeakPtr.h>
#include <niLang_ModuleDef.h>
#include "API/niUI_ModuleDef.h"

// #define _UIInputTrace(X)         { niPrint("... UIInputTrace: "); niPrintln(X); }
// #define _UIInputKeyboardTrace(X) { niPrint("... UIInputKeyboardTrace: "); niPrintln(X); }
// #define _UIInputMMTrace(X)       { niPrint("... UIInputMMTrace: "); niPrintln(X); }
// #define _UIFocusTrace(X)         { niPrint("... UIFocusTrace: "); niPrintln(X); }
// #define _UISkinTrace(X)          { niPrint("... UISkinTrace: "); niPrintln(X); }
// #define _UIDragTrace(X)          { niPrint("... UIDragTrace: "); niPrintln(X); }

// This used to be the default but Cursor capture is only implemented on
// Windows and use feedback has been mostly that its confusing them. I dont
// think the UI should do this by default, a particular widget can choose to
// do so internally if it needs to.
// #define UICONTEXT_USE_CURSOR_CAPTURE

#ifndef _UIInputTrace
#define _UIInputTrace(X)
#endif
#ifndef _UIInputKeyboardTrace
#define _UIInputKeyboardTrace(X)
#endif
#ifndef _UIInputMMTrace
#define _UIInputMMTrace(X)
#endif
#ifndef _UIDragTrace
#define _UIDragTrace(X)
#endif
#ifndef _UIFocusTrace
#define _UIFocusTrace(X)
#endif
#ifndef _UISkinTrace
#define _UISkinTrace(X)
#endif

#define PRINT_RELAYOUT 0

#define FINGER_POINTER_ID_MIN 0x10000
#define FINGER_POINTER_ID_MAX 0x10003
#define FINGER_POINTER_ID_NUM (FINGER_POINTER_ID_MAX-FINGER_POINTER_ID_MIN)
#define FINGER_ID_POINTER_LEFT   (FINGER_POINTER_ID_MIN+ePointerButton_Left)
#define FINGER_ID_POINTER_RIGHT  (FINGER_POINTER_ID_MIN+ePointerButton_Right)
#define FINGER_ID_POINTER_MIDDLE (FINGER_POINTER_ID_MIN+ePointerButton_Middle)

static const tU32 knNumFingersBase = 10;
static const tU32 knNumFingers = knNumFingersBase + FINGER_POINTER_ID_NUM;

#define FINGER_INDEX(ID)                                    \
  (((ID) < 10) ? (ID) :                                     \
   (FINGER_IS_POINTER(ID) ? ((ID)-FINGER_POINTER_ID_MIN) :  \
    ni::eInvalidHandle))
#define CHECK_FINGER(ID,RET) if (FINGER_INDEX(ID) >= knNumFingers) { return RET; }
#define GET_FINGER(ID) mFingers[FINGER_INDEX(ID)]
#define FINGER_IS_POINTER(ID) (((ID) >= FINGER_POINTER_ID_MIN) && ((ID) < FINGER_POINTER_ID_MAX))
#define FINGER_ID_TO_POINTER_BUTTON(ID) ((ePointerButton)((ID)-FINGER_POINTER_ID_MIN))
#define FINGER_POINTER_BUTTON_TO_ID(ID) ((ID)+FINGER_POINTER_ID_MIN)

enum eUIStandardCursor {
  eUIStandardCursor_Current = 0,
  eUIStandardCursor_Invalid = 1,
  eUIStandardCursor_Arrow = 2,
  eUIStandardCursor_DraggingMove = 3,
  eUIStandardCursor_DraggingCopy = 4,
  eUIStandardCursor_DraggingInvalid = 5,
  //! \internal
  eUIStandardCursor_Last niMaybeUnused = 6
};

class cWidget;
class cUIContext;
class cWidgetClass;

typedef astl::vector<WeakPtr<cWidget> >  tWeakWidgetList;
typedef astl::list<Ptr<cWidget> > tWidgetLst;

const sVec2f _kvMinSize = {2,2};
const sVec2f _kvMaxSize = {100000,100000};

const tU32 knDrawOpCaptureToggleKey = eKeyMod_Alt|eKey_Tilde;
const tU32 knTerminalToggleKey = eKeyMod_Control|eKey_Tilde;

class cUIContext : public ImplRC<iUIContext,eImplFlags_Default>, public TimerManager
{
  niBeginClass(cUIContext);

 public:
  //! Constructor.
  cUIContext(iGraphicsContext* apContext, iHString* ahspDefaultSkinPath, tF32 afContentsScale);
  //! Destructor.
  ~cUIContext();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  void __stdcall Invalidate();

  //// iUIContext ///////////////////////////////

  iGraphics* __stdcall GetGraphics() const { return mptrGraphics; }
  iGraphicsContext* __stdcall GetGraphicsContext() const;

  void __stdcall SendWindowMessage(eOSWindowMessage aMsg, const Var& avarA, const Var& avarB);
  void __stdcall SetInputModifiers(tU32 anInputModifier);
  tU32 __stdcall GetInputModifiers() const;
  void __stdcall SetDefaultInputSubmitFlags(tUIInputSubmitFlags aSubmitFlags);
  tUIInputSubmitFlags __stdcall GetDefaultInputSubmitFlags() const;

  void __stdcall SetImageMap(iImageMap*) niImpl;
  iImageMap* __stdcall GetImageMap() const niImpl;

  tBool __stdcall SetErrorOverlay(iOverlay* apOverlay) niImpl;
  iOverlay* __stdcall GetErrorOverlay() const niImpl;
  void __stdcall ClearSkins() niImpl;
  tBool __stdcall AddSkin(iDataTable* apDT) niImpl;
  tBool __stdcall AddSkinFromRes(iHString* ahspRes) niImpl;
  tBool __stdcall RemoveSkin(iHString* ahspSkin) niImpl;
  tBool __stdcall SetDefaultSkin(iHString* ahspName) niImpl;
  iHString* __stdcall GetDefaultSkin() const niImpl;
  tBool __stdcall ApplySkin(iWidget* apWidget, iHString* ahspName) niImpl;
  tU32 __stdcall GetNumSkins() const niImpl;
  iHString* __stdcall GetSkinName(tU32 anIndex) const niImpl;
  tU32 __stdcall GetSkinIndex(iHString* ahspName) const niImpl;
  iDataTable* __stdcall GetSkinDataTable(iHString* ahspSkin) const niImpl;
  iFont*   __stdcall FindSkinFont(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const niImpl;
  iOverlay* __stdcall FindSkinCursor(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const niImpl;
  iOverlay* __stdcall FindSkinElement(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const niImpl;
  sColor4f __stdcall FindSkinColor(const sColor4f& aDefault, iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const niImpl;

  tBool __stdcall HasWidgetSinkClass(const achar *aszClassName) const niImpl;
  iWidgetSink* __stdcall CreateWidgetSink(const achar *aszClassName, iWidget* apWidget) niImpl;
#if niMinFeatures(15)
  iWidgetSink* __stdcall CreateWidgetSinkFromScript(iHString* ahspRes) niImpl;
#endif

  iWidget* __stdcall CreateWidget(const achar *aszClassName, iWidget *apwParent,const sRectf& arectPos, tU32 anStyle = 0, iHString* ahspID = NULL);
  iWidget* __stdcall CreateWidget2(const achar *aszClassName, iWidget *apwParent,const sRectf& arectPos, tU32 anStyle, iHString* ahspID, iHString* ahspSkinClass, iHString* ahspText);
  iWidget* __stdcall CreateWidgetRaw(const achar *aszClassName, iWidget *apwParent,const sRectf& arectPos, tU32 anStyle = 0, iHString* ahspID = NULL);
  iWidget* __stdcall CreateWidgetRaw2(const achar *aszClassName, iWidget *apwParent,const sRectf& arectPos, tU32 anStyle, iHString* ahspID, iHString* ahspSkinClass, iHString* ahspText);
  iWidget* __stdcall CreateWidgetFromDataTable(iDataTable* apDT, iWidget* apwParent, iHString* ahspID, iHString* ahspTitle);
  iWidget* __stdcall CreateWidgetFromResource(iHString* ahspRes, iWidget* apwParent, iHString* ahspID, iHString* ahspTitle);

  void __stdcall Draw();
  void __stdcall DrawCursor(iOSWindow* apWindow);
  tBool __stdcall Update(tF32 fTime);
  tBool __stdcall Resize(const sRectf& aRootRect, const tF32 afContentsScale);
  tF32 __stdcall GetContentsScale() const;
  iWidget* __stdcall GetRootWidget() const;

  tBool __stdcall SetCursor(iOverlay* pCursor);
  iOverlay* __stdcall GetCursor() const;
  sVec2f __stdcall GetCursorPosition() const;

  tBool __stdcall SetActiveWidget(iWidget* apWidget);
  iWidget* __stdcall GetActiveWidget() const;
  tU32 __stdcall GetNumWidgets() const;
  iWidget* __stdcall GetWidget(tU32 anIndex) const;
  tBool __stdcall SerializeWidget(iWidget* apWidget, iDataTable* apDT, tWidgetSerializeFlags anFlags, iRegex* apFilter);
  iWidgetCommand* __stdcall CreateWidgetCommand();
  tBool __stdcall SendCommand(iWidget* apDest, iWidgetCommand* apCmd);

  void __stdcall SetDebugDraw(tBool abDebug);
  tBool __stdcall GetDebugDraw() const;

#if niMinFeatures(20)
  void __stdcall InitializeDefaultToolbar();
  ni::tBool __stdcall SetToolbar(iWidget* apToolbar);
  ni::iWidget* __stdcall GetToolbar() const;
#endif

  void __stdcall SetDrawOpCapture(tBool abEnabled);
  ni::tBool __stdcall GetDrawOpCapture() const;

  void __stdcall SetShowTerminal(tBool abEnabled);
  ni::tBool __stdcall GetShowTerminal() const;

  void __stdcall SetDragStartDistance(tU32 anPixelDistance);
  tU32 __stdcall GetDragStartDistance() const;

  sVec3f __stdcall GetFingerPosition(tU32 anFinger) const;
  tBool __stdcall GetFingerDown(tU32 anFinger) const;
  void __stdcall SetDragFingerStartDistance(tU32 anFinger, tU32 anPixelDistance);
  tU32 __stdcall GetDragFingerStartDistance(tU32 anFinger) const;

  void __stdcall ClearShortcuts();
  void __stdcall AddShortcut(tU32 anKey, iHString* ahspCmd);
  void __stdcall RemoveShortcut(tU32 anKey);
  iHString* __stdcall GetShortcutCommand(tU32 anKey) const;

  tBool __stdcall GetKeyDown(tU8 aKey) const;
  void __stdcall InputFingerMove(tU32 anFinger, const sVec3f& avPosition);
  void __stdcall InputFingerRelativeMove(tU32 anFinger, const sVec3f& avPosition);
  void __stdcall InputFingerPress(tU32 anFinger, const sVec3f& avPosition, tBool abDown);
  void __stdcall InputKeyPress(eKey aKey, tBool abDown);
  void __stdcall InputKeyChar(tU32 aCharCodePoint, eKey aKeyLeadingToKeyChar);
  void __stdcall InputMouseWheel(const tF32 afDelta);
  void __stdcall InputDoubleClick(ePointerButton aPointer);
  void __stdcall InputRelativeMouseMove(const sVec2f& avRelMove);
  void __stdcall InputGameCtrl(iGameCtrl* apGameController);
  void __stdcall InputPinch(const tF32 afScale, const eGestureState aState);

  iProfDraw* __stdcall CreateProfDraw(iCanvas* apCanvas, iFont* apFont) const;
  //// iUIContext ///////////////////////////////

  tBool __stdcall DoSerializeLayout(iWidget* apWidget, iDataTable* apDT, tWidgetSerializeFlags anFlags, iRegex* apRegex);
  // public for the widget internals;
  const sVec2f& GetMousePos() const { return GetPrimaryFinger().GetPosition(); }

  // utils
  cWidget *GetMessageTargetByPos(const sVec2f &pos);
  cWidget *GetDragDestinationByPos(const sVec2f &pos); // returns NULL if the target isnt a drag destination
  cWidget *GetMouseMessageTarget(tBool abUpdate);
  cWidget *GetFingerMessageTarget(tU32 anFinger, tBool abUpdate);
  const WeakPtr<cWidget>& GetInputMessageTarget() const;
  // returns true if the active widget has changed
  tBool SetFocusInput(cWidget *pNewTarget, tBool abMouseClick);
  iWidget* __stdcall GetFocusedWidget() const;
  void InvalidateTarget(cWidget *pWidget);
  void MoveOnTop(cWidget* pNewTop);
  void _AddExclusive(cWidget *widget);
  void _RemoveExclusive(cWidget* widget, tBool abLost);
  tBool _IsExclusive(const cWidget *widget) const;
  tBool _IsFocusInput(const cWidget *widget) const {
    QPtr<cWidget> focusInput(mpwFocusInput);
    return widget == focusInput;
  }
  void _WindowUpdateCursorStates();

  void _AddCaptureAll(cWidget *widget);
  void _RemoveCaptureAll(cWidget *widget, tBool abLost);
  tBool _IsCaptureAll(const cWidget *widget) const;

  void _AddFingerCapture(tU32 anFinger, cWidget *widget);
  void _RemoveFingerCapture(tU32 anFinger, cWidget *widget, tBool abLost);
  tBool _IsFingerCapture(tU32 anFinger, const cWidget *widget) const;

  void RegisterWidget(cWidget* apWdg);
  void UnregisterWidget(cWidget* apWdg);

  tBool MoveFocus(cWidget* apWidget, tBool abPrev);

  void  _FingerMove(tU32 anFinger, sVec3f avNewPos, tBool abOnlyIfChanged);

  void SetTopWidget(cWidget* apWidget);
  void UpdateFreeWidgets();

  tBool BeginFingerDrag(tU32 anFinger, cWidget* apWidget, const sVec3f& avStartPos, const sVec3f& avMousePos);
  void  EndFingerDrag(tU32 anFinger, const sVec3f& avAbsPos, tBool abCancel);
  tBool IsFingerDragging(tU32 anFinger) const;
  tBool IsFingerDrag(tU32 anFinger, const iWidget* apWidget) const;

  void __stdcall ActivateStandardCursor(eUIStandardCursor aCursor) {
    SetCursor(GetStandardCursor(aCursor));
  }

  iOverlay* __stdcall GetStandardCursor(eUIStandardCursor aCursor) {
    if (aCursor >= eUIStandardCursor_Last) return mptrMouseCursor[eUIStandardCursor_Invalid];
    return mptrMouseCursor[aCursor];
  }

  tBool _RedrawWidget(cWidget* apWidget);
  tBool _CanSendFingerDragMessage(tU32 anFinger);
  tBool _CanSendFingerDropMessage(tU32 anFinger);

  const achar* __stdcall GetTimersName() const;
  void __stdcall TimerTriggered(iMessageHandler* apHandler, tU32 anId, tF32 afDuration);

 public:
  Ptr<iGraphics> mptrGraphics;
  Ptr<iGraphicsContext> mptrGraphicsContext;
  Ptr<cWidget> mpwRootWidget;
  tWeakWidgetList mlstExclusiveStack;
  WeakPtr<cWidget> mpwTopWidget;
  WeakPtr<cWidget> mpwFocusInput;
  WeakPtr<cWidget> mpwActive;

  // Widgets
  tWeakWidgetList mlstWidgets;
  struct sFreeWidgets : public ImplLocal<iWidgetZMap> {
    cWidgetZMap zmapInput;
    cWidgetZMap zmapDraw;
    sFreeWidgets() {
    }
    void Clear() {
      zmapInput.Clear();
      zmapDraw.Clear();
    }
    tBool __stdcall RemoveOfZMap(cWidget* apW) {
      tBool bRes;
      bRes = zmapInput.RemoveOfZMap(apW) ? eTrue : eFalse;
      bRes = bRes || (zmapDraw.RemoveOfZMap(apW) ? eTrue : eFalse);
      return bRes;
    }
    void __stdcall SetZOrder(cWidget* apW, eWidgetZOrder aZOrder) {
      if (zmapInput.HasWidget(apW))
        zmapInput.SetZOrder(apW,aZOrder);
      if (zmapDraw.HasWidget(apW))
        zmapDraw.SetZOrder(apW,aZOrder);
    }
    void __stdcall SetZOrderAbove(cWidget* apW, cWidget* apAbove) {
      if (zmapInput.HasWidget(apW))
        zmapInput.SetZOrderAbove(apW,apAbove);
      if (zmapDraw.HasWidget(apW))
        zmapDraw.SetZOrderAbove(apW,apAbove);
    }
  } mFreeWidgets;

  // Mouse cursors
  Ptr<iOverlay> mptrMouseCursor[eUIStandardCursor_Last];

  // Debug draw
  tBool mbDebugDraw;

#if niMinFeatures(20)
  // Toolbar
  Ptr<iWidget> mptrToolbar;
#endif

  // Capture
  tBool mbDrawOpCapture;
  tBool mbDrawOpCaptureDrawn;
  Ptr<iGraphicsDrawOpCapture> mptrDrawOpCapture;

  // Show terminal
  tBool mbShowTerminal = eFalse;

  // Drawing
  tBool __stdcall _DrawWidget(cWidget* apWidget, iCanvas* apCanvas, const sRectf& aParentRect,
                              tU32 anRawViewport, const sMatrixf& aBaseMatrix);
  tBool __stdcall DrawWidget(iWidget* apWidget, iCanvas* apCanvas);
  tBool __stdcall DrawTransformedWidget(iWidget* apWidget, iCanvas* apCanvas, const sMatrixf& aBaseMatrix, tBool abUseScissor);

  // Localization
  tHStringPtr mhspCurrentLocale;

  /// Input ////
 public:
  void __stdcall _KeyDown(tU32 aKey);
  void __stdcall _KeyUp(tU32 aKey);
  void __stdcall _KeyChar(tU32 ch, eKey aKeyLeadingToKeyChar);
  void __stdcall _LeftDblClick(tBool abTriggered);
  void __stdcall _RightDblClick(tBool abTriggered);
  void __stdcall _Wheel(tF32 value);
  void __stdcall _RelativeMouseMove(const sVec2f& avRelMove);

  tBool __stdcall _FingerUpdatePosition(tU32 anFinger, const sVec3f& avNewPos);
  void __stdcall _FingerClick(tU32 anFinger, const sVec3f& avNewPos, tBool abTriggered);

  tBool mKeyIsDown[0xFF];
  eKey mKeyEatChar;

  tU32 mnInputModifiers;
  tU32 mnDefaultInputSubmitFlags;

  typedef astl::hash_map<tU32,tHStringPtr> tShortcutsMap;
  tShortcutsMap mmapShortcuts;

  tF32 mfContentsScale;

  // Finger Positions
  struct sFinger {
    sVec3f        mvPosition;
    WeakPtr<cWidget> mpwHover;
    WeakPtr<cWidget> mpwCapture;
    tWeakWidgetList  mlstCaptureStack;
    WeakPtr<cWidget> mpDragWidget;
    WeakPtr<cWidget> mpwDragWidgetMT;
    Ptr<iDataTable>  mptrDragDT;
    tBool      mbDragDown;
    tU32             mnDragTimeMs;
    tU32             mnDragStartDistance;
    sVec3f      mvDragStart;
    WeakPtr<cWidget> mpwMoveTarget;
    WeakPtr<cWidget> mpwNCMoveTarget;
    WeakPtr<cWidget> mpwDragMoveTarget;
    WeakPtr<cWidget> mpwNCDragMoveTarget;
    tBool            mbIsDown;
    sFinger() {
      mbIsDown = eFalse;
      mvPosition = sVec3f::Zero();
      mpwHover.SetNull();
      mpwCapture.SetNull();
      mpDragWidget.SetNull();
      mvDragStart = sVec3f::Zero();
      mbDragDown = eFalse;
      mnDragTimeMs = 0;
#ifdef niEmbedded
      mnDragStartDistance = 20;
#else
      mnDragStartDistance = 10;
#endif
    }
    sVec2f& GetPosition() {
      return (sVec2f&)mvPosition;
    }
    const sVec2f& GetPosition() const {
      return (sVec2f&)mvPosition;
    }
    const tF32 GetPressure() const {
      return mvPosition.z;
    }
    void _CancelHoverTimers() {
      {
        QPtr<cWidget> moveTarget(mpwMoveTarget);
        if (niIsOK(moveTarget)) {
          moveTarget->SetTimer(eWidgetSystemTimer_Hover,-1);
        }
      }
      {
        QPtr<cWidget> ncMoveTarget(mpwNCMoveTarget);
        if (niIsOK(ncMoveTarget)) {
          ncMoveTarget->SetTimer(eWidgetSystemTimer_NCHover,-1);
        }
      }
    }
  };
  sFinger mFingers[knNumFingers];
  WeakPtr<cWidget> mpwCaptureAll;
  tWeakWidgetList mlstCaptureAllStack;

  tU32 mnLastPrimaryFinger;
  tBool IsPrimaryFinger(tU32 anFinger);
  __forceinline tU32 GetPrimaryFingerID() const {
    return mnLastPrimaryFinger;
  }
  __forceinline const sFinger& GetPrimaryFinger() const {
    return mFingers[FINGER_INDEX(GetPrimaryFingerID())];
  }
  __forceinline sFinger& GetPrimaryFinger() {
    return mFingers[FINGER_INDEX(GetPrimaryFingerID())];
  }

  // Layout...
  tU32 mnRelayoutCount;

  // Game controller
  astl::map<tU32,tU32> mmapGameCtrlInputMasks;

  /// Skins ///
  Ptr<iOverlay> mptrErrorOverlay;
  Ptr<iDataTable> mptrErrorSkin;
  tBool _InitializeSkinDataTable(iDataTable* apDT);
  tHStringPtr mhspDefaultSkin;
  typedef astl::hstring_hash_map<Ptr<iDataTable> > tSkinMap;
  tSkinMap mmapSkins;

  Ptr<iImageMap>  mptrImageMap;

  tBool _CanHover() const;
  tF32 mfHoverDelay;
  tU32 mnHoverInputModifiers;

  virtual void __stdcall SetHoverDelay(tF32 afDelay) {
    mfHoverDelay = afDelay;
  }
  virtual tF32 __stdcall GetHoverDelay() const {
    return mfHoverDelay;
  }
  virtual void __stdcall SetHoverInputModifiers(tU32 anInputModifiers) {
    mnHoverInputModifiers = anInputModifiers;
  }
  virtual tU32 __stdcall GetHoverInputModifiers() const {
    return mnInputModifiers;
  }

  tBool _UpdateOSCursor(iOSWindow* apWindow, iOverlay* apCursor);

  niEndClass(cUIContext);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __UICONTEXT_15354543_H__
