#pragma once
#ifndef __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__
#define __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__

#include "API/niUI/Experimental/IGpu.h"
#include "API/niUI/GraphicsEnum.h"
#include "API/niUI/IShader.h"

namespace ni {

const sGpuBlendModeDesc& ToGpuBlendModeDesc(eBlendMode aBlendMode);
iGpuPipelineDesc* CreateGpuPipelineDesc();

iGpuBlendMode* CreateGpuBlendMode();

// 128 bit unsigned int for pipeline id
// 32 for FVF
// 32 for vertex shader id
// 32 for pixel shader id
// 32 for the reset flags
typedef unsigned __int128 tFixedGpuPipelineId;

struct sFixedGpuPipelineId {
  tU32 fvf : 32;
  eGpuPipelineColorFormat rt0 : 4;
  eGpuPipelineColorFormat rt1 : 4;
  eGpuPipelineColorFormat rt2 : 4;
  eGpuPipelineColorFormat rt3 : 4;
  eGpuPipelineDepthFormat ds : 2;
  tU32 blendMode : 4;
  tU32 colorWriteMask : 4;
  tU32 msaa : 4;
  tU32 alphaToCoverage : 2;
  tU32 vertFuncId : 32;
  tU32 fragFuncId : 32;

  sFixedGpuPipelineId() {
    (tFixedGpuPipelineId&)(*this) = 0;
  }

  void Clear() { (tFixedGpuPipelineId&)(*this) = 0; };

  tBool IsNull() const {
    return (tFixedGpuPipelineId&)(*this) == 0;
  }
  cString ToString() const {
    return niFmt("fvf: %d, rt: [%d,%d,%d,%d], ds: %d, blendMode: %d, colorWriteMask: %d, msaa: %d, alphaToCoverage: %d, vertFunc: %d, fragFunc: %d",
                 fvf, rt0, rt1, rt2, rt3, ds, blendMode, colorWriteMask, msaa, alphaToCoverage, vertFuncId, fragFuncId);
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
  virtual Ptr<iGpuFunction> __stdcall CompileShader(iGraphicsDriverGpu* apGpuDriver, iHString* ahspName, const achar* aShaderProgram) = 0;

  //! Get or create a render pipeline for the given id and shaders
  virtual Ptr<iGpuPipeline> __stdcall GetRenderPipeline(iGraphicsDriverGpu* apGpuDriver, ain<tFixedGpuPipelineId> aPipelineId, iFixedGpuShader* apVS, iFixedGpuShader* apPS) = 0;

  //! Create a new fixed gpu shader
  virtual Ptr<iFixedGpuShader> __stdcall CreateFixedGpuShader(iGraphics* apGraphics, iGpuFunction* apFunc, iHString* ahspName) = 0;
};

iFixedGpuPipelines* CreateFixedGpuPipelines();

}
#endif // __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__
