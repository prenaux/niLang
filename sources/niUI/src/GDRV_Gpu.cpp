#include "stdafx.h"
#include "API/niUI_ModuleDef.h"
#include "GDRV_Gpu.h"
#include "GDRV_Utils.h"
#include <niLang/Utils/DataTableUtils.h>
#include <niLang/Utils/IDGenerator.h>
#include "Image.h"

namespace ni {

_HDecl(gpufunction);

/////////////////////////////////////////////////////////////////
#define GPU_TRACE(aFmt) //niDebugFmt(aFmt)

/////////////////////////////////////////////////////////////////
eGpuPixelFormat _GetClosestGpuPixelFormatForRT(const achar* aRTFormat) {
  // the default RT is using BGRA8, other RT will follow this format so we can
  // reuse those pipelines
  return eGpuPixelFormat_BGRA8;
}

/////////////////////////////////////////////////////////////////
eGpuPixelFormat _GetClosestGpuPixelFormatForDS(const achar* aDSFormat) {
  if (niStringIsOK(aDSFormat)) {
    return eGpuPixelFormat_D32;
  }
  return eGpuPixelFormat_None;
}

/////////////////////////////////////////////////////////////////
eGpuPixelFormat _GetClosestGpuPixelFormatForTexture(const achar* aTexFormat, tTextureFlags aTexFlags) {
  if (niFlagIs(aTexFlags,eTextureFlags_DepthStencil)) {
    return _GetClosestGpuPixelFormatForDS(aTexFormat);
  }
  else if (niFlagIs(aTexFlags,eTextureFlags_RenderTarget)) {
    return _GetClosestGpuPixelFormatForRT(aTexFormat);
  }

  // Depth formats
  if (ni::StrEq(aTexFormat,"D16")) {
    return eGpuPixelFormat_D16;
  }
  else if (ni::StrEq(aTexFormat,"D32")) {
    return eGpuPixelFormat_D32;
  }
  else if (ni::StrEq(aTexFormat,"D24S8")) {
    return eGpuPixelFormat_D24S8;
  }
  else if (ni::StrEq(aTexFormat,"D24X8")) {
    return eGpuPixelFormat_D32; // Fallback to D32
  }

  // Color formats
  if (ni::StrEq(aTexFormat,"R8G8B8A8")) {
    return eGpuPixelFormat_RGBA8;
  }
  else if (ni::StrEq(aTexFormat,"B8G8R8A8")) {
    return eGpuPixelFormat_BGRA8;
  }
  else if (ni::StrEq(aTexFormat,"FR16G16B16A16")) {
    return eGpuPixelFormat_RGBA16F;
  }
  else if (ni::StrEq(aTexFormat,"FR16")) {
    return eGpuPixelFormat_R16F;
  }
  else if (ni::StrEq(aTexFormat,"FR32")) {
    return eGpuPixelFormat_R32F;
  }

  // Default fallback
  return eGpuPixelFormat_RGBA8;
}

/////////////////////////////////////////////////////////////////
iPixelFormat* _GetIPixelFormat(iGraphics* apGraphics, eGpuPixelFormat aFormat) {
  static Ptr<iPixelFormat> _pixelFormats[eGpuPixelFormat_Last];
  if (!_pixelFormats[aFormat].IsOK()) {
    const achar* pxf = nullptr;
    switch (aFormat) {
      case eGpuPixelFormat_BGRA8: pxf = "B8G8R8A8"; break;
      case eGpuPixelFormat_RGBA8: pxf = "R8G8B8A8"; break;
      case eGpuPixelFormat_RGBA16F: pxf = "FR16G16B16A16"; break;
      case eGpuPixelFormat_R16F: pxf = "FR16"; break;
      case eGpuPixelFormat_R32F: pxf = "FR32"; break;
      case eGpuPixelFormat_D32: pxf = "D32"; break;
      case eGpuPixelFormat_D16: pxf = "D16"; break;
      case eGpuPixelFormat_D24S8: pxf = "D24S8"; break;
      case eGpuPixelFormat_None: return nullptr;
    }
    _pixelFormats[aFormat] = pxf ? apGraphics->CreatePixelFormat(pxf) : nullptr;
  }
  return _pixelFormats[aFormat];
}

/////////////////////////////////////////////////////////////////
#define GPU_BLENDMODE(OP,SRCRGB,SRCALPHA,DESTRGB,DESTALPHA) \
  { eGpuBlendOp_##OP, eGpuBlendFactor_##SRCRGB, eGpuBlendFactor_##SRCALPHA, eGpuBlendFactor_##DESTRGB, eGpuBlendFactor_##DESTALPHA }

/////////////////////////////////////////////////////////////////
const sGpuBlendModeDesc& _BlendModeToGpuBlendModeDesc(eBlendMode aBlendMode) {
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

/////////////////////////////////////////////////////////////////
struct sGpuBlendMode : public ImplRC<iGpuBlendMode> {
  sGpuBlendModeDesc _desc;

  sGpuBlendMode() {
    _desc.mOp = eGpuBlendOp_Add;
    _desc.mSrcRGB = eGpuBlendFactor_One;
    _desc.mSrcAlpha = eGpuBlendFactor_One;
    _desc.mDstRGB = eGpuBlendFactor_Zero;
    _desc.mDstAlpha = eGpuBlendFactor_Zero;
  }

  sGpuBlendMode(const sGpuBlendModeDesc& aDesc) {
    _desc = aDesc;
  }

  tBool __stdcall Copy(const iGpuBlendMode* apBlend) niImpl {
    niCheckIsOK(apBlend,eFalse);
    _desc = *(sGpuBlendModeDesc*)apBlend->GetDescStructPtr();
    return eTrue;
  }

  Ptr<iGpuBlendMode> __stdcall Clone() const niImpl {
    return MakeNN<sGpuBlendMode>(_desc);
  }

  tBool __stdcall SetOp(eGpuBlendOp aOp) niImpl {
    _desc.mOp = aOp;
    return eTrue;
  }
  eGpuBlendOp __stdcall GetOp() const niImpl {
    return _desc.mOp;
  }

  tBool __stdcall SetSrcRGB(eGpuBlendFactor aFactor) niImpl {
    _desc.mSrcRGB = aFactor;
    return eTrue;
  }
  eGpuBlendFactor __stdcall GetSrcRGB() const niImpl {
    return _desc.mSrcRGB;
  }

  tBool __stdcall SetSrcAlpha(eGpuBlendFactor aFactor) niImpl {
    _desc.mSrcAlpha = aFactor;
    return eTrue;
  }
  eGpuBlendFactor __stdcall GetSrcAlpha() const niImpl {
    return _desc.mSrcAlpha;
  }

  tBool __stdcall SetDstRGB(eGpuBlendFactor aFactor) niImpl {
    _desc.mDstRGB = aFactor;
    return eTrue;
  }
  eGpuBlendFactor __stdcall GetDstRGB() const niImpl {
    return _desc.mDstRGB;
  }

  tBool __stdcall SetDstAlpha(eGpuBlendFactor aFactor) niImpl {
    _desc.mDstAlpha = aFactor;
    return eTrue;
  }
  eGpuBlendFactor __stdcall GetDstAlpha() const niImpl {
    return _desc.mDstAlpha;
  }

  tPtr __stdcall GetDescStructPtr() const niImpl {
    return (tPtr)&_desc;
  }
};

/////////////////////////////////////////////////////////////////
NN<iGpuBlendMode> _CreateGpuBlendMode() {
  return MakeNN<sGpuBlendMode>();
}

/////////////////////////////////////////////////////////////////
struct sGpuPipelineDescImpl : public ImplRC<iGpuPipelineDesc> {
  sGpuPipelineDesc _desc;

  sGpuPipelineDescImpl() {
    _desc.mDepthFormat = eGpuPixelFormat_None;
    _desc.mFVF = 0;
    _desc.mhRS = 0;
    _desc.mhDS = 0;
    niCAssert(niCountOf(_desc.mColorFormats) == 4);
    niLoop(i,niCountOf(_desc.mColorFormats)) {
      _desc.mColorFormats[i] = eGpuPixelFormat_None;
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

  tBool __stdcall SetColorFormat(tU32 anIndex, eGpuPixelFormat aFormat) niImpl {
    if (anIndex >= 4) return eFalse;
    _desc.mColorFormats[anIndex] = aFormat;
    return eTrue;
  }
  eGpuPixelFormat __stdcall GetColorFormat(tU32 anIndex) const niImpl {
    return anIndex >= 4 ? eGpuPixelFormat_None : _desc.mColorFormats[anIndex];
  }

  tBool __stdcall SetDepthFormat(eGpuPixelFormat aFormat) niImpl {
    _desc.mDepthFormat = aFormat;
    return eTrue;
  }
  eGpuPixelFormat __stdcall GetDepthFormat() const niImpl {
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

  virtual tBool __stdcall SetBlendMode(iGpuBlendMode* apBlendMode) niImpl {
    _desc.mptrBlendMode = apBlendMode;
    return eTrue;
  }
  virtual iGpuBlendMode* __stdcall GetBlendMode() const niImpl {
    return _desc.mptrBlendMode;
  }

  tPtr __stdcall GetDescStructPtr() const niImpl {
    return (tPtr)&_desc;
  }
};

/////////////////////////////////////////////////////////////////
NN<iGpuPipelineDesc> _CreateGpuPipelineDesc() {
  return MakeNN<sGpuPipelineDescImpl>();
}

/////////////////////////////////////////////////////////////////
struct sFixedGpuPipelines : public ImplRC<iFixedGpuPipelines> {
  typedef sVertexPA tVertexClearRects;
  static_assert(sizeof(sVertexPA) == 16);

  typedef astl::hash_map<tFixedGpuPipelineId,Ptr<iGpuPipeline> > tPipelineMap;
  tPipelineMap _pipelines;
  NN<iGpuFunction> _vertFuncP = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _vertFuncPA = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _vertFuncPT1 = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _vertFuncPAT1 = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelFuncTex = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelFuncTexAlphaTest = niDeferredInit(NN<iGpuFunction>);
  NN<iTexture> _texWhite = niDeferredInit(NN<iTexture>);

  NN<iGpuPipeline> _pipelineClearColorDepth = niDeferredInit(NN<iGpuPipeline>);
  NN<iGpuPipeline> _pipelineClearColor = niDeferredInit(NN<iGpuPipeline>);
  NN<iGpuPipeline> _pipelineClearDepth = niDeferredInit(NN<iGpuPipeline>);

  tBool _CreateFixedGpuPipelines(iGraphicsDriver* apDriver) {
    iGraphics* g = apDriver->GetGraphics();
    NN<iGraphicsDriverGpu> gpuDriver = niCheckNN(gpuDriver,ni::QueryInterface<iGraphicsDriverGpu>(apDriver),eFalse);

    {
      _texWhite = niCheckNN(
        _texWhite,
        g->CreateTextureFromRes(_H("niUI://white1x1.tga"),nullptr,eTextureFlags_Default),
        eFalse);
    }

#define LOAD_FIXED_GPUFUNC(TYPE,VAR,PATH) {                             \
      tHStringPtr hspPath = _H("niUI://gpufunc/fixed_" #PATH ".gpufunc.xml"); \
      VAR = niCheckNN(                                                  \
        VAR, gpuDriver->CreateGpuFunction(eGpuFunctionType_##TYPE,hspPath), \
        eFalse);                                                        \
    }

    LOAD_FIXED_GPUFUNC(Vertex, _vertFuncP, p_vs);
    LOAD_FIXED_GPUFUNC(Vertex, _vertFuncPA, pa_vs);
    LOAD_FIXED_GPUFUNC(Vertex, _vertFuncPT1, pt1_vs);
    LOAD_FIXED_GPUFUNC(Vertex, _vertFuncPAT1, pat1_vs);
    LOAD_FIXED_GPUFUNC(Pixel, _pixelFuncTex, tex_ps);
    LOAD_FIXED_GPUFUNC(Pixel, _pixelFuncTexAlphaTest, tex_alphatest_ps);

#undef LOAD_FIXED_GPUFUNC

    {
      tHStringPtr hspVfPath = _H("niUI://gpufunc/clear_vs.gpufunc.xml");
      niLet dtVertex = niCheckNN(dtVertex,LoadDataTable(niHStr(hspVfPath)),eFalse);
      niLet vertexGpuFun = niCheckNN(vertexGpuFun, gpuDriver->CreateGpuFunction(
        eGpuFunctionType_Vertex,hspVfPath),eFalse);

      tHStringPtr hspPfPath = _H("niUI://gpufunc/clear_ps.gpufunc.xml");
      niLet dtPixel = niCheckNN(dtPixel,LoadDataTable(niHStr(hspPfPath)),eFalse);
      niLet pixelGpuFun = niCheckNN(pixelGpuFun, gpuDriver->CreateGpuFunction(
        eGpuFunctionType_Pixel,hspPfPath),eFalse);

      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, gpuDriver->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexClearRects::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,pixelGpuFun);

      pipelineDesc->SetRasterizerStates(eCompiledStates_RS_NoCullingFilled);
      pipelineDesc->SetDepthStencilStates(eCompiledStates_DS_DepthWriteOnly);
      _pipelineClearColorDepth = niCheckNN(_pipelineClearColorDepth, gpuDriver->CreateGpuPipeline(_H("GpuTriangle_Pipeline"),pipelineDesc), eFalse);

      pipelineDesc->SetRasterizerStates(eCompiledStates_RS_NoCullingFilled);
      pipelineDesc->SetDepthStencilStates(eCompiledStates_DS_NoDepthTest);
      _pipelineClearColor = niCheckNN(_pipelineClearColor, gpuDriver->CreateGpuPipeline(_H("GpuTriangle_Pipeline"),pipelineDesc), eFalse);

      pipelineDesc->SetRasterizerStates(eCompiledStates_RS_NoCullingColorWriteNone);
      pipelineDesc->SetDepthStencilStates(eCompiledStates_DS_DepthWriteOnly);
      _pipelineClearDepth = niCheckNN(_pipelineClearDepth, gpuDriver->CreateGpuPipeline(_H("GpuTriangle_Pipeline"),pipelineDesc), eFalse);
    }

    return eTrue;
  }

  Ptr<iGpuPipeline> __stdcall GetRenderPipeline(iGraphicsDriverGpu* apGpuDriver, tFixedGpuPipelineId aPipelineId, iGpuFunction* apFuncVertex, iGpuFunction* apFuncPixel) niImpl {
    tPipelineMap::iterator it = _pipelines.find(aPipelineId);
    if (it == _pipelines.end()) {
      Ptr<iGpuPipeline> pipeline = CreateFixedGpuPipeline(
        apGpuDriver,aPipelineId,apFuncVertex,apFuncPixel);
      if (!pipeline.IsOK()) {
        // niDebugFmt(("VS %s, PS %s",vs->GetCode(),ps->GetCode()));
        niPanicUnreachable("Can't create gpu pipeline.");
        return nullptr;
      }
      it = _pipelines.insert(astl::make_pair(aPipelineId,pipeline)).first;
    }
    GPU_TRACE((">>> sFixedGpuPipelines::GetRenderPipeline: %s.",((sFixedGpuPipelineId&)aPipelineId).ToString()));
    return it->second;
  }

  iGpuFunction* __stdcall GetFixedGpuFuncVertex(ain<tFVF> aFVF) const niImpl {
    niUnused(aFVF);
    if (aFVF & eFVF_ColorA) {
      if (aFVF & eFVF_Tex1) {
        return _vertFuncPAT1;
      }
      else {
        return _vertFuncPA;
      }
    }
    else {
      if (aFVF & eFVF_Tex1) {
        return _vertFuncPT1;
      }
      else {
        return _vertFuncP;
      }
    }
  }

  iGpuFunction* __stdcall GetFixedGpuFuncPixel(ain<sMaterialDesc> aMatDesc) const niImpl {
    if (aMatDesc.mFlags & eMaterialFlags_Transparent) {
      return _pixelFuncTexAlphaTest;
    }
    else {
      return _pixelFuncTex;
    }
  }

  nn<iTexture> __stdcall GetWhiteTexture() const niImpl {
    return _texWhite;
  }

  // Convert from screen coordinates (pixels) to normalized device coordinates (NDC)
  // Screen space: (0,0) is top-left, (width,height) is bottom-right
  // Clip space: (-1,-1) is bottom-left, (1,1) is top-right, z is unchanged
  __forceinline sVec3f ScreenToClipSpace(ain<sVec2f> aPixelSize, tF32 x, tF32 y, tF32 z) {
    return Vec3f(
      (x * aPixelSize.x) - 1.0f,    // Map [0,width] to [-1,1]
      1.0f - (y * aPixelSize.y),    // Map [0,height] to [1,-1] (flip Y)
      z                             // Keep Z as-is
    );
  }

  tBool __stdcall ClearRect(iGpuCommandEncoder* apCmdEncoder, ain<sVec2f> aPixelSize, tClearBuffersFlags aFlags, ain<sRectf> aRect, tU32 anColor, tF32 afZ) niImpl {
    if (aFlags == eClearBuffersFlags_Depth) {
      apCmdEncoder->SetPipeline(_pipelineClearDepth);
    }
    else if (aFlags == eClearBuffersFlags_Color) {
      apCmdEncoder->SetPipeline(_pipelineClearColor);
    }
    else if (aFlags == eClearBuffersFlags_ColorDepth ||
             aFlags == eClearBuffersFlags_ColorDepthStencil) {
      apCmdEncoder->SetPipeline(_pipelineClearColorDepth);
    }
    else {
      return eFalse;
    }

    niLet posTL = aRect.GetTopLeft();
    niLet posBR = aRect.GetBottomRight();
    tVertexClearRects verts[4] = {
      {ScreenToClipSpace(aPixelSize,posTL.x,posTL.y,afZ), anColor}, // TL
      {ScreenToClipSpace(aPixelSize,posBR.x,posTL.y,afZ), anColor}, // TR
      {ScreenToClipSpace(aPixelSize,posTL.x,posBR.y,afZ), anColor}, // BL
      {ScreenToClipSpace(aPixelSize,posBR.x,posBR.y,afZ), anColor}, // BR
    };
    apCmdEncoder->StreamVertexBuffer((tPtr)verts,sizeof(verts),0);
    apCmdEncoder->Draw(eGraphicsPrimitiveType_TriangleStrip,4,0);
    return eTrue;
  }
};

/////////////////////////////////////////////////////////////////
Ptr<iFixedGpuPipelines> CreateFixedGpuPipelines(iGraphicsDriver* apGpuDriver) {
  NN<sFixedGpuPipelines> fixedPipelines = ni::MakeNN<sFixedGpuPipelines>();
  niCheck(
    fixedPipelines->_CreateFixedGpuPipelines(apGpuDriver),
    nullptr);
  return fixedPipelines;
}

/////////////////////////////////////////////////////////////////
struct sFixedGpuVertexArray : public ni::ImplRC<iVertexArray> {
  NN<iGpuBuffer> _buffer;
  tFVF _fvf;
  tU32 _fvfStride;
  const eArrayUsage _arrayUsage;

  sFixedGpuVertexArray(iGpuBuffer* apGpuBuffer, tFVF aFVF, eArrayUsage aUsage)
      : _arrayUsage(aUsage)
      , _buffer(apGpuBuffer)
  {
    _fvf = aFVF;
    _fvfStride = FVFGetStride(_fvf);
    GPU_TRACE((
      ">>> sFixedGpuVertexArray: FVF:%s, NumVertex: %d, Stride: %d, Size: %db (%gMB).",
      FVFToString(_fvf).Chars(),
      this->GetNumVertices(),_fvfStride,
      _fvfStride * anNumVertices,
      ((tF64)(_fvfStride * anNumVertices))/(1024.0*1024.0)));
  }

  virtual tBool __stdcall IsOK() const niImpl {
    return _buffer->IsOK();
  }

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return NULL;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    return _buffer->Bind(apDevice);
  }

  virtual tFVF __stdcall GetFVF() const niImpl {
    return _fvf;
  }
  virtual tU32 __stdcall GetNumVertices() const niImpl {
    return _buffer->GetSize() / _fvfStride;
  }
  virtual eArrayUsage __stdcall GetUsage() const niImpl {
    return _arrayUsage;
  }
  virtual tPtr __stdcall Lock(tU32 anFirstVertex, tU32 anNumVertex, eLock aLock) niImpl {
    niUnused(aLock);
    return _buffer->Lock(anFirstVertex * _fvfStride, anNumVertex * _fvfStride, aLock);
  }
  virtual tBool __stdcall Unlock() niImpl {
    return _buffer->Unlock();
  }
  virtual tBool __stdcall GetIsLocked() const niImpl {
    return _buffer->GetIsLocked();
  }
};

/////////////////////////////////////////////////////////////////
iGpuBuffer* GetVertexArrayGpuBuffer(iVertexArray* apVA) {
  return ((sFixedGpuVertexArray*)apVA)->_buffer;
}

/////////////////////////////////////////////////////////////////
sVec2i GetVertexArrayFvfAndStride(iVertexArray* apVA) {
  niLet va = (sFixedGpuVertexArray*)apVA;
  return Vec2i((tI32)va->_fvf,(tI32)va->_fvfStride);
}

/////////////////////////////////////////////////////////////////
struct sFixedGpuIndexArray : public ni::ImplRC<iIndexArray> {
  NN<iGpuBuffer> _buffer;
  eGraphicsPrimitiveType _primType;
  const eArrayUsage _arrayUsage;

  sFixedGpuIndexArray(iGpuBuffer* apGpuBuffer, eGraphicsPrimitiveType aPrimType, eArrayUsage aUsage)
      : _arrayUsage(aUsage)
      , _primType(aPrimType)
      , _buffer(apGpuBuffer)
  {
    GPU_TRACE((
      ">>> sFixedGpuIndexArray: PT: %s, MaxVertexIndex:%d, NumIndices: %d, Stride: %d, Size: %db (%gMB).",
      niEnumToChars(eGraphicsPrimitiveType,_primType),
      0xFFFFFFFF,
      this->GetNumIndices(), knFixedGpuIndexSize,
      knFixedGpuIndexSize * anNumIndices,
      ((tF64)(knFixedGpuIndexSize * anNumIndices))/(1024.0*1024.0)));
  }

  virtual tBool __stdcall IsOK() const niImpl {
    return _buffer->IsOK();
  }

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return NULL;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) niImpl {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() niImpl {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) niImpl {
    return _buffer->Bind(apDevice);
  }

  virtual eGraphicsPrimitiveType __stdcall GetPrimitiveType() const niImpl {
    return _primType;
  }
  virtual tU32 __stdcall GetNumIndices() const niImpl {
    return _buffer->GetSize() / knFixedGpuIndexSize;
  }
  virtual tU32 __stdcall GetMaxVertexIndex() const niImpl {
    return 0xFFFFFFFF;
  }
  virtual eArrayUsage __stdcall GetUsage() const niImpl {
    return _arrayUsage;
  }

  virtual tPtr __stdcall Lock(tU32 anFirstIndex, tU32 anNumIndex, eLock aLock) niImpl {
    return _buffer->Lock(anFirstIndex * knFixedGpuIndexSize, anNumIndex * knFixedGpuIndexSize, aLock);
  }
  virtual tBool __stdcall Unlock() niImpl {
    return _buffer->Unlock();
  }
  virtual tBool __stdcall GetIsLocked() const niImpl {
    return _buffer->GetIsLocked();
  }
};

/////////////////////////////////////////////////////////////////
iGpuBuffer* GetIndexArrayGpuBuffer(iIndexArray* apVA) {
  return ((sFixedGpuIndexArray*)apVA)->_buffer;
}

/////////////////////////////////////////////////////////////////
iVertexArray* CreateFixedGpuVertexArray(iGraphicsDriverGpu* apGpuDriver, tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) {
    niLet fvfStride = FVFGetStride(anFVF);
    Ptr<iGpuBuffer> vaBuffer = apGpuDriver->CreateGpuBuffer(
      nullptr,
      fvfStride * anNumVertices,
      eGpuBufferMemoryMode_Shared,
      eGpuBufferUsageFlags_Vertex);
    niCheckIsOK(vaBuffer,nullptr);
    return niNew sFixedGpuVertexArray(vaBuffer, anFVF, aUsage);
  }

iIndexArray* CreateFixedGpuIndexArray(iGraphicsDriverGpu* apGpuDriver, eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndices, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
  niUnused(anMaxVertexIndex);
  Ptr<iGpuBuffer> iaBuffer = apGpuDriver->CreateGpuBuffer(
    nullptr,
    knFixedGpuIndexSize * anNumIndices,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Index);
  return niNew sFixedGpuIndexArray(iaBuffer, aPrimitiveType, aUsage);
}

/////////////////////////////////////////////////////////////////
static Ptr<iDataTable> GpuFunctionDT_FindTarget(iDataTable* apDT, const iHString* ahspTarget) {
  if (!apDT)
    return nullptr;

  niLet dtName = apDT->GetName();
  if (!StrEq(dtName,"Shader") &&
      !StrEq(dtName,"Target") &&
      !StrEq(dtName,"GpuFunction") &&
      !StrEq(dtName,"GpuLibrary"))
    return nullptr;

  tHStringPtr hspTarget = apDT->GetHString("target");
  //niDebugFmt(("... FindGpuFunctionDT: %s, %s", apDT->GetName(), hspTarget));
  if (hspTarget == ahspTarget) {
    return apDT;
  }
  niLoop(i,apDT->GetNumChildren()) {
    iDataTable* foundDT = GpuFunctionDT_FindTarget(apDT->GetChildFromIndex(i), ahspTarget);
    if (foundDT)
      return foundDT;
  }
  return nullptr;
}

/////////////////////////////////////////////////////////////////
Ptr<iDataTable> GpuFunctionDT_Load(const achar* aURL, iHString* ahspTarget, eGpuFunctionBindType* apOutBindType) {
  niLet dtRoot = niCheckNN_(
    dtRoot,LoadDataTable(aURL),
    niFmt("Can't load gpufunc datatable from '%s'.",aURL),
    nullptr);

  if (apOutBindType) {
    *apOutBindType = eGpuFunctionBindType_None;
    niLet dtReflection = niCheckNN(dtReflection,dtRoot->GetChild("Reflection"),nullptr);
    niLoop(i,dtReflection->GetNumChildren()) {
      niLet dtChild = dtReflection->GetChildFromIndex(i);
      niLet name = dtChild->GetName();
      if ((StrEq(name,"SeparateImages") ||
           StrEq(name,"UBOs")) &&
          (dtChild->GetNumChildren() > 0)) {
        *apOutBindType = eGpuFunctionBindType_Fixed;
        break;
      }
    }
    niDebugFmt((
      "... GpuFunctionDT_Load: url: %s, target: %s, bindType: %d",
      aURL, ahspTarget, niEnumToChars(eGpuFunctionBindType,*apOutBindType)));
  }

  niLet dtTarget = niCheckNN_(
    dtTarget,GpuFunctionDT_FindTarget(dtRoot,ahspTarget),
    niFmt("Can't find gpufunc target '%s' datatable from '%s'.",ahspTarget,aURL),
    nullptr);

  niLet dataPI = dtTarget->GetPropertyIndex("_data");
  if (dataPI == eInvalidHandle) {
    niLet codePath = dtTarget->GetString("code_path");
    if (!codePath.empty()) {
      niLet foundCodePath = ni::GetLang()->URLFindFilePath(codePath.c_str(),aURL,nullptr);
      niLet fpCode = niCheckNN_(
        dtTarget,ni::GetLang()->URLOpen(foundCodePath.c_str()),
        niFmt("Can't load gpufunc target '%s' code from '%s' in datatable '%s'.",ahspTarget,codePath,aURL),
        nullptr);
      dtTarget->SetIUnknown("code_fp",fpCode);
    }
    else {
      niError(niFmt("Can't find gpufunc target '%s' code in datatable '%s'.",ahspTarget,aURL));
      return nullptr;
    }
  }

  return dtTarget;
}

/////////////////////////////////////////////////////////////////
cString GpuFunctionDT_GetSourceText(ain<nn<iDataTable>> aDT) {
  if (aDT->HasProperty("_data")) {
    return aDT->GetString("_data");
  }
  else if (aDT->HasProperty("code_fp")) {
    NN<iFile> fpCode = niCheckNN(fpCode,QueryInterface<iFile>(aDT->GetIUnknown("code_fp")),AZEROSTR);
    return fpCode->ReadString();
  }
  niError("No source text in gpufunction datatable.");
  return AZEROSTR;
}

/////////////////////////////////////////////////////////////////
Ptr<iFile> GpuFunctionDT_GetSourceData(ain<nn<iDataTable>> aDT) {
  if (aDT->HasProperty("code_fp")) {
    NN<iFile> fpCode = niCheckNN(fpCode,QueryInterface<iFile>(aDT->GetIUnknown("code_fp")),nullptr);
    return fpCode;
  }
  niError("No source binary data in gpufunction datatable.");
  return nullptr;
}



static tU32 _VertexFormatToFixedGpuVertexFormat(tFVF aFVF) {
  switch (aFVF) {
    case eVertexFormat_P: return eFixedGpuVertexFormat_P;
    case eVertexFormat_PA: return eFixedGpuVertexFormat_PA;
    case eVertexFormat_PAT1: return eFixedGpuVertexFormat_PAT1;
    case eVertexFormat_PN: return eFixedGpuVertexFormat_PN;
    case eVertexFormat_PNA: return eFixedGpuVertexFormat_PNA;
    case eVertexFormat_PNT1: return eFixedGpuVertexFormat_PNT1;
    case eVertexFormat_PNAT1: return eFixedGpuVertexFormat_PNAT1;
    case eVertexFormat_PNT2: return eFixedGpuVertexFormat_PNT2;
    case eVertexFormat_PNAT2: return eFixedGpuVertexFormat_PNAT2;
    case eVertexFormat_PB4INT1: return eFixedGpuVertexFormat_PB4INT1;
    case eVertexFormat_PB4INAT1: return eFixedGpuVertexFormat_PB4INAT1;
    default: return eInvalidHandle;
  }
}

static tU32 _CompiledStatesRSToFixedGpuRS(eCompiledStates aRS) {
  switch (aRS) {
    case eCompiledStates_RS_ColorWriteNone: return eFixedGpuRS_ColorWriteNone;
    case eCompiledStates_RS_Filled: return eFixedGpuRS_Filled;
    case eCompiledStates_RS_Wireframe: return eFixedGpuRS_Wireframe;
    case eCompiledStates_RS_NoCullingColorWriteNone: return eFixedGpuRS_NoCullingColorWriteNone;
    case eCompiledStates_RS_NoCullingFilled: return eFixedGpuRS_NoCullingFilled;
    case eCompiledStates_RS_NoCullingWireframe: return eFixedGpuRS_NoCullingWireframe;
    case eCompiledStates_RS_CWCullingColorWriteNone: return eFixedGpuRS_CWCullingColorWriteNone;
    case eCompiledStates_RS_CWCullingFilled: return eFixedGpuRS_CWCullingFilled;
    case eCompiledStates_RS_CWCullingWireframe: return eFixedGpuRS_CWCullingWireframe;
    default: return eInvalidHandle;
  }
}

static tU32 _CompiledStatesDSToFixedGpuDS(eCompiledStates aDS) {
  switch (aDS) {
    case eCompiledStates_DS_NoDepthTest: return eFixedGpuDS_NoDepthTest;
    case eCompiledStates_DS_DepthTestAndWrite: return eFixedGpuDS_DepthTestAndWrite;
    case eCompiledStates_DS_DepthTestOnly: return eFixedGpuDS_DepthTestOnly;
    default: return eInvalidHandle;
  }
}

static tFVF _FixedGpuVertexFormatToVertexFormat(tU32 aVF) {
  switch (aVF) {
    case eFixedGpuVertexFormat_P: return eVertexFormat_P;
    case eFixedGpuVertexFormat_PA: return eVertexFormat_PA;
    case eFixedGpuVertexFormat_PAT1: return eVertexFormat_PAT1;
    case eFixedGpuVertexFormat_PN: return eVertexFormat_PN;
    case eFixedGpuVertexFormat_PNA: return eVertexFormat_PNA;
    case eFixedGpuVertexFormat_PNT1: return eVertexFormat_PNT1;
    case eFixedGpuVertexFormat_PNAT1: return eVertexFormat_PNAT1;
    case eFixedGpuVertexFormat_PNT2: return eVertexFormat_PNT2;
    case eFixedGpuVertexFormat_PNAT2: return eVertexFormat_PNAT2;
    case eFixedGpuVertexFormat_PB4INT1: return eVertexFormat_PB4INT1;
    case eFixedGpuVertexFormat_PB4INAT1: return eVertexFormat_PB4INAT1;
  }
  niPanicUnreachable(niFmt("Unknown fixed gpu vertex format '%d'.", (tU32)aVF));
  return 0;
}

static eCompiledStates _FixedGpuRSToCompiledStatesRS(tU32 aRS) {
  switch (aRS) {
    case eFixedGpuRS_ColorWriteNone: return eCompiledStates_RS_ColorWriteNone;
    case eFixedGpuRS_Filled: return eCompiledStates_RS_Filled;
    case eFixedGpuRS_Wireframe: return eCompiledStates_RS_Wireframe;
    case eFixedGpuRS_NoCullingColorWriteNone: return eCompiledStates_RS_NoCullingColorWriteNone;
    case eFixedGpuRS_NoCullingFilled: return eCompiledStates_RS_NoCullingFilled;
    case eFixedGpuRS_NoCullingWireframe: return eCompiledStates_RS_NoCullingWireframe;
    case eFixedGpuRS_CWCullingColorWriteNone: return eCompiledStates_RS_CWCullingColorWriteNone;
    case eFixedGpuRS_CWCullingFilled: return eCompiledStates_RS_CWCullingFilled;
    case eFixedGpuRS_CWCullingWireframe: return eCompiledStates_RS_CWCullingWireframe;
  }
  niPanicUnreachable(niFmt("Unknown fixed gpu RS '%d'.", (tU32)aRS));
  return eCompiledStates_Invalid;
}

static eCompiledStates _FixedGpuDSToCompiledStatesDS(tU32 aDS) {
  switch (aDS) {
    case eFixedGpuDS_NoDepthTest: return eCompiledStates_DS_NoDepthTest;
    case eFixedGpuDS_DepthTestAndWrite: return eCompiledStates_DS_DepthTestAndWrite;
    case eFixedGpuDS_DepthTestOnly: return eCompiledStates_DS_DepthTestOnly;
    case eFixedGpuDS_DepthWriteOnly: return eCompiledStates_DS_DepthWriteOnly;
  }
  niPanicUnreachable(niFmt("Unknown fixed gpu DS '%d'.", (tU32)aDS));
  return eCompiledStates_Invalid;
}

tFixedGpuPipelineId GetFixedGpuPipelineId(
  eGpuPixelFormat aRT0Format,
  eGpuPixelFormat aDSFormat,
  tFVF aFVF,
  eBlendMode aBlendMode,
  eCompiledStates aRS,
  eCompiledStates aDS,
  const iGpuFunction* apFuncVertex,
  const iGpuFunction* apFuncPixel)
{
  sFixedGpuPipelineIdDesc id = {};

  id.rt0 = aRT0Format;
  id.ds = aDSFormat;

  {
    niLet v = _VertexFormatToFixedGpuVertexFormat(aFVF);
    if (v == eInvalidHandle) {
      niError(niFmt("Invalid vertex format: %s (%x).", FVFToString(aFVF), aFVF));
      return 0;
    }
    id.vertexFormat = (eFixedGpuVertexFormat)v;
  }

  {
    niLet v = _CompiledStatesRSToFixedGpuRS(aRS);
    if (v == eInvalidHandle) {
      niError(niFmt("Invalid rasterizer state: %d.", aRS));
      return 0;
    }
    id.compiledRS = (eFixedGpuRS)v;
  }

  {
    niLet v = _CompiledStatesDSToFixedGpuDS(aDS);
    if (id.compiledDS == eInvalidHandle) {
      niError(niFmt("Invalid depth state: %d.", aDS));
      return 0;
    }
    id.compiledDS = (eFixedGpuDS)v;
  }

  id.blendMode = aBlendMode;

  niCheckIsOK(apFuncVertex,0);
  {
    niLet v = apFuncVertex->GetFunctionId();
    if (v >= knFixedGpuMaxFuncId) {
      niError(niFmt("Invalid vertex func id: %d.", v));
      return 0;
    }
    id.funcVertex = v;
  }

  niCheckIsOK(apFuncPixel,0);
  {
    niLet v = apFuncPixel->GetFunctionId();
    if (v >= knFixedGpuMaxFuncId) {
      niError(niFmt("Invalid pixel func id: %d.", v));
      return 0;
    }
    id.funcPixel = v;
  }

  return id.ToId();
}

Ptr<iGpuPipeline> CreateFixedGpuPipeline(
  iGraphicsDriverGpu* apGpuDriver,
  tFixedGpuPipelineId aPipelineId,
  iGpuFunction* apFuncVertex,
  iGpuFunction* apFuncPixel)
{
  niCheck(apFuncVertex != nullptr,nullptr);
  niCheck(apFuncVertex->GetFunctionType() == eGpuFunctionType_Vertex,nullptr);
  niCheck(apFuncPixel != nullptr,nullptr);
  niCheck(apFuncPixel->GetFunctionType() == eGpuFunctionType_Pixel,nullptr);

  sFixedGpuPipelineIdDesc idDesc = sFixedGpuPipelineIdDesc::FromId(aPipelineId);

  NN<iGpuPipelineDesc> ptrPipelineDesc{apGpuDriver->CreateGpuPipelineDesc()};
  sGpuPipelineDesc& desc = *(sGpuPipelineDesc*)ptrPipelineDesc->GetDescStructPtr();
  desc.mFVF = _FixedGpuVertexFormatToVertexFormat(idDesc.vertexFormat);
  desc.mColorFormats[0] = idDesc.GetRT0PixelFormat();
  desc.mColorFormats[1] = eGpuPixelFormat_None;
  desc.mColorFormats[2] = eGpuPixelFormat_None;
  desc.mColorFormats[3] = eGpuPixelFormat_None;
  desc.mDepthFormat = idDesc.GetDSPixelFormat();
  desc.mhRS = _FixedGpuRSToCompiledStatesRS(idDesc.compiledRS);
  desc.mhDS = _FixedGpuDSToCompiledStatesDS(idDesc.compiledDS);
  desc.mptrFuncs[eGpuFunctionType_Vertex] = apFuncVertex;
  desc.mptrFuncs[eGpuFunctionType_Pixel] = apFuncPixel;
  if (idDesc.blendMode == eBlendMode_NoBlending) {
    desc.mptrBlendMode = nullptr;
  }
  else if (idDesc.blendMode < eBlendMode_Last) {
    const sGpuBlendModeDesc& bm = _BlendModeToGpuBlendModeDesc((eBlendMode)idDesc.blendMode);
    desc.mptrBlendMode = apGpuDriver->CreateGpuBlendMode();
    *((sGpuBlendModeDesc*)desc.mptrBlendMode->GetDescStructPtr()) = bm;
  }
  else {
    desc.mptrBlendMode = nullptr;
    niWarning("Invalid blend mode, falling back to 'no blending'.");
  }
  return apGpuDriver->CreateGpuPipeline(
    HFmt("sFixedGpuPipelines{%s}",idDesc.ToString()),
    ptrPipelineDesc);
}

/////////////////////////////////////////////////////////////////
// Submits the draw call for the specified draw operation.
tBool DrawOperationSubmitGpuDrawCall(
  iGpuCommandEncoder* apCmdEncoder,
  iDrawOperation* apDrawOp)
{
  iVertexArray* va = apDrawOp->GetVertexArray();
  iIndexArray* ia = apDrawOp->GetIndexArray();
  const sVec2i fvfAndStride = GetVertexArrayFvfAndStride(va);
  //const tFVF fvf = (tFVF)fvfAndStride.x;
  const tU32 fvfStride = (tU32)fvfAndStride.y;

  const tU32 baseVertexIndex = apDrawOp->GetBaseVertexIndex();
  iGpuBuffer* vaBuffer = GetVertexArrayGpuBuffer(va);
  apCmdEncoder->SetVertexBuffer(vaBuffer, baseVertexIndex * fvfStride, 0);

  if (ia) {
    const tU32 firstInd = apDrawOp->GetFirstIndex();
    tU32 numInds = apDrawOp->GetNumIndices();
    if (!numInds) {
      numInds = ia->GetNumIndices()-firstInd;
    }
    iGpuBuffer* iaBuffer = GetIndexArrayGpuBuffer(ia);
    apCmdEncoder->SetIndexBuffer(iaBuffer, firstInd * knFixedGpuIndexSize, eGpuIndexType_U32);
    apCmdEncoder->DrawIndexed(apDrawOp->GetPrimitiveType(), numInds, 0);
  }
  else {
    const tU32 nNumVerts = va->GetNumVertices()-baseVertexIndex;
    apCmdEncoder->Draw(apDrawOp->GetPrimitiveType(), nNumVerts, baseVertexIndex);
  }

  return eTrue;
}

/////////////////////////////////////////////////////////////////
struct sGpuStream : public ImplRC<iGpuStream> {
  const tU32 _chunkSize;
  const tU32 _maxChunks;
  const tU32 _blockAlignment;
  const tGpuBufferUsageFlags _usageFlags;
  NN<iGraphicsDriverGpu> _driver;
  astl::vector<NN<iGpuBuffer>> _chunks;
  tU32 _currentChunk = 0;
  tU32 _currentOffset = 0;
  sGpuStreamBlock _lastBlock;
  tU32 _numBlocks = 0;

  sGpuStream(ain<nn<iGraphicsDriverGpu>> apDriver,
             tGpuBufferUsageFlags aUsageFlags,
             tU32 aBlockAlignment,
             tU32 aChunkSize,
             tU32 aMaxChunks)
      : _chunkSize(aChunkSize)
      , _maxChunks(aMaxChunks)
      , _blockAlignment(aBlockAlignment)
      , _usageFlags(aUsageFlags)
      , _driver(apDriver)
  {
    _chunks.reserve(_maxChunks);
  }

  tBool _AllocateNextChunk() {
    niLet chunk = niCheckNN(chunk, _driver->CreateGpuBuffer(
      nullptr, _chunkSize,
      eGpuBufferMemoryMode_Shared, _usageFlags),eFalse);
    _chunks.push_back(chunk);
    return eTrue;
  }

  tBool __stdcall UpdateNextBlock(ain<tPtr> apData, ain<tU32> anSize) niImpl {
    if (anSize > _chunkSize) {
      niError("Data size larger than chunk size");
      return eFalse;
    }

    if (_chunks.empty() || (_currentOffset + anSize > _chunkSize)) {
      if (_chunks.size() >= _maxChunks) {
        niError("Maximum chunks reached");
        return eFalse;
      }

      niCheck(_AllocateNextChunk(),eFalse);
      _currentChunk = (tU32)_chunks.size() - 1;
      _currentOffset = 0;
    }

    niLet& chunk = _chunks[_currentChunk];
    niLet dst = chunk->Lock(_currentOffset, anSize, eLock_Discard);
    if (!dst) {
      niError("Can't lock chunk");
      return eFalse;
    }

    memcpy(dst, apData, anSize);
    chunk->Unlock();

    _lastBlock.mBuffer = chunk;
    _lastBlock.mOffset = _currentOffset;
    _lastBlock.mSize = anSize;
    _lastBlock.mAlignedSize = _blockAlignment ?
        ((anSize + _blockAlignment - 1) & ~(_blockAlignment - 1)) :
        anSize;
    ++_numBlocks;

    _currentOffset += _lastBlock.mAlignedSize;
    return eTrue;
  }

  const sGpuStreamBlock& __stdcall GetLastBlock() const niImpl {
    return _lastBlock;
  }

  iGpuBuffer* __stdcall GetLastBuffer() const niImpl {
    return _lastBlock.mBuffer;
  }
  tU32 __stdcall GetLastOffset() const niImpl {
    return _lastBlock.mOffset;
  }
  tU32 __stdcall GetLastSize() const niImpl {
    return _lastBlock.mSize;
  }

  tU32 __stdcall GetNumBlocks() const niImpl {
    return _numBlocks;
  }

  void __stdcall Reset() niImpl {
    _currentChunk = 0;
    _currentOffset = 0;
    _numBlocks = 0;
  }
};

/////////////////////////////////////////////////////////////////
Ptr<iGpuStream> CreateGpuStream(
  ain<nn<iGraphicsDriverGpu>> apDriver,
  tGpuBufferUsageFlags aUsageFlags,
  tU32 aBlockAlignment,
  tU32 aChunkSize,
  tU32 aMaxChunks)
{
  return niNew sGpuStream(apDriver,aUsageFlags,aBlockAlignment,aChunkSize,aMaxChunks);
}

/////////////////////////////////////////////////////////////////
tBool UpdateGpuStreamToVertexBuffer(iGpuStream* apStream, iGpuCommandEncoder* apEncoder, const tPtr apData, tU32 anSize, tU32 anBinding) {
  niCheck(apStream->UpdateNextBlock(apData, anSize), eFalse);
  niLet& block = apStream->GetLastBlock();
  apEncoder->SetVertexBuffer(block.mBuffer, block.mOffset, anBinding);
  return eTrue;
}

tBool UpdateGpuStreamToIndexBuffer(iGpuStream* apStream, iGpuCommandEncoder* apEncoder, const tPtr apData, tU32 anSize, eGpuIndexType aIndexType) {
  niCheck(apStream->UpdateNextBlock(apData, anSize), eFalse);
  niLet& block = apStream->GetLastBlock();
  apEncoder->SetIndexBuffer(block.mBuffer, block.mOffset, aIndexType);
  return eTrue;
}

tBool UpdateGpuStreamToUniformBuffer(iGpuStream* apStream, iGpuCommandEncoder* apEncoder, const tPtr apData, tU32 anSize, tU32 anBinding) {
  niCheck(apStream->UpdateNextBlock(apData, anSize), eFalse);
  niLet& block = apStream->GetLastBlock();
  apEncoder->SetUniformBuffer(block.mBuffer, block.mOffset, anBinding);
  return eTrue;
}

/////////////////////////////////////////////////////////////////
nn<const sRasterizerStatesDesc> GetGpuRasterizerDesc(nn<iGraphics> aGraphics, tIntPtr ahRS) {
  iRasterizerStates* pRS = aGraphics->GetCompiledRasterizerStates(ahRS);
  if (!pRS) {
    pRS = aGraphics->GetCompiledRasterizerStates(eCompiledStates_RS_NoCullingFilled);
  }
  return as_nn((const sRasterizerStatesDesc*)pRS->GetDescStructPtr());
}

nn<const sDepthStencilStatesDesc> GetGpuDepthStencilDesc(nn<iGraphics> aGraphics, tIntPtr ahDS) {
  iDepthStencilStates* pDS = aGraphics->GetCompiledDepthStencilStates(ahDS);
  if (!pDS) {
    pDS = aGraphics->GetCompiledDepthStencilStates(
      eCompiledStates_DS_NoDepthTest);
  }
  return as_nn((const sDepthStencilStatesDesc*)pDS->GetDescStructPtr());
}


}
