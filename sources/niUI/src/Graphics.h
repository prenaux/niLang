#ifndef __GRAPHICS_62597650_H__
#define __GRAPHICS_62597650_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/Utils/IndexedMap.h>

#if !defined niEmbedded
#define CAN_RECREATE_TEXTURE_WHEN_CHANGED
#define CAN_RECREATE_SHADER_WHEN_CHANGED
#endif

#include "VG.h"
#define CHECKDRIVER(RET)  if (!mptrDrv.IsOK()) return RET;
#define CHECKMAINCTX(RET) if (!mptrMainContext.IsOK()) return RET;

struct sIM;

#define VAPOOL_SIZE   (2048)  // 512*4
#define IAPOOL_SIZE   (3072)  // 512*6
#define RECTBLOCK_SIZE  (VAPOOL_SIZE/4)


extern eBlendMode _kFixedPipelineChannelBlendModes[eMaterialChannel_Last];

namespace ni {

class cFont;

//////////////////////////////////////////////////////////////////////////////////////////////
// cGraphics declaration
class cGraphics : public ImplRC<iGraphics>
{
 public:
  cGraphics();
  ~cGraphics();

  //// iUnknown /////////////////////////////////
  tBool __stdcall IsOK() const;
  //// iUnknown /////////////////////////////////

