#pragma once
#ifndef __GDRV_UTILS_H_6F26F773_DB4F_2B46_9A99_2CF5575B8468__
#define __GDRV_UTILS_H_6F26F773_DB4F_2B46_9A99_2CF5575B8468__

namespace ni {

template <int MAXRT, typename BASE>
struct sGraphicsContext : public BASE {
  typedef sGraphicsContext<MAXRT,BASE> tGraphicsContextBase;

  Ptr<iFixedStates> mptrFS;
  Ptr<iMaterial> mptrMaterial;
  sMaterialDesc* mpMaterialDesc = NULL;
  Ptr<iTexture> mptrRT[MAXRT];
  Ptr<iTexture> mptrDS;

  sGraphicsContext(iGraphics* g) {
    mptrFS = g->CreateFixedStates();
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetFixedStates(iFixedStates* apStates) niImpl {
    niCheckSilent(niIsOK(apStates),eFalse);
    mptrFS = apStates;
    return eTrue;
  }
  virtual iFixedStates* __stdcall GetFixedStates() const niImpl {
    return mptrFS;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall SetMaterial(iMaterial* apMat) niImpl {
    mptrMaterial = apMat;
    if (mptrMaterial.IsOK()) {
      mpMaterialDesc = (sMaterialDesc*)apMat->GetDescStructPtr();
    }
    else {
      mpMaterialDesc = NULL;
    }
    return eTrue;
  }
  virtual const iMaterial* __stdcall GetMaterial() const niImpl {
    return mptrMaterial;
  }

  ///////////////////////////////////////////////
  virtual iTexture* __stdcall GetRenderTarget(tU32 anIndex) const {
    if (anIndex >= MAXRT)
      return NULL;
    return mptrRT[anIndex];
  }
  virtual iTexture* __stdcall GetDepthStencil() const {
    return mptrDS;
  }
  virtual tU32 __stdcall GetWidth() const {
    return mptrRT[0]->GetWidth();
  }
  virtual tU32 __stdcall GetHeight() const {
    return mptrRT[0]->GetHeight();
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall ChangeRenderTarget(tU32 anIndex, iTexture* apRT) niOverride {
    niCheckIsOK(mptrRT[anIndex], eFalse);
    niCheckIsOK(apRT, eFalse);
    niCheck(anIndex < MAXRT, eFalse);
    if (!mptrRT[anIndex]->GetPixelFormat()->IsSamePixelFormat(apRT->GetPixelFormat())) {
      niError(niFmt("Incompatible render target pxf, expected '%s', trying to set '%s'",
                    mptrRT[anIndex]->GetPixelFormat()->GetFormat(),
                    apRT->GetPixelFormat()->GetFormat()));
      return eFalse;
    }
    mptrRT[anIndex] = apRT;
    const sRecti r = Recti(0,0,this->GetWidth(),this->GetHeight());
    this->SetViewport(r);
    this->SetScissorRect(r);
    return eTrue;
  }
  virtual tBool __stdcall ChangeDepthStencil(iTexture* apDS) niOverride {
    niCheckIsOK(mptrDS, eFalse);
    niCheckIsOK(apDS, eFalse);
    if (!mptrDS->GetPixelFormat()->IsSamePixelFormat(apDS->GetPixelFormat())) {
      niError(niFmt("Incompatible depth stencil pxf, expected '%s', trying to set '%s'",
                    mptrDS->GetPixelFormat()->GetFormat(),
                    apDS->GetPixelFormat()->GetFormat()));
      return eFalse;
    }
    mptrDS = apDS;
    const sRecti r = Recti(0,0,this->GetWidth(),this->GetHeight());
    this->SetViewport(r);
    this->SetScissorRect(r);
    return eTrue;
  }

  /////////////////////////////////////////////
  __forceinline tMaterialFlags _GetMaterialFlags(const sMaterialDesc* apDOMat) {
    return apDOMat->mFlags;
  }

  /////////////////////////////////////////////
  __forceinline eBlendMode _GetBlendMode(const sMaterialDesc* apDOMat) {
    return apDOMat->mBlendMode;
  }

  /////////////////////////////////////////////
  __forceinline tIntPtr _GetRS(const sMaterialDesc* apDOMat) {
    if (apDOMat->mhRS) {
      return apDOMat->mhRS;
    }
    if (mpMaterialDesc && mpMaterialDesc->mhRS) {
      return mpMaterialDesc->mhRS;
    }
    return eCompiledStates_RS_NoCullingFilledScissor;
  }

  /////////////////////////////////////////////
  __forceinline tIntPtr _GetDS(const sMaterialDesc* apDOMat) {
    if (apDOMat->mhDS) {
      return apDOMat->mhDS;
    }
    if (mpMaterialDesc && mpMaterialDesc->mhDS) {
      return mpMaterialDesc->mhDS;
    }
    return eCompiledStates_DS_NoDepthTest;
  }

  /////////////////////////////////////////////
  __forceinline const sMaterialChannel& _GetChannel(const sMaterialDesc* apDOMat, eMaterialChannel aChannel) {
    if (mpMaterialDesc && mpMaterialDesc->mChannels[aChannel].mTexture.IsOK()) {
      return mpMaterialDesc->mChannels[aChannel];
    }
    return apDOMat->mChannels[aChannel];
  }
};
} // end of namespace ni

#endif // __GDRV_UTILS_H_6F26F773_DB4F_2B46_9A99_2CF5575B8468__
