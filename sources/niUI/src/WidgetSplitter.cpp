// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetSplitter.h"

#pragma niTodo("The serialization is pretty bad, we cant use it when hand writing the UI definition.")

#define MIN_DRAG_SIZE     4
#define DRAGGINGBORDER_FIRST  (eInvalidHandle+1)
#define DRAGGINGBORDER_LEFT   (DRAGGINGBORDER_FIRST+0)
#define DRAGGINGBORDER_RIGHT  (DRAGGINGBORDER_FIRST+1)
#define DRAGGINGBORDER_TOP    (DRAGGINGBORDER_FIRST+2)
#define DRAGGINGBORDER_BOTTOM (DRAGGINGBORDER_FIRST+3)

#define FOLD_NONE         0
#define FOLD_LEFT_TOP     1
#define FOLD_RIGHT_BOTTOM 2
#define FOLD_UNFOLD       3

#define FOLD_DRAW_MARGIN  1

static __forceinline iWidget* _CreateCanvas(iWidget* apParent) {
  static tU32 _nCounter = 0;
  return apParent->GetUIContext()->CreateWidget(_A("Canvas"),apParent,sRectf(0,0,0,0),0,_H(niFmt(_A("SplitterCanvas%d"),_nCounter++)));
}

static __forceinline void _ApplyFoldDrawBorder(sRectf& r) {
  r.x += FOLD_DRAW_MARGIN;
  r.y += FOLD_DRAW_MARGIN;
  r.z -= FOLD_DRAW_MARGIN;
  r.w -= FOLD_DRAW_MARGIN;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cWidgetSplitter implementation.

///////////////////////////////////////////////
cWidgetSplitter::cWidgetSplitter(iWidget* apWidget)
  : mfMinPos(10.0f)
  , mfSize(5.0f)
  , mnDragging(eInvalidHandle)
  , mfBorderSize(0.0f)
  , mnResizableBorders(eRectEdges_All)
  , mvParentDockRectMinSize(Vec2<tF32>(20,20))
  , mnFillerIndex(eInvalidHandle)
  , mFoldMode(eWidgetSplitterFoldMode_Auto)
{
  mpWidget = apWidget;
  mvWidgets.push_back(_CreateCanvas(mpWidget));
  if (niFlagIsNot(mpWidget->GetStyle(),eWidgetSplitterStyle_Empty)) {
    AddSplitterBack(0.5f);
  }
}

///////////////////////////////////////////////
cWidgetSplitter::~cWidgetSplitter() {
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cWidgetSplitter::IsOK() const
{
  niClassIsOK(cWidgetSplitter);
  return mpWidget != NULL;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::AddSplitterFront(tF32 afPos)
{
  mvSplitters.insert(mvSplitters.begin(),sSplitter());
  mvSplitters[0].mfPosition = afPos;
  iWidget* pNewW = _CreateCanvas(mpWidget);
  mvWidgets.insert(mvWidgets.begin(),pNewW);
  UpdateSplitterRects(0);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::AddSplitterBack(tF32 afPos)
{
  mvSplitters.push_back(sSplitter());
  mvSplitters.back().mfPosition = afPos;
  iWidget* pNewW = _CreateCanvas(mpWidget);
  mvWidgets.push_back(pNewW);
  UpdateSplitterRects(mvSplitters.size()-1);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::RemoveSplitter(tU32 anIndex)
{
  if (anIndex >= GetNumSplitters()) {
    return eFalse;
  }
  if (mnDragging == anIndex) {
    mnDragging = eInvalidHandle;
  }
  FoldClear(anIndex);
  return RemoveSplitterWidget(anIndex+1);
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetSplitter::GetNumSplitters() const
{
  return mvSplitters.size();
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetSplitter::GetNumSplitterWidgets() const
{
  return mvWidgets.size();
}

///////////////////////////////////////////////
iWidget* __stdcall cWidgetSplitter::GetSplitterWidget(tU32 anIndex) const
{
  if (anIndex >= GetNumSplitterWidgets()) return NULL;
  return mvWidgets[anIndex];
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetSplitter::GetSplitterWidgetIndex(iWidget* apWidget) const
{
  for (tU32 i = 0; i < mvWidgets.size(); ++i) {
    if (mvWidgets[i] == apWidget)
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::RemoveSplitterWidget(tU32 anIndex)
{
  if (anIndex >= GetNumSplitterWidgets() || (anIndex == 0 && GetNumSplitters() == 0)) {
    return eFalse;
  }
  if (anIndex == 0) {
    mvSplitters.erase(mvSplitters.begin());
  }
  else {
    mvSplitters.erase(mvSplitters.begin()+(anIndex-1));
  }
  mvWidgets[anIndex]->SetParent(NULL);
  mvWidgets.erase(mvWidgets.begin()+anIndex);
  UpdateSplitterRects(eInvalidHandle);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::AddSplitterBefore(tU32 anWidget, tF32 afPos)
{
  if (anWidget >= GetNumSplitterWidgets()) {
    return eFalse;
  }
  if (mvSplitters.empty()) {
    return AddSplitterFront(afPos);
  }
  else {
    tU32 nNewSplitterIndex = anWidget;
    mvSplitters.insert(mvSplitters.begin()+nNewSplitterIndex,sSplitter());
    tF32 fNextSplitter = ((nNewSplitterIndex+1)>=mvSplitters.size())?1.0f:mvSplitters[nNewSplitterIndex+1].mfPosition;
    tF32 fPrevSplitter = (nNewSplitterIndex<1)?0.0f:mvSplitters[nNewSplitterIndex-1].mfPosition;
    mvSplitters[anWidget].mfPosition = (fNextSplitter-fPrevSplitter)*afPos + fPrevSplitter;
    iWidget* pNewW = _CreateCanvas(mpWidget);
    mvWidgets.insert(mvWidgets.begin()+anWidget,pNewW);
    UpdateSplitterRects(anWidget);
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::AddSplitterAfter(tU32 anWidget, tF32 afPos)
{
  if (anWidget >= GetNumSplitterWidgets()) {
    return eFalse;
  }
  if (mvSplitters.empty()) {
    return AddSplitterBack(1.0f-afPos);
  }
  else {
    tU32 nNewSplitterIndex = anWidget;
    mvSplitters.insert(mvSplitters.begin()+nNewSplitterIndex,sSplitter());
    tF32 fNextSplitter = ((nNewSplitterIndex+1)>=mvSplitters.size())?1.0f:mvSplitters[nNewSplitterIndex+1].mfPosition;
    tF32 fPrevSplitter = (nNewSplitterIndex<1)?0.0f:mvSplitters[nNewSplitterIndex-1].mfPosition;
    mvSplitters[anWidget].mfPosition = fNextSplitter-((fNextSplitter-fPrevSplitter)*afPos);
    iWidget* pNewW = _CreateCanvas(mpWidget);
    mvWidgets.insert(mvWidgets.begin()+anWidget+1,pNewW);
    UpdateSplitterRects(anWidget);
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::SetSplitterPosition(tU32 anIndex, tF32 aVal)
{
  if (anIndex >= GetNumSplitterWidgets()) return eFalse;
  mvSplitters[anIndex].mfPosition = aVal;
  UpdateSplitterRects(anIndex);
  return eTrue;
}

///////////////////////////////////////////////
tF32 __stdcall cWidgetSplitter::GetSplitterPosition(tU32 anIndex) const
{
  if (anIndex >= GetNumSplitterWidgets()) return 0.0f;
  return mvSplitters[anIndex].mfPosition;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::SetSplitterMinPosition(tF32 aVal)
{
  if (aVal < 0.0f) return eFalse;
  mfMinPos = aVal;
  UpdateSplitterRects(eInvalidHandle);
  return eTrue;
}

///////////////////////////////////////////////
tF32 __stdcall cWidgetSplitter::GetSplitterMinPosition() const
{
  return mfMinPos;
}

///////////////////////////////////////////////
tBool cWidgetSplitter::SetSplitterSize(tF32 afSize)
{
  mfSize = afSize;
  UpdateSplitterRects(eInvalidHandle);
  return eTrue;
}

///////////////////////////////////////////////
tF32 cWidgetSplitter::GetSplitterSize() const
{
  return mfSize;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::SwapSplitterWidget(tU32 anA, tU32 anB)
{
  if (anA >= GetNumSplitterWidgets()) return eFalse;
  if (anB >= GetNumSplitterWidgets()) return eFalse;
  ni::Swap(mvWidgets[anA],mvWidgets[anB]);
  UpdateSplitterRects(eInvalidHandle);
  return eTrue;
}

///////////////////////////////////////////////
void __stdcall cWidgetSplitter::SetSplitterBorderSize(tF32 afSize)
{
  mfBorderSize = afSize;
  UpdateClientRect();
}

///////////////////////////////////////////////
tF32 __stdcall cWidgetSplitter::GetSplitterBorderSize() const
{
  return mfBorderSize;
}

///////////////////////////////////////////////
void __stdcall cWidgetSplitter::SetSplitterResizableBorders(tU32 aEdges)
{
  mnResizableBorders = aEdges;
  UpdateClientRect();
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetSplitter::GetSplitterResizableBorders() const
{
  return mnResizableBorders;
}

///////////////////////////////////////////////
void __stdcall cWidgetSplitter::SetSplitterParentDockRectMinimumSize(sVec2f avMinSize)
{
  mvParentDockRectMinSize = avMinSize;
}

///////////////////////////////////////////////
sVec2f __stdcall cWidgetSplitter::GetSplitterParentDockRectMinimumSize() const
{
  return mvParentDockRectMinSize;
}

///////////////////////////////////////////////
tBool __stdcall cWidgetSplitter::OnWidgetSink(iWidget* apWidget, tU32 anMsg, const Var& aA, const Var& aB)
{
  niGuardObject((iWidgetSink*)this);
  switch (anMsg)
  {
    case eUIMessage_SkinChanged:
      {
        InitSkin();
        return eFalse;
      }
    case eUIMessage_NCPaint:
      {
        if (_SplittersEnabled() && mfBorderSize > 0.0f) {
          iCanvas* c = VarQueryInterface<iCanvas>(aB);
          if (!c) return eFalse;
          // left
          if (niFlagIs(mnResizableBorders,eRectEdges_Left)) {
            PushBorder(c,DRAGGINGBORDER_LEFT,eTrue);
          }
          // right
          else if (niFlagIs(mnResizableBorders,eRectEdges_Right)) {
            PushBorder(c,DRAGGINGBORDER_RIGHT,eTrue);
          }
          // top
          if (niFlagIs(mnResizableBorders,eRectEdges_Top)) {
            PushBorder(c,DRAGGINGBORDER_TOP,eTrue);
          }
          // bottom
          if (niFlagIs(mnResizableBorders,eRectEdges_Bottom)) {
            PushBorder(c,DRAGGINGBORDER_BOTTOM,eTrue);
          }
        }
        break;
      }
    case eUIMessage_Paint:
      {
        if (_SplittersEnabled() && !mvSplitters.empty()) {
          iCanvas* c = VarQueryInterface<iCanvas>(aB);
          if (!c) return eFalse;
          niLoop(i,mvSplitters.size()) {
            PushBorder(c,i,eFalse);
          }
        }
        break;
      }
    case eUIMessage_NCSize: {
      UpdateClientRect();
      break;
    }
    case eUIMessage_Layout: {
      UpdateSplitterRects(eInvalidHandle);
      return eFalse;
    }
    case eUIMessage_LeftClickDown:
    case eUIMessage_NCLeftClickDown:
      {
        if (!_SplittersEnabled()) return eFalse;
        if (niFlagIsNot(mpWidget->GetStyle(),eWidgetSplitterStyle_NoCursorResize)) {
          sVec2f vMousePos;
          if (anMsg == eUIMessage_NCLeftClickDown) {
            vMousePos = *(sVec2f*)aB.mV2F;
          }
          else {
            vMousePos = *(sVec2f*)aA.mV2F;
          }
          tU32 nI, nF;
          mnDragging = eInvalidHandle;
          if (TestIntersection(vMousePos,nI,nF)) {
            if (!nF) {
              mnDragging = nI;
              mpWidget->SetCapture(eTrue);
            }
            else {
              FoldSplitter(nI,nF==FOLD_LEFT_TOP?eTrue:eFalse);
            }
          }
        }
        break;
      }
    case eUIMessage_NCLeftClickUp:
    case eUIMessage_LeftClickUp:
      {
        if (!_SplittersEnabled()) return eFalse;
        if (mnDragging != eInvalidHandle) {
          mnDragging = eInvalidHandle;
          mpWidget->SetCapture(eFalse);
          apWidget->SendMessage(eUIMessage_SetCursor,aA,aB);
        }
        break;
      }
    case eUIMessage_NCMouseMove:
    case eUIMessage_MouseMove:
      {
        if (!_SplittersEnabled()) return eFalse;
        if (mnDragging != eInvalidHandle) {
          FoldClear(mnDragging);
          if (mnDragging >= DRAGGINGBORDER_FIRST) {
            sRectf rect = mpWidget->GetAbsoluteRect();

            sVec2f mousepos = aB.GetVec2f();
            mousepos += mpWidget->GetAbsolutePosition();

            iWidget* pParent = mpWidget->GetParent();
            if (mpWidget->GetDockStyle() == eWidgetDockStyle_None) {
              pParent = NULL;
            }

            sRectf rectDockFill(0,0,0,0);
            if (pParent) {
              rectDockFill = pParent->GetDockFillRect();
              rectDockFill += pParent->GetAbsolutePosition()+pParent->GetClientPosition();
            }

            switch (mnDragging) {
              case DRAGGINGBORDER_LEFT:
                {
                  tF32 fNewPos = mousepos.x;
                  if (pParent && mvParentDockRectMinSize.x >= 0.0f  &&
                      fNewPos < (rectDockFill.GetLeft()+mvParentDockRectMinSize.x)) {
                    fNewPos = rectDockFill.GetLeft()+mvParentDockRectMinSize.x;
                  }
                  rect.SetLeft(fNewPos);
                  break;
                }
              case DRAGGINGBORDER_RIGHT:
                {
                  tF32 fNewPos = mousepos.x;
                  if (pParent && mvParentDockRectMinSize.x >= 0.0f &&
                      fNewPos > (rectDockFill.GetRight()-mvParentDockRectMinSize.x)) {
                    fNewPos = rectDockFill.GetRight()-mvParentDockRectMinSize.x;
                  }
                  rect.SetRight(fNewPos);
                  break;
                }
              case DRAGGINGBORDER_TOP:
                {
                  tF32 fNewPos = mousepos.y;
                  if (pParent && mvParentDockRectMinSize.y >= 0.0f &&
                      fNewPos < (rectDockFill.GetTop()+mvParentDockRectMinSize.y)) {
                    fNewPos = rectDockFill.GetTop()+mvParentDockRectMinSize.y;
                  }
                  rect.SetTop(fNewPos);
                  break;
                }
              case DRAGGINGBORDER_BOTTOM:
                {
                  tF32 fNewPos = mousepos.y;
                  if (pParent && mvParentDockRectMinSize.y >= 0.0f &&
                      fNewPos > (rectDockFill.GetBottom()-mvParentDockRectMinSize.y)) {
                    fNewPos = rectDockFill.GetBottom()-mvParentDockRectMinSize.y;
                  }
                  rect.SetBottom(fNewPos);
                  break;
                }
            }

            //niTrace(niFmt(_A("NEWABSRECT: %.2f,%.2f,%.2f,%.2f\n"),rect.Left(),rect.Top(),rect.GetWidth(),rect.GetHeight()));
            mpWidget->SetAbsoluteRect(rect);
          }
          else {
            sVec2f mousepos;
            mousepos.x = aA.mV2F[0];
            mousepos.y = aA.mV2F[1];
            if (niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_Horizontal)) {
              SetSplitterPosition(mnDragging,mousepos.y/mpWidget->GetClientSize().y);
            }
            else {
              SetSplitterPosition(mnDragging,mousepos.x/mpWidget->GetClientSize().x);
            }
          }
        }
        break;
      }
    case eUIMessage_SetCursor:
      {
        if (!_SplittersEnabled()) return eFalse;

        sVec2f vMousePos = *((sVec2f*)aA.mV2F);
        if (mnDragging != eInvalidHandle) {
          apWidget->GetUIContext()->SetCursor(apWidget->FindSkinCursor(NULL,NULL,_H("Hand")));
          return eTrue;
        }

        if (mnDragging != eInvalidHandle || niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_NoCursorResize))
          return eTrue;

        tU32 nI, nF;
        if (TestIntersection(vMousePos+mpWidget->GetClientPosition(),nI,nF)) {
          if (GetIsZoneHorizontal(nI)) {
            switch (nF) {
              case FOLD_UNFOLD:   apWidget->GetUIContext()->SetCursor(skin.curArrowBiVt); break;
              case FOLD_LEFT_TOP:     apWidget->GetUIContext()->SetCursor(skin.curArrowUp); break;
              case FOLD_RIGHT_BOTTOM: apWidget->GetUIContext()->SetCursor(skin.curArrowDown); break;
              default:        apWidget->GetUIContext()->SetCursor(skin.curResizeVt); break;
            }
          }
          else {
            switch (nF) {
              case FOLD_UNFOLD:   apWidget->GetUIContext()->SetCursor(skin.curArrowBiHz); break;
              case FOLD_LEFT_TOP:     apWidget->GetUIContext()->SetCursor(skin.curArrowLeft); break;
              case FOLD_RIGHT_BOTTOM: apWidget->GetUIContext()->SetCursor(skin.curArrowRight); break;
              default:        apWidget->GetUIContext()->SetCursor(skin.curResizeHz); break;
            }
          }
        }
        else {
          return eFalse;
        }
        break;
      }
    case eUIMessage_SerializeWidget:
      {
        Ptr<iDataTable> ptrDT = ni::VarQueryInterface<iDataTable>(aA);
        niAssert(ptrDT.IsOK());
        tU32 nFlags = aB.mU32;
        if (niFlagIs(nFlags,eWidgetSerializeFlags_Write)) {
          Ptr<iDataTableWriteStack> dt = ni::CreateDataTableWriteStack(ptrDT);
          ptrDT->SetFloat(_A("splitter_min_pos"),mfMinPos);
          ptrDT->SetFloat(_A("splitter_size"),mfSize);
          ptrDT->SetFloat(_A("splitter_border_size"),mfBorderSize);
          ptrDT->SetInt(_A("splitter_resizable_borders"),mnResizableBorders);
          ptrDT->SetVec2(_A("splitter_parent_dock_rect_min_size"),mvParentDockRectMinSize);
          ptrDT->SetInt(_A("num_splitters"),mvSplitters.size());
          for (tSplitterVec::iterator itS = mvSplitters.begin(); itS != mvSplitters.end(); ++itS) {
            Ptr<iDataTable> ptrNewDT = ni::CreateDataTable(_A("Splitter"));
            ptrNewDT->SetFloat(_A("position"),itS->mfPosition);
            ptrNewDT->SetVec4(_A("rect"),itS->mRect);
            ptrDT->AddChild(ptrNewDT);
          }
          ptrDT->SetInt(_A("num_splitter_widgets"),mvWidgets.size());
          for (tWidgetVec::iterator itW = mvWidgets.begin(); itW != mvWidgets.end(); ++itW) {
            Ptr<iDataTable> ptrNewDT = ni::CreateDataTable(_A("SplitterWidget"));
            iWidget* pW = *itW;
            tU32 k = 0;
            for ( ; k < mpWidget->GetNumChildren(); ++k) {
              if (mpWidget->GetChildFromIndex(k) == pW)
                break;
            }
            niAssert(k!=mpWidget->GetNumChildren());
            ptrNewDT->SetInt(_A("child_index"),k);
            ptrNewDT->SetString(_A("id"),HStringGetStringEmpty(pW->GetID()));
            ptrDT->AddChild(ptrNewDT);
          }
          if (!mmapFoldMap.empty()) {
            dt->Push(_A("FoldMap"));
            for (tFoldMap::iterator itF = mmapFoldMap.begin(); itF != mmapFoldMap.end(); ++itF) {
              if (dt->PushNew(_A("Fold"))) {
                dt->SetInt(_A("splitter"),itF->first);
                dt->SetFloat(_A("pos"),itF->second.first);
                dt->SetInt(_A("lefttop"),itF->second.second);
                dt->Pop();
              }
            }
            dt->Pop();
          }
        }
        else {
          Ptr<iDataTableReadStack> dt = ni::CreateDataTableReadStack(ptrDT);
          ClearSplitters();
          mfMinPos = ptrDT->GetFloat(_A("splitter_min_pos"));
          mfSize = ptrDT->GetFloat(_A("splitter_size"));
          mfBorderSize = ptrDT->GetFloat(_A("splitter_border_size"));
          mnResizableBorders = ptrDT->GetInt(_A("splitter_resizable_borders"));
          mvParentDockRectMinSize = ptrDT->GetVec2(_A("splitter_parent_dock_rect_min_size"));
          tU32 nChildTable = 0, i;
          tU32 nNumSplitters = ptrDT->GetInt(_A("num_splitters"));
          for (i = 0; i < nNumSplitters; ++i, ++nChildTable) {
            AddSplitterBack(ptrDT->GetChildFromIndex(nChildTable)->GetFloat(_A("position")));
          }
          tU32 nNumSplitterWidgets = ptrDT->GetInt(_A("num_splitter_widgets"));
          for (i = 0; i < nNumSplitterWidgets; ++i, ++nChildTable) {
            mvWidgets[i] = mpWidget->GetChildFromIndex(ptrDT->GetChildFromIndex(nChildTable)->GetInt(_A("child_index")));
            mvWidgets[i]->SetID(_H(ptrDT->GetChildFromIndex(nChildTable)->GetString(_A("id"))));
          }
          if (dt->PushFail(_A("FoldMap"))) {
            mmapFoldMap.clear();
            niLoop(i,dt->GetNumChildren()) {
              dt->PushChild(i);
              // we cant allow invalid fold node to be loaded, so we check that all properties are set
              if (ni::StrCmp(dt->GetName(),_A("Fold")) == 0 &&
                  dt->GetTop()->GetPropertyIndex(_A("splitter")) != eInvalidHandle &&
                  dt->GetTop()->GetPropertyIndex(_A("pos")) != eInvalidHandle &&
                  dt->GetTop()->GetPropertyIndex(_A("lefttop")) != eInvalidHandle)
              {
                tU32 id = (tU32)dt->GetInt(_A("splitter"));
                tF32 pos = dt->GetFloat(_A("pos"));
                tBool leftTop = dt->GetInt(_A("lefttop"))?eTrue:eFalse;
                astl::upsert(mmapFoldMap,id,astl::make_pair(pos,leftTop));
              }
              dt->Pop();
            }
            dt->Pop();
          }
          UpdateClientRect();
        }
        break;
      }
    default:
      return eFalse;
  }
  return eTrue;
}

///////////////////////////////////////////////
void cWidgetSplitter::ClearSplitters()
{
  while (GetNumSplitters()) {
    RemoveSplitter(GetNumSplitters()-1);
  }
}

///////////////////////////////////////////////
void cWidgetSplitter::UpdateSplitterRects(tU32 anPivotSplitter)
{
  //niTrace(niFmt(_A("$$$ UpdateSplitterRect: %.2f\n"),mfPosition));

  if (mvWidgets.size() != mvSplitters.size()+1)
    return;

  if (mnFillerIndex < mvSplitters.size()) {
    niLoop(i,mvWidgets.size()) {
      iWidget* w = mvWidgets[i];
      if (i == mnFillerIndex) {
        w->SetPosition(sVec2f::Zero());
        w->SetSize(mpWidget->GetClientSize());
        w->SetEnabled(eTrue);
        w->SetVisible(eTrue);
      }
      else {
        w->SetEnabled(eFalse);
        w->SetVisible(eFalse);
      }
    }
  }
  else if (mvSplitters.empty()) {
    mvWidgets[0]->SetPosition(sVec2f::Zero());
    mvWidgets[0]->SetSize(mpWidget->GetClientSize());
  }
  else {
    sVec2f vSize = mpWidget->GetClientSize();
    if (vSize.x < 1.0f) vSize.x = 1.0f;
    if (vSize.y < 1.0f) vSize.y = 1.0f;
    tSplitterVec::iterator itS;
    tF32 fSize = ni::Max(1.0f,niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_Horizontal)?vSize.y:vSize.x);

    // apply the splitter's position constraints
    tF32 fMinPosPercent = mfMinPos/fSize;
    if (anPivotSplitter != eInvalidHandle) {
      niAssert(anPivotSplitter < mvSplitters.size());
      if (anPivotSplitter > 0) {
        for (tI32 i = (tI32)anPivotSplitter-1; i >= 0; --i) {
          sSplitter& s = mvSplitters[i];
          sSplitter& n = mvSplitters[i+1];
          if (s.mfPosition > n.mfPosition-fMinPosPercent)
            s.mfPosition = n.mfPosition-fMinPosPercent;
        }
      }
      if (anPivotSplitter < mvSplitters.size()-1) {
        for (tU32 i = anPivotSplitter; i < mvSplitters.size()-1; ++i) {
          sSplitter& p = mvSplitters[i];
          sSplitter& s = mvSplitters[i+1];
          if (s.mfPosition < p.mfPosition+fMinPosPercent)
            s.mfPosition = p.mfPosition+fMinPosPercent;
        }
      }
    }

    // update the rectangles
    for (itS = mvSplitters.begin(); itS != mvSplitters.end(); ++itS) {
      tF32 fRectTop = (itS->mfPosition*fSize)-(mfSize/2);
      if (niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_Horizontal)) {
        itS->mRect.SetTop(fRectTop);
        itS->mRect.SetBottom(fRectTop+mfSize);
        itS->mRect.SetLeft(0.0f);
        itS->mRect.SetRight(vSize.x);
      }
      else {
        itS->mRect.SetLeft(fRectTop);
        itS->mRect.SetRight(fRectTop+mfSize);
        itS->mRect.SetTop(0.0f);
        itS->mRect.SetBottom(vSize.y);
      }
    }

    // update the widget's rectangles
    tF32 fRectTop = 0.0f;
    tWidgetVec::iterator itW = mvWidgets.begin();

    if (niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_Horizontal)) {
      for (itS = mvSplitters.begin(); itS != mvSplitters.end(); ++itS, ++itW) {
        (*itW)->SetRect(sRectf(0,fRectTop,vSize.x,itS->mRect.GetTop()-fRectTop));
        fRectTop = itS->mRect.GetBottom();
      }
      // update the last widget
      (*itW)->SetRect(sRectf(0,fRectTop,vSize.x,vSize.y-fRectTop));
    }
    else {
      for (itS = mvSplitters.begin(); itS != mvSplitters.end(); ++itS, ++itW) {
        (*itW)->SetRect(sRectf(fRectTop,0,itS->mRect.GetLeft()-fRectTop,vSize.y));
        fRectTop = itS->mRect.GetRight();
      }
      // update the last widget
      (*itW)->SetRect(sRectf(fRectTop,0,vSize.x-fRectTop,vSize.y));
    }
  }
}

///////////////////////////////////////////////
void cWidgetSplitter::UpdateClientRect()
{
  sVec2f vSize = mpWidget->GetSize();
  sRectf newRect = sRectf(0,0,vSize.x,vSize.y);
  if (_SplittersEnabled()) {
    if (niFlagIs(mnResizableBorders,eRectEdges_Left)) {
      newRect.SetLeft(mfBorderSize);
    }
    if (niFlagIs(mnResizableBorders,eRectEdges_Right)) {
      newRect.SetRight(vSize.x-mfBorderSize);
    }
    if (niFlagIs(mnResizableBorders,eRectEdges_Top)) {
      newRect.SetTop(mfBorderSize);
    }
    if (niFlagIs(mnResizableBorders,eRectEdges_Bottom)) {
      newRect.SetBottom(vSize.y-mfBorderSize);
    }
  }
  mpWidget->SetClientRect(newRect);
  UpdateSplitterRects(eInvalidHandle);
}

///////////////////////////////////////////////
void cWidgetSplitter::PushBorder(iCanvas* c, tU32 anZone, tBool abBorder)
{
  sRectf rect = GetZoneRect(anZone,0);
  if (_CanFold()) {
    if (FoldIsFolded(anZone)) {
      sRectf rectFoldUnflod = GetZoneRect(anZone,FOLD_UNFOLD);
      _ApplyFoldDrawBorder(rectFoldUnflod);
      c->BlitFillAlpha(rectFoldUnflod,skin.colFoldUnfold);
    }
    else {
      if (_CanFoldLeftTop()) {
        sRectf rectFoldLeftTop = GetZoneRect(anZone,FOLD_LEFT_TOP);
        _ApplyFoldDrawBorder(rectFoldLeftTop);
        c->BlitFillAlpha(rectFoldLeftTop,skin.colFoldLeftTop);
      }
      if (_CanFoldRightBottom()) {
        sRectf rectFoldRightBottom = GetZoneRect(anZone,FOLD_RIGHT_BOTTOM);
        _ApplyFoldDrawBorder(rectFoldRightBottom);
        c->BlitFillAlpha(rectFoldRightBottom,skin.colFoldRightBottom);
      }
    }
  }
  else {
    c->BlitFill(rect,skin.colFill);
  }
}

///////////////////////////////////////////////
void cWidgetSplitter::InitSkin()
{
  skin.curResizeHz   = mpWidget->FindSkinCursor(NULL,NULL,_H("ResizeHorizontal"));
  skin.curResizeVt   = mpWidget->FindSkinCursor(NULL,NULL,_H("ResizeVertical"));
  skin.curArrowUp    = mpWidget->FindSkinCursor(NULL,NULL,_H("ArrowUp"));
  skin.curArrowDown  = mpWidget->FindSkinCursor(NULL,NULL,_H("ArrowDown"));
  skin.curArrowLeft  = mpWidget->FindSkinCursor(NULL,NULL,_H("ArrowLeft"));
  skin.curArrowRight = mpWidget->FindSkinCursor(NULL,NULL,_H("ArrowRight"));
  skin.curArrowBiVt  = mpWidget->FindSkinCursor(NULL,NULL,_H("ArrowBiVertical"));
  skin.curArrowBiHz  = mpWidget->FindSkinCursor(NULL,NULL,_H("ArrowBiHorizontal"));

  const sColor4f defColor = sColor4f::White();
  skin.colBorder = ULColorBuild(mpWidget->FindSkinColor(defColor,NULL,NULL,_H("Border")));
  skin.colFill = ULColorBuild(mpWidget->FindSkinColor(defColor,NULL,NULL,_H("Fill")));
  skin.colFoldLeftTop = ULColorBuild(mpWidget->FindSkinColor(defColor,NULL,NULL,_H("FoldLeftTop")));
  skin.colFoldRightBottom = ULColorBuild(mpWidget->FindSkinColor(defColor,NULL,NULL,_H("FoldRightBottom")));
  skin.colFoldUnfold = ULColorBuild(mpWidget->FindSkinColor(defColor,NULL,NULL,_H("FoldUnfold")));
}

///////////////////////////////////////////////
void cWidgetSplitter::FoldSplitter(tU32 anID, tBool abLeftTop)
{
  tFoldMap::iterator it = mmapFoldMap.find(anID);
  if (it != mmapFoldMap.end()) {
    tF32 fRestorePos = it->second.first;
    abLeftTop = it->second.second;
    switch (anID) {
      case DRAGGINGBORDER_LEFT: {
        sRectf rect = mpWidget->GetAbsoluteRect();
        rect.SetLeft(fRestorePos);
        mpWidget->SetAbsoluteRect(rect);
        break;
      }
      case DRAGGINGBORDER_RIGHT: {
        sRectf rect = mpWidget->GetAbsoluteRect();
        rect.SetRight(fRestorePos);
        mpWidget->SetAbsoluteRect(rect);
        break;
      }
      case DRAGGINGBORDER_BOTTOM: {
        sRectf rect = mpWidget->GetAbsoluteRect();
        rect.SetBottom(fRestorePos);
        mpWidget->SetAbsoluteRect(rect);
        break;
      }
      case DRAGGINGBORDER_TOP: {
        sRectf rect = mpWidget->GetAbsoluteRect();
        rect.SetTop(fRestorePos);
        mpWidget->SetAbsoluteRect(rect);
        break;
      }
      default: {
        SetSplitterPosition(anID,fRestorePos);
        break;
      }
    }
    // Unfolding
    mmapFoldMap.erase(it);
  }
  else {
    tF32 fToSave = 0.0f;

    iWidget* pParent = mpWidget->GetParent();
    if (mpWidget->GetDockStyle() == eWidgetDockStyle_None) {
      pParent = NULL;
    }
    sRectf rectDockFill(0,0,0,0);
    if (pParent) {
      rectDockFill = pParent->GetDockFillRect();
      rectDockFill += pParent->GetAbsolutePosition()+pParent->GetClientPosition();
    }

    const tF32 foldedSize = mfSize+2; // kSplitterFoldedSize;
    switch (anID) {
      case DRAGGINGBORDER_LEFT: {
        fToSave = mpWidget->GetAbsoluteRect().GetLeft();
        sRectf rect = mpWidget->GetAbsoluteRect();
        if (abLeftTop) {
          if (pParent) {
            rect.SetLeft(rectDockFill.GetLeft()+foldedSize);
          }
        }
        else {
          rect.SetLeft(rect.GetRight()-foldedSize);
        }
        mpWidget->SetAbsoluteRect(rect);
        break;
      }
      case DRAGGINGBORDER_RIGHT: {
        fToSave = mpWidget->GetAbsoluteRect().GetRight();
        sRectf rect = mpWidget->GetAbsoluteRect();
        if (abLeftTop) {
          rect.SetRight(rect.GetLeft()+foldedSize);
        }
        else {
          if (pParent) {
            rect.SetRight(rectDockFill.GetRight()-foldedSize);
          }
        }
        mpWidget->SetAbsoluteRect(rect);
        break;
      }
      case DRAGGINGBORDER_BOTTOM: {
        fToSave = mpWidget->GetAbsoluteRect().GetBottom();
        sRectf rect = mpWidget->GetAbsoluteRect();
        if (abLeftTop) {
          rect.SetBottom(rect.GetTop()+foldedSize);
        }
        else {
          if (pParent) {
            rect.SetBottom(rectDockFill.GetBottom()-foldedSize);
          }
        }
        mpWidget->SetAbsoluteRect(rect);
        break;
      }
      case DRAGGINGBORDER_TOP: {
        fToSave = mpWidget->GetAbsoluteRect().GetTop();
        sRectf rect = mpWidget->GetAbsoluteRect();
        if (abLeftTop) {
          if (pParent) {
            rect.SetTop(rectDockFill.GetTop()+foldedSize);
          }
        }
        else {
          rect.SetTop(rect.GetBottom()-foldedSize);
        }
        mpWidget->SetAbsoluteRect(rect);
        break;
      }
      default: {
        fToSave = GetSplitterPosition(anID);
        if (abLeftTop) {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_Horizontal)) {
            SetSplitterPosition(anID,(foldedSize)/mpWidget->GetSize().y);
          }
          else {
            SetSplitterPosition(anID,(foldedSize)/mpWidget->GetSize().x);
          }
        }
        else {
          if (niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_Horizontal)) {
            SetSplitterPosition(anID,(mpWidget->GetSize().y-foldedSize)/mpWidget->GetSize().y);
          }
          else {
            SetSplitterPosition(anID,(mpWidget->GetSize().x-foldedSize)/mpWidget->GetSize().x);
          }
        }
        break;
      }
    }
    // Folding
    astl::upsert(mmapFoldMap,anID,astl::make_pair(fToSave,abLeftTop));
  }
}

///////////////////////////////////////////////
void cWidgetSplitter::FoldClear(tU32 anID)
{
  tFoldMap::iterator it = mmapFoldMap.find(anID);
  if (it != mmapFoldMap.end()) {
    mmapFoldMap.erase(it);
  }
}

///////////////////////////////////////////////
tBool cWidgetSplitter::FoldIsFolded(tU32 anID) const
{
  return mmapFoldMap.find(anID) != mmapFoldMap.end();
}

///////////////////////////////////////////////
tBool cWidgetSplitter::TestRect(const sVec2f& avMousePos, tU32 anZone, tU32& anFold) const
{
  anFold = FOLD_NONE;
  if (_CanFold()) {
    if (FoldIsFolded(anZone)) {
      if (GetZoneRect(anZone,FOLD_UNFOLD).Intersect(avMousePos)) {
        anFold = FOLD_UNFOLD;
        return eTrue;
      }
    }
    else if (_CanFoldLeftTop() && GetZoneRect(anZone,FOLD_LEFT_TOP).Intersect(avMousePos)) {
      anFold = FOLD_LEFT_TOP;
      return eTrue;
    }
    else if (_CanFoldRightBottom() && GetZoneRect(anZone,FOLD_RIGHT_BOTTOM).Intersect(avMousePos)) {
      anFold = FOLD_RIGHT_BOTTOM;
      return eTrue;
    }
  }
  if (GetZoneRect(anZone,FOLD_NONE).Intersect(avMousePos)) {
    anFold = FOLD_NONE;
    return eTrue;
  }
  return eFalse;
}

///////////////////////////////////////////////
tBool cWidgetSplitter::TestIntersection(const sVec2f& avNCMousePos, tU32& anIntersect, tU32& anFold) const
{
  anIntersect = eInvalidHandle;

  // test the internal splitters
  {
    const sVec2f vMousePos = avNCMousePos-mpWidget->GetClientPosition();
    if (niFlagIsNot(mpWidget->GetStyle(),eWidgetSplitterStyle_NoCursorResize)) {
      niLoop(i,mvSplitters.size()) {
        if (TestRect(vMousePos,i,anFold)) {
          anIntersect = i;
          return eTrue;
        }
      }
    }
  }

  // test the borders
  {
    const sVec2f vMousePos = avNCMousePos;

    if (niFlagIs(mnResizableBorders,eRectEdges_Left) && (anIntersect == eInvalidHandle)) {
      if (TestRect(vMousePos,DRAGGINGBORDER_LEFT,anFold)) {
        anIntersect = DRAGGINGBORDER_LEFT;
        return eTrue;
      }
    }

    if (niFlagIs(mnResizableBorders,eRectEdges_Right) && (anIntersect == eInvalidHandle)) {
      if (TestRect(vMousePos,DRAGGINGBORDER_RIGHT,anFold)) {
        anIntersect = DRAGGINGBORDER_RIGHT;
        return eTrue;
      }
    }

    if (niFlagIs(mnResizableBorders,eRectEdges_Top) && (anIntersect == eInvalidHandle)) {
      if (TestRect(vMousePos,DRAGGINGBORDER_TOP,anFold)) {
        anIntersect = DRAGGINGBORDER_TOP;
        return eTrue;
      }
    }

    if (niFlagIs(mnResizableBorders,eRectEdges_Bottom) && (anIntersect == eInvalidHandle)) {
      if (TestRect(vMousePos,DRAGGINGBORDER_BOTTOM,anFold)) {
        anIntersect = DRAGGINGBORDER_BOTTOM;
        return eTrue;
      }
    }
  }

  return eFalse;
}

///////////////////////////////////////////////
sRectf cWidgetSplitter::GetZoneRect(tU32 anZone, tU32 anFold) const
{
  tBool bHorizontal = eFalse;
  sRectf rect = sRectf::Null();
  sVec2f vSize = mpWidget->GetSize();
  switch (anZone) {
    case DRAGGINGBORDER_LEFT:
      rect = sRectf(0,0,mfBorderSize,vSize.y);
      bHorizontal = eFalse;
      break;
    case DRAGGINGBORDER_RIGHT:
      rect = sRectf(vSize.x-mfBorderSize,0,mfBorderSize,vSize.y);
      bHorizontal = eFalse;
      break;
    case DRAGGINGBORDER_TOP:
      rect = sRectf(0,0,vSize.x,mfBorderSize);
      bHorizontal = eTrue;
      break;
    case DRAGGINGBORDER_BOTTOM:
      rect = sRectf(0,vSize.y-mfBorderSize,vSize.x,mfBorderSize);
      bHorizontal = eTrue;
      break;
    default:
      rect = mvSplitters[anZone].mRect;
      bHorizontal = niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_Horizontal);
      break;
  }

  if (anFold) {
    sRectf foldZone = rect;
    if (anFold == FOLD_UNFOLD || _CanFoldOnlyOne()) {
      if (bHorizontal) {
        foldZone.Left()  = rect.Left() + ((rect.GetWidth()/2)-(kSplitterFoldHandleSize/2));
        foldZone.Right() = rect.Left() + ((rect.GetWidth()/2)+(kSplitterFoldHandleSize/2));
      }
      else {
        foldZone.Top()    = rect.Top() + ((rect.GetHeight()/2)-(kSplitterFoldHandleSize/2));
        foldZone.Bottom() = rect.Top() + ((rect.GetHeight()/2)+(kSplitterFoldHandleSize/2));
      }
    }
    else if (anFold == FOLD_LEFT_TOP) {
      if (bHorizontal) {
        foldZone.Left()  = rect.Left() + ((rect.GetWidth()/2)-(kSplitterFoldHandleSize/2));
        foldZone.Right() = foldZone.Left() + (kSplitterFoldHandleSize/2);
      }
      else {
        foldZone.Top()    = rect.Top() + ((rect.GetHeight()/2)-(kSplitterFoldHandleSize/2));
        foldZone.Bottom() = foldZone.Top() + (kSplitterFoldHandleSize/2);
      }
    }
    else if (anFold == FOLD_RIGHT_BOTTOM) {
      if (bHorizontal) {
        foldZone.Left()  = rect.Left() + ((rect.GetWidth()/2)-(kSplitterFoldHandleSize/2)) + (kSplitterFoldHandleSize/2);
        foldZone.Right() = foldZone.Left() + (kSplitterFoldHandleSize/2);
      }
      else {
        foldZone.Top()    = rect.Top() + ((rect.GetHeight()/2)-(kSplitterFoldHandleSize/2)) + (kSplitterFoldHandleSize/2);
        foldZone.Bottom() = foldZone.Top() + (kSplitterFoldHandleSize/2);
      }
    }
    rect = foldZone;
  }

  return rect;
}

///////////////////////////////////////////////
tBool cWidgetSplitter::GetIsZoneHorizontal(tU32 anZone) const
{
  switch (anZone) {
    case DRAGGINGBORDER_LEFT:
    case DRAGGINGBORDER_RIGHT:
      return eFalse;
    case DRAGGINGBORDER_TOP:
    case DRAGGINGBORDER_BOTTOM:
      return eTrue;
    default:
      return niFlagIs(mpWidget->GetStyle(),eWidgetSplitterStyle_Horizontal);
  }
}

///////////////////////////////////////////////
void __stdcall cWidgetSplitter::SetSplitterFillerIndex(ni::tU32 anIndex)
{
  if (mnFillerIndex == anIndex) return;
  mnFillerIndex = anIndex;
  if (_SplittersEnabled()) {
    niLoop(i,mvWidgets.size()) {
      iWidget* w = mvWidgets[i];
      w->SetEnabled(eTrue);
      w->SetVisible(eTrue);
    }
  }
  this->UpdateClientRect();
  //  this->UpdateSplitterRects(eInvalidHandle);
}

///////////////////////////////////////////////
tU32 __stdcall cWidgetSplitter::GetSplitterFillerIndex() const
{
  return mnFillerIndex;
}
