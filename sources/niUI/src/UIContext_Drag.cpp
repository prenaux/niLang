// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "UIContext.h"

///////////////////////////////////////////////
void __stdcall cUIContext::SetDragStartDistance(tU32 anPixelDistance) {
  SetDragFingerStartDistance(eInvalidHandle,anPixelDistance);
}
tU32 __stdcall cUIContext::GetDragStartDistance() const {
  return GetPrimaryFinger().mnDragStartDistance;
}

///////////////////////////////////////////////
tBool cUIContext::BeginFingerDrag(tU32 anFinger, cWidget* apWidget, const sVec3f& avStartPos, const sVec3f& avMousePos)
{
  CHECK_FINGER(anFinger,eFalse);
  sFinger& f = GET_FINGER(anFinger);

  if (ni::VecDistance((sVec2f&)f.mvDragStart,f.GetPosition()) < f.mnDragStartDistance)
    return eFalse;

  const tBool isPrimaryFinger = IsPrimaryFinger(anFinger);

  f.mbDragDown = eFalse;
  niCheckSilent(niIsOK(apWidget),eFalse);
  sVec2f vNCMousePos = ((sVec2f&)avStartPos)-apWidget->GetAbsolutePosition();
  if (apWidget->mClientRect.Intersect(vNCMousePos)) {
    apWidget->SendMessage(ni::eUIMessage_DragFingerBegin,anFinger,avStartPos);
    if (isPrimaryFinger) {
      const sVec2f a = {avStartPos.x,avStartPos.y};
      apWidget->SendMessage(ni::eUIMessage_DragBegin,a,niVarNull);
    }
  }
  else if (apWidget->GetWidgetRect().Intersect(vNCMousePos)) {
    apWidget->SendMessage(ni::eUIMessage_NCDragFingerBegin,anFinger,avStartPos);
    if (isPrimaryFinger) {
      const sVec2f a = {avStartPos.x,avStartPos.y};
      apWidget->SendMessage(ni::eUIMessage_NCDragBegin,a,niVarNull);
    }
  }
  else {
    return eFalse; // Cant start dragging if outside of widget's rect
  }
  f.mpwDragWidgetMT = apWidget;
  _UIDragTrace(niFmt(_A("BEGIN DRAG FINGER[%d]: [%s], (%.2f,%.2f)\n"),anFinger,niHStr(apWidget->GetID()),avStartPos.x,avStartPos.y));

  // Skip if widget is not a DragSource
  niCheckSilent(
      niFlagIs(apWidget->GetStyle(),eWidgetStyle_DragSource),
      eFalse);

  f.mptrDragDT = ni::CreateDataTable(_A("Clipboard"));
  f.mptrDragDT->SetVec2(_A("cursor_start"),(sVec2f&)avStartPos);
  f.mptrDragDT->SetVec3(_A("finger_start"),avStartPos);
  f.mpDragWidget = apWidget;
  // dragging began, update the finger move
  _FingerMove(anFinger,f.mvPosition,eFalse);
  return ni::eTrue;
}

