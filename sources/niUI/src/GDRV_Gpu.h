#pragma once
#ifndef __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__
#define __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__

#include <niCC.h>
#include "API/niUI/IGraphics.h"
#include "API/niUI/IGpu.h"
#include "API/niUI/GraphicsEnum.h"
#include "API/niUI/IShader.h"

namespace ni {

struct sGpuVertexAttribute {
  cString _name;
  tU32 _location = eInvalidHandle;
  tType _type = 0;
  tFVF _fvf = 0;

  cString ToString() const {
    return niFmt("sGpuVertexAttribute { name='%s', loc: %d, type: %s, fvf: '%s' (0x%x) }",
                 _name, _location,
                 ni::GetTypeString(_type),
                 ni::FVFToShortString(_fvf), _fvf);
  }
};

eGpuPixelFormat _GetClosestGpuPixelFormatForRT(const achar* aRTFormat);
eGpuPixelFormat _GetClosestGpuPixelFormatForDS(const achar* aDSFormat);
eGpuPixelFormat _GetClosestGpuPixelFormatForTexture(const achar* aTexFormat);
iPixelFormat* _GetIPixelFormat(iGraphics* apGraphics, eGpuPixelFormat aFormat);

const sGpuBlendModeDesc& _BlendModeToGpuBlendModeDesc(eBlendMode aBlendMode);
iGpuPipelineDesc* _CreateGpuPipelineDesc();
iGpuBlendMode* _CreateGpuBlendMode();

// 128 bit unsigned int for pipeline id
// 32 for FVF
// 32 for vertex shader id
// 32 for pixel shader id
// 32 for the reset flags
typedef unsigned __int128 tFixedGpuPipelineId;

// Needed for our cache to work
niCAssert(eGpuPixelFormat_Last < niBit(4));
niCAssert(eBlendMode_Last < niBit(4));

struct sFixedGpuPipelineId {
  tU32 fvf : 32;
  eGpuPixelFormat rt0 : 4;
  eGpuPixelFormat rt1 : 4;
  eGpuPixelFormat rt2 : 4;
  eGpuPixelFormat rt3 : 4;
  eGpuPixelFormat ds : 4;
  tU32 blendMode : 4;
  tU32 vertFuncId : 24;
  tU32 fragFuncId : 24;

  sFixedGpuPipelineId() {
    (tFixedGpuPipelineId&)(*this) = 0;
  }

  void Clear() { (tFixedGpuPipelineId&)(*this) = 0; };

  tBool IsNull() const {
    return (tFixedGpuPipelineId&)(*this) == 0;
  }
  cString ToString() const {
    return niFmt(
      "fvf: %d, rt: [%d,%d,%d,%d], ds: %d, blendMode: %d, vertFunc: %d, fragFunc: %d",
      fvf, rt0, rt1, rt2, rt3, ds, blendMode, vertFuncId, fragFuncId);
  }
  operator tFixedGpuPipelineId () const {
    return (tFixedGpuPipelineId&)(*this);
  }
};
niCAssert(sizeof(sFixedGpuPipelineId) == sizeof(tFixedGpuPipelineId));

struct iFixedGpuShader : public iShader {
  niDeclareInterfaceUUID(iFixedGpuShader,0x27897b00,0x0c78,0x4815,0x92,0x2e,0x17,0x38,0x5f,0xef,0x23,0x97);

  virtual tU32 __stdcall GetUID() const = 0;
  virtual iGpuFunction* __stdcall GetGpuFunction() const = 0;
};

struct iFixedGpuPipelines : public iUnknown {
  niDeclareInterfaceUUID(iFixedGpuPipelines,0x716fc8a9,0x30e6,0x3641,0xb4,0xf2,0x69,0x0f,0xb8,0x83,0x58,0xa7);

  //! Compile a shader from source code
  virtual Ptr<iGpuFunction> __stdcall CompileShader(iGraphicsDriverGpu* apGpuDriver, eGpuFunctionType aType, iHString* ahspName, const achar* aShaderProgram) = 0;

  //! Get or create a render pipeline for the given id and shaders
  virtual Ptr<iGpuPipeline> __stdcall GetRenderPipeline(iGraphicsDriverGpu* apGpuDriver, ain<tFixedGpuPipelineId> aPipelineId, iFixedGpuShader* apVS, iFixedGpuShader* apPS) = 0;

  //! Create a new fixed gpu shader
  virtual Ptr<iFixedGpuShader> __stdcall CreateFixedGpuShader(iGraphics* apGraphics, iGpuFunction* apFunc, iHString* ahspName) = 0;
};

iFixedGpuPipelines* CreateFixedGpuPipelines();

static const tU32 knFixedGpuIndexSize = sizeof(tU32);

iVertexArray* CreateFixedGpuVertexArray(iGraphicsDriverGpu* apGpuDriver, tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage);
iGpuBuffer* GetVertexArrayGpuBuffer(iVertexArray* apVA);
sVec2i GetVertexArrayFvfAndStride(iVertexArray* apVA);

iIndexArray* CreateFixedGpuIndexArray(iGraphicsDriverGpu* apGpuDriver, eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndices, tU32 anMaxVertexIndex, eArrayUsage aUsage);
iGpuBuffer* GetIndexArrayGpuBuffer(iIndexArray* apVA);

NN<iDataTable> CreateGpuFunctionDT(iGraphicsDriverGpu* apGpuDriver, const achar* aaszSource);
iDataTable* FindGpuFunctionDT(iDataTable* apDT, const iHString* ahspTarget);

}
#endif // __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__
