#include "stdafx.h"
#include "TextMetric.h"
#include "FontTTF.h"
#include <niLang/Utils/UnitSnap.h>

using namespace ni;

#define TEXT_METRIC_TRACE(FMT) //niDebugFmt(FMT)

tF32 sDisplayGlyph::Compute(
  const cFont* apFont,
  const tF32 afFontSizeDivByRes,
  const tF32 afX, const tF32 afY,
  const tU32 anPrevChar,
  const tU32 anChar)
{
  sFontGlyph tg;
  const sFontGlyph* g = apFont->GetGlyph(anChar,&tg);
  const sVec2f lead = g->lead * afFontSizeDivByRes;
  tF32 advance = g->xadv * afFontSizeDivByRes;

  if (g->img.IsOK()) {
    this->displayTexCoo = g->texCoo;
    this->tex = g->img->GrabTexture(eImageUsage_Source,sRecti::Null());

    {
      const tF32 fW = apFont->GetCharRectWidth(this->displayTexCoo);
      const tF32 fH = apFont->GetCharRectHeight(this->displayTexCoo);

      sVec4f dpos;
      dpos.x = afX;
      dpos.y = afY;

      if (anPrevChar) {
        const tF32 kerning = apFont->GetKerning(anPrevChar, anChar);
        if (kerning != 0) {
          TEXT_METRIC_TRACE(("... TEXT_METRIC: KERN %d-%d (%c-%c): %g",
                             anPrevChar, anChar, anPrevChar, anChar,
                             kerning));
          advance += kerning;
          dpos.x += kerning;
        }
      }

      dpos.x += lead.x;
      dpos.y += lead.y;
      dpos.z = dpos.x + fW;
      dpos.w = dpos.y + fH;
      this->displayPos = dpos;
    }

    {
      sVec2f texTL;
      sVec2f texBR;
      if (apFont->mStates.mfYSign < 0) {
        texTL = Vec2(this->displayTexCoo.GetLeft(),this->displayTexCoo.GetBottom());
        texBR = Vec2(this->displayTexCoo.GetRight(),this->displayTexCoo.GetTop());
      }
      else {
        texTL = this->displayTexCoo.GetTopLeft();
        texBR = this->displayTexCoo.GetBottomRight();
      }
      this->displayTexCoo = sRectf(texTL,texBR);
    }
  }

  return advance;
}

void TextLineMetric_Clear(sTextLineMetric* apTM) {
  apTM->glyphs.clear();
  apTM->rect = sRectf::Null();
}

tU32 TextLineMetric_AddText(
  sTextLineMetric* apTM,
  const iFont* apFont,
  const tF32 afStartX, const tF32 afStartY,
  const achar* aaszText, const tU32 anTextLen,
  const tBool abKerning)
{
  cFont* font = (cFont*)apFont;
  const tBool kerning = abKerning && font->GetHasKerning();

  astl::vector<sDisplayGlyph>& glyphs = apTM->glyphs;
  const tU32 prevGlyphLen = (tU32)glyphs.size();
  glyphs.reserve(prevGlyphLen + anTextLen);

  sDisplayGlyph gm;
  const tF32 fontSizeByRes = ni::FDiv(font->mStates.GetWidth(),(tF32)font->mStates.mnResolution);
  const tF32 lineHeight = TextLineMetric_ComputeLineHeight(font);

  sDisplayGlyph gmSpace;
  const tF32 spaceAdv = gmSpace.Compute(font,fontSizeByRes,0,0,0,' ');
  const tF32 tabSize = apFont->GetTabSize()*spaceAdv;

  tU32 prevch = 0;
  tU32 ch = 0;
  const tF32 y = afStartY;
  tF32 x = afStartX;
  StrCharIt it(aaszText,aaszText+anTextLen);
  while (!it.is_end()) {
    prevch = ch;
    ch = it.next();
    if (ch == ' ') {
      gm = gmSpace;
      gm.displayPos.Move(Vec2f(x,y));
      gm.displayPos.SetWidth(spaceAdv);
      glyphs.push_back(gm);
      x += spaceAdv;
    }
    else if (ch == '\t') {
      const tF32 tabAdv = tabSize-::fmodf(x-afStartX,tabSize);
      gm = gmSpace;
      gm.displayPos.Move(Vec2f(x,y));
      gm.displayPos.SetWidth(tabAdv);
      glyphs.push_back(gm);
      x += tabAdv;
    }
    else if (ch < 32) {
      // skip invisible characters
    }
    else {
      const tF32 adv = gm.Compute(font,fontSizeByRes,x,y,kerning ? prevch : 0,ch);
      glyphs.push_back(gm);
      x += adv;
    }
  }

  const tU32 glyphsAdded = (tU32)glyphs.size() - prevGlyphLen;
  if (glyphsAdded > 0) {
    apTM->rect.Add(
      Vec2f(glyphs[prevGlyphLen].displayPos.x, afStartY),
      Vec2f(glyphs.back().displayPos.z, y + lineHeight));
  }
  return glyphsAdded;
};

