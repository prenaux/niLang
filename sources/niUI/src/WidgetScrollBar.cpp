// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetScrollBar.h"
#include "HardCodedSkin.h"

static niDefConstHString(ScrollBar_ButtonUp);
static niDefConstHString(ScrollBar_ButtonDown);
static niDefConstHString(ScrollBar_Thumb);
static niDefConstHString(Slider_Thumb);

#define _SBID(ID) niGetConstHString(ScrollBar_##ID)
#define _SLID(ID) niGetConstHString(Slider_##ID)
#define  SB_Y_MINSIZE 10

#define TIMER_SCROLL_SECS        0.02f
#define TIMER_SCROLL_SPLITS      2.0f

#define TIMER_SCROLL_PAGE_SECS   0.15f
#define TIMER_SCROLL_PAGE_SPLITS 7.0f

struct sScrollBarThumbMetrics {
  tBool bHorz;
  tBool bSlider;
  tF32 realsize;
  tF32 tsize;
  tF32 unit;

  sScrollBarThumbMetrics(cScrollBarWidget* apScrollBar, const tF32 w, const tF32 h) {
    const tU32 style = apScrollBar->mpWidget->GetStyle();
    bHorz = niFlagIs(style,eWidgetScrollBarStyle_Horz);
    bSlider = niFlagIs(style,eWidgetScrollBarStyle_Slider);
    if (!bSlider) {
      realsize = bHorz ? w : h;
      if (apScrollBar->mButtons.HasButtons()) {
        realsize -= bHorz ? (h*2) : (w*2);
      }
      tsize = ni::Clamp(
        ni::FDiv(realsize, ((apScrollBar->mfMax-apScrollBar->mfMin)/apScrollBar->mfPageSize)+1.0f),
        kfScrollBarMinThumbSize, realsize);
      unit = (realsize-tsize) / (apScrollBar->mfMax-apScrollBar->mfMin);
    }
    else {
      realsize = bHorz ? w : h;
      tsize = bHorz ?
          apScrollBar->mptrThumb->GetWidgetRect().GetWidth() :
          apScrollBar->mptrThumb->GetWidgetRect().GetHeight();
      unit = (realsize-tsize) / (apScrollBar->mfMax-apScrollBar->mfMin);
    }
  }
};

class cThumbButton : public ImplRC<iWidgetSink>
{
 public:
  cThumbButton(cScrollBarWidget *parent){ mpParent=parent; mbDragging = eFalse;}
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
  {
    niGuardObject((iWidgetSink*)this);
    switch(nMsg)
    {
      case eUIMessage_NCMouseMove:
      case eUIMessage_MouseMove:
        if(mbDragging){
          mpParent->ThumbMove(mvPivotPos-varParam1.GetVec2f());
          //mvPivotPos = ((Var&)varParam0).GetVec2f().v;
        }
        return eFalse;
      case eUIMessage_NCLeftClickDown:
      case eUIMessage_LeftClickDown:
        mbDragging = eTrue;
        mvPivotPos = ((Var&)varParam1).GetVec2f();
        apWidget->SetCapture(eTrue);
        break;
      case eUIMessage_NCLeftClickUp:
      case eUIMessage_LeftClickUp:
        apWidget->SetCapture(eFalse);
        mbDragging = eFalse;
        break;
    }
    return eFalse;
  }
  sVec2f mvPivotPos;
  tBool mbDragging;
  cScrollBarWidget *mpParent;
};

cScrollBarWidget::cScrollBarWidget(iWidget *pWidget) {
  mpWidget = pWidget;
  mfMin = 0.0f;
  mfMax = 100.0f;
  mfPageSize = 1.0f;
  mfScrollPos = 0.0f;
  mfNormalizedScrollPos = 0.0f;
  mbDragging = eFalse;
  mnScrollTimerID = 0;
  UpdateStyle();
}

cScrollBarWidget::~cScrollBarWidget() {
}

