// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"

#ifdef GDRV_DUMMY

#include "Graphics.h"
#include "GDRV_Utils.h"

//--------------------------------------------------------------------------------------------
//
// DummyTexture
//
//--------------------------------------------------------------------------------------------
struct cDummyTexture : public ni::cIUnknownImpl<iTexture,eIUnknownImplFlags_DontInherit1,iDeviceResource>
{
  WeakPtr<iGraphics> mwGraphics;
  tHStringPtr mhspName;
  const eBitmapType mType;
  const tU32 mnW, mnH, mnD;
  const tU32 mnMM;
  Ptr<iPixelFormat> mptrPxf;
  const tTextureFlags mnFlags;
  Ptr<iBitmapBase> mptrBitmap;

  cDummyTexture(iGraphics* apGraphics,
               iHString* ahspName,
               eBitmapType aType,
               tU32 anW, tU32 anH, tU32 anD,
               tU32 anMM,
               iPixelFormat* apPxf,
               tTextureFlags anFlags)
      : mwGraphics(apGraphics)
      , mhspName(ahspName)
      , mType(aType)
      , mnW(anW)
      , mnH(anH)
      , mnD(anD)
      , mnMM(anMM)
      , mptrPxf(apPxf)
      , mnFlags(anFlags)
  {
    apGraphics->GetTextureDeviceResourceManager()->Register(this);
    switch (aType) {
      case eBitmapType_2D:
        mptrBitmap = apGraphics->CreateBitmap2DEx(mnW,mnH,mptrPxf);
        break;
      case eBitmapType_Cube:
        mptrBitmap = apGraphics->CreateBitmapCubeEx(mnW,mptrPxf);
        break;
      case eBitmapType_3D:
        mptrBitmap = apGraphics->CreateBitmap3DEx(mnW,mnH,mnD,mptrPxf);
        break;
    }
  }
  cDummyTexture(iGraphics* apGraphics, iBitmap2D* apBmp)
      : mwGraphics(apGraphics)
      , mhspName(NULL)
      , mType(eBitmapType_2D)
      , mnW(apBmp->GetWidth())
      , mnH(apBmp->GetHeight())
      , mnD(0)
      , mnMM(apBmp->GetNumMipMaps())
      , mptrPxf(apBmp->GetPixelFormat())
      , mnFlags(eTextureFlags_SubTexture)
  {
    mptrBitmap = apBmp;
  }

  ~cDummyTexture() {
    QPtr<iGraphics> g = mwGraphics;
    if (g.IsOK() && g->GetTextureDeviceResourceManager()) {
      g->GetTextureDeviceResourceManager()->Unregister(this);
    }
  }

