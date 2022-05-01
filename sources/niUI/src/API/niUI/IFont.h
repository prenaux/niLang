#ifndef __IFONT_47888711_H__
#define __IFONT_47888711_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
// forward declarations
struct iCanvas;
struct iVGPath;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Font format flags.
enum eFontFormatFlags
{
  //! Text will be horizontally centered.
  eFontFormatFlags_CenterH    = niBit(0),
  //! Text will be vertically centered.
  eFontFormatFlags_CenterV    = niBit(1),
  //! Text will be right aligned.
  eFontFormatFlags_Right      = niBit(2),
  //! Text will be bottom aligned.
  eFontFormatFlags_Bottom     = niBit(3),
  //! Clip text horizontally.
  eFontFormatFlags_ClipH      = niBit(4),
  //! Clip text vertically.
  eFontFormatFlags_ClipV      = niBit(5),
  //! Draw a border around the text.
  eFontFormatFlags_Border     = niBit(6),
  //! Use kerning if available in the font.
  eFontFormatFlags_Kerning    = niBit(7),
  //! Dont snap to the unit grid after layout.
  //! \remark Snapping is needed for pixel perfect text rendering, since if
  //!         the text starts on a pixel fraction it will look blurry.
  //!         Snapping is the default but you will want to turn it off if
  //!         you're drawing animated text, text in a 3D space or you take
  //!         care of the snapping yourself.
  eFontFormatFlags_NoUnitSnap = niBit(8),
  //! \internal
  eFontFormatFlags_ForceDWORD = 0xFFFFFFFF
};

//! Font format flags type. \see ni::eFontFormatFlags
typedef tU32  tFontFormatFlags;

//! Get font flags.
enum eFontLoadFlags
{
  //! Search in the registered fonts.
  eFontLoadFlags_Registered = niBit(0),
  //! Search in the system fonts before searching in the registered fonts.
  eFontLoadFlags_SystemFirst = niBit(1),
  //! Search in the system fonts after searching in the registered fonts.
  eFontLoadFlags_SystemLast = niBit(2),
  //! Look for the closest match in the name.
  //! \remark If not specified will look for exact match only.
  //! \remark Match is always case insensitive.
  eFontLoadFlags_ClosestMatch = niBit(3),
  //! Get bold font.
  eFontLoadFlags_Bold = niBit(4),
  //! Get italic font.
  eFontLoadFlags_Italic = niBit(5),
  //! Default flags.
  eFontLoadFlags_Default = eFontLoadFlags_Registered|eFontLoadFlags_SystemLast|eFontLoadFlags_ClosestMatch,
  //! \internal
  eFontLoadFlags_ForceDWORD = 0xFFFFFFFF
};

//! Get font flags type.
typedef tU32 tFontLoadFlags;

//! Font interface.
struct iFont : public iUnknown
{
  niDeclareInterfaceUUID(iFont,0x4605c928,0x77ba,0x45df,0x95,0x05,0x18,0x46,0x77,0xe7,0x86,0x92)


  //########################################################################################
  //! \name Infos
  //########################################################################################
  //! @{

  //! Get the name of the font.
  //! {Property}
  virtual iHString* __stdcall GetName() const = 0;
  //! Get the family name of the font.
  //! {Property}
  virtual iHString* __stdcall GetFamilyName() const = 0;
  //! Get the style name of the font.
  //! {Property}
  virtual iHString* __stdcall GetStyleName() const = 0;
  //! Get the file path of the font.
  //! {Property}
  virtual iHString* __stdcall GetFilePath() const = 0;
  //! Return eTrue if the pixel size if fixed.
  //! \remark Fixed size fonts are generaly bitmap fonts.
  //! {Property}
  virtual tBool __stdcall GetIsFixedResolution() const = 0;
  //! Return eTrue if the font has kerning informations.
  //! {Property}
  virtual tBool __stdcall GetHasKerning() const = 0;
  //! Return the maximum pixel size.
  //! {Property}
  virtual tU32 __stdcall GetMaxResolution() const = 0;
  //! Return eTrue if this font is an instance.
  //! {Property}
  virtual tBool __stdcall GetIsInstance() const = 0;
  //! @}

  //########################################################################################
  //! \name Instance
  //########################################################################################
  //! @{

  //! Create an instance of this font.
  //! \param apMaterial is the material that the font will use instead of creating its own.
  virtual iFont* __stdcall CreateFontInstance(iMaterial* apMaterial) = 0;
  //! @}

  //########################################################################################
  //! \name Rendering parameters
  //########################################################################################
  //! @{

