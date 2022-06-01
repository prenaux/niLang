// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "UIContext.h"
#include "niUI_HString.h"
#include <niLang/STL/utils.h>

///////////////////////////////////////////////
static inline tBool __stdcall _UIInputSendMsg(cWidget* apWidget,
                                              tU32 anID,
                                              const Var& avarA = (Var&)niVarNull,
                                              const Var& avarB = (Var&)niVarNull) {
  tBool r = apWidget->SendMessage(anID,avarA,avarB);
  if (niFlagIsNot(apWidget->mnStyle,eWidgetStyle_NoAutoRedraw) &&
      anID != eUIMessage_SetCursor)
  {
    apWidget->Redraw();
  }
  return r;
}

//=================================================================
//
//  Utils
//
//=================================================================
tBool cUIContext::IsPrimaryFinger(tU32 anFinger) {
  if (anFinger == 0 || anFinger == FINGER_ID_POINTER_LEFT) {
    mnLastPrimaryFinger = anFinger;
    return eTrue;
  }
  else {
    return eFalse;
  }
}

//=================================================================
//
//  UIContext
//
//=================================================================

///////////////////////////////////////////////
void __stdcall cUIContext::SetInputModifiers(tU32 anInputModifier) {
  mnInputModifiers = anInputModifier;
}
tU32 __stdcall cUIContext::GetInputModifiers() const {
  return mnInputModifiers;
}

///////////////////////////////////////////////
static __forceinline void __stdcall _UpdateInputModifiersFromFromKeys(
    cUIContext* apUICtx, tU32 aKey, const tBool* aKeys, tU32& anInputModifiers)
{
  switch (aKey) {
    case eKey_LControl: case eKey_RControl:
    case eKey_LAlt:     case eKey_RAlt:
    case eKey_LShift:   case eKey_RShift:
    case eKey_LWin:     case eKey_RWin:
    case eKey_NumLock:
    case eKey_CapsLock:
    case eKey_Scroll:
      break;
    default:
      return;
  }

#define INPUT_IS_DOWN(X) (X)
  niFlagOnIf(anInputModifiers,eKeyMod_Control,
             INPUT_IS_DOWN(aKeys[eKey_LControl]) ||
             INPUT_IS_DOWN(aKeys[eKey_RControl]) ||
             INPUT_IS_DOWN(aKeys[eKey_LWin]) ||
             INPUT_IS_DOWN(aKeys[eKey_RWin]));
  niFlagOnIf(anInputModifiers,eKeyMod_Alt,
             INPUT_IS_DOWN(aKeys[eKey_LAlt]) ||
             INPUT_IS_DOWN(aKeys[eKey_RAlt]));
  niFlagOnIf(anInputModifiers,eKeyMod_Shift,
             INPUT_IS_DOWN(aKeys[eKey_LShift]) ||
             INPUT_IS_DOWN(aKeys[eKey_RShift]));
  niFlagOnIf(anInputModifiers,eKeyMod_NumLock,
             INPUT_IS_DOWN(aKeys[eKey_NumLock]));
  niFlagOnIf(anInputModifiers,eKeyMod_CapsLock,
             INPUT_IS_DOWN(aKeys[eKey_CapsLock]));
  niFlagOnIf(anInputModifiers,eKeyMod_ScrollLock,
             INPUT_IS_DOWN(aKeys[eKey_Scroll]));
#undef INPUT_IS_DOWN

  if (apUICtx->mnHoverInputModifiers && apUICtx->_CanHover()) {
    // Update the finger move which might trigger the hover
    apUICtx->_FingerMove(
        apUICtx->GetPrimaryFingerID(),
        apUICtx->GetPrimaryFinger().mvPosition,
        eFalse);
  }
}