tBool cScrollBarWidget::OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
{
  niGuardObject((iWidgetSink*)this);
  switch(nMsg)
  {
    case eUIMessage_SkinChanged:
    case eUIMessage_StyleChanged: {
      UpdateStyle();
      return eFalse;
    }

    case eUIMessage_KeyDown: {
      const tU32 key = varParam0.mU32;
      if (key == eKey_Down || key == eKey_Left || key == eKey_PgUp) {
        SetScrollPosition(ComputeRoundedPosition(mfScrollPos-1));
      }
      else if (key == eKey_Up || key == eKey_Right || key == eKey_PgDn) {
        SetScrollPosition(ComputeRoundedPosition(mfScrollPos+1));
      }
      break;
    }

    case eUIMessage_NCPaint: {
      iOverlay* bk = skin.back;
      iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
      if (bk && c) {
        const tU32 style = mpWidget->GetStyle();
        const tBool bSlider = !!(style&eWidgetScrollBarStyle_Slider);
        const tBool bHorz = !!(style&eWidgetScrollBarStyle_Horz);
        if (!bSlider) {
          sRectf rect = apWidget->GetWidgetRect();
          if (mButtons.HasButtons()) {
            if (bHorz) {
              rect.x += mButtons.up->GetSize().x;
              rect.z -= mButtons.down->GetSize().x;
            }
            else {
              rect.y += mButtons.up->GetSize().y;
              rect.w -= mButtons.down->GetSize().y;
            }
          }
          c->BlitOverlayFrame(rect,bk,eRectFrameFlags_All);
        }
        else {
          sRectf rect = apWidget->GetWidgetRect();
          if (bHorz) {
            rect.SetTop((rect.GetHeight()-bk->GetSize().y)/2);
            rect.SetHeight(bk->GetSize().y);
          }
          else {
            rect.SetLeft((rect.GetWidth()-bk->GetSize().x)/2);
            rect.SetWidth(bk->GetSize().x);
          }
          c->BlitOverlay(rect,skin.back);
        }
      }
      break;
    }

    case eUIMessage_Paint: {
      tBool isSlider = !!(mpWidget->GetStyle()&eWidgetScrollBarStyle_Slider);
      if (isSlider) {
        iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
        if (c) {
          iFont* pFont = apWidget->GetFont();
          c->BlitText(
            pFont,
            apWidget->GetRect(),
            eFontFormatFlags_CenterV|eFontFormatFlags_CenterH|eFontFormatFlags_Border,
            ((ni::FuzzyEqual(ni::Floor(mfPageSize),mfPageSize,niEpsilon5)) ?
             niFmt(_A(" %g "),(tF32)mfScrollPos) :
             niFmt(_A(" %.2f "),(tF32)mfScrollPos)));
        }
      }
      break;
    }

    case eUIMessage_Size: {
      AdjustChildrenLayout(varParam0.mV2F[0],varParam0.mV2F[1]);
      break;
    }

    case eUIMessage_Command: {
      if (mButtons.HasButtons()) {
        Ptr<iWidgetCommand> ptrCmd = ni::VarQueryInterface<iWidgetCommand>(varParam0);
        niAssert(ptrCmd.IsOK());
        if (ptrCmd->GetID() == eWidgetButtonCmd_Pushed) {
          if (ptrCmd->GetSender() == mButtons.up) {
            _StartScroll(SCROLL_UP|SCROLL_PAGE);
          }
          else if(ptrCmd->GetSender() == mButtons.down) {
            _StartScroll(SCROLL_DOWN|SCROLL_PAGE);
          }
        }
        else if (ptrCmd->GetID() == eWidgetButtonCmd_UnPushed) {
          if (ptrCmd->GetSender() == mButtons.up) {
            _StopScroll();
          }
          else if(ptrCmd->GetSender() == mButtons.down) {
            _StopScroll();
          }
        }
      }
      break;
    }

    case eUIMessage_LeftClickDown: {
      tU32 style = mpWidget->GetStyle();
      if (style&eWidgetScrollBarStyle_Slider) {
        ThumbMove(mptrThumb->GetPosition()-((Var&)varParam0).GetVec2f());
      }
      else {
        mpWidget->SetCapture(eTrue);
        if (!(style&eWidgetScrollBarStyle_Horz)) {
          if (mptrThumb->GetPosition().y<varParam0.mV2F[1]) {
            _StartScroll(SCROLL_DOWN,varParam0.mV2F[1]);
          }
          else {
            _StartScroll(SCROLL_UP,varParam0.mV2F[1]);
          }
        }
        else {
          if (mptrThumb->GetPosition().x<varParam0.mV2F[0]) {
            _StartScroll(SCROLL_DOWN,varParam0.mV2F[0]);
          }
          else {
            _StartScroll(SCROLL_UP,varParam0.mV2F[0]);
          }
        }
      }
      break;
    }
    case eUIMessage_NCLeftClickUp:
    case eUIMessage_LeftClickUp: {
      _StopScroll();
      mpWidget->SetCapture(eFalse);
      break;
    }

    case eUIMessage_MouseMove:
    case eUIMessage_NCMouseMove: {
      if (mnScrollTimerID && mfScrollStopAt != niMaxF32) {
        tU32 style = mpWidget->GetStyle();
        if (!(style&eWidgetScrollBarStyle_Horz)) {
          mfScrollStopAt = varParam0.mV2F[1];
        }
        else {
          mfScrollStopAt = varParam0.mV2F[0];
        }
      }
      break;
    }

    case eUIMessage_Timer: {
      const tU32 id = varParam0.mU32;
      _DoScroll(id);
      break;
    }

    case eUIMessage_NCWheel:
    case eUIMessage_Wheel: {
      tF32 fMove = (tF32)varParam0.GetFloatValue();
      _SetScrollPosition(ComputeRoundedPosition(mfScrollPos-fMove),eTrue);
      break;
    }

    case eUIMessage_SerializeWidget: {
      Ptr<iDataTable> ptrDT = ni::VarQueryInterface<iDataTable>(varParam0);
      tU32 nFlags = varParam1.mU32;
      if (niFlagIs(nFlags,eWidgetSerializeFlags_Read)) {
        tU32 nIndex;
        nIndex = ptrDT->GetPropertyIndex(_A("scroll_range"));
        if (nIndex != eInvalidHandle) {
          SetScrollRange(ptrDT->GetVec2FromIndex(nIndex));
        }
        nIndex = ptrDT->GetPropertyIndex(_A("scroll_position"));
        if (nIndex != eInvalidHandle) {
          SetScrollPosition((tF32)ptrDT->GetFloatFromIndex(nIndex));
        }
        nIndex = ptrDT->GetPropertyIndex(_A("scroll_page_size"));
        if (nIndex != eInvalidHandle) {
          SetPageSize((tF32)ptrDT->GetFloatFromIndex(nIndex));
        }
      }
      else {
        ptrDT->SetVec2(_A("scroll_range"),GetScrollRange());
        ptrDT->SetFloat(_A("scroll_position"),GetScrollPosition());
        ptrDT->SetFloat(_A("scroll_page_size"),GetPageSize());
      }
      break;
    }

    default:
      return eFalse;
  }

  return eTrue;
}

