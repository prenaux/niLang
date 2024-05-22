// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetListBox.h"
#include "HardCodedSkin.h"
#include <niLang/STL/sort.h>
#include <niLang/Utils/UnitSnap.h>

#define LB_SEP_W 2
#define LB_TEXT_X_OFFSET 1

#define SEL_SINGLE  0
#define SEL_RANGE 1
#define SEL_ADD   2

static Ptr<iFont> SerializeReadFont(iUIContext* apCtx, iDataTable* apDT) {
  Ptr<iFont> font = apCtx->GetGraphics()->LoadFont(apDT->GetHStringDefault("font", _H("default")));
  font->SetColor(ULColorBuild(apDT->GetVec4Default("font_color", sColor4f::White())));

  tF32 fontSize = Clamp(apDT->GetFloatDefault("font_size", 24), 4, 256);
  font->SetSizeAndResolution(
    Vec2f(fontSize, fontSize),
    Clamp(apDT->GetIntDefault("font_res", 64), 4, 64),
    apCtx->GetContentsScale());
  font->SetFiltering(eTrue);
  font->SetBlendMode(ni::eBlendMode_Translucent);
  return font;
}

static void SerializeWriteFont(iFont* apFont, iDataTable* apDT, tBool hasMetadata) {
  apDT->SetString("font", apFont->GetName()->GetChars());
  apDT->SetInt("font_res", apFont->GetResolution());
  apDT->SetFloat("font_size", apFont->GetSize().x);
  apDT->SetVec4("font_color", ULColorToVec4f(apFont->GetColor()));

  if (hasMetadata) {
    apDT->SetMetadata("font_res", _H("irange[min=4,max=64,step=1]"));
    apDT->SetMetadata("font_size", _H("range[min=4,max=256,step=1]"));
    apDT->SetMetadata("font_color", _H("color"));
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetListBox implementation.

///////////////////////////////////////////////
cWidgetListBox::cWidgetListBox(iWidget *apWidget)
{
  ZeroMembers();
  mpWidget = apWidget;
  mpSpringScroll = mpWidget->GetGraphics()->CreateDampedSpringPosition1(100, 0);
  mpSpringScroll->SetKs(500);
  mpSpringScroll->SetDampingRatio(1);
  mpSpringScroll->SetEndThreshold(0.01);

  // vertical scroll bar
  mptrVtScroll = mpWidget->GetUIContext()->CreateWidget(_A("ScrollBar"),mpWidget,sRectf(),eWidgetStyle_DontSerialize|eWidgetStyle_NCRelative);
  QPtr<iWidgetScrollBar>(mptrVtScroll)->SetScrollPosition(0);
  mptrVtScroll->SetVisible(eFalse);
  mptrVtScroll->SetEnabled(eFalse);
  // horizontal scroll bar
  mptrHzScroll = mpWidget->GetUIContext()->CreateWidget(_A("ScrollBar"),mpWidget,sRectf(),eWidgetStyle_DontSerialize|eWidgetStyle_NoClip|eWidgetStyle_NCRelative|eWidgetScrollBarStyle_Horz);
  QPtr<iWidgetScrollBar>(mptrHzScroll)->SetScrollPosition(0);
  mptrHzScroll->SetVisible(eFalse);
  mptrHzScroll->SetEnabled(eFalse);
  InitSkin();
  AddColumn("Items", 0);
  niAssert(GetNumColumns() == 1);
  UpdateLayout();
}

///////////////////////////////////////////////
cWidgetListBox::~cWidgetListBox()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cWidgetListBox::ZeroMembers()
{
  mpWidget = NULL;
  mnSortKey = eInvalidHandle;
  mfWidestItem = 0.0f;
  mfRealW = 0.0f;
  mfRealH = 0.0f;
  mnColumns = 0;
  mnHeaderHandlePressed = eInvalidHandle;
  mnBaseSize = 0;
  mvHeaderPivot = sVec2f::Zero();
  mrectHeader = sRectf(0,0);
  mfItemHeight = 0;
  mnInputSelected = eInvalidHandle;
  mbSortType = eTrue;
  mnMaxNumItems = 0;
  ni::MemZero((tPtr)&mInternalFlags,sizeof(mInternalFlags));
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetListBox::IsOK() const
{
  niClassIsOK(cWidgetListBox);
  return ni::eTrue;
}


///////////////////////////////////////////////
void cWidgetListBox::AddColumn(const achar *aaszName, tU32 anSize)
{
  sColumn col;
  col.hspName = _H(aaszName);
  col.nSetSize = anSize;
  mvColumns.push_back(col);
  niLoop(i,mvItems.size()) {
    sItem& item = *mvItems[i];
    item.vData.push_back();
  }
  UpdateLayout();
}

///////////////////////////////////////////////
tBool __stdcall cWidgetListBox::SetColumn(tU32 anColumn, const achar* aaszName, tU32 anSize) {
  if (anColumn >= GetNumColumns())
    return eFalse;
  mvColumns[anColumn].hspName = _H(aaszName);
  mvColumns[anColumn].nSetSize = anSize;
  UpdateLayout();
  return eTrue;
}

///////////////////////////////////////////////
tBool cWidgetListBox::RemoveColumn(tU32 anColumn)
{
  const tU32 numCols = GetNumColumns();
  if (numCols < 0 || anColumn >= numCols)
    return eFalse;
  mvColumns.erase(mvColumns.begin()+anColumn);

  // niLoop(i,mvItems.size()) {
    // sItem& item = *mvItems[i];
    // item.InvalidateColumn(anColumn);
    // item.vData.erase(item.vData.begin()+anColumn);
  // }

  // it may sort an invalid column after the column is removed, so we reset it
  mnSortKey = eInvalidHandle;

  UpdateLayout();
  return eTrue;
}

///////////////////////////////////////////////
void cWidgetListBox::ClearColumns()
{
  tI32 n = GetNumColumns() - 1;
  while (n >= 0) {
    RemoveColumn(n);
    --n;
  }
}

///////////////////////////////////////////////
tU32 cWidgetListBox::GetNumColumns() const
{
  return mvColumns.size();
}

///////////////////////////////////////////////
tBool cWidgetListBox::SetColumnName(tU32 anColumn, const achar *aaszName)
{
  if (anColumn >= GetNumColumns()) return eFalse;
  mvColumns[anColumn].hspName = _H(aaszName);
  return eTrue;
}

///////////////////////////////////////////////
const achar * cWidgetListBox::GetColumnName(tU32 anColumn) const
{
  if (anColumn >= GetNumColumns()) return AZEROSTR;
  return niHStr(mvColumns[anColumn].hspName);
}

///////////////////////////////////////////////
tBool cWidgetListBox::SetColumnWidth(tU32 anColumn, tU32 anWidth)
{
  if (anColumn >= GetNumColumns()) return eFalse;
  mvColumns[anColumn].nSetSize = anWidth;
  UpdateLayout();
  return eTrue;
}

///////////////////////////////////////////////
tU32 cWidgetListBox::GetColumnWidth(tU32 anColumn) const
{
  if (anColumn >= GetNumColumns()) return eInvalidHandle;
  return mvColumns[anColumn].nSetSize; // return the set size here, not the computed width
}

///////////////////////////////////////////////
tU32 cWidgetListBox::GetNumItems() const
{
  return mvItems.size();
}

///////////////////////////////////////////////
void cWidgetListBox::ClearItems()
{
  ClearSelection();
  for (tU32 i = 0; i < mvItems.size(); ++i) {
    mvItems[i]->Invalidate();
  }
  mvItems.clear();
  UpdateLayout();
}

///////////////////////////////////////////////
void cWidgetListBox::_RemoveOverflowingItems() {
  if (mnMaxNumItems > 0) {
    while (mvItems.size() >= mnMaxNumItems) {
      RemoveItem(0);
    }
  }
}

///////////////////////////////////////////////
tU32 cWidgetListBox::_ComputeDefaultColumnWidth() const {
  if (mpWidget == NULL)
    return knWidgetListBoxMinDefaultColumnWidth;
  const tU32 clientW = (tU32)mpWidget->GetClientSize().x;
  const tU32 numCols = GetNumColumns();
  if (numCols <= 1) {
    return ni::Max(clientW,1);
  }
  else {
    tU32 w = (tU32)((tF32)clientW / (tF32)numCols);
    tU32 minW = ni::Min((tU32)clientW,knWidgetListBoxMinDefaultColumnWidth);
    return ni::Max(w,knWidgetListBoxMinDefaultColumnWidth);
  }
}

///////////////////////////////////////////////
tU32 cWidgetListBox::_ComputeColumnWidth(const tU32 anCol, const tU32 anDefaultColumnWidth) const {
  const sColumn& col = mvColumns[anCol];
  return (col.nSetSize > 0) ? col.nSetSize : anDefaultColumnWidth;
}

///////////////////////////////////////////////
tU32 cWidgetListBox::_ComputeItemsPerPage() const {
  const tF32 itemHeight = GetItemHeight();
  return (tU32)ni::Ceil(mfRealH/itemHeight);
}

///////////////////////////////////////////////
tU32 cWidgetListBox::AddItem(const achar *aszName)
{
  _RemoveOverflowingItems();
  sItem& item = *astl::push_back(mvItems, astl::make_shared<sItem>());
  item.vData.resize(GetNumColumns());
  item.vData[0].strText = aszName;
  mInternalFlags.bShouldSortItems = (bool)eTrue;
  UpdateLayout();
  return mvItems.size()-1;
}

///////////////////////////////////////////////
tBool cWidgetListBox::RemoveItem(tU32 anItem)
{
  if (anItem >= GetNumItems()) return eFalse;
  mvItems[anItem]->Invalidate();
  mvItems.erase(mvItems.begin()+anItem);
  UpdateLayout();
  return eTrue;
}

///////////////////////////////////////////////
tBool cWidgetListBox::SetItemText(tU32 anColumn, tU32 anItem, const achar *aaszText)
{
  if (anItem >= GetNumItems()) return eFalse;
  if (anColumn != 0 && anColumn >= GetNumColumns())
    return eFalse;
  mvItems[anItem]->vData[anColumn].strText = aaszText;
  if (mnSortKey == anColumn) {
    mInternalFlags.bShouldSortItems = (bool)eTrue;
  }
  return eTrue;
}

///////////////////////////////////////////////
const achar * cWidgetListBox::GetItemText(tU32 anColumn, tU32 anItem) const
{
  if (anItem >= GetNumItems()) return AZEROSTR;
  if (anColumn != 0 && anColumn >= GetNumColumns()) return AZEROSTR;
  return mvItems[anItem]->vData[anColumn].strText.c_str();
}

///////////////////////////////////////////////
tBool cWidgetListBox::SetItemTextColor(tU32 anColumn, tU32 anItem, tU32 anTextColor)
{
  if (anItem >= GetNumItems())
    return eFalse;
  sItem& item = *mvItems[anItem];
  if (anColumn == eInvalidHandle) {
    niLoop(i,GetNumColumns()) {
      item.vData[i].nTextColor = anTextColor;
    }
  }
  else {
    if (anColumn != 0 && anColumn >= GetNumColumns())
      return eFalse;
    item.vData[anColumn].nTextColor = anTextColor;
  }
  return eTrue;
}
tU32 cWidgetListBox::GetItemTextColor(tU32 anColumn, tU32 anItem) const
{
  if (anItem >= GetNumItems()) return 0;
  if (anColumn != 0 && anColumn >= GetNumColumns()) return 0;
  const sItem& item = *mvItems[anItem];
  return item.vData[anColumn].nTextColor;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetListBox::SetItemWidget(tU32 anColumn, tU32 anItem, iWidget* apWidget)
{
  if (anItem >= GetNumItems()) return eFalse;
  if (anColumn != 0 && anColumn >= GetNumColumns()) return eFalse;
  sItem& item = *mvItems[anItem];
  item.SetWidget(anColumn,apWidget);
  return eTrue;
}

///////////////////////////////////////////////
iWidget* __stdcall cWidgetListBox::GetItemWidget(tU32 anColumn, tU32 anItem) const
{
  if (anItem >= GetNumItems()) return NULL;
  if (anColumn != 0 && anColumn >= GetNumColumns()) return NULL;
  const sItem& item = *mvItems[anItem];
  return item.vData[anColumn].ptrWidget;
}

///////////////////////////////////////////////
tBool cWidgetListBox::SetItemIcon(tU32 anItem, iOverlay *apIcon)
{
  if (anItem >= GetNumItems()) return eFalse;
  sItem& item = *mvItems[anItem];
  item.ptrIcon = apIcon;
  return eTrue;
}

///////////////////////////////////////////////
iOverlay * cWidgetListBox::GetItemIcon(tU32 anItem) const
{
  if (anItem >= GetNumItems()) return NULL;
  sItem& item = *mvItems[anItem];
  return item.ptrIcon;
}

///////////////////////////////////////////////
tBool cWidgetListBox::SetItemData(tU32 anItem, iUnknown *apData)
{
  if (anItem >= GetNumItems()) return eFalse;
  sItem& item = *mvItems[anItem];
  item.ptrUserData = apData;
  return eTrue;
}

///////////////////////////////////////////////
iUnknown * cWidgetListBox::GetItemData(tU32 anItem) const
{
  if (anItem >= GetNumItems())
    return NULL;
  const sItem& item = *mvItems[anItem];
  return item.ptrUserData;
}

///////////////////////////////////////////////
tBool cWidgetListBox::SetSortKey(tU32 anKeyColumn)
{
  if (anKeyColumn == eInvalidHandle) {
    // disable sorting
    mnSortKey = eInvalidHandle;
    return eTrue;
  }
  else {
    if (anKeyColumn >= GetNumColumns())
      return eFalse; // invalid sort key
    if (mnSortKey == anKeyColumn)
      return eTrue; // nothing changed
    mnSortKey = anKeyColumn;
    mInternalFlags.bShouldSortItems = (bool)eTrue;
    return !(mnSortKey==eInvalidHandle);
  }
}

///////////////////////////////////////////////
tU32 cWidgetListBox::GetSortKey() const
{
  return mnSortKey;
}

///////////////////////////////////////////////
void cWidgetListBox::SetSortAscendant(tBool abAscendant)
{
  if (abAscendant == mbSortType)
    return; // nothing changed
  mbSortType = abAscendant;
  mInternalFlags.bShouldSortItems = (bool)eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetListBox::GetSortAscendant() const
{
  return mbSortType;
}

///////////////////////////////////////////////
bool __stdcall cWidgetListBox::SortItem(const sItem& a,const sItem& b)
{
  tU32 column = (mnSortKey != eInvalidHandle) ? mnSortKey : 0;
  const achar* strA = a.vData[column].strText.c_str();
  const achar* strB = b.vData[column].strText.c_str();
  tI32 res = 0;
  if (ni::StrIsDigit(strA[0]) && ni::StrIsDigit(strB[0])) {
    const achar *ea;
    const achar *eb;
    res = (tI32)(ni::StrToL(strA,&ea,10) - ni::StrToL(strB,&eb,10));
    if (StrIsLetter(*ea) || StrIsLetter(*eb)) {
      // not a valid number...
      res = ni::StrICmp(strA,strB);
    }
  }
  else {
    res = ni::StrICmp(strA,strB);
  }
  return mbSortType ? res < 0 : res > 0;
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetListBox::GetItemFromText(tU32 anColumn, const achar* aaszText) const
{
  if (anColumn != 0 && anColumn >= GetNumColumns())
    return eInvalidHandle;
  niLoop(i,mvItems.size()) {
    const sItem& item = *mvItems[i];
    if (ni::StrEq(item.vData[anColumn].strText.c_str(),aaszText)) {
      return i;
    }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
void __stdcall cWidgetListBox::ClearSelection()
{
  mvSelection.clear();
  NotifySelChange();
}

///////////////////////////////////////////////
tBool cWidgetListBox::AddSelection(tU32 anItem)
{
  if (niFlagIsNot(mpWidget->GetStyle(),eWidgetListBoxStyle_Multiselect) &&
      mvSelection.size() > 0)
    return eFalse;
  if (anItem >= GetNumItems())
    return eFalse;
  mvSelection.push_back(anItem);
  NotifySelChange();
  return eTrue;
}

///////////////////////////////////////////////
tBool cWidgetListBox::RemoveSelection(tU32 anItem)
{
  if (astl::find_erase(mvSelection,anItem)) {
    NotifySelChange();
    return eTrue;
  }
  return eFalse;
}

///////////////////////////////////////////////
tU32 cWidgetListBox::GetNumSelections() const
{
  return mvSelection.size();
}

///////////////////////////////////////////////
tU32 cWidgetListBox::GetSelection(tU32 anIndex) const
{
  if (anIndex >= GetNumSelections()) return eInvalidHandle;
  return mvSelection[anIndex];
}

///////////////////////////////////////////////
tBool cWidgetListBox::SetSelected(tU32 anSelection)
{
  // here we don't need to call the NotifySelChange() method, cause the AddSelection will do it.
  mvSelection.clear();
  return AddSelection(anSelection);
}

///////////////////////////////////////////////
tU32 cWidgetListBox::GetSelected() const
{
  return GetSelection(0);
}

///////////////////////////////////////////////
tBool __stdcall cWidgetListBox::GetIsItemSelected(tU32 anItem) const
{
  if (mvSelection.empty()) return eFalse;
  if (anItem >= GetNumItems()) return eFalse;
  for (tU32 i = 0; i < mvSelection.size(); ++i) {
    if (mvSelection[i] == anItem)
      return eTrue;
  }
  return eFalse;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetListBox::OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
{
  niGuardObject((iWidgetSink*)this);
  switch (nMsg)
  {
    case eUIMessage_SkinChanged:
      InitSkin();
      UpdateLayout();
      return eFalse;

    case eUIMessage_SinkDetached:
      mpWidget->SetTimer(mnAutoScroll,-1);
      return eFalse;

    case eUIMessage_NCSize:
      DoUpdateLayout(eTrue);
      break;
    case eUIMessage_NCPaint: {
      DoUpdateLayout(eFalse);
      iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
      if (c) {
        sRectf rect = mpWidget->GetWidgetRect();
        if (ULColorGetA(skin.ulcolBg) > 0) {
          c->BlitFillAlpha(rect, skin.ulcolBg);
        }
        c->BlitOverlayFrame(
          rect,
          apWidget->GetHasFocus() ? skin.focusedFrame : skin.normalFrame,
          eRectFrameFlags_All); // here we should draw all frame;

        // The Columns should draw after the UI background
        if (GetNumColumns() && (apWidget->GetStyle()&eWidgetListBoxStyle_HasHeader)) {
          Paint_Header(c,rect);
        }
      }
      break;
    }
    case eUIMessage_Paint: {
      iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
      if (c) {
        Paint_Items(c);
      }
      break;
    }
    case eUIMessage_MouseMove:
    case eUIMessage_NCMouseMove:
      if (mnHeaderHandlePressed != eInvalidHandle) {
        tI32 diff = ni::FloatToIntNearest(varParam1.GetVec2f().x - mvHeaderPivot.x);
        if (diff != 0) {
          tI32 nSize = ni::Max(5, (tI32)mnBaseSize + diff);
          mvColumns[mnHeaderHandlePressed].nSetSize = nSize;
          UpdateLayout();
        }
      }
      else {
        UpdateSelectOnMove();
      }
      break;
    case eUIMessage_RightClickDown:
    case eUIMessage_LeftClickDown:
      {
        apWidget->SetCapture(eTrue);
        mnInputSelected = nMsg;
        SelectItemByPos(varParam1.GetVec2f()+mpWidget->GetAbsolutePosition());
        mnInputSelected = eInvalidHandle;
        break;
      }
    case eUIMessage_NCLeftClickDown:
      {
        apWidget->SetCapture(eTrue);
        sVec2f vMousePos = varParam1.GetVec2f();
        if (mrectHeader.Intersect(vMousePos)) {
          mnHeaderHandlePressed = GetSelectedColumnHandle(vMousePos);
          niDebugFmt(("... mnHeaderHandlePressed: %d",
                      mnHeaderHandlePressed));
          if (mnHeaderHandlePressed != eInvalidHandle) {
            mvHeaderPivot = vMousePos;
            mnBaseSize = mvColumns[mnHeaderHandlePressed].nSetSize;
          } else {
            // EDMOND
            if (mpWidget->GetStyle()&eWidgetListBoxStyle_HeaderSort)
            {
              const tU32 defColW = _ComputeDefaultColumnWidth();
              tF32 basex = 0.0f;
              niLoop(col, mvColumns.size()) {
                basex += _ComputeColumnWidth(col,defColW);
                if ((vMousePos.x-basex) < 0) {
                  mnSortKey = col;
                  break;
                }
                basex += LB_SEP_W;
              }
              SetSortAscendant(!mbSortType);
              skin.arrow = (mbSortType) ? skin.arrowDown  : skin.arrowUp;
            }
          }
        }
        break;
      }
    case eUIMessage_TextChanged:
      {
        SetSelectionFromText();
        break;
      }
    case eUIMessage_SetText:
      {
        SetTextFromSelection();
        break;
      }
    case eUIMessage_RightClickUp:
    case eUIMessage_LeftClickUp:
      //    if (mnHeaderHandlePressed < 0) {  // Select on click down...
      //    } //else continues to the next case (eUIMessage_NCLeftClickUp)
    case eUIMessage_NCLeftClickUp:
      if (mnHeaderHandlePressed != eInvalidHandle) {
        mnHeaderHandlePressed = eInvalidHandle;
      }
      apWidget->SetCapture(eFalse);
      break;
    case eUIMessage_SetCursor:
      if (mnHeaderHandlePressed >= 0) {
        mpWidget->GetUIContext()->SetCursor(skin.curResizeHz);
        return eTrue;
      }
      else if(mrectHeader.Intersect(varParam1.GetVec2f())) {
        if (GetSelectedColumnHandle(varParam1.GetVec2f()) != eInvalidHandle) {
          mpWidget->GetUIContext()->SetCursor(skin.curResizeHz);
          return eTrue;
        }
      }
      return eFalse;
    case eUIMessage_KeyDown:
      {
        OnKeyDown(varParam0.mU32);
        break;
      }
    case eUIMessage_Command:
      if (mpWidget->GetParent()) {
        Ptr<iWidgetCommand> ptrCmd = ni::VarQueryInterface<iWidgetCommand>(varParam0);
        if (ptrCmd.IsOK() && (ptrCmd->GetSender() == mptrHzScroll || ptrCmd->GetSender() == mptrVtScroll)) {
          if (ptrCmd->GetExtra2().mBool) {
            // Wheel scroll ?
            UpdateSelectOnMove();
          }
        }
        else {
          mpWidget->GetParent()->SendMessage(nMsg,varParam0,varParam1);
        }
      }
      break;

    case eUIMessage_Timer:  {
      tU32 id = varParam0.mU32;
      if (id == mnAutoScroll && !mpWidget->GetAbsoluteRect().Intersect(mpWidget->GetUIContext()->GetCursorPosition())) {
        QPtr<iWidgetScrollBar> ptrVSB = mptrVtScroll.ptr();
        if (ptrVSB.IsOK() && mptrVtScroll->GetVisible()) {
          tF32 scrollpos = Floor(ptrVSB->GetScrollPosition());
          tF32 pagesize = ptrVSB->GetPageSize();
          tU32 itemSize  = mvItems.size();

          if (itemSize <= pagesize) {
            break;
          }

          tF32 k = Ceil(pagesize) - pagesize;
          scrollpos += k;

          ++scrollpos;
          if (scrollpos > ptrVSB->GetScrollRange().y) {
            scrollpos = 0;
          }
          mpSpringScroll->SetIdealPosition(scrollpos);
        }
      }
      break;
    }

    case eUIMessage_ExpressionUpdate: {
      QPtr<iExpressionContext> _ctx(varParam0);
      if (!_ctx.IsOK()) return eTrue;

      if (mstrItemsExpr.IsEmpty()) return eTrue;
      Ptr<iExpressionContext> ctx = _ctx->CreateContext();

      ClearItems();
      Ptr<iExpressionVariable> v = ctx->Eval(mstrItemsExpr.Chars());
      if (!v.IsOK()) return eFalse;

      QPtr<iDataTable> dt = v->GetIUnknown();
      if (!dt.IsOK()) return eFalse;

      Ptr<iExpressionVariable> ItemText = ctx->CreateVariable("ItemText",eExpressionVariableType_String,0);
      ctx->AddVariable(ItemText);

      Ptr<iExpressionVariable> ItemColumn = ctx->CreateVariable("ItemColumn",eExpressionVariableType_Float,0);
      ctx->AddVariable(ItemColumn);

      Ptr<iExpressionVariable> ItemRow = ctx->CreateVariable("ItemRow",eExpressionVariableType_Float,0);
      ctx->AddVariable(ItemRow);

      tU32 numColumns = GetNumColumns();
      niLoop(i, dt->GetNumChildren()) {
        Ptr<iDataTable> item = dt->GetChildFromIndex(i);
        tU32 itemId = AddItem(item->GetName());
        ItemRow->SetFloat(itemId);

        tBool isArray = item->GetBoolDefault("__isArray", false);
        niLoop(j, numColumns) {
          ItemColumn->SetFloat(j);

          cString text;
          if (isArray) {
            Ptr<iDataTable> itemVar = item->GetChildFromIndex(j);
            if (itemVar.IsOK()) {
              text = itemVar->GetString("v").Chars();
            }
          }
          else {
            text = item->GetString(HStringGetStringEmpty(mvColumns[j].hspKey));
          }
          ItemText->SetString(text);

          tHStringPtr widgetExpr = mvColumns[j].hspWidgetExpr;
          if (!HStringIsEmpty(widgetExpr)) {
            Ptr<iExpressionVariable> vw = ctx->Eval(HStringGetStringEmpty(widgetExpr));
            if (vw.IsOK() && vw->GetString().IsNotEmpty()) {
              Ptr<iWidget> tmpWidget = mpWidget->FindWidget(_H(vw->GetString()));
              if (tmpWidget.IsOK()) {
                Ptr<iDataTable> wDT = CreateDataTable("Widget");
                mpWidget->GetUIContext()->SerializeWidget(
                  tmpWidget,
                  wDT,
                  eWidgetSerializeFlags_Write|ni::eWidgetSerializeFlags_Children,
                  NULL);

                Ptr<iWidget> itemWidget = mpWidget->GetUIContext()->CreateWidgetFromDataTable(
                  wDT,mpWidget,_H(niFmt("%s_%d_%d",mpWidget->GetID(), j, itemId)), NULL);
                itemWidget->SetStyle(itemWidget->GetStyle() | eWidgetStyle_ItemOwned | eWidgetStyle_DontSerialize);

                Ptr<iWidget> w = mvItems[itemId]->vData[j].ptrWidget;
                if (w.IsOK()) {
                  w->Invalidate();
                }
                mvItems[itemId]->vData[j].ptrWidget = itemWidget;

                itemWidget->BroadcastMessage(eUIMessage_ExpressionUpdate, varParam0);
                SetItemWidget(j, itemId, itemWidget);
              }
            }
          }
          else {
            SetItemText(j, itemId, text.Chars());
          }
        }
      }
      return eTrue;
    }

    case eUIMessage_SerializeWidget: {
      QPtr<iDataTable> ptrDT(varParam0);
      if (!ptrDT.IsOK()) return eTrue;

      tU32 nFlags = varParam1.mU32;
      if (nFlags & eWidgetSerializeFlags_Read) {
        mfItemHeight = ptrDT->GetFloatDefault("item_height", mfItemHeight);
        mstrItemsExpr = ptrDT->GetString("item_expr");

        mpWidget->SetTimer(mnAutoScroll,-1);
        mnAutoScroll = ptrDT->GetIntDefault("auto_scroll", -1);
        mpWidget->SetTimer(mnAutoScroll, mnAutoScroll);

        Ptr<iDataTable> columns = ptrDT->GetChild("Columns");
        if (columns.IsOK()) {
          tU32 num = columns->GetNumChildren();
          while (GetNumColumns() > num) {
            RemoveColumn(GetNumColumns() - 1);
          }

          while (GetNumColumns() < num) {
            AddColumn("", 100);
          }

          niLoop(i, num) {
            Ptr<iDataTable> column = columns->GetChildFromIndex(i);
            SetColumn(i, column->GetString("header").Chars(), column->GetInt("width"));
            mvColumns[i].hspWidgetExpr = column->GetHString("widget_expr");
            mvColumns[i].hspKey = column->GetHString("key");
            Ptr<iDataTable> f = column->GetChild("Font");
            if (f.IsOK()) {
              mvColumns[i].ptrFont = SerializeReadFont(mpWidget->GetUIContext(), f);
            }
          }
        }
      }
      else if (nFlags & eWidgetSerializeFlags_Write) {
        tBool hasMetadata = niFlagIs(nFlags,eWidgetSerializeFlags_PropertyBox);
        ptrDT->SetFloat("item_height", GetItemHeight());
        ptrDT->SetString("item_expr", mstrItemsExpr.Chars());
        ptrDT->SetInt("auto_scroll", mnAutoScroll);
        if (hasMetadata) {
          ptrDT->SetMetadata("item_height", _H("range[min=0,max=10000,step=1]"));
          ptrDT->SetMetadata("item_expr", _H("expr"));
          ptrDT->SetMetadata("auto_scroll", _H("int"));
        }

        Ptr<iDataTable> columns = CreateDataTable("Columns");
        niLoop(i, GetNumColumns())
        {
          Ptr<iDataTable> column = CreateDataTable("column");
          column->SetString("header", GetColumnName(i));
          column->SetHString("key", mvColumns[i].hspKey);
          column->SetInt("width", GetColumnWidth(i));
          column->SetHString("widget_expr", mvColumns[i].hspWidgetExpr);

          if (hasMetadata) {
            column->SetMetadata(_A("width"),_H("range[min=4,max=10000,step=1]"));
            column->SetMetadata("widget_expr", _H("expr"));
          }

          Ptr<iFont> font = mvColumns[i].ptrFont;
          if (!font.IsOK()) {
            font = mpWidget->GetFont();
          }

          Ptr<iDataTable> f = CreateDataTable("Font");
          SerializeWriteFont(font, f, hasMetadata);
          column->AddChild(f);

          columns->AddChild(column);
        }
        ptrDT->AddChild(columns);
      }
      break;
    }
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
void cWidgetListBox::UpdateLayout()
{
  mInternalFlags.bShouldUpdateLayout = (bool)eTrue;
}

void cWidgetListBox::DoUpdateLayout(tBool abForce)
{
  if (abForce || mInternalFlags.bShouldSortItems) {
    if (mnSortKey != eInvalidHandle) {
      astl::stable_sort(mvItems.begin(),mvItems.end(),
                        [&](const astl::shared_ptr<sItem>& a,const astl::shared_ptr<sItem>& b) {
                          return this->SortItem(*a,*b);
                        });
    }
    mInternalFlags.bShouldSortItems = eFalse;
  }

  if (abForce || mInternalFlags.bShouldUpdateLayout) {
    RecomputeWidestItem();
    sVec2f vSize = mpWidget->GetSize();
    ComputeClientRect(vSize.x,vSize.y);
    mInternalFlags.bShouldUpdateLayout = eFalse;
  }

  QPtr<iWidgetScrollBar> ptrVSB = mptrVtScroll.ptr();
  if (ptrVSB.IsOK() && mptrVtScroll->GetVisible()) {
    if (mnAutoScroll > 0 && !mpWidget->GetAbsoluteRect().Intersect(mpWidget->GetUIContext()->GetCursorPosition())) {
      mpSpringScroll->UpdatePosition(GetLang()->GetFrameTime());
      ptrVSB->SetScrollPosition(mpSpringScroll->GetCurrentPosition());
    }
    else {
      mpSpringScroll->SetCurrentPosition(ptrVSB->GetScrollPosition());
      mpSpringScroll->SetIdealPosition(ptrVSB->GetScrollPosition());
    }
  }

  if (mInternalFlags.bShouldAutoScroll) {
    QPtr<iWidgetScrollBar> ptrVSB = mptrVtScroll.ptr();
    if (ptrVSB.IsOK() && mptrVtScroll->GetVisible()) {
      tU32 scrollpos = (tU32)ptrVSB->GetScrollPosition();
      tU32 pagesize = (tU32)ptrVSB->GetPageSize();
      // niDebugFmt(("... scrollpos: %g, pagesize: %g, destPos: %g, items: %g",
                  // scrollpos, pagesize, destPos, mvItems.size()));

      if (mvSelection.empty()) {
        ptrVSB->SetScrollPosition(ptrVSB->GetScrollRange().y);
      }
      else {
        tU32 dest = mvSelection.back();
        if (scrollpos < dest && scrollpos + pagesize > dest + 1) {
          // already inside;
        }
        else  {
          const tI32 midPageOffset = ni::Max(0,(pagesize/2) - 1);
          ptrVSB->SetScrollPosition((tF32)dest - midPageOffset);
        }
      }
    }
    QPtr<iWidgetScrollBar> ptrHSB = mptrHzScroll.ptr();
    if (ptrHSB.IsOK() && mptrHzScroll->GetVisible()) {
      ptrHSB->SetScrollPosition(0);
    }
    mInternalFlags.bShouldAutoScroll = eFalse;
  }
}

///////////////////////////////////////////////
void cWidgetListBox::ComputeClientRect(tF32 w,tF32 h)
{
  iOverlay* frame = skin.normalFrame;
  sRectf clirect = frame->ComputeFrameCenter(sRectf(0,0,w,h));

  if (GetNumColumns() && (eWidgetListBoxStyle_HasHeader&mpWidget->GetStyle())) {
    iOverlay* ovr = skin.header;
    const tF32 h = GetItemHeight()+ovr->GetFrame().y+ovr->GetFrame().w;
    mrectHeader.Set(0,0,w,h);
    clirect.SetTop(clirect.GetTop()+h);
  }
  else {
    mrectHeader.Set(Vec2<tF32>(0,0),0,0);
  }

  UpdateWidgetScrollBars(w,h);
  if (mptrHzScroll->GetVisible()) {
    clirect.SetBottom(mptrHzScroll->GetRect().Top());
  }
  if (mptrVtScroll->GetVisible()) {
    clirect.SetRight(mptrVtScroll->GetRect().Left());
  }

  const sVec4f padding = mpWidget->GetPadding();
  clirect.x += padding.x;
  clirect.y += padding.y;
  clirect.z -= padding.z;
  clirect.w -= padding.w;
  mpWidget->SetClientRect(clirect);
}


///////////////////////////////////////////////
tU32 cWidgetListBox::GetSelectedColumnHandle(const sVec2f &mpos)
{
  tF32 basex = _GetScrolledBaseX();
  tU32 res = eInvalidHandle;
  const tU32 defColW = _ComputeDefaultColumnWidth();
  niLoop(col,mvColumns.size()) {
    basex += _ComputeColumnWidth(col,defColW);
    if (fabs(mpos.x-basex) <= 5) {
      res = col;
      break;
    }
    basex += LB_SEP_W;
  }
  return res;
}

///////////////////////////////////////////////
void cWidgetListBox::AutoScroll()
{
  mInternalFlags.bShouldAutoScroll = (bool)eTrue;
}

///////////////////////////////////////////////
void cWidgetListBox::OnKeyDown(tU32 key)
{
  const tU32 selMode = GetModSelMode();
  switch(key)
  {
    case eKey_Up:
      {
        if (!mvSelection.empty() && mvSelection.back() > 0)
        {
          if (selMode == SEL_RANGE) {
            AddSelection(mvSelection.back()-1);
          }
          else {
            SetSelected(mvSelection.back()-1);
          }
          AutoScroll();
        }
        break;
      }
    case eKey_Down:
      {
        if (!mvSelection.empty() && ((tI32)mvSelection.back() < (tI32)mvItems.size()-1))
        {
          if (selMode == SEL_RANGE) {
            AddSelection(mvSelection.back()+1);
          }
          else {
            SetSelected(mvSelection.back()+1);
          }
          AutoScroll();
        }
        break;
      }
    case eKey_PgUp:
      {
        if (!mvSelection.empty() && mvSelection.back() > 0)
        {
          const tU32 nNumLines = _ComputeItemsPerPage();
          if (selMode == SEL_RANGE) {
            tI32 nSelEnd = (tI32)ni::Max(0,(tI32)mvSelection.back()-(tI32)nNumLines);
            for (tI32 i = (tI32)mvSelection.back()-1; i >= nSelEnd; --i) {
              AddSelection(i);
            }
          }
          else {
            SetSelected(ni::Max(0,(tI32)mvSelection.back()-(tI32)nNumLines));
          }
          AutoScroll();
        }
        break;
      }
    case eKey_PgDn:
      {
        if (!mvSelection.empty() && ((tI32)mvSelection.back() < (tI32)mvItems.size()-1))
        {
          const tU32 nNumLines = _ComputeItemsPerPage();
          if (selMode == SEL_RANGE) {
            tI32 nSelEnd = (tI32)ni::Min(mvItems.size()-1,mvSelection.back()+nNumLines);
            for (tI32 i = (tI32)mvSelection.back()+1; i <= nSelEnd; ++i) {
              AddSelection(i);
            }
          }
          else {
            SetSelected(ni::Min(mvItems.size()-1,mvSelection.back()+nNumLines));
          }
          AutoScroll();
        }
        break;
      }
    case eKey_Home:
      {
        if (mvSelection.empty() || mvSelection.back() != 0)
        {
          if (selMode == SEL_RANGE) {
            for (tI32 i = mvSelection.back()-1; i >= 0; --i) {
              AddSelection(i);
            }
          }
          else {
            SetSelected(0);
          }
          AutoScroll();
        }
        break;
      }
    case eKey_End:
      {
        if (mvSelection.empty() || mvSelection.back() != mvItems.size()-1)
        {
          if (selMode == SEL_RANGE) {
            for (tI32 i = tI32(mvSelection.back()+1); i <= tI32(mvItems.size()-1); ++i) {
              AddSelection(i);
            }
          }
          else {
            SetSelected(mvItems.size()-1);
          }
          AutoScroll();
        }
        break;
      }
  }
}

///////////////////////////////////////////////
tU32 cWidgetListBox::RecomputeWidestItem()
{
  iFont *font = mpWidget->GetFont();
  const tU32 numCols = GetNumColumns();
  const tU32 defColW = _ComputeDefaultColumnWidth();
  const tF32 sepw = niFlagIs(mpWidget->GetStyle(),eWidgetListBoxStyle_HasHeader) ? LB_SEP_W : 0;
  mfWidestItem = 0;
  if (numCols == 1) {
    niLoop(i,mvItems.size()) {
      const sItem& item = *mvItems[i];
      const tF32 x = font->ComputeTextSize(sRectf(0,0),item.vData[0].strText.c_str(),0).GetWidth();
      mfWidestItem = ni::Max(mfWidestItem,x);
    }
  }
  else {
    niLoop(i,mvItems.size()) {
      const sItem& item = *mvItems[i];
      tF32 x = 0;
      niLoop(col,mvColumns.size()) {
        x += _ComputeColumnWidth(col,defColW);
        x += sepw;
      }
      mfWidestItem = ni::Max(mfWidestItem,x);
    }
  }
  return (tU32)mfWidestItem;
}

///////////////////////////////////////////////
void cWidgetListBox::UpdateWidgetScrollBars(tF32 w, tF32 h)
{
  const tF32 fh = (tF32)GetItemHeight();
  const tF32 fw = (tF32)mpWidget->GetFont()->GetMaxCharWidth();

  QPtr<iWidgetScrollBar> ptrHSB = mptrHzScroll.ptr();
  QPtr<iWidgetScrollBar> ptrVSB = mptrVtScroll.ptr();
  if (!ptrHSB.IsOK() || !ptrVSB.IsOK())
    return;

  // Use the value we'll set, don't get it from the scrollbar itself since it
  // might not have been initialized yet.
  const tF32 vtScrollBarW = kfScrollBarSize;
  const tF32 vtScrollBarH = kfScrollBarSize;

  const sRectf borders = skin.normalFrame->GetFrame();
  mfRealW = w - (borders.x+borders.z);
  mfRealH = h - mrectHeader.GetHeight() - (borders.y+borders.w);

  const sVec2f vSize = mpWidget->GetSize();

  sRectf rectHz;
  sRectf rectVt;

  // We show the hz scrolling if the widest item would overflow with the vt
  // scrollbar visible.
  const tBool bHorzScollbar = (mfWidestItem > (mfRealW-vtScrollBarW));
  if (bHorzScollbar) {
    mfRealH -= vtScrollBarH;
    mptrHzScroll->SetVisible(eTrue);
    mptrHzScroll->SetEnabled(eTrue);
    tF32 nHorzItemsPage = ni::Floor<tF32>(mfRealW/fw);
    ptrHSB->SetPageSize((tF32)nHorzItemsPage);
    ptrHSB->SetScrollRange(Vec2<tF32>(0,(tF32)((mfWidestItem/fw)-nHorzItemsPage)));
    rectHz = sRectf(
      borders.x,vSize.y-kfScrollBarSize-borders.w,
      mfRealW,kfScrollBarSize);
  }
  else {
    mptrHzScroll->SetVisible(eFalse);
    mptrHzScroll->SetEnabled(eFalse);
    ptrHSB->SetScrollPosition(0);
  }

  const tBool bVertScollbar = (_ComputeItemsPerPage() < mvItems.size());
  if (bVertScollbar) {
    mfRealW -= vtScrollBarW;
    mptrVtScroll->SetVisible(eTrue);
    mptrVtScroll->SetEnabled(eTrue);

    const tF32 pageSize = mfRealH/GetItemHeight();
    ptrVSB->SetPageSize(pageSize);
    ptrVSB->SetScrollRange(Vec2<tF32>(0,(tF32)mvItems.size() - pageSize));
    tF32 vertY = borders.y;
    if (GetNumColumns() && (mpWidget->GetStyle()&eWidgetListBoxStyle_HasHeader)) {
      vertY += mrectHeader.GetBottom();
    }
    rectVt = sRectf(
      vSize.x-kfScrollBarSize-borders.z,vertY,
      kfScrollBarSize,mfRealH);
    rectHz.SetRight(rectHz.GetRight() - kfScrollBarSize);
  }
  else {
    mptrVtScroll->SetVisible(eFalse);
    mptrVtScroll->SetEnabled(eFalse);
    ptrVSB->SetScrollPosition(0);
  }

  if (bHorzScollbar) {
    mptrHzScroll->SetRect(rectHz);
  }
  if (bVertScollbar) {
    mptrVtScroll->SetRect(rectVt);
  }
}

///////////////////////////////////////////////
void cWidgetListBox::NotifySelChange()
{
  if (mInternalFlags.bInNotifySelChanged)
    return;
  mInternalFlags.bInNotifySelChanged = (bool)eTrue;
  mpWidget->SendCommand(mpWidget->GetParent(),eWidgetListBoxCmd_SelectionChanged,mnInputSelected);
  mInternalFlags.bInNotifySelChanged = eFalse;
}

///////////////////////////////////////////////
tBool cWidgetListBox::SelectItemByPos(const sVec2f& avAbsPos)
{
  const sVec2f pos = avAbsPos - mpWidget->GetAbsolutePosition() - mpWidget->GetClientPosition();
  tF32 scrollpos = _GetVScrollPos();
  tF32 fh = (tF32)GetItemHeight();

  tU32 nNewSelection = (tU32)floor(((scrollpos * fh) + pos.y) / fh);
  if (nNewSelection >= GetNumItems())
    return eFalse;

  // ignore selection change when it triggered by the MouseMove event;
  if (mnInputSelected == eUIMessage_MouseMove && nNewSelection == GetSelected())
    return eFalse;

  const tU32 selMode = GetModSelMode();
  switch (selMode) {
    case SEL_SINGLE:
      SetSelected(nNewSelection);
      break;
    case SEL_ADD:
      if (GetIsItemSelected(nNewSelection))
        RemoveSelection(nNewSelection);
      else
        AddSelection(nNewSelection);
      break;
    case SEL_RANGE:
      if (mvSelection.empty()) {
        SetSelected(nNewSelection);
      }
      else if (nNewSelection != mvSelection.back()) {
        if (nNewSelection < mvSelection.back()) {
          for (tI32 i = tI32(mvSelection.back()-1); i >= tI32(nNewSelection); --i) {
            AddSelection(i);
          }
        }
        else /* if (nNewSelection > mvSelection.back()) */ {
          for (tU32 i = mvSelection.back()+1; i <= nNewSelection; ++i) {
            AddSelection(i);
          }
        }
      }
      break;
  }

  return eTrue;
}

///////////////////////////////////////////////
void cWidgetListBox::Paint_Items(iCanvas* apCanvas)
{
  if (mvItems.empty()) return;
  iFont *font = mpWidget->GetFont();
  const tF32 fh = (tF32)GetItemHeight();
  const tU32 numCols = mvColumns.size();
  const tF32 scrollpos = _GetVScrollPos();
  const tF32 scrollposY = scrollpos * fh;
  const tBool hasHeader = ((numCols > 1) && (mpWidget->GetStyle()&eWidgetListBoxStyle_HasHeader));
  const tF32 sepw = hasHeader ? LB_SEP_W : 0;
  const sRectf rect = mpWidget->GetClientRect();
  const tF32 startX = _GetScrolledBaseX();
  const tF32 clientW = mpWidget->GetClientSize().x;
  const sVec2f vAbsPos = mpWidget->GetAbsolutePosition()+mpWidget->GetClientPosition();
  const tU32 defColW = _ComputeDefaultColumnWidth();

  auto paintItem = [this,apCanvas,font,vAbsPos,defColW,&rect,fh,sepw,clientW,numCols]
      (const sItem& item, const tU32 itemIndex, const tF32 startX, const tF32 y) -> tF32
  {
    if (this->GetIsItemSelected(itemIndex)) {
      iOverlay *sel = mpWidget->GetHasFocus() ? skin.sel : skin.selNF;
      apCanvas->BlitOverlay(
        sRectf(0,y,rect.GetWidth(),fh),
        sel);
    }

    tF32 curX = startX;

    niLoop(col,mvColumns.size()) {
      Ptr<iFont> pFont = mvColumns[col].ptrFont;
      if (!pFont.IsOK()) {
        pFont = font;
      }

      const tU32 wasFontColor = pFont->GetColor();
      const sItemColumnData& itemColData = item.vData[col];
      const tU32 colw = _ComputeColumnWidth(col,defColW);
      if (col == 0 && item.ptrIcon.IsOK()) {
        const sVec2f iconSize = item.ptrIcon->GetSize();
        apCanvas->BlitOverlay(
          sRectf(curX,y+(fh-iconSize.y)/2,iconSize.x,iconSize.x),
          item.ptrIcon);
        curX += iconSize.x;
      }

      iWidget* pW = itemColData.ptrWidget;
      if (pW) {
        pW->SetVisible(eTrue);
        sRectf curRect = pW->GetAbsoluteRect();
        sRectf newRect;
        newRect.SetLeft(vAbsPos.x+curX);
        newRect.SetRight(vAbsPos.x+curX+colw);
        tF32 offset = (fh-curRect.GetHeight())/2;
        newRect.SetTop(vAbsPos.y+y+offset);
        newRect.SetHeight(curRect.GetHeight());
        if (newRect != curRect) {
          pW->SetAbsoluteRect(newRect);
        }
      }
      else
      {
        const sRectf textRect = sRectf(curX,y,colw,fh);
        const achar* text = itemColData.strText.c_str();
        pFont->SetColor(itemColData.nTextColor ? itemColData.nTextColor : wasFontColor);
        apCanvas->BlitText(
          pFont, textRect,
          ((numCols > 1) ? eFontFormatFlags_ClipH : 0)|
          eFontFormatFlags_CenterV|eFontFormatFlags_NoUnitSnap, text);
      }

      pFont->SetColor(wasFontColor);
      curX += (colw+sepw);
      if (curX > rect.GetRight())
        break;
    }

    return fh;
  };

  {
    tF32 curY = 0;
    niLoop(i,mvItems.size()) {
      const astl::shared_ptr<sItem> item = mvItems[i];
      if (i >= (scrollpos-1) && (curY-scrollposY) < mfRealH) {
        paintItem(*item,i,startX,curY-scrollposY);
      }
      else {
        niLoop(col,numCols) {
          if (item->vData[col].ptrWidget.IsOK()) {
            item->vData[col].ptrWidget->SetVisible(eFalse);
          }
        }
      }
      curY += fh;
    }
  }
}

///////////////////////////////////////////////
void cWidgetListBox::Paint_Header(iCanvas* apCanvas, const sRectf &rect)
{
  iOverlay* header = skin.header;
  iOverlay* sortArrow = skin.arrow;
  iFont* headerFont = skin.headerFont;
  niCheckIsOK(headerFont,;);

  const tWidgetStyleFlags style = mpWidget->GetStyle();
  const sVec4f headerFrame = header->GetFrame();
  const tF32 fh = (tF32)GetItemHeight();
  apCanvas->BlitOverlay(mrectHeader,header);

  const tF32 basex = _GetScrolledBaseX();
  const tF32 sortArrowSize = 8;
  const tU32 sepColor = ULColorBuildf(0.5f,0.5f,0.5f,0.5f);
  const tF32 y = 0;
  const tU32 numCols = mvColumns.size();
  const tU32 defColW = _ComputeDefaultColumnWidth();

  tF32 x = basex + mpWidget->GetClientPosition().x + LB_TEXT_X_OFFSET;
  niLoop(colIndex, numCols) {
    const tF32 colW = (tF32)_ComputeColumnWidth(colIndex,defColW);
    const tBool hasSortArrow = (colIndex == mnSortKey) && (style&eWidgetListBoxStyle_HeaderSort);

    if (hasSortArrow) {
      apCanvas->BlitOverlay(
          sRectf(x + colW - sortArrowSize - headerFrame.y,
                 y + (mrectHeader.GetHeight()-sortArrowSize)/2.0f,
                 sortArrowSize, sortArrowSize),
          sortArrow);
    }

    apCanvas->BlitLineAlpha(
      Vec2f(x+colW-1,y),
      Vec2f(x+colW-1,y+mrectHeader.GetHeight()),
      sepColor
    );

    const sRectf textRect = apCanvas->BlitText(
        headerFont,
        Rectf(x+headerFrame.x,
              y+headerFrame.y,
              colW-(hasSortArrow?sortArrowSize+headerFrame.y:0)-3,
              fh),
        eFontFormatFlags_ClipH|eFontFormatFlags_Bottom,
        niHStr(mvColumns[colIndex].hspName));

    x += colW;
  }
}

///////////////////////////////////////////////
tBool __stdcall cWidgetListBox::AddSelectedItem(tU32 anCol, const achar* aaszText)
{
  tU32 nItem = GetItemFromText(anCol,aaszText);
  if (nItem != eInvalidHandle)
    return AddSelection(nItem);
  return eFalse;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetListBox::SetSelectedItem(tU32 anCol, const achar* aaszText)
{
  tU32 nItem = GetItemFromText(anCol,aaszText);
  if (nItem != eInvalidHandle)
    return SetSelected(nItem);
  return eFalse;
}

///////////////////////////////////////////////
const achar* __stdcall cWidgetListBox::GetSelectedItem(tU32 anCol) const
{
  return GetItemText(anCol,GetSelection(0));
}

///////////////////////////////////////////////
const achar* __stdcall cWidgetListBox::GetSelectedItemText(tU32 anCol, tU32 anIndex) const
{
  return GetItemText(anCol,GetSelection(anIndex));
}

///////////////////////////////////////////////
iWidget* __stdcall cWidgetListBox::GetSelectedItemWidget(tU32 anCol, tU32 anIndex) const
{
  return GetItemWidget(anCol,GetSelection(anIndex));
}

///////////////////////////////////////////////
tBool __stdcall cWidgetListBox::SetItemHeight(tF32 afHeight)
{
  mfItemHeight = afHeight;
  return eTrue;
}

///////////////////////////////////////////////
tF32 __stdcall cWidgetListBox::GetItemHeight() const
{
  return ni::Max(mfItemHeight,mpWidget->GetFont()->GetHeight()+2);
}

///////////////////////////////////////////////
void cWidgetListBox::InitSkin()
{
  skin.normalFrame = mpWidget->FindSkinElement(NULL,NULL,_H("Frame"));
  skin.focusedFrame = mpWidget->FindSkinElement(NULL,_H("Focused"),_H("Frame"));
  if (skin.focusedFrame == mpWidget->GetUIContext()->GetErrorOverlay())
    skin.focusedFrame = skin.normalFrame;
  skin.header = mpWidget->FindSkinElement(NULL,NULL,_H("Header"));
  skin.curResizeHz = mpWidget->FindSkinCursor(NULL,NULL,_H("ResizeHorizontal"));
  skin.sel = mpWidget->FindSkinElement(NULL,NULL,_H("Selection"));
  skin.selNF = mpWidget->FindSkinElement(NULL,NULL,_H("SelectionNoFocus"));
  skin.arrowDown = mpWidget->FindSkinElement(NULL,NULL,_H("ArrowDown"));
  skin.arrowUp = mpWidget->FindSkinElement(NULL,NULL,_H("ArrowUp"));
  skin.arrow = skin.arrowDown;
  skin.headerFont = mpWidget->FindSkinFont(NULL,NULL,_H("Header"));
  skin.ulcolBg = ULColorBuild(
    mpWidget->FindSkinColor(sVec4f::Zero(),NULL,NULL,_H("Background")));
}

///////////////////////////////////////////////
tU32 cWidgetListBox::GetModSelMode() const
{
  tU32 m = mpWidget->GetUIContext()->GetInputModifiers();
  if (niFlagIs(mpWidget->GetStyle(),eWidgetListBoxStyle_ClickAddSelection)) {
    if (niFlagIs(m,eUIInputModifier_AddSelection)) {
      return SEL_SINGLE;
    }
    if (niFlagIs(m,eUIInputModifier_AddSelectionRange)) {
      return SEL_RANGE;
    }
    return SEL_ADD;
  }
  else {
    if (niFlagIs(m,eUIInputModifier_AddSelection)) {
      return SEL_ADD;
    }
    if (niFlagIs(m,eUIInputModifier_AddSelectionRange)) {
      return SEL_RANGE;
    }
    return SEL_SINGLE;
  }
}

///////////////////////////////////////////////
static void _GetFlags(iHString* ahspText, astl::vector<cString>& aFlags) {
  //niPrintln(niFmt(_A("## GET FLAGS: %s\n"),niHStr(ahspText)));
  cString str = niHStr(ahspText);
  cFlagsStringTokenizer toks;
  StringTokenize(str,aFlags,&toks);
}

///////////////////////////////////////////////
void cWidgetListBox::SetSelectionFromText()
{
  if (niFlagIs(mpWidget->GetStyle(),eWidgetListBoxStyle_Multiselect)) {
    astl::vector<cString> vFlags;
    _GetFlags(mpWidget->GetText(),vFlags);
    //niPrintln(niFmt(_A("### SetSelectionFromText (%d)\n"),vFlags.size()));
    ClearSelection();
    for (tU32 i = 0; i < vFlags.size(); ++i) {
      //niPrintln(niFmt(_A("### LB FLAG: %s\n"),vFlags[i].c_str()));
      AddSelection(GetItemFromText(0,vFlags[i].c_str()));
    }
  }
  else {
    SetSelected(GetItemFromText(0,niHStr(mpWidget->GetText())));
  }
}

///////////////////////////////////////////////
void cWidgetListBox::SetTextFromSelection()
{
  if (niFlagIs(mpWidget->GetStyle(),eWidgetListBoxStyle_Multiselect)) {
    cString strText;
    // get the current flags
    astl::vector<cString> vFlags;
    _GetFlags(mpWidget->GetText(),vFlags);

    tU32 ns = GetNumSelections();
    for (tU32 i = 0; i < ns; ++i) {
      // get the flag set
      cString strFlag = GetSelectedItemText(0,i);
      strText += strFlag;
      if (i+1 != ns)
        strText += _A("|");
    }

    mpWidget->SetText(_H(strText.c_str()));
  }
  else {
    mpWidget->SetText(_H(GetItemText(0,GetSelection(0))));
  }
}

///////////////////////////////////////////////
void cWidgetListBox::UpdateSelectOnMove()
{
  if (GetModSelMode() == SEL_SINGLE &&
      (mpWidget->GetCapture() || niFlagIs(mpWidget->GetStyle(),eWidgetListBoxStyle_SelectOnMove)))
  {
    sVec2f mousePos = mpWidget->GetUIContext()->GetCursorPosition();
    mnInputSelected = eUIMessage_MouseMove;
    SelectItemByPos(mousePos);
    mnInputSelected = eInvalidHandle;
  }
}

///////////////////////////////////////////////
tF32 cWidgetListBox::_GetVScrollPos() const {
  if (!mptrVtScroll->GetVisible())
    return 0;
  return QPtr<iWidgetScrollBar>(mptrVtScroll)->GetScrollPosition();
}

///////////////////////////////////////////////
tF32 cWidgetListBox::_GetHScrollPos() const {
  if (!mptrHzScroll->GetVisible())
    return 0;
  return QPtr<iWidgetScrollBar>(mptrHzScroll)->GetScrollPosition();
}

///////////////////////////////////////////////
tF32 cWidgetListBox::_GetScrolledBaseX() const {
  tF32 wscrollpos = _GetHScrollPos();
  iFont *font = mpWidget->GetFont();
  tF32 fw = (tF32)font->GetMaxCharWidth();
  tF32 basex = (tF32)(- (wscrollpos * fw)) + LB_TEXT_X_OFFSET;
  return ni::UnitSnapf(basex);
}

///////////////////////////////////////////////
void __stdcall cWidgetListBox::SetMaxNumItems(tU32 anMaxItems)  {
  mnMaxNumItems = anMaxItems;
  _RemoveOverflowingItems();
}
tU32 __stdcall cWidgetListBox::GetMaxNumItems() const {
  return mnMaxNumItems;
}