///////////////////////////////////////////////
void __stdcall cUIContext::_KeyDown(tU32 aKey)
{
  if (aKey >= niCountOf(mKeyIsDown))
    return;

  const tBool wasDown = mKeyIsDown[aKey];
  const tBool isRepeat = wasDown;

  const tU32 keyModToSend = mnInputModifiers|(isRepeat?eUIInputModifier_KeyDownRepeat:0);
  _UIInputKeyboardTrace(niFmt("KEYDOWN: %s, %s",
                              niEnumToChars(eKey,aKey),
                              niFlagsToChars(
                                eUIInputModifier,keyModToSend)));

  const tU32 fullKey = aKey|keyModToSend;

  if (!wasDown) {
    // Handle console commands
    {
      tHStringPtr hspCommand = GetShortcutCommand(fullKey);
      if (HStringIsNotEmpty(hspCommand)) {
        if (ni::GetConsole()) {
          ni::GetConsole()->RunCommand(niHStr(hspCommand));
        }
        mKeyEatChar = (eKey)aKey;
        return;
      }
    }

    // Toggle draw op capture
    {
      if (knDrawOpCaptureToggleKey == (aKey|mnInputModifiers)) {
        SetDrawOpCapture(!GetDrawOpCapture());
        mKeyEatChar = (eKey)aKey;
        return;
      }
      else if (knTerminalToggleKey == (aKey|mnInputModifiers)) {
        SetShowTerminal(!GetShowTerminal());
        mKeyEatChar = (eKey)aKey;
        return;
      }
    }
  }

  mKeyIsDown[aKey] = eTrue; // Set, clear the up flag...
  if (!isRepeat) {
    _UpdateInputModifiersFromFromKeys(this,aKey,mKeyIsDown,mnInputModifiers);
  }

  // System shortcuts
  {
    /// Cancel ///
    if (fullKey == eKey_Escape) {
      if (IsFingerDragging(GetPrimaryFingerID())) {
        EndFingerDrag(GetPrimaryFingerID(),GetPrimaryFinger().mvPosition,eTrue);
      }
      else {
        QPtr<cWidget> target(GetInputMessageTarget());
        if (target.IsOK()) {
          _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-Cancel: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
          _UIInputSendMsg(target,eUIMessage_Cancel,niVarNull,niVarNull);
        }
      }
    }
    /// SubmitA/B/C ///
    else if (
        // SubmitA
        ((fullKey == eKey_Enter) ||
         (fullKey == eKey_NumPadEnter)) ||
        // SubmitB
        (fullKey == eKey_Space) ||
        // SubmitC
        (fullKey == (eKeyMod_Control|eKey_Enter)))
    {
      QPtr<cWidget> target(GetInputMessageTarget());
      if (target.IsOK()) {
        if ((niFlagIs(target->mnInputSubmitFlags,eUIInputSubmitFlags_SubmitA) &&
             ((fullKey == eKey_Enter) ||
              (fullKey == eKey_NumPadEnter))) ||
            (niFlagIs(target->mnInputSubmitFlags,eUIInputSubmitFlags_SubmitB) &&
             (fullKey == eKey_Space)) ||
            (niFlagIs(target->mnInputSubmitFlags,eUIInputSubmitFlags_SubmitC) &&
             (fullKey == (eKeyMod_Control|eKey_Enter))))
        {
          _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-Submit: %p (ID:%s)."),
                              target.ptr(),niHStr(target->GetID())));
          _UIInputSendMsg(target,eUIMessage_Submit,target.ptr(),niVarNull);
        }
      }
    }
    else if (!GetPrimaryFinger().mpDragWidget.IsOK()) {
      /// Copy ///
      if (fullKey == (eKeyMod_Control|eKey_C)) {
        QPtr<cWidget> target(GetInputMessageTarget());
        if (target.IsOK()) {
          _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-Copy: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
          ni::Ptr<iDataTable> dt = ni::GetLang()->GetClipboard(eClipboardType_System);
          target->SendMessage(eUIMessage_Copy,dt.ptr(),niVarNull);
          ni::GetLang()->SetClipboard(eClipboardType_System,dt);
        }
      }
      /// Cut ///
      else if (fullKey == (eKeyMod_Control|eKey_X)) {
        QPtr<cWidget> target(GetInputMessageTarget());
        if (target.IsOK()) {
          _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-Cut: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
          ni::Ptr<iDataTable> dt = ni::GetLang()->GetClipboard(eClipboardType_System);
          target->SendMessage(eUIMessage_Cut,dt.ptr(),niVarNull);
          ni::GetLang()->SetClipboard(eClipboardType_System,dt);
        }
      }
      /// Paste ///
      else if (fullKey == (eKeyMod_Control|eKey_V)) {
        QPtr<cWidget> target(GetInputMessageTarget());
        if (target.IsOK()) {
          _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-Paste: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
          ni::Ptr<iDataTable> dt = ni::GetLang()->GetClipboard(eClipboardType_System);
          target->SendMessage(eUIMessage_Paste,dt.ptr(),niVarNull);
        }
      }
      /// Undo ///
      else if (fullKey == (eKeyMod_Control|eKey_Z)) {
        QPtr<cWidget> target(GetInputMessageTarget());
        if (target.IsOK()) {
          _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-Undo: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
          _UIInputSendMsg(target,eUIMessage_Undo,niVarNull,niVarNull);
        }
      }
      /// Redo ///
      else if (fullKey == (eKeyMod_Control|eKey_Y)) {
        QPtr<cWidget> target(GetInputMessageTarget());
        if (target.IsOK()) {
          _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-Redo: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
          _UIInputSendMsg(target,eUIMessage_Redo,niVarNull,niVarNull);
        }
      }
      /// MoveFocus ///
      else if (fullKey == (eKeyMod_Shift|eKey_Tab) ||
               fullKey == eKey_Tab) {
        QPtr<cWidget> target(GetInputMessageTarget());
        if (target.IsOK()) {
          _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-MoveFocus: %p (ID:%s)."),
                              target.ptr(),
                              niHStr(target->GetID())));
          _UIInputSendMsg(target,eUIMessage_MoveFocus,
                          (fullKey & eKeyMod_Shift) ? eTrue : eFalse,
                          niVarNull);
        }
      }
    }
  }

  // Send the key down message
  {
    QPtr<cWidget> target = GetInputMessageTarget();
    if (target.IsOK()) {
      _UIInputKeyboardTrace(niFmt("SEND KEYDOWN TO INPUT TARGET: '%s' (%p)", target->GetID(), (tIntPtr)target.ptr()));
      _UIInputSendMsg(target,eUIMessage_KeyDown,aKey,keyModToSend);
    }
  }
}

void __stdcall cUIContext::_KeyUp(tU32 aKey)
{
  if (aKey >= niCountOf(mKeyIsDown))
    return;

  if (!mKeyIsDown[aKey])
    return;

  mKeyIsDown[aKey] = eFalse;
  _UpdateInputModifiersFromFromKeys(this,aKey,mKeyIsDown,mnInputModifiers);
  _UIInputKeyboardTrace(niFmt("KEYUP: %s, %s",
                              niEnumToChars(eKey,aKey),
                              niFlagsToChars(eUIInputModifier,mnInputModifiers)));

  QPtr<cWidget> target(GetInputMessageTarget());
  if (!target.IsOK())
    return;

  _UIInputSendMsg(target,eUIMessage_KeyUp,aKey,mnInputModifiers);
}

void __stdcall cUIContext::_KeyChar(tU32 ch, eKey aKeyLeadingToKeyChar)
{
  _UIInputKeyboardTrace(niFmt("KEYCHAR: %c (%d), keyLeadingToKeyChar: %d, keyEatChar: %d, inputModifier: %s",ch,ch,aKeyLeadingToKeyChar,mKeyEatChar,niFlagsToChars(eUIInputModifier,mnInputModifiers)));
  if (niFlagIs(mnInputModifiers,eUIInputModifier_Control))
    return; // we dont send key char for Control/Command combination

  if (mKeyEatChar && (aKeyLeadingToKeyChar == mKeyEatChar)) {
    // This prevents a KeyChar message from being sent when system keys are
    // pressed, for example to show the console - so that we dont get an
    // unwanted backtick in editboxes.
    mKeyEatChar = eKey_Unknown;
    return;
  }
  else {
    mKeyEatChar = eKey_Unknown;
  }

  QPtr<cWidget> target(GetInputMessageTarget());
  if (!target.IsOK())
    return;

  _UIInputSendMsg(target,eUIMessage_KeyChar,ch,mnInputModifiers);
}

