#pragma once
#ifndef __ICANVAS_H_C64A5B5C_B71A_403F_A371_E2BDD3F1D6F4__
#define __ICANVAS_H_C64A5B5C_B71A_403F_A371_E2BDD3F1D6F4__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "IImage.h"
#include "IVGStyle.h"
#include "IVGTransform.h"
#include "IVGPath.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Billboard mode flags.
enum eBillboardModeFlags
{
  //! Disabled billboard mode.
  eBillboardModeFlags_Disabled = 0,
  //! Custom center specified.
  eBillboardModeFlags_CustomCenter = niBit(0),
  //! Anchor at the center (default if anchor not specified).
  eBillboardModeFlags_AnchorCenter = niBit(1),
  //! \internal
  eBillboardModeFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Billboard mode flags. \see ni::eBillboardModeFlags
typedef tU32 tBillboardModeFlags;

//! Canvas interface.
struct iCanvas : public iUnknown
{
  niDeclareInterfaceUUID(iCanvas,0x0f47c834,0x5618,0x42b0,0xb1,0x69,0xbe,0x54,0x9a,0x29,0x87,0xbd)

  //! Get the parent graphics context.
  //! {Property}
  virtual iGraphicsContext* __stdcall GetGraphicsContext() const = 0;

  //! Set the canvas' contents scale.
  //! {Property}
  //! \remark The canvas in the UI rendering system are setup to use the UIContext's contents scale.
  virtual void __stdcall SetContentsScale(tF32 afContentsScale) = 0;
  //! Get the canva's contents scale.
  //! {Property}
  virtual tF32 __stdcall GetContentsScale() const = 0;
  //! Get the viewport size in UI units.
  //! {Property}
  virtual sRectf __stdcall GetViewport() const = 0;

  //! Flush the rendering to the target immediatly.
  virtual tBool __stdcall Flush() = 0;

  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitFill(const sRectf& aDestRect, tU32 aColor) = 0;
  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitFillAlpha(const sRectf& aDestRect, tU32 aColor) = 0;

  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitStretch(const sRectf& aDestRect, iTexture* apSrc, const sRectf& aSrcRect) = 0;
  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitStretchAlpha(const sRectf& aDestRect, iTexture* apSrc, const sRectf& aSrcRect) = 0;
  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitStretchAlpha1(const sRectf& aDestRect, iTexture* apSrc, const sRectf& aSrcRect, tF32 afAlpha) = 0;

  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitOverlay(const sRectf& aDestRect, iOverlay* apOverlay) = 0;
  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitOverlayFrame(const sRectf& aDestRect, iOverlay* apOverlay,  tRectFrameFlags aFrame) = 0;

  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitLine(const sVec2f& aStart, const sVec2f& aEnd, tU32 aColor) = 0;
  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitLineAlpha(const sVec2f& aStart, const sVec2f& aEnd, tU32 aColor) = 0;
  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitRect(const sRectf& aDestRect, tU32 aColor) = 0;
  //! \remark Destination is relative to the current viewport.
  virtual tBool __stdcall BlitRectAlpha(const sRectf& aDestRect, tU32 aColor) = 0;
  //! Blit text with the specified font.
  //! \remark Destination is relative to the current viewport.
  virtual sRectf __stdcall BlitText(iFont* apFont, const sRectf& aRect, tFontFormatFlags aFormatFlags, const achar* aaszText) = 0;

  //! Reset the canvas states to the default states.
  virtual void __stdcall ResetStates() = 0;

  //! Check whether any vertex has been pushed in the rendering buffer.
  //! {Property}
  virtual tBool __stdcall GetHasVertices() const = 0;

  //! Set the default color A of the canvas. (default is 0xFFFFFFFF - white)
  //! {Property}
  virtual void __stdcall SetColorA(tU32 anColor) = 0;
  //! Get the color A.
  //! {Property}
  virtual tU32 __stdcall GetColorA() const = 0;

  //! Set the default normal of the canvas. (default sVec3f::YAxis())
  //! {Property}
  virtual void __stdcall SetNormal(const sVec3f& avNormal) = 0;
  //! Get the default normal.
  //! {Property}
  virtual sVec3f __stdcall GetNormal() const = 0;

