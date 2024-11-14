#include "stdafx.h"
#include <niLang/Utils/IDGenerator.h>
#include "API/niUI_ModuleDef.h"
#include "GDRV_Gpu.h"

namespace ni {

_HDecl(gpufunction);

#define GPU_TRACE(aFmt) //niDebugFmt(aFmt)

eGpuPixelFormat _GetClosestGpuPixelFormatForRT(const achar* aRTFormat) {
  // the default RT is using BGRA8, other RT will follow this format so we can
  // reuse those pipelines
  return eGpuPixelFormat_BGRA8;
}

eGpuPixelFormat _GetClosestGpuPixelFormatForDS(const achar* aDSFormat) {
  if (niStringIsOK(aDSFormat)) {
    return eGpuPixelFormat_D32;
  }
  return eGpuPixelFormat_None;
}

eGpuPixelFormat _GetClosestGpuPixelFormatForTexture(const achar* aTexFormat) {
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
  else if (ni::StrEq(aTexFormat,"A8")) {
    return eGpuPixelFormat_RGBA8; // Fallback to RGBA8
  }
  else if (ni::StrEq(aTexFormat,"FR16G16B16A16")) {
    return eGpuPixelFormat_RGBA16F;
  }

  // Default fallback
  return eGpuPixelFormat_RGBA8;
}

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

#define GPU_BLENDMODE(OP,SRCRGB,SRCALPHA,DESTRGB,DESTALPHA) \
  { eGpuBlendOp_##OP, eGpuBlendFactor_##SRCRGB, eGpuBlendFactor_##SRCALPHA, eGpuBlendFactor_##DESTRGB, eGpuBlendFactor_##DESTALPHA }

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

iGpuBlendMode* _CreateGpuBlendMode() {
  return niNew sGpuBlendMode();
}

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

iGpuPipelineDesc* _CreateGpuPipelineDesc() {
  return niNew sGpuPipelineDescImpl();
}

template <typename T>
class cFixedGpuShaderBase : public T, public sShaderDesc
{
 public:
  typedef cFixedGpuShaderBase tFixedGpuShaderBase;

  cFixedGpuShaderBase(
    iGraphics* apGraphics, LocalIDGenerator* apIDGenerator,
    iHString* ahspName,
    iDeviceResourceManager* apDevResMan,
    iHString* ahspProfile,
    iGpuFunction* apGpuFunction)
      : mptrFunction(apGpuFunction)
  {
    niPanicAssert(mptrFunction.IsOK());
    mpGraphics = apGraphics;
    mhspName = ahspName;
    mhspProfile = ahspProfile;
    mptrDevResMan = apDevResMan;
    mptrDevResMan->Register(this);
    mpIDGenerator = apIDGenerator;
    mnUID = apIDGenerator->AllocID();
  }

  ~cFixedGpuShaderBase() {
    Invalidate();
  }

  virtual iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    if (aIID == niGetInterfaceUUID(iGpuFunction)) {
      return mptrFunction;
    }
    return T::QueryInterface(aIID);
  }

  virtual void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anFlags) const {
    apLst->Add(niGetInterfaceUUID(iGpuFunction));
    T::ListInterfaces(apLst,anFlags);
  }

  iHString* __stdcall GetDeviceResourceName() const
  {
    return mhspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eFalse;
  }

  virtual void __stdcall Invalidate()
  {
    mptrConstants = NULL;
    mpIDGenerator->FreeID(mnUID);
    if (!mptrDevResMan.IsOK()) return;
    mptrDevResMan->Unregister(this);
    mptrDevResMan = NULL;
    mptrFunction = nullptr;
  }

  iHString* __stdcall GetProfile() const
  {
    return mhspProfile;
  }

  iShaderConstants* __stdcall GetConstants() const
  {
    if (!mptrConstants.IsOK()) {
      niThis(cFixedGpuShaderBase)->mptrConstants = mpGraphics->CreateShaderConstants(4096);
    }
    return mptrConstants;
  }

  iGpuFunction* __stdcall GetGpuFunction() const niImpl {
    return mptrFunction;
  }

