// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include <niUI/Utils/StatesUtils.h>
#include "FixedShaders.h"

#define CONTEXT_NOT_IMPLEMENTED(NAME,RET)                      \
  niWarning("Shouldn't called captured context's " #NAME);     \
  return RET;


struct sCapturedGraphicsContext : public cIUnknownImpl<iGraphicsContext> {
  Ptr<iGraphicsContext> mptrCapturedContext;
  Ptr<iFixedStates> mptrFS;
  Ptr<iMaterial> mptrMaterial;
  Ptr<iTexture> mptrRT[4];
  Ptr<iTexture> mptrDS;
  sRecti mrectViewport;
  sRecti mrectScissor;

  sCapturedGraphicsContext(iGraphicsContext* gc) {
    mptrCapturedContext = gc;
    mrectViewport = gc->GetViewport();
    mrectScissor = gc->GetScissorRect();
    mptrFS = gc->GetFixedStates() ? gc->GetFixedStates()->Clone() : NULL;
    mptrMaterial = gc->GetMaterial() ? gc->GetMaterial()->Clone() : NULL;
    niLoop(i,niCountOf(mptrRT)) {
      mptrRT[i] = gc->GetRenderTarget(i);
    }
    mptrDS = gc->GetDepthStencil();
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return mptrCapturedContext->GetGraphics();
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return mptrCapturedContext->GetDriver();
  }
  virtual tU32 __stdcall GetWidth() const {
    return mptrCapturedContext->GetWidth();
  }
  virtual tU32 __stdcall GetHeight() const {
    return mptrCapturedContext->GetHeight();
  }

  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    CONTEXT_NOT_IMPLEMENTED(ClearBuffers,;);
  }

  virtual iTexture* __stdcall GetRenderTarget(tU32 anIndex) const {
    if (anIndex >= niCountOf(mptrRT))
      return NULL;
    return mptrRT[anIndex];
  }
  virtual iTexture* __stdcall GetDepthStencil() const {
    return mptrDS;
  }

  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    CONTEXT_NOT_IMPLEMENTED(Display,eTrue);
  }

  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    // will be used for drawop rendering
    return mptrCapturedContext->DrawOperation(apDrawOp);
  }

  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    CONTEXT_NOT_IMPLEMENTED(CaptureFrontBuffer,NULL);
  }

  virtual void __stdcall SetViewport(const sRecti& aVal) {
    CONTEXT_NOT_IMPLEMENTED(SetViewport,;);
  }
  virtual sRecti __stdcall GetViewport() const {
    return mrectViewport;
  }
  virtual void __stdcall SetScissorRect(const sRecti& aVal) {
    CONTEXT_NOT_IMPLEMENTED(SetScissorRect,;);
  }
  virtual sRecti __stdcall GetScissorRect() const {
    return mrectScissor;
  }
  virtual tBool __stdcall SetFixedStates(iFixedStates* apStates) {
    CONTEXT_NOT_IMPLEMENTED(SetFixedStates,eTrue);
  }
  virtual iFixedStates* __stdcall GetFixedStates() const {
    return mptrFS;
  }
  virtual tBool __stdcall SetMaterial(iMaterial* apMat) {
    CONTEXT_NOT_IMPLEMENTED(SetMaterial,eTrue);
  }
  virtual const iMaterial* __stdcall GetMaterial() const {
    return mptrMaterial;
  }
};

class cGraphicsDrawOpCapture : public cIUnknownImpl<iGraphicsDrawOpCapture>
{
  niBeginClass(cGraphicsDrawOpCapture);

 public:
  cGraphicsDrawOpCapture() {
    _began = 0;
    _flags = 0;
    _stopAt = eInvalidHandle;
    _count = 0;
    _current = NULL;
    _matStopAt = NULL;
  }
  ~cGraphicsDrawOpCapture() {
    Invalidate();
  }

  void __stdcall Invalidate() {
    ClearCapture();
  }