  //! Set the tabulation size.
  //! {Property}
  virtual void __stdcall SetTabSize(tU32 ulNumChar) = 0;
  //! Get the tabulation size.
  //! {Property}
  virtual tU32 __stdcall GetTabSize() const = 0;

  //! Set the filtering mode.
  //! {Property}
  virtual void __stdcall SetFiltering(tBool abLinear) = 0;
  //! Get the filtering mode.
  //! {Property}
  virtual tBool __stdcall GetFiltering() const = 0;

  //! Set the default font color.
  //! {Property}
  virtual void __stdcall SetColor(tU32 anColor) = 0;
  //! Get the default font color.
  //! {Property}
  virtual tU32 __stdcall GetColor() const = 0;

  //! Set the blending mode.
  //! {Property}
  virtual void __stdcall SetBlendMode(eBlendMode aMode) = 0;
  //! Get the blending mode.
  //! {Property}
  virtual eBlendMode __stdcall GetBlendMode() const = 0;

  //! Set the font size and resolution taking into account the contents scale.
  //! \remark If the avSize is (0,0) the size set is the resolution. Note that
  //!         the effective resolution set will be modified by the contents
  //!         scale, so GetResolution() will not always return anResolution.
  virtual void __stdcall SetSizeAndResolution(const sVec2f& avSize, const tU32 anResolution, const tF32 afContentsScale) = 0;
  //! Get the character's resolution.
  //! {Property}
  virtual tU32 __stdcall GetResolution() const  = 0;
  //! Get the character rendering size.
  //! {Property}
  virtual sVec2f __stdcall GetSize() const = 0;

  //! Set whether the glyphs are using a distance field.
  //! {Property}
  virtual void __stdcall SetDistanceField(tBool abDistanceField) = 0;
  //! Get whether the glyphs are using a distance field.
  //! {Property}
  virtual tBool __stdcall GetDistanceField() const = 0;
  //! @}

  //########################################################################################
  //! \name Metrics
  //########################################################################################
  //! @{

  //! Return the width of a character.
  //! \remark Passing zero will return the default width.
  //! {Property}
  virtual tF32 __stdcall GetCharWidth(tU32 c) const = 0;
  //! Return the height of a character.
  //! \remark Passing zero will return the default height.
  //! {Property}
  virtual tF32 __stdcall GetCharHeight(tU32 c) const = 0;

  //! Return the maximum char width.
  //! {Property}
  virtual tF32 __stdcall GetMaxCharWidth() const = 0;
  //! Return the maximum char height.
  //! {Property}
  virtual tF32 __stdcall GetMaxCharHeight() const = 0;

  //! Return the average char width.
  //! \remark Generally defined as the width of the letter x.
  //! {Property}
  virtual tF32 __stdcall GetAverageCharWidth() const = 0;
  //! Return the average char height.
  //! \remark Generally defined as the height of the letter x.
  //! {Property}
  virtual tF32 __stdcall GetAverageCharHeight() const = 0;

  //! Get the ascent (units above the base line) of characters.
  //! {Property}
  virtual tF32 __stdcall GetAscent() const = 0;
  //! Get the descent (units below the base line) of characters.
  //! {Property}
  virtual tF32 __stdcall GetDescent() const = 0;
  //! Get the height of the font (ascent + descent + linegap).
  //! {Property}
  virtual tF32 __stdcall GetHeight() const = 0;

  //! Get the value of the first character defined in the font.
  //! {Property}
  virtual tU32 __stdcall GetFirstChar() const = 0;
  //! Get the value of the last character defined in the font.
  //! {Property}
  virtual tU32 __stdcall GetLastChar() const = 0;

  //! Get the advance for the specified character.
  virtual tF32 __stdcall GetAdvance(tU32 anChar) const = 0;
  //! Get the kerning for the specified character.
  virtual tF32 __stdcall GetKerning(tU32 anPrevChar, tU32 anChar) const = 0;

  //! Set the normalized line spacing.
  //! {Property}
  //! \remark The
  virtual void __stdcall SetLineSpacing(const tF32 afSpacing) = 0;
  //! Get the normalized spacing a line.
  //! {Property}
  virtual tF32 __stdcall GetLineSpacing() const = 0;
  //! Get the a line's height.
  //! {Property}
  //! \remark GetHeight() * GetLineSpacing()
  virtual tF32 __stdcall GetLineHeight() const = 0;
  //! @}

  //########################################################################################
  //! \name Material and glyph cache
  //########################################################################################
  //! @{