void TextLineMetric_Move(
  sTextLineMetric* apTM,
  tF32 afOffX, tF32 afOffY)
{
  if (!niFloatIsZero(afOffX) && !niFloatIsZero(afOffY)) {
    niLoop(i,apTM->glyphs.size()) {
      sDisplayGlyph& gm = apTM->glyphs[i];
      gm.displayPos.x += afOffX;
      gm.displayPos.z += afOffX;
      gm.displayPos.y += afOffY;
      gm.displayPos.w += afOffY;
    }
    apTM->rect.x += afOffX;
    apTM->rect.z += afOffX;
    apTM->rect.y += afOffY;
    apTM->rect.w += afOffY;
  }
  else if (!niFloatIsZero(afOffX)) {
    niLoop(i,apTM->glyphs.size()) {
      sDisplayGlyph& gm = apTM->glyphs[i];
      gm.displayPos.x += afOffX;
      gm.displayPos.z += afOffX;
    }
    apTM->rect.x += afOffX;
    apTM->rect.z += afOffX;
  }
  else if (!niFloatIsZero(afOffY)) {
    niLoop(i,apTM->glyphs.size()) {
      sDisplayGlyph& gm = apTM->glyphs[i];
      gm.displayPos.y += afOffY;
      gm.displayPos.w += afOffY;
    }
    apTM->rect.y += afOffY;
    apTM->rect.w += afOffY;
  }
}

tFontFormatFlags TextLayout_ValidateFlags(const sRectf& aRect, tFontFormatFlags aFlags) {
  if (aRect.GetWidth() == 0) {
    niFlagOff(aFlags, eFontFormatFlags_Right);
    niFlagOff(aFlags, eFontFormatFlags_CenterH);
    niFlagOff(aFlags, eFontFormatFlags_ClipH);
  }
  if (aRect.GetHeight() == 0) {
    niFlagOff(aFlags, eFontFormatFlags_Bottom);
    niFlagOff(aFlags, eFontFormatFlags_CenterV);
    niFlagOff(aFlags, eFontFormatFlags_ClipV);
  }
  return aFlags;
}

