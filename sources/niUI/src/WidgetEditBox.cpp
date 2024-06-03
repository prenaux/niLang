// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "WidgetEditBox.h"
#include "HardCodedSkin.h"
#include <niLang/STL/scope_guard.h>

#define EB_SB_SIZE kfScrollBarSize
niConstValue tU32 knBlinkTimerID = 500;
niConstValue tF32 kfBlinkTimerTime = 0.5f;

inline tBool _IsSeparator(tU32 c) {
  return !StrIsLetterDigit(c);
}

cEditBoxWidget::cEditBoxWidget(iWidget *pWidget)
    : mbufText(this)
{
  mpWidget = pWidget;
  mbufText.PushLine();
  mbCursorVisible =  eTrue;
  mnTabSize = 4;
  mbDragging = eFalse;
  mnWidestLineNumOfChars = 1;
  mfHScrollOffs = 0;
  mnReplaceChar = 0;
  mbDontMoveCursorOnNextClickDown = eFalse;
  UpdateStyle();
}

cEditBoxWidget::~cEditBoxWidget()
{
}

void cEditBoxWidget::ComputeWidestLine()
{
  const LineVec &lines = mbufText.GetLines();
  mfWidestLine = 0;
  mnWidestLineNumOfChars = 1;
  iFont* font = mpWidget->GetFont();
  sTextLineMetric tlm;
  for (LineVec::const_iterator itr = lines.begin(); itr != lines.end(); ++itr) {
    TextLineMetric_Clear(&tlm);
    TextLineMetric_AddText(&tlm, font, 0, 0, itr->GetChars(), itr->GetSizeInBytes(), eFalse);
    if (tlm.rect.GetWidth() > mfWidestLine) {
      mfWidestLine = tlm.rect.GetWidth();
      mnWidestLineNumOfChars = itr->GetLength();
    }
  }
}

void cEditBoxWidget::AdjustChildrenLayout(tF32 w,tF32 h)
{
  if (mpWidget->GetStyle() & eWidgetEditBoxStyle_MultiLine)
  {
    ComputeWidestLine();
    const tF32 fh = GetLineHeight();
    const sRectf clir = mpWidget->GetClientRect();
    tF32 realw = clir.GetWidth();
    tF32 realh = clir.GetHeight();

    tBool bVertScollbar = eFalse;
    tBool bHorzScollbar = eFalse;
    tU32 nLines = mbufText.GetLineCount();
    tU32 nLinePerPage = (tU32)floor(realh/fh);
    if (nLinePerPage < nLines) {
      sRectf wr = mpBottomScrollbar->GetClientRect();
      if(!mpLeftScrollbar->GetVisible())
        mpsbScrollBarLeft->SetScrollPosition(0);
      mpLeftScrollbar->SetVisible(eTrue);
      realw -= EB_SB_SIZE;
      bVertScollbar = eTrue;
    }
    if (mfWidestLine > realw) {
      sRectf wr = mpBottomScrollbar->GetClientRect();
      realh -= EB_SB_SIZE;
      bHorzScollbar = eTrue;
    }
    if (bVertScollbar) {
      mpLeftScrollbar->SetVisible(eTrue);
    }
    else {
      mpLeftScrollbar->SetVisible(eFalse);
      mpsbScrollBarLeft->SetScrollPosition(0);
    }
    if (bHorzScollbar) {
      mpBottomScrollbar->SetVisible(eTrue);
    }
    else {
      mpBottomScrollbar->SetVisible(eFalse);
      mpsbScrollBarLeft->SetScrollPosition(0);
    }

    nLinePerPage = (tU32)floor(realh/fh);

    mpsbScrollBarLeft->SetPageSize((tF32)nLinePerPage);
    mpsbScrollBarLeft->SetScrollRange(Vec2<tF32>(0,(tF32)(nLines-nLinePerPage+1)));

    mpsbScrollBarBottom->SetPageSize((tF32)w);
    mpsbScrollBarBottom->SetScrollRange(Vec2<tF32>(0,(tF32)mfWidestLine));

    tF32 lsbx = (realw - clir.GetLeft()) - (realw - clir.GetRight());
    tF32 lsby = (realh - clir.GetTop()) - (realh - clir.GetBottom());
    mpLeftScrollbar->SetRect(sRectf(lsbx-EB_SB_SIZE,0,EB_SB_SIZE,realh));
    mpBottomScrollbar->SetRect(sRectf(0,lsby-EB_SB_SIZE,realw,EB_SB_SIZE));
  }
}

void cEditBoxWidget::ComputeClientRect(tF32 w,tF32 h)
{
  sRectf rect = skin.normalFrame->ComputeFrameCenter(sRectf(0,0,w,h));
  // put scrollbars in the right place
  mpWidget->SetClientRect(rect);
}

void cEditBoxWidget::AutoScroll() {
  AutoScroll(eFalse);
}

