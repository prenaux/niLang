#ifndef __EDITBOXWIDGET_3414480_H__
#define __EDITBOXWIDGET_3414480_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/Types.h>
#include <niLang/Utils/UnknownImpl.h>
#include <niLang/STL/utils.h>
#include "API/niUI/IWidget.h"
#include "Line.h"
#include "EditorBuffer.h"
#include "TextMetric.h"

class cEditBoxWidget : public ImplRC<iWidgetSink,eImplFlags_Default,iWidgetEditBox>
{
  niBeginClass(cEditBoxWidget);
 public:
  cEditBoxWidget(iWidget *pWidget);
  ~cEditBoxWidget();

  //iWidgetSink interface
  tBool __stdcall OnWidgetSink(iWidget *apWidget, tU32 nMsg, const Var& varParam0, const Var& varParam1);
  //iWidgetEditBox
  void __stdcall AddText(const achar* aaszText);
  //internals
  void Paint(iCanvas* apCanvas);
  void ComputeClientRect(tF32 w,tF32 h);
  void AdjustChildrenLayout(tF32 w,tF32 h);
  Position TranslateMousePos(tF32 x,tF32 y);
  void __stdcall AutoScroll(tBool abForceRefresh);
  void __stdcall AutoScroll();
  void DeleteSelection();
  void ComputeWidestLine();
  void __stdcall MoveCursorLineDelta(tI32 line);
  void __stdcall MoveCursorColumnDelta(tI32 col);
  bool OnKeyDown(tU32 key,tU32 mod,tU32 c);
  void OnLeftClickUp(const sVec2f &mpos);
  void OnLeftClickDown(const sVec2f &mpos);
  void OnMouseSelect(const sVec2f &mpos);
  void UpdateStyle();
  tBool __stdcall SetTextBuffer(iHString* ahspText);
  Ptr<iHString> __stdcall GetTextBuffer() const;
  tBool __stdcall OnCopy(ni::iDataTable* apDT);
  tBool __stdcall OnCut(ni::iDataTable* apDT);
  tBool __stdcall OnPaste(ni::iDataTable* apDT);

  void __stdcall SetReplaceChar(tU32 anChar);
  tU32 __stdcall GetReplaceChar() const;

  void __stdcall MoveCursorHome(tBool abLine);
  void __stdcall MoveCursorEnd(tBool abLine);
  void __stdcall MoveCursor(const sVec2i& pos, tBool abForceLogicalCol);
  void __stdcall MoveCursorWordDelta(tI32 word);
  void __stdcall MoveCursorParaDelta(const tI32 aDelta);

  sVec2i __stdcall GetCursorPosition() const;
  tU32 __stdcall GetCursorLine() const;
  tU32 __stdcall GetCursorColumn() const;

  tF32 __stdcall GetLineHeight() const;

  cString __stdcall GetTextInRange(const sVec4i& aRange) const;
  sVec4i __stdcall GetAllTextRange() const;
  void __stdcall SetSelection(const sVec4i& aSelection);
  sVec4i __stdcall GetSelection() const;

 private:
  tBool __stdcall DoSetTextBuffer(const achar* aaszText);

  iWidget *mpWidget;
  iWidget *mpLeftScrollbar;
  iWidget *mpBottomScrollbar;
  iWidgetScrollBar *mpsbScrollBarLeft;
  iWidgetScrollBar *mpsbScrollBarBottom;
  EditorBuffer mbufText;
  Selection mSelection;
  Position mSelPivot;
  tBool mbCursorVisible;
  tBool mbDragging;
  tU32 mnTabSize;
  tF32 mfHScrollOffs;
  tF32 mfWidestLine;
  tU32 mnWidestLineNumOfChars;
  tU32 mnReplaceChar;
  tBool mbDontMoveCursorOnNextClickDown;

  struct sGlyphCache {
    sDisplayGlyph gm;
    tU32          color;
  };
  astl::vector<sGlyphCache> mGlyphCache;

  void ResetGlyphCache(tU32 anReserve) {
    mGlyphCache.clear();
    mGlyphCache.reserve(anReserve);
  }
  tF32 PushGlyph(const iFont* apFont, tF32 x, tF32 y, tU32 anChar, tU32 fg, const tF32 afFontSizeDivByRes) {
    sGlyphCache& gc = astl::push_back(mGlyphCache);
    gc.color = fg;
    return gc.gm.Compute(
      (const cFont*)apFont,
      afFontSizeDivByRes,
      x, y, 0,
      anChar);
  }

  void InitSkin();
  struct sSkin {
    Ptr<iOverlay> normalFrame;
    Ptr<iOverlay> hoverFrame;
    Ptr<iOverlay> focusedFrame;
    Ptr<iOverlay> curText;
    Ptr<iOverlay> curHand;
    sColor4f    colFrontNormal;
    sColor4f    colFrontSel;
    sColor4f    colBackSel;
  } skin;
  niEndClass(cEditBoxWidget);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __EDITBOXWIDGET_3414480_H__
