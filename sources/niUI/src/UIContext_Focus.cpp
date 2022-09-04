// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "UIContext.h"
#include "niUI_HString.h"

static iWidget* _GetFocusActivateWidget(iWidget* apWidget) {
  if (niFlagIs(apWidget->GetStyle(),eWidgetStyle_FocusActivate))
    return apWidget;
  if (apWidget->GetParent()) {
    return _GetFocusActivateWidget(apWidget->GetParent());
  }
  return NULL;
}

static cWidget* _GetFocusTarget(cWidget* apWidget) {
  niAssert(apWidget);
  if (niFlagIs(apWidget->GetStyle(),eWidgetStyle_HoldFocus) || !(apWidget->GetParent()))
    return apWidget;
  return _GetFocusTarget(static_cast<cWidget*>(apWidget->GetParent()));
}

///////////////////////////////////////////////
void cUIContext::SetTopWidget(cWidget* apWidget)
{
  if (!niIsOK(apWidget))
    apWidget = mpwRootWidget;
  {
    QPtr<cWidget> ptrTopWidget(mpwTopWidget);
    if (ptrTopWidget == apWidget) {
      _UIFocusTrace(niFmt("ALREADY TOP WIDGET: Widget %p (ID:%s) is the top widget.",
                          (tIntPtr)apWidget,niHStr(apWidget->GetID())));
      return;
    }
  }
  mpwTopWidget = apWidget;
  UpdateFreeWidgets();
  _UIFocusTrace(niFmt("TOP WIDGET: Widget %p (ID:%s) is the top widget.",
                      (tIntPtr)apWidget,apWidget?niHStr(apWidget->GetID()):""));
}