///////////////////////////////////////////////
void cUIContext::EndFingerDrag(tU32 anFinger, const sVec3f& avAbsPos, tBool abCancel)
{
  CHECK_FINGER(anFinger,;);
  sFinger& f = GET_FINGER(anFinger);

  f.mbDragDown = eFalse;

  {
    QPtr<cWidget> dragWidgetMT(f.mpwDragWidgetMT);
    if (!dragWidgetMT.IsOK()) {
      return;
    }

    const tBool isPrimaryFinger = IsPrimaryFinger(anFinger);

    if (dragWidgetMT->mClientRect.Intersect(
            ((sVec2f&)(avAbsPos))-dragWidgetMT->GetAbsolutePosition()))
    {
      dragWidgetMT->SendMessage(ni::eUIMessage_DragFingerEnd,anFinger,avAbsPos);
      if (isPrimaryFinger) {
        const sVec2f a = {avAbsPos.x,avAbsPos.y};
        dragWidgetMT->SendMessage(ni::eUIMessage_DragEnd,a,niVarNull);
      }
    }
    else {
      dragWidgetMT->SendMessage(ni::eUIMessage_NCDragFingerEnd,anFinger,avAbsPos);
      if (isPrimaryFinger) {
        const sVec2f a = {avAbsPos.x,avAbsPos.y};
        dragWidgetMT->SendMessage(ni::eUIMessage_NCDragEnd,a,niVarNull);
      }
    }
    _UIDragTrace(niFmt(_A("END DRAG FINGER[%d] (%.2f,%.2f), TO [%s]\n"),
                       anFinger,
                       avAbsPos.x,avAbsPos.y,
                       dragWidgetMT->GetID()));
  }

  {
    QPtr<cWidget> dragWidget(f.mpDragWidget);
    if (!dragWidget.IsOK())
      return;

    _UIDragTrace(niFmt(_A("DRAG WAS FROM [%s]\n"),dragWidget->GetID()));

    if (!abCancel && _CanSendFingerDropMessage(anFinger)) {
      Ptr<iWidget> ptrW = GetDragDestinationByPos((sVec2f&)avAbsPos);
      if (ptrW.IsOK()) {
        f.mptrDragDT->SetString(_A("type"),_A("drop"));
        f.mptrDragDT->SetInt(_A("finger"),anFinger);
        f.mptrDragDT->SetInt(_A("inner"),dragWidget.ptr()==ptrW.ptr());
        f.mptrDragDT->SetInt(_A("input_modifier"),GetInputModifiers());
        f.mptrDragDT->SetIUnknown(_A("source"),dragWidget);
        f.mptrDragDT->SetIUnknown(_A("destination"),ptrW);
        f.mptrDragDT->SetVec2(_A("cursor_end"),(sVec2f&)avAbsPos);
        f.mptrDragDT->SetVec3(_A("finger_end"),avAbsPos);
        dragWidget->SendMessage(eUIMessage_Copy,f.mptrDragDT.ptr(),avAbsPos);
        ptrW->SendMessage(eUIMessage_Paste,f.mptrDragDT.ptr(),avAbsPos);
      }
    }
  }

  f.mpDragWidget.SetNull();
  f.mptrDragDT = NULL;
  // dragging ended, update the finger move
  _FingerMove(anFinger,f.mvPosition,eFalse);
}

///////////////////////////////////////////////
tBool cUIContext::IsFingerDragging(tU32 anFinger) const
{
  CHECK_FINGER(anFinger,eFalse);
  const sFinger& f = GET_FINGER(anFinger);
  QPtr<cWidget> dragWidget(f.mpDragWidget);
  return dragWidget.IsOK();
}

///////////////////////////////////////////////
tBool cUIContext::IsFingerDrag(tU32 anFinger, const iWidget* apWidget) const
{
  CHECK_FINGER(anFinger,eFalse);
  const sFinger& f = GET_FINGER(anFinger);
  QPtr<cWidget> dragWidget(f.mpDragWidget);
  return dragWidget == apWidget;
}

///////////////////////////////////////////////
tBool cUIContext::_CanSendFingerDragMessage(tU32 anFinger) {
  CHECK_FINGER(anFinger,eFalse);
  sFinger& f = GET_FINGER(anFinger);
  if (f.mnDragTimeMs == eInvalidHandle)
    return eTrue;
  tU32 time = ((tU32)(ni::TimerInSeconds()*1000)) - f.mnDragTimeMs;
  if (time > 200) {
    f.mnDragTimeMs = eInvalidHandle;
    return eTrue;
  }
  return eFalse;
}
tBool cUIContext::_CanSendFingerDropMessage(tU32 anFinger) {
  CHECK_FINGER(anFinger,eFalse);
  sFinger& f = GET_FINGER(anFinger);
  return f.mnDragTimeMs == eInvalidHandle;
}

///////////////////////////////////////////////
void __stdcall cUIContext::SetDragFingerStartDistance(tU32 anFinger, tU32 anPixelDistance) {
  if (anFinger == eInvalidHandle) {
    niLoop(i,knNumFingers) {
      mFingers[i].mnDragStartDistance = anPixelDistance;
    }
  }
  else {
    CHECK_FINGER(anFinger,;);
    GET_FINGER(anFinger).mnDragStartDistance = anPixelDistance;
  }
}
tU32 __stdcall cUIContext::GetDragFingerStartDistance(tU32 anFinger) const {
  CHECK_FINGER(anFinger,0);
  return GET_FINGER(anFinger).mnDragStartDistance;
}