void cEditBoxWidget::AutoScroll(tBool abForceRefresh)
{
  if (mpsbScrollBarLeft && (mpWidget->GetStyle() & eWidgetEditBoxStyle_MultiLine)) {
    Position cur = mbufText.GetCursor();
    tF32 scrollpos = mpsbScrollBarLeft->GetScrollPosition();
    tF32 pagesize = mpsbScrollBarLeft->GetPageSize();
    if ((tF32)cur._line > (scrollpos+pagesize-1)) {
      mpsbScrollBarLeft->SetScrollPosition((tF32)cur._line - pagesize + 1);
    }
    else if ((tF32)cur._line < scrollpos) {
      mpsbScrollBarLeft->SetScrollPosition((tF32)cur._line);
    }
  }

  const tF32 leftScrollBarWidth = mpLeftScrollbar ? mpLeftScrollbar->GetSize().x : 0.0f;
  const tF32 w = mpWidget->GetClientSize().x -
      ((mpLeftScrollbar&&mpLeftScrollbar->GetVisible()) ?
       mpLeftScrollbar->GetSize().x : 0);
  const Position curpos = mbufText.GetCursor();
  const Line &line = mbufText.GetLine(curpos._line);
  iFont* font = mpWidget->GetFont();

  sTextLineMetric tlm;
  TextLineMetric_Clear(&tlm);
  TextLineMetric_AddText(&tlm, font, 0, 0, line.GetChars(), line.GetSizeInBytes(), eFalse);
  const tF32 fullTxtW = tlm.rect.GetWidth();
  if (w > fullTxtW || curpos._col == 0) {
    mfHScrollOffs = 0;
  }
  else {
    TextLineMetric_Clear(&tlm);
    StrCharIt it(line.GetChars(),line.GetSizeInBytes());
    it.advance(curpos._col);
    TextLineMetric_AddText(&tlm, font, 0, 0, line.GetChars(), it.current() - it.start(), eFalse);
    const tF32 txtW = tlm.rect.GetWidth();
    const tF32 charWidth = font->GetAverageCharWidth();
    const tF32 scrollMargin = charWidth*2;
    const tF32 curX = txtW+mfHScrollOffs+mpWidget->GetPadding().x;
    if (abForceRefresh || curX < scrollMargin || curX > w-scrollMargin) {
      mfHScrollOffs = ni::Min(w-(txtW+(w/2)),0);
      mfHScrollOffs = ni::Max(mfHScrollOffs,w-(fullTxtW+charWidth)) - leftScrollBarWidth;
    }
  }


  if (mpsbScrollBarBottom) {
    mpsbScrollBarBottom->SetScrollPosition(-mfHScrollOffs);
  }
}

tBool cEditBoxWidget::OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1)
{
  niGuardObject((iWidgetSink*)this);
  switch(nMsg){
    case eUIMessage_SinkAttached:
      MoveCursorHome(eFalse);
      AutoScroll();
      break;
    case eUIMessage_SkinChanged:
      InitSkin();
      return eFalse;
    case eUIMessage_NCSize:
      ComputeClientRect(varParam0.mV2F[0],varParam0.mV2F[1]);
      break;
    case eUIMessage_NCPaint: {
      iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
      if (c) {
        iOverlay* frame;
        if (mpWidget->GetHasFocus()) {
          frame = skin.focusedFrame;
        }
        else if (mpWidget->GetIsMouseOver()) {
          frame = skin.hoverFrame;
        }
        else {
          frame = skin.normalFrame;
        }
        c->BlitOverlayFrame(
            mpWidget->GetWidgetRect(),
            frame,
            eRectFrameFlags_All);
      }
    }
      break;
    case eUIMessage_Size:
      AdjustChildrenLayout(varParam0.mV2F[0],varParam0.mV2F[1]);
      AutoScroll(eTrue);
      break;
    case eUIMessage_Paint: {
      iCanvas* c = VarQueryInterface<iCanvas>(varParam1);
      Paint(c);
      break;
    }
    case eUIMessage_Timer: {
      if (varParam0.mU32 == 500) {
        mbCursorVisible = !mbCursorVisible;
        apWidget->Redraw();
      }
      break;
    }
    case eUIMessage_SetFocus: {
      apWidget->SetTimer(knBlinkTimerID,kfBlinkTimerTime);
      mbCursorVisible = eTrue;
      if (!(mpWidget->GetStyle()&eWidgetEditBoxStyle_NoSelect)) {
        if (mpWidget->GetStyle()&eWidgetEditBoxStyle_SelectAllOnSetFocus) {
          MoveCursorEnd(false);
          AutoScroll();
          mSelection = Selection(mbufText.GetStartPosition(),mbufText.GetEndPosition(),stNormal);
          if (varParam0.mBool) {
            // focus was given by the mouse
            mbDontMoveCursorOnNextClickDown = eTrue;
          }
        }
      }
      return eFalse;
    }
    case eUIMessage_LostFocus: {
      apWidget->SetTimer(knBlinkTimerID,-1);
      mbCursorVisible = eFalse;
      if (!(mpWidget->GetStyle()&eWidgetEditBoxStyle_DontLoseSelection)) {
        mSelection = Selection(); // clear the selection when losing the focus
      }
      mpWidget->SendCommand(mpWidget->GetParent(),eWidgetEditBoxCmd_LostFocus);
      if (!niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_ReadOnly) &&
          mpWidget->GetStyle()&eWidgetEditBoxStyle_ValidateOnLostFocus)
      {
        mpWidget->SendCommand(mpWidget->GetParent(),eWidgetEditBoxCmd_Validated);
      }
      break;
    }
    case eUIMessage_SetCursor:
      if (mpWidget->GetStyle()&eWidgetEditBoxStyle_ReadOnly) {
        apWidget->GetUIContext()->SetCursor(skin.curHand);
      }
      else {
        apWidget->GetUIContext()->SetCursor(skin.curText);
      }
      break;
    case eUIMessage_MoveFocus:
      {
        if (mpWidget->GetStyle()&eWidgetEditBoxStyle_CanInsertTabs) {
          return eTrue;
        }
        else {
          return eFalse;
        }
        break;
      }
    case eUIMessage_KeyChar:
    case eUIMessage_KeyDown:
      {
        tU32 key = 0, keyMod = 0, ch = 0;
        if (nMsg == eUIMessage_KeyDown) {
          key = varParam0.mU32;
          keyMod = varParam1.mU32;
        }
        else {
          ch = varParam0.mU32;
        }
        bool bRecomputeLayout = OnKeyDown(key,keyMod,ch);
        if (bRecomputeLayout) {
          sRectf rect = apWidget->GetWidgetRect();
          AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
          AutoScroll();
        }
        break;
      }
    case eUIMessage_LeftClickDown:
    case eUIMessage_NCLeftClickDown:
      OnLeftClickDown(((Var&)varParam0).GetVec2f());
      break;
    case eUIMessage_NCLeftClickUp:
    case eUIMessage_LeftClickUp:
      OnLeftClickUp(((Var&)varParam0).GetVec2f());
      break;
    case eUIMessage_NCDragBegin:
    case eUIMessage_DragBegin: {
      mbDragging = eTrue;
      break;
    }
    case eUIMessage_NCDragEnd:
    case eUIMessage_DragEnd: {
      mbDragging = eFalse;
      break;
    }
    case eUIMessage_NCMouseMove:
    case eUIMessage_MouseMove:
      if (mbDragging) {
        OnMouseSelect(((Var&)varParam0).GetVec2f());
      }
      break;
    case eUIMessage_StyleChanged:
      UpdateStyle();
      break;
    case eUIMessage_TextChanged:
      SetTextBuffer(mpWidget->GetText());
      break;
    case eUIMessage_SetText:
      if (varParam0.mBool) {
        mpWidget->SetText(_H(""));
      }
      else {
        mpWidget->SetText(GetTextBuffer());
      }
      break;
    case eUIMessage_Copy:
      return OnCopy(ni::VarQueryInterface<ni::iDataTable>(varParam0));
    case eUIMessage_Cut:
      return OnCut(ni::VarQueryInterface<ni::iDataTable>(varParam0));
    case eUIMessage_Paste:
      return OnPaste(ni::VarQueryInterface<ni::iDataTable>(varParam0));
    case eEditorBufferMessage_Modified: {
      mpWidget->SendCommand(mpWidget->GetParent(),eWidgetEditBoxCmd_Modified);
      break;
    }
    case eUIMessage_SerializeWidget: {
      QPtr<iDataTable> ptrDT = varParam0;
      if (ptrDT.IsOK()) {
        tU32 nFlags = varParam1.mU32;
        if (nFlags & eWidgetSerializeFlags_Write) {
          if (mnReplaceChar) {
            cString str; str.appendChar(mnReplaceChar);
            ptrDT->SetString("replace_char",str.Chars());
          }
        }
        else if (nFlags & eWidgetSerializeFlags_Read) {
          tU32 nIndex = ptrDT->GetPropertyIndex(_A("replace_char"));
          if (nIndex != eInvalidHandle) {
            cString str = ptrDT->GetStringFromIndex(nIndex);
            if (!str.empty()) {
              mnReplaceChar = str.charIt().next();
            }
          }
          if (ptrDT->HasProperty("_data")) {
            apWidget->SetText(ptrDT->GetHString("_data"));
          }
        }
      }
      break;
    }
    default:
      return eFalse;
  }
  return eTrue;
}