void cScrollBarWidget::AdjustChildrenLayout(tF32 w, tF32 h)
{
  const sScrollBarThumbMetrics thumbSize(this,w,h);
  const tF32 fScrollPos = mfScrollPos-mfMin;
  if (!thumbSize.bHorz) {
    // VERTICAL
    if (!thumbSize.bSlider) {
      if (mButtons.HasButtons()) {
        mButtons.up->SetRect(sRectf(0,0,w,w));
        mButtons.down->SetRect(sRectf(0,h-w,w,w));
        mptrThumb->SetRect(sRectf(0,w+(fScrollPos*thumbSize.unit),w,thumbSize.tsize));
      }
      else {
        mptrThumb->SetRect(sRectf(0,fScrollPos*thumbSize.unit,w,thumbSize.tsize));
      }
    }
    else {
      tF32 tw = mptrThumb->GetWidgetRect().GetWidth();
      sRectf rect = mptrThumb->GetWidgetRect();
      mptrThumb->SetRect(Rectf(
        (w*0.5f)-(tw*0.5f),
        (thumbSize.realsize-thumbSize.tsize)-(fScrollPos*thumbSize.unit),
        rect.GetWidth(), rect.GetHeight()));
    }

  }
  else {
    // HORIZONTAL
    if (!thumbSize.bSlider) {
      if (mButtons.HasButtons()) {
        mButtons.up->SetRect(sRectf(0,0,h,h));
        mButtons.down->SetRect(sRectf(w-h,0,h,h));
        mptrThumb->SetRect(sRectf(h+(fScrollPos*thumbSize.unit),0,thumbSize.tsize,h));
      }
      else {
        mptrThumb->SetRect(sRectf(fScrollPos*thumbSize.unit,0,thumbSize.tsize,h));
      }
    }
    else {
      tF32 th = mptrThumb->GetWidgetRect().GetHeight();
      sRectf rect = mptrThumb->GetWidgetRect();
      mptrThumb->SetRect(sRectf(
        fScrollPos*thumbSize.unit,
        (h*0.5f)-(th*0.5f),
        rect.GetWidth(),rect.GetHeight()));
    }
  }
}

