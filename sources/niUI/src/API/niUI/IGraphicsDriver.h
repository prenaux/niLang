#ifndef __IGRAPHICSDRIVER_6010323_H__
#define __IGRAPHICSDRIVER_6010323_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/IOSWindow.h>
#include "ITexture.h"
#include "IShader.h"
#include "IMaterial.h"

namespace ni {

struct iGraphicsContext;
struct iGraphicsContextRT;
struct iGraphicsDriver;
struct iDrawOperation;
struct iDrawOperationSet;
struct iBitmapFormat;
struct iOcclusionQuery;
struct iVertexArray;
struct iIndexArray;
struct iSamplerStates;
struct iRasterizerStates;
struct iDepthStencilStates;
struct iFixedStates;

/** \addtogroup niUI
 * @{
 */

//! Graphics capture flag.
enum eGraphicsCaptureFlags
{
  //! Clone all draw ops up until stop at.
  eGraphicsCaptureFlags_All = niBit(0),
  //! Cloning of the geometry of the stop at draw op.
  eGraphicsCaptureFlags_CloneStopAtGeometry = niBit(1),
  //! Clone the textures at the specified index.
  eGraphicsCaptureFlags_CloneStopAtTextures = niBit(2),
  //! Clone the states at the specified index.
  eGraphicsCaptureFlags_CloneStopAtStates = niBit(3),
  //! Replace the states of the stop at draw op by the states cloned during
  //! the first capture of that draw op. This allows the states to be
  //! modified dynamically.
  eGraphicsCaptureFlags_KeepStopAt = niBit(4),
  //! Do not count time perf when capturing the draw ops.
  eGraphicsCaptureFlags_NoTime = niBit(5),
  //! Breakpoint on StopAt begin drawop.
  eGraphicsCaptureFlags_BreakOnStopAtBegin = niBit(6),
  //! Breakpoint on StopAt end drawop.
  eGraphicsCaptureFlags_BreakOnStopAtEnd = niBit(7),
  //! \internal
  eGraphicsCaptureFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Graphics capture flag type. \see ni::eGraphicsCapture
typedef tU32 tGraphicsCaptureFlags;

//! Draw operation capture object.
struct iGraphicsDrawOpCapture : public iUnknown
{
  niDeclareInterfaceUUID(iGraphicsDrawOpCapture,0xb9df0bbe,0x865d,0x464a,0x9e,0xfe,0xeb,0xe0,0x88,0x77,0x97,0xe3);

  //! Begin capturing.
  //! \remark DOES NOT clear the previous capture data.
  virtual tBool __stdcall BeginCapture() = 0;
  //! End capturing.
  //! \return The number of draw operations captured.
  virtual tU32 __stdcall EndCapture() = 0;
  //! Return whether we the object is currently capturing.
  //! {Property}
  virtual tBool __stdcall GetIsCapturing() const = 0;
  //! Clear all captured data.
  virtual void __stdcall ClearCapture() = 0;
  //! Set the capture flags.
  //! {Property}
  virtual void __stdcall SetCaptureFlags(tGraphicsCaptureFlags aFlags) = 0;
  //! Get the capture flags.
  //! {Property}
  virtual tGraphicsCaptureFlags __stdcall GetCaptureFlags() const = 0;
  //! Set the current stop at index.
  //! {Property}
  virtual void __stdcall SetCaptureStopAt(tU32 anStopAt) = 0;
  //! Get the current stop at index.
  //! {Property}
  virtual tU32 __stdcall GetCaptureStopAt() const = 0;
  //! Get the number of draw ops captured.
  //! {Property}
  virtual tU32 __stdcall GetNumCaptured() const = 0;
  //! Get the captured clear operation parameters.
  //! {Property}
  //! \remark The format is (ClearFlags,Color,ftoul(Depth),Stencil)
  virtual sVec4i __stdcall GetCapturedClear(tU32 anIndex) const = 0;
  //! Get the draw operation at the specified index.
  //! {Property}
  virtual iDrawOperation* __stdcall GetCapturedDrawOp(tU32 anIndex) const = 0;
  //! Get the time taken to submit the specified draw operation
  //! {Property}
  virtual tF32 __stdcall GetCapturedDrawOpTime(tU32 anIndex) const = 0;
  //! Get the context that submited the draw operation.
  //! {Property}
  virtual iGraphicsContext* __stdcall GetCapturedDrawOpContext(tU32 anIndex) const = 0;
  //! Called to capture a draw operation.
  //! \return eFalse if the draw operation should not be submitted.
  virtual tBool __stdcall BeginCaptureDrawOp(iGraphicsContext* apContext, iDrawOperation* apDrawOp, const sVec4i& aClearParams) = 0;
  //! Called to finish the draw operation capture.
  virtual void __stdcall EndCaptureDrawOp(iGraphicsContext* apContext, iDrawOperation* apDrawOp, const sVec4i& aClearParams) = 0;
};

//! Graphics driver.
//! \remark This interface should never be used directly, iGraphics is the interface that should be used.
struct iGraphicsDriver : public iUnknown {
  niDeclareInterfaceUUID(iGraphicsDriver,0x18477542,0x75b2,0x4c16,0x9e,0xe0,0xb2,0x31,0x61,0xcb,0x3e,0xb2)

