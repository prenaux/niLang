#ifndef __WIDGETSINKIMPL_58007776_H__
#define __WIDGETSINKIMPL_58007776_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../UIMessages.h"
#include "../IWidget.h"
#include "../IUIContext.h"
#include <niLang/IGameCtrl.h>
#include <niLang/IDataTable.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

template <typename T=cUnknown1>
class cWidgetSinkImpl : public ni::ImplRC<ni::iWidgetSink,ni::eImplFlags_Default,T>
{
 public:
  typedef cWidgetSinkImpl tWidgetBaseImpl;
  typedef ni::ImplRC<ni::iWidgetSink,ni::eImplFlags_Default,T> BaseImpl;

  iWidget* mpWidget;

  cWidgetSinkImpl() : mpWidget(NULL) {}

  virtual iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    if (aIID == niGetInterfaceUUID(iWidget)) {
      return mpWidget;
    }
    return BaseImpl::QueryInterface(aIID);
  }
  virtual void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const {
    apLst->Add(niGetInterfaceUUID(iWidget));
    BaseImpl::ListInterfaces(apLst,anFlags);
  }

  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 anMsg, const Var& avarA, const Var& avarB)
  {
    switch (anMsg) {
      case eUIMessage_SinkAttached:   mpWidget = apWidget; return OnSinkAttached();
      case eUIMessage_SinkDetached:   return OnSinkDetached();
      case eUIMessage_Destroy:      return OnDestroy();
      case eUIMessage_NCPaint: {
        iCanvas* c = ni::VarQueryInterface<ni::iCanvas>(avarB);
        if (c) return OnNCPaint(*(sVec2f*)avarA.mV2F, c);
        return eTrue;
      }
      case eUIMessage_Paint: {
        iCanvas* c = ni::VarQueryInterface<ni::iCanvas>(avarB);
        if (c) return OnPaint(*(sVec2f*)avarA.mV2F, c);
        return eTrue;
      }
      case eUIMessage_NCSize:       return OnNCSize(*(sVec2f*)avarA.mV2F);
      case eUIMessage_Size:       return OnSize(*(sVec2f*)avarA.mV2F);
      case eUIMessage_Notify:       return OnNotify(ni::VarQueryInterface<ni::iWidget>(avarA),ni::VarQueryInterface<ni::iMessageDesc>(avarB));
      case eUIMessage_NCLeftClickDown:  return OnNCLeftClickDown(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_LeftClickDown:    return OnLeftClickDown(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCLeftClickUp:    return OnNCLeftClickUp(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_LeftClickUp:    return OnLeftClickUp(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCRightClickDown: return OnNCRightClickDown(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_RightClickDown:   return OnRightClickDown(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCRightClickUp:   return OnNCRightClickUp(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_RightClickUp:   return OnRightClickUp(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCMouseMove:    return OnNCMouseMove(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_MouseMove:      return OnMouseMove(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCWheel:      return OnNCWheel((tF32)avarA.GetFloatValue(),*(sVec2f*)avarB.mV2F);
      case eUIMessage_Wheel:        return OnWheel((tF32)avarA.GetFloatValue(),*(sVec2f*)avarB.mV2F);
      case eUIMessage_KeyChar:        return OnKeyChar(avarA.mU32);
      case eUIMessage_KeyDown:        return OnKeyDown((eKey)avarA.mU32,avarB.mU32);
      case eUIMessage_KeyUp:            return OnKeyUp((eKey)avarA.mU32,avarB.mU32);
      case eUIMessage_GameCtrl:       return OnGameCtrl(ni::VarQueryInterface<ni::iGameCtrl>(avarA));
      case eUIMessage_GameCtrlButtonDown:       return OnGameCtrlButtonDown(ni::VarQueryInterface<ni::iGameCtrl>(avarA),avarB.mU32);
      case eUIMessage_GameCtrlButtonUp:       return OnGameCtrlButtonUp(ni::VarQueryInterface<ni::iGameCtrl>(avarA),avarB.mU32);
      case eUIMessage_Submit:       return OnSubmit(ni::VarQueryInterface<ni::iWidget>(avarA),ni::VarQueryInterface<ni::iMessageDesc>(avarB));
      case eUIMessage_SetCursor:      return OnSetCursor(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_SetFocus:     return OnSetFocus();
      case eUIMessage_LostFocus:      return OnLostFocus();
      case eUIMessage_Activate:     return OnActivate();
      case eUIMessage_Deactivate:     return OnDeactivate();
      case eUIMessage_Command:      return OnCommand(ni::VarQueryInterface<ni::iWidgetCommand>(avarA));
      case eUIMessage_Timer:        return OnTimer((tU32)avarA.GetIntValue(),avarB.GetFloatValue());
      case eUIMessage_Enabled:      return OnEnabled(avarA.mBool);
      case eUIMessage_Visible:      return OnVisible(avarA.mBool);
      case eUIMessage_Layout:       return OnLayout();
      case eUIMessage_StyleChanged:   return OnStyleChanged(avarA.mU32);
      case eUIMessage_FontChanged:    return OnFontChanged();
      case eUIMessage_TextChanged:    return OnTextChanged();
      case eUIMessage_SetText:      return OnSetText(avarA.mBool);
      case eUIMessage_NCLeftClick:    return OnNCLeftClick(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_LeftClick:      return OnLeftClick(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCLeftDoubleClick:  return OnNCLeftDoubleClick(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_LeftDoubleClick:  return OnLeftDoubleClick(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCRightClick:   return OnNCRightClick(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_RightClick:     return OnRightClick(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCRightDoubleClick: return OnNCRightDoubleClick(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_RightDoubleClick: return OnRightDoubleClick(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_ChildAdded:     return OnChildAdded(ni::VarQueryInterface<ni::iWidget>(avarA));
      case eUIMessage_ChildRemoved:   return OnChildRemoved(ni::VarQueryInterface<ni::iWidget>(avarA));
      case eUIMessage_Padding:      return OnPadding();
      case eUIMessage_Border:       return OnBorder();
      case eUIMessage_BorderStyle:    return OnBorderStyle();
      case eUIMessage_Margin:       return OnMargin();
      case eUIMessage_MarginMerge:    return OnMarginMerge();
      case eUIMessage_BeforeDocked:   return OnBeforeDocked();
      case eUIMessage_AfterDocked:    return OnAfterDocked(ni::VarQueryInterface<ni::iWidget>(avarA));
      case eUIMessage_Undocked:       return OnUndocked();
      case eUIMessage_SerializeLayout:  return OnSerializeLayout(ni::VarQueryInterface<ni::iDataTable>(avarA),avarB.mU32);
      case eUIMessage_SerializeWidget:
        {
          if (niFlagIs(avarB.mU32,eWidgetSerializeFlags_Read)) {
            ni::Nonnull<ni::iDataTableReadStack> dt {ni::CreateDataTableReadStack(ni::VarQueryInterface<ni::iDataTable>(avarA))};
            return OnSerializeReadWidget(dt,avarB.mU32);
          }
          else if (niFlagIs(avarB.mU32,eWidgetSerializeFlags_Write)) {
            ni::Nonnull<ni::iDataTableWriteStack> dt {ni::CreateDataTableWriteStack(ni::VarQueryInterface<ni::iDataTable>(avarA))};
            return OnSerializeWriteWidget(dt,avarB.mU32);
          }
          niAssertUnreachable("Neither read nor write flag set");
          break;
        }
      case eUIMessage_SerializeChildren:  return OnSerializeChildren(ni::VarQueryInterface<ni::iDataTable>(avarA),avarB.mU32);
      case eUIMessage_SerializeFinalize:  return OnSerializeFinalize(ni::VarQueryInterface<ni::iDataTable>(avarA),avarB.mU32);
      case eUIMessage_ContextMenu:    return OnContextMenu(avarA.mBool,*(sVec2f*)avarB.mV2F);
      case eUIMessage_ContextHelp:    return OnContextHelp();
      case eUIMessage_Copy:       return OnCopy(ni::VarQueryInterface<ni::iDataTable>(avarA));
      case eUIMessage_Cut:        return OnCut(ni::VarQueryInterface<ni::iDataTable>(avarA));
      case eUIMessage_Paste:        return OnPaste(ni::VarQueryInterface<ni::iDataTable>(avarA));
      case eUIMessage_Undo:       return OnUndo();
      case eUIMessage_Redo:       return OnRedo();
      case eUIMessage_Cancel:       return OnCancel();
      case eUIMessage_NCMouseEnter:   return OnNCMouseEnter(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_MouseEnter:     return OnMouseEnter(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCMouseLeave:   return OnNCMouseLeave(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_MouseLeave:     return OnMouseLeave(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_NCMouseHover:   return OnNCMouseHover(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_MouseHover:     return OnMouseHover(*(sVec2f*)avarA.mV2F,*(sVec2f*)avarB.mV2F);
      case eUIMessage_SkinChanged:    return OnSkinChanged();
      case eUIMessage_ContextMenuChanged:   return OnContextMenuChanged();
      case eUIMessage_HoverTextChanged:   return OnHoverTextChanged();
      case eUIMessage_MoveFocus:    return OnMoveFocus(avarA.mBool);
    }
    return OnDefault(anMsg,avarA,avarB);
  }

  virtual tBool __stdcall OnDefault(tU32 anMsg, const Var& avarA, const Var& avarB) { return eFalse; }
  virtual tBool __stdcall OnSinkAttached() { return eFalse; }
  virtual tBool __stdcall OnSinkDetached() { return eFalse; }
  virtual tBool __stdcall OnDestroy() { return eFalse; }
  virtual tBool __stdcall OnNCPaint(const sVec2f& avNCMousePos, iCanvas* apCanvas) { return eFalse; }
  virtual tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) { return eFalse; }
  virtual tBool __stdcall OnNCSize(const sVec2f& avNewSize) { return eFalse; }
  virtual tBool __stdcall OnSize(const sVec2f& avNewSize) { return eFalse; }
  virtual tBool __stdcall OnNotify(iWidget* apNotifiedWidget, iMessageDesc* apMsg) { return eFalse; }
  virtual tBool __stdcall OnNCLeftClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnLeftClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCLeftClickUp(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnLeftClickUp(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCRightClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnRightClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCRightClickUp(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnRightClickUp(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCMouseMove(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnMouseMove(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCMouseEnter(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnMouseEnter(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCMouseLeave(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnMouseLeave(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCMouseHover(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnMouseHover(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCWheel(tF32 afWheel, const sVec2f& avAbsMousePos) { return eFalse; }
  virtual tBool __stdcall OnWheel(tF32 afWheel, const sVec2f& avAbsMousePos) { return eFalse; }
  virtual tBool __stdcall OnKeyChar(tU32 ch)   { return eFalse; }
  virtual tBool __stdcall OnKeyDown(eKey aKey, tU32 aKeyMod) { return eFalse; }
  virtual tBool __stdcall OnKeyUp(eKey aKey, tU32 aKeyMod)   { return eFalse; }
  virtual tBool __stdcall OnGameCtrl(iGameCtrl* apGameCtrl) { return ni::eFalse; }
  virtual tBool __stdcall OnGameCtrlButtonDown(iGameCtrl* apGameCtrl, tU32 anButton) { return ni::eFalse; }
  virtual tBool __stdcall OnGameCtrlButtonUp(iGameCtrl* apGameCtrl, tU32 anButton) { return ni::eFalse; }
  virtual tBool __stdcall OnSubmit(iWidget* apSender, iMessageDesc* apMsg) { return eFalse; }
  virtual tBool __stdcall OnSetCursor(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnSetFocus() { return eFalse; }
  virtual tBool __stdcall OnLostFocus() { return eFalse; }
  virtual tBool __stdcall OnActivate() { return eFalse; }
  virtual tBool __stdcall OnDeactivate() { return eFalse; }
  virtual tBool __stdcall OnCommand(iWidgetCommand* apCmd) { return eFalse; }
  virtual tBool __stdcall OnTimer(tU32 anID, tF64 afDuration) { return eFalse; }
  virtual tBool __stdcall OnEnabled(tBool abEnabled) { return eFalse; }
  virtual tBool __stdcall OnVisible(tBool abVisible) { return eFalse; }
  virtual tBool __stdcall OnLayout() { return eFalse; }
  virtual tBool __stdcall OnStyleChanged(tWidgetStyleFlags aPrevStyle) { return eFalse; }
  virtual tBool __stdcall OnFontChanged() { return eFalse; }
  virtual tBool __stdcall OnTextChanged() { return eFalse; }
  virtual tBool __stdcall OnSetText(tBool abInSerialize) { return eFalse; }
  virtual tBool __stdcall OnNCLeftClick(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnLeftClick(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCLeftDoubleClick(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnLeftDoubleClick(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCRightClick(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnRightClick(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnNCRightDoubleClick(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnRightDoubleClick(const sVec2f& avMousePos, const sVec2f& avNCMousePos) { return eFalse; }
  virtual tBool __stdcall OnChildAdded(iWidget* apChild) { return eFalse; }
  virtual tBool __stdcall OnChildRemoved(iWidget* apChild) { return eFalse; }
  virtual tBool __stdcall OnPadding() { return eFalse; }
  virtual tBool __stdcall OnBorder() { return eFalse; }
  virtual tBool __stdcall OnBorderStyle() { return eFalse; }
  virtual tBool __stdcall OnMargin() { return eFalse; }
  virtual tBool __stdcall OnMarginMerge() { return eFalse; }
  virtual tBool __stdcall OnBeforeDocked() { return eFalse; }
  virtual tBool __stdcall OnAfterDocked(iWidget* apTabWidget) { return eFalse; }
  virtual tBool __stdcall OnUndocked() { return eFalse; }
  virtual tBool __stdcall OnSerializeLayout(iDataTable* apDT, tWidgetSerializeFlags aFlags) { return eFalse; }
  virtual tBool __stdcall OnSerializeReadWidget(iDataTableReadStack* apDT, tWidgetSerializeFlags aFlags) { return eFalse; }
  virtual tBool __stdcall OnSerializeWriteWidget(iDataTableWriteStack* apDT, tWidgetSerializeFlags aFlags) { return eFalse; }
  virtual tBool __stdcall OnSerializeChildren(iDataTable* apDT, tWidgetSerializeFlags aFlags) { return eFalse; }
  virtual tBool __stdcall OnSerializeFinalize(iDataTable* apDT, tWidgetSerializeFlags aFlags) { return eFalse; }
  virtual tBool __stdcall OnContextMenu(tBool abCursor, const sVec2f& avMousePos) { return eFalse; }
  virtual tBool __stdcall OnContextHelp() { return eFalse; }
  virtual tBool __stdcall OnCopy(ni::iDataTable* apDT) { return eFalse; }
  virtual tBool __stdcall OnCut(ni::iDataTable* apDT) { return eFalse; }
  virtual tBool __stdcall OnPaste(ni::iDataTable* apDT) { return eFalse; }
  virtual tBool __stdcall OnUndo() { return eFalse; }
  virtual tBool __stdcall OnRedo() { return eFalse; }
  virtual tBool __stdcall OnCancel() { return eFalse; }
  virtual tBool __stdcall OnSkinChanged() { return eFalse; }
  virtual tBool __stdcall OnContextMenuChanged() { return eFalse; }
  virtual tBool __stdcall OnHoverTextChanged() { return eFalse; }
  virtual tBool __stdcall OnMoveFocus(tBool abPrev) { return eFalse; }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __WIDGETSINKIMPL_58007776_H__