  virtual tBool __stdcall BeginCapture() {
    ++_began;
    return eTrue;
  }
  virtual tU32 __stdcall EndCapture() {
    if (!_began) return eFalse;
    --_began;
    return _count;
  }
  virtual tBool __stdcall GetIsCapturing() const {
    return _began > 0;
  }
  virtual void __stdcall ClearCapture() {
    _captured.clear();
    _count = 0;
    _current = NULL;
  }
  virtual void __stdcall SetCaptureFlags(tGraphicsCaptureFlags aFlags) {
    _flags = aFlags;
  }
  virtual tGraphicsCaptureFlags __stdcall GetCaptureFlags() const {
    return _flags;
  }
  virtual void __stdcall SetCaptureStopAt(tU32 anStopAt) {
    _stopAt = anStopAt;
  }
  virtual tU32 __stdcall GetCaptureStopAt() const {
    return _stopAt;
  }
  virtual tU32 __stdcall GetNumCaptured() const {
    return _count;
  }
  virtual sVec4i __stdcall GetCapturedClear(tU32 anIndex) const {
    if (anIndex >= _captured.size()) return sVec4i::Zero();
    return _captured[anIndex].clear;
  }
  virtual iDrawOperation* __stdcall GetCapturedDrawOp(tU32 anIndex) const {
    if (anIndex >= _captured.size()) return NULL;
    return _captured[anIndex].drawOp;
  }
  virtual tF32 __stdcall GetCapturedDrawOpTime(tU32 anIndex) const {
    if (anIndex >= _captured.size()) return -1.0f;
    return (tF32)_captured[anIndex].time;
  }
  virtual iGraphicsContext* __stdcall GetCapturedDrawOpContext(tU32 anIndex) const {
    if (anIndex >= _captured.size()) return NULL;
    return _captured[anIndex].context;
  }
  virtual tBool __stdcall BeginCaptureDrawOp(iGraphicsContext* apContext, iDrawOperation* apDrawOp, const sVec4i& aClearParams) {
    if (!_began) // not began, do nothing
      return eTrue;
    if (_count > _stopAt) {
      ++_count;
      return eFalse;
    }
    _captured.push_back(sDrawOp());
    _current = &_captured.back();
    _current->time = ni::TimerInSeconds();
    if ((_flags&eGraphicsCaptureFlags_BreakOnStopAtBegin) &&  _count == _stopAt) {
      ni_debug_break();
      _flags &= ~eGraphicsCaptureFlags_BreakOnStopAtBegin;
    }
    return eTrue;
  }
  virtual void __stdcall EndCaptureDrawOp(iGraphicsContext* apContext, iDrawOperation* apDrawOp, const sVec4i& aClearParams) {
    if (!_began || _count > _stopAt) return;
    _current->context = apContext;
    _current->time = ni::TimerInSeconds()-_current->time;
    _current->clear = aClearParams;
    if (apContext && _count == _stopAt) {
      _began = eFalse;
      iGraphics* g = apContext->GetGraphics();
      if (_current->context.IsOK()) {
        _current->context = niNew sCapturedGraphicsContext(_current->context.ptr());
      }
      if (apDrawOp) {
        _current->drawOp = apDrawOp->Clone();
        if (_current->drawOp->GetMaterial()) {
          iDeviceResourceManager* drm = apContext->GetGraphics()->GetShaderDeviceResourceManager();
          Ptr<iMaterial> mat = _current->drawOp->GetMaterial()->Clone();
          if (!mat->GetShader(eShaderUnit_Vertex)) {
            const char* name = sFixedShaders::GetFixedVertexShaderName(
              sFixedShaders::GetFixedVertexShader(_current->drawOp->GetFVF(),*(sMaterialDesc*)mat->GetDescStructPtr()));
            mat->SetShader(eShaderUnit_Vertex,(iShader*)drm->GetFromName(sFixedShaders::GetFixedShaderPath(name)));
          }
          if (!mat->GetShader(eShaderUnit_Pixel)) {
            const char* name = sFixedShaders::GetFixedPixelShaderName(
              sFixedShaders::GetFixedPixelShader(*(sMaterialDesc*)mat->GetDescStructPtr()));
            mat->SetShader(eShaderUnit_Pixel, (iShader*)drm->GetFromName(sFixedShaders::GetFixedShaderPath(name)));
          }
          _current->drawOp->SetMaterial(mat);
        }
      }
      if (g) {
        // Create the "stop at" material
        if (!_matStopAt.IsOK()) {
          _matStopAt = g->CreateMaterial();
          _matStopAt->SetFlags(eMaterialFlags_NoLighting|
                               eMaterialFlags_DiffuseModulate|
                               eMaterialFlags_Translucent);
          _matStopAt->SetBlendMode(eBlendMode_Additive);
          _matStopAt->SetChannelColor(eMaterialChannel_Base,
                                      Vec4<tF32>(0.7f,0,0.7f,0));
          _matStopAt->SetDepthStencilStates(eCompiledStates_DS_NoDepthTest);
          _matStopAt->SetRasterizerStates(eCompiledStates_RS_NoCullingWireframe);
        }

        // Draw the draw operation once more in wireframe
        if (_current->drawOp.IsOK() && _matStopAt.IsOK()) {
          sGraphicsAllStates states(apContext);
          Ptr<iMaterial> prevMat = _current->drawOp->GetMaterial();
          _current->drawOp->SetMaterial(_matStopAt);
          apContext->DrawOperation(_current->drawOp);
          _current->drawOp->SetMaterial(prevMat);
        }
      }
      _began = eTrue;
    }
    else {
      _current->drawOp = apDrawOp;
    }
    if ((_flags&eGraphicsCaptureFlags_BreakOnStopAtEnd) &&  _count == _stopAt) {
      ni_debug_break();
      _flags &= ~eGraphicsCaptureFlags_BreakOnStopAtEnd;
    }
    ++_count;
  }

 private:
  struct sDrawOp {
    sVec4i clear;
    Ptr<iGraphicsContext> context;
    Ptr<iDrawOperation> drawOp;
    tF64 time;
  };
  sDrawOp* _current;
  astl::vector<sDrawOp> _captured;

  tU32 _began;
  tGraphicsCaptureFlags _flags;
  tU32 _stopAt;
  tU32 _count;

  Ptr<iMaterial> _matStopAt;

  niEndClass(cGraphicsDrawOpCapture);
};

///////////////////////////////////////////////
iGraphicsDrawOpCapture* __stdcall cGraphics::CreateDrawOpCapture() {
  return niNew cGraphicsDrawOpCapture();
}

///////////////////////////////////////////////
void __stdcall cGraphics::SetDrawOpCapture(iGraphicsDrawOpCapture* apCapture) {
  mptrDrawOpCapture = niGetIfOK(apCapture);
  if (mptrDrv.IsOK()) {
    mptrDrv->SetDrawOpCapture(mptrDrawOpCapture);
  }
}

///////////////////////////////////////////////
iGraphicsDrawOpCapture* __stdcall cGraphics::GetDrawOpCapture() const {
  return mptrDrawOpCapture;
}