  //########################################################################################
  //! \name System
  //########################################################################################
  //! @{

  //! Get the parent iGraphics interface.
  //! {Property}
  virtual iGraphics* __stdcall GetGraphics() const = 0;
  //! Get the driver's name.
  //! {Property}
  virtual const achar* __stdcall GetName() const = 0;
  //! Get the driver's description.
  //! {Property}
  virtual const achar* __stdcall GetDesc() const = 0;
  //! Get the driver device's name.
  //! {Property}
  virtual const achar* __stdcall GetDeviceName() const = 0;
  //! Get the driver's capabilities.
  //! {Property}
  virtual tInt __stdcall GetCaps(eGraphicsCaps aCaps) const = 0;
  //! Get the driver implementation details.
  //! {Property}
  virtual tGraphicsDriverImplFlags __stdcall GetGraphicsDriverImplFlags() const = 0;
  //! Set the draw op capture object.
  //! {Property}
  virtual void __stdcall SetDrawOpCapture(iGraphicsDrawOpCapture* apCapture) = 0;
  //! Get the draw op capture object.
  //! {Property}
  virtual iGraphicsDrawOpCapture* __stdcall GetDrawOpCapture() const = 0;
  //! @}

  //########################################################################################
  //! \name Context
  //########################################################################################
  //! @{

  //! Create a new context for the specified OS window.
  virtual iGraphicsContext* __stdcall CreateContextForWindow(
    iOSWindow* apWindow,
    const achar* aaszBBFormat, const achar* aaszDSFormat,
    tU32 anSwapInterval, tTextureFlags aBackBufferFlags) = 0;
  //! Creates a new context for the specified render targets.
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(
    iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS) = 0;
  //! Reset all graphics driver caches.
  //! \remark This is to be called after using any Native API used by the
  //!         renderer. If not called the cached states, textures, shaders,
  //!         etc... wont be properly syncronised, resulting in unpredicatable
  //!         behaviours.
  virtual tBool __stdcall ResetAllCaches() = 0;
  //! @}

  //########################################################################################
  //! \name Textures
  //########################################################################################
  //! @{

  //! Check whether the specified texture format is supported and
  //! set the bitmap format object to the nearest matching native bitmap format.
  virtual tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags) = 0;
  //! Create a new texture.
  //! \remark Setting anNumMipMaps to zero with eTextureFlags_MipMaps automatically computes the number of mipmaps down to 4x4.
  virtual iTexture* __stdcall CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags) = 0;
  //! Blit a bitmap to a texture.
  virtual tBool __stdcall BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) = 0;
  //! Blit a texture to a bitmap.
  virtual tBool __stdcall BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) = 0;
  //! Blit a texture into another texture.
  virtual tBool __stdcall BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) = 0;
  //! Blit a 3d bitmap to a 3d texture.
  virtual tBool __stdcall BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags) = 0;
  //! Blit a 3d texture to a 3d bitmap.
  virtual tBool __stdcall BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags) = 0;

  //! @}

  //########################################################################################
  //! \name Shaders
  //########################################################################################
  //! @{

  //! Get the number of Shader profile supported in the specified unit.
  virtual tU32 __stdcall GetNumShaderProfile(eShaderUnit aUnit) const = 0;
  //! Get the profile at the specified index.
  virtual iHString* __stdcall GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const = 0;
  //! Load a compiled shader from the specified bytecode.
  virtual iShader* __stdcall CreateShader(iHString* ahspName, iFile* apByteCode) = 0;
  //! @}

  //########################################################################################
  //! \name Occlusion queries.
  //########################################################################################
  //! @{

  //! Create a new occlusion query object.
  virtual iOcclusionQuery* __stdcall CreateOcclusionQuery() = 0;
  //! @}

  //########################################################################################
  //! \name Optionally driver specific objects.
  //########################################################################################
  //! @{

  //! Create a new driver vertex array instance.
  virtual iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) = 0;
  //! Create a new driver index array instance.
  virtual iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex, eArrayUsage aUsage) = 0;
  //! Creates a new empty driver shader constants instance.
  virtual iShaderConstants* __stdcall CreateShaderConstants(tU32 anMaxRegisters) const = 0;
  //! Compile a sampler states.
  virtual tIntPtr __stdcall CompileSamplerStates(iSamplerStates* apStates) = 0;
  //! Compile a rasterizer states.
  virtual tIntPtr __stdcall CompileRasterizerStates(iRasterizerStates* apStates) = 0;
  //! Compile a depth stencil states.
  virtual tIntPtr __stdcall CompileDepthStencilStates(iDepthStencilStates* apStates) = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IGRAPHICSDRIVER_6010323_H__