  //// iGraphics ////////////////////////////////
  iPixelFormat* __stdcall CreatePixelFormat(const achar* aszFormat);
  iBitmapFormat* __stdcall CreateBitmapFormat(eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth);
  iBitmapFormat* __stdcall CreateBitmapFormatEx(eBitmapType aType, iPixelFormat* pFmt, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth);
  iBitmapFormat* __stdcall CreateBitmapFormatEmpty();
  iBitmapBase* __stdcall CreateBitmap(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth);
  iBitmapBase* __stdcall CreateBitmapEx(iHString* ahspName, eBitmapType aType, iPixelFormat* pFmt, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth);
  iBitmap2D* __stdcall CreateBitmap2DEx(tU32 nW, tU32 nH, iPixelFormat* pPixFmt);
  iBitmap2D* __stdcall CreateBitmap2D(tU32 nW, tU32 nH, const achar* aszPixFmt);
  virtual iBitmap2D* __stdcall CreateBitmap2DMemoryEx(tU32 nW, tU32 nH, iPixelFormat* pFmt, tU32 anPitch, tPtr ptrAddr, tBool bFreeAddr);
  virtual iBitmap2D* __stdcall CreateBitmap2DMemory(tU32 nW, tU32 nH, const achar* aszPixFmt, tU32 anPitch, tPtr ptrAddr, tBool bFreeAddr);
  iBitmapCube* __stdcall CreateBitmapCubeEx(tU32 ulSize, iPixelFormat* pFmt);
  iBitmapCube* __stdcall CreateBitmapCube(tU32 ulSize, const achar* aszPixFmt);
  iBitmap3D*  __stdcall CreateBitmap3DEx(tU32 ulW, tU32 ulH, tU32 ulD, iPixelFormat* pFmt);
  iBitmap3D*  __stdcall CreateBitmap3D(tU32 ulW, tU32 ulH, tU32 ulD, const achar* aszPixFmt);
  iBitmap3D* __stdcall CreateBitmap3DMemoryEx(tU32 nW, tU32 nH, tU32 nD, iPixelFormat* pFmt, tU32 anRowPitch, tU32 anSlicePitch, tPtr ptrAddr, tBool bFreeAddr);
  iBitmap3D* __stdcall CreateBitmap3DMemory(tU32 nW, tU32 nH, tU32 nD, const achar* aszPixFmt, tU32 anRowPitch, tU32 anSlicePitch, tPtr ptrAddr, tBool bFreeAddr);
  iBitmapBase* __stdcall LoadBitmap(iFile* pFile);
  iBitmapBase* __stdcall LoadBitmapFromRes(iHString* ahspRes, iHString* ahspBasePath);
  iBitmapBase* __stdcall LoadBitmapEx(const achar* aaszFormat, iFile* pFile);
  tBool __stdcall SaveBitmap(const achar* aszFile, iBitmapBase* pBmp, tU32 ulCompression = 0);
  tBool __stdcall SaveBitmapEx(const achar* aaszFormat, iFile* apFile, iBitmapBase* apBmp, tU32 anCompression);
  cString __stdcall URLFindBitmapFilePath(const achar* aszRes, const achar* aszBasePath) niImpl;
  iFile* __stdcall OpenBitmapFile(const achar* aszRes, const achar* aszBasePath = NULL);
  tI64 __stdcall ComputeBitmapDiff(const iBitmap2D* apImgA,
                                   const iBitmap2D* apImgB,
                                   iBitmap2D* apOutput = NULL,
                                   tF64 afThreshold = 0.1,
                                   tBool abIgnoreAA = eTrue);
  iGeometry* __stdcall CreateGeometryPolygonal(tU32 aulNumVertices, tU32 aulNumIndices, tGeometryCreateFlags aFlags, tFVF aFVF);
  iGeometry* __stdcall CreateGeometryPolygonalEx(iVertexArray* apVA, iIndexArray* apIA);
  iGeometry* __stdcall CreateGeometryPolygonalCube(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                   const sVec3f& avCenter, tF32 afWidth,
                                                   tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);
  iGeometry* __stdcall CreateGeometryPolygonalPlane(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                    ePlaneType aPlaneType, const sVec3f& avCenter,
                                                    tF32 afWidth, tF32 afHeight, tU32 aulNumDiv,
                                                    tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);
  iGeometry* __stdcall CreateGeometryPolygonalCylinder(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                       tF32 baseRadius, tF32 topRadius,
                                                       tF32 height, tI32 slices, tI32 stacks,
                                                       tBool abCW, tU32 aulColor,
                                                       const sMatrixf& amtxUV,
                                                       tBool abCap, tBool abCentered);
  iGeometry* __stdcall CreateGeometryPolygonalSphere(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                     tF32 radius, tI32 slices, tI32 stacks,
                                                     tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);
  iGeometry* __stdcall CreateGeometryPolygonalGeosphere(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                        tF32 radius, tI32 slices,
                                                        tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);
  iGeometry* __stdcall CreateGeometryPolygonalDiskSweep(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                        tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks, tF32 startAngle, tF32 sweepAngle,
                                                        tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);
  tBool __stdcall UpdateGeometryPolygonalDiskSweep(iGeometry* apGeom,
                                                   tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks, tF32 startAngle, tF32 sweepAngle,
                                                   tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);
  iGeometry* __stdcall CreateGeometryPolygonalDisk(tGeometryCreateFlags aFlags, tFVF aFVF,
                                                   tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 stacks,
                                                   tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);
  iGeometryModifier* __stdcall CreateGeometryModifier(const achar* aszName, iGeometry* apGeometry, iUnknown* apInitData);

  // GDRV_Main.cpp
  tBool __stdcall InitializeDriver(iHString* ahspDriverName);
  iGraphicsDriver* __stdcall GetDriver() const;
  tBool __stdcall GetIsDriverInitialized() const;
  tInt __stdcall GetDriverCaps(eGraphicsCaps aCaps) const;
  void __stdcall InvalidateDriver();
  iGraphicsContext* __stdcall CreateContextForWindow(
      iOSWindow* apWindow,
      const achar* aaszBBFormat, const achar* aaszDSFormat,
      tU32 anSwapInterval, tTextureFlags aBackBufferFlags);
  iGraphicsContextRT* __stdcall CreateContextForRenderTargets(
    iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS);
  iDeviceResourceManager* __stdcall GetGenericDeviceResourceManager() const;

  // GDRV_SamplerStates.cpp
  iSamplerStates* __stdcall CreateSamplerStates();

  // GDRV_DepthStencilStates.cpp
  iDepthStencilStates* __stdcall CreateDepthStencilStates();

  // GDRV_RasterizerStates.cpp
  iRasterizerStates* __stdcall CreateRasterizerStates();

  // GDRV_FixedStates.cpp
  iFixedStates* __stdcall CreateFixedStates();