///////////////////////////////////////////////
void __stdcall cUIContext::_LeftDblClick(tBool abTriggered) {
  Ptr<cWidget> target = this->GetMouseMessageTarget(eTrue);
  if (target.IsOK()) {
    sVec2f mpos = this->GetMousePos()-target->GetAbsolutePosition();
    sVec2f clientPos;
    if (abTriggered)
    { // Down
      _UIFocusTrace(niFmt(_A("### UICONTEXT _LeftDblClick: %p (ID:%s)."),(tIntPtr)target.ptr(),target->GetID()));
      this->SetFocusInput(target,eTrue);
      if (target->MapToClientRect(mpos,clientPos)) {
        _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-LeftDoubleClick: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
        _UIInputSendMsg(target,eUIMessage_LeftDoubleClick,clientPos,mpos);
      }
      else {
        _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-NCLeftDoubleClick: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
        _UIInputSendMsg(target,eUIMessage_NCLeftDoubleClick,clientPos,mpos);
      }
    }
  }
}
void __stdcall cUIContext::_RightDblClick(tBool abTriggered) {
  Ptr<cWidget> target = this->GetMouseMessageTarget(eTrue);
  if (target.IsOK())
  {
    sVec2f mpos = this->GetMousePos()-target->GetAbsolutePosition();
    sVec2f clientPos;
    if (abTriggered)
    { // Down
      _UIFocusTrace(niFmt(_A("### UICONTEXT _RightDblClick: %p (ID:%s)."),(tIntPtr)target.ptr(),target->GetID()));
      this->SetFocusInput(target,eTrue);
      if (target->MapToClientRect(mpos,clientPos)) {
        _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-RightDoubleClick: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
        _UIInputSendMsg(target,eUIMessage_RightDoubleClick,clientPos,mpos);
      }
      else {
        _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-NCRightDoubleClick: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
        _UIInputSendMsg(target,eUIMessage_NCRightDoubleClick,clientPos,mpos);
      }
    }
  }
}

void __stdcall cUIContext::_Wheel(tF32 value) {
  Ptr<cWidget> target;
  if (GetPrimaryFinger().mpDragWidget.IsOK()) {
    const sFinger& f = GetPrimaryFinger();
    if (f.mpwDragMoveTarget.IsOK()) {
      target = QPtr<cWidget>(f.mpwDragMoveTarget);
    }
    else if (f.mpwNCDragMoveTarget.IsOK()) {
      target = QPtr<cWidget>(f.mpwNCDragMoveTarget);
    }
    else {
      target = this->GetMessageTargetByPos(this->GetMousePos());
    }
  }
  else {
    target = this->GetMouseMessageTarget(eTrue); //this->GetInputMessageTarget();
  }
  if (target->GetHoverWidget()) {
    // send the wheel to the hover widget...
    target = (cWidget*)target->GetHoverWidget();
  }
  if (target.IsOK()) {
    // this->SetFocusInput(target,eTrue);
    if ((target->GetClientRect()+target->GetAbsolutePosition()).Intersect(this->GetMousePos())) {
      _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-Wheel: %g, %p (ID:%s)."),
                          value,target.ptr(),niHStr(target->GetID())));
      _UIInputSendMsg(target,eUIMessage_Wheel,value,this->GetCursorPosition());
    }
    else {
      _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-NCWheel: %g, %p (ID:%s)."),
                          value,target.ptr(),niHStr(target->GetID())));
      _UIInputSendMsg(target,eUIMessage_NCWheel,value,this->GetCursorPosition());
    }
  }

  // Update the finger move, the wheel usually scrolls stuff so we might need to send mouse leave / enter...
  {
    _FingerMove(
        GetPrimaryFingerID(),
        GetPrimaryFinger().mvPosition,
        eFalse);
  }
}

void __stdcall cUIContext::_RelativeMouseMove(const sVec2f& aRelMove)
{
  Ptr<cWidget> target = this->GetMouseMessageTarget(eTrue);
  if (!target.IsOK())
    return;

  if ((target->GetClientRect()+target->GetAbsolutePosition()).Intersect(this->GetMousePos())) {
    _UIInputSendMsg(target,
                    eUIMessage_FingerRelativeMove,
                    FINGER_POINTER_ID_MIN+ePointerButton_Left,
                    Vec3f(aRelMove.x,aRelMove.y,1.0));
  }
  else {
    _UIInputSendMsg(target,
                    eUIMessage_NCFingerRelativeMove,
                    FINGER_POINTER_ID_MIN+ePointerButton_Left,
                    Vec3f(aRelMove.x,aRelMove.y,1.0));
  }
}

#define FPARAMS(CLIENTPOS) anFinger,Vec3f(CLIENTPOS.x,CLIENTPOS.y,f.GetPressure())

///////////////////////////////////////////////
tBool __stdcall cUIContext::_FingerUpdatePosition(tU32 anFinger, const sVec3f& avNewPos)
{
  CHECK_FINGER(anFinger,eFalse);
  sFinger& f = GET_FINGER(anFinger);
  if (f.mvPosition == avNewPos)
    return eFalse;
  f.mvPosition = avNewPos;
  return eTrue;
}

