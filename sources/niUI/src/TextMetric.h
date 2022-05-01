#pragma once
#ifndef __TEXTMETRIC_H_5258E94E_7174_4DCF_80AC_A1022DB00D35__
#define __TEXTMETRIC_H_5258E94E_7174_4DCF_80AC_A1022DB00D35__

#include "Font.h"

//----------------------------------------------------------------------
//
//  sDisplayGlyph
//
//----------------------------------------------------------------------
namespace ni {

struct sDisplayGlyph {
  Ptr<iTexture> tex;
  sRectf displayPos = sRectf::Null();
  sRectf displayTexCoo = sRectf::Null();

  // Kerning is used when anPrevChar is != 0
  tF32 Compute(
    const cFont* apFont,
    const tF32 afFontSizeDivByRes,
    const tF32 afX, const tF32 afY,
    const tU32 anPrevChar,
    const tU32 anChar);

  ///////////////////////////////////////////////
  __forceinline void PushToCanvas(
    ni::iCanvas* apCanvas,
    const cFont* apFont,
    const tU32 aColFront,
    const sVec3f& avPos) const
  {
    iTexture* ogTex = tex;
    if (!ogTex)
      return; // invisible character

    if (!apCanvas->GetMaterial() ||
        apCanvas->GetMaterial() != apFont->mStates.mptrMaterial ||
        apFont->mStates.mptrMaterial->GetChannelTexture(eMaterialChannel_Base) != ogTex)
    {
      apCanvas->Flush();
      apFont->mStates.mptrMaterial->SetChannelTexture(eMaterialChannel_Base,ogTex);
      apCanvas->SetMaterial(apFont->mStates.mptrMaterial);
    }

    const sVec2f tlPos = {
      displayPos.x + avPos.x,
      displayPos.y + avPos.y
    };
    const sVec2f brPos = {
      displayPos.z + avPos.x,
      displayPos.w + avPos.y
    };
    const sVec2f tlTexCoo = {
      displayTexCoo.x,
      displayTexCoo.y
    };
    const sVec2f brTexCoo = {
      displayTexCoo.z,
      displayTexCoo.w
    };

    apCanvas->RectTA(
      tlPos, brPos,
      tlTexCoo, brTexCoo,
      avPos.z,
      aColFront);
  }
};

//----------------------------------------------------------------------
//
//  TextLineMetric
//
//----------------------------------------------------------------------
struct sTextLineMetric {
  astl::vector<sDisplayGlyph> glyphs;
  sRectf rect = sRectf::Null();
};
} // end of namespace ni

static __forceinline tBool TextLineMetric_IsEmpty(const sTextLineMetric* apTM) {
  return apTM->glyphs.empty();
}
void TextLineMetric_Clear(sTextLineMetric* apTM);

// Skips all character < 32. New lines, etc are not handled by this function.
tU32 TextLineMetric_AddText(sTextLineMetric* apTM, const iFont* apFont,
                            const tF32 afStartX, const tF32 afStartY,
                            const achar* aaszText, const tU32 anTextLen,
                            const tBool abKerning);

///////////////////////////////////////////////
static __forceinline void TextLineMetric_PushToCanvas(
  const sTextLineMetric* apTM,
  ni::iCanvas* apCanvas,
  const iFont* apFont,
  const tU32 aColFront,
  const sVec3f& avPos,
  // apRectClipH is in "position space", so it assume that it has been offset
  // by "avPos" aswell
  const sRectf* apRectClipH)
{
  cFont* font = (cFont*)apFont;
  const sDisplayGlyph* gm = apTM->glyphs.data();
  if (apRectClipH) {
    const tF32 l = apRectClipH->Left();
    const tF32 r = apRectClipH->Right();
    const tU32 numGlyphs = (tU32)apTM->glyphs.size();
    tU32 i = 0;
    // find the first visible glyph
    for ( ; i < numGlyphs; ++i, ++gm) {
      if ((gm->displayPos.x+avPos.x) >= l)
        break;
    }
    // draw all the visible glyphs
    for ( ; i < numGlyphs; ++i, ++gm) {
      if ((gm->displayPos.z+avPos.x) > r)
        break;
      gm->PushToCanvas(apCanvas,font,aColFront,avPos);
    }
  }
  else {
    niLoop(i,apTM->glyphs.size()) {
      gm->PushToCanvas(apCanvas,font,aColFront,avPos);
      ++gm;
    }
  }
}

///////////////////////////////////////////////
static __forceinline sRectf TextLineMetric_ComputeRect(const iFont* apFont, const achar* aaszText, const tU32 anTextLen, const tBool abKerning) {
  sTextLineMetric tm;
  TextLineMetric_AddText(&tm, apFont, 0, 0, aaszText, anTextLen, abKerning);
  return tm.rect;
}
static __forceinline sRectf TextLineMetric_ComputeRect(const iFont* apFont, const cString& aStr, const tBool abKerning) {
  return TextLineMetric_ComputeRect(apFont, aStr.Chars(), aStr.size(), abKerning);
}

///////////////////////////////////////////////
static __forceinline tF32 TextLineMetric_ComputeLineHeight(const iFont* apFont) {
  return apFont->GetLineHeight();
}

//----------------------------------------------------------------------
//
//  TextLayout
//
//----------------------------------------------------------------------
niExportFuncCPP(sRectf) TextLayout_Compute(
  const iFont* apFont,
  const sRectf& aRect,
  const achar* aaszText,
  const tU32 anTextLen,
  // apFlags are the initial formatting flags, the validated flags are written
  // back in there
  tFontFormatFlags* aFlags,
  astl::vector<sTextLineMetric>& aMetrics);

///////////////////////////////////////////////
// apRectClipH & apRectClipV are in "position space", so it assume that it has
// been offset by "avPos" aswell.
static __forceinline void TextLayout_PushToCanvas(
  const sTextLineMetric* apTextLineMetrics,
  const tU32 anNumTM,
  ni::iCanvas* apCanvas,
  const iFont* apFont,
  const tU32 aColFront,
  const sVec3f& avPos,
  const sRectf* apRectClipH,
  const sRectf* apRectClipV)
{
  if (apRectClipV) {
    const tF32 t = apRectClipV->Top();
    const tF32 b = apRectClipV->Bottom();
    tU32 i = 0;
    const sTextLineMetric* tm = apTextLineMetrics;
    // find the first visible line
    for ( ; i < anNumTM; ++i, ++tm) {
      if ((tm->rect.y+avPos.y) >= t)
        break;
    }
    // draw all the visible lines
    for ( ; i < anNumTM; ++i, ++tm) {
      if ((tm->rect.w+avPos.y) > b)
        break;
      TextLineMetric_PushToCanvas(
        tm,apCanvas,apFont,aColFront,avPos,apRectClipH);
    }
  }
  else {
    niLoop(i,anNumTM) {
      TextLineMetric_PushToCanvas(
        &apTextLineMetrics[i],apCanvas,apFont,aColFront,avPos,apRectClipH);
    }
  }
}

#endif // __TEXTMETRIC_H_5258E94E_7174_4DCF_80AC_A1022DB00D35__