tF32 __stdcall cEditBoxWidget::GetLineHeight() const {
  return TextLineMetric_ComputeLineHeight(mpWidget->GetFont());
}

void cEditBoxWidget::Paint(iCanvas* apCanvas)
{
  const tU32 frontnormal = ULColorBuild(skin.colFrontNormal);
  const tU32 frontsel = ULColorBuild(skin.colFrontSel);
  const tU32 backsel = ULColorBuild(skin.colBackSel);

  const tBool multiline = (mpWidget->GetStyle() & eWidgetEditBoxStyle_MultiLine)?eTrue:eFalse;
  sRectf rect = mpWidget->GetClientRect();
  rect.MoveTo(sVec2f::Zero());
  const sRectf& padding = mpWidget->GetPadding();
  rect.x += padding.x+1;
  rect.y += padding.y+1;
  rect.z -= padding.z;
  rect.w -= padding.w;
  iFont *font = mpWidget->GetFont();
  tU32 firstvisibleline = 0;
  if (mpsbScrollBarLeft)
    firstvisibleline = (tU32)mpsbScrollBarLeft->GetScrollPosition();
  if (mpsbScrollBarBottom && mpBottomScrollbar->GetVisible())
    mfHScrollOffs = -mpsbScrollBarBottom->GetScrollPosition();
  const LineVec &lines=mbufText.GetLines();
  const tF32 h = GetLineHeight();
  tF32 y = rect.y;
  if (!multiline) {
    tF32 add = ni::Max(0.0f,(rect.GetHeight()-h)/2.0f);
    y += add;
  }
  const tF32 rectWidth = rect.GetWidth() -
      ((mpLeftScrollbar&&mpLeftScrollbar->GetVisible()) ?
       mpLeftScrollbar->GetSize().x : 0);
  const tF32 rectHeight = rect.GetHeight() -
      ((mpBottomScrollbar&&mpBottomScrollbar->GetVisible()) ?
       mpBottomScrollbar->GetSize().y : 0);

  const tF32 fontSizeByRes = ni::FDiv(font->GetSize().x,(tF32)font->GetResolution());

  tF32 cursorx = -1;
  tF32 cursory = -1;
  tU32 curline = 0;
  for (LineVec::const_iterator itr = lines.begin(); itr != lines.end(); ++itr)
  {
    if (curline >= firstvisibleline) {
      const Line &line = *itr;

      tF32 x = mfHScrollOffs+rect.x;
      tU32 col = 0;
      tU32 cur_realcol = 0;

      Position cursor = mbufText.GetCursor();
      tBool bDrawCursor =
          (mbCursorVisible) &&
          (niFlagIsNot(mpWidget->GetStyle(),eWidgetEditBoxStyle_ReadOnly)) &&
          (niFlagIsNot(mpWidget->GetStyle(),eWidgetEditBoxStyle_NoCursor)) &&
          (mpWidget->GetHasFocus()?eTrue:eFalse);

      if (bDrawCursor && cursor._line==curline && cursor._col==0) {
        cursorx = ni::Max(0.0f,x);
      }

      if (line.IsEmpty()) {
        // nothing to draw...
      }
      else {
        ResetGlyphCache(line.GetLength());
        tBool bDrawSel = eFalse;
        tF32 selStartX = 0.0f;
        tF32 selEndX = 0.0f;
        StrCharIt charIt(line.GetChars(),line.GetSizeInBytes());
        for (tU32 ch = charIt.peek_next(), prevch = 0; !charIt.is_end(); charIt.next(), ch = charIt.peek_next(), prevch = ch)
        {
          niAssert(mnTabSize > 0 && mnTabSize < 128);
          tU32 thechar = ch;
          tU32 rep = 1;
          if (ch == '\t') {
            thechar = ' ';
            prevch = 0;
            rep = (mnTabSize-(cur_realcol%mnTabSize));
          }
          const tBool isSelected = (!mSelection.IsEmpty() && mSelection.IsIn(curline,col));
          tU32 frontcolor = isSelected ? frontsel : frontnormal;
          if (mnReplaceChar) {
            thechar = mnReplaceChar;
          }

          if (x < 0) {
            // cull left...
            const tF32 startx = x;
            for (tU32 i = 0; i < rep; i++) {
              x += font->GetAdvance(thechar);
            }
            if (x > 0) {
              x = startx;
              if (isSelected && !bDrawSel) {
                selStartX = x;
                bDrawSel = eTrue;
              }
              for (tU32 i = 0; i < rep; i++) {
                x += PushGlyph(font,x,y,thechar,frontcolor,fontSizeByRes);
                ++cur_realcol;
              }
              if (isSelected) {
                selStartX = x;
              }
            }
          }
          else {
            if (isSelected && !bDrawSel) {
              selStartX = x;
              bDrawSel = eTrue;
            }
            for (tU32 i = 0; i < rep; i++) {
              x += PushGlyph(font,x,y,thechar,frontcolor,fontSizeByRes);
              ++cur_realcol;
            }
          }
          if (isSelected) {
            selEndX = x;
          }

          col++;
          if (bDrawCursor && cursor._line==curline && cursor._col==col) {
            cursorx = x;
          }
          if (x > rectWidth) {
            break; // cull right...
          }
        }
        if (bDrawSel) {
          apCanvas->BlitFillAlpha(
              sRectf(selStartX,y,selEndX-selStartX,h),
              backsel);
        }
        if (!mGlyphCache.empty()) {
          niLoop(i,mGlyphCache.size()) {
            mGlyphCache[i].gm.PushToCanvas(
              apCanvas, (cFont*)font,
              mGlyphCache[i].color, sVec3f::Zero());
          }
        }
      }

      y += h;
      if (bDrawCursor && cursor._line==curline
          && cursorx >= 0 && cursorx <= rectWidth) {
        cursory = y;
      }
      if (y > rectHeight)
        break;
    }
    curline ++;
  }

  if (cursory >= 0) {
    apCanvas->BlitFill(
        sRectf(Vec2(cursorx-1.0f,cursory-h),
                    Vec2(cursorx+1.0f,cursory)),
        frontnormal);
  }
}