  tBool __stdcall GetHasConstants() const {
    return mptrConstants.IsOK();
  }

  tPtr __stdcall GetDescStructPtr() const {
    const sShaderDesc* d = this;
    return (tPtr)d;
  }

  iHString* __stdcall GetCode() const {
    return mhspName;
  }

  tU32 __stdcall GetUID() const niImpl {
    return mnUID;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    return this;
  }

  iGraphics* mpGraphics;
  Ptr<iDeviceResourceManager> mptrDevResMan;
  Ptr<iGpuFunction> mptrFunction;
  tU32 mnUID;
  LocalIDGenerator* mpIDGenerator;
};

// Vertex program
class cFixedGpuShaderVertex :
    public cFixedGpuShaderBase<ImplRC<iFixedGpuShader,eImplFlags_DontInherit1|eImplFlags_DontInherit2,iShader,iDeviceResource> >
{
 public:
  cFixedGpuShaderVertex(
    iGraphics* apGraphics, LocalIDGenerator* apIDGenerator,
    iHString* ahspName, iDeviceResourceManager* apDevResMan,
    iHString* ahspProfile, iGpuFunction* apFunction)
    : tFixedGpuShaderBase(apGraphics,apIDGenerator,
                       ahspName,apDevResMan,ahspProfile,
                       apFunction)
  {
    niPanicAssert(mptrFunction->GetFunctionType() == eGpuFunctionType_Vertex);
  }
  ~cFixedGpuShaderVertex() {
  }
  eShaderUnit __stdcall GetUnit() const {
    return eShaderUnit_Vertex;
  }
};

// Pixel program
class cFixedGpuShaderPixel :
    public cFixedGpuShaderBase<ImplRC<iFixedGpuShader,eImplFlags_DontInherit1|eImplFlags_DontInherit2,iShader,iDeviceResource> >
{
 public:
  cFixedGpuShaderPixel(
    iGraphics* apGraphics, LocalIDGenerator* apIDGenerator,
    iHString* ahspName, iDeviceResourceManager* apDevResMan,
    iHString* ahspProfile, iGpuFunction* apFunction)
    : tFixedGpuShaderBase(
        apGraphics,apIDGenerator,
        ahspName,apDevResMan,
        ahspProfile,apFunction)
  {
    niPanicAssert(mptrFunction->GetFunctionType() == eGpuFunctionType_Pixel);
  }
  ~cFixedGpuShaderPixel() {
  }
  eShaderUnit __stdcall GetUnit() const {
    return eShaderUnit_Pixel;
  }
};

struct sFixedGpuPipelines : public ImplRC<iFixedGpuPipelines> {
  typedef astl::map<tFixedGpuPipelineId,Ptr<iGpuPipeline> > tPipelineMap;
  tPipelineMap _pipelines;
  LocalIDGenerator _idGenerator;

  Ptr<iGpuFunction> CompileShader(iGraphicsDriverGpu* apGpuDriver, eGpuFunctionType aType, iHString* ahspName, const achar* aSource) niImpl {
    NN<iDataTable> dtFunc = CreateGpuFunctionDT(apGpuDriver,aSource);
    return apGpuDriver->CreateGpuFunction(aType,ahspName,dtFunc);
  }

