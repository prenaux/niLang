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

template <typename T>
void _MTLSetLabel(T* aObj, const achar* aaszLabel) {
  if (niStringIsOK(aaszLabel)) {
    aObj.label = [NSString stringWithUTF8String:aaszLabel];
  }
}

#if defined METAL_MAC
static const tU32 knUniformBufferAlignment = 256;
#elif defined METAL_IOS
static const tU32 knUniformBufferAlignment = 16;
#endif

static const tU32 knMetalMaxFrameInFlight = 2;

// This is where the [[stage_in]] vertex buffer is bound. We reserve the last
// index for it so that we dont overlap with [[buffer(0)]] which will usually
// be a uniform buffer.
static const tU32 knMetalStageInBufferIndex = 30;

// Commented out, we dont want to use native DXT support on macOS only since its not supported on iOS, which is our primary target
// #define USE_METAL_DXT

// TODO: This should move to some "context creation" time thing, similar to the MSAA setting.
static tU32 knMetalSamplerFilterAnisotropy = 8;

static cString _GetDeviceCaps(id<MTLDevice> aDevice) {
  cString r;
  r += niFmt("  name: %s\n", [aDevice.name UTF8String]);
  r += niFmt("  maxFrameInFlight: %d\n", knMetalMaxFrameInFlight);
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

static MTLVertexDescriptor* _CreateMetalVertDescForFVF(const tU32 aBufferIndex, tFVF aFVF)
{
  MTLVertexDescriptor* vertDesc = [MTLVertexDescriptor vertexDescriptor];

  tU32 index = 0;
  tU32 off = 0;

  niLet _CreateMetalAttrDesc = [](const tU32 aBufferIndex, MTLVertexFormat aFmt, const tU32 aOffset)
      -> MTLVertexAttributeDescriptor*
  {
    MTLVertexAttributeDescriptor* attrDesc = [MTLVertexAttributeDescriptor new];
    attrDesc.format = aFmt;
    attrDesc.offset = aOffset;
    attrDesc.bufferIndex = aBufferIndex;
    return attrDesc;
  };

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
  [vertDesc.layouts setObject: layoutDesc atIndexedSubscript: aBufferIndex];

  return vertDesc;

#undef METAL_ATTR
}

static MTLVertexDescriptor* _CreateMetalVertDescForFVFAndGpuVertexAttibutes(
  ain<tU32> aBufferIndex,
  ain<tFVF> aFVF,
  ain<astl::vector<sGpuVertexAttribute>> aVertAttrs)
{
  MTLVertexDescriptor* vertDesc = [MTLVertexDescriptor vertexDescriptor];

  cFVFDescription fvfDesc(aFVF);
  for (const auto& attr : aVertAttrs) {
    if ((attr._fvf & aFVF) != attr._fvf) {
      niError(niFmt("Vertex attribute FVF mismatch: expected '%s' (0x%x), got '%s' (0x%x)",
                    ni::FVFToShortString(aFVF), aFVF, ni::FVFToShortString(attr._fvf), attr._fvf));
      return nil;
    }

    MTLVertexAttributeDescriptor* attrDesc = [MTLVertexAttributeDescriptor new];
    attrDesc.bufferIndex = aBufferIndex;

    niLet handleTexCoo = [&](const tU32 i) -> tBool {
      const tU32 dim = eFVF_TexCooDim(aFVF,i);
      switch(dim) {
        case 1:
          attrDesc.format = MTLVertexFormatFloat;
          break;
        case 2:
          attrDesc.format = MTLVertexFormatFloat2;
          break;
        case 3:
          attrDesc.format = MTLVertexFormatFloat3;
          break;
        case 4:
          attrDesc.format = MTLVertexFormatFloat4;
          break;
        default: {
          niError(niFmt("Unsupported vertex attribute '%s' (%d) texcoo size '%d' fvf: %s (0x%x)",
                        attr._name, attr._location,
                        dim,
                        ni::FVFToShortString(attr._fvf), attr._fvf));
          return eFalse;
        }
      }
      attrDesc.offset = fvfDesc.GetTexCooOffset(i);
      return eTrue;
    };

    tBool handledFvf = eTrue;
    switch(attr._fvf) {
      case eFVF_PositionB4:
        attrDesc.format = MTLVertexFormatFloat4;
        attrDesc.offset = fvfDesc.GetWeightsOffset();
        break;
      case eFVF_Position:
        attrDesc.format = MTLVertexFormatFloat3;
        attrDesc.offset = fvfDesc.GetPositionOffset();
        break;
      case eFVF_Indices:
        attrDesc.format = MTLVertexFormatUChar4;
        attrDesc.offset = fvfDesc.GetIndicesOffset();
        break;
      case eFVF_Normal:
        attrDesc.format = MTLVertexFormatFloat3;
        attrDesc.offset = fvfDesc.GetNormalOffset();
        break;
      case eFVF_PointSize:
        attrDesc.format = MTLVertexFormatFloat;
        attrDesc.offset = fvfDesc.GetPointSizeOffset();
        break;
      case eFVF_ColorA:
        attrDesc.format = MTLVertexFormatUChar4Normalized_BGRA;
        attrDesc.offset = fvfDesc.GetColorAOffset();
        break;
      case eFVF_Tex1: handledFvf = handleTexCoo(0); break;
      case eFVF_Tex2: handledFvf = handleTexCoo(1); break;
      case eFVF_Tex3: handledFvf = handleTexCoo(2); break;
      case eFVF_Tex4: handledFvf = handleTexCoo(3); break;
      case eFVF_Tex5: handledFvf = handleTexCoo(4); break;
      case eFVF_Tex6: handledFvf = handleTexCoo(5); break;
      case eFVF_Tex7: handledFvf = handleTexCoo(6); break;
      case eFVF_Tex8: handledFvf = handleTexCoo(7); break;
      default:
        handledFvf = eFalse;
        break;
    }
    if (!handledFvf) {
      niError(niFmt("Unsupported vertex attribute '%s' (%d) fvf: %s (0x%x)",
                    attr._name, attr._location,
                    ni::FVFToShortString(attr._fvf), attr._fvf));
      return nil;
    }

    [vertDesc.attributes setObject:attrDesc atIndexedSubscript:attr._location];
  }

  MTLVertexBufferLayoutDescriptor* layoutDesc = [MTLVertexBufferLayoutDescriptor new];
  layoutDesc.stride = fvfDesc.GetStride();
  layoutDesc.stepFunction = MTLVertexStepFunctionPerVertex;
  layoutDesc.stepRate = 1;
  [vertDesc.layouts setObject:layoutDesc atIndexedSubscript:aBufferIndex];

  return vertDesc;
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

struct sMetalFunction : public ImplRC<iGpuFunction,eImplFlags_DontInherit1,iDeviceResource> {
  const tU32 _id;
  tHStringPtr _hspName;
  Ptr<iDataTable> _datatable;
  id<MTLFunction> _mtlFunction;

  sMetalFunction(tU32 anID) : _id(anID) {}

  tBool _Compile(id<MTLDevice> aDevice, iHString* ahspName, iDataTable* apDTRoot) {
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

#if 0
    niLog(Info,niFmt(
      "Creating shader '%s':\n----------\n%s\n----------\n",
      ahspName,source));
#endif

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

    _MTLSetLabel(_mtlFunction,niHStr(ahspName));
    return eTrue;
  }

  virtual tU32 __stdcall GetFunctionId() const {
    return _id;
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

static inline tType _MTLDataTypeToType(MTLDataType aMTLDataType) {
  switch (aMTLDataType) {
    case MTLDataTypeFloat: return eType_F32;
    case MTLDataTypeFloat2: return eType_Vec2f;
    case MTLDataTypeFloat3: return eType_Vec3f;
    case MTLDataTypeFloat4: return eType_Vec4f;
    case MTLDataTypeInt: return eType_I32;
    case MTLDataTypeInt2: return eType_Vec2i;
    case MTLDataTypeInt3: return eType_Vec3i;
    case MTLDataTypeInt4: return eType_Vec4i;
    case MTLDataTypeUInt: return eType_U32;
    case MTLDataTypeUInt2: return eType_Vec2i;
    case MTLDataTypeUInt3: return eType_Vec3i;
    case MTLDataTypeUInt4: return eType_Vec4i;
    default: return eType_Null;
  }
}

tBool _MTLVertexAttributeToGpuVertexAttribute(aout<sGpuVertexAttribute> aOut, MTLVertexAttribute* attr) {
  aOut._name = attr.name.UTF8String;
  niLet& attrName = aOut._name;
  aOut._location = attr.attributeIndex;
  aOut._type = _MTLDataTypeToType(attr.attributeType);
  if (attrName.StartsWith("IN_")) {
    if (attrName.EndsWith("_position")) {
      aOut._fvf = eFVF_Position;
    }
    else if (attrName.EndsWith("_weights")) {
      aOut._fvf = eFVF_PositionB4;
    }
    else if (attrName.EndsWith("_indices")) {
      aOut._fvf = eFVF_Indices;
    }
    else if (attrName.EndsWith("_normal")) {
      aOut._fvf = eFVF_Normal;
    }
    else if (attrName.EndsWith("_color")) {
      aOut._fvf = eFVF_ColorA;
    }
    else if (attrName.EndsWith("_tex0")) {
      aOut._fvf = eFVF_Tex1;
    }
    else if (attrName.EndsWith("_tex1")) {
      aOut._fvf = eFVF_Tex2;
    }
    else if (attrName.EndsWith("_tex2")) {
      aOut._fvf = eFVF_Tex3;
    }
    else if (attrName.EndsWith("_tex3")) {
      aOut._fvf = eFVF_Tex4;
    }
    else if (attrName.EndsWith("_tex4")) {
      aOut._fvf = eFVF_Tex5;
    }
    else if (attrName.EndsWith("_tex5")) {
      aOut._fvf = eFVF_Tex6;
    }
    else if (attrName.EndsWith("_tex6")) {
      aOut._fvf = eFVF_Tex7;
    }
    else if (attrName.EndsWith("_tex7")) {
      aOut._fvf = eFVF_Tex8;
    }
    else {
      niError(niFmt("Unidentified IN vertex attribute name '%s'.", attrName));
      return eFalse;
    }
    return eTrue;
  }
  else if (attrName.Eq("v0") ||
           attrName.Eq("v1") ||
           attrName.Eq("v2") ||
           attrName.Eq("v3") ||
           attrName.Eq("v4") ||
           attrName.Eq("v5") ||
           attrName.Eq("v6") ||
           attrName.Eq("v7") ||
           attrName.Eq("v8") ||
           attrName.Eq("v9"))
  {
    aOut._fvf = 0;
    return eTrue;
  }
  else {
    niError(niFmt("Unidentified IN vertex attribute name '%s'.", attrName));
    return eFalse;
  }
}

struct sMetalPipeline : public ImplRC<iGpuPipeline,eImplFlags_DontInherit1,iDeviceResource> {
  tHStringPtr _hspName;
  NN<iGpuPipelineDesc> _desc = niDeferredInit(NN<iGpuPipelineDesc>);
  id<MTLRenderPipelineState> _mtlPipeline;
  id<MTLDepthStencilState> _mtlDepthStencil;
  MTLCullMode _cullMode = MTLCullModeNone;
  MTLTriangleFillMode _fillMode = MTLTriangleFillModeFill;
  MTLWinding _winding = MTLWindingClockwise;

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

  tBool _CreateMTLPipeline(
    id<MTLDevice> aDevice, iHString* ahspName,
    const iGpuPipelineDesc* apDesc,
    const iRasterizerStates* apRS,
    id<MTLDepthStencilState> aDS)
  {
    niCheckIsOK(apDesc,eFalse);
    _hspName = ahspName;
    _desc = niCheckNN(_desc,apDesc->Clone(),eFalse);
    _mtlDepthStencil = aDS;

    MTLRenderPipelineDescriptor* pipelineDesc = [MTLRenderPipelineDescriptor new];
    _MTLSetLabel(pipelineDesc, niHStr(ahspName));
    pipelineDesc.rasterizationEnabled = YES;

    // Vertex gpu function
    {
      sMetalFunction* vertexFunction = (sMetalFunction*)_desc->GetFunction(eGpuFunctionType_Vertex);
      niCheck(vertexFunction != nullptr, 0);
      pipelineDesc.vertexFunction = vertexFunction->_mtlFunction;

      // Vertex attributes
      tBool hasMojoVertexAttribute = eFalse;
      astl::vector<sGpuVertexAttribute> attrs;
      {
        NSArray<MTLVertexAttribute*>* attributes = vertexFunction->_mtlFunction.vertexAttributes;
        for (MTLVertexAttribute* attr in attributes) {
          sGpuVertexAttribute& va = attrs.emplace_back();
          if (!_MTLVertexAttributeToGpuVertexAttribute(va, attr)) {
            niError(niFmt("Can't create pipeline, cant validate vertex attribute [%d]: %s", attrs.size()-1,va.ToString()));
            return eFalse;
          }
          // niDebugFmt(("... Vertex Attribute[%d]: %s.", attrs.size()-1, va.ToString()));

          niLet isMojoVA = (va._fvf == 0);
          // first attribute
          if (attrs.size() == 1) {
            hasMojoVertexAttribute = isMojoVA;
          }
          else if ((hasMojoVertexAttribute && !isMojoVA) ||
                   (!hasMojoVertexAttribute && isMojoVA)) {
            niError(niFmt("Can't create pipeline, cant mix legacy and IN vertex attributes [%d]: %s",
                          attrs.size()-1,va.ToString()));
            return eFalse;
          }
        }
      }
      if (attrs.empty()) {
        niError(niFmt("Can't create pipeline, gpu vertex function '%s' doesn't have vertex attributes.",
                      vertexFunction->GetDeviceResourceName()));
        return eFalse;
      }
      if (hasMojoVertexAttribute) {
        pipelineDesc.vertexDescriptor = _CreateMetalVertDescForFVF(knMetalStageInBufferIndex, _desc->GetFVF());
      }
      else {
        pipelineDesc.vertexDescriptor = _CreateMetalVertDescForFVFAndGpuVertexAttibutes(
          knMetalStageInBufferIndex, _desc->GetFVF(), attrs);
      }
    }

    // Pixel gpu function
    {
      sMetalFunction* pixelFunction = (sMetalFunction*)_desc->GetFunction(eGpuFunctionType_Pixel);
      niCheck(pixelFunction != nullptr, 0);
      pipelineDesc.fragmentFunction = pixelFunction->_mtlFunction;
    }

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

      if (apRS) {
        _fillMode = apRS->GetWireframe() ? MTLTriangleFillModeLines : MTLTriangleFillModeFill;
        _cullMode = _toMTLCullMode[apRS->GetCullingMode()];
        niLet colorWriteMask = apRS->GetColorWriteMask();
        switch (colorWriteMask) {
          case eColorWriteMask_None:
            ca.writeMask = MTLColorWriteMaskNone;
            break;
          case eColorWriteMask_All:
            ca.writeMask = MTLColorWriteMaskAll;
            break;
          default: {
            MTLColorWriteMask mask = MTLColorWriteMaskNone;
            if (colorWriteMask & eColorWriteMask_Red)
              mask |= MTLColorWriteMaskRed;
            if (colorWriteMask & eColorWriteMask_Green)
              mask |= MTLColorWriteMaskGreen;
            if (colorWriteMask & eColorWriteMask_Blue)
              mask |= MTLColorWriteMaskBlue;
            if (colorWriteMask & eColorWriteMask_Alpha)
              mask |= MTLColorWriteMaskAlpha;
            ca.writeMask = mask;
            break;
          }
        }
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

  void Apply(id<MTLRenderCommandEncoder> aEncoder) {
    [aEncoder setRenderPipelineState:_mtlPipeline];
    [aEncoder setDepthStencilState:_mtlDepthStencil];
    [aEncoder setFrontFacingWinding:_winding];
    [aEncoder setTriangleFillMode:_fillMode];
    [aEncoder setCullMode:_cullMode];
  }
};

//--------------------------------------------------------------------------------------------
//
// MetalBuffer
//
//--------------------------------------------------------------------------------------------
struct sMetalBuffer : public ni::ImplRC<iGpuBuffer,eImplFlags_DontInherit1,iDeviceResource> {
  id<MTLBuffer> _mtlBuffer;
  eGpuBufferMemoryMode _memMode;
  tGpuBufferUsageFlags _usage;
  tU32 _lockOffset = 0, _lockSize = 0;
  tU32 _modifiedOffset = 0, _modifiedSize = 0;
  eLock _lockMode;
  tBool _tracked;

  tBool _CreateMTLBuffer(
    id<MTLDevice> aDevice,
    iHString* ahspName,
    void* apInitialData, tU32 anSize,
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

    _MTLSetLabel(_mtlBuffer,niHStr(ahspName));
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
  NN<iFixedGpuPipelines> mFixedPipelines = niDeferredInit(NN<iFixedGpuPipelines>);
  LocalIDGenerator mIDGenerator;

  cMetalGraphicsDriver(iGraphics* apGraphics, id<MTLDevice> aDevice) {
    mpGraphics = apGraphics;
    mMetalDevice = aDevice;

    _InitCompiledSamplerStates();
    _InitCompiledDepthStencilStates();

    mFixedPipelines = niCheckNNIfNull(mFixedPipelines, CreateFixedGpuPipelines(this)) {
      niError("CreateFixedGpuPipelines failed.");
      mpGraphics = nullptr;
      return;
    }

    niLog(Info, niFmt(
      "--- Metal Device Info ---\n%s\n-------------------------",
      _GetDeviceCaps(mMetalDevice)));
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

  /////////////////////////////////////////////
  tBool _DetectShaderProfiles() {
    if (mvProfiles[0].empty()) {
      niLoop(i,eShaderUnit_Last) {
        mvProfiles[i].clear();
      }
      mvProfiles[eShaderUnit_Vertex].push_back(_H("hlsl_vs_3_0"));
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
      this, knMetalMaxFrameInFlight,
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

    NN<iDataTable> dtFunc = CreateGpuFunctionDT(this,code.Chars());
    auto func = this->CreateGpuFunction(
      HFmt("gpufunc_%s",ahspName),
      nUnit == eShaderUnit_Pixel ? eGpuFunctionType_Pixel : eGpuFunctionType_Vertex,
      dtFunc);
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

  /////////////////////////////////////////////
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

  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBuffer(iHString* ahspName, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl {
    niLet buffer = MakeNN<sMetalBuffer>();
    niCheck(
      buffer->_CreateMTLBuffer(mMetalDevice, ahspName, nullptr, anSize, aMemMode, aUsage),
      nullptr);
    return buffer;
  }

  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromData(iHString* ahspName, iFile* apFile, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl {
    niCheckIsOK(apFile,nullptr);
    astl::vector<tU8> data;
    data.resize(anSize);
    if (apFile->ReadRaw(data.data(),anSize) != anSize) {
      return nullptr;
    }
    return this->CreateGpuBufferFromDataRaw(ahspName,data.data(),anSize,aMemMode,aUsage);
  }

  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromDataRaw(iHString* ahspName, tPtr apData, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) niImpl {
    niCheck(apData != nullptr, nullptr);
    niLet buffer = MakeNN<sMetalBuffer>();
    niCheck(
      buffer->_CreateMTLBuffer(mMetalDevice, ahspName, apData, anSize, aMemMode, aUsage),
      nullptr);
    return buffer;
  }

  virtual iHString* __stdcall GetGpuFunctionTarget() const {
    return _GetGpuFunctionTarget();
  }

  virtual Ptr<iGpuFunction> __stdcall CreateGpuFunction(iHString* ahspName, eGpuFunctionType aType, iDataTable* apGpuFunctionDT) niImpl {
    niLet newId = mIDGenerator.AllocID();
    NN<sMetalFunction> func = MakeNN<sMetalFunction>(newId);
    if (!func->_Compile(mMetalDevice, ahspName, apGpuFunctionDT)) {
      mIDGenerator.FreeID(newId);
      niError(niFmt(
        "Can't create gpu function '%s': Compilation failed.",
        ahspName));
      return nullptr;
    }
    if (func->GetFunctionType() != aType) {
      mIDGenerator.FreeID(newId);
      niError(niFmt(
        "Can't create gpu function '%s': Expected function type '%s' but got '%s'.",
        ahspName,
        niEnumToChars(eGpuFunctionType,aType),
        niEnumToChars(eGpuFunctionType,func->GetFunctionType())));
      return nullptr;
    }
    return func;
  }

  virtual Ptr<iGpuPipelineDesc> __stdcall CreateGpuPipelineDesc() niImpl {
    return ni::_CreateGpuPipelineDesc();
  }

  virtual Ptr<iGpuPipeline> __stdcall CreateGpuPipeline(iHString* ahspName, const iGpuPipelineDesc* apDesc) niImpl {
    niCheckIsOK(apDesc,nullptr);
    niLet ds = _GetMTLDepthStencilState(apDesc->GetDepthStencilStates());
    niLet rs = mpGraphics->GetCompiledRasterizerStates(apDesc->GetRasterizerStates());
    NN<sMetalPipeline> pipeline = MakeNN<sMetalPipeline>();
    niCheck(
      pipeline->_CreateMTLPipeline(mMetalDevice,ahspName,apDesc,rs,ds),
      nullptr);
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
struct sMetalEncoderFrameData : public ImplRC<iRunnable> {
  ThreadEvent _event = ThreadEvent(eFalse);
  astl::vector<Ptr<sMetalBuffer>> _trackedBuffers;
  Ptr<iGpuStream> _stream;
  tU32 _frameNumber = 0;
  tU32 _frameInFlight = 0;

  sMetalEncoderFrameData(ain<nn<cMetalGraphicsDriver>> aDriver) {
    _stream = CreateGpuStream(
      aDriver,
      eGpuBufferUsageFlags_Vertex|
      eGpuBufferUsageFlags_Index|
      eGpuBufferUsageFlags_Uniform);
  }

  void StartFrame(const tU32 anFrameNumber, const tU32 anCurrentFrame) {
    niPanicAssert(!IsInUse());
    _frameNumber = anFrameNumber;
    _frameInFlight = anCurrentFrame;
  }

  void OnFrameCompleted() {
    _frameNumber = 0;
  }

  tBool IsInUse() {
    return _frameNumber != 0;
  }

  void _TrackBuffer(iGpuBuffer* apBuffer) {
    sMetalBuffer* buffer = (sMetalBuffer*)apBuffer;
    if (!buffer->_tracked && buffer->_modifiedSize) {
      _trackedBuffers.push_back(buffer);
      buffer->_tracked = eTrue;
    }
  }

  void _WaitCompletion() {
    _event.InfiniteWait();
  }

  virtual Var __stdcall Run() niImpl {
    niLet& lastBlock = _stream->GetLastBlock();
    // niDebugFmt((
    //   "... OnFrameCompleted: sMetalEncoderFrameData{_frameNumber=%d,_frameInFlight=%d,_trackedBuffers=%d,_stream._numBlocks=%d,_stream.mOffset=%d,_stream.mSize=%d}",
    //   _frameNumber,_frameInFlight,
    //   _trackedBuffers.size(),_stream->GetNumBlocks(),
    //   lastBlock.mOffset,lastBlock.mSize));
    niLoop(i,_trackedBuffers.size()) {
      _trackedBuffers[i]->_Untrack();
    }
    _trackedBuffers.clear();
    _stream->Reset();
    _event.Signal();
    this->OnFrameCompleted();
    return niVarNull;
  }
};

struct sMetalCommandEncoder : public ImplRC<iGpuCommandEncoder> {
  nn<cMetalGraphicsDriver> _driver;
  id<MTLRenderCommandEncoder> _cmdEncoder;
  astl::vector<NN<sMetalEncoderFrameData>> _frames;
  tU32 _currentFrame = 0;
  Ptr<sMetalBuffer> _indexBuffer;
  tU32 _indexOffset = 0;
  eGpuIndexType _indexType = eGpuIndexType_U32;

  sMetalCommandEncoder(
    ain<nn<cMetalGraphicsDriver>> aDriver,
    ain<tU32> aFrameMaxInFlight)
      : _driver(aDriver)
  {
    _frames.reserve(aFrameMaxInFlight);
    niLoop(i,aFrameMaxInFlight) {
      _frames.emplace_back(niNew sMetalEncoderFrameData(aDriver));
    }
  }

  sMetalEncoderFrameData* _GetCurrentFrame() {
    return _frames[_currentFrame];
  }

  void _BeginFrame(id<MTLRenderCommandEncoder> aNewEncoder, tU32 aCurrentFrame) {
    niPanicAssert(_cmdEncoder == nil);
    _cmdEncoder = aNewEncoder;
    _currentFrame = aCurrentFrame;
    if (_frames[_currentFrame]->IsInUse()) {
      niWarning(niFmt("Waiting for frame %d completion...",_currentFrame));
      _frames[_currentFrame]->_WaitCompletion();
    }
    _frames[_currentFrame]->StartFrame(
      ni::GetLang()->GetFrameNumber(),_currentFrame);
  }

  Ptr<sMetalEncoderFrameData> _EndFrame() {
    niLet frameData = _frames[_currentFrame];
    _cmdEncoder = nil;
    return frameData;
  }

  virtual void __stdcall SetPipeline(iGpuPipeline* apPipeline) niImpl {
    niCheck(apPipeline != nullptr,;);
    ((sMetalPipeline*)apPipeline)->Apply(_cmdEncoder);
  }

  virtual void __stdcall SetVertexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) niImpl {
    niCheck(apBuffer != nullptr, ;);
    _GetCurrentFrame()->_TrackBuffer(apBuffer);
    sMetalBuffer* buffer = (sMetalBuffer*)apBuffer;
    [_cmdEncoder setVertexBuffer:buffer->_mtlBuffer offset:anOffset atIndex:knMetalStageInBufferIndex];
  }

  virtual void __stdcall SetIndexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, eGpuIndexType aIndexType) niImpl {
    niCheck(apBuffer != nullptr, ;);
    _GetCurrentFrame()->_TrackBuffer(apBuffer);
    _indexBuffer = (sMetalBuffer*)apBuffer;
    _indexOffset = anOffset;
    _indexType = aIndexType;
  }

  virtual void __stdcall SetUniformBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) niImpl {
    niCheck(apBuffer != nullptr, ;);
    _GetCurrentFrame()->_TrackBuffer(apBuffer);
    sMetalBuffer* buffer = (sMetalBuffer*)apBuffer;
    [_cmdEncoder setVertexBuffer:buffer->_mtlBuffer offset:anOffset atIndex:anBinding];
    [_cmdEncoder setFragmentBuffer:buffer->_mtlBuffer offset:anOffset atIndex:anBinding];
  }

  virtual void __stdcall SetTexture(iTexture* apTexture, tU32 anBinding) niImpl {
    niCheck(apTexture != nullptr, ;);
    cMetalTexture* tex = (cMetalTexture*)apTexture;
    [_cmdEncoder setFragmentTexture:tex->_tex atIndex:anBinding];
  }

  virtual tBool __stdcall StreamVertexBuffer(const tPtr apData, tU32 anSize, tU32 anBinding) niImpl {
    return UpdateGpuStreamToVertexBuffer(_GetCurrentFrame()->_stream,this,apData,anSize,anBinding);
  }

  virtual tBool __stdcall StreamIndexBuffer(const tPtr apData, tU32 anSize, eGpuIndexType aIndexType) niImpl {
    return UpdateGpuStreamToIndexBuffer(_GetCurrentFrame()->_stream,this,apData,anSize,aIndexType);
  }

  virtual tBool __stdcall StreamUniformBuffer(const tPtr apData, tU32 anSize, tU32 anBinding) niImpl {
    return UpdateGpuStreamToUniformBuffer(_GetCurrentFrame()->_stream,this,apData,anSize,anBinding);
  }

  virtual void __stdcall SetSamplerState(tIntPtr ahSS, tU32 anBinding) niImpl {
    [_cmdEncoder
     setFragmentSamplerState:_driver->_GetMTLSamplerState(ahSS)
     atIndex:anBinding];
  }

  virtual void __stdcall SetPolygonOffset(const sVec2f& avOffset) niImpl {
    [_cmdEncoder setDepthBias:avOffset.x
     slopeScale:avOffset.y
     clamp:0.0f];
  }

  virtual void __stdcall SetScissorRect(const sRecti& aRect) niImpl {
    MTLScissorRect rect;
    rect.x = aRect.x;
    rect.y = aRect.y;
    rect.width = aRect.GetWidth();
    rect.height = aRect.GetHeight();
    [_cmdEncoder setScissorRect:rect];
  }

  virtual void __stdcall SetViewport(const sRecti& aRect) niImpl {
    MTLViewport vp;
    vp.originX = aRect.x;
    vp.originY = aRect.y;
    vp.width = aRect.GetWidth();
    vp.height = aRect.GetHeight();
    vp.znear = 0.0f;
    vp.zfar = 1.0f;
    [_cmdEncoder setViewport:vp];
  }

  virtual void __stdcall SetStencilReference(tI32 aRef) niImpl {
    [_cmdEncoder setStencilReferenceValue:aRef];
  }

  virtual void __stdcall SetStencilMask(tU32 aMask) niImpl {
    [_cmdEncoder setStencilFrontReferenceValue:aMask
     backReferenceValue:aMask];
  }

  virtual void __stdcall SetBlendColorConstant(const sColor4f& aColor) niImpl {
    [_cmdEncoder setBlendColorRed:aColor.x
     green:aColor.y
     blue:aColor.z
     alpha:aColor.w];
  }

  virtual tBool __stdcall DrawIndexed(eGraphicsPrimitiveType aPrimType, tU32 anNumIndices, tU32 anFirstIndex) niImpl
  {
    niCheck(aPrimType <= eGraphicsPrimitiveType_Last, eFalse);
    niCheckIsOK(_indexBuffer,eFalse);
    niCheck(_indexType <= eGpuIndexType_Last,eFalse);
    [_cmdEncoder drawIndexedPrimitives:_toMTLPrimitiveType[aPrimType]
     indexCount:anNumIndices
     indexType:_toMTLIndexType[_indexType]
     indexBuffer:_indexBuffer->_mtlBuffer
     indexBufferOffset:_indexOffset + (anFirstIndex * sizeof(tU32))];
    return eTrue;
  }

  virtual tBool __stdcall Draw(eGraphicsPrimitiveType aPrimType, tU32 anVertexCount, tU32 anFirstVertex) niImpl
  {
    niCheck(aPrimType <= eGraphicsPrimitiveType_Last, eFalse);
    [_cmdEncoder drawPrimitives:_toMTLPrimitiveType[aPrimType]
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

  eGpuPixelFormat mRT0Format;
  eGpuPixelFormat mDSFormat;

  double4 mClearColor = double4::Zero();
  tF32 mClearDepth = 1.0f;
  tI32 mClearStencil = 0;

  tBool mbBeganFrame = eFalse;

  tU32 mnCurrentFrame = 0;
  const tU32 mFrameMaxInFlight;
  ThreadSem mFrameSem = ThreadSem(0);
  virtual ni::Var __stdcall Run() niImpl {
    mFrameSem.Signal();
    return niVarNull;
  }

  tFixedGpuPipelineId mCurrentFixedGpuPipelineId = 0;
  tU32 mNumDrawOps = 0;

  cMetalContextBase(cMetalGraphicsDriver* apParent, const tU32 aFrameMaxInFlight)
      : tGraphicsContextBase(apParent->GetGraphics())
      , mFrameMaxInFlight(aFrameMaxInFlight)
      , mCmdEncoder(MakeNN<sMetalCommandEncoder>(as_nn(apParent),aFrameMaxInFlight))
  {
    mpParent = apParent;
    niAssert(mpParent != NULL);
    mFrameSem.Signal(mFrameMaxInFlight);
  }

  virtual id<MTLRenderCommandEncoder> __stdcall _NewRenderCommandEncoder() = 0;
  virtual tBool __stdcall _EndCommandEncoder() = 0;

  virtual iGpuCommandEncoder* __stdcall BeginCommandEncoder() niImpl {
    if (!mbBeganFrame) {
      _BeginFrame();
    }
    return mCmdEncoder;
  }

  virtual void __stdcall EndCommandEncoder() niImpl {
    _EndFrame();
  }

  void _AddCompletedHandler(iRunnable* apRunnable) {
    niLet commandBuffer = [mpParent->_GetCommandQueue() commandBuffer];
    Ptr<iRunnable> onCompleted = apRunnable;
    [commandBuffer addCompletedHandler: ^(id<MTLCommandBuffer> _Nonnull) {
        onCompleted->Run();
      }];
  }

  inline tBool _BeginFrame() {
    niPanicAssert(mbBeganFrame == eFalse);
    mbBeganFrame = eTrue;

    // wait on the previous N frames to be completed
    {
      mFrameSem.InfiniteWait();
      mNumDrawOps = 0;
      mCurrentFixedGpuPipelineId = 0;
    }

    mRT0Format = _GetClosestGpuPixelFormatForRT(mptrRT[0]->GetPixelFormat()->GetFormat());
    if (mptrDS.IsOK()) {
      mDSFormat = _GetClosestGpuPixelFormatForDS(mptrDS->GetPixelFormat()->GetFormat());
    }
    else {
      mDSFormat = eGpuPixelFormat_None;
    }

    mCmdEncoder->_BeginFrame(_NewRenderCommandEncoder(),mnCurrentFrame);
    // niDebugFmt(("... _BeginFrame: %d (inFlight: %d)", ni::GetLang()->GetFrameNumber(), mnCurrentFrame));
    return eTrue;
  }

  inline void _EndFrame() {
    if (!mbBeganFrame) {
      return;
    }
    _AddCompletedHandler(mCmdEncoder->_EndFrame());
    _AddCompletedHandler(this);
    _EndCommandEncoder();
    mnCurrentFrame = (mnCurrentFrame + 1) % mFrameMaxInFlight;
    mbBeganFrame = eFalse;
    // niDebugFmt(("... _EndFrame: %d (inFlight: %d)", ni::GetLang()->GetFrameNumber(), mnCurrentFrame));
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
  }

  /////////////////////////////////////////////
  virtual tBool __stdcall DrawOperation(iDrawOperation* apDrawOp) {
    niCheckSilent(niIsOK(apDrawOp), eFalse);
    ++mNumDrawOps;

    // niAssert(mbBeganFrame);
    if (!mbBeganFrame) {
      niCheck(_BeginFrame(),eFalse);
    }

    niLet cmdEncoder = mCmdEncoder->_cmdEncoder;

    iVertexArray* va = apDrawOp->GetVertexArray();
    if (!va) {
      return eFalse;
    }

    METAL_TRACE(("DrawOperation BEGIN %s:%s",this->GetWidth(),this->GetHeight()));
    niLet fvf = va->GetFVF();
    niLet pDOMatDesc = (const sMaterialDesc*)apDrawOp->GetMaterial()->GetDescStructPtr();

    iShader* pVS = pDOMatDesc->mShaders[eShaderUnit_Vertex];
    if (!pVS) {
      pVS = mpParent->mFixedShaders.GetVertexShader(fvf,*pDOMatDesc);
      if (!pVS) {
        niError("Can't get fixed vertex shader.");
        return eFalse;
      }
      METAL_TRACE(("VS : %s",pVS->GetDeviceResourceName()));
      FixedShaders_UpdateConstants(this, (iShaderConstants*)pVS->GetConstants(), apDrawOp);
    }

    iShader* pPS = pDOMatDesc->mShaders[eShaderUnit_Pixel];
    if (!pPS) {
      pPS = mpParent->mFixedShaders.GetPixelShader(*pDOMatDesc);
      if (!pPS) {
        niError("Can't get fixed pixel shader.");
        return eFalse;
      }
      METAL_TRACE(("PS : %s",pPS->GetDeviceResourceName()));
      FixedShaders_UpdateConstants(this, (iShaderConstants*)pPS->GetConstants(), apDrawOp);
    }

    const tFixedGpuPipelineId rpId = GetFixedGpuPipelineId(
      mRT0Format, mDSFormat,
      fvf,
      _GetBlendMode(pDOMatDesc),
      (eCompiledStates)_GetRS(pDOMatDesc),
      (eCompiledStates)_GetDS(pDOMatDesc),
      ((iFixedGpuShader*)pVS)->GetGpuFunction(),
      ((iFixedGpuShader*)pPS)->GetGpuFunction());
    niCheck(rpId != 0, eFalse);

    if (rpId != mCurrentFixedGpuPipelineId) {
      sMetalPipeline* pipeline = (sMetalPipeline*)mpParent->mFixedPipelines->GetRenderPipeline(
        mpParent,rpId,
        ((iFixedGpuShader*)pVS)->GetGpuFunction(),
        ((iFixedGpuShader*)pPS)->GetGpuFunction()).raw_ptr();
      if (!pipeline) {
        niError("Can't get the pipeline.");
        return eFalse;
      }
      pipeline->Apply(cmdEncoder);
      mCurrentFixedGpuPipelineId = rpId;
    }

    [cmdEncoder setViewport:mViewport];
    [cmdEncoder setScissorRect:mScissorRect];

    auto applyMaterialChannel = [&](cMetalContextBase* apContext,
                                    iGraphics* apGraphics,
                                    tU32 anTSS, eMaterialChannel aChannel,
                                    const sMaterialDesc* apDOMat)
    {
      const sMaterialChannel& ch = apContext->_GetChannel(apDOMat, aChannel);
      if (ch.mTexture.IsOK()) {
        cMetalTexture* tex = (cMetalTexture*)ch.mTexture->Bind(NULL);
        if (tex == NULL || tex->_tex == nil) {
          // Probably trying to use the frame buffer we don't support this atm...
          [cmdEncoder setFragmentTexture:nil atIndex:anTSS];
        }
        else {
          [cmdEncoder setFragmentTexture:tex->_tex atIndex:anTSS];
          // niDebugFmt(("TEX SET >>> %s",tex->GetDeviceResourceName()));
          [cmdEncoder setFragmentSamplerState: mpParent->_GetMTLSamplerState(ch.mhSS) atIndex: anTSS];
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

    auto updateConstant = [&](eShaderUnit aUnit, const sShaderConstantsDesc* constDesc) {
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

    return DrawOperationSubmitGpuDrawCall(mCmdEncoder,apDrawOp);
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
  virtual tBool __stdcall _EndCommandEncoder() {
    return mptrMetalAPI->PresentAndCommit(nullptr);
  }

  tBool __stdcall _DoResizeContext() {
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
  virtual tBool __stdcall _EndCommandEncoder() {
    const tBool r = (_commandBuffer != NULL);
    if (_commandEncoder) {
      [_commandEncoder endEncoding];
    }
    if (_commandBuffer) {
#ifdef niOSX
      //
      // TODO: The code and note below look wrong and incorrect, needs to be reviewed.
      //

      // NOTE: iOS don't have synchronizeTexture function, maybe there is no
      // need to, we need some test. We need to synchronize the renderTarget
      // here to get the right texture.
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