///////////////////////////////////////////////
tBool cUIContext::SetFocusInput(cWidget* apNewTarget, tBool abMouseClick)
{
  QPtr<cWidget> ptrOldTarget(mpwFocusInput);
  if (apNewTarget == ptrOldTarget)
    return eTrue;

  {
    QPtr<cWidget> ptrTopWidget(mpwTopWidget);
    if (ptrTopWidget.IsOK() &&
        apNewTarget &&
        ptrTopWidget.ptr() != mpwRootWidget.ptr() &&
        ptrTopWidget.ptr() != apNewTarget &&
        !ptrTopWidget->HasChild(apNewTarget,eTrue))
    {
      _UIFocusTrace(
          niFmt(_A("### UICONTEXT-INPUT: REFUSED To %p (ID:%s), because not a child of the top widget %p (ID:%s)."),
                (tIntPtr)apNewTarget,
                apNewTarget ? niHStr(apNewTarget->GetID()) : AZEROSTR,
                (tIntPtr)ptrTopWidget.ptr(),
                ptrTopWidget->GetID()));
      return eFalse;
    }
  }

  Ptr<cWidget> ptrNewTarget = _GetFocusTarget(apNewTarget);
  if (ptrOldTarget.ptr() != ptrNewTarget.ptr()) {
    mpwFocusInput = ptrNewTarget;

    _UIFocusTrace(niFmt(_A("### UICONTEXT-INPUT: GIVE FOCUS TO: %p (ID:%s)."),
                        (tIntPtr)ptrNewTarget.ptr(),
                        ptrNewTarget.IsOK() ? niHStr(ptrNewTarget->GetID()) : AZEROSTR));

    // update the active widget
    {
      if (ptrNewTarget.IsOK()) {
        Ptr<iWidget> activateWidget = _GetFocusActivateWidget(ptrNewTarget);
        if (activateWidget.IsOK()) {
          // dont change the active widget until we find
          // another active widget to give the active status
          // to
          SetActiveWidget(activateWidget);
        }
      }
    }

    // send the focus messages
    if (ptrOldTarget.IsOK()) {
      ptrOldTarget->SendMessage(eUIMessage_LostFocus,
                                abMouseClick,
                                ptrNewTarget.ptr());
      _RedrawWidget(ptrOldTarget);
    }
    if (ptrNewTarget.IsOK()) {
      ptrNewTarget->SendMessage(eUIMessage_SetFocus,
                                abMouseClick,
                                ptrOldTarget.ptr());
      _RedrawWidget(ptrNewTarget);
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
struct sMoveFocusCollect {
  Ptr<iWidget> w;
};
struct _MoveFocusSort {
  static bool leftToRight(const sMoveFocusCollect& a, const sMoveFocusCollect& b) {
    return a.w->GetAbsolutePosition().x < b.w->GetAbsolutePosition().x;
  }
  static bool topToBottom(const sMoveFocusCollect& a, const sMoveFocusCollect& b) {
    return a.w->GetAbsolutePosition().y < b.w->GetAbsolutePosition().y;
  }
};
static tBool _MoveFocusCollectGroups(iWidget* w, astl::list<sMoveFocusCollect>& aLst)
{
  niLoop(i,w->GetNumChildren()) {
    Ptr<iWidget> dw = w->GetChildFromIndex(i);
    if (dw.IsOK()) {
      if (!dw->GetVisible() || !dw->GetEnabled())
        continue;
      if (dw->GetStyle()&eWidgetStyle_MoveFocusGroup) {
        sMoveFocusCollect mvc;
        mvc.w = dw;
        aLst.push_back(mvc);
      }
      if (!_MoveFocusCollectGroups(dw,aLst))
        return eFalse;
    }
  }
  return eTrue;
}
static tBool _MoveFocusCollect(iWidget* w, cWidget* apPivot, astl::list<sMoveFocusCollect>& aLst)
{
  niLoop(i,w->GetNumChildren()) {
    Ptr<iWidget> dw = w->GetChildFromIndex(i);
    if (dw.IsOK()) {
      if (!dw->GetVisible() || !dw->GetEnabled())
        continue;
      if (dw->GetStyle()&eWidgetStyle_MoveFocusGroup) {
        // each move focus group creates a boundary and will be processed separately
        continue;
      }
      if (dw == apPivot ||
          ((dw->GetStyle()&eWidgetStyle_HoldFocus) &&
           !(dw->GetStyle()&eWidgetStyle_NoMoveFocus) &&
           !dw->GetIgnoreInput()))
      {
        sMoveFocusCollect mvc;
        mvc.w = dw;
        aLst.push_back(mvc);
      }
      if (!_MoveFocusCollect(dw,apPivot,aLst))
        return eFalse;
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool cUIContext::MoveFocus(cWidget* apWidget, tBool abPrev) {
  if (!apWidget)
    return eFalse;

  Ptr<cWidget> p = apWidget;
  while (p.IsOK()) {
    cWidget* parent = (cWidget*)p->GetParent();
    if (!parent)
      return eFalse; // no form parent, return...
    p = parent;
    if (p->GetClassName() == _HC(Form) ||
        p->GetClassName() == _HC(Canvas))
      break;
  }

  astl::list<sMoveFocusCollect> groups;
  if (!_MoveFocusCollectGroups(p,groups)) {
    return eFalse;
  }
  if (groups.empty()) {
    sMoveFocusCollect mvc;
    mvc.w = p;
    groups.push_back(mvc);
  }
  groups.sort(_MoveFocusSort::leftToRight);
  groups.sort(_MoveFocusSort::topToBottom);

  astl::list<sMoveFocusCollect> lst;
  for (astl::list<sMoveFocusCollect>::iterator itG = groups.begin(); itG != groups.end(); ++itG)
  {
    astl::list<sMoveFocusCollect> tmpLst;
    if (!_MoveFocusCollect(itG->w,apWidget,tmpLst)) {
      return eFalse;
    }
    tmpLst.sort(_MoveFocusSort::leftToRight);
    tmpLst.sort(_MoveFocusSort::topToBottom);
    lst.insert(lst.end(),tmpLst.begin(),tmpLst.end());
  }

  if (lst.size() >= 2) {
    Ptr<iWidget> prev = lst.rbegin()->w;
    Ptr<iWidget> next = NULL;
    astl::list<sMoveFocusCollect>::iterator it = lst.begin();
    for(;;) {
      Ptr<iWidget> pivot = it->w;
      ++it;
      if (it == lst.end()) {
        next = lst.begin()->w;
      }
      else {
        next = it->w;
      }
      if (pivot == apWidget) {
        niAssert(next.IsOK());
        niAssert(prev.IsOK());
        if (abPrev) {
          _UIFocusTrace((niFmt(_A("## PREV FOCUS: %s (%s-%s)\n"),
                               niHStr(next->GetID()),
                               niHStr(next->GetClassName()),
                               niHStr(next->GetText())
                         )));
          prev->SetFocus();
        }
        else {
          _UIFocusTrace((niFmt(_A("## NEXT FOCUS: %s (%s-%s)\n"),
                               niHStr(next->GetID()),
                               niHStr(next->GetClassName()),
                               niHStr(next->GetText())
                         )));
          next->SetFocus();
        }
        return eTrue;
      }
      if (it == lst.end()) {
        break;
      }
      prev = pivot;
    };
  }

  return eFalse;
}


///////////////////////////////////////////////
static void _ApplyExclusiveToTopWidget(cUIContext* apContext) {
  QPtr<cWidget> ptrTopWidget(apContext->mpwTopWidget);
  if (ptrTopWidget.IsOK()) {
    _UIFocusTrace(niFmt(_A("### UICONTEXT _ApplyExclusiveToTopWidget: %p (ID:%s)."),(tIntPtr)ptrTopWidget.ptr(),ptrTopWidget->GetID()));
    apContext->SetFocusInput(ptrTopWidget,eFalse);
    ptrTopWidget->SetZOrder(eWidgetZOrder_TopMost);
    ptrTopWidget->_CheckCaptureAndExclusive();
  }
}

void cUIContext::_AddExclusive(cWidget *widget) {
  niAssert(widget->GetVisible() && widget->GetEnabled());
  _RemoveExclusive(widget,eTrue);
  mlstExclusiveStack.push_back(widget);
  SetTopWidget(widget);
  _UIFocusTrace(niFmt("EXCLUSIVE: Widget %p (ID:%s) is exclusive.",(tIntPtr)widget,niHStr(widget->GetID())));
  _ApplyExclusiveToTopWidget(this);
}
void cUIContext::_RemoveExclusive(cWidget *widget, tBool abLost) {
  if (!mlstExclusiveStack.empty() && astl::find_erase(mlstExclusiveStack,WeakPtr<cWidget>(widget))) {
    _UIFocusTrace(niFmt("EXCLUSIVE: Widget %p (ID:%s) is not exclusive anymore.",
                        (tIntPtr)widget,niHStr(widget->GetID())));

    tBool isTopWidget;
    {
      QPtr<cWidget> ptrTopWidget(mpwTopWidget);
      isTopWidget = (ptrTopWidget == widget);
    }
    if (isTopWidget) {
      if (mlstExclusiveStack.empty()) {
        SetTopWidget(mpwRootWidget);
      }
      else {
        QPtr<cWidget> ptrExclusive(mlstExclusiveStack.back());
        SetTopWidget(ptrExclusive);
      }
      _ApplyExclusiveToTopWidget(this);
    }
  }
}
tBool cUIContext::_IsExclusive(const cWidget *widget) const {
  if (mlstExclusiveStack.empty()) return eFalse;
  return astl::find(mlstExclusiveStack,WeakPtr<cWidget>(widget)) !=
      mlstExclusiveStack.end();
}

///////////////////////////////////////////////
void cUIContext::_AddCaptureAll(cWidget *widget) {
  niAssert(widget->GetVisible() && widget->GetEnabled());
  _RemoveCaptureAll(widget,eTrue);
  mlstCaptureAllStack.push_back(widget);
  mpwCaptureAll = widget;
  widget->SetFocus();
  _UIFocusTrace(("## _AddCaptureAll: %s", niHStr(widget->GetID())));
  _WindowUpdateCursorStates();
}
void cUIContext::_RemoveCaptureAll(cWidget *widget, tBool abLost) {
  if (!mlstCaptureAllStack.empty()) {
    astl::find_erase(mlstCaptureAllStack,WeakPtr<cWidget>(widget));
    {
      QPtr<cWidget> ptrCaptureAll(mpwCaptureAll);
      if (ptrCaptureAll == widget) {
        if (mlstCaptureAllStack.empty()) {
          mpwCaptureAll.SetNull();
        }
        else {
          mpwCaptureAll = mlstCaptureAllStack.back();
        }
        _WindowUpdateCursorStates();
      }
    }
    {
      QPtr<cWidget> ptrCaptureAll(mpwCaptureAll);
      if (ptrCaptureAll.IsOK()) {
        ptrCaptureAll->_CheckCaptureAndExclusive();
      }
    }
  }
}
tBool cUIContext::_IsCaptureAll(const cWidget *widget) const {
  if (mlstCaptureAllStack.empty()) return eFalse;
  return astl::find(mlstCaptureAllStack,WeakPtr<cWidget>(widget)) !=
      mlstCaptureAllStack.end();
}

///////////////////////////////////////////////
void cUIContext::_AddFingerCapture(tU32 anFinger, cWidget *widget) {
  niAssert(widget->GetVisible() && widget->GetEnabled());
  CHECK_FINGER(anFinger,;);
  sFinger& f = GET_FINGER(anFinger);
  _RemoveFingerCapture(anFinger,widget,eTrue);
  f.mlstCaptureStack.push_back(widget);
  f.mpwCapture = widget;
  _FingerMove(anFinger,f.mvPosition,eFalse);
  //  niDebugFmt((_A("Widget %p (ID:%s, Class:%s) add finger capture.\n"),widget,niHStr(widget->GetID()),niHStr(widget->GetClassName())));
}
void cUIContext::_RemoveFingerCapture(tU32 anFinger, cWidget *widget, tBool abLost) {
  CHECK_FINGER(anFinger,;);
  sFinger& f = GET_FINGER(anFinger);
  if (!f.mlstCaptureStack.empty()) {
    astl::find_erase(f.mlstCaptureStack,WeakPtr<cWidget>(widget));
    {
      QPtr<cWidget> ptrCapture(f.mpwCapture);
      if (ptrCapture == widget) {
        if (f.mlstCaptureStack.empty()) {
          f.mpwCapture.SetNull();
        }
        else {
          f.mpwCapture = f.mlstCaptureStack.back();
        }
        _FingerMove(anFinger,f.mvPosition,eFalse);
      }
    }
    {
      QPtr<cWidget> ptrCapture(f.mpwCapture);
      if (ptrCapture.IsOK()) {
        ptrCapture->_CheckCaptureAndExclusive();
      }
    }
    //    niDebugFmt((_A("Widget %p (ID:%s, Class:%s) remove capture.\n"),widget,niHStr(widget->GetID()),niHStr(widget->GetClassName())));
  }
}
tBool cUIContext::_IsFingerCapture(tU32 anFinger, const cWidget *widget) const {
  CHECK_FINGER(anFinger,eFalse);
  const sFinger& f = GET_FINGER(anFinger);
  if (f.mlstCaptureStack.empty()) return eFalse;
  return astl::find(f.mlstCaptureStack,WeakPtr<cWidget>(widget)) !=
      f.mlstCaptureStack.end();
}