  Ptr<iGpuPipeline> GetRenderPipeline(iGraphicsDriverGpu* apGpuDriver, ain<tFixedGpuPipelineId> aPipelineId, iFixedGpuShader* apVS, iFixedGpuShader* apPS) niImpl {
    tPipelineMap::iterator it = _pipelines.find(aPipelineId);
    if (it == _pipelines.end()) {
      Ptr<iGpuPipeline> pipeline = _CreatePipeline(apGpuDriver,(sFixedGpuPipelineId&)aPipelineId,apVS,apPS);
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

  Ptr<iGpuPipeline> _CreatePipeline(
    iGraphicsDriverGpu* apGpuDriver,
    const sFixedGpuPipelineId& aIdBits,
    iFixedGpuShader* apVS, iFixedGpuShader* apPS)
  {
    niCheck(apVS && apVS->GetGpuFunction() != nullptr,nullptr);
    niCheck(apVS->GetGpuFunction()->GetFunctionType() == eGpuFunctionType_Vertex,nullptr);
    niCheck(apPS && apPS->GetGpuFunction() != nullptr,nullptr);
    niCheck(apPS->GetGpuFunction()->GetFunctionType() == eGpuFunctionType_Pixel,nullptr);
    cFixedGpuShaderVertex* vs = (cFixedGpuShaderVertex*)apVS;
    cFixedGpuShaderPixel* ps = (cFixedGpuShaderPixel*)apPS;

    NN<iGpuPipelineDesc> ptrPipelineDesc{apGpuDriver->CreateGpuPipelineDesc()};
    sGpuPipelineDesc& desc = *(sGpuPipelineDesc*)ptrPipelineDesc->GetDescStructPtr();
    desc.mFVF = aIdBits.fvf;
    desc.mColorFormats[0] = aIdBits.rt0;
    desc.mColorFormats[1] = aIdBits.rt1;
    desc.mColorFormats[2] = aIdBits.rt2;
    desc.mColorFormats[3] = aIdBits.rt3;
    desc.mDepthFormat = aIdBits.ds;
    desc.mptrFuncs[eGpuFunctionType_Vertex] = vs->mptrFunction;
    desc.mptrFuncs[eGpuFunctionType_Pixel] = ps->mptrFunction;
    if (aIdBits.blendMode == 0) {
      desc.mptrBlendMode = nullptr;
    }
    else if (aIdBits.blendMode < eBlendMode_Last) {
      const sGpuBlendModeDesc& bm = _BlendModeToGpuBlendModeDesc((eBlendMode)aIdBits.blendMode);
      desc.mptrBlendMode = apGpuDriver->CreateGpuBlendMode();
      *((sGpuBlendModeDesc*)desc.mptrBlendMode->GetDescStructPtr()) = bm;
    }
    else {
      desc.mptrBlendMode = nullptr;
      niWarning("Invalid blend mode, falling back to 'no blending'.");
    }
    return apGpuDriver->CreateGpuPipeline(ptrPipelineDesc);
  }

  Ptr<iFixedGpuShader> CreateFixedGpuShader(iGraphics* apGraphics, iGpuFunction* apFunc, iHString* ahspName) niImpl {
    niCheckIsOK(apFunc,nullptr);
    switch (apFunc->GetFunctionType()) {
      case eGpuFunctionType_Vertex:
        return niNew cFixedGpuShaderVertex(
          apGraphics,
          &_idGenerator,
          ahspName,
          apGraphics->GetShaderDeviceResourceManager(),
          _HC(gpufunction),
          apFunc);
      case eGpuFunctionType_Pixel:
        return niNew cFixedGpuShaderPixel(
          apGraphics,
          &_idGenerator,
          ahspName,
          apGraphics->GetShaderDeviceResourceManager(),
          _HC(gpufunction),
          apFunc);
      default:
        niPanicUnreachable(niFmt("Invalid gpufunction type '%d'.",
                                 (tU32)apFunc->GetFunctionType()));
        return nullptr;
    }
  }
};

iFixedGpuPipelines* CreateFixedGpuPipelines() {
  return niNew sFixedGpuPipelines();
}

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
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) {
    return _buffer->Bind(apDevice);
  }

  virtual tFVF __stdcall GetFVF() const {
    return _fvf;
  }
  virtual tU32 __stdcall GetNumVertices() const {
    return _buffer->GetSize() / _fvfStride;
  }
  virtual eArrayUsage __stdcall GetUsage() const {
    return _arrayUsage;
  }
  virtual tPtr __stdcall Lock(tU32 anFirstVertex, tU32 anNumVertex, eLock aLock) {
    niUnused(aLock);
    return _buffer->Lock(anFirstVertex * _fvfStride, anNumVertex * _fvfStride, aLock);
  }
  virtual tBool __stdcall Unlock() {
    return _buffer->Unlock();
  }
  virtual tBool __stdcall GetIsLocked() const {
    return _buffer->GetIsLocked();
  }
};