///////////////////////////////////////////////
void __stdcall cUIContext::_FingerClick(tU32 anFinger, const sVec3f& avNewPos, tBool abTriggered) {
  CHECK_FINGER(anFinger,;);
  sFinger& f = GET_FINGER(anFinger);
  f.mbIsDown = abTriggered;
  const tBool isPrimaryFinger = IsPrimaryFinger(anFinger);

  _FingerUpdatePosition(anFinger,avNewPos);

  if (isPrimaryFinger && (!abTriggered) && (mnLastPrimaryFinger != FINGER_ID_POINTER_LEFT)) {
    // If primary finger up and its not a mouse cursor (aka its a finger) we disable the hover
    // timer so that it doesn't pop.
    f._CancelHoverTimers();
  }

  // Cancel primary drag'n drop when right click up or down...
  if ((anFinger == FINGER_ID_POINTER_RIGHT) && IsFingerDragging(GetPrimaryFingerID())) {
    EndFingerDrag(mnLastPrimaryFinger,avNewPos,eTrue);
    return;
  }

  _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-_FingerClick[%d]: %d"),anFinger,abTriggered));
  Ptr<cWidget> target = this->GetFingerMessageTarget(anFinger,eTrue);
  if (target.IsOK()) {
    sVec2f mpos = f.GetPosition()-target->GetAbsolutePosition();
    sVec2f clientPos;
    if (abTriggered)
    { // Down
      if (isPrimaryFinger ||
          (FINGER_IS_POINTER(mnLastPrimaryFinger) && FINGER_IS_POINTER(anFinger)))
      {
        _UIFocusTrace(niFmt(_A("### UICONTEXT _FingetClick down primary finger: %p (ID:%s)."),(tIntPtr)target.ptr(),target->GetID()));
        this->SetFocusInput(target,eTrue);
      }
      if (target->MapToClientRect(mpos,clientPos)) {
        _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-FingerDown[%d]: %p (ID:%s)."),anFinger,target.ptr(),niHStr(target->GetID())));
        _UIInputSendMsg(target,eUIMessage_FingerDown,FPARAMS(clientPos));
        if (isPrimaryFinger) {
          _UIInputSendMsg(target,eUIMessage_LeftClickDown,clientPos,mpos);
        }
        else if (anFinger == FINGER_ID_POINTER_RIGHT) {
          _UIInputSendMsg(target,eUIMessage_RightClickDown,clientPos,mpos);
        }
      }
      else {
        _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-NCFingerDown[%d]: %p (ID:%s)."),anFinger,target.ptr(),niHStr(target->GetID())));
        _UIInputSendMsg(target,eUIMessage_NCFingerDown,FPARAMS(clientPos));
        if (isPrimaryFinger) {
          _UIInputSendMsg(target,eUIMessage_NCLeftClickDown,clientPos,mpos);
        }
        else if (anFinger == FINGER_ID_POINTER_RIGHT) {
          _UIInputSendMsg(target,eUIMessage_NCRightClickDown,clientPos,mpos);
        }
      }
      // DND
      f.mbDragDown = eTrue;
      f.mnDragTimeMs = ((tU32)(ni::TimerInSeconds()*1000.0));
      f.mvDragStart = f.mvPosition;
    }
    else
    { // Up
      // DND, End drag first, since some things like the tree
      // view might have a "secondary/pre" selection which is
      // released when click is up, yet the drag'n drop needs to
      // copy that secondary selection and not the "real"
      // selection
      this->EndFingerDrag(anFinger,f.mvPosition,eFalse);

      if (target->MapToClientRect(mpos,clientPos)) {
        _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-FingerUp[%d]: %p (ID:%s)."),anFinger,target.ptr(),niHStr(target->GetID())));
        _UIInputSendMsg(target,eUIMessage_FingerUp,FPARAMS(clientPos));
        if (isPrimaryFinger) {
          _UIInputSendMsg(target,eUIMessage_LeftClickUp,clientPos,mpos);
        }
        else if (anFinger == FINGER_ID_POINTER_RIGHT) {
          _UIInputSendMsg(target,eUIMessage_RightClickUp,clientPos,mpos);
        }
      }
      else {
        _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-NCFingerUp[%d]: %p (ID:%s)."),anFinger,target.ptr(),niHStr(target->GetID())));
        _UIInputSendMsg(target,eUIMessage_NCFingerUp,FPARAMS(clientPos));
        if (isPrimaryFinger) {
          _UIInputSendMsg(target,eUIMessage_NCLeftClickUp,clientPos,mpos);
        }
        else if (anFinger == FINGER_ID_POINTER_RIGHT) {
          _UIInputSendMsg(target,eUIMessage_NCRightClickUp,clientPos,mpos);
        }
      }
    }
  }
}

///////////////////////////////////////////////
tBool cUIContext::_CanHover() const {
  return
      (this->mfHoverDelay > 0.0f) &&
      (!mnHoverInputModifiers || (mnHoverInputModifiers == mnInputModifiers));
}