void cScrollBarWidget::ThumbMove(const sVec2f &pos)
{
  const sScrollBarThumbMetrics thumbSize(this,mpWidget->GetWidgetRect().GetWidth(),mpWidget->GetWidgetRect().GetHeight());
  if (thumbSize.bHorz) {
    SetScrollPosition(ComputeRoundedPosition(mfScrollPos-(pos.x/thumbSize.unit)));
  }
  else {
    SetScrollPosition(ComputeRoundedPosition(mfScrollPos-(pos.y/thumbSize.unit)));
  }
}

void cScrollBarWidget::SetScrollRange(const sVec2f& avRange)
{
  if (avRange.x == mfMin && avRange.y == mfMax)
    return;
  mfMin = ni::Min(avRange.x,avRange.y);
  mfMax = ni::Max(avRange.x,avRange.y);
  if (mfMin == mfMax) {
    mfMax += 1.0f; // <!> Range cannot be zero, removing this will break the scroll bar is subtle ways.
    if (mptrThumb.IsOK()) {
      mptrThumb->SetVisible(eFalse);
      mptrThumb->SetEnabled(eFalse);
    }
    if (mButtons.HasButtons()) {
      mButtons.up->SetEnabled(eFalse);
      mButtons.down->SetEnabled(eFalse);
    }
  }
  else {
    if (mptrThumb.IsOK()) {
      mptrThumb->SetEnabled(eTrue);
      mptrThumb->SetVisible(eTrue);
    }
    if (mButtons.HasButtons()) {
      mButtons.up->SetEnabled(eTrue);
      mButtons.down->SetEnabled(eTrue);
    }
  }
  sRectf rect = mpWidget->GetWidgetRect();
  AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
  SetNormalizedScrollPosition(mfNormalizedScrollPos);
}

void cScrollBarWidget::SetPageSize(tF32 afPageSize)
{
  if (mfPageSize == afPageSize)
    return;
  mfPageSize = ni::Max(1.0f, afPageSize);
  sRectf rect = mpWidget->GetWidgetRect();
  AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
}

void cScrollBarWidget::SetScrollPosition(tF32 afScrollPos) {
  _SetScrollPosition(afScrollPos,eFalse);
}

tF32 __stdcall cScrollBarWidget::GetScrollPosition() const {
  return mfScrollPos;
}

void cScrollBarWidget::SetNormalizedScrollPosition(tF32 afScrollPos) {
  mfNormalizedScrollPos = afScrollPos;
  const tF32 fRange = (mfMax-mfMin);
  if (fRange > 0) {
    _SetScrollPosition((mfNormalizedScrollPos*fRange)+mfMin,eFalse);
  }
}

tF32 __stdcall cScrollBarWidget::GetNormalizedScrollPosition() const {
  return mfNormalizedScrollPos;
}

void cScrollBarWidget::_SetScrollPosition(tF32 afScrollPos, tBool abWheel)
{
  tF32 newval;
  if (afScrollPos > mfMax) {
    newval = mfMax;
  }
  else if( afScrollPos < mfMin) {
    newval = mfMin;
  }
  else {
    newval = afScrollPos;
  }

  if (newval != mfScrollPos)
  {
    tF32 fPrevPos = mfScrollPos;
    mfScrollPos = newval;
    const tF32 fRange = (mfMax-mfMin);
    if (fRange > 0) {
      mfNormalizedScrollPos = (mfScrollPos-mfMin) / fRange;
    }
    sRectf rect = mpWidget->GetWidgetRect();
    AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
    mpWidget->SendCommand(mpWidget->GetParent(),eWidgetScrollBarCmd_Scrolled,
                          fPrevPos,abWheel);
  }
}