  //! Set whether line have a constant size in screen space.
  //! {Property}
  virtual void __stdcall SetLineConstantScreenSize(tBool abConstSize) = 0;
  //! Get whether line have a constant size in screen space
  //! {Property}
  virtual tBool __stdcall GetLineConstantScreenSize() const = 0;

  //! Set the line size.
  //! \remark Begin reset the default size to 1.
  //! {Property}
  virtual void __stdcall SetLineSize(tF32 afSize) = 0;
  //! Get the line size.
  //! {Property}
  virtual tF32 __stdcall GetLineSize() const = 0;

  //! Set the drawop's material.
  //! {Property}
  //! \remark Sets the default blit material when apMaterial is NULL.
  virtual void __stdcall SetMaterial(iMaterial* apMaterial) = 0;
  //! Get the drawop's material.
  //! {Property}
  virtual iMaterial* __stdcall GetMaterial() const = 0;
  //! Sets the default material as current material and sets its texture, blendmode and sampler states.
  virtual void __stdcall SetDefaultMaterial(iTexture* apTexture, eBlendMode aBlendMode, tIntPtr aSamplerStates) = 0;

  //! Set the current matrix.
  //! {Property}
  virtual void __stdcall SetMatrix(const sMatrixf& aMatrix) = 0;
  //! Get the current matrix.
  //! {Property}
  virtual sMatrixf __stdcall GetMatrix() const = 0;

  //! Set billboard mode.
  //! {Property}
  //! \remark Billboard mode influence the next rectangle/frame pushed.
  //! \remark If the billboard right and up vectors haven't been set they will be derived from the current the view matrix.
  virtual void __stdcall SetBillboard(tBillboardModeFlags aMode) = 0;
  //! Get billboard mode.
  //! {Property}
  virtual tBillboardModeFlags __stdcall GetBillboard() const = 0;
  //! Set the billboard right vector.
  //! \remark If not set explicitly, the first SetBillboard(true) after Begin sets it to the current camera view matrix's right vector.
  //! {Property}
  virtual void __stdcall SetBillboardRight(const sVec3f& avRight) = 0;
  //! Get the billboard right vector.
  //! {Property}
  virtual sVec3f __stdcall GetBillboardRight() const = 0;
  //! Set the billboard up vector.
  //! \remark If not set explicitly, the first SetBillboard(true) after Begin sets it to the current camera view matrix's up vector.
  //! {Property}
  virtual void __stdcall SetBillboardUp(const sVec3f& avUp) = 0;
  //! Get the billboard up vector.
  //! {Property}
  virtual sVec3f __stdcall GetBillboardUp() const = 0;
  //! Set the custom billboard center.
  //! {Property}
  virtual void __stdcall SetBillboardCenter(const sVec3f& avCenter) = 0;
  //! Get the custom billboard center.
  //! {Property}
  virtual sVec3f __stdcall GetBillboardCenter() const = 0;

  //! Push an immediate mode rectangle.
  virtual tBool __stdcall Rect(const sVec2f& avTL, const sVec2f& avBR, tF32 afZ) = 0;
  //! Push an immediate mode rectangle, specifying the texture coordinates.
  virtual tBool __stdcall RectT(const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ) = 0;
  //! Push an immediate mode rectangle with color A.
  virtual tBool __stdcall RectA(const sVec2f& avTL, const sVec2f& avBR, tF32 afZ, tU32 anColA) = 0;
  //! Push an immediate mode rectangle, specifying the texture coordinates and color A.
  virtual tBool __stdcall RectTA(const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ, tU32 anColA) = 0;
  //! Push an immediate mode rectangle, specifying the texture coordinates, color A and Color B.
  virtual tBool __stdcall RectTA2(const sVec2f& avTL, const sVec2f& avBR,
                                  const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ,
                                  tU32 anTLColA, tU32 anTRColA, tU32 anBRColA, tU32 anBLColA) = 0;