Position cEditBoxWidget::TranslateMousePos(tF32 x,tF32 y)
{
  const tF32 lineHeight = GetLineHeight();
  x += -mfHScrollOffs - mpWidget->GetPadding().x;
  y -= mpWidget->GetPadding().y;
  if (mpsbScrollBarLeft)
    y += mpsbScrollBarLeft->GetScrollPosition() * lineHeight;
  tI32 lineidx= ni::Max(0,(tI32)(y/lineHeight));
  tI32 colidx=0;
  tI32 simlen=0;
  const Line &line=mbufText.GetLine(lineidx);
  tF32 totalx = 0.0f;
  const iFont* pFont = mpWidget->GetFont();
  StrCharIt itText(line.GetChars(),line.GetSizeInBytes());
  while (!itText.is_end()) {
    tU32 nchar = 1;
    tU32 c = itText.next();
    if (c == '\t') {
      c = ' ';
      nchar = mnTabSize-(simlen%mnTabSize);
    }
    const tF32 cw = pFont->GetAdvance(c);
    simlen += nchar;
    if ((totalx+cw*(tF32)nchar) > x) {
      break;
    }
    totalx += cw*(tF32)nchar;
    colidx++;
  }
  return Position(lineidx,colidx);
}

void cEditBoxWidget::DeleteSelection()
{
  if (!mSelection.IsEmpty()) {
    mbufText.DeleteSelectionContent(mSelection);
    mbufText.SetCursorPos(mSelection._start,eTrue);
    mSelection=Selection();
  }
}

void cEditBoxWidget::MoveCursorHome(tBool bLine)
{
  Position curpos;
  if (bLine) {
    curpos = mbufText.GetCursor();
    curpos._col=0;
  }
  else {
    curpos = mbufText.GetStartPosition();
  }
  MoveCursor((sVec2i&)curpos,eTrue);
}

void cEditBoxWidget::MoveCursorEnd(tBool bLine)
{
  Position curpos;
  if (bLine) {
    curpos = mbufText.GetCursor();
    curpos._col = mbufText.GetLineLen(curpos._line);
  }
  else {
    curpos = mbufText.GetEndPosition();
  }
  MoveCursor((sVec2i&)curpos,eTrue);
}