void cScrollBarWidget::UpdateStyle()
{
  iUIContext* pUICtx = mpWidget->GetUIContext();
  const tU32 style = mpWidget->GetStyle();
  const tBool isSlider = !!(style&eWidgetScrollBarStyle_Slider);
  const tBool bHorz = !!(style&eWidgetScrollBarStyle_Horz);
  const tBool hasButtons = kbSrollBarHasButtons;

  tHStringPtr hspSkinClass = mpWidget->GetSkinClass();
  if (isSlider && (hspSkinClass == mpWidget->GetClassName())) {
    hspSkinClass = _H("Slider");
  }

  skin.back = mpWidget->FindSkinElement(hspSkinClass,NULL,bHorz?_H("BackgroundHorz"):_H("BackgroundVert"));

  mButtons.Clear();
  if (mptrThumb.IsOK()) {
    mptrThumb->Destroy();
  }

  if (!isSlider) {
    if (hasButtons) {
      mButtons.up = pUICtx->CreateWidget(
        _A("Button"),mpWidget,sRectf(0,0,0,0),
        eWidgetButtonStyle_Sticky|eWidgetButtonStyle_IconFit|eWidgetButtonStyle_IconCenter,
        _SBID(ButtonUp));
      mButtons.up->SetSkinClass(bHorz?_H("ScrollLeft"):_H("ScrollUp"));
      {
        QPtr<iWidgetButton> btUp = mButtons.up;
        if (btUp.IsOK()) {
          btUp->SetIcon(mpWidget->FindSkinElement(NULL,NULL,bHorz?_H("ArrowLeft"):_H("ArrowUp")));
        }
      }

      mButtons.down = pUICtx->CreateWidget(
        _A("Button"),mpWidget,sRectf(0,0,0,0),
        eWidgetButtonStyle_Sticky|eWidgetButtonStyle_IconFit|eWidgetButtonStyle_IconCenter,
        _SBID(ButtonDown));
      mButtons.down->SetSkinClass(bHorz?_H("ScrollRight"):_H("ScrollDown"));
      {
        QPtr<iWidgetButton> btDown = mButtons.down;
        if (btDown.IsOK()) {
          btDown->SetIcon(mpWidget->FindSkinElement(NULL,NULL,bHorz?_H("ArrowRight"):_H("ArrowDown")));
        }
      }
    }

    mptrThumb = pUICtx->CreateWidget(
        _A("Button"),mpWidget,sRectf(0,0,0,0),
        eWidgetButtonStyle_Sticky|eWidgetButtonStyle_IconFit|eWidgetButtonStyle_IconCenter,
        _SBID(Thumb));
    mptrThumb->AddSink(niNew cThumbButton(this));
    mptrThumb->SetSkinClass(_H("ScrollThumb"));
    {
      QPtr<iWidgetButton> btThumb = mptrThumb;
      if (btThumb.IsOK()) {
        btThumb->SetIcon(mpWidget->FindSkinElement(NULL,NULL,_H("Thumb")));
      }
    }
  }
  else {
    mptrThumb = pUICtx->CreateWidget(_A("Button"),mpWidget,sRectf(0,0,0,0),
                                     eWidgetButtonStyle_Sticky,_SLID(Thumb));
    mptrThumb->AddSink(niNew cThumbButton(this));
    QPtr<iWidgetButton> btn = mptrThumb.ptr();
    niAssert(btn.IsOK());
    if (btn.IsOK()) {
      Ptr<iOverlay> bitmap;
      if (style&eWidgetScrollBarStyle_Right) {
        bitmap = mpWidget->FindSkinElement(_H("ThumbRight"),NULL,_H("Frame"));
        mptrThumb->SetSkinClass(_H("ThumbRight"));
      }
      else if (style&eWidgetScrollBarStyle_Bottom) {
        bitmap = mpWidget->FindSkinElement(_H("ThumbBottom"),NULL,_H("Frame"));
        mptrThumb->SetSkinClass(_H("ThumbBottom"));
      }
      else if (style & eWidgetScrollBarStyle_Horz) {
        bitmap = mpWidget->FindSkinElement(_H("ThumbHorz"),NULL,_H("Frame"));
        mptrThumb->SetSkinClass(_H("ThumbHorz"));
      }
      else {
        bitmap = mpWidget->FindSkinElement(_H("ThumbVert"),NULL,_H("Frame"));
        mptrThumb->SetSkinClass(_H("ThumbVert"));
      }

      //    btn->SetIcon(bitmap);
      //    mptrThumb->SetRect(sRectf(0,0,bitmap->GetSize().x,bitmap->GetSize().y));
      mptrThumb->SetRect(sRectf(0,0,bitmap->GetSize().x,bitmap->GetSize().y));
    }
  }
  AdjustChildrenLayout(mpWidget->GetSize().x,mpWidget->GetSize().y);
}