///////////////////////////////////////////////
void cUIContext::_FingerMove(tU32 anFinger, sVec3f avNewPos, tBool abOnlyIfChanged)
{
  CHECK_FINGER(anFinger,;);

  sFinger& f = GET_FINGER(anFinger);
  if (abOnlyIfChanged) {
    if ((sVec2f&)avNewPos == (sVec2f&)f.mvPosition)
      return;
  }
  const tBool isPrimaryFinger = IsPrimaryFinger(anFinger);

  _FingerUpdatePosition(anFinger,avNewPos);

  Ptr<cWidget> target = GetFingerMessageTarget(anFinger,eTrue);

  // different target, send the finger leave message
  {
    QPtr<cWidget> moveTarget(f.mpwMoveTarget);
    if (moveTarget.IsOK() && moveTarget.ptr() != target.ptr()) {
      sVec2f mpos = f.GetPosition()-target->GetAbsolutePosition();
      sVec2f clientPos;
      moveTarget->MapToClientRect(mpos,clientPos);
      _UIInputSendMsg(moveTarget,eUIMessage_FingerLeave,FPARAMS(clientPos));
      if (isPrimaryFinger) {
        f._CancelHoverTimers();
        _UIInputSendMsg(moveTarget,eUIMessage_MouseLeave,clientPos,mpos);
      }
      _UIInputMMTrace(niFmt(_A("### UICONTEXT-INPUT-FingerLeave: %p (ID:%s)."),moveTarget.ptr(),niHStr(moveTarget->GetID())));
      f.mpwMoveTarget.SetNull();
    }
  }

  // different target, send the finger leave message
  {
    QPtr<cWidget> ncMoveTarget(f.mpwNCMoveTarget);
    if (ncMoveTarget.IsOK() && ncMoveTarget.ptr() != target.ptr()) {
      sVec2f mpos = f.GetPosition()-target->GetAbsolutePosition();
      sVec2f clientPos;
      ncMoveTarget->MapToClientRect(mpos,clientPos);
      _UIInputSendMsg(ncMoveTarget,eUIMessage_NCFingerLeave,FPARAMS(clientPos));
      if (isPrimaryFinger) {
        f._CancelHoverTimers();
        _UIInputSendMsg(ncMoveTarget,eUIMessage_NCMouseLeave,clientPos,mpos);
      }
      _UIInputMMTrace(niFmt(_A("### UICONTEXT-INPUT-NCFingerLeave: %p (ID:%s)."),ncMoveTarget.ptr(),niHStr(ncMoveTarget->GetID())));
      f.mpwNCMoveTarget.SetNull();
    }
  }

  if (target.IsOK())
  {
    sVec2f mpos = f.GetPosition()-target->GetAbsolutePosition();
    sVec2f clientPos;

    if (target->MapToClientRect(mpos,clientPos)) {

      // different target, send the finger enter message
      {
        QPtr<cWidget> moveTarget(f.mpwMoveTarget);
        if (moveTarget.ptr() != target.ptr()) {
          niAssert(!f.mpwMoveTarget.IsOK());
          f.mpwMoveTarget.Swap(target.ptr());
          _UIInputSendMsg(target,eUIMessage_FingerEnter,FPARAMS(clientPos));
          if (isPrimaryFinger) {
            _UIInputSendMsg(target,eUIMessage_MouseEnter,clientPos,mpos);
          }
          _UIInputMMTrace(niFmt(_A("### UICONTEXT-INPUT-FingerEnter: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
        }
      }

      _UIInputSendMsg(target,eUIMessage_FingerMove,FPARAMS(clientPos));
      if (isPrimaryFinger) {
        _UIInputSendMsg(target,eUIMessage_MouseMove,clientPos,mpos);
      }
      _UIInputMMTrace(niFmt(_A("### UICONTEXT-INPUT-FingerMove[%s,%s]: %p (ID:%s)."),
                            clientPos,mpos,
                            target.ptr(),niHStr(target->GetID())));
      // update the hover
      if (_CanHover() && isPrimaryFinger) {
        QPtr<cWidget> moveTarget(f.mpwMoveTarget);
        if (moveTarget.IsOK() &&
            niFlagIsNot(moveTarget->mStatus,WDGSTATUS_HASHOVERWIDGET))
        {
          f._CancelHoverTimers();
          moveTarget->SetTimer(eWidgetSystemTimer_Hover,this->mfHoverDelay);
        }
      }
    }
    else {
      // different target, send the nc finger enter message
      {
        QPtr<cWidget> ncMoveTarget(f.mpwNCMoveTarget);
        if (ncMoveTarget.ptr() != target.ptr()) {
          niAssert(!ncMoveTarget.IsOK());
          f.mpwNCMoveTarget.Swap(target);
          _UIInputSendMsg(target,eUIMessage_NCFingerEnter,FPARAMS(clientPos));
          if (isPrimaryFinger) {
            _UIInputSendMsg(target,eUIMessage_NCMouseEnter,clientPos,mpos);
          }
          _UIInputMMTrace(niFmt(_A("### UICONTEXT-INPUT-NCFingerEnter: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
        }
      }
      _UIInputSendMsg(target,eUIMessage_NCFingerMove,FPARAMS(clientPos));
      if (isPrimaryFinger) {
        _UIInputSendMsg(target,eUIMessage_NCMouseMove,clientPos,mpos);
      }
      _UIInputMMTrace(niFmt(_A("### UICONTEXT-INPUT-NCFingerMove: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
      // update the hover
      if (_CanHover() && isPrimaryFinger) {
        QPtr<cWidget> ncMoveTarget(f.mpwNCMoveTarget);
        if (ncMoveTarget.IsOK() &&
            niFlagIsNot(ncMoveTarget->mStatus,WDGSTATUS_HASHOVERWIDGET))
        {
          f._CancelHoverTimers();
          ncMoveTarget->SetTimer(eWidgetSystemTimer_NCHover,this->mfHoverDelay);
        }
      }
    }

    // Drag finger move messages
    if (f.mbDragDown) {
      if (isPrimaryFinger || !FINGER_IS_POINTER(anFinger)) {
        BeginFingerDrag(anFinger,target,f.mvDragStart,f.mvPosition);
      }
    }

    if (isPrimaryFinger) {
      _UIInputMMTrace(niFmt(_A("### UICONTEXT-INPUT-SetCursor: %p (ID:%s)."),target.ptr(),niHStr(target->GetID())));
      _UIInputSendMsg(target,eUIMessage_SetCursor,clientPos,mpos);
    }
  }

  if (f.mpDragWidget.IsOK()) {
    if (_CanSendFingerDragMessage(anFinger)) {
      target = GetDragDestinationByPos(f.GetPosition());

      // different target, send the finger leave message
      {
        QPtr<cWidget> ptrDragMoveTarget(f.mpwDragMoveTarget);
        if (ptrDragMoveTarget.IsOK() && (ptrDragMoveTarget.ptr() != target.ptr() || !f.mpDragWidget.IsOK())) {
          _UIInputSendMsg(ptrDragMoveTarget,eUIMessage_DragFingerLeave,anFinger,niVarNull);
          if (isPrimaryFinger) {
            _UIInputSendMsg(ptrDragMoveTarget,eUIMessage_DragMouseLeave,niVarNull,niVarNull);
          }
          _UIDragTrace(niFmt(_A("### UICONTEXT-INPUT-DragFingerLeave: %p (ID:%s)."),
                             ptrDragMoveTarget.ptr(),
                             niHStr(ptrDragMoveTarget->GetID())));
          f.mpwDragMoveTarget.SetNull();
        }
      }

      // different target, send the finger leave message
      {
        QPtr<cWidget> ptrNCDragMoveTarget(f.mpwNCDragMoveTarget);
        if (ptrNCDragMoveTarget.IsOK() && (ptrNCDragMoveTarget.ptr() != target.ptr() || !f.mpDragWidget.IsOK())) {
          _UIInputSendMsg(ptrNCDragMoveTarget,eUIMessage_NCDragFingerLeave,anFinger,niVarNull);
          if (isPrimaryFinger) {
            _UIInputSendMsg(ptrNCDragMoveTarget,eUIMessage_NCDragMouseLeave,niVarNull,niVarNull);
          }
          _UIDragTrace(niFmt(_A("### UICONTEXT-INPUT-NCDragFingerLeave: %p (ID:%s)."),
                             ptrNCDragMoveTarget.ptr(),
                             niHStr(ptrNCDragMoveTarget->GetID())));
          f.mpwNCDragMoveTarget.SetNull();
        }
      }

      if (target.IsOK()) {
        sVec2f mpos = f.GetPosition()-target->GetAbsolutePosition();
        sVec2f clientPos;
        if (target->MapToClientRect(mpos,clientPos)) {
          {
            QPtr<cWidget> ptrDragMoveTarget(f.mpwDragMoveTarget);
            // different target, send the finger enter message
            if (ptrDragMoveTarget.ptr() != target.ptr()) {
              niAssert(!f.mpwDragMoveTarget.IsOK());
              f.mpwDragMoveTarget.Swap(target);
              _UIInputSendMsg(target,eUIMessage_DragFingerEnter,FPARAMS(clientPos));
              if (isPrimaryFinger) {
                _UIInputSendMsg(target,eUIMessage_DragMouseEnter,clientPos,mpos);
              }
              _UIDragTrace(niFmt(_A("### UICONTEXT-INPUT-DragFingerEnter: %p (ID:%s)."),
                                 target.ptr(),niHStr(target->GetID())));
            }
          }
          _UIInputSendMsg(target,eUIMessage_DragFingerMove,FPARAMS(clientPos));
          if (isPrimaryFinger) {
            _UIInputSendMsg(target,eUIMessage_DragMouseMove,clientPos,mpos);
          }
          _UIDragTrace(niFmt(_A("### UICONTEXT-INPUT-DragFingerMove: %p (ID:%s)."),
                             target.ptr(),niHStr(target->GetID())));
        }
        else {
          {
            QPtr<cWidget> ptrNCDragMoveTarget(f.mpwNCDragMoveTarget);
            // different target, send the nc finger enter message
            if (ptrNCDragMoveTarget.ptr() != target.ptr()) {
              niAssert(!f.mpwNCDragMoveTarget.IsOK());
              f.mpwNCDragMoveTarget.Swap(target);
              _UIInputSendMsg(target,eUIMessage_NCDragFingerEnter,FPARAMS(clientPos));
              if (isPrimaryFinger) {
                _UIInputSendMsg(target,eUIMessage_NCDragMouseEnter,clientPos,mpos);
              }
              _UIDragTrace(niFmt(_A("### UICONTEXT-INPUT-NCDragFingerEnter: %p (ID:%s)."),
                                 target.ptr(),niHStr(target->GetID())));
            }
          }
          _UIInputSendMsg(target,eUIMessage_NCDragFingerMove,FPARAMS(clientPos));
          if (isPrimaryFinger) {
            _UIInputSendMsg(target,eUIMessage_NCDragMouseMove,clientPos,mpos);
          }
          _UIDragTrace(niFmt(_A("### UICONTEXT-INPUT-NCDragFingerMove: %p (ID:%s)."),
                             target.ptr(),niHStr(target->GetID())));
        }
      }
    }
  }
  else {
    {
      QPtr<cWidget> ptrDragMoveTarget(f.mpwDragMoveTarget);
      // different target, send the finger leave message
      if (ptrDragMoveTarget.IsOK() && (ptrDragMoveTarget.ptr() != target.ptr() || !f.mpDragWidget.IsOK())) {
        _UIInputSendMsg(ptrDragMoveTarget,eUIMessage_DragFingerLeave,anFinger,niVarNull);
        if (isPrimaryFinger) {
          _UIInputSendMsg(ptrDragMoveTarget,eUIMessage_DragMouseLeave,niVarNull,niVarNull);
        }
        _UIDragTrace(niFmt(_A("### UICONTEXT-INPUT-DragFingerLeave: %p (ID:%s)."),
                           ptrDragMoveTarget.ptr(),niHStr(ptrDragMoveTarget->GetID())));
        f.mpwDragMoveTarget.SetNull();
      }
    }
    {
      QPtr<cWidget> ptrNCDragMoveTarget(f.mpwNCDragMoveTarget);
      // different target, send the finger leave message
      if (ptrNCDragMoveTarget.IsOK() && (ptrNCDragMoveTarget.ptr() != target.ptr() || !f.mpDragWidget.IsOK())) {
        _UIInputSendMsg(ptrNCDragMoveTarget,eUIMessage_NCDragFingerLeave,anFinger,niVarNull);
        if (isPrimaryFinger) {
          _UIInputSendMsg(ptrNCDragMoveTarget,eUIMessage_NCDragMouseLeave,niVarNull,niVarNull);
        }
        _UIDragTrace(niFmt(_A("### UICONTEXT-INPUT-NCDragFingerLeave: %p (ID:%s)."),
                           ptrNCDragMoveTarget.ptr(),niHStr(ptrNCDragMoveTarget->GetID())));
        f.mpwNCDragMoveTarget.SetNull();
      }
    }
  }
}

///////////////////////////////////////////////
void __stdcall cUIContext::SendWindowMessage(eOSWindowMessage aMsg, const Var& avarA, const Var& avarB) {
  switch (aMsg) {
    case eOSWindowMessage_MouseButtonDoubleClick:
      InputDoubleClick((ePointerButton)avarA.mU32);
      break;
    case eOSWindowMessage_RelativeMouseMove: {
      // we can get vec2i or vec2f here...
      Var varRel = avarA;
      VarConvertType(varRel,eType_Vec2f);
      InputRelativeMouseMove(varRel.GetVec2fValue());
      break;
    }
    case eOSWindowMessage_MouseWheel:
      InputMouseWheel(avarA.GetFloatValue());
      break;
    case eOSWindowMessage_Pinch: {
      InputPinch(avarA.GetFloatValue(),(eGestureState)avarB.GetEnumValue());
      break;
    }
    case eOSWindowMessage_MouseButtonDown:
    case eOSWindowMessage_MouseButtonUp: {
      const tU32 fingerId = FINGER_POINTER_ID_MIN+avarA.mU32;
      const tU32 fingerIndex = FINGER_INDEX(fingerId);
      if (fingerIndex < knNumFingers) {
        const sFinger& f = GetPrimaryFinger();
        _FingerClick(fingerId,f.mvPosition,(aMsg==eOSWindowMessage_MouseButtonDown));
      }
      break;
    }
    case eOSWindowMessage_MouseMove: {
      const sFinger& f = GetPrimaryFinger();
      sVec2f clientPos = avarA.GetVec2fValue() / mfContentsScale;
      _FingerMove(
          GetPrimaryFingerID(),
          Vec3f(clientPos.x,clientPos.y,f.mvPosition.z),
          eTrue);
      break;
    }
    case eOSWindowMessage_KeyDown: {
      _KeyDown((eKey)avarA.mU32);
      break;
    }
    case eOSWindowMessage_KeyUp: {
      _KeyUp((eKey)avarA.mU32);
      break;
    }
    case eOSWindowMessage_KeyChar: {
      _KeyChar(avarA.mU32,(eKey)avarB.mU32);
      break;
    }
    case eOSWindowMessage_FingerDown: {
      const tU32 fingerIndex = avarA.mU32;
      const sVec3f fingerPos = avarB.GetVec3fValue() / Vec3f(mfContentsScale,mfContentsScale,1.0f);
      _FingerClick(fingerIndex,fingerPos,eTrue);
      break;
    }
    case eOSWindowMessage_FingerUp: {
      const tU32 fingerIndex = avarA.mU32;
      const sVec3f fingerPos = avarB.GetVec3fValue() / Vec3f(mfContentsScale,mfContentsScale,1.0f);
      _FingerClick(fingerIndex,fingerPos,eFalse);
      break;
    }
    case eOSWindowMessage_FingerMove: {
      const tU32 fingerIndex = avarA.mU32;
      const sVec3f fingerPos = avarB.GetVec3fValue() / Vec3f(mfContentsScale,mfContentsScale,1.0f);
      _FingerMove(fingerIndex,fingerPos,eTrue);
      break;
    }
    case eOSWindowMessage_FingerRelativeMove: {
      const tU32 fingerIndex = avarA.mU32;
      const sVec3f fingerPos = avarB.GetVec3fValue() / Vec3f(mfContentsScale,mfContentsScale,1.0f);
      InputFingerRelativeMove(fingerIndex,fingerPos);
      break;
    }
    case eOSWindowMessage_SwitchIn: {
      // Update input modifier when the window is re-activated
      mnInputModifiers = avarB.mU32;
      break;
    }
  }
}

///////////////////////////////////////////////
void cUIContext::_WindowUpdateCursorStates() {
#ifdef UICONTEXT_USE_CURSOR_CAPTURE
  QPtr<iOSWindow> w = mptrGraphicsContext;
  if (w.IsOK()) {
    const tBool bCapture = (mpwCaptureAll.IsOK());
    if (bCapture) {
      w->SetCursorCapture(eTrue);
    }
    else {
      w->SetCursorCapture(eFalse);
    }
  }
#endif
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::SetCursor(iOverlay *pCursor)
{
  if (IsFingerDragging(GetPrimaryFingerID()) &&
      _CanSendFingerDragMessage(GetPrimaryFingerID()))
  {
    Ptr<iWidget> ptrTarget = GetDragDestinationByPos(GetMousePos());
    if (!ptrTarget.IsOK() || niFlagIsNot(ptrTarget->GetStyle(),eWidgetStyle_DragDestination)) {
      if (mptrMouseCursor[eUIStandardCursor_DraggingInvalid].IsOK())
        pCursor = mptrMouseCursor[eUIStandardCursor_DraggingInvalid];
    }
    else if (GetInputModifiers() & eUIInputModifier_Control) {
      if (mptrMouseCursor[eUIStandardCursor_DraggingCopy].IsOK())
        pCursor = mptrMouseCursor[eUIStandardCursor_DraggingCopy];
    }
    else {
      if (mptrMouseCursor[eUIStandardCursor_DraggingMove].IsOK())
        pCursor = mptrMouseCursor[eUIStandardCursor_DraggingMove];
    }
  }
  mptrMouseCursor[eUIStandardCursor_Current] = niGetIfOK(pCursor);
  return mptrMouseCursor[eUIStandardCursor_Current].IsOK();
}
iOverlay* __stdcall cUIContext::GetCursor() const
{
  return mptrMouseCursor[eUIStandardCursor_Current];
}

///////////////////////////////////////////////
sVec2f __stdcall cUIContext::GetCursorPosition() const {
  return GetPrimaryFinger().GetPosition();
}

///////////////////////////////////////////////
sVec3f __stdcall cUIContext::GetFingerPosition(tU32 anFinger) const {
  CHECK_FINGER(anFinger,sVec3f::Zero());
  return GET_FINGER(anFinger).mvPosition;
}
tBool __stdcall cUIContext::GetFingerDown(tU32 anFinger) const {
  CHECK_FINGER(anFinger,eFalse);
  return GET_FINGER(anFinger).mbIsDown;
}

///////////////////////////////////////////////
void __stdcall cUIContext::ClearShortcuts() {
  mmapShortcuts.clear();
}
void __stdcall cUIContext::AddShortcut(tU32 anKey, iHString* ahspCmd) {
  astl::upsert(mmapShortcuts,anKey,ahspCmd);
}
void __stdcall cUIContext::RemoveShortcut(tU32 anKey) {
  tShortcutsMap::iterator it = mmapShortcuts.find(anKey);
  if (it != mmapShortcuts.end()) {
    mmapShortcuts.erase(it);
  }
}
iHString* __stdcall cUIContext::GetShortcutCommand(tU32 anKey) const {
  tShortcutsMap::const_iterator it = mmapShortcuts.find(anKey);
  if (it != mmapShortcuts.end()) {
    return it->second;
  }
  return NULL;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::GetKeyDown(tU8 aKey) const {
  return mKeyIsDown[aKey];
}

///////////////////////////////////////////////
void __stdcall cUIContext::InputFingerMove(tU32 anFinger, const sVec3f& avPosition) {
  _FingerMove(anFinger,avPosition,eTrue);
}
void __stdcall cUIContext::InputFingerRelativeMove(tU32 anFinger, const sVec3f& avRelMove) {
  CHECK_FINGER(anFinger,;);
  Ptr<cWidget> target = this->GetFingerMessageTarget(anFinger,eTrue);
  _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-FingerRelativeMove: %p (%s) [%d]: %s"),target.ptr(),target.IsOK()?target->GetID():NULL,anFinger,avRelMove));
  if (target.IsOK()) {
    sFinger& f = GET_FINGER(anFinger);
    sVec2f mpos = f.GetPosition()-target->GetAbsolutePosition();
    sVec2f clientPos;
    if (target->MapToClientRect(mpos,clientPos)) {
      _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-FingerRelativeMove")));
      _UIInputSendMsg(target,eUIMessage_FingerRelativeMove,anFinger,avRelMove);
    }
    else {
      _UIInputTrace(niFmt(_A("### UICONTEXT-INPUT-NCFingerRelativeMove")));
      _UIInputSendMsg(target,eUIMessage_NCFingerRelativeMove,anFinger,avRelMove);
    }
  }
}
void __stdcall cUIContext::InputFingerPress(tU32 anFinger, const sVec3f& avPosition, tBool abDown) {
  _FingerClick(anFinger,avPosition,abDown);
}
void __stdcall cUIContext::InputKeyPress(eKey aKey, tBool abDown) {
  if (abDown)
    _KeyDown(aKey);
  else
    _KeyUp(aKey);
}
void __stdcall cUIContext::InputKeyChar(tU32 aCharCodePoint, eKey aKeyLeadingToKeyChar) {
  _KeyChar(aCharCodePoint,aKeyLeadingToKeyChar);
}
void __stdcall cUIContext::InputMouseWheel(const tF32 afDelta) {
  _Wheel(afDelta);
}
void __stdcall cUIContext::InputPinch(const tF32 afScale, const eGestureState aState) {
  Ptr<cWidget> target = this->GetMouseMessageTarget(eTrue);
  if (!target.IsOK())
    return;
  _UIInputSendMsg(target,eUIMessage_Pinch,afScale,(tU32)aState);
}
void __stdcall cUIContext::InputDoubleClick(ePointerButton aPointer) {
  switch (aPointer) {
    case ePointerButton_Left:
      _LeftDblClick(eTrue);
      break;
    case ePointerButton_Right:
      _RightDblClick(eTrue);
      break;
  }
}
void __stdcall cUIContext::InputRelativeMouseMove(const sVec2f& avRelMove) {
  _RelativeMouseMove(avRelMove);
}

void __stdcall cUIContext::InputGameCtrl(iGameCtrl* apGameController) {
  if (!niIsOK(apGameController))
    return;

  QPtr<cWidget> target(GetInputMessageTarget());
  if (target.IsOK()) {
    _UIInputSendMsg(target,eUIMessage_GameCtrl,apGameController,niVarNull);
    astl::map<tU32,tU32>::iterator it = mmapGameCtrlInputCount.find(apGameController->GetIndex());
    if (it == mmapGameCtrlInputCount.end()) {
      // First time we've seen it only send the down messages....
      tU32 mask = 0;
      niLoop(i,ni::Min(apGameController->GetNumButtons(),32)) {
        const tF32 v = apGameController->GetButton(i);
        if (!niFloatIsZero(v)) {
          mask |= niBit(i);
          // GameCtrlButtonDown
          _UIInputSendMsg(target,eUIMessage_GameCtrlButtonDown,apGameController,i);
        }
      }
      it = astl::upsert(mmapGameCtrlInputCount,apGameController->GetIndex(),1);
    }
    else {
      tU32 wasMask = it->second;
      tU32 mask = 0;
      niLoop(i,ni::Min(apGameController->GetNumButtons(),32)) {
        const tF32 v = apGameController->GetButton(i);
        if (!niFloatIsZero(v)) {
          mask |= niBit(i);
          // was up previously, send the down message
          if (!(wasMask & niBit(i))) {
            _UIInputSendMsg(target,eUIMessage_GameCtrlButtonDown,apGameController,i);
          }
        }
        else {
          // was down previously, send the up message
          if (wasMask & niBit(i)) {
            _UIInputSendMsg(target,eUIMessage_GameCtrlButtonUp,apGameController,i);
          }
        }
      }
      it->second = mask;
    }
  }
}
