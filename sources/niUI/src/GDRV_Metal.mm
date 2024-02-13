// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#ifdef __APPLE__
#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>
#import <MetalKit/MetalKit.h>
#endif

#include "stdafx.h"

#ifdef GDRV_METAL

#include <niLang/Platforms/OSX/osxMetal.h>
#include "Graphics.h"
#include "GDRV_Utils.h"
#include "API/niUI_ModuleDef.h"
#include "API/niUI/Utils/BufferCache.h"
#include "../../nicgc/src/mojoshader/mojoshader.h"
#include "FixedShaders.h"
#include <niLang/STL/scope_guard.h>
#include <niLang/Utils/IDGenerator.h>

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

#define METAL_TRACE(aFmt) //niDebugFmt(aFmt)

#define METAL_DEVICE(aDevice) id<MTLDevice> mMetalDevice = (id<MTLDevice>)aDevice->GetDevice();

#define MAX_COMPILED_STATES 1000
#define COMPILED_SS (eCompiledStates_Driver+(MAX_COMPILED_STATES*1))
#define COMPILED_DS (eCompiledStates_Driver+(MAX_COMPILED_STATES*2))

const tU32 knMetalIndexSize = sizeof(tU32);

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
  r += niFmt("  supportsFeatureSet:");

#if defined METAL_IOS
  if ([aDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1]) {
    r += niFmt(" iOS_GPUFamily1_v1,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1]) {
    r += niFmt(" iOS_GPUFamily2_v1,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v2]) {
    r += niFmt(" iOS_GPUFamily1_v2,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v2]) {
    r += niFmt(" iOS_GPUFamily2_v2,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1]) {
    r += niFmt(" iOS_GPUFamily3_v1,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v3]) {
    r += niFmt(" iOS_GPUFamily1_v3,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v3]) {
    r += niFmt(" iOS_GPUFamily2_v3,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2]) {
    r += niFmt(" iOS_GPUFamily3_v2,");
  }
#endif
#if defined METAL_MAC && !defined niIOSMac
  if ([aDevice supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v1]) {
    r += niFmt(" OSX_GPUFamily1_v1,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_OSX_GPUFamily1_v2]) {
    r += niFmt(" OSX_GPUFamily1_v2,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_OSX_ReadWriteTextureTier2]) {
    r += niFmt(" OSX_ReadWriteTextureTier2,");
  }
#endif
#if defined METAL_TVOS
  if ([aDevice supportsFeatureSet:MTLFeatureSet_tvOS_GPUFamily1_v1]) {
    r += niFmt(" tvOS_GPUFamily1_v1,");
  }
  if ([aDevice supportsFeatureSet:MTLFeatureSet_tvOS_GPUFamily1_v2]) {
    r += niFmt(" tvOS_GPUFamily1_v2,");
  }
#endif

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


//--------------------------------------------------------------------------------------------
//
// States & Shaders
//
//-------------------------------------------------------------------------------------------
enum eRenderPipelineColorFormat {
  eRenderPipelineColorFormat_None = 0,
  eRenderPipelineColorFormat_BGRA8 = 1,
  eRenderPipelineColorFormat_RGBA8 = 2,
  eRenderPipelineColorFormat_RGBA16F = 3,
  eRenderPipelineColorFormat_R16F = 4,
  eRenderPipelineColorFormat_R32F = 5,
  eRenderPipelineColorFormat_Last = 6,
  eRenderPipelineColorFormat_ForceDWORD = 0xFFFFFFFF
};
niCAssert(eRenderPipelineColorFormat_Last <= 8);

static const MTLPixelFormat _toMTLColorFormat[] = {
  MTLPixelFormatInvalid,
  MTLPixelFormatBGRA8Unorm,
  MTLPixelFormatRGBA8Unorm,
  MTLPixelFormatRGBA16Float,
  MTLPixelFormatR16Float,
  MTLPixelFormatR32Float,
};
niCAssert(niCountOf(_toMTLColorFormat) == eRenderPipelineColorFormat_Last);

// Commented out, we dont want to use native DXT support on macOS only since its not supported on iOS, which is our primary target
// #define USE_METAL_DXT

static MTLPixelFormat _GetMTLTexturePixelFormat(const iPixelFormat* apPxf) {
  niAssert(apPxf != NULL);
  const achar* fmt = apPxf->GetFormat();
  if (ni::StrEq(fmt,"D16") ||
      ni::StrEq(fmt,"D32") ||
      ni::StrEq(fmt,"D24X8") ||
      ni::StrEq(fmt,"D24S8"))
  {
    // We don't support other depth format because only macOS supports them
    // and iOS is our primary target right now.
    return MTLPixelFormatDepth32Float;
  }

  if (ni::StrEq(fmt,"R8G8B8A8")) {
    return MTLPixelFormatRGBA8Unorm;
  }
  else if (ni::StrEq(fmt,"B8G8R8A8")) {
    return MTLPixelFormatBGRA8Unorm;
  }
  else if (ni::StrEq(fmt,"A8")) {
    return MTLPixelFormatA8Unorm;
  }
  else if (ni::StrEq(fmt,"FR16G16B16A16")) {
    return MTLPixelFormatRGBA16Float;
  }
#ifdef USE_METAL_DXT
  else if (ni::StrEq(fmt,"DXT3")) {
    return MTLPixelFormatBC2_RGBA;
  }
#endif
  return MTLPixelFormatRGBA8Unorm;
}

static const achar* _GetNITexturePixelFormat(MTLPixelFormat format) {
  switch (format) {
    case MTLPixelFormatDepth32Float:
      return "D32";
    case MTLPixelFormatRGBA8Unorm:
      return "R8G8B8A8";
    case MTLPixelFormatBGRA8Unorm:
      return "B8G8R8A8";
    case MTLPixelFormatA8Unorm:
      return "A8";
    case MTLPixelFormatRGBA16Float:
      return "FR16G16B16A16";
#ifdef USE_METAL_DXT
    case MTLPixelFormatBC2_RGBA:
      return "DXT3";
#endif
    default:
      return NULL;
  }
}


static MTLPixelFormat _GetMTLPixelFormat(const iPixelFormat* apPxf) {
  niAssert(apPxf != NULL);
  const achar* fmt = apPxf->GetFormat();
  if (ni::StrEq(fmt,"D16") ||
      ni::StrEq(fmt,"D32") ||
      ni::StrEq(fmt,"D24X8") ||
      ni::StrEq(fmt,"D24S8"))
  {
    return MTLPixelFormatDepth32Float;
  }
  return MTLPixelFormatBGRA8Unorm;
}

enum eRenderPipelineDepthFormat {
  eRenderPipelineDepthFormat_None = 0,
  eRenderPipelineDepthFormat_D32 = 1,
#if defined METAL_IOS
  eRenderPipelineDepthFormat_Last = 2,
#else
  eRenderPipelineDepthFormat_D16 = 2,
  eRenderPipelineDepthFormat_D24S8 = 3,
  eRenderPipelineDepthFormat_Last = 4,
#endif
  eRenderPipelineDepthFormat_ForceDWORD = 0xFFFFFFFF
};
#if defined METAL_IOS
niCAssert(eRenderPipelineDepthFormat_Last == 2);
#else
niCAssert(eRenderPipelineDepthFormat_Last == 4);
#endif

static eRenderPipelineColorFormat _GetRTRenderPipelineColorFormat(const achar* aaszDSFormat) {
  // the default metal RT is using BGRA8, other RT will follow this format so we can reuse those Pipelines
  return eRenderPipelineColorFormat_BGRA8;
}


static eRenderPipelineDepthFormat _GetDSRenderPipelineDepthFormat(const achar* aaszDSFormat) {
  if (niStringIsOK(aaszDSFormat)) {
    return eRenderPipelineDepthFormat_D32;
  }
  return eRenderPipelineDepthFormat_None;
}

static const MTLPixelFormat _toMTLDepthFormat[eRenderPipelineDepthFormat_Last] = {
  MTLPixelFormatInvalid,
  MTLPixelFormatDepth32Float,
#if !defined METAL_IOS
  MTLPixelFormatDepth16Unorm,
  MTLPixelFormatDepth24Unorm_Stencil8,
#endif
};
niCAssert(niCountOf(_toMTLDepthFormat) == eRenderPipelineDepthFormat_Last);

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
      desc.maxAnisotropy = 4;
#if defined METAL_IOS
      desc.lodAverage = YES;
#endif
      break;
    }
    case eSamplerFilter_Sharp: {
      desc.minFilter = MTLSamplerMinMagFilterLinear;
      desc.magFilter = MTLSamplerMinMagFilterLinear;
      desc.mipFilter = MTLSamplerMipFilterLinear;
      desc.maxAnisotropy = 8;
#if defined METAL_IOS
      desc.lodAverage = NO;
#endif
      break;
    }
    case eSamplerFilter_Point: {
      desc.minFilter = MTLSamplerMinMagFilterNearest;
      desc.magFilter = MTLSamplerMinMagFilterNearest;
      desc.mipFilter = MTLSamplerMipFilterNearest;
      desc.maxAnisotropy = 4;
#if defined METAL_IOS
      desc.lodAverage = YES;
#endif
      break;
    }
    case eSamplerFilter_SharpPoint: {
      desc.minFilter = MTLSamplerMinMagFilterNearest;
      desc.magFilter = MTLSamplerMinMagFilterNearest;
      desc.mipFilter = MTLSamplerMipFilterNearest;
      desc.maxAnisotropy = 8;
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

static const MTLBlendFactor _toMTLBlendFactor[][2] = {
#define MTL_BLENDFACTOR_ZERO 0
  { MTLBlendFactorZero,                     MTLBlendFactorZero                     },
#define MTL_BLENDFACTOR_ONE 1
  { MTLBlendFactorOne,                      MTLBlendFactorOne                      },
#define MTL_BLENDFACTOR_SRCCOLOR 2
  { MTLBlendFactorSourceColor,              MTLBlendFactorSourceAlpha              },
#define MTL_BLENDFACTOR_INVSRCCOLOR 3
  { MTLBlendFactorOneMinusSourceColor,      MTLBlendFactorOneMinusSourceAlpha      },
#define MTL_BLENDFACTOR_SRCALPHA 4
  { MTLBlendFactorSourceAlpha,              MTLBlendFactorSourceAlpha              },
#define MTL_BLENDFACTOR_INVSRCALPHA 5
  { MTLBlendFactorOneMinusSourceAlpha,      MTLBlendFactorOneMinusSourceAlpha      },
#define MTL_BLENDFACTOR_DESTALPHA 6
  { MTLBlendFactorDestinationAlpha,         MTLBlendFactorDestinationAlpha         },
#define MTL_BLENDFACTOR_INVDESTALPHA 7
  { MTLBlendFactorOneMinusDestinationAlpha, MTLBlendFactorOneMinusDestinationAlpha },
#define MTL_BLENDFACTOR_DESTCOLOR 8
  { MTLBlendFactorDestinationColor,         MTLBlendFactorDestinationAlpha         },
#define MTL_BLENDFACTOR_INVDESTCOLOR 9
  { MTLBlendFactorOneMinusDestinationColor, MTLBlendFactorOneMinusDestinationAlpha },
#define MTL_BLENDFACTOR_SRCALPHA_SAT 10
  { MTLBlendFactorSourceAlphaSaturated,     MTLBlendFactorOne                      },
#define MTL_BLENDFACTOR_FACTOR 11
  { MTLBlendFactorBlendColor,               MTLBlendFactorBlendColor               },
#define MTL_BLENDFACTOR_INVFACTOR 12
  { MTLBlendFactorOneMinusBlendColor,       MTLBlendFactorOneMinusBlendColor       },
};

struct sMetalBlendMode {
  MTLBlendOperation op;
  MTLBlendFactor    srcRGB, srcA;
  MTLBlendFactor    dstRGB, dstA;
};
#define METAL_BLENDMODE(OP,SRCRGB,SRCALPHA,DESTRGB,DESTALPHA) \
  { MTLBlendOperation##OP, _toMTLBlendFactor[MTL_BLENDFACTOR_##SRCRGB][0], _toMTLBlendFactor[MTL_BLENDFACTOR_##SRCALPHA][1],  _toMTLBlendFactor[MTL_BLENDFACTOR_##DESTRGB][0], _toMTLBlendFactor[MTL_BLENDFACTOR_##DESTALPHA][1] }

static const sMetalBlendMode _toMTLBlendMode[] = {
  // eBlendMode_NoBlending
  METAL_BLENDMODE(Add, ONE, ONE, ZERO, ZERO),
  // eBlendMode_ReplaceAlpha
  METAL_BLENDMODE(Add, ZERO, ZERO, ONE, ONE),
  // eBlendMode_Additive
  METAL_BLENDMODE(Add, ONE, ONE, ONE, ONE),
  // eBlendMode_Modulate
  METAL_BLENDMODE(Add, DESTCOLOR, DESTCOLOR, ZERO, ZERO),
  // eBlendMode_Modulate2x
  METAL_BLENDMODE(Add, DESTCOLOR, DESTCOLOR, SRCCOLOR, SRCCOLOR),
  // eBlendMode_Translucent
  METAL_BLENDMODE(Add, SRCALPHA, SRCALPHA, INVSRCALPHA, INVSRCALPHA),
  // eBlendMode_TranslucentInvAlpha
  METAL_BLENDMODE(Add, INVSRCALPHA, INVSRCALPHA, SRCALPHA, SRCALPHA),
  // eBlendMode_TintedGlass
  METAL_BLENDMODE(Add, SRCCOLOR, SRCCOLOR, INVSRCCOLOR, INVSRCCOLOR),
  // eBlendMode_PreMulAlpha
  METAL_BLENDMODE(Add, ONE, ONE, INVSRCALPHA, INVSRCALPHA),
  // eBlendMode_ModulateReplaceAlpha
  METAL_BLENDMODE(Add, DESTCOLOR, DESTCOLOR, ZERO, ZERO),
  // eBlendMode_Modulate2xReplaceAlpha
  METAL_BLENDMODE(Add, DESTCOLOR, DESTCOLOR, SRCCOLOR, SRCCOLOR),
};
niCAssert(niCountOf(_toMTLBlendMode) == eBlendMode_Last);

// 128 bit unsigned int for pipeline id
// 32 for FVF
// 32 for vertex shader id
// 32 for pixel shader id
// 32 for the reset flags
// cause all metal devices are x64 platform, so we just use unsigned __int128
#define tU128 unsigned __int128

struct sMetalRenderPipelineId {
  tU32 fvf : 32;
  tU32 rt0 : 4;
  tU32 rt1 : 4;
  tU32 rt2 : 4;
  tU32 rt3 : 4;
  tU32 ds : 2;
  tU32 blendMode : 4;
  tU32 colorWriteMask : 4;
  tU32 msaa : 4;
  tU32 alphaToCoverage : 2;
  tU32 vertFuncId : 32;
  tU32 fragFuncId : 32;

  sMetalRenderPipelineId() {
    (tU128&)(*this) = 0;
  }
  tBool IsNull() const {
    return (tU128&)(*this) == 0;
  }
  cString ToString() const {
    return niFmt("fvf: %d, rt: [%d,%d,%d,%d], ds: %d, blendMode: %d, colorWriteMask: %d, msaa: %d, alphaToCoverage: %d, vertFunc: %d, fragFunc: %d",
                 fvf, rt0, rt1, rt2, rt3, ds, blendMode, colorWriteMask, msaa, alphaToCoverage, vertFuncId, fragFuncId);
  }
  operator tU128 () const {
    return (tU128&)(*this);
  }
};
niCAssert(sizeof(sMetalRenderPipelineId) == sizeof(tU128));

template <typename T>
class cMetalShaderBase : public T, public sShaderDesc
{
 public:
  typedef cMetalShaderBase tMetalShaderBase;

  cMetalShaderBase(iGraphics* apGraphics, LocalIDGenerator* apIDGenerator, iHString* ahspName, iDeviceResourceManager* apDevResMan, iHString* ahspProfile, id<MTLFunction> apMetalShader, const astl::vector<tU8>& avData)
      : mvData(avData)
  {
    mpGraphics = apGraphics;
    mhspName = ahspName;
    mhspProfile = ahspProfile;
    mpShader = apMetalShader;
    mptrDevResMan = apDevResMan;
    mptrDevResMan->Register(this);
    mpIDGenerator = apIDGenerator;
    mnUID = apIDGenerator->AllocID();
  }

  ~cMetalShaderBase() {
    Invalidate();
  }

  iHString* __stdcall GetDeviceResourceName() const
  {
    return mhspName;
  }
  virtual tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) {
    return eFalse;
  }
  virtual tBool __stdcall ResetDeviceResource() {
    _DeleteShader();
    return eFalse;
  }

  virtual void __stdcall Invalidate()
  {
    mptrConstants = NULL;
    mpIDGenerator->FreeID(mnUID);
    if (!mptrDevResMan.IsOK()) return;
    mptrDevResMan->Unregister(this);
    mptrDevResMan = NULL;
    _DeleteShader();
  }

  void _DeleteShader() {
    if (mpShader) {
      mpShader = NULL;
    }
  }

  iHString* __stdcall GetProfile() const
  {
    return mhspProfile;
  }

  iShaderConstants* __stdcall GetConstants() const
  {
    if (!mptrConstants.IsOK()) {
      niThis(cMetalShaderBase)->mptrConstants = mpGraphics->CreateShaderConstants(4096);
    }
    return mptrConstants;
  }

  id<MTLFunction> __stdcall GetShader() const {
    return mpShader;
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

  tU32 GetUID() const {
    return mnUID;
  }

  ///////////////////////////////////////////////
  iDeviceResource* __stdcall Bind(iUnknown*) {
    return this;
  }

 private:
  iGraphics*      mpGraphics;
  Ptr<iDeviceResourceManager>   mptrDevResMan;
  id<MTLFunction> mpShader;
  astl::vector<tU8> mvData;
  tU32 mnUID;
  LocalIDGenerator* mpIDGenerator;
};


struct iMetalShader : public iShader {
  niDeclareInterfaceUUID(iMetalShader,0x27897b00,0x0c78,0x4815,0x92,0x2e,0x17,0x38,0x5f,0xef,0x23,0x97);
  virtual id<MTLFunction> __stdcall GetShader() const = 0;
};


// Vertex program
class cMetalShaderVertex :
    public cMetalShaderBase<cIUnknownImpl<iMetalShader,eIUnknownImplFlags_DontInherit1|eIUnknownImplFlags_DontInherit2,iShader,iDeviceResource> >
{
 public:
  cMetalShaderVertex(iGraphics* apGraphics, LocalIDGenerator* apIDGenerator, iHString* ahspName, iDeviceResourceManager* apDevResMan, iHString* ahspProfile, id<MTLFunction> apShader, const astl::vector<tU8>& avData)
    : tMetalShaderBase(apGraphics,apIDGenerator,ahspName,apDevResMan,ahspProfile,apShader,avData)
  {
  }
  ~cMetalShaderVertex() {
  }
  eShaderUnit __stdcall GetUnit() const {
    return eShaderUnit_Vertex;
  }
};

// Pixel program
class cMetalShaderPixel :
    public cMetalShaderBase<cIUnknownImpl<iMetalShader,eIUnknownImplFlags_DontInherit1|eIUnknownImplFlags_DontInherit2,iShader,iDeviceResource> >
{
 public:
  cMetalShaderPixel(iGraphics* apGraphics, LocalIDGenerator* apIDGenerator, iHString* ahspName, iDeviceResourceManager* apDevResMan, iHString* ahspProfile, id<MTLFunction> apShader, const astl::vector<tU8>& avData)
    : tMetalShaderBase(apGraphics,apIDGenerator,ahspName,apDevResMan,ahspProfile,apShader,avData)
  {
  }
  ~cMetalShaderPixel() {
  }
  eShaderUnit __stdcall GetUnit() const {
    return eShaderUnit_Pixel;
  }
};

struct sMetalShaderLibrary {
  std::map<tU128,id<MTLRenderPipelineState> > _pipelines;
  std::vector<id<MTLFunction> > _funcs;

  id<MTLLibrary> LoadLibrary(id<MTLDevice> aDevice, const achar* aString) {
    auto str = [NSString stringWithUTF8String:aString];
    NSError *error;
    MTLCompileOptions* compileOptions = [MTLCompileOptions new];
    compileOptions.languageVersion = MTLLanguageVersion1_1;
    auto library = [aDevice newLibraryWithSource:str options:compileOptions error:&error];
    if (!library) {
      NSLog(@"Error occurred when compiling shader library: %@", error);
      return NULL;
    }
    return library;
  }

  id<MTLFunction> CompileShader(id<MTLDevice> aDevice, const achar* aShaderProgram, const achar* aShaderFunc) {
    auto library = LoadLibrary(aDevice, aShaderProgram);
    if (library != NULL) {
      NSString* shaderFuncName = [NSString stringWithUTF8String:aShaderFunc];
      id<MTLFunction> shaderFunc = [library newFunctionWithName:library.functionNames[0]];
      if (shaderFunc == NULL) {
        niError(niFmt("CompileShader FIALED function[%s] not found",aShaderFunc));
        niError(niFmt("IN Library[%s]",aShaderProgram));
      }
      // niDebugFmt(("Shader Function Loaded %s",aShaderFunc));
      return shaderFunc;
    }
    niError("CompileShader FIALED library not loaded");
    return NULL;
  }

  id<MTLRenderPipelineState> GetRenderPipeline(id<MTLDevice> aDevice, const tU128 aPipelineId, cMetalShaderVertex* vs, cMetalShaderPixel* ps) {
    std::map<tU128,id<MTLRenderPipelineState> >::iterator it = _pipelines.find(aPipelineId);
    if (it == _pipelines.end()) {
      id<MTLRenderPipelineState> metalRenderPipelineState = _CreatePipeline(
        aDevice,(sMetalRenderPipelineId&)aPipelineId,vs,ps);
      if (!metalRenderPipelineState) {
        // niDebugFmt(("VS %s, PS %s",vs->GetCode(),ps->GetCode()));
        niError("Can't create the metal render pipeline states.");
        return NULL;
      }
      it = _pipelines.insert(std::make_pair(aPipelineId,metalRenderPipelineState)).first;
    }
    METAL_TRACE((">>> MetalRenderPipeline: %s.",((sMetalRenderPipelineId&)aPipelineId).ToString()));
    return it->second;
  }

  id<MTLRenderPipelineState> _CreatePipeline(id<MTLDevice> aDevice, const sMetalRenderPipelineId aIdBits, cMetalShaderVertex* vs, cMetalShaderPixel* ps) {

    if (vs->GetShader() == NULL || ps->GetShader() == NULL) return NULL;
    NSError *error = nil;
    MTLRenderPipelineDescriptor *pipelineDescriptor = [MTLRenderPipelineDescriptor new];
    pipelineDescriptor.vertexFunction = vs->GetShader();
    pipelineDescriptor.fragmentFunction = ps->GetShader();
    {
      pipelineDescriptor.depthAttachmentPixelFormat = _toMTLDepthFormat[aIdBits.ds];
    }
    {
      MTLRenderPipelineColorAttachmentDescriptor* ca = pipelineDescriptor.colorAttachments[0];
      ca.pixelFormat = _toMTLColorFormat[aIdBits.rt0]; //MTLPixelFormatBGRA8Unorm; // pipline for render target can only be BGRA8 in metal
      if (aIdBits.blendMode == 0) {
        ca.blendingEnabled = NO;
      }
      else if (aIdBits.blendMode < eBlendMode_Last) {
        const sMetalBlendMode& bm = _toMTLBlendMode[aIdBits.blendMode];
        ca.blendingEnabled = YES;
        ca.rgbBlendOperation = ca.alphaBlendOperation = bm.op;
        ca.sourceRGBBlendFactor = bm.srcRGB;
        ca.sourceAlphaBlendFactor = bm.srcA;
        ca.destinationRGBBlendFactor = bm.dstRGB;
        ca.destinationAlphaBlendFactor = bm.dstA;
      }
      else {
        ca.blendingEnabled = NO;
        niWarning("Invalid blend mode, falling back to 'no blending'.");
      }
    }

    {
      pipelineDescriptor.vertexDescriptor = _CreateMetalVertDesc(0, aIdBits.fvf);
    }

    id<MTLRenderPipelineState> pipeline = [aDevice newRenderPipelineStateWithDescriptor:pipelineDescriptor
                                           error:&error];
    if (!pipeline) {
      NSLog(@"Error occurred when creating render pipeline state: %@", error);
      return NULL;
    }

    METAL_TRACE((">>> MetalRenderPipeline: %s.", aIdBits.ToString()));
    return pipeline;
  }
};

//--------------------------------------------------------------------------------------------
//
// MetalBuffer
//
//--------------------------------------------------------------------------------------------
struct sMetalBuffer {
  id<MTLBuffer> _mtlBuffer;
  tU32 _lockStart = 0, _lockSize = 0;
  eArrayUsage _usage;

  tBool IsOK() const {
    return _mtlBuffer != NULL;
  }

  void Adopt(id<MTLBuffer> aBuffer) {
    _mtlBuffer = aBuffer;
  }

  tBool Create(id<MTLDevice> aDevice, void* apInitialData, tU32 anSize, eArrayUsage aUsage) {
    // Use MTLResourceStorageModeShared atm, other modes seem to be buggy on
    // NVidia cards. At least on hackintosh. It'll have to be changed once we
    // get a mac with a real GPU...
    MTLResourceOptions resOptions = MTLResourceStorageModeShared;
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

  tU32 GetSize() const {
    return [_mtlBuffer length];
  }

  tBool __stdcall IsLocked() const {
    return _lockSize != 0;
  }

  tPtr __stdcall Lock(tU32 anStart, tU32 anSize, eLock aLock) {
    niUnused(aLock);
    niAssert(!IsLocked());
    _lockStart = anStart;
    _lockSize = anSize ? anSize : (GetSize()-anStart);
    niAssert(_lockSize <= this->GetSize());
    niAssert((_lockStart + _lockSize) <= this->GetSize());
    return ((tPtr)[_mtlBuffer contents]) + _lockStart;
  }
  tBool __stdcall Unlock() {
    if (!IsLocked())
      return eFalse;
    _lockStart = _lockSize = 0;
    return eTrue;
  }

  id<MTLBuffer> Bind() {
    return _mtlBuffer;
  }
};

struct cMetalVertexArray : public ni::cIUnknownImpl<iVertexArray> {
  sMetalBuffer _buffer;
  tFVF _fvf;
  tU32 _fvfStride;

  cMetalVertexArray(id<MTLDevice> aDevice, tU32 anNumVertices, tFVF aFVF, eArrayUsage aUsage) {
    _fvf = aFVF;
    _fvfStride = FVFGetStride(_fvf);
    _buffer.Create(aDevice, NULL, _fvfStride * anNumVertices, aUsage);

    METAL_TRACE((">>> MetalVertexArray: FVF:%s, NumVertex: %d, Stride: %d, Size: %db (%gMB).",
                 FVFToString(_fvf).Chars(),
                 this->GetNumVertices(),_fvfStride,
                 _fvfStride * anNumVertices,
                 ((tF64)(_fvfStride * anNumVertices))/(1024.0*1024.0)));
  }

  virtual tBool __stdcall IsOK() const niImpl {
    return _buffer._mtlBuffer != NULL;
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
    _buffer.Bind();
    return this;
  }

  virtual tFVF __stdcall GetFVF() const {
    return _fvf;
  }
  virtual tU32 __stdcall GetNumVertices() const {
    return _buffer.GetSize() / _fvfStride;
  }
  virtual eArrayUsage __stdcall GetUsage() const {
    return _buffer._usage;
  }
  virtual tPtr __stdcall Lock(tU32 anFirstVertex, tU32 anNumVertex, eLock aLock) {
    return _buffer.Lock(anFirstVertex * _fvfStride, anNumVertex * _fvfStride, aLock);
  }
  virtual tBool __stdcall Unlock() {
    return _buffer.Unlock();
  }
  virtual tBool __stdcall GetIsLocked() const {
    return _buffer.IsLocked();
  }
};

struct cMetalIndexArray : public ni::cIUnknownImpl<iIndexArray> {
  sMetalBuffer _buffer;
  eGraphicsPrimitiveType _primType;

  cMetalIndexArray(id<MTLDevice> aDevice, eGraphicsPrimitiveType aPrimType, tU32 anNumIndices, eArrayUsage aUsage) {
    _primType = aPrimType;
    _buffer.Create(aDevice, NULL, knMetalIndexSize * anNumIndices, aUsage);

    METAL_TRACE((">>> MetalIndexArray: PT: %s, MaxVertexIndex:%d, NumIndices: %d, Stride: %d, Size: %db (%gMB).",
                 niEnumToChars(eGraphicsPrimitiveType,_primType),
                 0xFFFFFFFF,
                 this->GetNumIndices(), knMetalIndexSize,
                 knMetalIndexSize * anNumIndices,
                 ((tF64)(knMetalIndexSize * anNumIndices))/(1024.0*1024.0)));
  }

  virtual tBool __stdcall IsOK() const niImpl {
    return _buffer._mtlBuffer != NULL;
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
    _buffer.Bind();
    return this;
  }

  virtual eGraphicsPrimitiveType __stdcall GetPrimitiveType() const {
    return _primType;
  }
  virtual tU32 __stdcall GetNumIndices() const {
    return _buffer.GetSize() / knMetalIndexSize;
  }
  virtual tU32 __stdcall GetMaxVertexIndex() const {
    return 0xFFFFFFFF;
  }
  virtual eArrayUsage __stdcall GetUsage() const {
    return _buffer._usage;
  }

  virtual tPtr __stdcall Lock(tU32 anFirstIndex, tU32 anNumIndex, eLock aLock) {
    return _buffer.Lock(anFirstIndex * knMetalIndexSize, anNumIndex * knMetalIndexSize, aLock);
  }
  virtual tBool __stdcall Unlock() {
    return _buffer.Unlock();
  }
  virtual tBool __stdcall GetIsLocked() const {
    return _buffer.IsLocked();
  }
};

//--------------------------------------------------------------------------------------------
//
// MetalTexture
//
//--------------------------------------------------------------------------------------------
struct cMetalTexture : public ni::cIUnknownImpl<iTexture,eIUnknownImplFlags_DontInherit1,iDeviceResource>
{
  iGraphics* mpGraphics;
  tHStringPtr mhspName;
  const eBitmapType mType;
  const tU32 mnW, mnH, mnD;
  const tU32 mnMM;
  Ptr<iPixelFormat> mptrPxf;
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
      , mptrPxf(apPxf)
      , mnFlags(anFlags)
  {
    if (niFlagIsNot(anFlags,eTextureFlags_Surface)) {
      MTLPixelFormat pixFmt = _GetMTLTexturePixelFormat(apPxf);
      cString fmt = _GetNITexturePixelFormat(pixFmt);
      if (!fmt.Eq(apPxf->GetFormat())) {
        niWarning(niFmt("Texture format %s not supported by metal, convert to %s",apPxf->GetFormat(),fmt));
        mptrPxf = apGraphics->CreatePixelFormat(fmt.Chars());
      }

      METAL_TRACE(("## METAL-TEXTURE [%s] fmt:%s w:%s h:%s",ahspName,apPxf->GetFormat(),mnW,mnH));
      if (pixFmt == MTLPixelFormatInvalid) {
        niError(niFmt("Can't get compatible MTLPixelFormat for '%s'.", apPxf->GetFormat()));
        return;
      }
      if (niFlagIs(anFlags,eTextureFlags_RenderTarget)) {
        pixFmt = MTLPixelFormatBGRA8Unorm;
      }

      MTLTextureDescriptor*  desc = [MTLTextureDescriptor new];
      desc.width = anW;
      desc.height = anH;
      desc.pixelFormat = pixFmt;
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
            mptrBitmap = apGraphics->CreateBitmap2DEx(mnW,mnH,mptrPxf);
            break;
          case eBitmapType_Cube:
            mptrBitmap = apGraphics->CreateBitmapCubeEx(mnW,mptrPxf);
            break;
          case eBitmapType_3D:
            mptrBitmap = apGraphics->CreateBitmap3DEx(mnW,mnH,mnD,mptrPxf);
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
                                            mnW,mnH,mnD,mnMM,mptrPxf,eTextureFlags_SubTexture|eTextureFlags_Surface);
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
    return mptrPxf;
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
    if (!niIsOK(mptrBitmap)) {
      Ptr<iBitmap2D> bmp = mpGraphics->CreateBitmap2DEx(mnW, mnH, mptrPxf);
      [_tex getBytes:bmp->GetData() bytesPerRow:bmp->GetPitch() fromRegion:MTLRegionMake2D(0, 0, mnW, mnH) mipmapLevel:mnMM];
      return bmp;
    }
    return (iBitmap2D*)mptrBitmap.ptr();
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

struct cMetalGraphicsDriver : public cIUnknownImpl<iGraphicsDriver>
{
  iGraphics* mpGraphics;
  id<MTLDevice> mMetalDevice;
  id<MTLCommandQueue> mMetalCommandQueue;
  sFixedShaders mFixedShaders;
  astl::vector<Ptr<iHString>> mvProfiles[eShaderUnit_Last];
  sMetalShaderLibrary mLibrary;
  LocalIDGenerator mIDGenerator;

  cMetalGraphicsDriver(iGraphics* apGraphics, id<MTLDevice> aDevice) {
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
        mtkView.depthStencilPixelFormat = _toMTLDepthFormat[_GetDSRenderPipelineDepthFormat(aaszDSFormat)];
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
    }
    return 0;
  }


  /////////////////////////////////////////////
  virtual tBool __stdcall ResetAllCaches() {
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tGraphicsDriverImplFlags __stdcall GetGraphicsDriverImplFlags() const {
    return eGraphicsDriverImplFlags_IndexArrayObject|
        eGraphicsDriverImplFlags_VertexArrayObject|
        eGraphicsDriverImplFlags_CompileDepthStencilStates;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall GetIsInitialized() const {
    return eTrue;
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags) {
    niCheckSilent(niIsOK(apFormat),eFalse);
    MTLPixelFormat mtlFormat;
    if (niFlagIs(aFlags,eTextureFlags_RenderTarget) || niFlagIs(aFlags,eTextureFlags_DepthStencil))
    {
      mtlFormat = _GetMTLPixelFormat(apFormat->GetPixelFormat());
    }
    else {
      mtlFormat = _GetMTLTexturePixelFormat(apFormat->GetPixelFormat());
    }

    const achar* fmt = _GetNITexturePixelFormat(mtlFormat);
    if (StrEq(fmt, apFormat->GetPixelFormat()->GetFormat()))
    {
      return eTrue;
    }
    else {
      apFormat->SetPixelFormat(mpGraphics->CreatePixelFormat(fmt));
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
    iBitmap2D* texBmp = tex->GetBitmap();
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
    iBitmap2D* texBmp = tex->GetBitmap();
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
    iBitmap2D* sTexBmp = sTex->GetBitmap();
    niCheckSilent(niIsOK(sTexBmp),eFalse);
    cMetalTexture* dTex = (cMetalTexture*)apDest;
    iBitmap2D* dTexBmp = dTex->GetBitmap();
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

    const MOJOSHADER_parseData *shaderPD = MOJOSHADER_parse(MOJOSHADER_PROFILE_METAL,
                                                            &vData[0],
                                                            nBufferSize, NULL, 0,
                                                            NULL,
                                                            NULL,
                                                            NULL);
    niDefer { MOJOSHADER_freeParseData(shaderPD); };
    ni::cString code(shaderPD->output, shaderPD->output_len);

    cString funcName = cPath(ahspName->GetChars()).GetFileNoExt();
    iShader* pProg = NULL;
    auto shader = mLibrary.CompileShader(mMetalDevice, code.Chars(), funcName.Chars());
    if (nUnit == eShaderUnit_Vertex) {
      pProg = niNew cMetalShaderVertex(mpGraphics,
                                       &mIDGenerator,
                                       ahspName,
                                       mpGraphics->GetShaderDeviceResourceManager(),
                                       hspProfileName,shader,vData);
    }
    else if (nUnit == eShaderUnit_Pixel)
    {
      pProg = niNew cMetalShaderPixel(mpGraphics,
                                      &mIDGenerator,
                                      ahspName,
                                      mpGraphics->GetShaderDeviceResourceManager(),
                                      hspProfileName,shader,vData);
    }

    if (!niIsOK(pProg)) {
      niSafeRelease(pProg);
      niError(_A("Can't create the shader instance."));
      return NULL;
    }

    if (bHasConstants) {
      if (!ni::GetLang()->SerializeObject(apFile,(iShaderConstants*)pProg->GetConstants(),eSerializeMode_ReadRaw,NULL)) {
        niSafeRelease(pProg);
        niError(_A("Can't load the shader constants."));
        return NULL;
      }
    }

    return pProg;
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
  virtual iVertexArray* __stdcall CreateVertexArray(tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) {
    return niNew cMetalVertexArray(mMetalDevice, anNumVertices, anFVF, aUsage);
  }
  virtual iIndexArray* __stdcall CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
    niUnused(anMaxVertexIndex);
    return niNew cMetalIndexArray(mMetalDevice, aPrimitiveType, anNumIndex, aUsage);
  }

  /////////////////////////////////////////////
  virtual iShaderConstants* __stdcall CreateShaderConstants(tU32) const {
    return NULL;
  }

  /////////////////////////////////////////////
  virtual tIntPtr __stdcall CompileRasterizerStates(iRasterizerStates* apStates) {
    return NULL;
  }

  /////////////////////////////////////////////
  id<MTLSamplerState> _ssCompiled[(eCompiledStates_SS_SharpPointMirror-eCompiledStates_SS_PointRepeat)+1];
  std::map<tU32,id<MTLSamplerState> > _ssMap;

  void _InitCompiledSamplerStates() {
    MTLSamplerDescriptor* desc = [MTLSamplerDescriptor new];

#define INIT_COMPILED_SAMPLER_STATES(STATE,FILTER,WRAP) {               \
      const tU32 ssKey = niFourCC(eSamplerFilter_##FILTER, eSamplerWrap_##WRAP, eSamplerWrap_##WRAP, eSamplerWrap_##WRAP); \
      _toMTLSamplerFilter(desc, eSamplerFilter_##FILTER);               \
      desc.sAddressMode = desc.tAddressMode = desc.rAddressMode = _toMTLSamplerAddress[eSamplerWrap_##WRAP]; \
      id<MTLSamplerState> ss = [mMetalDevice newSamplerStateWithDescriptor: desc]; \
      _ssCompiled[eCompiledStates_##STATE - eCompiledStates_SS_PointRepeat] = ss; \
      _ssMap[ssKey] = ss;                                               \
    }

    INIT_COMPILED_SAMPLER_STATES(SS_PointRepeat, Point, Repeat);
    INIT_COMPILED_SAMPLER_STATES(SS_PointClamp,  Point, Clamp);
    INIT_COMPILED_SAMPLER_STATES(SS_PointMirror, Point, Mirror);

    INIT_COMPILED_SAMPLER_STATES(SS_SmoothRepeat, Smooth, Repeat);
    INIT_COMPILED_SAMPLER_STATES(SS_SmoothClamp,  Smooth, Clamp);
    INIT_COMPILED_SAMPLER_STATES(SS_SmoothMirror, Smooth, Mirror);

    INIT_COMPILED_SAMPLER_STATES(SS_SharpRepeat, Sharp, Repeat);
    INIT_COMPILED_SAMPLER_STATES(SS_SharpClamp,  Sharp, Clamp);
    INIT_COMPILED_SAMPLER_STATES(SS_SharpMirror, Sharp, Mirror);

    INIT_COMPILED_SAMPLER_STATES(SS_SharpPointRepeat, SharpPoint, Repeat);
    INIT_COMPILED_SAMPLER_STATES(SS_SharpPointClamp,  SharpPoint, Clamp);
    INIT_COMPILED_SAMPLER_STATES(SS_SharpPointMirror, SharpPoint, Mirror);
  }

  inline id<MTLSamplerState> _GetMTLSamplerState(tIntPtr ahSS) {
    if (ahSS >= eCompiledStates_SS_PointRepeat && ahSS <= eCompiledStates_SS_SharpMirror) {
      return _ssCompiled[ahSS-eCompiledStates_SS_PointRepeat];
    }
    return _ssCompiled[0];
  }

  id<MTLSamplerState> __stdcall _CompileSamplerStates(const sSamplerStatesDesc* ssDesc) {
    const tU32 ssKey = niFourCC(ssDesc->mFilter, ssDesc->mWrapS, ssDesc->mWrapT, ssDesc->mWrapR);
    std::map<tU32,id<MTLSamplerState> >::const_iterator itSS = _ssMap.find(ssKey);
    if (itSS != _ssMap.end()) {
      return itSS->second;
    }

    MTLSamplerDescriptor* desc = [MTLSamplerDescriptor new];
    _toMTLSamplerFilter(desc, ssDesc->mFilter);
    desc.sAddressMode = _toMTLSamplerAddress[ssDesc->mWrapS];
    desc.tAddressMode = _toMTLSamplerAddress[ssDesc->mWrapT];
    desc.rAddressMode = _toMTLSamplerAddress[ssDesc->mWrapR];
    id<MTLSamplerState> ss = [mMetalDevice newSamplerStateWithDescriptor: desc];
    _ssMap[ssKey] = ss;
    return ss;
  }

  virtual tIntPtr __stdcall CompileSamplerStates(iSamplerStates* apStates) {
#if 1
    niAssertUnreachable("Not implemented.");
    return 0;
#else
    niCheckIsOK(apStates,0);
    const sSamplerStatesDesc* ssDesc = (const sSamplerStatesDesc*)apStates->GetDescStructPtr();
    return eCompiledStates_Driver + (tIntPtr)_CompileSamplerStates(ssDesc);
#endif
  }

  /////////////////////////////////////////////
  id<MTLDepthStencilState> _dsNoDepthTest;
  id<MTLDepthStencilState> _dsDepthTestAndWrite;
  id<MTLDepthStencilState> _dsDepthTestOnly;
  id<MTLDepthStencilState> _dsStatesWrite[eGraphicsCompare_Last] = {0};
  id<MTLDepthStencilState> _dsStatesNoWrite[eGraphicsCompare_Last] = {0};

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

  virtual tIntPtr __stdcall CompileDepthStencilStates(iDepthStencilStates* apStates) {
#if 1
    niAssertUnreachable("Not implemented.");
    return 0;
#else
    niCheckIsOK(apStates,0);

    const sDepthStencilStatesDesc* dsDesc = (const sDepthStencilStatesDesc*)apStates->GetDescStructPtr();
    if (!dsDesc->mbDepthTest) {
      return eCompiledStates_DS_NoDepthTest;
    }

    id<MTLDepthStencilState> dsStates;
    if (dsDesc->mbDepthTestWrite) {
      dsStates = _dsStatesWrite[dsDesc->mDepthTestCompare];
      if (!dsStates) {
        MTLDepthStencilDescriptor* desc = [MTLDepthStencilDescriptor new];
        desc.depthWriteEnabled = YES;
        desc.depthCompareFunction = _toMTLCompareFunction[dsDesc->mDepthTestCompare];
        dsStates = _dsStatesWrite[dsDesc->mDepthTestCompare] = [mMetalDevice newDepthStencilStateWithDescriptor: desc];
      }
    }
    else {
      dsStates = _dsStatesWrite[dsDesc->mDepthTestCompare];
      if (!dsStates) {
        MTLDepthStencilDescriptor* desc = [MTLDepthStencilDescriptor new];
        desc.depthWriteEnabled = NO;
        desc.depthCompareFunction = _toMTLCompareFunction[dsDesc->mDepthTestCompare];
        dsStates = _dsStatesWrite[dsDesc->mDepthTestCompare] = [mMetalDevice newDepthStencilStateWithDescriptor: desc];
      }
    }
    if (!dsStates) {
      niError("Can't create the depth stencil states.");
      return NULL;
    }

    return eCompiledStates_Driver + (tIntPtr)dsStates;
#endif
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
  return niNew cMetalGraphicsDriver(ptrGraphics,device);
}


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
    public sGraphicsContext<1, ni::cIUnknownImpl<iGraphicsContextRT,eIUnknownImplFlags_DontInherit1,iGraphicsContext,iRunnable> >
{
  cMetalGraphicsDriver* mpParent;
  MTLViewport mViewport;
  MTLScissorRect mScissorRect;

  id<MTLRenderCommandEncoder> mCommandEncoder;

  sMetalRenderPipelineId mBaseRenderPipelineId;

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
  tU32 mDirtyFlags = 0;
  tIntPtr mCurrentRS = eInvalidHandle;
  tIntPtr mCurrentDS = eInvalidHandle;
  tIntPtr mCurrentSS = eInvalidHandle;
  tU32 mCurrentBufferOffset = 0;
  tU64 mCurrentRenderPipelineId = 0;
  tU32 mNumDrawOps = 0;

  cMetalContextBase(cMetalGraphicsDriver* apParent, const tU32 aFrameMaxInFlight)
      : tGraphicsContextBase(apParent->GetGraphics())
      , mFrameMaxInFlight(aFrameMaxInFlight)
  {
    mpParent = apParent;
    niAssert(mpParent != NULL);

    mFrameSem.Signal(mFrameMaxInFlight);

  }

  virtual id<MTLRenderCommandEncoder> __stdcall _NewRenderCommandEncoder() = 0;
  virtual tBool __stdcall _PresentAndCommit(iRunnable* apOnCompleted) = 0;

  inline tBool _BeginFrame() {
    niAssert(mbBeganFrame == eFalse);

    // wait on the previous frame to be completed
    {
      mFrameSem.InfiniteWait();
      mDirtyFlags = DIRTY_VIEWPORT | DIRTY_SCISSOR;
      mNumDrawOps = 0;
      mCurrentBufferOffset = 0;
      mCurrentRenderPipelineId = 0;
      mCurrentRS = eInvalidHandle;
      mCurrentDS = eInvalidHandle;
      mCurrentSS = eInvalidHandle;
    }

    if (mBaseRenderPipelineId.IsNull()) {
      mBaseRenderPipelineId.rt0 = _GetRTRenderPipelineColorFormat(mptrRT[0]->GetPixelFormat()->GetFormat());
      if (mptrDS.IsOK()) {
        mBaseRenderPipelineId.ds = _GetDSRenderPipelineDepthFormat(mptrDS->GetPixelFormat()->GetFormat());
      }
    }

    mCommandEncoder = _NewRenderCommandEncoder();
    mbBeganFrame = eTrue;
    // niDebugFmt(("... _BeginFrame: %d", ni::GetLang()->GetFrameNumber()));
    return eTrue;
  }

  inline void _EndFrame() {
    if (!mbBeganFrame) {
      return;
    }
    _PresentAndCommit(this);
    mCommandEncoder = NULL;
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
      _BeginFrame();
    }

    cMetalVertexArray* va = (cMetalVertexArray*)apDrawOp->GetVertexArray();
    if (!va) {
      return eFalse;
    }

    METAL_TRACE(("DrawOperation BEGIN %s:%s",this->GetWidth(),this->GetHeight()));
    const iMaterial* pDOMat = apDrawOp->GetMaterial();
    const sMaterialDesc* pDOMatDesc = (const sMaterialDesc*)pDOMat->GetDescStructPtr();
    const sMatrixf mtxDrawOp = apDrawOp->GetMatrix();
    const sFixedStatesDesc* fixedDesc = (const sFixedStatesDesc*)mptrFS->GetDescStructPtr();
    const tMaterialFlags matFlags = _GetMaterialFlags(pDOMatDesc);

    cMetalIndexArray* ia = (cMetalIndexArray*)apDrawOp->GetIndexArray();
    if (ia) {
      const tU32 baseVertexIndex = apDrawOp->GetBaseVertexIndex();
      const tU32 firstInd = apDrawOp->GetFirstIndex();
      tU32 numInds = apDrawOp->GetNumIndices();
      if (!numInds) {
        numInds = ia->GetNumIndices()-firstInd;
      }

      // niDebugFmt(("... DrawOperation: frame: %d, base: %d, first: %d, num: %d (%d tris), uoff: %d, uSize: %d",
                  // ni::GetLang()->GetFrameNumber(),
                  // baseVertexIndex, firstInd, numInds, numInds/3,
                  // mCurrentBufferOffset,
                  // sizeof(sUniformsFixed)));

      sMetalRenderPipelineId rpId = mBaseRenderPipelineId;


      iShader* pVS = pDOMatDesc->mShaders[eShaderUnit_Vertex];
      iShader* pPS = pDOMatDesc->mShaders[eShaderUnit_Pixel];

      if (!pVS) {
        pVS = mpParent->mFixedShaders.GetVertexShader(va->_fvf,*pDOMatDesc);
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

      rpId.vertFuncId = ((cMetalShaderVertex*)pVS)->GetUID();
      rpId.fragFuncId = ((cMetalShaderPixel*)pPS)->GetUID();
      rpId.blendMode = _GetBlendMode(pDOMatDesc);
      rpId.fvf = va->_fvf;

      {
        const tIntPtr hRS = _GetRS(pDOMatDesc);
        if (mCurrentRS != hRS) {
          mCurrentRS = hRS;
        }
      }

      {
        const tIntPtr hDS = _GetDS(pDOMatDesc);
        if (rpId.ds && (mCurrentDS != hDS)) {
          [mCommandEncoder setDepthStencilState:mpParent->_GetMTLDepthStencilState(hDS)];
          mCurrentDS = hDS;
        }
      }

      if (rpId != mCurrentRenderPipelineId) {
        id<MTLRenderPipelineState> pipeline = mpParent->mLibrary.GetRenderPipeline(mpParent->mMetalDevice,
                                                                                   rpId,
                                                                                   (cMetalShaderVertex*)pVS,
                                                                                   (cMetalShaderPixel*)pPS);
        if (!pipeline) {
          niError("Can't get the render pipeline.");
          return eFalse;
        }
        [mCommandEncoder setRenderPipelineState:pipeline];
        mCurrentRenderPipelineId = rpId;
      }

      if (mDirtyFlags & DIRTY_VIEWPORT) {
        [mCommandEncoder setViewport:mViewport];
      }
      if (mDirtyFlags & DIRTY_SCISSOR) {
        [mCommandEncoder setScissorRect:mScissorRect];
      }

      [mCommandEncoder setVertexBuffer:va->_buffer._mtlBuffer offset:(baseVertexIndex*va->_fvfStride) atIndex:0];

      auto applyMaterialChannel =
      [&](cMetalContextBase* apContext, iGraphics* apGraphics, tU32 anTSS, eMaterialChannel aChannel, const sMaterialDesc* apDOMat)
      {
        const sMaterialChannel& ch = apContext->_GetChannel(apDOMat, aChannel);
        if (ch.mTexture.IsOK()) {
          cMetalTexture* tex = (cMetalTexture*)ch.mTexture->Bind(NULL);
          if (tex == NULL || tex->_tex == nil) {
            // Probably trying to use the frame buffer we don't support this atm...
            [mCommandEncoder setFragmentTexture:NULL atIndex:anTSS];
          }
          else {
            [mCommandEncoder setFragmentTexture:tex->_tex atIndex:anTSS];
            // niDebugFmt(("TEX SET >>> %s",tex->GetDeviceResourceName()));
            if (mCurrentSS != ch.mhSS) {
              [mCommandEncoder setFragmentSamplerState: mpParent->_GetMTLSamplerState(ch.mhSS) atIndex: anTSS];
              mCurrentSS = ch.mhSS;
            }
          }
        }
        else {
          [mCommandEncoder setFragmentTexture:NULL atIndex: anTSS];
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
          [mCommandEncoder setVertexBytes:&vf[0] length:size atIndex:16];
        }
        else if (aUnit == eShaderUnit_Pixel) {
          [mCommandEncoder setFragmentBytes:&vf[0] length:size atIndex:16];
        }
        return eTrue;
      };

      updateConstant(eShaderUnit_Vertex, (sShaderConstantsDesc*)pVS->GetConstants()->GetDescStructPtr());
      updateConstant(eShaderUnit_Pixel, (sShaderConstantsDesc*)pPS->GetConstants()->GetDescStructPtr());

      [mCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
       indexCount:numInds
       indexType:MTLIndexTypeUInt32
       indexBuffer:ia->_buffer._mtlBuffer
       indexBufferOffset:(firstInd*sizeof(tU32))];
      METAL_TRACE(("DrawOperation END"));
    }
    else {
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