void cEditBoxWidget::MoveCursor(const sVec2i &pos, tBool forcelogicalcol)
{
  tBool bSelection =
      mbDragging ||
      (mpWidget->GetUIContext()->GetInputModifiers()&eUIInputModifier_Shift);
  Position curpos = mbufText.GetCursor();
  Position realnewpos = mbufText.SetCursorPos((Position&)pos,forcelogicalcol?true:false);
  if (niFlagIsNot(mpWidget->GetStyle(),eWidgetEditBoxStyle_NoSelect) && bSelection) { //select from cusor to new position
    if (!(curpos == realnewpos)) {
      if (mSelection.IsEmpty()) {
        mSelection = Selection(curpos, realnewpos, stNormal);
        mSelPivot = curpos;
      }
      else {
        mSelection.Merge(mSelPivot,realnewpos);
      }
    }
  }
  else {
    mSelection = Selection();
  }
  if (mpWidget->GetHasFocus()) {
    mpWidget->SetTimer(knBlinkTimerID,kfBlinkTimerTime);
  }
}

void __stdcall cEditBoxWidget::MoveCursorWordDelta(tI32 word) {
  if (!word || !mbufText.GetLinesCount())
    return;

  tBool bPrev = word < 0 ? eTrue : eFalse;
  tU32 count = (tU32)ni::Abs(word);
  niLoop(i,count) {
    const Position pos = mbufText.GetCursor();
    const Line& line = mbufText.GetLine(pos._line);
    if (line.IsEmpty()) {
      MoveCursorLineDelta(bPrev?-1:1);
      bPrev ? MoveCursorEnd(eTrue) : MoveCursorHome(eTrue);
      continue;
    }
    if (bPrev) {
      if (pos._col <= 0) {
        if (pos._line > 0) {
          MoveCursorLineDelta(-1);
          MoveCursorEnd(eTrue);
        }
        continue;
      }
    }
    else {
      if (pos._col >= line.GetLength()-1) {
        if (pos._line < (tU32)(mbufText.GetLinesCount()-1)) {
          MoveCursorLineDelta(1);
          MoveCursorHome(eTrue);
        }
        continue;
      }
    }

    tI32 c = (tI32)pos._col;
    if (bPrev) {
      const tBool startIsSeparator = _IsSeparator(line.GetAt(c-1));
      if (startIsSeparator) {
        // skip separators
        while ((c >= 0) && _IsSeparator(line.GetAt(c-1))) {
          --c;
        }
      }
      // skip current word
      while ((c >= 0) && !_IsSeparator(line.GetAt(c-1))) {
        --c;
      }
    }
    else {
      // const tU32 lineLen = line.GetLength();
      StrCharIt it(line.GetChars()+StrOffset(line.GetChars(),c),line.GetSizeInBytes());
      const tBool startIsSeparator = _IsSeparator(it.peek_next());
      if (!startIsSeparator) {
        // skip current word
        while (!it.is_end() && !_IsSeparator(it.peek_next())) {
          it.next();
          ++c;
        }
      }
      // skip separators
      while (!it.is_end() && _IsSeparator(it.peek_next())) {
        it.next();
        ++c;
      }
    }

    MoveCursorColumnDelta(c-pos._col);

    if (pos == mbufText.GetCursor())
      return; // break directly if it didnt move at all
  }
}

void __stdcall cEditBoxWidget::MoveCursorParaDelta(const tI32 aDelta) {
  if (!aDelta || !mbufText.GetLinesCount())
    return;

  const tBool bPrev = aDelta < 0 ? eTrue : eFalse;
  niDefer {
    Position closestPos = mbufText.GetClosestValidPos(mbufText.GetCursor(),aDelta);
    if (closestPos != mbufText.GetCursor()) {
      MoveCursor((sVec2i&)closestPos,eFalse);
    }
  };

  auto isCurrentLineEmpty = [&]() -> tBool {
    const Position pos = mbufText.GetCursor();
    const Line& line = mbufText.GetLine(pos._line);
    return line.IsEmpty();
  };

  auto nextLine = [&]() {
    const Position pos = mbufText.GetCursor();
    MoveCursorLineDelta(bPrev?-1:1);
    if (pos == mbufText.GetCursor()) {
      // Cursor hasnt move, so we reached the begining or the end of the file so
      // we should stop.
      return eFalse;
    }
    return eTrue;
  };

  const tU32 count = (tU32)ni::Abs(aDelta);
  niLoop(i,count) {
    // we're starting in an empty line, so got to the first non empty line
    while (isCurrentLineEmpty()) {
      if (!nextLine())
        return;
    }

    // skip the non empty lines
    while (!isCurrentLineEmpty()) {
      if (!nextLine())
        return;
    }
  }
}

void cEditBoxWidget::MoveCursorLineDelta(tI32 line)
{
  Position cursor=mbufText.GetCursor();
  tI32 targetline=cursor._line+line;
  tI32 targetcol=mbufText.GetLogicalCursorCol();
  if(targetline<0)targetline=0;
  Position pos(targetline,targetcol);
  Position closestPos = mbufText.GetClosestValidPos(pos,0);
  MoveCursor((sVec2i&)closestPos,eFalse);
}

void cEditBoxWidget::MoveCursorColumnDelta(tI32 col)
{
  Position cursor=mbufText.GetCursor();
  tI32 targetline=cursor._line;
  tI32 targetcol=cursor._col+col;
  if(targetcol<0 && targetline>0){
    targetline--;
    if(targetline>=0){
      targetcol=mbufText.GetLineLen(targetline);
    }
  }else if(targetcol > (mbufText.GetLineLen(targetline))){
    if(targetline+1 < mbufText.GetLineCount())
      targetline++;
  }
  if(targetline<0)targetline=0;
  if(targetcol<0)targetcol=0;

  Position pos(targetline,targetcol);
  Position closestPos = mbufText.GetClosestValidPos(pos);
  MoveCursor((sVec2i&)closestPos,eTrue);
}

