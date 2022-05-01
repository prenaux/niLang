#ifndef __IGRAPHICSCONTEXT_175BBCD3_F2EC_40C8_883A_8FFB806BF1C6_H__
#define __IGRAPHICSCONTEXT_175BBCD3_F2EC_40C8_883A_8FFB806BF1C6_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {

struct iGraphicsContext;
struct iGraphicsDriver;
struct iDrawOperation;
struct iDrawOperationSet;

/** \addtogroup niUI
 * @{
 */

//! Graphics context description interface.
struct iGraphicsContext : public iUnknown
{
  niDeclareInterfaceUUID(iGraphicsContext,0xf61a7db5,0x532f,0x4c23,0x90,0xf5,0xb5,0x86,0xf9,0x9c,0x78,0x53)

  //########################################################################################
  //! \name System
  //########################################################################################
  //! @{

  //! Get the context's graphics object.
  //! {Property}
  virtual iGraphics* __stdcall GetGraphics() const = 0;
  //! Get the context's driver.
  //! {Property}
  virtual iGraphicsDriver* __stdcall GetDriver() const = 0;
  //! Get the context width.
  //! {Property}
  virtual tU32 __stdcall GetWidth() const = 0;
  //! Get the context height.
  //! {Property}
  virtual tU32 __stdcall GetHeight() const = 0;
  //! @}

  //########################################################################################
  //! \name Rendering and main buffers
  //########################################################################################
  //! @{

  //! Clear the specified main buffers.
  //! \param  clearBuffer specifies the buffers to clear.
  //! \param  anColor is the color to use to clear the color buffer.
  //! \param  afDepth is the value to use to clear the depth buffer.
  //! \param  anStencil is the value to use to clear the stencil buffer.
  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) = 0;

  //! Get the render target set on the specified index.
  //! \remark Might be ignored by some contexts
  //! {Property}
  virtual iTexture* __stdcall GetRenderTarget(tU32 anIndex) const = 0;
  //! Get the depth stencil target.
  //! \remark Might be ignored by some contexts
  //! {Property}
  virtual iTexture* __stdcall GetDepthStencil() const = 0;

  //! Display the canvas in its destination window/context.
  //! \remark Should be called once rendering to the context is
  //!         finished, this will flush and unlock shared resources
  //!         for drivers that dont fully support multi-threaded
  //!         rendering. If you do not wish to show the content of
  //!         the context use eGraphicsDisplayFlags_Skip.
  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) = 0;

  //! Draw the specified draw operation.
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) = 0;

  //! Capture the front buffer.
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const = 0;
  //! @}

  //########################################################################################
  //! \name States
  //########################################################################################
  //! @{

  //! Set the viewport.
  //! {Property}
  virtual void __stdcall SetViewport(const sRecti& aVal) = 0;
  //! Get the viewport.
  //! {Property}
  virtual sRecti __stdcall GetViewport() const = 0;
  //! Set the scissor rectangle.
  //! {Property}
  virtual void __stdcall SetScissorRect(const sRecti& aVal) = 0;
  //! Get the scissor rectangle.
  //! {Property}
  virtual sRecti __stdcall GetScissorRect() const = 0;
  //! Set the default fixed pipeline states.
  //! {Property}
  virtual tBool __stdcall SetFixedStates(iFixedStates* apStates) = 0;
  //! Get the default fixed pipeline states.
  //! {Property}
  virtual iFixedStates* __stdcall GetFixedStates() const = 0;
  //! Set the default material states.
  //! {Property}
  virtual tBool __stdcall SetMaterial(iMaterial* apMat) = 0;
  //! Get the default material states.
  //! {Property}
  virtual const iMaterial* __stdcall GetMaterial() const = 0;
  //! @}
};

//! Render target graphics context interface.
struct iGraphicsContextRT : public iGraphicsContext
{
  niDeclareInterfaceUUID(iGraphicsContextRT,0x16b1a400,0x2499,0xc141,0xa6,0x51,0x1c,0x46,0xca,0xc3,0x13,0xc3);

  //! Change a render target.
  //! \remark the RT must use the same pixel format as the specified render target.
  virtual tBool __stdcall ChangeRenderTarget(tU32 anIndex, iTexture* apRT) = 0;
  //! Change the depth stencil.
  //! \remark the DS must use the same pixel format as the current depth stencil.
  virtual tBool __stdcall ChangeDepthStencil(iTexture* apDS) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IGRAPHICSCONTEXT_175BBCD3_F2EC_40C8_883A_8FFB806BF1C6_H__