  //! Update the font's material.
  //! \param abUpdateMaterialStates if true the states will set otherwise only the material's base channel texture is set to the current glyph cache's texture.
  //! \return eFalse on failure, else eTrue.
  virtual tBool __stdcall UpdateMaterial(tBool abUpdateMaterialStates) = 0;
  //! Cache the caracters contained in the passed string.
  virtual tBool __stdcall CacheText(const achar* aaszText) = 0;
  //! Cache a range of character.
  //! \remark First and Last will be included in the cache.
  virtual tBool __stdcall CacheRange(tU32 anFirst, tU32 anLast) = 0;
  //! Clear the glyph and texture caches.
  virtual tBool __stdcall ClearCache() = 0;
  //! Get the ID of the current cache.
  //! {Property}
  //! \remark The cache ID changes every time the cache is cleared.
  //! \remark The ID can be used to detect changes in the font
  //!         cache. For example to relayout geometries that depend
  //!         on the font's internal texture cache.
  virtual tIntPtr __stdcall GetCacheID() const = 0;
  //! Get the material used for the font's rendering.
  //! {Property}
  virtual iMaterial* __stdcall GetMaterial() const = 0;
  //! Get the texture coordinate of a character.
  //! {Property}
  //! \remark This has the side effect of caching the characters.
  virtual sRectf __stdcall GetCharTexCoo(tU32 c) const = 0;
  //! Get the texture in which the specified glyph is cached.
  //! {Property}
  virtual iTexture* __stdcall GetCharTexture(tU32 anChar) const = 0;
  //! Get the bitmaps in which the specified glyph is cached.
  //! {Property}
  virtual iBitmap2D* __stdcall GetCharBitmap(tU32 anChar) const = 0;
  //! @}

  //########################################################################################
  //! \name Bitmap drawing
  //########################################################################################
  //! @{

  //! Blit a character to the specified bitmap using the pixel size.
  virtual sVec2i __stdcall BlitChar(iBitmap2D* apBmp, tI32 anX, tI32 anY, tU32 anChar) = 0;
  //! Blit a character to the specified bitmap to the current font size.
  virtual sVec2i __stdcall BlitCharStretch(iBitmap2D* apBmp, tI32 anX, tI32 anY, tU32 anChar) = 0;
  //! Blit a character to the specified bitmap with the specified size and blend mode.
  virtual sVec2i __stdcall BlitCharEx(iBitmap2D* apBmp, tI32 anX, tI32 anY, tI32 anW, tI32 anH, tU32 anChar, eBlendMode aBlendMode) = 0;
  //! @}

  //########################################################################################
  //! \name Canvas drawing
  //########################################################################################
  //! @{

  //! Compute text size in pixels.
  //! \param aRect: If not Null the rectangle will be used as 'target' box where the text should be drawn, the compted text will take into account word wrapping, etc. in that box.
  //! \param aaszText is the text to compute the size of.
  //! \param aFormat: if 0 the format flags set is used, else the specified format is used.
  virtual sRectf __stdcall ComputeTextSize(const sRectf& aRect, const achar* aaszText, tFontFormatFlags aFormat) = 0;
  //! Draw text layed out in the specified rectangle.
  virtual sRectf __stdcall DrawText(
      ni::iCanvas* apCanvas, const sRectf& aposRect, tF32 afZ, const achar* aaszText, tFontFormatFlags aFormat) = 0;
  //! @}

  //########################################################################################
  //! \name Glyphs
  //########################################################################################
  //! @{

  //! Get the glyph index from its name.
  //! {Property}
  virtual tU32 __stdcall GetGlyphIndexFromName(const achar* aaszName) const = 0;
  //! Get the glyph index from its codepoint.
  //! {Property}
  virtual tU32 __stdcall GetGlyphIndexFromCodepoint(const tU32 anCodepoint) const = 0;
  //! Get the glyph name from its codepoint.
  //! {Property}
  virtual cString __stdcall GetGlyphName(const tU32 anGlyphIndex) const = 0;
  //! Enumerate all the glyphs with a codepoint in the font.
  //! \return Map<GlyphIndex:tU32,Codepoint:tU32>
  virtual Ptr<tU32CMap> __stdcall EnumGlyphs() const = 0;

  //! Get the glyph's codepoint from its name.
  //! {Property}
  virtual tU32 __stdcall GetGlyphCodePointFromName(const achar* aaszName) const = 0;

  //! Get a glyph's outline to the specified path.
  virtual sVec2f __stdcall GetGlyphPath(iVGPath* apPath, tU32 anGlyphIndex, const sVec2f& avOffset, const tF32 afScale) const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __IFONT_47888711_H__