void cEditBoxWidget::OnMouseSelect(const sVec2f &mpos)
{
  if (niFlagIsNot(mpWidget->GetStyle(),eWidgetEditBoxStyle_NoSelect)) {
    Position newpos = TranslateMousePos(mpos.x,mpos.y);
    MoveCursor((sVec2i&)newpos,eTrue);
    AutoScroll();
    mbCursorVisible = eTrue;
  }
}

void cEditBoxWidget::OnLeftClickDown(const sVec2f &mpos)
{
  if (!mbDontMoveCursorOnNextClickDown) {
    Position newpos = TranslateMousePos(mpos.x,mpos.y);
    MoveCursor((sVec2i&)newpos,eTrue);
  }
  mpWidget->SetCapture(eTrue);
  mbCursorVisible = eTrue;
}

void cEditBoxWidget::OnLeftClickUp(const sVec2f &mpos)
{
  mpWidget->SetCapture(eFalse);
  mbDontMoveCursorOnNextClickDown = eFalse;
}

bool cEditBoxWidget::OnKeyDown(tU32 key,tU32 mod,tU32 c)
{
  bool bRecomputeLayout = false;
  bool bInsertChar = false;
  switch (key)
  {
    case eKey_LShift:
    case eKey_RShift:
      return false;
    case eKey_Up:
      if (niFlagIs(mod,eKeyMod_Control)) {
        MoveCursorLineDelta(-1);
        if (mpsbScrollBarLeft)
          mpsbScrollBarLeft->SetScrollPosition(
              mpsbScrollBarLeft->GetScrollPosition()-1);
      }
      else if (niFlagIs(mod,eKeyMod_Alt)) {
        MoveCursorParaDelta(-1);
        AutoScroll();
      }
      else {
        MoveCursorLineDelta(-1);
        AutoScroll();
      }
      break;
    case eKey_Down:
      if (niFlagIs(mod,eKeyMod_Control)) {
        MoveCursorLineDelta(1);
        if (mpsbScrollBarLeft)
          mpsbScrollBarLeft->SetScrollPosition(
              mpsbScrollBarLeft->GetScrollPosition()+1);
      }
      else if (niFlagIs(mod,eKeyMod_Alt)) {
        MoveCursorParaDelta(1);
        AutoScroll();
      }
      else {
        MoveCursorLineDelta(1);
        AutoScroll();
      }
      break;
    case eKey_Left: {
      if (niFlagIs(mod,eKeyMod_Control) || niFlagIs(mod,eKeyMod_Alt)) {
        MoveCursorWordDelta(-1);
        AutoScroll();
      }
      else {
        MoveCursorColumnDelta(-1);
        AutoScroll();
      }
      break;
    }
    case eKey_Right: {
      if (niFlagIs(mod,eKeyMod_Control) || niFlagIs(mod,eKeyMod_Alt)) {
        MoveCursorWordDelta(1);
        AutoScroll();
      }
      else {
        MoveCursorColumnDelta(1);
        AutoScroll();
      }
      break;
    }
    case eKey_PgUp:
      {
        sRectf clir = mpWidget->GetClientRect();
        tF32 realh = clir.GetHeight();
        const tF32 ch = GetLineHeight();
        tI32 nLinePerPage = ((tU32)floor(realh/ch))-1;
        if (nLinePerPage > 0) {
          MoveCursorLineDelta(-nLinePerPage);
          AutoScroll();
        }
        break;
      }
    case eKey_PgDn:
      {
        sRectf clir = mpWidget->GetClientRect();
        tF32 realh = clir.GetHeight();
        const tF32 ch = GetLineHeight();
        tI32 nLinePerPage = ((tU32)floor(realh/ch))-1;
        if (nLinePerPage > 0) {
          MoveCursorLineDelta(nLinePerPage);
          AutoScroll();
        }
        break;
      }
    case eKey_Delete:
      {
        if (!niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_ReadOnly)) {
          if (niFlagIs(mod,eKeyMod_Control) || niFlagIs(mod,eKeyMod_Alt)) {
            mbDragging = eTrue;
            MoveCursorWordDelta(1);
            mbDragging = eFalse;
          }

          if (mSelection.IsEmpty())
            mbufText.DeleteChar();
          else
            DeleteSelection();
          sRectf rect = mpWidget->GetClientRect();
          AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
        }
      }
      break;
    case eKey_Home: {
      MoveCursorHome(!niFlagIs(mod,eKeyMod_Control));
      AutoScroll();
      break;
    }
    case eKey_End: {
      MoveCursorEnd(!niFlagIs(mod,eKeyMod_Control));
      AutoScroll();
      break;
    }
    case eKey_Enter:
    case eKey_NumPadEnter:
      {
        if (!niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_ReadOnly)) {
          if (mpWidget->GetStyle() & eWidgetEditBoxStyle_MultiLine) {
            if (niFlagIs(mod,eKeyMod_Control) || niFlagIs(mod,eKeyMod_Alt)) {
              mpWidget->SendCommand(mpWidget->GetParent(),eWidgetEditBoxCmd_Validated);
            }
            else {
              DeleteSelection();
              mbufText.AddNewLine();
              sRectf rect = mpWidget->GetWidgetRect();
              AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
              AutoScroll();
            }
          }
          else {
            mpWidget->SendCommand(mpWidget->GetParent(),eWidgetEditBoxCmd_Validated);
          }
        }
        break;
      }
    case eKey_BackSpace:
      {
        if (!niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_ReadOnly)) {
          if (niFlagIs(mod,eKeyMod_Control) || niFlagIs(mod,eKeyMod_Alt)) {
            mbDragging = eTrue;
            MoveCursorWordDelta(-1);
            mbDragging = eFalse;
          }
          if (!mSelection.IsEmpty()) {
            DeleteSelection();
            sRectf rect = mpWidget->GetWidgetRect();
            AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
          }
          else {
            mbufText.BackSpace();
            sRectf rect = mpWidget->GetWidgetRect();
            AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
          }
          AutoScroll();
        }
        break;
      }
    case eKey_A: {
      bInsertChar = eTrue;
      if (niFlagIsNot(mpWidget->GetStyle(),eWidgetEditBoxStyle_NoSelect)) {
        if (niFlagIs(mod,eKeyMod_Control)) {
          MoveCursorEnd(false);
          AutoScroll();
          mSelection = Selection(mbufText.GetStartPosition(),mbufText.GetEndPosition(),stNormal);
        }
      }
      break;
    };
    default:
      bInsertChar = eTrue;
      break;
  }

  if (bInsertChar) {
    const tU32 tabChar = niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_CanInsertTabs)
        ? '\t' : ' ';
    // Insert char
    if (!niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_ReadOnly) &&
        ((c >= 32) || (c == tabChar)))
    {
      DeleteSelection();
      mbufText.AddChar(c);
    }
  }

  mbCursorVisible=eTrue;
  AutoScroll();
  return bRecomputeLayout;
}

