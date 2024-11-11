#include "stdafx.h"
#include "API/niUI/Experimental/IGpu.h"

#define GPU_BLENDMODE(OP,SRCRGB,SRCALPHA,DESTRGB,DESTALPHA) \
  { eGpuBlendOp_##OP, eGpuBlendFactor_##SRCRGB, eGpuBlendFactor_##SRCALPHA, eGpuBlendFactor_##DESTRGB, eGpuBlendFactor_##DESTALPHA }

const sGpuBlendModeDesc& ToGpuBlendModeDesc(eBlendMode aBlendMode) {
  static const sGpuBlendModeDesc _toBlendModeDesc[] = {
    // eBlendMode_NoBlending
    GPU_BLENDMODE(Add, One, One, Zero, Zero),
    // eBlendMode_ReplaceAlpha
    GPU_BLENDMODE(Add, Zero, Zero, One, One),
    // eBlendMode_Additive
    GPU_BLENDMODE(Add, One, One, One, One),
    // eBlendMode_Modulate
    GPU_BLENDMODE(Add, DstColor, DstColor, Zero, Zero),
    // eBlendMode_Modulate2x
    GPU_BLENDMODE(Add, DstColor, DstColor, SrcColor, SrcColor),
    // eBlendMode_Translucent
    GPU_BLENDMODE(Add, SrcAlpha, SrcAlpha, InvSrcAlpha, InvSrcAlpha),
    // eBlendMode_TranslucentInvAlpha
    GPU_BLENDMODE(Add, InvSrcAlpha, InvSrcAlpha, SrcAlpha, SrcAlpha),
    // eBlendMode_TintedGlass
    GPU_BLENDMODE(Add, SrcColor, SrcColor, InvSrcColor, InvSrcColor),
    // eBlendMode_PreMulAlpha
    GPU_BLENDMODE(Add, One, One, InvSrcAlpha, InvSrcAlpha),
    // eBlendMode_ModulateReplaceAlpha
    GPU_BLENDMODE(Add, DstColor, DstColor, Zero, Zero),
    // eBlendMode_Modulate2xReplaceAlpha
    GPU_BLENDMODE(Add, DstColor, DstColor, SrcColor, SrcColor),
  };
  niCAssert(niCountOf(_toBlendModeDesc) == eBlendMode_Last);
  return _toBlendModeDesc[aBlendMode];
}

struct sGpuPipelineDescImpl : public ImplRC<iGpuPipelineDesc> {
  sGpuPipelineDesc _desc;

  sGpuPipelineDescImpl() {
    _desc.mDepthFormat = eGpuPipelineDepthFormat_None;
    _desc.mFVF = 0;
    _desc.mhRS = 0;
    _desc.mhDS = 0;
    niCAssert(niCountOf(_desc.mColorFormats) == 4);
    niLoop(i,niCountOf(_desc.mColorFormats)) {
      _desc.mColorFormats[i] = eGpuPipelineColorFormat_None;
    }
  }

  tBool __stdcall Copy(const iGpuPipelineDesc* apPipe) niImpl {
    niCheckIsOK(apPipe,eFalse);
    _desc = *(sGpuPipelineDesc*)apPipe->GetDescStructPtr();
    return eTrue;
  }

  Ptr<iGpuPipelineDesc> __stdcall Clone() const niImpl {
    NN<sGpuPipelineDescImpl> clone = MakeNN<sGpuPipelineDescImpl>();
    clone->Copy(this);
    return clone;
  }

  tBool __stdcall SetColorFormat(tU32 anIndex, eGpuPipelineColorFormat aFormat) niImpl {
    if (anIndex >= 4) return eFalse;
    _desc.mColorFormats[anIndex] = aFormat;
    return eTrue;
  }
  eGpuPipelineColorFormat __stdcall GetColorFormat(tU32 anIndex) const niImpl {
    return anIndex >= 4 ? eGpuPipelineColorFormat_None : _desc.mColorFormats[anIndex];
  }

  tBool __stdcall SetDepthFormat(eGpuPipelineDepthFormat aFormat) niImpl {
    _desc.mDepthFormat = aFormat;
    return eTrue;
  }
  eGpuPipelineDepthFormat __stdcall GetDepthFormat() const niImpl {
    return _desc.mDepthFormat;
  }

  tBool __stdcall SetFVF(tFVF aFVF) niImpl {
    _desc.mFVF = aFVF;
    return eTrue;
  }
  tFVF __stdcall GetFVF() const niImpl {
    return _desc.mFVF;
  }

  tBool __stdcall SetRasterizerStates(tIntPtr aHandle) niImpl {
    _desc.mhRS = aHandle;
    return eTrue;
  }
  tIntPtr __stdcall GetRasterizerStates() const niImpl {
    return _desc.mhRS;
  }

  tBool __stdcall SetDepthStencilStates(tIntPtr aHandle) niImpl {
    _desc.mhDS = aHandle;
    return eTrue;
  }
  tIntPtr __stdcall GetDepthStencilStates() const niImpl {
    return _desc.mhDS;
  }

  tBool __stdcall SetFunction(eGpuFunctionType aType, iGpuFunction* apFunc) niImpl {
    if (aType >= eGpuFunctionType_Last) return eFalse;
    _desc.mptrFuncs[aType] = apFunc;
    return eTrue;
  }
  iGpuFunction* __stdcall GetFunction(eGpuFunctionType aType) const niImpl {
    return aType >= eGpuFunctionType_Last ? nullptr : _desc.mptrFuncs[aType];
  }

  virtual tBool __stdcall SetBlendMode(iGpuBlendMode* apBlendMode) {
    _desc.mptrBlendMode = apBlendMode;
    return eTrue;
  }
  virtual iGpuBlendMode* __stdcall GetBlendMode() const {
    return _desc.mptrBlendMode;
  }

  tPtr __stdcall GetDescStructPtr() const niImpl {
    return (tPtr)&_desc;
  }
};

iGpuPipelineDesc* CreateGpuPipelineDesc() {
  return niNew sGpuPipelineDescImpl();
}
