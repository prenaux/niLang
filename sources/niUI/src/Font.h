// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#ifndef __FONT_H__
#define __FONT_H__

#include "Graphics.h"
#include "niUI_HString.h"

namespace ni {

class cGraphics;
class cFontTTF;
struct sMyFTLibrary;

niConstValue auto kFontFilterOnSS = eCompiledStates_SS_SharpClamp;
niConstValue auto kFontFilterOffSS = eCompiledStates_SS_PointClamp;

struct sFontGlyph {
  Ptr<iBitmap2D> bmp;
  Ptr<iImage>    img;
  sVec2f         lead;
  sVec2i         dim;
  sRectf         texCoo;
  tF32           xadv;
  tU32           glyphIndex;

  sFontGlyph()
      : xadv(0)
      , lead(sVec2f::Zero())
      , dim(sVec2i::Zero())
      , texCoo(sRectf::Null())
      , glyphIndex(0)
  {}

  inline void SetInvalidGlyph() {
    this->dim = Vec2i(-1,-1);
  }
  inline tBool IsValidGlyph() const {
    return dim.x != -1 || dim.y != -1;
  }

  inline tI32 GetWidth() const {
    return dim.x;
  }
  inline tI32 GetHeight() const {
    return dim.y;
  }

  inline void SetGlyphInBitmap(tU32 anChar, const sVec2f& avSize,
                               const tF32 afFontSizeDivByRes) {
    const tF32 x = float(anChar&0xF)*(1/16.0f);
    const tF32 y = float(anChar>>4)*(1/16.0f);
    texCoo.SetLeft(x);
    texCoo.SetTop(y);
    texCoo.SetRight(x+(1/16.0f));
    texCoo.SetBottom(y+(1/16.0f));
    dim.x = (tI32)avSize.x;
    dim.y = (tI32)avSize.y;
    // '/ afFontSizeDivByRes' to match the convention used by ttf fonts
    xadv = avSize.x / afFontSizeDivByRes;
    lead = sVec2f::Zero();
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Font interface implementation
class cFont : public ImplRC<iFont>
{
 public:
  cFont(cGraphics* pFontFactory);
  ~cFont();

  tBool _CreateFromFile(iHString* ahspName, iFile* pFile);
  tBool _CreateFromBitmap(iHString* ahspName, iBitmap2D* pBmp);
  tBool _Create8x8(iHString* ahspName);

  //// iFont /////////////////////////////////
  tBool __stdcall IsOK() const;
  void __stdcall Invalidate();

  iHString* __stdcall GetName() const;
  iHString* __stdcall GetFamilyName() const;
  iHString* __stdcall GetStyleName() const;
  iHString* __stdcall GetFilePath() const;
  tBool __stdcall GetIsInstance() const;
  tBool __stdcall GetIsFixedResolution() const;
  tBool __stdcall GetHasKerning() const;
  tU32 __stdcall GetMaxResolution() const;

  iFont* __stdcall CreateFontInstance(iMaterial* apMaterial);

  void __stdcall SetTabSize(tU32 ulNumChar);
  tU32 __stdcall GetTabSize() const;
  void __stdcall SetFiltering(tBool abFiltering);
  tBool __stdcall GetFiltering() const;
  void __stdcall SetDistanceField(tBool abDistanceField);
  tBool __stdcall GetDistanceField() const;
  void __stdcall SetColor(tU32 col);
  tU32 __stdcall GetColor() const;
  void __stdcall SetBlendMode(eBlendMode aBlend);
  eBlendMode __stdcall GetBlendMode() const;
  void __stdcall SetSizeAndResolution(const sVec2f& avSize, const tU32 anResolution, const tF32 afContentsScale);
  tU32 __stdcall GetResolution() const;
  sVec2f __stdcall GetSize() const;

  tF32 __stdcall GetCharWidth(tU32 c) const;
  tF32 __stdcall GetCharHeight(tU32 c) const;

  tF32 __stdcall GetAscent() const;
  tF32 __stdcall GetDescent() const;
  tF32 __stdcall GetMaxCharWidth() const;
  tF32 __stdcall GetAverageCharWidth() const;
  tF32 __stdcall GetMaxCharHeight() const;
  tF32 __stdcall GetAverageCharHeight() const;
  tF32 __stdcall GetHeight() const;
  tU32 __stdcall GetFirstChar() const;
  tU32 __stdcall GetLastChar() const;

  tBool __stdcall CacheText(const achar* aaszText);
  tBool __stdcall CacheRange(tU32 anFirst, tU32 anLast);
  tBool __stdcall ClearCache();
  tIntPtr __stdcall GetCacheID() const;

  tBool __stdcall _UpdateMaterial(ni::tBool abUpdateMaterialStates);
  sRectf __stdcall GetCharTexCoo(tU32 c) const;
  iTexture* __stdcall GetTexture();
  iMaterial* __stdcall GetMaterial() const;

  tF32 __stdcall GetAdvance(tU32 anChar) const;
  tF32 __stdcall GetKerning(tU32 a, tU32 b) const;

  void __stdcall SetLineSpacing(const tF32 afSpacing);
  tF32 __stdcall GetLineSpacing() const;
  tF32 __stdcall GetLineHeight() const;

  sVec2i __stdcall BlitChar(iBitmap2D* apBmp, tI32 anX, tI32 anY, tU32 anChar) {
    return BlitCharEx(apBmp,anX,anY,0,0,anChar,eBlendMode_Translucent);
  }
  sVec2i __stdcall BlitCharStretch(iBitmap2D* apBmp, tI32 anX, tI32 anY, tU32 anChar) {
    return BlitCharEx(apBmp,anX,anY,(tI32)GetCharWidth(anChar),(tI32)GetCharHeight(anChar),anChar,eBlendMode_Translucent);
  }
  sVec2i __stdcall BlitCharEx(iBitmap2D* apBmp, tI32 anX, tI32 anY, tI32 anW, tI32 anH, tU32 anChar, eBlendMode aBlendMode);
  iTexture* __stdcall GetCharTexture(tU32 anChar) const;
  iBitmap2D* __stdcall GetCharBitmap(tU32 anChar) const;

  sRectf __stdcall ComputeTextSize(const sRectf& aRect, const achar* aaszText, tFontFormatFlags aFormat);
  sRectf __stdcall DrawText(ni::iCanvas* apCanvas, const sRectf& aRect, tF32 afZ, const achar* aaszText, tFontFormatFlags aFormat);

  tU32 __stdcall GetGlyphIndexFromName(const achar* aaszName) const;
  tU32 __stdcall GetGlyphIndexFromCodepoint(const tU32 anCodepoint) const;
  cString __stdcall GetGlyphName(const tU32 anGlyphIndex) const;
  Ptr<tU32CMap> __stdcall EnumGlyphs() const;
  tU32 __stdcall GetGlyphCodePointFromName(const achar* aaszName) const;

  sVec2f __stdcall GetGlyphPath(iVGPath* apPath, tU32 anGlyphIndex, const sVec2f& avOffset, const tF32 afScale) const;
  ///////////////////////////////////////////////

  const sFontGlyph* GetGlyph(tU32 anChar, sFontGlyph* apTmpStorage) const;

  tF32 GetCharRectWidth(const sRectf& rectTexCoo) const;
  tF32 GetCharRectHeight(const sRectf& rectTexCoo) const;

  tU32 GetTextureWidth() const;
  tU32 GetTextureHeight() const;

 public:
  WeakPtr<cGraphics> mpwGraphics;
  tHStringPtr        mhspName;
  tHStringPtr        mhspFamilyName;
  tHStringPtr        mhspStyleName;
  tHStringPtr        mhspFilePath;
  tHStringPtr        mhspFileName;
  Ptr<iFont>         mptrParentFont;
  Ptr<iImage>        mptrImage;
  Ptr<cFontTTF>      mptrTTF;

  struct sStates
  {
    Ptr<iMaterial> mptrMaterial;
    tF32 mfTabSize;
    tU32 mnColor;
    tU32 mnResolution;
    sVec2f mvSize;
    tF32 mfInvResolution;
    tF32 mfYSign;
    tBool mbFiltering;
    tBool mbDistanceField;
    tBool mbMaterialInstanced;
    tF32 mfLineSpacing;

    sStates(iGraphics* apGraphics) {
      mfTabSize = 4;
      mfYSign = 1.0f;
      mbFiltering = eTrue;
      mbDistanceField = eFalse;
      mnColor = 0xFFFFFFFF;
      mnResolution = 0;
      mvSize = sVec2f::Zero();
      mptrMaterial = NULL;
      mbMaterialInstanced = eFalse;
      mfLineSpacing = 1.0f;
      mptrMaterial = apGraphics->CreateMaterial();
      mptrMaterial->SetName(_HC(Default));
      mptrMaterial->SetFlags(mptrMaterial->GetFlags()|
                             eMaterialFlags_Transparent|
                             eMaterialFlags_Translucent|
                             eMaterialFlags_NoLighting|
                             eMaterialFlags_DoubleSided|
                             eMaterialFlags_Vertex);
      mptrMaterial->SetBlendMode(eBlendMode_Translucent);
      mptrMaterial->SetChannelSamplerStates(eMaterialChannel_Base,kFontFilterOnSS);
    }
    tF32 GetWidth() const {
      return mvSize.x;
    }
    tF32 GetHeight() const {
      return mvSize.y;
    }
    void SetResolution(tU32 anSize) {
      niAssert(anSize > 0);
      mnResolution = anSize;
      mfInvResolution = 1.0f/tF32(mnResolution);
    }
    tU32 GetResolution() const {
      return mnResolution;
    }
    void NewMaterial(iGraphics* apGraphics) {
    }
   private:
    sStates(const sStates&);
  };
  sStates mStates;
};
} // end of namespace ni

#endif // __FONT_H__