Ptr<iHString> __stdcall cEditBoxWidget::GetTextBuffer() const
{
  if (mpWidget->GetStyle() & eWidgetEditBoxStyle_MultiLine) {
    const tU32 lines = mbufText.GetLineCount();
    const tU32 lastlinelen = mbufText.GetLine(lines-1).GetLength();
    Selection sel(Position(0,0),Position(lines-1,lastlinelen),stNormal);
    cString strData;
    niThis(cEditBoxWidget)->mbufText.GetSelectionData(sel,strData);
    return _H(strData);
  }
  else {
    const LineVec &lines = mbufText.GetLines();
    return _H(lines[0].GetChars());
  }
}

tBool __stdcall cEditBoxWidget::SetTextBuffer(iHString* ahspText)
{
  return DoSetTextBuffer(ni::HStringGetStringEmpty(ahspText));
}

tBool __stdcall cEditBoxWidget::DoSetTextBuffer(const achar* aszText)
{
  if(mpWidget->GetStyle() & eWidgetEditBoxStyle_MultiLine)
  {
    Line l(eNewLine_LF);
    mbufText.Clear();
    StrCharIt itText(aszText);
    while (!itText.is_end()) {
      const tU32 c = itText.next();
      if (c == '\r' || c == '\n') {
        if (c == _A('\r')) {
          const tU32 nextC = itText.peek_next();
          if (nextC == '\n') {
            // DOS newline
            l.SetNewLine(eNewLine_CRLF);
            itText.next();
          }
          else {
            // MAC newline
            l.SetNewLine(eNewLine_CR);
          }
        }
        else {
          l.SetNewLine(eNewLine_LF);
        }
        mbufText.PushLine(l);
        l = Line(eNewLine_LF);
      }
      else {
        l.AppendChar(c);
      }
    }
    mbufText.PushLine(l);
  }
  else
  {
    //mvText[0] = aszText;
    mbufText.SetLineData(0,aszText);
  }


  sRectf rect = mpWidget->GetWidgetRect();
  AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
  MoveCursorEnd(false);
  mSelection = Selection();
  if (niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_AutoScroll)) {
    AutoScroll();
  }
  return eTrue;
}

void __stdcall cEditBoxWidget::AddText(const achar* aaszText)
{
  mbufText.AddText(aaszText);
  sRectf rect = mpWidget->GetWidgetRect();
  AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
  if (niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_AutoScroll)) {
    MoveCursorEnd(false);
    AutoScroll();
  }
}

///////////////////////////////////////////////
void cEditBoxWidget::UpdateStyle()
{
  if(mpWidget->GetStyle() & eWidgetEditBoxStyle_MultiLine)
  {
    iUIContext *pUICtx = mpWidget->GetUIContext();
    mpLeftScrollbar = pUICtx->CreateWidget(_A("ScrollBar"),mpWidget,sRectf(),0);
    mpLeftScrollbar->SetVisible(eFalse);
    mpsbScrollBarLeft = ni::QueryInterface<iWidgetScrollBar>(mpLeftScrollbar);
    mpsbScrollBarLeft->SetScrollPosition(0);
    mpBottomScrollbar = pUICtx->CreateWidget(_A("ScrollBar"),mpWidget,sRectf(),eWidgetScrollBarStyle_Horz);
    mpBottomScrollbar->SetVisible(eFalse);
    mpsbScrollBarBottom = ni::QueryInterface<iWidgetScrollBar>(mpBottomScrollbar);
    mpsbScrollBarBottom->SetScrollPosition(0);
  } else {
    mpLeftScrollbar = NULL;
    mpBottomScrollbar = NULL;
    mpsbScrollBarBottom = NULL;
    mpsbScrollBarLeft = NULL;
  }
  if (niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_NoSelect)) {
    mSelection = Selection();
  }
}

