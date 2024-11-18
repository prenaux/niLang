// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include <niLang/Utils/ConcurrentImpl.h>

#ifdef niWindows
#include <niLang/Platforms/Win32/Win32_Redef.h>
#endif

#ifdef GDRV_DUMMY
niExportFunc(iUnknown*) New_GraphicsDriver_Dummy(const Var& avarA, const Var&);
#endif
#ifdef GDRV_GL2
niExportFunc(iUnknown*) New_GraphicsDriver_GL2(const Var& avarA, const Var&);
#endif
#ifdef GDRV_METAL
niExportFunc(iUnknown*) New_GraphicsDriver_Metal(const Var& avarA, const Var&);
#endif

//--------------------------------------------------------------------------------------------
//
//  Main
//
//--------------------------------------------------------------------------------------------

static inline void _RegisterGraphicsDriver(const achar* aName, tpfnCreateObjectInstance apfnCreateInstance) {
  astl::upsert(
      *ni::GetLang()->GetCreateInstanceMap(),
      niFmt("GraphicsDriver.%s",aName),
      ni::Callback2<tpfnCreateObjectInstance>(apfnCreateInstance));
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::InitializeDriver(iHString* ahspDriverName) {
  {
    static tBool _bRegisteredDriver = eFalse;
    if (!_bRegisteredDriver) {
      _bRegisteredDriver = eTrue;
#ifdef GDRV_DUMMY
      _RegisterGraphicsDriver("Dummy",New_GraphicsDriver_Dummy);
#endif
#ifdef GDRV_GL2
      _RegisterGraphicsDriver("GL2",New_GraphicsDriver_GL2);
#endif
#ifdef GDRV_METAL
      _RegisterGraphicsDriver("Metal",New_GraphicsDriver_Metal);
#endif
    }
  }

  if (mptrDrv.IsOK()) {
    niWarning(_A("Driver already initialized."));
    return eFalse;
  }

  tHStringPtr hspDriver = ahspDriverName;

#ifdef GDRV_DUMMY
  if (HStringIsEmpty(hspDriver) || StrIEq(niHStr(hspDriver),"Dummy")) {
    hspDriver = _H("Dummy");
  }
  else
#endif

  if (StrIEq(niHStr(hspDriver),"Auto")) {
#if defined niWindows
    if (GetSystemMetrics(SM_REMOTESESSION)) {
      // Windows RDP works only with DirectX
      niLog(Info, "Windows RDP detected.");
    }
#endif

    {
#if defined GDRV_GL2
      hspDriver = _H("GL2");
#elif defined GDRV_METAL
      hspDriver = _H("Metal");
#else
      niError("No Auto graphics driver available.");
      return eFalse;
#endif
    }
  }

  {
    cString driverName = niFmt("GraphicsDriver.%s",hspDriver);
    QPtr<iGraphicsDriver> ptrDrv = ni::GetLang()->CreateInstance(driverName.c_str(),this);
    if (!ptrDrv.IsOK()) {
      niError(niFmt(_A("Can't create instance of '%s'."),driverName));
      return eFalse;
    }
    mptrDrv = ptrDrv;
  }

  mptrDRMGeneric = ni::GetLang()->CreateDeviceResourceManager(_A("GraphicsGeneric"));
  niCheck(mptrDRMGeneric.IsOK(),eFalse);

  mptrDRMTextures = ni::GetLang()->CreateDeviceResourceManager(_A("GraphicsTextures"));
  niCheck(mptrDRMTextures.IsOK(),eFalse);

  mptrDRMShaders = ni::GetLang()->CreateDeviceResourceManager(_A("GraphicsShaders"));
  niCheck(mptrDRMShaders.IsOK(),eFalse);

  niCheck(_CompileDefaultRasterizerStates(),eFalse);
  niCheck(_CompileDefaultDepthStencilStates(),eFalse);
  niCheck(_CompileDefaultSamplerStates(),eFalse);

  niLog(Info, niFmt("Initialized graphics driver '%s'.", hspDriver));
  return eTrue;
}

void __stdcall cGraphics::InvalidateDriver() {
  if (mptrDRMGeneric.IsOK()) {
    mptrDRMGeneric->Invalidate();
    mptrDRMGeneric = NULL;
  }
  if (mptrDRMTextures.IsOK()) {
    mptrDRMTextures->Invalidate();
    mptrDRMTextures = NULL;
  }
  if (mptrDRMShaders.IsOK()) {
    mptrDRMShaders->Invalidate();
    mptrDRMShaders = NULL;
  }
  if (mptrMainContext.IsOK()) {
    mptrMainContext->Invalidate();
    mptrMainContext = NULL;
  }
  if (mptrDrv.IsOK()) {
    mptrDrv->Invalidate();
    mptrDrv = NULL;
  }
}

///////////////////////////////////////////////
iGraphicsContext* __stdcall cGraphics::CreateContextForWindow(
    iOSWindow* apWindow,
    const achar* aaszBBFormat, const achar* aaszDSFormat,
    tU32 anSwapInterval, tTextureFlags aBackBufferFlags)
{
  if (!mptrDrv.IsOK()) {
    niWarning(_A("Driver not initialized."));
    return NULL;
  }

  Ptr<iGraphicsContext> ctx = mptrDrv->CreateContextForWindow(
      apWindow,
      aaszBBFormat,aaszDSFormat,
      anSwapInterval,aBackBufferFlags);
  if (!ctx.IsOK()) {
    niError(_A("Can't initialize the graphics driver context."));
    return NULL;
  }

  if (!niIsOK(ctx->GetFixedStates())) {
    niError(_A("No valid fixed states."));
    return NULL;
  }
  if (!niIsOK(ctx->GetRenderTarget(0))) {
    niError(_A("No valid main render target."));
    return NULL;
  }
  if (!niIsOK(ctx->GetDepthStencil())) {
    niError(_A("No valid depth stencil."));
    return NULL;
  }

  niLog(Info,
      niFmt(_A("Context '%p' created : driver '%s', %dx%d, backbuffer '%s', depth stencil '%s'."),
            ctx.ptr(),
            mptrDrv->GetName(),
            ctx->GetWidth(),ctx->GetHeight(),
            ctx->GetRenderTarget(0)->GetPixelFormat()->GetFormat(),
            ctx->GetDepthStencil()->GetPixelFormat()->GetFormat()));

  return ctx.GetRawAndSetNull();
}

///////////////////////////////////////////////
iGraphicsContextRT* __stdcall cGraphics::CreateContextForRenderTargets(
  iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS)
{
  if (!mptrDrv.IsOK()) {
    niWarning(_A("Driver not initialized."));
    return NULL;
  }

  Ptr<iGraphicsContextRT> ctx = mptrDrv->CreateContextForRenderTargets(
      apRT0, apRT1, apRT2, apRT3, apDS);
  if (!ctx.IsOK()) {
    niError(_A("Can't initialize the graphics driver context."));
    return NULL;
  }

  if (!niIsOK(ctx->GetFixedStates())) {
    niError(_A("No valid fixed states."));
    return NULL;
  }
  if (apRT0 && !niIsOK(ctx->GetRenderTarget(0))) {
    niError(_A("No valid main render target."));
    return NULL;
  }
  if (apDS && !niIsOK(ctx->GetDepthStencil())) {
    niError(_A("No valid depth stencil."));
    return NULL;
  }

  return ctx.GetRawAndSetNull();
}

///////////////////////////////////////////////
tInt __stdcall cGraphics::GetDriverCaps(eGraphicsCaps aCaps) const {
  CHECKDRIVER(0);
  return mptrDrv->GetCaps(aCaps);
}


///////////////////////////////////////////////
iGraphicsDriver* __stdcall cGraphics::GetDriver() const
{
  return mptrDrv;
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::GetIsDriverInitialized() const {
  return mptrDrv.IsOK();
}

///////////////////////////////////////////////
iDeviceResourceManager* __stdcall cGraphics::GetGenericDeviceResourceManager() const {
  //  CHECKCONTEXT(NULL); // removed because initialize context needs it
  return mptrDRMGeneric;
}

///////////////////////////////////////////////
iBitmap2D* __stdcall cGraphics::CaptureFrontBuffer() const {
  CHECKMAINCTX(NULL);
  return mptrMainContext->CaptureFrontBuffer();
}

///////////////////////////////////////////////
iOcclusionQuery* __stdcall cGraphics::CreateOcclusionQuery() {
  CHECKDRIVER(NULL);
  return mptrDrv->CreateOcclusionQuery();
}

///////////////////////////////////////////////
iTexture* __stdcall cGraphics::GetErrorTexture() {
  if (!mptrErrorTexture.IsOK()) {
    mptrErrorTexture = GetTextureFromName(_H("base/error"));
  }

  if (!mptrErrorTexture.IsOK()) {
    Ptr<iFile> fp = OpenBitmapFile(_A("base/error"),NULL);
    if (fp.IsOK()) {
      Ptr<iBitmapBase> bmp = LoadBitmap(fp);
      if (bmp.IsOK()) {
        mptrErrorTexture = CreateTextureFromBitmap(_H("base/error"),bmp,0);
      }
    }
  }

  if (!mptrErrorTexture.IsOK()) {
    niWarning(_A("Can't load the Error texture."));
  }

  return mptrErrorTexture;
}