niExportFuncCPP(sRectf) TextLayout_Compute(
  const iFont* apFont,
  const sRectf& aRect,
  const achar* aaszText,
  const tU32 anTextLen,
  tFontFormatFlags* apFlags,
  astl::vector<sTextLineMetric>& aMetrics)
{
  sRectf bbRect = sRectf::Null();
  const tFontFormatFlags flags = *apFlags = TextLayout_ValidateFlags(aRect,*apFlags);

  const tBool bUnitSnap = niFlagIsNot(flags,eFontFormatFlags_NoUnitSnap);
  const tBool bKerning = apFont->GetHasKerning() && niFlagIs(flags,eFontFormatFlags_Kerning);

  tU32 ch = 0;
  const tF32 start_y = bUnitSnap ? ni::UnitSnapf(aRect.y) : aRect.y;
  const tF32 start_x = bUnitSnap ? ni::UnitSnapf(aRect.x) : aRect.x;
  tF32 x = start_x;
  tF32 y = start_y;

  const tF32 lineHeight = TextLineMetric_ComputeLineHeight(apFont);

  StrCharIt it(aaszText, (achar*)NULL, (tSize)anTextLen);
  cFont* f = (cFont*)apFont;

  // Process all the lines
  const tBool bWrapText = niFlagIs(flags,eFontFormatFlags_WrapText);

  sDisplayGlyph gm;
  const tF32 fontSizeByRes = ni::FDiv(f->mStates.GetWidth(),(tF32)f->mStates.mnResolution);

  sDisplayGlyph gmSpace;
  const tF32 spaceAdv = gmSpace.Compute(f,fontSizeByRes,0,0,0,' ');
  const tF32 tabSize = apFont->GetTabSize()*spaceAdv;

  tU32 prevch = 0;

  astl::push_back(aMetrics);

  auto addLine = [&] (tBool lastLine = eFalse) {
    // update Y position
    auto h = lastLine ? apFont->GetHeight() : lineHeight;
    y = bUnitSnap ? ni::UnitSnapf(y + h) : (y + h);

    if ((tU32)aMetrics.back().glyphs.size() > 0) {
      // aMetrics.back().glyphs.back().displayPos.z
      aMetrics.back().rect.Add(
        Vec2f(start_x, start_y),
        Vec2f(aMetrics.back().glyphs.back().displayPos.z, y));
    }
    bbRect.Add(aMetrics.back().rect);

    if (niFlagIs(flags,eFontFormatFlags_CenterH)) {
      tF32 offX = (aRect.GetWidth() - aMetrics.back().rect.GetWidth()) / 2;
      if (bUnitSnap) {
        offX = ni::UnitSnapf(offX);
      }
      TextLineMetric_Move(&aMetrics.back(), offX, 0);
    }
    else if (niFlagIs(flags,eFontFormatFlags_Right)) {
      tF32 offX = (aRect.GetWidth() - aMetrics.back().rect.GetWidth());
      if (bUnitSnap) {
        offX = ni::UnitSnapf(offX);
      }
      TextLineMetric_Move(&aMetrics.back(), offX, 0);
    }

    astl::push_back(aMetrics);

    // reset X position
    x = aRect.x;
  };

  while (!it.is_end()) {
    prevch = ch;
    ch = it.next();

    if (ch == ' ') {
      if (bWrapText && x + spaceAdv > aRect.z) {
        addLine();
      }

      gm = gmSpace;
      gm.displayPos.Move(Vec2f(x,y));
      gm.displayPos.SetWidth(spaceAdv);
      aMetrics.back().glyphs.push_back(gm);
      x += spaceAdv;
    }
    else if (ch == '\t') {
      const tF32 tabAdv = tabSize-::fmodf(x-aRect.x,tabSize);
      if (bWrapText && x + tabAdv > aRect.z) {
        addLine();
      }

      gm = gmSpace;
      gm.displayPos.Move(Vec2f(x,y));
      gm.displayPos.SetWidth(tabAdv);
      aMetrics.back().glyphs.push_back(gm);
      x += tabAdv;
    }
    else if (ch == '\n') {
      addLine();
    }
    else if (ch < 32) {
      // skip invisible characters
    }
    else {
      tF32 adv = gm.Compute(f,fontSizeByRes,x,y,bKerning ? prevch : 0, ch);
      if (bWrapText && gm.displayPos.z > aRect.z) {
        addLine();
        // recompute the glyph location when new line started
        adv = gm.Compute(f,fontSizeByRes,x,y,bKerning ? prevch : 0, ch);
      }

      aMetrics.back().glyphs.push_back(gm);
      x += adv;
    }
  }
  addLine(true);

  // Vertical alignment
  if (niFlagIs(flags,eFontFormatFlags_CenterV)) {
    tF32 offY = (aRect.GetHeight() - bbRect.GetHeight()) / 2;
    if (bUnitSnap) {
      offY = ni::UnitSnapf(offY);
    }
    niLoop(i,aMetrics.size()) {
      sTextLineMetric& tm = aMetrics[i];
      TextLineMetric_Move(&tm, 0, offY);
    }
    bbRect.Move(Vec2f(0,offY));
  }
  else if (niFlagIs(flags,eFontFormatFlags_Bottom)) {
    tF32 offY = (aRect.GetHeight() - bbRect.GetHeight());
    if (bUnitSnap) {
      offY = ni::UnitSnapf(offY);
    }
    niLoop(i,aMetrics.size()) {
      sTextLineMetric& tm = aMetrics[i];
      TextLineMetric_Move(&tm, 0, offY);
    }
    bbRect.Move(Vec2f(0,offY));
  }

  return bbRect;
}