  //! Push an immediate mode quad with 3d positions, normals, texture coordinates, color A and color B.
  virtual tBool __stdcall QuadEx(
      const sVec3f& avTL, const sVec3f& avTLN, const sVec2f& avTLTex, tU32 anTLA,
      const sVec3f& avTR, const sVec3f& avTRN, const sVec2f& avTRTex, tU32 anTRA,
      const sVec3f& avBR, const sVec3f& avBRN, const sVec2f& avBRTex, tU32 anBRA,
      const sVec3f& avBL, const sVec3f& avBLN, const sVec2f& avBLTex, tU32 anBLA) = 0;
  //! Push an immediate mode quad with 3d positions, texture coordinates, color A and color B.
  virtual tBool __stdcall Quad(const sVec3f& avTL, const sVec3f& avTR, const sVec3f& avBR, const sVec3f& avBL) = 0;
  //! Push an immediate mode quad with 3d positions, texture coordinates, color A and color B.
  virtual tBool __stdcall QuadA(const sVec3f& avTL, const sVec3f& avTR, const sVec3f& avBR, const sVec3f& avBL, tU32 anColorA) = 0;
  //! Push an immediate mode quad with 3d positions, texture coordinates, color A and color B.
  virtual tBool __stdcall QuadA2(
      const sVec3f& avTL, tU32 anTLColA,
      const sVec3f& avTR, tU32 anTRColA,
      const sVec3f& avBR, tU32 anBRColA,
      const sVec3f& avBL, tU32 anBLColA) = 0;
  //! Push an immediate mode quad with 3d positions, texture coordinates, color A and color B.
  virtual tBool __stdcall QuadT(
      const sVec3f& avTL, const sVec2f& avTLTex,
      const sVec3f& avTR, const sVec2f& avTRTex,
      const sVec3f& avBR, const sVec2f& avBRTex,
      const sVec3f& avBL, const sVec2f& avBLTex) = 0;
  //! Push an immediate mode quad with 3d positions, texture coordinates, color A and color B.
  virtual tBool __stdcall QuadTA(
      const sVec3f& avTL, const sVec2f& avTLTex,
      const sVec3f& avTR, const sVec2f& avTRTex,
      const sVec3f& avBR, const sVec2f& avBRTex,
      const sVec3f& avBL, const sVec2f& avBLTex,
      tU32 anColorA) = 0;
  //! Push an immediate mode quad with 3d positions, texture coordinates, color A and color B.
  virtual tBool __stdcall QuadTA2(
      const sVec3f& avTL, const sVec2f& avTLTex, tU32 anTLColA,
      const sVec3f& avTR, const sVec2f& avTRTex, tU32 anTRColA,
      const sVec3f& avBR, const sVec2f& avBRTex, tU32 anBRColA,
      const sVec3f& avBL, const sVec2f& avBLTex, tU32 anBLColA) = 0;

  //! Push an immediate mode rectangle.
  virtual tBool __stdcall Frame(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, tF32 afZ) = 0;
  //! Push an immediate mode rectangle with the specified color.
  virtual tBool __stdcall FrameA(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, tF32 afZ, tU32 anColA) = 0;
  //! Push an immediate mode rectangle, specifying the texture coordinates.
  virtual tBool __stdcall FrameT(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ) = 0;
  //! Push an immediate mode rectangle, specifying the texture coordinates and color a.
  virtual tBool __stdcall FrameTA(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ, tU32 anColA) = 0;
  //! Push an immediate mode rectangle, specifying the texture coordinates, color A and Color B.
  virtual tBool __stdcall FrameTA2(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ, tU32 anTLColA, tU32 anTRColA, tU32 anBRColA, tU32 anBLColA) = 0;

  //! Push a line.
  virtual tBool __stdcall Line(const sVec3f& avStart, const sVec3f& avEnd) = 0;
  //! Push a line.
  virtual tBool __stdcall LineA(const sVec3f& avStart, const sVec3f& avEnd, tU32 anCol) = 0;
  //! Push a line.
  virtual tBool __stdcall LineEx(const sVec3f& avStart, const sVec2f& avStartTex,
                                 tF32 afStartSize, tU32 anStartCol,
                                 const sVec3f& avEnd, const sVec2f& avEndTex,
                                 tF32 afEndSize, tU32 anEndCol) = 0;


  //! Push a XY grid made of lines.
  //! \param aX Size in X of one cell of the grid.
  //! \param aY Size in Y of one cell of the grid.
  //! \param anNumColumns
  //! \param anNumRows
  virtual void __stdcall LineGridXY(ni::tF32 aX, ni::tF32 aY, ni::tU32 anNumColumns, ni::tU32 anNumRows) = 0;
  //! Push a XZ grid made of lines.
  //! \param aX Size in X of one cell of the grid.
  //! \param aZ Size in Z of one cell of the grid.
  //! \param anNumColumns
  //! \param anNumRows
  virtual void __stdcall LineGridXZ(ni::tF32 aX, ni::tF32 aZ, ni::tU32 anNumColumns, ni::tU32 anNumRows) = 0;