///////////////////////////////////////////////
void cEditBoxWidget::InitSkin() {
  skin.normalFrame = mpWidget->FindSkinElement(NULL,NULL,_H("Frame"));
  skin.hoverFrame = mpWidget->FindSkinElement(NULL,_H("Hover"),_H("Frame"));
  skin.focusedFrame = mpWidget->FindSkinElement(NULL,_H("Focused"),_H("Frame"));
  if (skin.hoverFrame == mpWidget->GetUIContext()->GetErrorOverlay())
    skin.hoverFrame = skin.normalFrame;
  if (skin.focusedFrame == mpWidget->GetUIContext()->GetErrorOverlay())
    skin.focusedFrame = skin.normalFrame;
  skin.curText = mpWidget->FindSkinCursor(NULL,NULL,_H("Text"));
  skin.curHand = mpWidget->FindSkinCursor(NULL,NULL,_H("Hand"));
  skin.colFrontNormal = mpWidget->FindSkinColor(Vec4<tF32>(0,0,0,1),NULL,NULL,_H("TextFrontNormal"));
  skin.colFrontSel = mpWidget->FindSkinColor(Vec4<tF32>(1,1,1,1),NULL,NULL,_H("TextFrontSel"));
  skin.colBackSel = mpWidget->FindSkinColor(Vec4<tF32>(49.0f/255.0f,106.0f/255.0f,197.0f/255.0f,1.0),NULL,NULL,_H("TextBackSel"));
}

///////////////////////////////////////////////
void __stdcall cEditBoxWidget::SetReplaceChar(tU32 anChar)
{
  mnReplaceChar = anChar;
}

///////////////////////////////////////////////
tU32 __stdcall cEditBoxWidget::GetReplaceChar() const
{
  return mnReplaceChar;
}

///////////////////////////////////////////////
sVec2i __stdcall cEditBoxWidget::GetCursorPosition() const
{
  Position pos = mbufText.GetCursor();
  return ni::Vec2((tI32)pos._line,(tI32)pos._col);
}

///////////////////////////////////////////////
tU32 __stdcall cEditBoxWidget::GetCursorLine() const
{
  return GetCursorPosition().x;
}

///////////////////////////////////////////////
tU32 __stdcall cEditBoxWidget::GetCursorColumn() const
{
  return GetCursorPosition().y;
}

///////////////////////////////////////////////
tBool __stdcall cEditBoxWidget::OnCopy(ni::iDataTable* apDT)
{
  if (apDT && niFlagIsNot(mpWidget->GetStyle(),eWidgetEditBoxStyle_NoSelect) && !mSelection.IsEmpty()) {
    cString strText;
    mbufText.GetSelectionData(mSelection,strText);
    apDT->SetString(_A("text"),strText.Chars());
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cEditBoxWidget::OnCut(ni::iDataTable* apDT)
{
  if (apDT && niFlagIsNot(mpWidget->GetStyle(),eWidgetEditBoxStyle_NoSelect) && !mSelection.IsEmpty()) {
    if (!niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_ReadOnly)) {
      cString strText;
      mbufText.GetSelectionData(mSelection,strText);
      DeleteSelection();
      apDT->SetString(_A("text"),strText.Chars());

      sRectf rect = mpWidget->GetWidgetRect();
      AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
      AutoScroll();
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cEditBoxWidget::OnPaste(ni::iDataTable* apDT)
{
  if (apDT && !niFlagIs(mpWidget->GetStyle(),eWidgetEditBoxStyle_ReadOnly)) {
    cString strType = apDT->GetString(_A("type"));
    if (strType.IEq(_A("drop"))) {
      cString str = apDT->GetString(_A("text"));
      if (str.IsNotEmpty()) {
        DoSetTextBuffer(str.Chars());
        if (mpWidget->GetStyle()&eWidgetEditBoxStyle_PasteValidate) {
          mpWidget->SendCommand(mpWidget->GetParent(),eWidgetEditBoxCmd_Validated);
        }
      }
    }
    else {
      cString strText = apDT->GetString(_A("text"));
      if (!strText.empty()) {
        Position a, b;
        bool changeSel = !mSelection.IsEmpty();
        if (changeSel) {
          DeleteSelection();
          a = mbufText.GetCursor();
        }
        mbufText.AddText(strText.Chars());
        if (changeSel) {
          b = mbufText.GetCursor();
          if (a != b) {
            mSelection = Selection(a,b,stRange);
          }
        }

        sRectf rect = mpWidget->GetWidgetRect();
        AdjustChildrenLayout(rect.GetWidth(),rect.GetHeight());
        AutoScroll();
      }
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
Selection _ToSelectionClamped(const sVec4i& aRange, const Selection& aClamp) {
  return Selection(
      Position(ni::Max(aRange.x,aClamp._start._line),ni::Max(aRange.y,aClamp._start._col)),
      Position(ni::Min(aRange.z,aClamp._end._line),ni::Min(aRange.w,aClamp._end._col)),
      stNormal);
}
Selection _ToSelection(const sVec4i& aRange) {
  return Selection(
      Position(aRange.x,aRange.y),
      Position(aRange.z,aRange.y),
      stNormal);
}
sVec4i _FromSelection(const Selection& aSel) {
  return Vec4i(aSel._start._line,aSel._start._col,
               aSel._end._line,aSel._end._col);
}

cString __stdcall cEditBoxWidget::GetTextInRange(const sVec4i& aRange) const {
  const Selection sel = _ToSelectionClamped(aRange, Selection(mbufText.GetStartPosition(), mbufText.GetEndPosition(), stNormal));
  cString strData;
  mbufText.GetSelectionData(sel,strData);
  return strData;
}
sVec4i __stdcall cEditBoxWidget::GetAllTextRange() const {
  return _FromSelection(Selection(mbufText.GetStartPosition(), mbufText.GetEndPosition(), stNormal));
}
void __stdcall cEditBoxWidget::SetSelection(const sVec4i& aRange) {
  mSelection = _ToSelectionClamped(aRange, Selection(mbufText.GetStartPosition(), mbufText.GetEndPosition(), stNormal));
}
sVec4i __stdcall cEditBoxWidget::GetSelection() const {
  return _FromSelection(mSelection);
}