  // GDRV_Textures.cpp
  tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags);
  iDeviceResourceManager* __stdcall GetTextureDeviceResourceManager() const;
  tU32 __stdcall GetNumTextures() const;
  iTexture* __stdcall GetTextureFromName(iHString* ahspName) const;
  iTexture* __stdcall GetTextureFromIndex(tU32 anIndex) const;
  iTexture* __stdcall CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags);
  iTexture* __stdcall CreateTextureFromBitmap(iHString* ahspName, iBitmapBase* apBitmap, tTextureFlags aFlags);
  iTexture* __stdcall CreateTextureFromRes(iHString* ahspRes, iHString* ahspBasePath, tTextureFlags aFlags);
  tBool __stdcall BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None);
  tBool __stdcall BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None);
  tBool __stdcall BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect = sRecti(0,0), const sRecti& aDestRect = sRecti(0,0), eTextureBlitFlags aFlags = eTextureBlitFlags_None);
  tBool __stdcall BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin = sVec3i::Zero(), const sVec3i& aDestMin = sVec3i::Zero(), const sVec3i& avSize = sVec3i::Zero(), eTextureBlitFlags aFlags = eTextureBlitFlags_None);
  tBool __stdcall BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin = sVec3i::Zero(), const sVec3i& aDestMin = sVec3i::Zero(), const sVec3i& avSize = sVec3i::Zero(), eTextureBlitFlags aFlags = eTextureBlitFlags_None);
  iBitmapBase* __stdcall CreateBitmapFromTexture(iTexture* apSrc);

  // GDRV_Shader.cpp
  tU32 __stdcall GetNumShaderProfile(eShaderUnit aUnit) const;
  iHString* __stdcall GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const;
  iShaderConstants* __stdcall CreateShaderConstants(tU32 anMaxRegisters) const;
  tBool __stdcall SerializeShaderConstants(iShaderConstants* apConsts, iDataTable* apDT, tSerializeFlags aFlags);

  iDeviceResourceManager* __stdcall GetShaderDeviceResourceManager() const niImpl;
  tU32 __stdcall GetNumShaders() const niImpl;
  iShader* __stdcall GetShaderFromName(iHString* ahspName) const niImpl;
  iShader* __stdcall GetShaderFromIndex(tU32 anIndex) const niImpl;
  iShader* __stdcall CreateShader(iHString* ahspName, iFile* apByteCode) niImpl;
  iShader* __stdcall CreateShaderFromRes(iHString* ahspRes) niImpl;

  void __stdcall SetRecreateShaderWhenChanged(tBool abEnabled);
  tBool __stdcall GetRecreateShaderWhenChanged() const;

  // GDRV_VertexArray.cpp
  iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage);

  // GDRV_IndexArray.cpp
  iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex, eArrayUsage aUsage);

  // GDRV_DrawOperation.cpp
  iDrawOperation* __stdcall CreateDrawOperation();
  iDrawOperationSet* __stdcall CreateDrawOperationSet();

  // GDRV_Query.cpp
  iOcclusionQuery* __stdcall CreateOcclusionQuery();

  // GDRV_Capture.cpp
  iBitmap2D* __stdcall CaptureFrontBuffer() const;

  // GDRV_Compiled.cpp
  tIntPtr __stdcall CompileSamplerStates(iSamplerStates* apStates);
  iSamplerStates* __stdcall GetCompiledSamplerStates(tIntPtr aHandle) const;
  tIntPtr __stdcall CompileRasterizerStates(iRasterizerStates* apStates);
  iRasterizerStates* __stdcall GetCompiledRasterizerStates(tIntPtr aHandle) const;
  tIntPtr __stdcall CompileDepthStencilStates(iDepthStencilStates* apStates);
  iDepthStencilStates* __stdcall GetCompiledDepthStencilStates(tIntPtr aHandle) const;

  // GDRV_Material.cpp
  iMaterial* __stdcall CreateMaterial();
  // MaterialLibrary.cpp
  iMaterialLibrary* __stdcall CreateMaterialLibrary();

  // GDRV_Capture.cpp
  iGraphicsDrawOpCapture* __stdcall CreateDrawOpCapture();
  void __stdcall SetDrawOpCapture(iGraphicsDrawOpCapture* apCapture);
  iGraphicsDrawOpCapture* __stdcall GetDrawOpCapture() const;

  tU32 __stdcall FVFGetTexCooDim(tFVF anFVF, tU32 anTexCooIndex) const;
  tU32 __stdcall FVFGetNumTexCoos(tFVF anFVF) const;
  cString __stdcall FVFToString(tFVF aFVF) const;
  tFVF __stdcall FVFFromString(const achar* aaszString) const;
  tU32 __stdcall FVFGetComponentOffset(tFVF aFVF, eFVF C) const;
  tU32 __stdcall FVFGetStride(tFVF aFVF) const;

  static iTransform* __stdcall _CreateTransform();
  iTransform* __stdcall CreateTransform() const { return _CreateTransform(); }
  static iFrustum* __stdcall _CreateFrustum();
  iFrustum* __stdcall CreateFrustum() const { return _CreateFrustum(); }
  static iBoundingVolume* __stdcall _CreateAABB();
  iBoundingVolume* __stdcall CreateAABB() const { return _CreateAABB(); }
  static iIntersection* __stdcall _CreateIntersection();
  iIntersection* __stdcall CreateIntersection() const { return _CreateIntersection(); }
  static iCamera* __stdcall _CreateCamera();
  iCamera* __stdcall CreateCamera() const { return _CreateCamera(); }

  tF32 __stdcall ColorLuminance(const sVec3f& aColor);
  tF32 __stdcall ColorLuminanceEx(const sVec3f& aColor, const sVec3f& avLuminanceDistribution);
  sVec3f __stdcall ColorGammaCorrect(const sVec3f& aC, tF32 afFactor);
  sVec3f __stdcall ColorAdjustContrast(const sVec3f& C, tF32 c);
  sVec3f __stdcall ColorAdjustSaturation(const sVec3f& C, tF32 s);
  sVec3f __stdcall ColorNegative(const sVec3f& C);
  sVec3f __stdcall ColorGrey(const sVec3f& C);
  sVec3f __stdcall ColorBlackWhite(const sVec3f& C, tF32 s);
  sVec4f __stdcall ColorConvert(eColorSpace aSource, eColorSpace aDest, const sVec4f& aColor);
  sVec4f __stdcall ColorConvertEx(eColorSpace aSource, eColorSpace aDest, const sVec4f& aColor, const sVec4<tF32>& aXYZRef = kcolXYZRef2D65);

  tU32 __stdcall GetNumNamedColors() const;
  const achar* __stdcall GetColorName(tU32 anIndex) const;
  tU32 __stdcall GetColorIndex(iHString* ahspName) const;
  tU32 __stdcall GetColorValue(tU32 anIndex) const;
  const achar* __stdcall FindColorName(const sColor4f& aColor) const;
  sVec3f __stdcall GetColor3FromName(iHString* ahspName) const;
  sVec4f __stdcall GetColor4FromName(iHString* ahspName) const;
  sVec4f __stdcall GetColor4FromNameA(iHString* ahspName, tF32 afAlpha) const;
  sVec3f __stdcall GetCssColor3FromName(iHString* ahspName) const;
  sVec4f __stdcall GetCssColor4FromName(iHString* ahspName) const;
  sVec4f __stdcall GetCssColor4FromNameA(iHString* ahspName, tF32 afAlpha) const;

  tF32 __stdcall DampedSpringGetDampingRatio(const tF32 Ks, const tF32 Kd) const;
  tF32 __stdcall DampedSpringComputeKdFromDampingRatio(const tF32 Ks, const tF32 E) const;
  tF32 __stdcall DampedSpringAcceleration1(tF32 D, tF32 V, const tF32 Ks, const tF32 Kd) const;
  sVec2f __stdcall DampedSpringAcceleration2(const sVec2f& D, const sVec2f& V, const tF32 Ks, const tF32 Kd) const;
  sVec3f __stdcall DampedSpringAcceleration3(const sVec3f& D, const sVec3f& V, const tF32 Ks, const tF32 Kd) const;
  sVec4f __stdcall DampedSpringAcceleration4(const sVec4f& D, const sVec4f& V, const tF32 Ks, const tF32 Kd) const;
  ni::iDampedSpring1* __stdcall CreateDampedSpring1(const tF32 Ks, const tF32 Kd) const;
  ni::iDampedSpring2* __stdcall CreateDampedSpring2(const tF32 Ks, const tF32 Kd) const;
  ni::iDampedSpring3* __stdcall CreateDampedSpring3(const tF32 Ks, const tF32 Kd) const;
  ni::iDampedSpring4* __stdcall CreateDampedSpring4(const tF32 Ks, const tF32 Kd) const;
  ni::iDampedSpringPosition1* __stdcall CreateDampedSpringPosition1(const tF32 Ks, const tF32 Kd) const;
  ni::iDampedSpringPosition2* __stdcall CreateDampedSpringPosition2(const tF32 Ks, const tF32 Kd) const;
  ni::iDampedSpringPosition3* __stdcall CreateDampedSpringPosition3(const tF32 Ks, const tF32 Kd) const;
  ni::iDampedSpringPosition4* __stdcall CreateDampedSpringPosition4(const tF32 Ks, const tF32 Kd) const;

  void __stdcall UDPointsSphereRandom(tVec3fCVec* apResults) const;
  void __stdcall UDPointsSphereHammersley(tVec3fCVec* apResults) const;
  void __stdcall UDPointsSphereHammersleyEx(tVec3fCVec* apResults, tI32 p1) const;
  void __stdcall UDPointsSphereHalton(tVec3fCVec* apResults, tI32 p2 = 3) const;
  void __stdcall UDPointsSphereHaltonEx(tVec3fCVec* apResults, tI32 p1, tI32 p2) const;
  void __stdcall UDPointsHemisphereRandom(tVec3fCVec* apResults) const;
  void __stdcall UDPointsHemisphereHammersley(tVec3fCVec* apResults) const;
  void __stdcall UDPointsHemisphereHammersleyEx(tVec3fCVec* apResults, tI32 p1) const;
  void __stdcall UDPointsHemisphereHalton(tVec3fCVec* apResults, tI32 p2 = 3) const;
  void __stdcall UDPointsHemisphereHaltonEx(tVec3fCVec* apResults, tI32 p1, tI32 p2) const;
  void __stdcall UDPointsPlaneRandom(tVec2fCVec* apResults) const;
  void __stdcall UDPointsPlaneHammersley(tVec2fCVec* apResults) const;
  void __stdcall UDPointsPlaneHammersleyEx(tVec2fCVec* apResults, tI32 p1) const;
  void __stdcall UDPointsPlaneHalton(tVec2fCVec* apResults, tI32 p2 = 3) const;
  void __stdcall UDPointsPlaneHaltonEx(tVec2fCVec* apResults, tI32 p1, tI32 p2) const;

  iNUSpline* __stdcall CreateNUSpline(eNUSplineType aType) const;

  iOverlay* __stdcall CreateOverlayResource(iHString* ahspRes) niImpl;
  iOverlay* __stdcall CreateOverlayTexture(iTexture* apTexture) niImpl;
  iOverlay* __stdcall CreateOverlayColor(const sColor4f& aColor) niImpl;
  iOverlay* __stdcall CreateOverlayImage(iImage* apImage) niImpl;
  iOverlay* __stdcall CreateOverlayMaterial(iMaterial* apMaterial) niImpl;
  iImageMap* __stdcall CreateImageMap(const achar* aaszBaseName, const achar* aaszPxf) niImpl;

  iFont* __stdcall CreateFont(iHString* ahspName, iFile* pFile) niImpl;
  iFont* __stdcall CreateFontFromBitmap(iHString* ahspName, iBitmap2D* pBmp) niImpl;
  iFont* __stdcall CreateFont8x8(iHString* ahspName) niImpl;
  tBool __stdcall ClearAllFontCaches() niImpl;
  tU32 __stdcall GetFontCacheID() const niImpl;

  Ptr<tStringCMap> __stdcall EnumOSFonts() const;

  tBool __stdcall RegisterSystemFonts() niImpl;
  tBool __stdcall RegisterSystemFont(iHString* ahspName, iHString* ahspFilePath, iFont* apFont) niImpl;

  tU32 __stdcall GetNumSystemFonts() const niImpl;
  iHString* __stdcall GetSystemFontName(tU32 anIndex) const niImpl;
  iHString* __stdcall GetSystemFontFilePath(tU32 anIndex) const niImpl;
  iHString* __stdcall GetSystemFontFileName(tU32 anIndex) const niImpl;

  tU32 __stdcall GetNumFonts() const niImpl;
  iFont* __stdcall GetFontFromIndex(tU32 anIndex) const niImpl;
  iFont* __stdcall GetFontFromName(iHString* ahspName) const niImpl;
  iFont* __stdcall FindFont(iHString* ahspName, tFontLoadFlags aFlags) const niImpl;

  iFont* __stdcall LoadFont(iHString* ahspName) niImpl;
  iFont* __stdcall LoadFontEx(iHString* ahspName, tFontLoadFlags aFlags) niImpl;

  virtual iImage* __stdcall CreateImage(tU32 anWidth, tU32 anHeight) niImpl;
  virtual iImage* __stdcall CreateImageFromFile(iFile* apFile) niImpl;
  virtual iImage* __stdcall CreateImageFromResource(iHString* ahspRes) niImpl;
  virtual iImage* __stdcall CreateImageFromBitmap(iBitmap2D* apBitmap) niImpl;
  virtual iImage* __stdcall CreateImageFromTexture(iTexture* apTexture) niImpl;
  virtual iImage* __stdcall CreateImageFromBitmapAndTexture(iBitmap2D* apBitmap, iTexture* apTexture) niImpl;
  virtual iImage* __stdcall CreateImageEx(tU32 anWidth, tU32 anHeight, iBitmap2D* apBmp, iTexture* apTex, iTexture* apDepthStencil) niImpl;
  virtual iGraphicsContext* __stdcall CreateImageGraphicsContext(iImage* apImage) niImpl;

  virtual iCanvas* __stdcall CreateCanvas(iGraphicsContext* apContext, iMaterial* apResetMaterial) niImpl;

  virtual Ptr<iTextObject> __stdcall CreateTextObject(const achar* aaszText, const sVec2f& avSize, const tF32 afContentsScale) niImpl;

  virtual void __stdcall SetRecreateTextureWhenChanged(tBool abEnabled) niImpl;
  virtual tBool __stdcall GetRecreateTextureWhenChanged() const niImpl;

  Ptr<iVGImage> __stdcall CreateVGImage(tU32 anWidth, tU32 anHeight);
  Ptr<iVGImage> __stdcall CreateVGImageFromBitmap(iBitmap2D* apBitmap);
  Ptr<iVGImage> __stdcall CreateVGImageFromTexture(iTexture* apTexture);
  Ptr<iVGImage> __stdcall CreateVGImageFromBitmapAndTexture(iBitmap2D* apBitmap, iTexture* apTexture);
  Ptr<iVGImage> __stdcall CreateVGImageFromFile(iFile* apFile);
  Ptr<iVGImage> __stdcall CreateVGImageFromResource(iHString* ahspRes);
  Ptr<iVGImage> __stdcall CreateVGImageFromAnyBitmap(iBitmap2D* apBmp);

  Ptr<iVGGradientTable> __stdcall CreateVGGradientTable(tU32 anSize);
  Ptr<iVGGradientTable> __stdcall CreateVGGradientTableTwoColors(const sColor4f& acolStart, const sColor4f& acolEnd);
  Ptr<iVGGradientTable> __stdcall CreateVGGradientTableStops(const tF32CVec* apOffsets, const tVec4fCVec* apColors);
  Ptr<iVGPaint>         __stdcall CreateVGPaint(eVGPaintType aType);
  Ptr<iVGPaint>         __stdcall CreateVGPaintSolid(const sColor4f& aColor);
  Ptr<iVGPaintImage>    __stdcall CreateVGPaintImage(iVGImage* apImage, eVGImageFilter aFilter, eVGWrapType aWrap);
  Ptr<iVGPaintGradient> __stdcall CreateVGPaintGradient(eVGGradientType aType, iVGGradientTable* apGradientTable);
  Ptr<iVGPaintGradient> __stdcall CreateVGPaintGradientTwoColors(eVGGradientType aType, const sColor4f& acolStart, const sColor4f& acolEnd);
  Ptr<iVGPaintGradient> __stdcall CreateVGPaintGradientStops(eVGGradientType aType, const tF32CVec* apOffsets, const tVec4fCVec* apColors);

  Ptr<iVGPath>                   __stdcall CreateVGPath();
  Ptr<iVGStyle>                  __stdcall CreateVGStyle();
  Ptr<iVGTransform>              __stdcall CreateVGTransform();
  Ptr<iVGPolygonTesselator>      __stdcall CreateVGPolygonTesselator();

  // Graphics_Video.cpp
  tBool __stdcall IsVideoFile(iFile* apFile) const niImpl;
  iVideoDecoder* __stdcall CreateVideoDecoder(iHString* ahspName, iFile* apFile, tU32 aFlags) niImpl;
  //// iGraphics ////////////////////////////////

 public:
  // Graphics driver
  Ptr<iGraphicsDriver>        mptrDrv;
  Ptr<iGraphicsContext>       mptrMainContext;
  Ptr<iDeviceResourceManager> mptrDRMGeneric;
  Ptr<iDeviceResourceManager> mptrDRMTextures;
  Ptr<iDeviceResourceManager> mptrDRMShaders;

  // Error texture
  Ptr<iTexture> mptrErrorTexture;
  ni::iTexture* __stdcall GetErrorTexture();

  // Draw op capture
  Ptr<iGraphicsDrawOpCapture>   mptrDrawOpCapture;

  tBool _CompileDefaultRasterizerStates();
  tBool _CompileDefaultDepthStencilStates();
  tBool _CompileDefaultSamplerStates();

  // States
  typedef astl::hash_map<tIntPtr,Ptr<iRasterizerStates> > tRSMap;
  typedef astl::hash_map<tIntPtr,Ptr<iDepthStencilStates> > tDSMap;
  typedef astl::hash_map<tIntPtr,Ptr<iSamplerStates> > tSSMap;
  tDSMap mmapDepthStencilStates;
  tRSMap mmapRasterizerStates;
  tSSMap mmapSamplerStates;
  tIntPtr mStatesHandleGen;

  /// Fonts ///
  struct sMyFTLibrary* __stdcall GetFTLibrary();
  cFont* __stdcall _GetFallbackFont() const;

  tBool mbRegisteredSystemFonts;
  Ptr<struct sMyFTLibrary> mptrFTLib;
  struct sSystemFont {
    tHStringPtr hspName;
    tHStringPtr hspFilePath;
    tHStringPtr hspFileName;
    Ptr<iFont>  ptrFont;
  };
  typedef ni::IndexedMap<tHStringPtr,sSystemFont> tSystemFontsMap;
  tSystemFontsMap mmapSystemFonts;

  typedef ni::IndexedMap<tHStringPtr,iFont*> tFontMap;
  void _RegisterBaseFont(cFont* apFont);
  tBool _UnregisterBaseFont(iFont* apFont);
  tFontMap   mmapBaseFonts;
  Ptr<cFont> mptrFallbackFont;
  tU32       mnFontCacheID;

  /// Resource auto reload when changed ///
  typedef astl::hstring_hash_map<Ptr<iTime> > tResourceTimeMap;
#ifdef CAN_RECREATE_TEXTURE_WHEN_CHANGED
  tBool mbRecreateTextureWhenChanged;
  tResourceTimeMap mTextureTimeMap;
#endif
#ifdef CAN_RECREATE_SHADER_WHEN_CHANGED
  tBool mbRecreateShaderWhenChanged;
  tResourceTimeMap mShaderTimeMap;
#endif
};
} // end of namespace ni

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __GRAPHICS_62597650_H__