  //! Draw a circle.
  //! \remark Uses the current line parameters.
  virtual tBool __stdcall WireframeCircle(const sVec3f& avCenter, tF32 afRadius, tU32 aulNumDiv = 32, ePlaneType aPlane = ePlaneType_XY) = 0;
  //! Draw an ellipse.
  //! \remark Uses the current line parameters.
  virtual tBool __stdcall WireframeEllipse(const sVec3f& avCenter, const sVec2f& avRadius, tU32 aulNumDiv = 32, ePlaneType aPlane = ePlaneType_XY) = 0;
  //! Draw wireframe AABB.
  //! \remark Uses the current line parameters.
  virtual tBool __stdcall WireframeAABB(const sVec3f& avMin, const sVec3f& avMax) = 0;
  //! Draw wireframe sphere.
  //! \remark Uses the current line parameters.
  virtual tBool __stdcall WireframeSphere(const sVec3f& aCenter, tF32 afRadius, tU32 aulNumDiv = 32) = 0;
  //! Draw wireframe sphere.
  //! \remark Uses the current line parameters.
  virtual tBool __stdcall WireframeSphereEx(const sVec3f& aCenter, tF32 afRadius, tU32 aulNumDiv,
                                            tU32 aulColorX, tU32 aulColorY, tU32 aulColorZ) = 0;
  //! Draw wireframe ellipsoid.
  //! \remark Uses the current line parameters.
  virtual tBool __stdcall WireframeEllipsoid(const sVec3f& aCenter, const sVec3f& avRadius, tU32 aulNumDiv = 32) = 0;
  //! Draw wireframe ellipsoid.
  //! \remark Uses the current line parameters.
  virtual tBool __stdcall WireframeEllipsoidEx(const sVec3f& aCenter, const sVec3f& avRadius, tU32 aulNumDiv,
                                               tU32 aulColorX, tU32 aulColorY, tU32 aulColorZ) = 0;
  //! Draw a wireframe cone.
  //! \remark Uses the current line parameters.
  virtual tBool __stdcall WireframeCone(const sVec3f& avBaseCenter, tF32 afBaseRadius, tF32 afHeight, tU32 aulNumDiv = 32, ePlaneType aPlane = ePlaneType_XY) = 0;

  //! Push a vertex with position.
  virtual tBool __stdcall VertexP(const sVec3f& avPosition) = 0;
  //! Push a vertex with position and normal.
  virtual tBool __stdcall VertexPN(const sVec3f& avPosition, const sVec3f& avNormal) = 0;
  //! Push a vertex with position and a 2d texture coodinate.
  virtual tBool __stdcall VertexPT(const sVec3f& avPosition, const sVec2f& avTex) = 0;
  //! Push a vertex with position, a 2d texture coodinate and color A.
  virtual tBool __stdcall VertexPTA(const sVec3f& avPosition, const sVec2f& avTex, tU32 anColorA) = 0;
  //! Push a vertex with position, normal and texture coodinate.
  virtual tBool __stdcall VertexPNT(const sVec3f& avPosition, const sVec3f& avNormal, const sVec2f& avTex) = 0;
  //! Push a vertex with position, normal, texture coodinate and color A.
  virtual tBool __stdcall VertexPNTA(const sVec3f& avPosition, const sVec3f& avNormal, const sVec2f& avTex, tU32 anColorA) = 0;

  //! Get the VG path used for drawing.
  //! {Property}
  virtual iMaterial* __stdcall GetVGMaterial() const = 0;
  //! Get the VG style used for drawing.
  //! {Property}
  virtual iVGStyle* __stdcall GetVGStyle() const = 0;
  //! Get the VG transform used for drawing.
  //! {Property}
  virtual iVGTransform* __stdcall GetVGTransform(eVGTransform aTransform) const = 0;
  //! Draw a VG path.
  virtual void __stdcall DrawPath(const iVGPath* apPath) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ICANVAS_H_C64A5B5C_B71A_403F_A371_E2BDD3F1D6F4__