tF32 cScrollBarWidget::ComputeRoundedPosition(tF32 afNewPos) const {
  tF32 newval = afNewPos;
  if (mpWidget->GetStyle()&eWidgetScrollBarStyle_Slider && mfPageSize > 0.0f) {
    newval = (ni::Sign(newval)*ni::Floor(ni::Abs(newval/mfPageSize)+0.5f))*
        mfPageSize;
  }
  return newval;
}

void cScrollBarWidget::_DoScroll(tU32 anFlags) {
  if (!mptrThumb.IsOK())
    return;

  const tU32 style = mpWidget->GetStyle();
  const tF32 thumbStart = (style&eWidgetScrollBarStyle_Horz) ?
      mptrThumb->GetPosition().x :
      mptrThumb->GetPosition().y;
  const tF32 thumbEnd = (style&eWidgetScrollBarStyle_Horz) ?
      mptrThumb->GetPosition().x + mptrThumb->GetSize().x :
      mptrThumb->GetPosition().y + mptrThumb->GetSize().y;
  tF32 moveUnits;
  //     if (anFlags&SCROLL_PAGE) {
  //          moveUnits = (mfPageSize/TIMER_SCROLL_PAGE_SPLITS);
  //     }
  //     else
  {
    tF32 size = mfMax-mfMin;
    tF32 numThumbs =
        (style&eWidgetScrollBarStyle_Horz) ?
        ni::FDiv(mpWidget->GetSize().x,mptrThumb->GetSize().x) :
        ni::FDiv(mpWidget->GetSize().y,mptrThumb->GetSize().y);
    moveUnits = size / numThumbs / (
        (anFlags&SCROLL_PAGE) ? TIMER_SCROLL_PAGE_SPLITS : TIMER_SCROLL_SPLITS);
  }
  if (anFlags&SCROLL_UP) {
    if (mfScrollStopAt != niMaxF32 &&
        ((mfScrollStopAt >= thumbStart &&
          mfScrollStopAt <= thumbEnd) ||
         (thumbStart < mfScrollStopAt)))
    {
      _StopScroll();
      return;
    }
    SetScrollPosition(ComputeRoundedPosition(mfScrollPos - moveUnits));
  }
  else if (anFlags&SCROLL_DOWN) {
    if (mfScrollStopAt != niMaxF32 &&
        ((mfScrollStopAt >= thumbStart &&
          mfScrollStopAt <= thumbEnd) ||
         (thumbStart > mfScrollStopAt)))
    {
      _StopScroll();
      return;
    }
    SetScrollPosition(ComputeRoundedPosition(mfScrollPos + moveUnits));
  }
}

void cScrollBarWidget::_StartScroll(tU32 anFlags, tF32 afStopAt) {
  if (!mpWidget) return;
  _StopScroll();
  if ((anFlags&SCROLL_UP) || (anFlags&SCROLL_DOWN)) {
    mnScrollTimerID = anFlags;
    mfScrollStopAt = afStopAt;
    mpWidget->SetTimer(
        anFlags,
        (anFlags&SCROLL_PAGE)?
        TIMER_SCROLL_PAGE_SECS:
        TIMER_SCROLL_SECS);
    _DoScroll(anFlags);
  }
}
void cScrollBarWidget::_StopScroll() {
  if (!mpWidget) return;
  if (mnScrollTimerID) {
    mpWidget->SetTimer(mnScrollTimerID,-1);
    mnScrollTimerID = 0;
  }
}
