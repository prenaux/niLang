#ifndef __FONTTTF_H__
#define __FONTTTF_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "FreetypeWrapper.h"
#include "TextMetric.h"

namespace ni {

class cGraphics;
typedef tU64 tGlyphHash;

// True type loader class
class cFontTTF : public ImplRC<iUnknown,eImplFlags_Default>
{
 public:
  typedef astl::hash_map<tGlyphHash,sFontGlyph>  tGlyphCacheHMap;

  cFontTTF(cGraphics* apGraphics, sMyFTLibrary *apLibrary, iFile* pFile);
  ~cFontTTF();

  tBool __stdcall IsOK() const;

  tBool GetCharSize(sVec2f& avOut, tU32 anChar, int anSize, tF32 afRealSize, tBool abDistanceField);
  tF32 GetCharWidth(tU32 anChar, int anSize, tF32 afRealSize, tBool abDistanceField);
  tF32 GetCharHeight(tU32 anChar, int anSize, tF32 afRealSize, tBool abDistanceField);
  tBool RenderGlyph(sFontGlyph* baseGlyph, tU32 anSize, tBool abDistanceField);
  const sFontGlyph* RenderGlyphIndex(tU32 anGlyphIndex, tU32 anSize, tBool abDistanceField);
  const sFontGlyph* RenderCodepoint(tU32 anCodepoint, tU32 anSize, tBool abDistanceField);

  cString GetFamilyName() const;
  cString GetStyleName() const;
  cString GetName() const;
  tU32    GetMaxCharSize() const { return 256; }

  tF32    GetAscender(tF32 afRealSize) const;
  tF32    GetDescender(tF32 afRealSize) const;
  tF32    GetLineHeight(tF32 afRealSize) const;
  FT_Face GetFTFace() const { return mFace; }
  __forceinline tBool GetIsFixedWidth() const { return FT_IS_FIXED_WIDTH(mFace) != 0; }
  __forceinline tBool GetHasKerning() const { return FT_HAS_KERNING(mFace) != 0; }
  __forceinline int GetCodepointGlyphIndex(tU32 anCodepoint) {
    return FT_Get_Char_Index(mFace, anCodepoint);
  }

  tI32 GetKerning(tU32 anPixelSize, tU32 anFirst, tU32 anSecond) const;
  tF32 GetAdvanceGlyphIndex(tU32 anPixelSize, tBool abDistanceField, tU32 anGlyphIndex) const;
  tF32 GetAdvanceCodepoint(tU32 anPixelSize, tBool abDistanceField, tU32 anCodepoint) const;

  sFontGlyph* CacheGlyphIndex(tU32 anGlyphIndex, tU32 anSize, tBool abDistanceField);
  sFontGlyph* CacheCodepoint(tU32 anCodepoint, tU32 anSize, tBool abDistanceField, cFontTTF** apRenderFont);
  tBool UncacheGlyphIndex(tU32 anGlyphIndex, tU32 anSize, tBool abDistanceField);

  tU32 __stdcall GetPageSize() const { return mnImageMapPageSize; }

  iImageMap* GetImageMap();
  tBool HasImageMap() const { return mptrImageMap.IsOK(); }

  void ClearCache();
  tIntPtr GetCacheID() const { return (tIntPtr)mptrImageMap.ptr(); }

  sVec2f __stdcall GetGlyphPath(iVGPath* apPath, tU32 anGlyphIndex, tU32 anSize, const sVec2f& avOffset, const tF32 afScale) const;

  Ptr<tU32CMap> __stdcall EnumGlyphs() const;

 private:
  cGraphics*        mpGraphics;
  Ptr<sMyFTLibrary> mptrFTLib;
  FT_Face           mFace;
  tF32              mfNormalizedAscender, mfNormalizedDescender;
  tGlyphCacheHMap   mhmapGlyphCache;
  Ptr<iImageMap>    mptrImageMap;
  const tU32        mnImageMapPageSize;
  mutable tU32      mnLastKerningPixelSize;
  mutable Ptr<tU32CMap> mEnumedGlyphs;
};
} // end of namespace ni

#endif // __FONTTTF_H__
