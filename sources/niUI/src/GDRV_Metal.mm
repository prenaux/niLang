// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifdef __APPLE__
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <MetalKit/MetalKit.h>
#endif

#include "stdafx.h"

#ifdef GDRV_METAL

#include "API/niUI_ModuleDef.h"

#include <niLang/Platforms/OSX/osxMetal.h>
#include <niLang/STL/scope_guard.h>
#include <niLang/Utils/IDGenerator.h>

#include "../../nicgc/src/mojoshader/mojoshader.h"
#include "FixedShaders.h"
#include "GDRV_Gpu.h"
#include "GDRV_Utils.h"
#include "Graphics.h"

#if defined niOSX || defined niIOSMac
#define METAL_MAC
#else
#define METAL_IOS
#endif

// We use the std containers to hold Objective-C types, ASTL doesnt work
// correctly with them & ARC
#include <vector>
#include <map>

namespace ni {

#define NISH_TARGET msl2_macos

_HDecl(NISH_TARGET);
static __forceinline iHString* _GetGpuFunctionTarget() {
  return _HC(NISH_TARGET);
}

#define METAL_TRACE(aFmt) //niDebugFmt(aFmt)

// Commented out, we dont want to use native DXT support on macOS only since its not supported on iOS, which is our primary target
// #define USE_METAL_DXT

// TODO: This should move to some "context creation" time thing, similar to the MSAA setting.
static tU32 knMetalSamplerFilterAnisotropy = 8;

static cString _GetDeviceCaps(id<MTLDevice> aDevice) {
  cString r;
  r += niFmt("  name: %s\n", [aDevice.name UTF8String]);
  r += niFmt("  maxThreadsPerThreadgroup: %dx%dx%d\n",
             (tI32)aDevice.maxThreadsPerThreadgroup.width,
             (tI32)aDevice.maxThreadsPerThreadgroup.height,
             (tI32)aDevice.maxThreadsPerThreadgroup.depth);
#if !defined METAL_IOS
  r += niFmt("  lowPower: %y\n", (tBool)!!aDevice.lowPower);
  r += niFmt("  headless: %y\n", (tBool)!!aDevice.headless);
  r += niFmt("  depth24Stencil8PixelFormatSupported: %y\n", (tBool)!!aDevice.depth24Stencil8PixelFormatSupported);
  r += niFmt("  recommendedMaxWorkingSetSize: %d\n", (tU64)aDevice.recommendedMaxWorkingSetSize);
#endif

  EA_DISABLE_CLANG_WARNING(-Wunguarded-availability-new);
  r += niFmt("  supportsFamily:");
  if ([aDevice supportsFamily:MTLGPUFamilyMetal3]) {
    r += niFmt(" Metal3,");
  }
  if ([aDevice supportsFamily:MTLGPUFamilyCommon3]) {
    r += niFmt(" Common3,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyCommon2]) {
    r += niFmt(" Common2,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyCommon1]) {
    r += niFmt(" Common1,");
  }
  if ([aDevice supportsFamily:MTLGPUFamilyApple9]) {
    r += niFmt(" Apple9,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyApple8]) {
    r += niFmt(" Apple8,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyApple7]) {
    r += niFmt(" Apple7,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyApple6]) {
    r += niFmt(" Apple6,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyApple5]) {
    r += niFmt(" Apple5,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyApple4]) {
    r += niFmt(" Apple4,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyApple3]) {
    r += niFmt(" Apple3,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyApple2]) {
    r += niFmt(" Apple2,");
  }
  else if ([aDevice supportsFamily:MTLGPUFamilyApple1]) {
    r += niFmt(" Apple1,");
  }
  if ([aDevice supportsFamily:MTLGPUFamilyMac2]) {
    r += niFmt(" Mac2,");
  }
  if (r.EndsWith(",")) {
    r.resize(r.length()-1);
  }
  r += niFmt("\n");

  return r;
}

// We're not using the vertex descriptor, the vertex shader takes care of
// that. But this might come in handy later so its here for reference.
inline MTLVertexAttributeDescriptor* _CreateMetalAttrDesc(const tU32 aBufferIndex, MTLVertexFormat aFmt, const tU32 aOffset) {
  MTLVertexAttributeDescriptor* attrDesc = [MTLVertexAttributeDescriptor new];
  attrDesc.format = aFmt;
  attrDesc.offset = aOffset;
  attrDesc.bufferIndex = aBufferIndex;
  return attrDesc;
}

inline MTLVertexDescriptor* _CreateMetalVertDesc(const tU32 aBufferIndex, tFVF aFVF)
{
  MTLVertexDescriptor* vertDesc = [MTLVertexDescriptor vertexDescriptor];

  tU32 index = 0;
  tU32 off = 0;

#define METAL_ATTR(TYPE, TYPESZ)                              \
  { [vertDesc.attributes                                      \
     setObject: _CreateMetalAttrDesc(aBufferIndex, TYPE, off) \
     atIndexedSubscript: (index++)];                          \
    off += TYPESZ; }

  if (niFlagTest(aFVF,eFVF_PositionB4))  {
    METAL_ATTR(MTLVertexFormatFloat3, sizeof(sVec3f));
    METAL_ATTR(MTLVertexFormatFloat4, sizeof(sVec4f));
  }
  else if (niFlagTest(aFVF,eFVF_PositionB3))  {
    METAL_ATTR(MTLVertexFormatFloat3, sizeof(sVec3f));
    METAL_ATTR(MTLVertexFormatFloat3, sizeof(sVec3f));
  }
  else if (niFlagTest(aFVF,eFVF_PositionB2))  {
    METAL_ATTR(MTLVertexFormatFloat3, sizeof(sVec3f));
    METAL_ATTR(MTLVertexFormatFloat2, sizeof(sVec2f));
  }
  else if (niFlagTest(aFVF,eFVF_PositionB1))  {
    METAL_ATTR(MTLVertexFormatFloat3, sizeof(sVec3f));
    METAL_ATTR(MTLVertexFormatFloat, sizeof(float));
  }
  else if (niFlagTest(aFVF,eFVF_Position))  {
    METAL_ATTR(MTLVertexFormatFloat3, sizeof(sVec3f));
  }

  if (niFlagTest(aFVF,eFVF_Indices)) {
    METAL_ATTR(MTLVertexFormatUChar4, sizeof(tU32));
  }

  if (niFlagTest(aFVF,eFVF_Normal)) {
    METAL_ATTR(MTLVertexFormatFloat3, sizeof(sVec3f));
  }
  if (niFlagTest(aFVF,eFVF_PointSize)) {
    METAL_ATTR(MTLVertexFormatFloat, sizeof(tF32));
  }
  if (niFlagTest(aFVF,eFVF_ColorA)) {

    METAL_ATTR(MTLVertexFormatUChar4Normalized_BGRA, sizeof(tU32));
  }

  const tU32 ulNumTexCoos  = eFVF_TexNumCoo(aFVF);
  for (tU32 i = 0; i < ulNumTexCoos; ++i) {
    const tU32 dim = eFVF_TexCooDim(aFVF,i);
    switch(dim) {
      case 1: METAL_ATTR(MTLVertexFormatFloat, sizeof(tF32)); break;
      case 2: METAL_ATTR(MTLVertexFormatFloat2, sizeof(sVec2f)); break;
      case 3: METAL_ATTR(MTLVertexFormatFloat3, sizeof(sVec3f)); break;
      case 4: METAL_ATTR(MTLVertexFormatFloat4, sizeof(sVec4f)); break;
      default: niAssertUnreachable("Invalid FVF.");
    }
  }

  const tU32 stride = off;
  MTLVertexBufferLayoutDescriptor *layoutDesc = [MTLVertexBufferLayoutDescriptor new];
  layoutDesc.stride = stride;
  layoutDesc.stepFunction = MTLVertexStepFunctionPerVertex;
  layoutDesc.stepRate = 1;
  [vertDesc.layouts setObject: layoutDesc atIndexedSubscript: 0];

  return vertDesc;

#undef METAL_ATTR
}

//----------------------------------------------------------------------------
//
// GpuFunction
//
//----------------------------------------------------------------------------
static inline eGpuFunctionType _GetGpuFunctionType(MTLFunctionType aFuncType) {
  switch (aFuncType) {
    case MTLFunctionTypeVertex:
      return eGpuFunctionType_Vertex;
    case MTLFunctionTypeFragment:
      return eGpuFunctionType_Pixel;
    case MTLFunctionTypeKernel:
      return eGpuFunctionType_Compute;
    default:
      niPanicUnreachable("Unsupported MTLFunction type");
      return eGpuFunctionType_Last;
  }
}

struct sMetalFunction : public ImplRC<iGpuFunction> {
  tHStringPtr _hspName;
  Ptr<iDataTable> _datatable;
  id<MTLFunction> _mtlFunction;

  tBool _Create(id<MTLDevice> aDevice, iHString* ahspName, iDataTable* apDTRoot) {
    niCheckIsOK(apDTRoot,eFalse);
    _hspName = ahspName;
    _datatable = niCheckNN(_datatable,FindGpuFunctionDT(apDTRoot,_GetGpuFunctionTarget()),eFalse);

    cString source;
    if (_datatable->HasProperty("source")) {
      source = _datatable->GetString("source");
    }
    else if (_datatable->HasProperty("_data")) {
      source = _datatable->GetString("_data");
    }
    niCheck(source.IsNotEmpty(),eFalse);

    NSString* nsSource = [NSString stringWithUTF8String:source.c_str()];

    NSError* error = nil;
    MTLCompileOptions* compileOptions = [MTLCompileOptions new];
    compileOptions.languageVersion = MTLLanguageVersion2_0;
    id<MTLLibrary> lib = [aDevice newLibraryWithSource:nsSource
                          options:compileOptions error:&error];
    if (!lib) {
      niError(niFmt(
        "MetalGpuFunction '%s': Couldn't create MTLLibrary: %s",
        _hspName,
        error.localizedDescription.UTF8String));
      return eFalse;
    }

    NSString* funcName = lib.functionNames[0];
    _mtlFunction = [lib newFunctionWithName:funcName];
    if (!_mtlFunction) {
      niError(niFmt(
        "MetalGpuFunction '%s': Couldn't find MTLFunction '%s'.",
        _hspName, funcName.UTF8String));
      return eFalse;
    }

    return eTrue;
  }

  virtual eGpuFunctionType __stdcall GetFunctionType() const {
    if (_mtlFunction == nil)
      return eGpuFunctionType_Last;
    return _GetGpuFunctionType(_mtlFunction.functionType);
  }

  virtual iDataTable* __stdcall GetDataTable() const niImpl {
    return _datatable;
  }

  iHString* __stdcall GetDeviceResourceName() const {
    return _hspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown *apDevice) {
    return this;
  }
};

//-------------------------------------------------------------------------------------------
//
// States & Shaders
//
//-------------------------------------------------------------------------------------------
static MTLPixelFormat _GetMTLPixelFormat(eGpuPixelFormat aFmt) {
  switch (aFmt) {
    case eGpuPixelFormat_None:
      return MTLPixelFormatInvalid;
    case eGpuPixelFormat_BGRA8:
      return MTLPixelFormatBGRA8Unorm;
    case eGpuPixelFormat_RGBA8:
      return MTLPixelFormatRGBA8Unorm;
    case eGpuPixelFormat_RGBA16F:
      return MTLPixelFormatRGBA16Float;
    case eGpuPixelFormat_R16F:
      return MTLPixelFormatR16Float;
    case eGpuPixelFormat_R32F:
      return MTLPixelFormatR32Float;
    case eGpuPixelFormat_D32:
      return MTLPixelFormatDepth32Float;
    case eGpuPixelFormat_D16:
      return MTLPixelFormatDepth16Unorm;
    case eGpuPixelFormat_D24S8:
      return MTLPixelFormatDepth24Unorm_Stencil8;
  }
  return MTLPixelFormatInvalid;
}

static const MTLCullMode _toMTLCullMode[] = {
  MTLCullModeNone,
  MTLCullModeFront,
  MTLCullModeBack,
};
niCAssert(niCountOf(_toMTLCullMode) == eCullingMode_Last);

static const MTLCompareFunction _toMTLCompareFunction[] = {
  MTLCompareFunctionNever,
  MTLCompareFunctionEqual,
  MTLCompareFunctionNotEqual,
  MTLCompareFunctionLess,
  MTLCompareFunctionLessEqual,
  MTLCompareFunctionGreater,
  MTLCompareFunctionGreaterEqual,
  MTLCompareFunctionAlways,
};
niCAssert(niCountOf(_toMTLCompareFunction) == eGraphicsCompare_Last);

static const MTLStencilOperation _toMTLStencilOp[] = {
  MTLStencilOperationKeep,
  MTLStencilOperationZero,
  MTLStencilOperationReplace,
  MTLStencilOperationIncrementWrap,
  MTLStencilOperationDecrementWrap,
  MTLStencilOperationIncrementClamp,
  MTLStencilOperationDecrementClamp,
  MTLStencilOperationInvert,
};
niCAssert(niCountOf(_toMTLStencilOp) == eStencilOp_Last);

void _toMTLSamplerFilter(MTLSamplerDescriptor* desc, eSamplerFilter aFilter) {
  switch(aFilter) {
    case eSamplerFilter_Smooth: {
      desc.minFilter = MTLSamplerMinMagFilterLinear;
      desc.magFilter = MTLSamplerMinMagFilterLinear;
      desc.mipFilter = MTLSamplerMipFilterLinear;
      desc.maxAnisotropy = knMetalSamplerFilterAnisotropy;
#if defined METAL_IOS
      desc.lodAverage = YES;
#endif
      break;
    }
    case eSamplerFilter_Point: {
      desc.minFilter = MTLSamplerMinMagFilterNearest;
      desc.magFilter = MTLSamplerMinMagFilterNearest;
      desc.mipFilter = MTLSamplerMipFilterNearest;
      desc.maxAnisotropy = knMetalSamplerFilterAnisotropy;
#if defined METAL_IOS
      desc.lodAverage = YES;
#endif
      break;
    }
  }
}

static const MTLSamplerAddressMode _toMTLSamplerAddress[] = {
  MTLSamplerAddressModeRepeat,
  MTLSamplerAddressModeClampToEdge,
  MTLSamplerAddressModeMirrorRepeat,
  MTLSamplerAddressModeClampToZero,
};
niCAssert(niCountOf(_toMTLSamplerAddress) == eSamplerWrap_Last);

static const MTLBlendOperation _toMTLBlendOp[] = {
  MTLBlendOperationAdd,
  MTLBlendOperationSubtract,
  MTLBlendOperationReverseSubtract,
  MTLBlendOperationMin,
  MTLBlendOperationMax,
};
niCAssert(niCountOf(_toMTLBlendOp) == eGpuBlendOp_Last);

static const MTLBlendFactor _toMTLBlendFactor[] = {
  MTLBlendFactorZero,
  MTLBlendFactorOne,
  MTLBlendFactorSourceColor,
  MTLBlendFactorOneMinusSourceColor,
  MTLBlendFactorSourceAlpha,
  MTLBlendFactorOneMinusSourceAlpha,
  MTLBlendFactorDestinationAlpha,
  MTLBlendFactorOneMinusDestinationAlpha,
  MTLBlendFactorDestinationColor,
  MTLBlendFactorOneMinusDestinationColor,
  MTLBlendFactorSourceAlphaSaturated,
  MTLBlendFactorBlendColor,
  MTLBlendFactorOneMinusBlendColor,
};
niCAssert(niCountOf(_toMTLBlendFactor) == eGpuBlendFactor_Last);

static const MTLPrimitiveType _toMTLPrimitiveType[] = {
  MTLPrimitiveTypePoint,
  MTLPrimitiveTypeLine,
  MTLPrimitiveTypeLineStrip,
  MTLPrimitiveTypeTriangle,
  MTLPrimitiveTypeTriangleStrip,
};
niCAssert(niCountOf(_toMTLPrimitiveType) == eGraphicsPrimitiveType_Last);

static const MTLIndexType _toMTLIndexType[] = {
  MTLIndexTypeUInt16,
  MTLIndexTypeUInt32,
};
niCAssert(niCountOf(_toMTLIndexType) == eGpuIndexType_Last);

struct sMetalPipeline : public ImplRC<iGpuPipeline> {
  NN<iGpuPipelineDesc> _desc = niDeferredInit(NN<iGpuPipelineDesc>);
  id<MTLRenderPipelineState> _mtlPipeline;

  tBool _Create(id<MTLDevice> aDevice, const iGpuPipelineDesc* apDesc) {
    niCheckIsOK(apDesc,eFalse);
    _desc = niCheckNN(_desc,apDesc->Clone(),eFalse);

    MTLRenderPipelineDescriptor* pipelineDesc = [MTLRenderPipelineDescriptor new];

    // Vertex & Fragment functions
    sMetalFunction* vertexFunction = (sMetalFunction*)_desc->GetFunction(eGpuFunctionType_Vertex);
    niCheck(vertexFunction != nullptr, 0);

    sMetalFunction* pixelFunction = (sMetalFunction*)_desc->GetFunction(eGpuFunctionType_Pixel);
    niCheck(pixelFunction != nullptr, 0);

    pipelineDesc.vertexFunction = vertexFunction->_mtlFunction;
    pipelineDesc.fragmentFunction = pixelFunction->_mtlFunction;

    // Color attachments
    {
      MTLRenderPipelineColorAttachmentDescriptor* ca = pipelineDesc.colorAttachments[0];
      ca.pixelFormat = _GetMTLPixelFormat(_desc->GetColorFormat(0));

      if (_desc->GetBlendMode()) {
        const sGpuBlendModeDesc* bm = (const sGpuBlendModeDesc*)_desc->GetBlendMode()->GetDescStructPtr();
        ca.blendingEnabled = YES;
        ca.rgbBlendOperation = ca.alphaBlendOperation = _toMTLBlendOp[bm->mOp];
        ca.sourceRGBBlendFactor = _toMTLBlendFactor[bm->mSrcRGB];
        ca.sourceAlphaBlendFactor = _toMTLBlendFactor[bm->mSrcAlpha];
        ca.destinationRGBBlendFactor = _toMTLBlendFactor[bm->mDstRGB];
        ca.destinationAlphaBlendFactor = _toMTLBlendFactor[bm->mDstAlpha];
      }
      else {
        ca.blendingEnabled = NO;
      }
    }

    // Depth
    pipelineDesc.depthAttachmentPixelFormat = _GetMTLPixelFormat(_desc->GetDepthFormat());
    niCheck(
      _desc->GetDepthFormat() == eGpuPixelFormat_None ||
      _desc->GetDepthFormat() == eGpuPixelFormat_D32 ||
      _desc->GetDepthFormat() == eGpuPixelFormat_D16 ||
      _desc->GetDepthFormat() == eGpuPixelFormat_D24S8,
      eFalse);
    pipelineDesc.vertexDescriptor = _CreateMetalVertDesc(0, _desc->GetFVF());

    // Create the pipeline state
    NSError* error = nil;
    _mtlPipeline = [aDevice newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
    if (!_mtlPipeline) {
      niError(niFmt("Cant create pipeline state: %s", error.localizedDescription.UTF8String));
      return eFalse;
    }

    return eTrue;
  }

  virtual const iGpuPipelineDesc* __stdcall GetDesc() const niImpl {
    return _desc;
  }
};

//--------------------------------------------------------------------------------------------
//
// MetalBuffer
//
//--------------------------------------------------------------------------------------------
struct sMetalBuffer : public ni::ImplRC<iGpuBuffer> {
  id<MTLBuffer> _mtlBuffer;
  eGpuBufferMemoryMode _memMode;
  tGpuBufferUsageFlags _usage;
  tU32 _lockOffset = 0, _lockSize = 0;
  tU32 _modifiedOffset = 0, _modifiedSize = 0;
  eLock _lockMode;
  tBool _tracked;

  tBool _Create(id<MTLDevice> aDevice, void* apInitialData, tU32 anSize,
                eGpuBufferMemoryMode aMemMode,
                tGpuBufferUsageFlags aUsage)
  {
    _tracked = eFalse;
    _usage = aUsage;
    _memMode = aMemMode;

    MTLResourceOptions resOptions;
    switch (_memMode) {
      case eGpuBufferMemoryMode_Shared: {
        resOptions = MTLResourceStorageModeShared;
        break;
      }
      case eGpuBufferMemoryMode_Managed: {
        resOptions = MTLResourceStorageModeManaged;
        break;
      }
      case eGpuBufferMemoryMode_Private: {
        resOptions = MTLResourceStorageModePrivate;
        break;
      }
    }

    if (apInitialData) {
      _mtlBuffer = [aDevice newBufferWithBytes:apInitialData
                    length: anSize
                    options: resOptions];
    }
    else {
      _mtlBuffer = [aDevice newBufferWithLength:anSize
                    options: resOptions];
    }
    return _mtlBuffer != NULL;
  }

  void _Untrack() {
    // niDebugFmt(("... Unbind: %p: [ms:%d,me:%d].", (tIntPtr)this, _modifiedStart,_modifiedEnd));
    _modifiedOffset = _modifiedSize = 0;
    _tracked = eFalse;
  }

  virtual tBool __stdcall IsOK() const niImpl {
    return _mtlBuffer != NULL;
  }

  virtual eGpuBufferMemoryMode __stdcall GetMemoryMode() const niImpl {
    return _memMode;
  }
  virtual tGpuBufferUsageFlags __stdcall GetUsageFlags() const niImpl {
    return _usage;
  }

  virtual tU32 GetSize() const niImpl {
    return [_mtlBuffer length];
  }

  virtual tPtr __stdcall Lock(tU32 anOffset, tU32 anSize, eLock aLock) niImpl {
    niCheck(_memMode != eGpuBufferMemoryMode_Private, nullptr);
    niCheck(!GetIsLocked(),nullptr);

    _lockMode = aLock;
    _lockOffset = anOffset;
    _lockSize = anSize ? anSize : (GetSize()-anOffset);

    if (_modifiedSize == 0) {
      _modifiedOffset = _lockOffset;
      _modifiedSize = _lockSize;
    }
    else if ((_lockOffset < (_modifiedOffset+_modifiedSize)) &&
             (_lockOffset+_lockSize) > _modifiedOffset)
    {
      if (_tracked) {
        // TODO: The lock should fail in this case and return nullptr? We
        // should not allow submitted buffers to be modified?
        niWarning(niFmt(
          "Lock(%d,%d,%d): %p: [lo:%d,ls:%d] [mo:%d,ms:%d] Locked inflight overlapping area.",
          anOffset,anSize,aLock,
          (tIntPtr)this,
          _lockOffset,_lockSize,
          _modifiedOffset,_modifiedSize));
      }
      const tU32 newStart = ni::Min(_modifiedOffset,_lockOffset);
      const tU32 newEnd = ni::Max(_modifiedOffset+_modifiedSize,
                                  _lockOffset+_lockSize);
      _modifiedOffset = newStart;
      _modifiedSize = newEnd - newStart;
    }
    return ((tPtr)[_mtlBuffer contents]) + _lockOffset;
  }

  virtual tBool __stdcall Unlock() niImpl {
    if (!GetIsLocked())
      return eFalse;
    if (_memMode == eGpuBufferMemoryMode_Managed &&
        _lockMode != eLock_ReadOnly)
    {
      [_mtlBuffer
       didModifyRange:NSMakeRange(_lockOffset,_lockSize)];
    }
    _lockOffset = _lockSize = 0;
    return eTrue;
  }

  virtual tBool __stdcall GetIsLocked() const {
    return _lockSize != 0;
  }

  virtual iHString* __stdcall GetDeviceResourceName() const niImpl {
    return nullptr;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    return eTrue;
  }
  virtual iDeviceResource* __stdcall Bind(iUnknown* apDevice) {
    return this;
  }
};

//--------------------------------------------------------------------------------------------
//
// MetalTexture
//
//--------------------------------------------------------------------------------------------
struct cMetalTexture : public ni::ImplRC<iTexture,eImplFlags_DontInherit1,iDeviceResource>
{
  iGraphics* mpGraphics;
  tHStringPtr mhspName;
  const eBitmapType mType;
  const tU32 mnW, mnH, mnD;
  const tU32 mnMM;
  eGpuPixelFormat mGpuPxf;
  tTextureFlags mnFlags;
  Ptr<iBitmapBase> mptrBitmap;
  tU32 mnFace;
  Ptr<cMetalTexture> _subTexs[6];
  id<MTLTexture> _tex;

  cMetalTexture(
    id<MTLDevice> aDevice,
    iGraphics* apGraphics,
    iHString* ahspName,
    eBitmapType aType,
    tU32 anW, tU32 anH, tU32 anD,
    tU32 anMM,
    iPixelFormat* apPxf,
    tTextureFlags anFlags)
      : mpGraphics(apGraphics)
      , mhspName(ahspName)
      , mType(aType)
      , mnW(anW)
      , mnH(anH)
      , mnD(anD)
      , mnMM(anMM)
      , mnFlags(anFlags)
  {
    niPanicAssert(niIsOK(apPxf));
    if (niFlagIs(anFlags,eTextureFlags_DepthStencil)) {
      mGpuPxf = _GetClosestGpuPixelFormatForDS(apPxf->GetFormat());
    }
    else if (niFlagIs(anFlags,eTextureFlags_RenderTarget)) {
      mGpuPxf = _GetClosestGpuPixelFormatForRT(apPxf->GetFormat());
    }
    else {
      mGpuPxf = _GetClosestGpuPixelFormatForTexture(apPxf->GetFormat());
    }
    Ptr<iPixelFormat> texPxf = _GetIPixelFormat(apGraphics,mGpuPxf);
    if (!texPxf.IsOK()) {
      niError(niFmt("Can't get compatible gpu pixel format for '%s'.", apPxf->GetFormat()));
      return;
    }

    if (niFlagIsNot(anFlags,eTextureFlags_Surface)) {
      METAL_TRACE(("## METAL-TEXTURE [%s] fmt:%s w:%s h:%s",ahspName,apPxf->GetFormat(),mnW,mnH));
      MTLTextureDescriptor*  desc = [MTLTextureDescriptor new];
      desc.width = anW;
      desc.height = anH;
      desc.pixelFormat = _GetMTLPixelFormat(mGpuPxf);
      desc.mipmapLevelCount = mnMM + 1;
      switch (aType) {
        case eBitmapType_2D:
          desc.textureType = MTLTextureType2D;
          break;
        case eBitmapType_Cube:
          desc.textureType = MTLTextureTypeCube;
          break;
        case eBitmapType_3D:
          desc.textureType = MTLTextureType3D;
          desc.depth = anD;
          break;
        default:
          niError("Invalid bitmap format");
          break;
      }

      desc.usage = MTLTextureUsageShaderRead;
      if (niFlagIs(anFlags,eTextureFlags_RenderTarget)) {
        desc.usage |= MTLTextureUsageRenderTarget | MTLTextureUsageShaderWrite;
      }
      else if (niFlagIs(anFlags,eTextureFlags_DepthStencil)) {
        desc.usage |= MTLTextureUsageRenderTarget;
        desc.storageMode = MTLStorageModePrivate;
      }
      else {
        switch (aType) {
          case eBitmapType_2D:
            mptrBitmap = apGraphics->CreateBitmap2DEx(mnW,mnH,texPxf);
            break;
          case eBitmapType_Cube:
            mptrBitmap = apGraphics->CreateBitmapCubeEx(mnW,texPxf);
            break;
          case eBitmapType_3D:
            mptrBitmap = apGraphics->CreateBitmap3DEx(mnW,mnH,mnD,texPxf);
            break;
        }
        if (anMM) {
          mptrBitmap->CreateMipMaps(anMM,eFalse);
        }
      }

      _tex = [aDevice newTextureWithDescriptor:desc];
      if (aType == eBitmapType_Cube) {
        niLoop(i,6) {
          _subTexs[i] = niNew cMetalTexture(aDevice,mpGraphics,ahspName,eBitmapType_2D,
                                            mnW,mnH,mnD,mnMM,texPxf,eTextureFlags_SubTexture|eTextureFlags_Surface);
          _subTexs[i]->_tex = _tex;
          _subTexs[i]->mnFace = i;
          _subTexs[i]->mptrBitmap = QPtr<iBitmapCube>(mptrBitmap)->GetFace((eBitmapCubeFace)i);
        }
      }
    }

    if (niFlagIsNot(mnFlags, eTextureFlags_SubTexture))
      mpGraphics->GetTextureDeviceResourceManager()->Register(this);
  }

  ~cMetalTexture() {
    Invalidate();
  }

  virtual tBool IsOK() const {
    return niFlagIs(mnFlags,eTextureFlags_Surface) || (_tex != nil);
  }

  virtual void __stdcall Invalidate() {
    if (mpGraphics && mpGraphics->GetTextureDeviceResourceManager()) {
      mpGraphics->GetTextureDeviceResourceManager()->Unregister(this);
      mpGraphics = NULL;
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
    return _GetIPixelFormat(mpGraphics,mGpuPxf);
  }
  virtual tU32 __stdcall GetNumMipMaps() const {
    return mnMM;
  }
  virtual tTextureFlags __stdcall GetFlags() const {
    return mnFlags;
  }
  virtual iTexture* __stdcall GetSubTexture(tU32 anIndex) const {
    if (anIndex >= 6) return NULL;
    return _subTexs[anIndex];
  }

  void UploadTexture(Ptr<iPixelFormat> aFormat,
                     const iBitmap2D* apBmpLevel,
                     const tU32 anLevel,
                     const sRecti& aDestRect) {
    const tU32 bpp = aFormat->GetBytesPerPixel();
    METAL_TRACE(("Bitmap Format[%s] bpp:%s Texture format[%s]",aFormat->GetFormat(), bpp, mptrPxf->GetFormat()));
    const tU32 bpr = apBmpLevel->GetPitch();
    const tU32 startOffset = (aDestRect.y * bpr) + (aDestRect.x * bpp);
    auto region = MTLRegionMake2D(aDestRect.x,aDestRect.y,aDestRect.GetWidth(),aDestRect.GetHeight());
    auto bytes = (void*)(apBmpLevel->GetData()+startOffset);
    if (niFlagIs(mnFlags,eTextureFlags_SubTexture)) {
      [_tex replaceRegion:region
       mipmapLevel:anLevel
       slice:mnFace
       withBytes:bytes
       bytesPerRow:bpr
       bytesPerImage:0];
    }
    else {
      [_tex replaceRegion:region
       mipmapLevel:anLevel
       withBytes:bytes
       bytesPerRow:bpr];
    }
  }

  iBitmap2D* GetBitmap() {
    QPtr<iBitmap2D> bmp(mptrBitmap);
    if (!bmp.IsOK()) {
      bmp = mpGraphics->CreateBitmap2DEx(mnW, mnH, this->GetPixelFormat());
      [_tex getBytes:bmp->GetData() bytesPerRow:bmp->GetPitch() fromRegion:MTLRegionMake2D(0, 0, mnW, mnH) mipmapLevel:mnMM];
    }
    return bmp.GetRawAndSetNull();
  }
};

//--------------------------------------------------------------------------------------------
//
// MetalGraphicsDriver
//
//--------------------------------------------------------------------------------------------
static iGraphicsContext* New_MetalContextWindow(
  struct cMetalGraphicsDriver* apParent,
  const tU32 aFrameMaxInFlight,
  iOSWindow* apWindow,
  iOSXMetalAPI* apAPI,
  const achar* aaszBBPxf,
  const achar* aaszDSPxf,
  tU32 anSwapInterval,
  tTextureFlags anBBFlags);
static iGraphicsContextRT* New_MetalContextRT(
  cMetalGraphicsDriver* apParent,
  const tU32 aFrameMaxInFlight,
 iTexture* apRT,
  iTexture* apDS);

struct cMetalGraphicsDriver : public ImplRC<iGraphicsDriver,eImplFlags_Default,iGraphicsDriverGpu>
{
  iGraphics* mpGraphics;
  id<MTLDevice> mMetalDevice;
  id<MTLCommandQueue> mMetalCommandQueue;
  sFixedShaders mFixedShaders;
  astl::vector<Ptr<iHString>> mvProfiles[eShaderUnit_Last];
  NN<iFixedGpuPipelines> mFixedPipelines;
  LocalIDGenerator mIDGenerator;

  cMetalGraphicsDriver(iGraphics* apGraphics, id<MTLDevice> aDevice)
      : mFixedPipelines(CreateFixedGpuPipelines())
  {
    mpGraphics = apGraphics;
    mMetalDevice = aDevice;

    _InitCompiledSamplerStates();
    _InitCompiledDepthStencilStates();
    {
      niLog(Info,niFmt("--- Metal Device Info ---\n%s\n-------------------------", _GetDeviceCaps(mMetalDevice)));
    }
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
    return _A("Metal");
  }
  virtual const achar* __stdcall GetDesc() const {
    return _A("Metal Graphics Driver");
  }
  virtual const achar* __stdcall GetDeviceName() const {
    return _A("Default");
  }

  /////////////////////////////////////////////
  id<MTLCommandQueue> _GetCommandQueue() {
    if (mMetalCommandQueue == nil) {
      mMetalCommandQueue = [mMetalDevice newCommandQueue];
      niWarning("No command queue explicitly set by a window context, creating our own.");
    }
    return mMetalCommandQueue;
  }

  tBool _DetectShaderProfiles() {
    // Detect profiles
    if (mvProfiles[0].empty()) {
      niLoop(i,eShaderUnit_Last) {
        mvProfiles[i].clear();
      }

      // mvProfiles[eShaderUnit_Vertex].push_back(_H("glslv"));
      mvProfiles[eShaderUnit_Vertex].push_back(_H("hlsl_vs_3_0"));
      // mvProfiles[eShaderUnit_Pixel].push_back(_H("glslf"));
      mvProfiles[eShaderUnit_Pixel].push_back(_H("hlsl_ps_3_0"));
    }
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual iGraphicsContext* __stdcall CreateContextForWindow(
    iOSWindow* apWindow,
    const achar* aaszBBFormat, const achar* aaszDSFormat,
    tU32 anSwapInterval, tTextureFlags aBackBufferFlags)
  {
    niCheckIsOK(apWindow,NULL);

    QPtr<iOSXMetalAPI> metalAPI = apWindow->GetGraphicsAPI();
    if (!metalAPI.IsOK()) {
#if defined METAL_MAC && !defined niIOSMac
      metalAPI = osxMetalCreateAPIForWindow((__bridge void*)mMetalDevice, apWindow);
      if (!metalAPI.IsOK()) {
        niError("No metal api set on the window and can't create one.");
        return NULL;
      }
#else
      niError("No metal api set on the window.");
      return NULL;
#endif
    }

    mMetalCommandQueue = (__bridge id<MTLCommandQueue>)metalAPI->GetCommandQueue();
    if (!mMetalCommandQueue) {
      niError("Can't get metal api's command queue.");
      return NULL;
    }
    niAssert(mMetalDevice == mMetalCommandQueue.device);

    {
      MTKView* mtkView = (__bridge MTKView*)metalAPI->GetMTKView();
      if (mtkView) {
        mtkView.depthStencilPixelFormat = _GetMTLPixelFormat(_GetClosestGpuPixelFormatForDS(aaszDSFormat));
        if (anSwapInterval) {
          mtkView.preferredFramesPerSecond = (anSwapInterval == 2) ? 30 : 60;
        }
        else {
          mtkView.preferredFramesPerSecond = 120;
        }
      }
    }

    _DetectShaderProfiles();

    Ptr<iGraphicsContext> ctx = New_MetalContextWindow(
      this, 1,
      apWindow,
      metalAPI,
      aaszBBFormat,aaszDSFormat,
      anSwapInterval,
      aBackBufferFlags);
    if (!niIsOK(ctx))
      return NULL;

    if (!mFixedShaders.Initialize(mpGraphics)) {
      niError("Can't load fixed shaders.");
      return NULL;
    }

    return ctx.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  virtual iGraphicsContextRT* __stdcall CreateContextForRenderTargets(
    iTexture* apRT0, iTexture* apRT1, iTexture* apRT2, iTexture* apRT3, iTexture* apDS)
  {
    niCheckIsOK(apRT0,NULL);
    niCheckIsOK(apDS,NULL);

    Ptr<iGraphicsContextRT> ctx = New_MetalContextRT(this, 1, apRT0, apDS);
    niCheck(ctx.IsOK(),NULL);
    return ctx.GetRawAndSetNull();
  }

  /////////////////////////////////////////////
  virtual tInt __stdcall GetCaps(eGraphicsCaps aCaps) const {
    switch (aCaps) {
      case eGraphicsCaps_Resize:
      case eGraphicsCaps_MultiContext:
      case eGraphicsCaps_ScissorTest:
      case eGraphicsCaps_OverlayTexture:
      case eGraphicsCaps_NumTextureUnits:
      case eGraphicsCaps_NumRenderTargetTextures:
        return 1;
      case eGraphicsCaps_Texture2DMaxSize:
        return 0xFFFF;
      case eGraphicsCaps_TextureCubeMaxSize:
      case eGraphicsCaps_Texture3DMaxSize:
        return 0;
      case eGraphicsCaps_MaxVertexIndex:
        return 0xFFFFFFFF;
      case eGraphicsCaps_DepthStencilTexture:
      case eGraphicsCaps_StencilTwoSideded:
      case eGraphicsCaps_StencilWrap:
      case eGraphicsCaps_OcclusionQueries:
      case eGraphicsCaps_MaxPointSize:
      case eGraphicsCaps_HardwareInstancing:
      case eGraphicsCaps_OrthoProjectionOffset:
      case eGraphicsCaps_BlitBackBuffer:
        return 0;
      case eGraphicsCaps_Wireframe:
        return 1;
      case eGraphicsCaps_IGpu:
        return 1;
    }
  }


  /////////////////////////////////////////////
  virtual tBool __stdcall ResetAllCaches() {
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tGraphicsDriverImplFlags __stdcall GetGraphicsDriverImplFlags() const {
    return eGraphicsDriverImplFlags_IndexArrayObject|
        eGraphicsDriverImplFlags_VertexArrayObject;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall GetIsInitialized() const {
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags) {
    niCheckSilent(niIsOK(apFormat),eFalse);

    eGpuPixelFormat gpufmt;
    if (niFlagIs(aFlags,eTextureFlags_DepthStencil)) {
      gpufmt = _GetClosestGpuPixelFormatForDS(apFormat->GetPixelFormat()->GetFormat());
    }
    else if (niFlagIs(aFlags,eTextureFlags_RenderTarget)) {
      gpufmt = _GetClosestGpuPixelFormatForRT(apFormat->GetPixelFormat()->GetFormat());
    }
    else {
      gpufmt = _GetClosestGpuPixelFormatForTexture(apFormat->GetPixelFormat()->GetFormat());
    }

    // TODO: For now all eGpuPixelFormat are supported by metal, but that
    // might not always be the case. Eventually we should validate this.
    MTLPixelFormat mtlFormat = _GetMTLPixelFormat(gpufmt);
    NN<iPixelFormat> bmpFormat = niCheckNN(bmpFormat,_GetIPixelFormat(mpGraphics,gpufmt),eFalse);
    if (bmpFormat->IsSamePixelFormat(apFormat->GetPixelFormat())) {
      return eTrue;
    }
    else {
      apFormat->SetPixelFormat(bmpFormat);
      return eFalse;
    }
  }

  virtual iTexture* __stdcall CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags)  {
    METAL_TRACE(("TEXTURE CREATED: %s",ahspName));
    Ptr<iPixelFormat> pxf = mpGraphics->CreatePixelFormat(aaszFormat);
    niCheck(pxf.IsOK(),NULL);
    return niNew cMetalTexture(
      mMetalDevice,
      mpGraphics,
      ahspName,
      aType,
      anWidth,anHeight,anDepth,
      anNumMipMaps,
      pxf,
      aFlags);
  }

  virtual tBool __stdcall ClearTextureRect(iTexture* apTexture, tU32 anLevel, const sRecti& aRect, tU32 anColor)  {
    return eTrue;
  }
  virtual tBool __stdcall ClearTextureRectf(iTexture* apTexture, tU32 anLevel, const sRecti& aRect, const sColor4f& aColor)  {
    return eTrue;
  }
  virtual tBool __stdcall BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags)  {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apDest->GetType() == eBitmapType_2D, eFalse);
    cMetalTexture* tex = (cMetalTexture*)apDest;
    Ptr<iBitmap2D> texBmp = tex->GetBitmap();
    niCheckSilent(niIsOK(texBmp),eFalse);
    texBmp = texBmp->GetLevel(anDestLevel);
    niCheckSilent(niIsOK(texBmp),eFalse);
    texBmp->BlitStretch(
      apSrc,
      aSrcRect.x,aSrcRect.y,
      aDestRect.x,aDestRect.y,
      aSrcRect.GetWidth(),aSrcRect.GetHeight(),
      aDestRect.GetWidth(),aDestRect.GetHeight());
    if (tex->_tex) {
      tex->UploadTexture(texBmp->GetPixelFormat(),
                         texBmp, anDestLevel,
                         aDestRect);
    }
    METAL_TRACE(("BlitBitmapToTexture %s %s %s", aSrcRect, aDestRect, apDest->GetDeviceResourceName()));
    return eTrue;
  }
  virtual tBool __stdcall BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags)  {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apSrc->GetType() == eBitmapType_2D, eFalse);

    cMetalTexture* tex = (cMetalTexture*)apSrc;
    Ptr<iBitmap2D> texBmp = tex->GetBitmap();
    niCheckSilent(niIsOK(texBmp),eFalse);
    apDest->BlitStretch(
      texBmp,
      aSrcRect.x,aSrcRect.y,
      aDestRect.x,aDestRect.y,
      aSrcRect.GetWidth(),aSrcRect.GetHeight(),
      aDestRect.GetWidth(),aDestRect.GetHeight());
    METAL_TRACE(("BlitTextureToBitmap %s %s %s", aSrcRect, aDestRect, apSrc->GetDeviceResourceName()));
    return eTrue;
  }
  virtual tBool __stdcall BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags)  {
    niCheckSilent(niIsOK(apSrc),eFalse);
    niCheckSilent(niIsOK(apDest),eFalse);
    niCheckSilent(apSrc->GetType() == eBitmapType_2D, eFalse);
    niCheckSilent(apDest->GetType() == eBitmapType_2D, eFalse);
    cMetalTexture* sTex = (cMetalTexture*)apSrc;
    Ptr<iBitmap2D> sTexBmp = sTex->GetBitmap();
    niCheckSilent(niIsOK(sTexBmp),eFalse);
    cMetalTexture* dTex = (cMetalTexture*)apDest;
    Ptr<iBitmap2D> dTexBmp = dTex->GetBitmap();
    niCheckSilent(niIsOK(dTexBmp),eFalse);
    dTexBmp->BlitStretch(
      sTexBmp,
      aSrcRect.x,aSrcRect.y,
      aDestRect.x,aDestRect.y,
      aSrcRect.GetWidth(),aSrcRect.GetHeight(),
      aDestRect.GetWidth(),aDestRect.GetHeight());
    METAL_TRACE(("BlitTextureToTexture %s %s %s", aSrcRect, aDestRect, apSrc->GetDeviceResourceName()));
    return eTrue;
  }
  virtual tBool __stdcall BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)  {
    return eFalse;
  }
  virtual tBool __stdcall BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)  {
    return eFalse;
  }

  /////////////////////////////////////////////
  virtual tU32 __stdcall GetNumShaderProfile(eShaderUnit aUnit) const {
    if (aUnit >= eShaderUnit_Last) return 0;
    return (tU32)mvProfiles[aUnit].size();
  }
  virtual iHString* __stdcall GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const {
    if (anIndex >= GetNumShaderProfile(aUnit)) return NULL;
    return mvProfiles[aUnit][anIndex];
  }
  virtual iShader* __stdcall CreateShader(iHString* ahspName, iFile* apFile) {
    if (!niIsOK(apFile)) {
      niError(_A("Invalid file."));
      return NULL;
    }

    if (apFile->ReadLE32() != kfccD3DShader) {
      niError(_A("The file is not a D3D shader."));
      return NULL;
    }

    if (apFile->ReadLE32() != niMakeVersion(1,0,0)) {
      niError(_A("Incompatible version."));
      return NULL;
    }

    apFile->BeginReadBits();
    tU8 nUnit = apFile->ReadBitsPackedU8();
    /*cString strName =*/ apFile->ReadBitsString();
    tHStringPtr hspProfileName = _H(apFile->ReadBitsString());
    tU32 nBufferSize = apFile->ReadBitsPackedU32();
    tBool bHasConstants = apFile->ReadBit();
    apFile->EndReadBits();

    astl::vector<tU8> vData;
    vData.resize(nBufferSize);
    apFile->ReadRaw(&vData[0],nBufferSize);

    if (!HasShaderProfile(this,nUnit,hspProfileName)) {
      niError(niFmt(_A("Profile '%s' is not available, can't load the program."),hspProfileName));
      return NULL;
    }

    const MOJOSHADER_parseData *shaderPD = MOJOSHADER_parse(
      MOJOSHADER_PROFILE_METAL,
      &vData[0],
      nBufferSize, NULL, 0,
      NULL,
      NULL,
      NULL);
    niDefer { MOJOSHADER_freeParseData(shaderPD); };
    ni::cString code(shaderPD->output, shaderPD->output_len);

    auto func = mFixedPipelines->CompileShader(
      this,
      nUnit == eShaderUnit_Pixel ? eGpuFunctionType_Pixel : eGpuFunctionType_Vertex,
      HFmt("gpufunc_%s",ahspName),
      code.Chars());
    niCheckIsOK_(func,"Can't compile shader.",nullptr);
    Ptr<iShader> ptrProg = mFixedPipelines->CreateFixedGpuShader(
      mpGraphics,func,ahspName);
    if (!ptrProg.IsOK()) {
      niError("Can't create the shader instance.");
      return nullptr;
    }

    if (bHasConstants) {
      if (!ni::GetLang()->SerializeObject(
            apFile,(iShaderConstants*)ptrProg->GetConstants(),
            eSerializeMode_ReadRaw,NULL)) {
        niError(_A("Can't load the shader constants."));
        return NULL;
      }
    }

    return ptrProg.GetRawAndSetNull();
  }
  virtual tBool __stdcall IsShader(iFile* apFile) const {
    return eFalse;
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
  virtual iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF aFVF, eArrayUsage aUsage) {
    return CreateFixedGpuVertexArray(this,anNumVertices,aFVF,aUsage);
  }
  virtual iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndices, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
    return CreateFixedGpuIndexArray(this,aPrimitiveType,anNumIndices,anMaxVertexIndex,aUsage);
  }

  /////////////////////////////////////////////
  id<MTLSamplerState> _ssCompiled[(eCompiledStates_SS_SmoothWhiteBorder-eCompiledStates_SS_PointRepeat)+1];
  std::map<tU32,id<MTLSamplerState> > _ssMap;

  void _InitCompiledSamplerStates() {
    MTLSamplerDescriptor* desc = [MTLSamplerDescriptor new];

#define INIT_COMPILED_SAMPLER_STATES(STATE,FILTER,WRAP) {               \
      const tU32 ssKey = niFourCC(eSamplerFilter_##FILTER, eSamplerWrap_##WRAP, eSamplerWrap_##WRAP, eSamplerWrap_##WRAP); \
      _toMTLSamplerFilter(desc, eSamplerFilter_##FILTER);               \
      desc.sAddressMode = desc.tAddressMode = desc.rAddressMode = _toMTLSamplerAddress[eSamplerWrap_##WRAP]; \
      desc.borderColor = MTLSamplerBorderColorOpaqueWhite;              \
      id<MTLSamplerState> ss = [mMetalDevice newSamplerStateWithDescriptor: desc]; \
      _ssCompiled[eCompiledStates_##STATE - eCompiledStates_SS_PointRepeat] = ss; \
      _ssMap[ssKey] = ss;                                               \
    }

    INIT_COMPILED_SAMPLER_STATES(SS_PointRepeat, Point, Repeat);
    INIT_COMPILED_SAMPLER_STATES(SS_PointClamp,  Point, Clamp);
    INIT_COMPILED_SAMPLER_STATES(SS_PointMirror, Point, Mirror);
    INIT_COMPILED_SAMPLER_STATES(SS_PointWhiteBorder, Point, Border);

    INIT_COMPILED_SAMPLER_STATES(SS_SmoothRepeat, Smooth, Repeat);
    INIT_COMPILED_SAMPLER_STATES(SS_SmoothClamp,  Smooth, Clamp);
    INIT_COMPILED_SAMPLER_STATES(SS_SmoothMirror, Smooth, Mirror);
    INIT_COMPILED_SAMPLER_STATES(SS_SmoothWhiteBorder, Smooth, Border);
  }

  inline id<MTLSamplerState> _GetMTLSamplerState(tIntPtr ahSS) {
    if (ahSS >= eCompiledStates_SS_PointRepeat && ahSS <= eCompiledStates_SS_SmoothWhiteBorder) {
      return _ssCompiled[ahSS-eCompiledStates_SS_PointRepeat];
    }
    return _ssCompiled[0];
  }

  /////////////////////////////////////////////
  id<MTLDepthStencilState> _dsNoDepthTest;
  id<MTLDepthStencilState> _dsDepthTestAndWrite;
  id<MTLDepthStencilState> _dsDepthTestOnly;

  void _InitCompiledDepthStencilStates() {
    MTLDepthStencilDescriptor* desc = [MTLDepthStencilDescriptor new];

    {
      desc.depthCompareFunction = _toMTLCompareFunction[eGraphicsCompare_Always];
      desc.depthWriteEnabled = NO;
      _dsNoDepthTest = [mMetalDevice newDepthStencilStateWithDescriptor: desc];
      niAssert(_dsNoDepthTest != nil);
    }

    {
      desc.depthCompareFunction = _toMTLCompareFunction[eGraphicsCompare_LessEqual];
      desc.depthWriteEnabled = YES;
      _dsDepthTestAndWrite = [mMetalDevice newDepthStencilStateWithDescriptor: desc];
      niAssert(_dsDepthTestAndWrite != nil);
    }

    {
      desc.depthCompareFunction = _toMTLCompareFunction[eGraphicsCompare_LessEqual];
      desc.depthWriteEnabled = NO;
      _dsDepthTestOnly = [mMetalDevice newDepthStencilStateWithDescriptor: desc];
      niAssert(_dsDepthTestOnly != nil);
    }
  }

  inline id<MTLDepthStencilState> _GetMTLDepthStencilState(tIntPtr ahDS) {
    if (!ahDS || ahDS == eCompiledStates_DS_NoDepthTest) {
      return _dsNoDepthTest;
    }
    if (ahDS == eCompiledStates_DS_DepthTestAndWrite) {
      return _dsDepthTestAndWrite;
    }
    else if (ahDS == eCompiledStates_DS_DepthTestOnly) {
      return _dsDepthTestOnly;
    }
    return _dsNoDepthTest;
  }

  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBuffer(tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl {
    niLet buffer = MakeNN<sMetalBuffer>();
    if (!buffer->_Create(mMetalDevice, nullptr, anSize, aMemMode, aUsage)) {
      return nullptr;
    }
    return buffer;
  }

  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromData(iFile* apFile, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl {
    niCheckIsOK(apFile,nullptr);
    astl::vector<tU8> data;
    data.resize(anSize);
    if (apFile->ReadRaw(data.data(),anSize) != anSize) {
      return nullptr;
    }
    return this->CreateGpuBufferFromDataRaw(data.data(),anSize,aMemMode,aUsage);
  }

  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromDataRaw(tPtr apData, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl {
    niCheck(apData != nullptr, nullptr);
    niLet buffer = MakeNN<sMetalBuffer>();
    if (!buffer->_Create(mMetalDevice, apData, anSize, aMemMode, aUsage)) {
      return nullptr;
    }
    return buffer;
  }

  virtual iHString* __stdcall GetGpuFunctionTarget() const {
    return _GetGpuFunctionTarget();
  }

  virtual Ptr<iGpuFunction> __stdcall CreateGpuFunction(eGpuFunctionType aType, iHString* ahspName, iDataTable* apGpuFunctionDT) niImpl {
    NN<sMetalFunction> func = MakeNN<sMetalFunction>();
    if (!func->_Create(mMetalDevice, ahspName, apGpuFunctionDT)) {
      return nullptr;
    }
    if (func->GetFunctionType() != aType) {
      niError(niFmt(
        "Expected function type '%s' by got '%s'.",
        (tU32)aType, (tU32)func->GetFunctionType()));
      return nullptr;
    }
    return func;
  }

  virtual Ptr<iGpuPipelineDesc> __stdcall CreateGpuPipelineDesc() niImpl {
    return ni::_CreateGpuPipelineDesc();
  }

  virtual Ptr<iGpuPipeline> __stdcall CreateGpuPipeline(const iGpuPipelineDesc* apDesc) niImpl {
    NN<sMetalPipeline> pipeline = MakeNN<sMetalPipeline>();
    if (!pipeline->_Create(mMetalDevice,apDesc)) {
      return nullptr;
    }
    return pipeline;
  }

  virtual Ptr<iGpuBlendMode> __stdcall CreateGpuBlendMode() niImpl {
    return ni::_CreateGpuBlendMode();
  }

  virtual tBool BlitManagedGpuBufferToSystemMemory(iGpuBuffer* apBuffer) niImpl {
    niPanicUnreachable("Unimplemented");
    return eFalse;
  }
};

niExportFunc(iUnknown*) New_GraphicsDriver_Metal(const Var& avarA, const Var& avarB) {
  QPtr<iGraphics> ptrGraphics = avarA;
  niCheckIsOK(ptrGraphics,NULL);
  id<MTLDevice> device = (__bridge id<MTLDevice>)osxMetalGetDevice();
  if (!device) {
    niLog(Info,"Metal graphics driver is creating the default system device.");
    osxMetalSetDefaultDevice();
    device = (__bridge id<MTLDevice>)osxMetalGetDevice();
  }
  if (!device) {
    niError("No metal device found.");
  }
  Ptr<iGraphicsDriver> driver = niNew cMetalGraphicsDriver(ptrGraphics,device);
  return driver.GetRawAndSetNull();
}

//----------------------------------------------------------------------------
//
// sMetalCommandEncoder
//
//----------------------------------------------------------------------------
struct sMetalCommandEncoder : public ImplRC<iGpuCommandEncoder> {
  nn<cMetalGraphicsDriver> _driver;
  id<MTLRenderCommandEncoder> _encoder;
  Ptr<sMetalBuffer> _indexBuffer;
  tU32 _indexOffset = 0;

  sMetalCommandEncoder(
    ain<nn<cMetalGraphicsDriver>> aDriver)
      : _driver(aDriver)
  {}

  virtual void __stdcall SetPipeline(iGpuPipeline* apPipeline) niImpl {
    niCheck(apPipeline != nullptr,;);
    [_encoder setRenderPipelineState:
     ((sMetalPipeline*)apPipeline)->_mtlPipeline];
  }

  virtual void __stdcall SetVertexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) niImpl {
    niCheck(apBuffer != nullptr, ;);
    sMetalBuffer* buffer = (sMetalBuffer*)apBuffer;
    [_encoder setVertexBuffer:buffer->_mtlBuffer offset:anOffset atIndex:anBinding];
  }

  virtual void __stdcall SetIndexBuffer(iGpuBuffer* apBuffer, tU32 anOffset) niImpl {
    niCheck(apBuffer != nullptr, ;);
    // Metal handles this in DrawIndexed
    _indexBuffer = (sMetalBuffer*)apBuffer;
    _indexOffset = anOffset;
  }

  virtual void __stdcall SetUniformBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) niImpl {
    niCheck(apBuffer != nullptr, ;);
    sMetalBuffer* buffer = (sMetalBuffer*)apBuffer;
    [_encoder setVertexBuffer:buffer->_mtlBuffer offset:anOffset atIndex:anBinding];
    [_encoder setFragmentBuffer:buffer->_mtlBuffer offset:anOffset atIndex:anBinding];
  }

  virtual void __stdcall SetTexture(iTexture* apTexture, tU32 anBinding) niImpl {
    niCheck(apTexture != nullptr, ;);
    cMetalTexture* tex = (cMetalTexture*)apTexture;
    [_encoder setFragmentTexture:tex->_tex atIndex:anBinding];
  }

  virtual void __stdcall SetSamplerState(tIntPtr ahSS, tU32 anBinding) niImpl {
    [_encoder
     setFragmentSamplerState:_driver->_GetMTLSamplerState(ahSS)
     atIndex:anBinding];
  }

  virtual void __stdcall SetPolygonOffset(const sVec2f& avOffset) niImpl {
    [_encoder setDepthBias:avOffset.x
     slopeScale:avOffset.y
     clamp:0.0f];
  }

  virtual void __stdcall SetScissorRect(const sRecti& aRect) niImpl {
    MTLScissorRect rect;
    rect.x = aRect.x;
    rect.y = aRect.y;
    rect.width = aRect.GetWidth();
    rect.height = aRect.GetHeight();
    [_encoder setScissorRect:rect];
  }

  virtual void __stdcall SetViewport(const sRecti& aRect) niImpl {
    MTLViewport vp;
    vp.originX = aRect.x;
    vp.originY = aRect.y;
    vp.width = aRect.GetWidth();
    vp.height = aRect.GetHeight();
    vp.znear = 0.0f;
    vp.zfar = 1.0f;
    [_encoder setViewport:vp];
  }

  virtual void __stdcall SetStencilReference(tI32 aRef) niImpl {
    [_encoder setStencilReferenceValue:aRef];
  }

  virtual void __stdcall SetStencilMask(tU32 aMask) niImpl {
    [_encoder setStencilFrontReferenceValue:aMask
     backReferenceValue:aMask];
  }

  virtual void __stdcall SetBlendColorConstant(const sColor4f& aColor) niImpl {
    [_encoder setBlendColorRed:aColor.x
     green:aColor.y
     blue:aColor.z
     alpha:aColor.w];
  }

  virtual tBool __stdcall DrawIndexed(eGraphicsPrimitiveType aPrimType, eGpuIndexType aIndexType, tU32 anNumIndices, tU32 anFirstIndex) niImpl
  {
    niCheck(aPrimType <= eGraphicsPrimitiveType_Last, eFalse);
    niCheck(aIndexType <= eGpuIndexType_Last, eFalse);
    [_encoder drawIndexedPrimitives:_toMTLPrimitiveType[aPrimType]
     indexCount:anNumIndices
     indexType:_toMTLIndexType[aIndexType]
     indexBuffer:_indexBuffer->_mtlBuffer
     indexBufferOffset:_indexOffset + (anFirstIndex * sizeof(tU32))];
    return eTrue;
  }

  virtual tBool __stdcall Draw(eGraphicsPrimitiveType aPrimType, tU32 anVertexCount, tU32 anFirstVertex) niImpl
  {
    niCheck(aPrimType <= eGraphicsPrimitiveType_Last, eFalse);
    [_encoder drawPrimitives:_toMTLPrimitiveType[aPrimType]
     vertexStart:anFirstVertex
     vertexCount:anVertexCount];
    return eTrue;
  }
};

//--------------------------------------------------------------------------------------------
//
// MetalContextWindow
//
//--------------------------------------------------------------------------------------------
#if defined METAL_MAC
static const tU32 knUniformBufferAlignment = 256;
#elif defined METAL_IOS
static const tU32 knUniformBufferAlignment = 16;
#endif

#define DIRTY_VIEWPORT niBit(0)
#define DIRTY_SCISSOR niBit(1)


struct sUniformsFixed {
  sMatrixf mtxViewProjection;
  sMatrixf mtxDrawOp;
  sVec4f matColorDiffuse;
  sVec4f matColorOpacity;
} niAligned(256);

struct cMetalContextBase :
    public sGraphicsContext<1,ni::ImplRC<
                                iGraphicsContextRT,
                                eImplFlags_DontInherit1,iGraphicsContext,
                                iRunnable,iGraphicsContextGpu> >
{
  cMetalGraphicsDriver* mpParent;
  MTLViewport mViewport;
  MTLScissorRect mScissorRect;

  NN<sMetalCommandEncoder> mCmdEncoder;

  sFixedGpuPipelineId mBaseRenderPipelineId;

  tIntPtr mFixedShaderVertexFVFPosColor;
  tIntPtr mFixedShaderVertexFVFPosNormalColor;
  tIntPtr mFixedShaderVertexFVFPosNormalTex1;
  tIntPtr mFixedShaderVertexFVFRigid;
  tIntPtr mFixedShaderVertexFVFRigid2;
  tIntPtr mFixedShaderVertexFVFSkinned;
  tIntPtr mFixedShaderFragSolid, mFixedShaderFragTex, mFixedShaderFragTexAlpha;

  double4 mClearColor = double4::Zero();
  tF32 mClearDepth = 1.0f;
  tI32 mClearStencil = 0;

  tBool mbBeganFrame = eFalse;

  const tU32 mFrameMaxInFlight;
  ThreadSem mFrameSem = ThreadSem(0);
  virtual ni::Var __stdcall Run() niImpl {
    mFrameSem.Signal();
    return niVarNull;
  }

  void _CleanupTrackedBuffers() {
    // niDebugFmt(("... CommandQueue mvTrackedBuffers: %d", mvTrackedBuffers.size()));
    niLoop(i,mvTrackedBuffers.size()) {
      mvTrackedBuffers[i]->_Untrack();
    }
    mvTrackedBuffers.clear();
  }

  tU32 mDirtyFlags = 0;
  tIntPtr mCurrentRS = eInvalidHandle;
  tIntPtr mCurrentDS = eInvalidHandle;
  tIntPtr mCurrentSS = eInvalidHandle;
  tU32 mCurrentBufferOffset = 0;
  sFixedGpuPipelineId mCurrentRenderPipelineId;
  tU32 mNumDrawOps = 0;
  astl::vector<Ptr<sMetalBuffer>> mvTrackedBuffers;

  cMetalContextBase(cMetalGraphicsDriver* apParent, const tU32 aFrameMaxInFlight)
      : tGraphicsContextBase(apParent->GetGraphics())
      , mFrameMaxInFlight(aFrameMaxInFlight)
      , mCmdEncoder(MakeNN<sMetalCommandEncoder>(as_nn(apParent)))
  {
    mpParent = apParent;
    niAssert(mpParent != NULL);

    mFrameSem.Signal(mFrameMaxInFlight);
    mvTrackedBuffers.reserve(128);
  }

  virtual id<MTLRenderCommandEncoder> __stdcall _NewRenderCommandEncoder() = 0;
  virtual tBool __stdcall _PresentAndCommit(iRunnable* apOnCompleted) = 0;

  virtual iGpuCommandEncoder* __stdcall BeginCommandEncoder() niImpl {
    if (!mbBeganFrame) {
      _BeginFrame();
    }
    return mCmdEncoder;
  }

  virtual void __stdcall EndCommandEncoder() niImpl {
    _EndFrame();
  }

  inline tBool _BeginFrame() {
    niAssert(mbBeganFrame == eFalse);

    // wait on the previous frame to be completed
    {
      mFrameSem.InfiniteWait();
      _CleanupTrackedBuffers();
      mDirtyFlags = DIRTY_VIEWPORT | DIRTY_SCISSOR;
      mNumDrawOps = 0;
      mCurrentBufferOffset = 0;
      mCurrentRenderPipelineId.Clear();
      mCurrentRS = eInvalidHandle;
      mCurrentDS = eInvalidHandle;
      mCurrentSS = eInvalidHandle;
    }

    if (mBaseRenderPipelineId.IsNull()) {
      mBaseRenderPipelineId.rt0 = _GetClosestGpuPixelFormatForRT(mptrRT[0]->GetPixelFormat()->GetFormat());
      if (mptrDS.IsOK()) {
        mBaseRenderPipelineId.ds = _GetClosestGpuPixelFormatForDS(mptrDS->GetPixelFormat()->GetFormat());
      }
    }

    mCmdEncoder->_encoder = _NewRenderCommandEncoder();
    mbBeganFrame = eTrue;
    // niDebugFmt(("... _BeginFrame: %d", ni::GetLang()->GetFrameNumber()));
    return eTrue;
  }

  inline void _EndFrame() {
    if (!mbBeganFrame) {
      return;
    }
    _PresentAndCommit(this);
    mCmdEncoder->_encoder = nil;
    mbBeganFrame = eFalse;
    // niDebugFmt(("... _EndFrame: %d", ni::GetLang()->GetFrameNumber()));
  }

  virtual tBool __stdcall IsOK() const {
    return mptrRT[0].IsOK();
  }

  virtual void __stdcall Invalidate() {
    mpParent = NULL;
  }

  virtual iGraphics* __stdcall GetGraphics() const {
    return mpParent?mpParent->GetGraphics():NULL;
  }
  virtual iGraphicsDriver* __stdcall GetDriver() const {
    return mpParent;
  }

  /////////////////////////////////////////////
  virtual void __stdcall ClearBuffers(tClearBuffersFlags clearBuffer, tU32 anColor, tF32 afDepth, tI32 anStencil) {
    mClearColor = Vec4<tF64>(ULColorGetRf(anColor),ULColorGetGf(anColor),ULColorGetBf(anColor),ULColorGetAf(anColor));
    mClearDepth = afDepth;
    mClearStencil = anStencil;

#if 0
    if (clearBuffer == eClearBuffersFlags_ColorDepthStencil) {
      niAssert(!mbBeganFrame);
      if (mbBeganFrame) {
        static tU32 _lastErrorFrame = ni::eInvalidHandle;
        if (_lastErrorFrame != ni::GetLang()->GetFrameNumber()) {
          niError("Frame already began");
          _lastErrorFrame = ni::GetLang()->GetFrameNumber();
        }
        return;
      }
      _BeginFrame();
    }
#endif
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    niCheckSilent(niIsOK(apDrawOp), eFalse);

    ++mNumDrawOps;

    // niAssert(mbBeganFrame);
    if (!mbBeganFrame) {
#if 0
      static tU32 _lastErrorFrame = ni::eInvalidHandle;
      if (_lastErrorFrame != ni::GetLang()->GetFrameNumber()) {
        niError("Frame not began.");
        _lastErrorFrame = ni::GetLang()->GetFrameNumber();
      }
      return eFalse;
#endif
      niCheck(_BeginFrame(),eFalse);
    }

    niLet cmdEncoder = mCmdEncoder->_encoder;

    iVertexArray* va = apDrawOp->GetVertexArray();
    if (!va) {
      return eFalse;
    }

    METAL_TRACE(("DrawOperation BEGIN %s:%s",this->GetWidth(),this->GetHeight()));
    const sVec2i fvfAndStride = GetVertexArrayFvfAndStride(va);
    const tFVF fvf = (tFVF)fvfAndStride.x;
    const tU32 fvfStride = (tU32)fvfAndStride.y;
    const iMaterial* pDOMat = apDrawOp->GetMaterial();
    const sMaterialDesc* pDOMatDesc = (const sMaterialDesc*)pDOMat->GetDescStructPtr();
    const sMatrixf mtxDrawOp = apDrawOp->GetMatrix();
    const sFixedStatesDesc* fixedDesc = (const sFixedStatesDesc*)mptrFS->GetDescStructPtr();
    const tMaterialFlags matFlags = _GetMaterialFlags(pDOMatDesc);

    // niDebugFmt(("... DrawOperation: frame: %d, base: %d, first: %d, num: %d (%d tris), uoff: %d, uSize: %d",
    // ni::GetLang()->GetFrameNumber(),
    // baseVertexIndex, firstInd, numInds, numInds/3,
    // mCurrentBufferOffset,
    // sizeof(sUniformsFixed)));

    sFixedGpuPipelineId rpId = mBaseRenderPipelineId;

    iShader* pVS = pDOMatDesc->mShaders[eShaderUnit_Vertex];
    iShader* pPS = pDOMatDesc->mShaders[eShaderUnit_Pixel];

    if (!pVS) {
      pVS = mpParent->mFixedShaders.GetVertexShader(fvf,*pDOMatDesc);
      if (!pVS) {
        niError("Can't get fixed vertex shader.");
        return eFalse;
      }
      METAL_TRACE(("VS : %s",pVS->GetDeviceResourceName()));
      FixedShaders_UpdateConstants(this, (iShaderConstants*)pVS->GetConstants(), apDrawOp);
    }
    if (!pPS) {
      pPS = mpParent->mFixedShaders.GetPixelShader(*pDOMatDesc);
      if (!pPS) {
        niError("Can't get fixed pixel shader.");
        return eFalse;
      }
      METAL_TRACE(("PS : %s",pPS->GetDeviceResourceName()));
      FixedShaders_UpdateConstants(this, (iShaderConstants*)pPS->GetConstants(), apDrawOp);
    }

    rpId.vertFuncId = ((iFixedGpuShader*)pVS)->GetUID();
    rpId.fragFuncId = ((iFixedGpuShader*)pPS)->GetUID();
    rpId.blendMode = _GetBlendMode(pDOMatDesc);
    rpId.fvf = fvf;

    {
      const tIntPtr hRS = _GetRS(pDOMatDesc);
      if (mCurrentRS != hRS) {
        mCurrentRS = hRS;
      }
    }

    {
      const tIntPtr hDS = _GetDS(pDOMatDesc);
      if (rpId.ds && (mCurrentDS != hDS)) {
        [cmdEncoder setDepthStencilState:mpParent->_GetMTLDepthStencilState(hDS)];
        mCurrentDS = hDS;
      }
    }

    if (rpId != mCurrentRenderPipelineId) {
      Ptr<iGpuPipeline> pipeline = mpParent->mFixedPipelines->GetRenderPipeline(
        mpParent,
        rpId,
        (iFixedGpuShader*)pVS,
        (iFixedGpuShader*)pPS);
      if (!pipeline.IsOK()) {
        niError("Can't get the render pipeline.");
        return eFalse;
      }
      [cmdEncoder setRenderPipelineState:
       ((sMetalPipeline*)pipeline.raw_ptr())->_mtlPipeline];
      mCurrentRenderPipelineId = rpId;
    }

    if (mDirtyFlags & DIRTY_VIEWPORT) {
      [cmdEncoder setViewport:mViewport];
    }
    if (mDirtyFlags & DIRTY_SCISSOR) {
      [cmdEncoder setScissorRect:mScissorRect];
    }

    auto applyMaterialChannel =
        [&](cMetalContextBase* apContext, iGraphics* apGraphics, tU32 anTSS, eMaterialChannel aChannel, const sMaterialDesc* apDOMat)
        {
          const sMaterialChannel& ch = apContext->_GetChannel(apDOMat, aChannel);
          if (ch.mTexture.IsOK()) {
            cMetalTexture* tex = (cMetalTexture*)ch.mTexture->Bind(NULL);
            if (tex == NULL || tex->_tex == nil) {
              // Probably trying to use the frame buffer we don't support this atm...
              [cmdEncoder setFragmentTexture:NULL atIndex:anTSS];
            }
            else {
              [cmdEncoder setFragmentTexture:tex->_tex atIndex:anTSS];
              // niDebugFmt(("TEX SET >>> %s",tex->GetDeviceResourceName()));
              if (mCurrentSS != ch.mhSS) {
                [cmdEncoder setFragmentSamplerState: mpParent->_GetMTLSamplerState(ch.mhSS) atIndex: anTSS];
                mCurrentSS = ch.mhSS;
              }
            }
          }
          else {
            [cmdEncoder setFragmentTexture:NULL atIndex: anTSS];
          }
        };

    niLoop(i,eMaterialChannel_Last) {
      eMaterialChannel channel = (eMaterialChannel)i;
      if (pDOMatDesc->mChannels[channel].mTexture.IsOK()) {
        const tI8 nSamplerIndex = (tI8)i;
        applyMaterialChannel(
          this,
          mpParent->GetGraphics(),
          nSamplerIndex,
          channel,
          pDOMatDesc);
      }
    }

    auto updateConstant =
        [&](eShaderUnit aUnit, const sShaderConstantsDesc* constDesc) {
          tU32 size = 0;
          sVec4f vf[constDesc->mvFloatRegisters.size()];
          for (auto c : constDesc->mvConstants) {
            auto cd = constDesc;
            iHString* hspName = c.hspName;
            // niDebugFmt(("Bind Constant[%s] %s,%s,%s,%s,%s",hspName,c.Type,c.hspMetadata,c.nDataIndex,c.nSize,c.nHwIndex));
            if (hspName) {
              auto matConst = pDOMatDesc->mptrConstants;
              if (matConst.IsOK()) {
                // niDebugFmt(("material is ok"));
                const sShaderConstantsDesc* ct = (const sShaderConstantsDesc*)matConst->GetDescStructPtr();
                sShaderConstantsDesc::tConstMap::const_iterator it = ct->mmapConstants.find(hspName);
                if (it != ct->mmapConstants.end() && it->second != eInvalidHandle) {
                  // niDebugFmt(("Find const material %s",hspName));
                  c = ct->mvConstants[it->second];
                  cd = ct;
                }
              }
            }

            switch (c.Type) {
              case eShaderRegisterType_ConstFloat: {
                niLoop(i, c.nSize) {
                  vf[i + c.nHwIndex] = cd->mvFloatRegisters[i + c.nDataIndex];
                }
                size += c.nSize * sizeof(sVec4f);
                break;
              }
              case eShaderRegisterType_ConstInt: {

                break;
              }
              case eShaderRegisterType_ConstBool: {
                break;
              }
              default:
                niAssertUnreachable("Unreachable.");
                return eFalse;
            }
          }

          if (aUnit == eShaderUnit_Vertex) {
            [cmdEncoder setVertexBytes:&vf[0] length:size atIndex:16];
          }
          else if (aUnit == eShaderUnit_Pixel) {
            [cmdEncoder setFragmentBytes:&vf[0] length:size atIndex:16];
          }
          return eTrue;
        };

    updateConstant(eShaderUnit_Vertex, (sShaderConstantsDesc*)pVS->GetConstants()->GetDescStructPtr());
    updateConstant(eShaderUnit_Pixel, (sShaderConstantsDesc*)pPS->GetConstants()->GetDescStructPtr());

    {
      sMetalBuffer* vaBuffer = (sMetalBuffer*)GetVertexArrayGpuBuffer(va);
      if (!vaBuffer->_tracked && vaBuffer->_modifiedSize) {
        mvTrackedBuffers.push_back(vaBuffer);
        vaBuffer->_tracked = eTrue;
      }
    }

    const tU32 baseVertexIndex = apDrawOp->GetBaseVertexIndex();

    sMetalBuffer* vaMtlBuffer = reinterpret_cast<sMetalBuffer*>(va->Bind(nullptr));
    niPanicAssert(vaMtlBuffer != nullptr);
    [cmdEncoder setVertexBuffer:vaMtlBuffer->_mtlBuffer
     offset:(baseVertexIndex*fvfStride)
     atIndex:0];

    iIndexArray* ia = apDrawOp->GetIndexArray();
    if (ia) {
      const tU32 firstInd = apDrawOp->GetFirstIndex();
      tU32 numInds = apDrawOp->GetNumIndices();
      if (!numInds) {
        numInds = ia->GetNumIndices()-firstInd;
      }
      {
        sMetalBuffer* iaBuffer = (sMetalBuffer*)GetIndexArrayGpuBuffer(ia);
        if (!iaBuffer->_tracked && iaBuffer->_modifiedSize) {
          mvTrackedBuffers.push_back(iaBuffer);
          iaBuffer->_tracked = eTrue;
        }
      }

      sMetalBuffer* iaMtlBuffer = reinterpret_cast<sMetalBuffer*>(ia->Bind(nullptr));
      niPanicAssert(vaMtlBuffer != nullptr);
      [cmdEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
       indexCount:numInds
       indexType:MTLIndexTypeUInt32
       indexBuffer:iaMtlBuffer->_mtlBuffer
       indexBufferOffset:(firstInd*sizeof(tU32))];
      METAL_TRACE(("DrawOperation END"));
    }
    else {
      const tU32 nNumVerts = va->GetNumVertices()-baseVertexIndex;
      [cmdEncoder drawPrimitives:MTLPrimitiveTypeTriangle
       vertexStart:0
       vertexCount:nNumVerts];
    }

    mDirtyFlags = 0;
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall Display(tGraphicsDisplayFlags aFlags, const sRecti& aRect) {
    _EndFrame();
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual iBitmap2D* __stdcall CaptureFrontBuffer() const {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetViewport(const sRecti& aVal) {
    if (aVal.GetWidth() == 0 || aVal.GetHeight() == 0) {
      mViewport.originX = 0;
      mViewport.originY = 0;
      mViewport.width = this->GetWidth();
      mViewport.height = this->GetHeight();
    }
    else {
      mViewport.originX = aVal.x;
      mViewport.originY = aVal.y;
      mViewport.width = aVal.GetWidth();
      mViewport.height = aVal.GetHeight();
    }
    mViewport.znear = 0.0f;
    mViewport.zfar = 1.0f;
    mDirtyFlags |= DIRTY_VIEWPORT;
  }
  virtual sRecti __stdcall GetViewport() const {
    return Recti(mViewport.originX,mViewport.originY,mViewport.width,mViewport.height);
  }

  /////////////////////////////////////////////
  virtual void __stdcall SetScissorRect(const sRecti& aVal) {
    if (aVal.GetWidth() == 0 || aVal.GetHeight() == 0) {
      mScissorRect.x = 0;
      mScissorRect.y = 0;
      mScissorRect.width = this->GetWidth();
      mScissorRect.height = this->GetHeight();
    }
    else {
      mScissorRect.x = aVal.x;
      mScissorRect.y = aVal.y;
      mScissorRect.width = aVal.GetWidth();
      mScissorRect.height = aVal.GetHeight();
    }
    mDirtyFlags |= DIRTY_SCISSOR;
  }
  virtual sRecti __stdcall GetScissorRect() const {
    return Recti(mScissorRect.x,mScissorRect.y,mScissorRect.width,mScissorRect.height);
  }
};

struct cMetalContextWindow : public cMetalContextBase
{
  Ptr<iOSXMetalAPI> mptrMetalAPI;
  Ptr<iOSWindow> mptrWindow;
  Ptr<iPixelFormat> mptrBBPxf, mptrDSPxf;

  cMetalContextWindow(
    cMetalGraphicsDriver* apParent,
    const tU32 aFrameMaxInFlight,
    iOSWindow* apWindow,
    iOSXMetalAPI* apAPI,
    const achar* aaszBBPxf,
    const achar* aaszDSPxf,
    tU32 anSwapInterval,
    tTextureFlags anBBFlags)
      : cMetalContextBase(apParent,aFrameMaxInFlight)
  {
    mptrWindow = apWindow;
    niAssert(mptrWindow.IsOK());
    mptrMetalAPI = apAPI;
    niAssert(mptrMetalAPI.IsOK());

    iGraphics* g = mpParent->GetGraphics();

    mptrBBPxf = g->CreatePixelFormat(aaszBBPxf);
    if (!mptrBBPxf.IsOK())
      return;

    mptrDSPxf = g->CreatePixelFormat(aaszDSPxf);
    if (!mptrDSPxf.IsOK())
      return;

    if (!_DoResizeContext()) {
      niError("Can't do the initial context initialization.");
      return;
    }
  }

  virtual id<MTLRenderCommandEncoder> __stdcall _NewRenderCommandEncoder() {
    if (!mptrWindow.IsOK()) {
      return NULL;
    }
    const sVec2i newSize = mptrWindow->GetClientSize();
    if ((newSize.x != this->GetWidth()) || (newSize.y != this->GetHeight())) {
      _DoResizeContext();
    }
    return (__bridge id<MTLRenderCommandEncoder>)mptrMetalAPI->NewRenderCommandEncoder(mClearColor,mClearDepth,mClearStencil);
  }
  virtual tBool __stdcall _PresentAndCommit(iRunnable* apOnCompleted) {
    return mptrMetalAPI->PresentAndCommit(apOnCompleted);
  }

  tBool __stdcall _DoResizeContext() {
    _EndFrame();

    iGraphics* g = mpParent->GetGraphics();
    niCheck(mptrWindow.IsOK(),eFalse);
    const tU32 w = mptrWindow->GetClientSize().x;
    const tU32 h = mptrWindow->GetClientSize().y;

    ni::SafeInvalidate(mptrRT[0].ptr());
    mptrRT[0] = niNew cMetalTexture(
      mpParent->mMetalDevice,g,_H("Metal_MainRT"),eBitmapType_2D,
      w,h,0,0,mptrBBPxf,eTextureFlags_RenderTarget|eTextureFlags_Surface);

    ni::SafeInvalidate(mptrDS.ptr());
    mptrDS = niNew cMetalTexture(
      mpParent->mMetalDevice,g,_H("Metal_MainDS"),eBitmapType_2D,
      w,h,0,0,mptrDSPxf,eTextureFlags_DepthStencil|eTextureFlags_Surface);

    SetViewport(sRecti(0,0,w,h));
    SetScissorRect(sRecti(0,0,w,h));

    niLog(Info, niFmt(
      "Metal Context - Resized [%p]: %dx%d, BB: %s, DS: %s, VP: %s, SC: %s",
      (tIntPtr)this,
      w,h,
      mptrRT[0]->GetPixelFormat()->GetFormat(),
      mptrDS->GetPixelFormat()->GetFormat(),
      GetViewport(),
      GetScissorRect()));
    return eTrue;
  }
};

static iGraphicsContext* New_MetalContextWindow(
  cMetalGraphicsDriver* apParent,
  const tU32 aFrameMaxInFlight,
  iOSWindow* apWindow,
  iOSXMetalAPI* apAPI,
  const achar* aaszBBPxf,
  const achar* aaszDSPxf,
  tU32 anSwapInterval,
  tTextureFlags anBBFlags)
{
  return niNew cMetalContextWindow(apParent,aFrameMaxInFlight,apWindow,apAPI,aaszBBPxf,aaszDSPxf,anSwapInterval,anBBFlags);
}

struct cMetalContextRT : public cMetalContextBase
{
  id<MTLCommandBuffer>        _commandBuffer;
  id<MTLRenderCommandEncoder> _commandEncoder;

  cMetalContextRT(
    cMetalGraphicsDriver* apParent,
    const tU32 aFrameMaxInFlight,
    iTexture* apRT,
    iTexture* apDS)
      : cMetalContextBase(apParent,aFrameMaxInFlight)
  {
    mptrRT[0] = apRT;
    mptrDS = apDS;
    const tU32 w = apRT->GetWidth();
    const tU32 h = apRT->GetHeight();
    SetViewport(sRecti(0,0,w,h));
    SetScissorRect(sRecti(0,0,w,h));
  }

  virtual id<MTLRenderCommandEncoder> __stdcall _NewRenderCommandEncoder() {
    _commandBuffer = [mpParent->_GetCommandQueue() commandBuffer];

    MTLRenderPassDescriptor *passDesc = [MTLRenderPassDescriptor renderPassDescriptor];
    {
      cMetalTexture* rtTex = (cMetalTexture*)mptrRT[0].ptr();
      passDesc.colorAttachments[0].texture = rtTex->_tex;
      niCAssert(sizeof(MTLClearColor) == sizeof(mClearColor));
      passDesc.colorAttachments[0].clearColor = (MTLClearColor&)mClearColor;
      // store all content on the render target
      passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;
      passDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
    }
    if (mptrDS.IsOK()) {
      cMetalTexture* dsTex = (cMetalTexture*)mptrDS.ptr();
      passDesc.depthAttachment.texture = dsTex->_tex;
      passDesc.depthAttachment.clearDepth = mClearDepth;
      passDesc.depthAttachment.loadAction = MTLLoadActionLoad;
      passDesc.depthAttachment.storeAction = MTLStoreActionStore;
    }

    _commandEncoder = [_commandBuffer renderCommandEncoderWithDescriptor:passDesc];
    [_commandBuffer enqueue];
    return _commandEncoder;
  }
  virtual tBool __stdcall _PresentAndCommit(iRunnable* apOnCompleted) {
    const tBool r = (_commandBuffer != NULL);
    if (_commandEncoder) {
      [_commandEncoder endEncoding];
    }

    if (_commandBuffer) {
      if (apOnCompleted) {
        Ptr<iRunnable> onCompleted = apOnCompleted;
        [_commandBuffer addCompletedHandler: ^(id<MTLCommandBuffer> _Nonnull) {
            onCompleted->Run();
          }];
      }

#ifdef niOSX
      // NOTE: iOS don't have synchronizeTexture function, maybe there is no need to, we need some test..
      // we need to synchronize the renderTarget here to get the right texture;
      id<MTLBlitCommandEncoder> blitEncoder = [_commandBuffer blitCommandEncoder];
      cMetalTexture* rtTex = (cMetalTexture*)mptrRT[0].ptr();
      [blitEncoder synchronizeTexture:rtTex->_tex slice:0 level:0];
      [blitEncoder endEncoding];
#endif

      [_commandBuffer commit];
      [_commandBuffer waitUntilCompleted];
    }
    _commandEncoder = NULL;
    _commandBuffer = NULL;
    return r;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall ChangeRenderTarget(tU32 anIndex, iTexture* apRT) niImpl {
    niCheck(_commandEncoder == NULL, eFalse); // can't change the RT while rendering is happening
    return cMetalContextBase::ChangeRenderTarget(anIndex,apRT);
  }
  virtual tBool __stdcall ChangeDepthStencil(iTexture* apDS) niImpl {
    niCheck(_commandEncoder == NULL, eFalse); // can't change the DS while rendering is happening
    return cMetalContextBase::ChangeDepthStencil(apDS);
  }
};

static iGraphicsContextRT* New_MetalContextRT(
  cMetalGraphicsDriver* apParent,
  const tU32 aFrameMaxInFlight,
  iTexture* apRT,
  iTexture* apDS)
{
  niCheckIsOK(apRT, NULL);
  return niNew cMetalContextRT(apParent,aFrameMaxInFlight,apRT,apDS);
}
} // end of namespace ni

#endif // GDRV_METAL
