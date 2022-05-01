#ifndef __IGRAPHICS_31779012_H__
#define __IGRAPHICS_31779012_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Colors.h"
#include "IDampedSpring.h"
#include "INUSpline.h"
#include "FVF.h"
#include "IBitmapBase.h"
#include "IBitmap2D.h"
#include "IBitmapCube.h"
#include "IBitmap3D.h"
#include "IGeometry.h"
#include "IGraphicsDriver.h"
#include "IOverlay.h"
#include "IFont.h"
#include "ICanvas.h"
#include "ITextObject.h"
#include "IVideoDecoder.h"

#include "IVGStyle.h"
#include "IVGTransform.h"
#include "IVGImage.h"
#include "IVGGradientTable.h"
#include "IVGPaint.h"
#include "IVGPath.h"
#include "IVGPolygonTesselator.h"

namespace ni {

struct iBitmapFormat;
struct iCamera;
struct iImage;
struct iCanvas;

struct iGeometryModifier;
struct iMaterialLibrary;

struct iTransform;
struct iFrustum;
struct iBoundingVolume;
struct iIntersection;

struct iImageMap;

/** \addtogroup niUI
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Graphics interface
struct iGraphics : public iUnknown
{
  niDeclareInterfaceUUID(iGraphics,0x92e5efcf,0xfcb3,0x4764,0x98,0x2b,0xd2,0x0c,0x83,0x4e,0x61,0xed)

  //########################################################################################
  //! \name FVF
  //########################################################################################
  //! @{

  //! Get the number of dimensions of the specified texture coordinates. anTexCooIndex is the index (0 to 7).
  virtual tU32 __stdcall FVFGetTexCooDim(tFVF anFVF, tU32 anTexCooIndex) const = 0;
  //! Get the number of texture coordinates of the specified FVF.
  virtual tU32 __stdcall FVFGetNumTexCoos(tFVF anFVF) const = 0;
  //! Convert the specified FVF to a string.
  virtual cString __stdcall FVFToString(tFVF aFVF) const = 0;
  //! Get a FVF from the specified string.
  virtual tFVF __stdcall FVFFromString(const achar* aaszString) const = 0;
  //! Get the offset of the specified component.
  virtual tU32 __stdcall FVFGetComponentOffset(tFVF aFVF, eFVF C) const = 0;
  //! Get the size of a FVF.
  virtual tU32 __stdcall FVFGetStride(tFVF aFVF) const = 0;
  //! @}

  //########################################################################################
  //! \name Pixel Format
  //########################################################################################
  //! @{

  //! Create a pixel format.
  //! \param  aszFormat is a achar string which describe the pixel format to create.
  //! \return an interface to handle the created pixel format.
  //! \remark Pixel format are created throught plugins, so the supported formats
  //!     depend of the loaded plugins.
  virtual iPixelFormat* __stdcall CreatePixelFormat(const achar* aszFormat) = 0;
  //! @}

  //########################################################################################
  //! \name Bitmaps
  //########################################################################################
  //! @{

  //! Create a new bitmap format.
  virtual iBitmapFormat* __stdcall CreateBitmapFormat(eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth) = 0;
  //! Create a new bitmap  format.
  virtual iBitmapFormat* __stdcall CreateBitmapFormatEx(eBitmapType aType, iPixelFormat* pFmt, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth) = 0;
  //! Create a new empty bitmap format.
  virtual iBitmapFormat* __stdcall CreateBitmapFormatEmpty() = 0;

  //! Create a new bitmap.
  virtual iBitmapBase* __stdcall CreateBitmap(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth) = 0;
  //! Create a new bitmap.
  virtual iBitmapBase* __stdcall CreateBitmapEx(iHString* ahspName, eBitmapType aType, iPixelFormat* pFmt, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth) = 0;

  //! Create a 2D bitmap.
  //! \remark the given iPixelFormat instance will be release if the creation failed and when
  //!     the bitmap is released.
  virtual iBitmap2D* __stdcall CreateBitmap2DEx(tU32 nW, tU32 nH, iPixelFormat* pFmt) = 0;
  //! Create a 2D bitmap.
  virtual iBitmap2D* __stdcall CreateBitmap2D(tU32 nW, tU32 nH, const achar* aszPixFmt) = 0;

  //! Create a 2D bitmap from the specified memory.
  //! \remark the given iPixelFormat instance will be release if the creation failed and when
  //!     the bitmap is released.
  virtual iBitmap2D* __stdcall CreateBitmap2DMemoryEx(tU32 nW, tU32 nH, iPixelFormat* pFmt, tU32 anPitch, tPtr ptrAddr, tBool bFreeAddr) = 0;
  //! Create a 2D bitmap from the specified memory.
  virtual iBitmap2D* __stdcall CreateBitmap2DMemory(tU32 nW, tU32 nH, const achar* aszPixFmt, tU32 anPitch, tPtr ptrAddr, tBool bFreeAddr) = 0;

  //! Create a Cube bitmap.
  virtual iBitmapCube* __stdcall CreateBitmapCubeEx(tU32 ulSize, iPixelFormat* pFmt) = 0;
  //! Create a Cube bitmap.
  virtual iBitmapCube* __stdcall CreateBitmapCube(tU32 ulSize, const achar* aszPixFmt) = 0;

  //! Create a 3D bitmap.
  virtual iBitmap3D*  __stdcall CreateBitmap3DEx(tU32 anW, tU32 anH, tU32 anD, iPixelFormat* apFmt) = 0;
  //! Create a 3D bitmap.
  virtual iBitmap3D*  __stdcall CreateBitmap3D(tU32 anW, tU32 anH, tU32 anD, const achar* aaszPixFmt) = 0;
  //! Create a 3D bitmap from the specified memory.
  virtual iBitmap3D* __stdcall CreateBitmap3DMemoryEx(tU32 anW, tU32 anH, tU32 anD, iPixelFormat* apFmt, tU32 anRowPitch, tU32 anSlicePitch, tPtr aptrAddr, tBool abFreeAddr) = 0;
  //! Create a 3D bitmap from the specified memory.
  virtual iBitmap3D* __stdcall CreateBitmap3DMemory(tU32 anW, tU32 anH, tU32 anD, const achar* aszPixFmt, tU32 anRowPitch, tU32 anSlicePitch, tPtr aptrAddr, tBool abFreeAddr) = 0;

  //! Load a bitmap from a file.
  virtual iBitmapBase* __stdcall LoadBitmap(iFile* pFile) = 0;
  //! Load a bitmap from a resource.
  virtual iBitmapBase* __stdcall LoadBitmapFromRes(iHString* ahspRes, iHString* ahspBasePath) = 0;
  //! Load a bitmap from a file with the specified format.
  virtual iBitmapBase* __stdcall LoadBitmapEx(const achar* aaszFormat, iFile* pFile) = 0;

  //! Save a bitmap in a file.
  virtual tBool __stdcall SaveBitmap(const achar* aszFile, iBitmapBase* pBmp, tU32 ulCompression = 0) = 0;

  //! Save a bitmap in the specified format in a file.
  virtual tBool __stdcall SaveBitmapEx(const achar* aaszFormat, iFile* apFile, iBitmapBase* apBmp, tU32 anCompression) = 0;

  //! Find the path/url matching the specified resource and basepath.
  virtual cString __stdcall URLFindBitmapFilePath(const achar* aszRes, const achar* aszBasePath) = 0;

  //! Open a bitmap file.
  //! \param  aszRes is the resource to open.
  //! \param  aszBasePath is an optional path where the resources will be searched first.
  //!     Only the directory of the path is used.
  //! \remark Search all the known extensions if no extension is specified.
  //! \remark If no resource manager is registered in the system only the low level File opening
  //!     methods are used.
  virtual iFile* __stdcall OpenBitmapFile(const achar* aszRes, const achar* aszBasePath = NULL) = 0;

  //! Computes the diff between two bitmaps.
  //! \param apImgA the first bitmap to compare
  //! \param apImgB the second bitmap to compare
  //! \param apOutput an optional bitmap where to output a visualisation of the difference
  //! \param afThreshold is the minimum color distance to be considered different. Between 0 and 1, default is 0.1.
  //! \param abIgnoreAA whether to ignore pixel that are part of antialiasing. Default is true.
  virtual tI64 __stdcall ComputeBitmapDiff(const iBitmap2D* apImgA, const iBitmap2D* apImgB, iBitmap2D* apOutput = NULL, tF64 afThreshold = 0.1, tBool abIgnoreAA = eTrue) = 0;
  //! @}

  //########################################################################################
  //! \name Geometries
  //########################################################################################
  //! @{

  //! Create a polygonal geometry.
  //! \remark to create a geometry that is not in system memory a renderer must be active.
  //! \remark if you switch renderer all geometries not created in system memory
  //!     will be invalidated.
  virtual iGeometry* __stdcall CreateGeometryPolygonal(tU32 aulNumVertices, tU32 aulNumIndices, tGeometryCreateFlags aFlags, tFVF aFVF) = 0;
  //! Create a polygonal geometry from an exisiting vertex array and index array.
  virtual iGeometry* __stdcall CreateGeometryPolygonalEx(iVertexArray* apVA, iIndexArray* apIA) = 0;

  //! Create a geometry modifier.
  //! \param  aszName is the name of the modifier.
  //! \param  apGeometry is the geometry that is gonna be modified by the modifier.
  //! \param  apInitData is a modifier specific initialization parameter.
  //! \return NULL if the modifier can't be created, else return a pointer to a valid geometry modifier.
  //! \remark Just a wrapper around CreateInstance("GeometryModifier.NAME",apGeometry,apInitData).
  virtual iGeometryModifier* __stdcall CreateGeometryModifier(const achar* aszName, iGeometry* apGeometry, iUnknown* apInitData) = 0;

  //! Create a cube polygonal geometry.
  virtual iGeometry* __stdcall CreateGeometryPolygonalCube(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                           const sVec3f& avCenter, tF32 afWidth,
                                                           tBool abCW,
                                                           tU32 aulColor,
                                                           const sMatrixf& amtxUV) = 0;
  //! Create a plane polygonal geometry.
  virtual iGeometry* __stdcall CreateGeometryPolygonalPlane(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                            ePlaneType aPlaneType, const sVec3f& avCenter,
                                                            tF32 afWidth, tF32 afHeight,
                                                            tU32 aulNumDiv,
                                                            tBool abCW,
                                                            tU32 aulColor,
                                                            const sMatrixf& amtxUV) = 0;
  //! Create a cylinder polygonal geometry.
  virtual iGeometry* __stdcall CreateGeometryPolygonalCylinder(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                               tF32 baseRadius, tF32 topRadius,
                                                               tF32 height, tI32 slices, tI32 stacks,
                                                               tBool abCW, tU32 aulColor,
                                                               const sMatrixf& amtxUV,
                                                               tBool abCap, tBool abCentered) = 0;
  //! Create a sphere polygonal geometry.
  virtual iGeometry* __stdcall CreateGeometryPolygonalSphere(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                             tF32 radius, tI32 slices, tI32 stacks,
                                                             tBool abCW, tU32 aulColor,
                                                             const sMatrixf& amtxUV) = 0;
  //! Create a geosphere polygonal geometry.
  virtual iGeometry* __stdcall CreateGeometryPolygonalGeosphere(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                                tF32 radius, tI32 slices,
                                                                tBool abCW, tU32 aulColor,
                                                                const sMatrixf& amtxUV) = 0;
  //! Create a disk sweep polygonal geometry.
  virtual iGeometry* __stdcall CreateGeometryPolygonalDiskSweep(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                                tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks, tF32 startAngle, tF32 sweepAngle,
                                                                tBool abCW, tU32 aulColor,
                                                                const sMatrixf& amtxUV) = 0;
  //! Update a disk polygonal geometry.
  virtual tBool __stdcall UpdateGeometryPolygonalDiskSweep(iGeometry* apGeom,
                                                           tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks, tF32 startAngle, tF32 sweepAngle,
                                                           tBool abCW, tU32 aulColor,
                                                           const sMatrixf& amtxUV) = 0;
  //! Create a disk polygonal geometry.
  virtual iGeometry* __stdcall CreateGeometryPolygonalDisk(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                           tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks,
                                                           tBool abCW, tU32 aulColor,
                                                           const sMatrixf& amtxUV) = 0;
  //! @}

  //########################################################################################
  //! \name Graphics context
  //########################################################################################
  //! @{

  //! Initialize the specified hardware driver.
  //! \remark Only one driver can be initialized for each graphics object instance.
  //! \remark "NULL/renNULL" driver will create a NULL driver.
  //! \remark "SOFT/renSOFT" driver will create a software renderer driver.
  //! \remark "Auto" driver will pick the most capable hardware driver first then fallback to software rendering if none can be found.
  virtual tBool __stdcall InitializeDriver(iHString* ahspDriverName) = 0;
  //! Get whether the driver has been initialized.
  //! {Property}
  virtual tBool __stdcall GetIsDriverInitialized() const = 0;
  //! Get the driver that has been initialized.
  //! {Property}
  virtual iGraphicsDriver* __stdcall GetDriver() const = 0;
  //! Get the driver's capabilities.
  //! {Property}
  virtual tInt __stdcall GetDriverCaps(eGraphicsCaps aCaps) const = 0;
  //! Create a new context for the specified OS window.
  virtual iGraphicsContext* __stdcall CreateContextForWindow(
    iOSWindow* apWindow,
    const achar* aaszBBFormat, const achar* aaszDSFormat,
    tU32 anSwapInterval, tTextureFlags aBackBufferFlags) = 0;
  //! Creates a new context for the specified render targets.
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(
    iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS) = 0;
  //! Get the generice device resource manager.
  //! {Property}
  virtual iDeviceResourceManager* __stdcall GetGenericDeviceResourceManager() const = 0;
  //! @}

  //########################################################################################
  //! \name States
  //########################################################################################
  //! @{

  //! Create a new sampler states instance.
  virtual iSamplerStates* __stdcall CreateSamplerStates() = 0;
  //! Create a new depth stencil states instance.
  virtual iDepthStencilStates* __stdcall CreateDepthStencilStates() = 0;
  //! Create a new rasterizer states instance.
  virtual iRasterizerStates* __stdcall CreateRasterizerStates() = 0;
  //! Create a new fixed pipeline states instance.
  virtual iFixedStates* __stdcall CreateFixedStates() = 0;
  //! @}

  //########################################################################################
  //! \name Textures
  //########################################################################################
  //! @{

  //! Check whether the specified texture format is supported and
  //! set the bitmap format object to the nearest matching native bitmap format.
  virtual tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags) = 0;

  //! Get the texture device resource manager.
  //! {Property}
  virtual iDeviceResourceManager* __stdcall GetTextureDeviceResourceManager() const = 0;

  //! Get the number of textures in the manager.
  //! {Property}
  virtual tU32 __stdcall GetNumTextures() const = 0;
  //! Get a texture from its name.
  //! {Property}
  virtual iTexture* __stdcall GetTextureFromName(iHString* ahspName) const = 0;
  //! Get a texture from its index.
  //! {Property}
  virtual iTexture* __stdcall GetTextureFromIndex(tU32 anIndex) const = 0;

  //! Create a new texture.
  //! \remark Setting anNumMipMaps to zero with eTextureFlags_MipMaps automatically computes the number of mipmaps down to 4x4.
  virtual iTexture* __stdcall CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags) = 0;
  //! Create a new texture from the specified bitmap.
  virtual iTexture* __stdcall CreateTextureFromBitmap(iHString* ahspName, iBitmapBase* apBitmap, tTextureFlags aFlags) = 0;
  //! Create a new texture from the specified resource.
  virtual iTexture* __stdcall CreateTextureFromRes(iHString* ahspRes, iHString* ahspBasePath, tTextureFlags aFlags) = 0;

  //! Set whether CreateTextureFromRes recreates the texture when the source resource has changed.
  //! {Property}
  virtual void __stdcall SetRecreateTextureWhenChanged(tBool abEnabled) = 0;
  //! Get whether CreateTextureFromRes recreates the texture when the source resource has changed.
  //! {Property}
  virtual tBool __stdcall GetRecreateTextureWhenChanged() const = 0;

  //! Blit a bitmap to a texture.
  virtual tBool __stdcall BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None) = 0;
  //! Blit a texture to a bitmap.
  virtual tBool __stdcall BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None) = 0;
  //! Blit a texture into another texture.
  virtual tBool __stdcall BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None) = 0;
  //! Blit a 3d bitmap to a 3d texture.
  virtual tBool __stdcall BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin = sVec3i::Zero(), const sVec3i& aDestMin = sVec3i::Zero(), const sVec3i& avSize = sVec3i::Zero(), eTextureBlitFlags aFlags = eTextureBlitFlags_None) = 0;
  //! Blit a 3d texture to a 3d bitmap.
  virtual tBool __stdcall BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin = sVec3i::Zero(), const sVec3i& aDestMin = sVec3i::Zero(), const sVec3i& avSize = sVec3i::Zero(), eTextureBlitFlags aFlags = eTextureBlitFlags_None) = 0;

  //! Create a bitmap from a texture.
  virtual iBitmapBase* __stdcall CreateBitmapFromTexture(iTexture* apSrc) = 0;
  //! @}

  //########################################################################################
  //! \name Shaders.
  //########################################################################################
  //! @{

  //! Get the number of Shader profile supported in the specified unit.
  virtual tU32 __stdcall GetNumShaderProfile(eShaderUnit aUnit) const = 0;
  //! Get the profile at the specified index.
  virtual iHString* __stdcall GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const = 0;

  //! Creates an empty shader constants instance.
  virtual iShaderConstants* __stdcall CreateShaderConstants(tU32 anMaxRegisters) const = 0;
  //! Serialize shader constants in the specified datatable.
  virtual tBool __stdcall SerializeShaderConstants(iShaderConstants* apConsts, iDataTable* apDT, tSerializeFlags aFlags) = 0;

  //! Get the shader device resource manager.
  //! {Property}
  virtual iDeviceResourceManager* __stdcall GetShaderDeviceResourceManager() const = 0;

  //! Get the number of shaders in the manager.
  //! {Property}
  virtual tU32 __stdcall GetNumShaders() const = 0;
  //! Get a shader from its name.
  //! {Property}
  virtual iShader* __stdcall GetShaderFromName(iHString* ahspName) const = 0;
  //! Get a shader from its index.
  //! {Property}
  virtual iShader* __stdcall GetShaderFromIndex(tU32 anIndex) const = 0;

  //! Create a new shader.
  virtual iShader* __stdcall CreateShader(iHString* ahspName, iFile* apByteCode) = 0;
  //! Create a new shader from the specified resource.
  virtual iShader* __stdcall CreateShaderFromRes(iHString* ahspRes) = 0;

  //! Set whether CreateShaderFromRes recreates the shader when the source resource has changed.
  //! {Property}
  virtual void __stdcall SetRecreateShaderWhenChanged(tBool abEnabled) = 0;
  //! Get whether CreateShaderFromRes recreates the shader when the source resource has changed.
  //! {Property}
  virtual tBool __stdcall GetRecreateShaderWhenChanged() const = 0;
  //! @}

  //########################################################################################
  //! \name Vertex, index and instance array.
  //########################################################################################
  //! @{

  //! Create a new driver vertex array instance.
  virtual iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) = 0;
  //! Create a new driver index array instance.
  virtual iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex, eArrayUsage aUsage) = 0;
  //! @}

  //########################################################################################
  //! \name Draw operation set.
  //########################################################################################
  //! @{

  //! Creates a new draw operation.
  virtual iDrawOperation* __stdcall CreateDrawOperation() = 0;
  //! Create a draw operation set.
  //! \remark if the passed sorter is NULL the default sorter will be used.
  virtual iDrawOperationSet* __stdcall CreateDrawOperationSet() = 0;
  //! @}

  //########################################################################################
  //! \name Occlusion queries.
  //########################################################################################
  //! @{

  //! Create a new occlusion query object.
  virtual iOcclusionQuery* __stdcall CreateOcclusionQuery() = 0;
  //! @}

  //########################################################################################
  //! \name Objects compilation
  //########################################################################################
  //! @{

  //! Compile a sampler states.
  virtual tIntPtr __stdcall CompileSamplerStates(iSamplerStates* apStates) = 0;
  //! Get the specified compiled sampler states.
  //! {Property}
  virtual iSamplerStates* __stdcall GetCompiledSamplerStates(tIntPtr aHandle) const = 0;
  //! Compile a rasterizer states.
  virtual tIntPtr __stdcall CompileRasterizerStates(iRasterizerStates* apStates) = 0;
  //! Get the specified compiled rasterizer states.
  //! {Property}
  virtual iRasterizerStates* __stdcall GetCompiledRasterizerStates(tIntPtr aHandle) const = 0;
  //! Compile a depth stencil states.
  virtual tIntPtr __stdcall CompileDepthStencilStates(iDepthStencilStates* apStates) = 0;
  //! Get the specified compiled depth stencil states.
  //! {Property}
  virtual iDepthStencilStates* __stdcall GetCompiledDepthStencilStates(tIntPtr aHandle) const = 0;
  //! @}

  //########################################################################################
  //! \name Materials
  //########################################################################################
  //! @{

  //! Create an empty material.
  virtual iMaterial* __stdcall CreateMaterial() = 0;
  //! Create a material library.
  virtual iMaterialLibrary* __stdcall CreateMaterialLibrary() = 0;
  //! @}

  //########################################################################################
  //! \name Graphics debugger
  //########################################################################################
  //! @{

  //! Create a new draw op capture object.
  virtual iGraphicsDrawOpCapture* __stdcall CreateDrawOpCapture() = 0;
  //! Set the draw op capture object.
  //! {Property}
  virtual void __stdcall SetDrawOpCapture(iGraphicsDrawOpCapture* apCapture) = 0;
  //! Get the draw op capture object.
  //! {Property}
  virtual iGraphicsDrawOpCapture* __stdcall GetDrawOpCapture() const = 0;
  //! @}

  //########################################################################################
  //! \name Colors
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Get the lumiance of a 3d color.
  //! \remark Uses the lumiance distribution kvColorLuminanceR21G71B07.
  virtual tF32 __stdcall ColorLuminance(const sVec3f& aColor) = 0;
  //! Get the lumiance of a 3d color, allow to specify a luminance distribution.
  virtual tF32 __stdcall ColorLuminanceEx(const sVec3f& aColor, const sVec3f& avLuminanceDistribution) = 0;
  //! Correct the gamma of a color.
  virtual sVec3f __stdcall ColorGammaCorrect(const sVec3f& aC, tF32 afFactor) = 0;
  //! Adjust the contrast of a color.
  virtual sVec3f __stdcall ColorAdjustContrast(const sVec3f& C, tF32 c) = 0;
  //! Adjust the saturation of a color.
  virtual sVec3f __stdcall ColorAdjustSaturation(const sVec3f& C, tF32 s) = 0;
  //! Get the negative of a color.
  virtual sVec3f __stdcall ColorNegative(const sVec3f& C) = 0;
  //! Gray a color.
  virtual sVec3f __stdcall ColorGrey(const sVec3f& C) = 0;
  //! Set a color to be black or white in function of the specified reference.
  virtual sVec3f __stdcall ColorBlackWhite(const sVec3f& C, tF32 s) = 0;
  //! Convert a color from one color space to another.
  virtual sVec4f __stdcall ColorConvert(eColorSpace aSource, eColorSpace aDest, const sVec4f& aColor) = 0;
  //! Convert a color from one color space to another, allows to specify the XYZ reference.
  //! \remark kcolXYZRef2D65 is the standard XYZ reference value
  virtual sVec4f __stdcall ColorConvertEx(eColorSpace aSource, eColorSpace aDest, const sVec4f& aColor, const sVec4f& aXYZRef) = 0;
#endif

  //! Get the number of named colors.
  //! {Property}
  virtual tU32 __stdcall GetNumNamedColors() const = 0;
  //! Get the name of the color at the specified index.
  //! {Property}
  virtual const achar* __stdcall GetColorName(tU32 anIndex) const = 0;
  //! Get the index of the color with the specified name.
  //! {Property}
  virtual tU32 __stdcall GetColorIndex(iHString* ahspName) const = 0;
  //! Get the value of the color at the specified index.
  //! {Property}
  virtual tU32 __stdcall GetColorValue(tU32 anIndex) const = 0;
  //! Find the color with the closest match to the specified color.
  virtual const achar* __stdcall FindColorName(const sColor4f& aColor) const = 0;
  //! Get a color from it's name.
  //! {Property}
  //! \remark Supported syntaxes, case insensitive, in order of priority:<br/>
  //!       - Vec3, Vec4, RGB, RGBA math expressions
  //!       - color name<br/>
  //!       - Hexadecimal code: "#AARRGGBB", "#RRGGBB", "#RGB", "#RGBA"<br/>
  //!       - (CSS) rgb(N,N,N), rgba(N,N,N,N), N = [0-255], [0-100]%, [0.0-1.0]f<br/>
  virtual sVec3f __stdcall GetColor3FromName(iHString* ahspName) const = 0;
  //! Get a color from it's name.
  //! {Property}
  virtual sVec4f __stdcall GetColor4FromName(iHString* ahspName) const = 0;
  //! Get a color from it's name, specifying the alpha.
  virtual sVec4f __stdcall GetColor4FromNameA(iHString* ahspName, tF32 afAlpha) const = 0;
  //! Get a color from it's name.
  //! {Property}
  //! \remark Supported syntaxes, case insensitive, in order of priority:<br/>
  //!       - Vec3 & Vec4 math expressions
  //!       - color name<br>
  //!       - Hexadecimal code : "#AARRGGBB", "#RRGGBB", "#RGB", "#RGBA"<br/>
  //!       - (CSS) rgb(N,N,N), rgba(N,N,N,N), N = [0-255], [0-100]%, [0.0-1.0]f<br/>
  //! \remark See the W3C CSS2 specifications.
  virtual sVec3f __stdcall GetCssColor3FromName(iHString* ahspName) const = 0;
  //! Get a css color from it's name.
  //! {Property}
  //! \remark See the W3C CSS2 specifications.
  virtual sVec4f __stdcall GetCssColor4FromName(iHString* ahspName) const = 0;
  //! Get a css color from it's name, specifying the alpha.
  //! \remark See the W3C CSS2 specifications.
  virtual sVec4f __stdcall GetCssColor4FromNameA(iHString* ahspName, tF32 afAlpha) const = 0;
  //! @}


  //########################################################################################
  //! \name UDPoints
  //########################################################################################
  //! @{

  //! Generates random points on a unit sphere.
  virtual void __stdcall UDPointsSphereRandom(tVec3fCVec* apResults) const = 0;
  //! Generates hammersley base2 points on a unit sphere.
  virtual void __stdcall UDPointsSphereHammersley(tVec3fCVec* apResults) const = 0;
  //! Generates hammersley base p1 points on a unit sphere.
  virtual void __stdcall UDPointsSphereHammersleyEx(tVec3fCVec* apResults, tI32 p1) const = 0;
  //! Generates halton p1=2,p2  points on a unit sphere.
  virtual void __stdcall UDPointsSphereHalton(tVec3fCVec* apResults, tI32 p2 = 3) const = 0;
  //! Generates halton p1,p2  points on a unit sphere.
  virtual void __stdcall UDPointsSphereHaltonEx(tVec3fCVec* apResults, tI32 p1, tI32 p2) const = 0;
  //! Generates random points on a unit hemisphere.
  virtual void __stdcall UDPointsHemisphereRandom(tVec3fCVec* apResults) const = 0;
  //! Generates hammersley base2 points on a unit hemisphere.
  virtual void __stdcall UDPointsHemisphereHammersley(tVec3fCVec* apResults) const = 0;
  //! Generates hammersley base p1 points on a unit hemisphere.
  virtual void __stdcall UDPointsHemisphereHammersleyEx(tVec3fCVec* apResults, tI32 p1) const = 0;
  //! Generates halton p1=2,p2  points on a unit hemisphere.
  virtual void __stdcall UDPointsHemisphereHalton(tVec3fCVec* apResults, tI32 p2 = 3) const = 0;
  //! Generates halton p1,p2  points on a unit hemisphere.
  virtual void __stdcall UDPointsHemisphereHaltonEx(tVec3fCVec* apResults, tI32 p1, tI32 p2) const = 0;
  //! Generate random points on a unit plane.
  virtual void __stdcall UDPointsPlaneRandom(tVec2fCVec* apResults) const = 0;
  //! Generate hammersley base2 points on a unit plane.
  virtual void __stdcall UDPointsPlaneHammersley(tVec2fCVec* apResults) const = 0;
  //! Generate hammersley base p1 points on a unit plane.
  virtual void __stdcall UDPointsPlaneHammersleyEx(tVec2fCVec* apResults, tI32 p1) const = 0;
  //! Generate halton p1=2,p2 points on a unit plane.
  virtual void __stdcall UDPointsPlaneHalton(tVec2fCVec* apResults, tI32 p2 = 3) const = 0;
  //! Generate halton p1,p2 points on a unit plane.
  virtual void __stdcall UDPointsPlaneHaltonEx(tVec2fCVec* apResults, tI32 p1, tI32 p2) const = 0;
  //! @}

  //########################################################################################
  //! \name Damped springs
  //########################################################################################
  //! @{

#if niMinFeatures(15)
  //! Get the damping ratio of a spring set with the specified Ks/Kd constants.
  virtual tF32 __stdcall DampedSpringGetDampingRatio(const tF32 Ks, const tF32 Kd) const = 0;
  //! Compute the value of Kd for the specified Ks that will result in a spring with the specified damping ratio.
  virtual tF32 __stdcall DampedSpringComputeKdFromDampingRatio(const tF32 Ks, const tF32 E) const = 0;
  //! Computes the acceleration of a 1D damped spring system.
  //! \see ni::iSystem::DampedSpringAcceleration4
  virtual tF32 __stdcall DampedSpringAcceleration1(tF32 D, tF32 V, const tF32 Ks, const tF32 Kd) const = 0;
  //! Computes the acceleration of a 1D damped spring system.
  //! \see ni::iSystem::DampedSpringAcceleration4
  virtual sVec2f __stdcall DampedSpringAcceleration2(const sVec2f& D, const sVec2f& V, const tF32 Ks, const tF32 Kd) const = 0;
  //! Computes the acceleration of a 1D damped spring system.
  //! \see ni::iSystem::DampedSpringAcceleration4
  virtual sVec3f __stdcall DampedSpringAcceleration3(const sVec3f& D, const sVec3f& V, const tF32 Ks, const tF32 Kd) const = 0;
  //! Computes the acceleration of a 4D damped spring system.
  //! The spring acceleration is applied in the direction opposite from its
  //! displacement from a rest position, and a damping acceleration is applied in
  //! the direction opposite from its velocity.
  //! The damping ratio is : E = Kd/(2*sqrt(Ks)).
  //! - When E=1, the spring system is critically damped, and the system will return
  //!   to its rest position in the minimal possibile time for the given value of
  //!   Ks.
  //! - When E < 1, the spring system is underdamped, and it will oscillate before
  //!   returning to rest.
  //! - When E > 1, then the system is overdamped and it will take longer than it is
  //!   necessary to reach equilibrum.
  //! \param D: is the displacement of the spring/object
  //! \param V: is the current velocity
  //! \param Ks: spring acceleration constant, control the spring component of the
  //!        acceleration - the stiffness of the spring (the higher the value the
  //!        faster the spring will reach its target).
  //! \param Kd: damping acceleration constant, control the damping component of
  //!        the acceleration.
  virtual sVec4f __stdcall DampedSpringAcceleration4(const sVec4f& D, const sVec4f& V, const tF32 Ks, const tF32 Kd) const = 0;
  //! Creates a 1D damped spring.
  //! \remark If Ks and Kd are zero a critically damped spring with Ks=10 is set.
  virtual ni::iDampedSpring1* __stdcall CreateDampedSpring1(const tF32 Ks, const tF32 Kd) const = 0;
  //! Creates a 2D damped spring.
  //! \remark If Ks and Kd are zero a critically damped spring with Ks=10 is set.
  virtual ni::iDampedSpring2* __stdcall CreateDampedSpring2(const tF32 Ks, const tF32 Kd) const = 0;
  //! Creates a 3D damped spring.
  //! \remark If Ks and Kd are zero a critically damped spring with Ks=10 is set.
  virtual ni::iDampedSpring3* __stdcall CreateDampedSpring3(const tF32 Ks, const tF32 Kd) const = 0;
  //! Creates a 4D damped spring.
  //! \remark If Ks and Kd are zero a critically damped spring with Ks=10 is set.
  virtual ni::iDampedSpring4* __stdcall CreateDampedSpring4(const tF32 Ks, const tF32 Kd) const = 0;
  //! Creates a 1D damped spring position.
  //! \remark If Ks and Kd are zero a critically damped spring with Ks=10 is set.
  virtual ni::iDampedSpringPosition1* __stdcall CreateDampedSpringPosition1(const tF32 Ks, const tF32 Kd) const = 0;
  //! Creates a 2D damped spring position.
  //! \remark If Ks and Kd are zero a critically damped spring with Ks=10 is set.
  virtual ni::iDampedSpringPosition2* __stdcall CreateDampedSpringPosition2(const tF32 Ks, const tF32 Kd) const = 0;
  //! Creates a 3D damped spring position.
  //! \remark If Ks and Kd are zero a critically damped spring with Ks=10 is set.
  virtual ni::iDampedSpringPosition3* __stdcall CreateDampedSpringPosition3(const tF32 Ks, const tF32 Kd) const = 0;
  //! Creates a 4D damped spring position.
  //! \remark If Ks and Kd are zero a critically damped spring with Ks=10 is set.
  virtual ni::iDampedSpringPosition4* __stdcall CreateDampedSpringPosition4(const tF32 Ks, const tF32 Kd) const = 0;
#endif
  //! @}

  //########################################################################################
  //! \name Objects
  //########################################################################################
  //! @{

  //! Creates a new transform.
  virtual iTransform* __stdcall CreateTransform() const = 0;
  //! Creates a new frustum.
  virtual iFrustum* __stdcall CreateFrustum() const = 0;
  //! Creates a new AABB.
  virtual iBoundingVolume* __stdcall CreateAABB() const = 0;
  //! Creates an intersection.
  virtual iIntersection* __stdcall CreateIntersection() const = 0;
  //! Creates a new camera instance.
  virtual iCamera* __stdcall CreateCamera() const = 0;
  //! Creates a new Non-uniform spline.
  virtual iNUSpline* __stdcall CreateNUSpline(eNUSplineType aType) const = 0;
  //! @}

  //########################################################################################
  //! \name Overlays
  //########################################################################################
  //! @{

  //! Creates a color overlay.
  virtual iOverlay* __stdcall CreateOverlayColor(const sColor4f& aColor) = 0;;
  //! Create an overlay from the specified resource.
  virtual iOverlay* __stdcall CreateOverlayResource(iHString* ahspRes) = 0;
  //! Creates an overlay that will use the specified texture.
  virtual iOverlay* __stdcall CreateOverlayTexture(iTexture* apTexture) = 0;
  //! Creates an overlay that will use the specified image.
  virtual iOverlay* __stdcall CreateOverlayImage(iImage* apImage) = 0;
  //! Creates an overlay that will use the specified material.
  virtual iOverlay* __stdcall CreateOverlayMaterial(iMaterial* apMaterial) = 0;
  //! @}

  //########################################################################################
  //! \name Font
  //########################################################################################
  //! @{

  //! Load a font. \see ni::iGraphics::LoadFont
  //! \remark This function uses the default get flags.
  //! \remark Returns a font instance that can be used directly.
  virtual iFont* __stdcall LoadFont(iHString* ahspName) = 0;
  //! Load a font.
  //! \remark A list of font names separated by commas can be
  //!         specified, the first font that matches will be used. A
  //!         font name can be a regular names such as 'Arial,
  //!         Courier New' and/or a file path. Ex: 'Arial,
  //!         fonts/arial.ttf' will look for the Arial font
  //!         registered in the system, if it isnt found it will try
  //!         to load from the fonts/arial.ttf resource file. Font
  //!         name that contain a '.' will not have Bold/Italic
  //!         appended to their name automatically when using the
  //!         eFontLoadFlags_Bold/Italic flags.
  //! \remark If no font matches, 'Default' is searched, if it fails the 8x8 font is returned.
  //! \remark For each name, system fonts are searched, then fonts which have
  //!         already been loaded.
  //! \remark Font names can be a "file pattern regex", aka contain wildcards etc...
  //! \remark Returns a font instance that can be used directly.
  virtual iFont* __stdcall LoadFontEx(iHString* ahspName, tFontLoadFlags aFlags) = 0;

  //! Create a font from the given file.
  //! \remark Font name is extract from the file name is available otherwise the
  //!         file name of the source path is used.
  //! \remark This method will not search for previously loaded font
  //!         and create a new base font object. The prefered method
  //!         to load/create fonts is to use the LoadFont/Ex() functions.
  virtual iFont* __stdcall CreateFont(iHString* ahspName, iFile* pFile) = 0;
  //! Create a font from the given bitmap.
  //! \remark This method will not search for previously loaded font
  //!         and create a new base font object. The prefered method
  //!         to load/create fonts is to use the LoadFont/Ex() functions.
  virtual iFont* __stdcall CreateFontFromBitmap(iHString* ahspName, iBitmap2D* pBmp) = 0;
  //! Create the default 8x8 system font.
  //! \remark The default font name is "8x8".
  //! \remark This method will not search for previously loaded font
  //!         and create a new base font object. The prefered method
  //!         to load/create fonts is to use the LoadFont/Ex() functions.
  virtual iFont* __stdcall CreateFont8x8(iHString* ahspName) = 0;

  //! Get the list of fonts find on the host OS.
  //! \return A map<display name, font file path> with all the fonts found on the host OS.
  virtual Ptr<tStringCMap> __stdcall EnumOSFonts() const = 0;

  //! Register all the default system fonts.
  virtual tBool __stdcall RegisterSystemFonts() = 0;
  //! Register a new font as a system font.
  virtual tBool __stdcall RegisterSystemFont(iHString* ahspName, iHString* ahspFilePath, iFont* apFont) = 0;
  //! Get the number of system fonts registered.
  //! {Property}
  virtual tU32 __stdcall GetNumSystemFonts() const = 0;
  //! Get the name of the specified system font.
  //! {Property}
  virtual iHString* __stdcall GetSystemFontName(tU32 anIndex) const = 0;
  //! Get the file path of the specified system font.
  //! {Property}
  virtual iHString* __stdcall GetSystemFontFilePath(tU32 anIndex) const = 0;
  //! Get the file name of the specified system font.
  //! {Property}
  virtual iHString* __stdcall GetSystemFontFileName(tU32 anIndex) const = 0;

  //! Get the number of fonts registered.
  //! {Property}
  //! \remark This doesn't include the font regular instances.
  virtual tU32 __stdcall GetNumFonts() const = 0;
  //! Get the font at the given index.
  //! {Property}
  virtual iFont* __stdcall GetFontFromIndex(tU32 anIndex) const = 0;
  //! Get a font from its exact name.
  //! {Property}
  virtual iFont* __stdcall GetFontFromName(iHString* ahspName) const = 0;
  //! Find a loaded font which matches the specified names.
  //! \remark Uses the same rules as LoadFontEx.
  virtual iFont* __stdcall FindFont(iHString* ahspName, tFontLoadFlags mode) const = 0;
  //! Clear all font caches.
  virtual tBool __stdcall ClearAllFontCaches() = 0;
  //! Get the global font cache ID.
  //! \remark The cache ID changes every time a font's cache is cleared.
  //! \remark The ID can be used to detect changes in the font
  //!         cache. For example to relayout geometries that depend
  //!         on the font's internal texture cache.
  //! \remark The global font cache ID should be used when the actual
  //!         fonts being used is not trivial (such as in a HTML context).
  //! {Property}
  virtual tU32 __stdcall GetFontCacheID() const = 0;
  //! @}

  //########################################################################################
  //! \name Images and Canvas.
  //########################################################################################
  //! @{

  //! Create a new blank empty image.
  virtual iImage* __stdcall CreateImage(tU32 anWidth, tU32 anHeight) = 0;
  //! Create a new image from the specified bitmap file.
  virtual iImage* __stdcall CreateImageFromFile(iFile* apFile) = 0;
  //! Create a new image from the specified resource.
  virtual iImage* __stdcall CreateImageFromResource(iHString* ahspRes) = 0;
  //! Create a new image from the specified bitmap.
  virtual iImage* __stdcall CreateImageFromBitmap(iBitmap2D* apBitmap) = 0;
  //! Create a new image from the specified texture.
  virtual iImage* __stdcall CreateImageFromTexture(iTexture* apTexture) = 0;
  //! Create a new image from the specified bitmap and texture.
  //! \remark The bitmap and texture must be of the same size.
  virtual iImage* __stdcall CreateImageFromBitmapAndTexture(iBitmap2D* apBitmap, iTexture* apTexture) = 0;
  //! Create a new image.
  virtual iImage* __stdcall CreateImageEx(tU32 anWidth, tU32 anHeight, iBitmap2D* apBmp, iTexture* apTex, iTexture* apDepthStencil) = 0;

  //! Create an image map.
  virtual iImageMap* __stdcall CreateImageMap(const achar* aaszBaseName, const achar* aaszPxf) = 0;

  //! Create a graphics context that uses the specified Image as main render target and depth buffer.
  virtual iGraphicsContext* __stdcall CreateImageGraphicsContext(iImage* apImage) = 0;

  //! Creates a new canvas.
  //! \param apContext is the graphics context where the canvas will draw
  //! \param apResetMaterial if not NULL is the material that will be used to reset the default material's value when ResetStates() is called
  virtual iCanvas* __stdcall CreateCanvas(iGraphicsContext* apContext, iMaterial* apResetMaterial) = 0;
  //! @}

  //########################################################################################
  //! \name Text object
  //########################################################################################
  //! @{

  //! Create a new text object.
  virtual Ptr<iTextObject> __stdcall CreateTextObject(const achar* aaszText, const sVec2f& avSize, const tF32 afContentsScale) = 0;
  //! @}

  //########################################################################################
  //! \name Video
  //########################################################################################
  //! @{

  //! Check whether the given file is a supported video file.
  virtual tBool __stdcall IsVideoFile(iFile* apFile) const = 0;
  //! Create a video decoder.
  virtual iVideoDecoder* __stdcall CreateVideoDecoder(iHString* ahspName, iFile* apFile, tU32 aFlags) = 0;
  //! @}

  //########################################################################################
  //! \name Vector Graphics rendering
  //########################################################################################
  //! @{

  //! Create iVGPath
  virtual Ptr<iVGPath> __stdcall CreateVGPath() = 0;

  //! Create iVGStyle
  virtual Ptr<iVGStyle> __stdcall CreateVGStyle() = 0;

  //! Create iVGTransform
  virtual Ptr<iVGTransform> __stdcall CreateVGTransform() = 0;

  //! Create iVGPolygonTesselator
  virtual Ptr<iVGPolygonTesselator> __stdcall CreateVGPolygonTesselator() = 0;

  virtual Ptr<iVGImage> __stdcall CreateVGImage(tU32 anWidth, tU32 anHeight) = 0;
  virtual Ptr<iVGImage> __stdcall CreateVGImageFromBitmap(iBitmap2D* apBitmap) = 0;
  virtual Ptr<iVGImage> __stdcall CreateVGImageFromTexture(iTexture* apTexture) = 0;
  virtual Ptr<iVGImage> __stdcall CreateVGImageFromBitmapAndTexture(iBitmap2D* apBitmap, iTexture* apTexture) = 0;
  virtual Ptr<iVGImage> __stdcall CreateVGImageFromFile(iFile* apFile) = 0;
  virtual Ptr<iVGImage> __stdcall CreateVGImageFromResource(iHString* ahspRes) = 0;
  virtual Ptr<iVGGradientTable> __stdcall CreateVGGradientTable(tU32 anSize) = 0;
  virtual Ptr<iVGGradientTable> __stdcall CreateVGGradientTableTwoColors(const sColor4f& acolStart, const sColor4f& acolEnd) = 0;
  virtual Ptr<iVGGradientTable> __stdcall CreateVGGradientTableStops(const tF32CVec* apOffsets, const tVec4fCVec* apColors) = 0;
  virtual Ptr<iVGPaint> __stdcall CreateVGPaint(eVGPaintType aType) = 0;
  virtual Ptr<iVGPaint> __stdcall CreateVGPaintSolid(const sColor4f& aColor) = 0;
  virtual Ptr<iVGPaintImage> __stdcall CreateVGPaintImage(iVGImage* apImage, eVGImageFilter aFilter, eVGWrapType aWrap) = 0;
  virtual Ptr<iVGPaintGradient> __stdcall CreateVGPaintGradient(eVGGradientType aType, iVGGradientTable* apGradientTable) = 0;
  virtual Ptr<iVGPaintGradient> __stdcall CreateVGPaintGradientTwoColors(eVGGradientType aType, const sColor4f& acolStart, const sColor4f& acolEnd) = 0;
  virtual Ptr<iVGPaintGradient> __stdcall CreateVGPaintGradientStops(eVGGradientType aType, const tF32CVec* apOffsets, const tVec4fCVec* apColors) = 0;

  //! @}
};

  template <typename T, typename S>
  static inline tBool HasShaderProfile(T* apGraphics, S aUnit, iHString* ahspProfileName) {
    niLoop(i,apGraphics->GetNumShaderProfile((eShaderUnit)aUnit)) {
      if (apGraphics->GetShaderProfile((eShaderUnit)aUnit,i) == ahspProfileName)
      return ni::eTrue;
  }
  return ni::eFalse;
}

niExportFunc(iUnknown*) New_niUI_Graphics(const Var&, const Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IGRAPHICS_31779012_H__