  virtual iHString *  __stdcall GetDeviceResourceName () const {
    return mhspName;
  }
  virtual tBool  __stdcall HasDeviceResourceBeenReset (tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource () {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown *apDevice) {
    return this;
  }

  virtual eBitmapType __stdcall GetType() const {
    return mType;
  }
  virtual tU32 __stdcall GetWidth() const {
    return mnW;
  }
  virtual tU32 __stdcall GetHeight() const {
    return mnH;
  }
  virtual tU32 __stdcall GetDepth() const {
    return mnD;
  }
  virtual iPixelFormat* __stdcall GetPixelFormat() const {
    return mptrPxf;
  }
  virtual tU32 __stdcall GetNumMipMaps() const {
    return mnMM;
  }
  virtual tTextureFlags __stdcall GetFlags() const {
    return mnFlags;
  }
  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const {
    QPtr<iGraphics> g = mwGraphics;
    if (g.IsOK() && mType == eBitmapType_Cube && anIndex < 6) {
      QPtr<iBitmapCube> cube = mptrBitmap.ptr();
      Ptr<cDummyTexture> tex = niNew cDummyTexture(
          g,cube->GetFace((eBitmapCubeFace)anIndex));
      return tex.GetRawAndSetNull();
    }
    return NULL;
  }
};


//--------------------------------------------------------------------------------------------
//
// DummyGraphicsContext
//
//--------------------------------------------------------------------------------------------
struct cDummyGraphicsContext :
    public sGraphicsContext<4,ni::cIUnknownImpl<iGraphicsContextRT,eIUnknownImplFlags_DontInherit1,iGraphicsContext> >
{
  iGraphicsDriver* mpParent;
  Ptr<iOSWindow> mptrWindow;
  tU32 mnSwapInterval;
  tTextureFlags mnBBFlags;
  sRecti mrectViewport;
  sRecti mrectScissor;

  cDummyGraphicsContext(
      iGraphicsDriver* apParent,
      iOSWindow* apWindow,
      const achar* aaszBBPxf,
      const achar* aaszDSPxf,
      tU32 anSwapInterval,
      tTextureFlags anBBFlags)
      : tGraphicsContextBase(apParent->GetGraphics())
  {
    mpParent = apParent;
    niAssert(mpParent != NULL);
    mptrWindow = apWindow;
    mnSwapInterval = anSwapInterval;
    mnBBFlags = anBBFlags;
    iGraphics* g = apParent->GetGraphics();
    Ptr<iPixelFormat> pxfBB = g->CreatePixelFormat(aaszBBPxf);
    if (!pxfBB.IsOK()) return;
    Ptr<iPixelFormat> pxfDS = g->CreatePixelFormat(aaszDSPxf);
    if (!pxfDS.IsOK()) return;
    const tU32 w = mptrWindow->GetClientSize().x;
    const tU32 h = mptrWindow->GetClientSize().y;
    mptrRT[0] = niNew cDummyTexture(g,_H("Dummy_MainRT"),eBitmapType_2D,
                                    w,h,0,0,pxfBB,eTextureFlags_RenderTarget);
    mptrDS = niNew cDummyTexture(g,_H("Dummy_MainDS"),eBitmapType_2D,
                                 w,h,0,0,pxfDS,eTextureFlags_DepthStencil);
    mptrFS = g->CreateFixedStates();
    mrectScissor = sRecti(0,0,w,h);
    mrectViewport = mrectScissor;
  }

  cDummyGraphicsContext(
      iGraphicsDriver* apParent,
      iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3,
      iTexture* apDS)
      : tGraphicsContextBase(apParent->GetGraphics())
  {
    iGraphics* g = apParent->GetGraphics();
    mpParent = apParent;
    niAssert(mpParent != NULL);
    mnSwapInterval = 0;
    mnBBFlags = 0;
    mptrRT[0] = apRT0;
    mptrRT[1] = apRT1;
    mptrRT[2] = apRT2;
    mptrRT[3] = apRT3;
    mptrDS = apRT1;
    mptrFS = g->CreateFixedStates();
    mrectScissor = sRecti(0,0,GetWidth(),GetHeight());
    mrectViewport = mrectScissor;
  }

  virtual tBool __stdcall IsOK() const {
    return mptrRT[0].IsOK();
  }

  virtual void __stdcall Invalidate() {
    mpParent = NULL;
    mptrWindow = NULL;
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return mpParent?mpParent->GetGraphics():NULL;
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return mpParent;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetViewport(const sRecti& aVal) {
    mrectViewport = aVal;
  }
  virtual sRecti __stdcall GetViewport() const {
    return mrectViewport;
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetScissorRect(const sRecti& aVal) {
    mrectScissor = aVal;
  }
  virtual sRecti __stdcall GetScissorRect() const {
    return mrectScissor;
  }
};

//--------------------------------------------------------------------------------------------
//
// DummyGraphicsDriver
//
//--------------------------------------------------------------------------------------------

struct cDummyGraphicsDriver : public cIUnknownImpl<iGraphicsDriver>
{
  iGraphics* mpGraphics;

  cDummyGraphicsDriver(iGraphics* apGraphics) {
    mpGraphics = apGraphics;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall IsOK() const {
    return mpGraphics != NULL;
  }
  virtual void __stdcall Invalidate() {
    mpGraphics = NULL;
  }

  /////////////////////////////////////////////
  virtual iGraphics* __stdcall GetGraphics() const {
    return mpGraphics;
  }

  /////////////////////////////////////////////
  virtual const achar* __stdcall GetName() const {
    return _A("Dummy");
  }
  virtual const achar* __stdcall GetDesc() const {
    return _A("Dummy Graphics Driver");
  }
  virtual const achar* __stdcall GetDeviceName() const {
    return _A("Default");
  }


  /////////////////////////////////////////////
  virtual iGraphicsContext* __stdcall CreateContextForWindow(
      iOSWindow* apWindow,
      const achar* aaszBBFormat, const achar* aaszDSFormat,
      tU32 anSwapInterval, tTextureFlags aBackBufferFlags)
  {
    Ptr<iGraphicsContext> ctx = niNew cDummyGraphicsContext(
        this,
        apWindow,
        aaszBBFormat,aaszDSFormat,
        anSwapInterval,
        aBackBufferFlags);
    if (!niIsOK(ctx))
      return NULL;
    return ctx.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(
    iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS)
  {
    Ptr<iGraphicsContextRT> ctx = niNew cDummyGraphicsContext(
      this, apRT0, apRT1, apRT2, apRT3, apDS);
    if (!niIsOK(ctx))
      return NULL;
    return ctx.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall ResetAllCaches() {
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tInt __stdcall GetCaps(eGraphicsCaps aCaps) const {
    switch (aCaps) {
      case eGraphicsCaps_Resize:
      case eGraphicsCaps_MultiContext:
      case eGraphicsCaps_ScissorTest:
      case eGraphicsCaps_OverlayTexture:
      case eGraphicsCaps_NumTextureUnits:
        return 1;
      case eGraphicsCaps_Texture2DMaxSize:
        return 0xFFFF;
      case eGraphicsCaps_TextureCubeMaxSize:
      case eGraphicsCaps_Texture3DMaxSize:
        return 0;
      case eGraphicsCaps_MaxVertexIndex:
        return 0xFFFFFFFF;
    }
    return 0;
  }

  /////////////////////////////////////////////
  virtual tGraphicsDriverImplFlags __stdcall GetGraphicsDriverImplFlags() const {
    return 0;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall GetIsInitialized() const {
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags) {
    return eTrue;
  }

  virtual iTexture* __stdcall CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags)  {
    Ptr<iPixelFormat> pxf = mpGraphics->CreatePixelFormat(aaszFormat);
    niCheck(pxf.IsOK(),NULL);
    return niNew cDummyTexture(
        mpGraphics,
        ahspName,
        aType,
        anWidth,anHeight,anDepth,
        anNumMipMaps,
        pxf,
        aFlags);
  }

  virtual tBool __stdcall BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags)  {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apDest->GetType() == eBitmapType_2D, eFalse);
    cDummyTexture* tex = (cDummyTexture*)apDest;
    iBitmap2D* texBmp = (iBitmap2D*)tex->mptrBitmap.ptr();
    texBmp->BlitStretch(
        apSrc,
        aSrcRect.x,aSrcRect.y,
        aDestRect.x,aDestRect.y,
        aSrcRect.GetWidth(),aSrcRect.GetHeight(),
        aDestRect.GetWidth(),aDestRect.GetHeight());
    return eTrue;
  }
  virtual tBool __stdcall BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags)  {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apSrc->GetType() == eBitmapType_2D, eFalse);
    cDummyTexture* tex = (cDummyTexture*)apSrc;
    iBitmap2D* texBmp = (iBitmap2D*)tex->mptrBitmap.ptr();
    apDest->BlitStretch(
        texBmp,
        aSrcRect.x,aSrcRect.y,
        aDestRect.x,aDestRect.y,
        aSrcRect.GetWidth(),aSrcRect.GetHeight(),
        aDestRect.GetWidth(),aDestRect.GetHeight());
    return eTrue;
  }
  virtual tBool __stdcall BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags)  {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apSrc->GetType() == eBitmapType_2D, eFalse);
    niCheckSilent(apDest->GetType() == eBitmapType_2D, eFalse);
    cDummyTexture* sTex = (cDummyTexture*)apSrc;
    iBitmap2D* sTexBmp = (iBitmap2D*)sTex->mptrBitmap.ptr();
    cDummyTexture* dTex = (cDummyTexture*)apDest;
    iBitmap2D* dTexBmp = (iBitmap2D*)dTex->mptrBitmap.ptr();
    dTexBmp->BlitStretch(
        sTexBmp,
        aSrcRect.x,aSrcRect.y,
        aDestRect.x,aDestRect.y,
        aSrcRect.GetWidth(),aSrcRect.GetHeight(),
        aDestRect.GetWidth(),aDestRect.GetHeight());
    return eTrue;
  }
  virtual tBool __stdcall BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)  {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apDest->GetType() == eBitmapType_3D, eFalse);
    cDummyTexture* dTex = (cDummyTexture*)apDest;
    iBitmap3D* dTexBmp = (iBitmap3D*)dTex->mptrBitmap.ptr();
    dTexBmp->Blit(apSrc,aSrcMin,aDestMin,avSize);
    return eTrue;
  }
  virtual tBool __stdcall BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)  {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    cDummyTexture* sTex = (cDummyTexture*)apSrc;
    iBitmap3D* sTexBmp = (iBitmap3D*)sTex->mptrBitmap.ptr();
    apDest->Blit(sTexBmp,aSrcMin,aDestMin,avSize);
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tU32 __stdcall GetNumShaderProfile(eShaderUnit aUnit) const {
    return 0;
  }
  virtual iHString* __stdcall GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const {
    return NULL;
  }
  virtual iShader* __stdcall CreateShader(iHString* ahspName, iFile* apFile) {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual iOcclusionQuery* __stdcall CreateOcclusionQuery() {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetDrawOpCapture(iGraphicsDrawOpCapture* apCapture) {
  }
  virtual iGraphicsDrawOpCapture* __stdcall GetDrawOpCapture() const {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) {
    return NULL;
  }
  virtual iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
    return NULL;
  }
  virtual iShaderConstants* __stdcall CreateShaderConstants(tU32) const {
    return NULL;
  }
  virtual tIntPtr __stdcall CompileSamplerStates(iSamplerStates* apStates) {
    return NULL;
  }
  virtual tIntPtr __stdcall CompileRasterizerStates(iRasterizerStates* apStates) {
    return NULL;
  }
  virtual tIntPtr __stdcall CompileDepthStencilStates(iDepthStencilStates* apStates) {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall InitHardwareCursor(iTexture* apTex, const sRecti& aRect, const sVec2i& avPivot) {
    return eTrue;
  }
  virtual tBool __stdcall UpdateHardwareCursor(tI32 anX, tI32 anY, tBool abImmediate) {
    return eTrue;
  }
  virtual tBool __stdcall ShowHardwareCursor(tBool abShown) {
    return eTrue;
  }
};

niExportFunc(iUnknown*) New_GraphicsDriver_Dummy(const Var& avarA, const Var&) {
  QPtr<iGraphics> ptrGraphics = avarA;
  niCheckIsOK(ptrGraphics,NULL);
  return niNew cDummyGraphicsDriver(ptrGraphics);
}

#endif // GDRV_DUMMY