iGpuBuffer* GetVertexArrayGpuBuffer(iVertexArray* apVA) {
  return ((sFixedGpuVertexArray*)apVA)->_buffer;
}
sVec2i GetVertexArrayFvfAndStride(iVertexArray* apVA) {
  niLet va = (sFixedGpuVertexArray*)apVA;
  return Vec2i((tI32)va->_fvf,(tI32)va->_fvfStride);
}

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
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) {
    return _buffer->Bind(apDevice);
  }

  virtual eGraphicsPrimitiveType __stdcall GetPrimitiveType() const {
    return _primType;
  }
  virtual tU32 __stdcall GetNumIndices() const {
    return _buffer->GetSize() / knFixedGpuIndexSize;
  }
  virtual tU32 __stdcall GetMaxVertexIndex() const {
    return 0xFFFFFFFF;
  }
  virtual eArrayUsage __stdcall GetUsage() const {
    return _arrayUsage;
  }

  virtual tPtr __stdcall Lock(tU32 anFirstIndex, tU32 anNumIndex, eLock aLock) {
    return _buffer->Lock(anFirstIndex * knFixedGpuIndexSize, anNumIndex * knFixedGpuIndexSize, aLock);
  }
  virtual tBool __stdcall Unlock() {
    return _buffer->Unlock();
  }
  virtual tBool __stdcall GetIsLocked() const {
    return _buffer->GetIsLocked();
  }
};

iGpuBuffer* GetIndexArrayGpuBuffer(iIndexArray* apVA) {
  return ((sFixedGpuIndexArray*)apVA)->_buffer;
}

iVertexArray* CreateFixedGpuVertexArray(iGraphicsDriverGpu* apGpuDriver, tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) {
    niLet fvfStride = FVFGetStride(anFVF);
    Ptr<iGpuBuffer> vaBuffer = apGpuDriver->CreateGpuBuffer(
      fvfStride * anNumVertices,
      eGpuBufferMemoryMode_Shared,
      eGpuBufferUsageFlags_Vertex);
    niCheckIsOK(vaBuffer,nullptr);
    return niNew sFixedGpuVertexArray(vaBuffer, anFVF, aUsage);
  }

iIndexArray* CreateFixedGpuIndexArray(iGraphicsDriverGpu* apGpuDriver, eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndices, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
  niUnused(anMaxVertexIndex);
  Ptr<iGpuBuffer> iaBuffer = apGpuDriver->CreateGpuBuffer(
    knFixedGpuIndexSize * anNumIndices,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Index);
  return niNew sFixedGpuIndexArray(iaBuffer, aPrimitiveType, aUsage);
}

NN<iDataTable> CreateGpuFunctionDT(iGraphicsDriverGpu* apGpuDriver, const achar* aaszSource) {
  NN<iDataTable> dtFunc = AsNN(ni::GetLang()->CreateDataTable("GpuFunction"));
  dtFunc->SetHString("target",apGpuDriver->GetGpuFunctionTarget());
  dtFunc->SetString("source",aaszSource);
  return dtFunc;
}

iDataTable* FindGpuFunctionDT(iDataTable* apDT, const iHString* ahspTarget) {
  if (!apDT)
    return nullptr;

  niLet dtName = apDT->GetName();
  if (!StrEq(dtName,"Shader") &&
      !StrEq(dtName,"Target") &&
      !StrEq(dtName,"GpuFunction") &&
      !StrEq(dtName,"GpuFunctionTarget") &&
      !StrEq(dtName,"GpuLibrary"))
    return nullptr;

  tHStringPtr hspTarget = apDT->GetHString("target");
  //niDebugFmt(("... FindGpuFunctionDT: %s, %s", apDT->GetName(), hspTarget));
  if (hspTarget == ahspTarget) {
    return apDT;
  }
  niLoop(i,apDT->GetNumChildren()) {
    iDataTable* foundDT = FindGpuFunctionDT(apDT->GetChildFromIndex(i), ahspTarget);
    if (foundDT)
      return foundDT;
  }
  return nullptr;
}

}
