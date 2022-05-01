#pragma once
#ifndef __ITEXTOBJECT_H_8780B4C6_BE06_49E6_9257_223899B946F4__
#define __ITEXTOBJECT_H_8780B4C6_BE06_49E6_9257_223899B946F4__

namespace ni {

struct iTextOccluder;
struct iTextObject;

/** \addtogroup niUI
 * @{
 */

//! Text truncation
enum eTextTruncation
{
  //! The words will be truncated on the left if bigger than the text block.
  eTextTruncation_Left    = 0,
  //! The words will be truncated on the right if bigger than the text block.
  eTextTruncation_Right   = 1,
  //! The words won't be truncated.
  eTextTruncation_None    = 2,
  //! \internal
  eTextTruncation_ForceDWORD   = 0xFFFFFFFF
};

//! Text occluder interface
struct iTextOccluder : public iUnknown
{
  niDeclareInterfaceUUID(iTextOccluder,0x0a985a32,0x1472,0x41e9,0x85,0xc9,0xaf,0xee,0xd9,0x72,0xe1,0x4c);
  //! Get the parent text object.
  //! {Property}
  virtual Ptr<iTextObject> __stdcall GetTextObject() const = 0;
  //! Set the occluder's rectangle.
  //! {Property}
  virtual void __stdcall SetRect(const sRectf& aRect) = 0;
  //! Get the occluder's rectangle.
  //! {Property}
  virtual sRectf __stdcall GetRect() const = 0;
  //! Set user data.
  //! {Property}
  virtual void __stdcall SetUserData(const Var& aUserData) = 0;
  //! Get the user data.
  //! {Property}
  virtual Var __stdcall GetUserData() const = 0;
};

struct iTextObject : public iUnknown
{
  niDeclareInterfaceUUID(iTextObject,0x81b98e50,0x26ea,0x4a69,0xb1,0x3d,0xcb,0x0a,0xa5,0xab,0xea,0x11);

  //########################################################################################
  //! \name General
  //########################################################################################
  //! @{

  //! Get the graphics object that created the text object.
  //! {Property}
  virtual iGraphics* __stdcall GetGraphics() const = 0;

  //! Set the contents' scale factor.
  //! {Property}
  virtual void __stdcall SetContentsScale(const tF32 afContentsScale) = 0;
  //! Get the contents' scale factor.
  //! {Property}
  virtual tF32 __stdcall GetContentsScale() const = 0;

  //! Set the default font used by the text object.
  //! {Property}
  //! \remark If an invalid font is passed the default font of the graphics's instance is used.
  virtual void __stdcall SetDefaultFont(iFont* apFont) = 0;
  //! Get the default font used by the text object.
  //! {Property}
  virtual iFont* __stdcall GetDefaultFont() const = 0;

  //! Set the load font callback used by the text object.
  //! {Property}
  //! \remark The callback's signature: iFont* LoadFont(iHString* ahspFontName)
  //! \remark If none are provided the default font will be used everywhere.
  virtual void __stdcall SetLoadFontCallback(iCallback* apLoadFontCallback) = 0;
  //! Get the load font callback used by the text object.
  //! {Property}
  virtual iCallback* __stdcall GetLoadFontCallback() const = 0;

  //! Set the size of the text object.
  //! {Property}
  virtual void __stdcall SetSize(const sVec2f& avSize) = 0;
  //! Get the size of the text object.
  //! {Property}
  virtual sVec2f __stdcall GetSize() const = 0;

  //! Set the word truncation mode.
  //! {Property}
  virtual void __stdcall SetTruncation(eTextTruncation aType) = 0;
  //! Get the word truncation mode.
  //! {Property}
  virtual eTextTruncation __stdcall GetTruncation() const = 0;
  //! Set the truncation text.
  //! {Property}
  virtual void __stdcall SetTruncationText(const achar* aaszString) = 0;
  //! Get the truncation text.
  //! {Property}
  virtual const achar* __stdcall GetTruncationText() const = 0;

  //! Set whether to trim the leading spaces.
  //! {Property}
  virtual void __stdcall SetTrimLeadingSpaces(tBool abTrimLeadingSpaces) = 0;
  //! Get whether to trim the leading spaces.
  //! {Property}
  virtual tBool __stdcall GetTrimLeadingSpaces() const = 0;

  //! Set whether to use kerning.
  //! {Property}
  virtual void __stdcall SetKerning(tBool abKerning) = 0;
  //! Get whether to use kerning.
  //! {Property}
  virtual tBool __stdcall GetKerning() const = 0;

  //! Get the computed size of the text after layout.
  //! {Property}
  virtual sVec2f __stdcall GetTextSize() const = 0;

  //! Set the expression context used for the <expr> tags.
  //! {Property}
  virtual void __stdcall SetExpressionContext(iExpressionContext* apContext) = 0;
  //! Get the expression context used for the <expr> tags.
  //! {Property}
  virtual iExpressionContext* __stdcall GetExpressionContext() const = 0;

  //! Update.
  virtual void __stdcall Update() = 0;
  //! Draw the text object in the specified canvas.
  virtual tBool __stdcall DrawAt(iCanvas* apCanvas, const sRectf& aClippingRect, const sVec3f& avPos) = 0;
  //! Draw the text object in the specified canvas.
  virtual tBool __stdcall Draw(iCanvas* apCanvas, const sRectf& aClippingRect) = 0;
  //! @}

  //########################################################################################
  //! \name Text occluders
  //########################################################################################
  //! @{

  //! Add a text occluder.
  virtual iTextOccluder* __stdcall AddOccluder(const sRectf& aRect, const Var& aUserData) = 0;
  //! Remove the text occluder at the specified index.
  virtual tBool __stdcall RemoveOccluder(tU32 anIndex) = 0;
  //! Removes all text occluders.
  virtual void __stdcall ClearOccluders() = 0;
  //! Get number of occluders.
  //! {Property}
  virtual tU32 __stdcall GetNumOccluders() const = 0;
  //! Get occluder from index.
  //! {Property}
  virtual iTextOccluder* __stdcall GetOccluder(tU32 anIndex) const = 0;
  //! @}

  //########################################################################################
  //! \name Text manipulation
  //########################################################################################
  //! @{

  //! Set the text of the text object.
  //! {Property}
  virtual void __stdcall SetText(const achar* aaszText) = 0;
  //! Get the text of the text object.
  //! {Property}
  virtual const achar* __stdcall GetText() const = 0;

  //! Add Text the the widget.
  //! \remark Will push back and layout the desired string without
  //!         processing the already-processed parts uselessly.
  virtual void __stdcall AddText(const achar* aaszString) = 0;

  //! Set the selection background color.
  //! {Property}
  virtual void __stdcall SetSelectionColor(sVec4f avColor) = 0;
  //! Get the selection background color.
  //! {Property}
  virtual sVec4f __stdcall GetSelectionColor() const = 0;

  //! Get the selected string.
  //! {Property}
  virtual cString __stdcall GetSelectedString() const = 0;

  //! Clear the current selection.
  virtual void __stdcall ClearSelection() = 0;
  //! Select the specified word range.
  virtual void __stdcall SelectRange(tU32 anBegin, tU32 anEnd) = 0;

  //! Find the index of the word nearest of the specified position.
  //! \return eInvalidHandle if no words can be found.
  virtual tU32 __stdcall FindWordIndexFromPosition(sVec2f avPosition) const = 0;
  //! @}
};

/**@}*/
}
#endif // __ITEXTOBJECT_H_8780B4C6_BE06_49E6_9257_223899B946F4__
