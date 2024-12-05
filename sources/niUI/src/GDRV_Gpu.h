#pragma once
#ifndef __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__
#define __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__

#include <niCC.h>
#include "API/niUI/IGraphics.h"
#include "API/niUI/IGpu.h"
#include "API/niUI/GraphicsEnum.h"

namespace ni {

struct sGpuVertexAttribute {
  cString _name;
  tU32 _location = eInvalidHandle;
  tType _type = 0;
  tFVF _fvf = 0;

  cString ToString() const {
    return niFmt("sGpuVertexAttribute { name='%s', loc=%d, type=%s, fvf='%s' (0x%x) }",
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
NN<iGpuPipelineDesc> _CreateGpuPipelineDesc();
NN<iGpuBlendMode> _CreateGpuBlendMode();

typedef tU64 tFixedGpuPipelineId;

niConstValue tU32 knFixedGpuMaxFuncId = niBit(20);
niConstValue tU32 knFixedGpuIndexSize = sizeof(tU32);

// Needed to fit in the pipeline id
static_assert(eBlendMode_Last <= niBit(4));

enum eFixedGpuVertexFormat {
  eFixedGpuVertexFormat_P = 0,
  eFixedGpuVertexFormat_PA = 1,
  eFixedGpuVertexFormat_PAT1 = 2,
  eFixedGpuVertexFormat_PN = 3,
  eFixedGpuVertexFormat_PNA = 4,
  eFixedGpuVertexFormat_PNT1 = 5,
  eFixedGpuVertexFormat_PNAT1 = 6,
  eFixedGpuVertexFormat_PNT2 = 7,
  eFixedGpuVertexFormat_PNAT2 = 8,
  eFixedGpuVertexFormat_PB4INT1 = 9,
  eFixedGpuVertexFormat_PB4INAT1 = 10,
  //! \internal
  eFixedGpuVertexFormat_Last niMaybeUnused = 11,
  //! \internal
  eFixedGpuVertexFormat_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
static_assert(eFixedGpuVertexFormat_Last <= niBit(4));

enum eFixedGpuRS {
  eFixedGpuRS_ColorWriteNone = 0,
  eFixedGpuRS_Filled = 1,
  eFixedGpuRS_Wireframe = 2,
  eFixedGpuRS_NoCullingColorWriteNone = 3,
  eFixedGpuRS_NoCullingFilled = 4,
  eFixedGpuRS_NoCullingWireframe = 5,
  eFixedGpuRS_CWCullingColorWriteNone = 6,
  eFixedGpuRS_CWCullingFilled = 7,
  eFixedGpuRS_CWCullingWireframe = 8,
  //! \internal
  eFixedGpuRS_Last niMaybeUnused = 9,
  //! \internal
  eFixedGpuRS_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
static_assert(eFixedGpuRS_Last <= niBit(4));

enum eFixedGpuDS {
  eFixedGpuDS_NoDepthTest = 0,
  eFixedGpuDS_DepthTestAndWrite = 1,
  eFixedGpuDS_DepthTestOnly = 2,
  eFixedGpuDS_DepthWriteOnly = 3,
  //! \internal
  eFixedGpuDS_Last niMaybeUnused = 4,
  //! \internal
  eFixedGpuDS_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
static_assert(eFixedGpuDS_Last <= niBit(2));

struct sFixedGpuPipelineIdDesc {
  eGpuPixelFormat rt0 : 4;
  eGpuPixelFormat ds : 4;
  eFixedGpuVertexFormat vertexFormat : 4;
  eBlendMode blendMode : 4;
  eFixedGpuRS compiledRS : 4;
  eFixedGpuDS compiledDS : 2;
  tU32 funcVertex : 20;
  tU32 funcPixel : 20;
  tU32 padding : 2;

  tFixedGpuPipelineId ToId() const {
    return *(tFixedGpuPipelineId*)this;
  }

  static sFixedGpuPipelineIdDesc FromId(tFixedGpuPipelineId aId) {
    return *(sFixedGpuPipelineIdDesc*)&aId;
  }

  cString ToString() const {
    return niFmt("sFixedGpuPipelineIdDesc{RT0=%d,DS=%d,VF=%d,BM=%d,RS=%d,DS=%d,VS=%d,PS=%d}",
                 rt0,ds,vertexFormat,blendMode,
                 compiledRS,compiledDS,
                 funcVertex,funcPixel);
  }
} niPacked(4);
static_assert(sizeof(sFixedGpuPipelineIdDesc) == sizeof(tU64));

struct iFixedGpuPipelines : public iUnknown {
  niDeclareInterfaceUUID(iFixedGpuPipelines,0x716fc8a9,0x30e6,0x3641,0xb4,0xf2,0x69,0x0f,0xb8,0x83,0x58,0xa7);

  //! Get or create a render pipeline for the given id and shaders
  virtual Ptr<iGpuPipeline> __stdcall GetRenderPipeline(iGraphicsDriverGpu* apGpuDriver, tFixedGpuPipelineId aPipelineId, iGpuFunction* apFuncVertex, iGpuFunction* apFuncPixel) = 0;

  virtual iGpuFunction* __stdcall GetFixedGpuFuncVertex(ain<tFVF> aFVF) const = 0;
  virtual iGpuFunction* __stdcall GetFixedGpuFuncPixel(ain<sMaterialDesc> aMatDesc) const = 0;

  virtual nn<iTexture> __stdcall GetWhiteTexture() const = 0;

  virtual tBool __stdcall ClearRect(iGpuCommandEncoder* apCmdEncoder, ain<sVec2f> aPixelSize, tClearBuffersFlags aFlags, ain<sRectf> aRect, tU32 anColor, tF32 afZ) = 0;
};

struct sGpuStreamBlock {
  //! Underlying GPU buffer holding the stream data.
  Ptr<iGpuBuffer> mBuffer;
  //! Byte offset from buffer start where this block begins.
  tU32 mOffset;
  //! Actual size in bytes of the data written to this block.
  tU32 mSize;
  //! Size in bytes after hardware alignment, used for offset calculations.
  //! Takes into account GPU/driver/OS alignment requirements and may be
  //! larger than mSize.
  tU32 mAlignedSize;
};

struct iGpuStream : public iUnknown {
  niDeclareInterfaceUUID(iGpuStream,0x7fcdb7ba,0x1dc1,0xef4a,0xa7,0xc2,0xf2,0x98,0xfd,0x47,0x76,0xfb);

  virtual tBool __stdcall UpdateNextBlock(const tPtr apData, tU32 anSize) = 0;
  virtual void __stdcall Reset() = 0;

  virtual const sGpuStreamBlock& __stdcall GetLastBlock() const = 0;
  virtual iGpuBuffer* __stdcall GetLastBuffer() const = 0;
  virtual tU32 __stdcall GetLastOffset() const = 0;
  virtual tU32 __stdcall GetLastSize() const = 0;

  virtual tU32 __stdcall GetNumBlocks() const = 0;
};

iVertexArray* CreateFixedGpuVertexArray(iGraphicsDriverGpu* apGpuDriver, tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage);
iGpuBuffer* GetVertexArrayGpuBuffer(iVertexArray* apVA);
sVec2i GetVertexArrayFvfAndStride(iVertexArray* apVA);

iIndexArray* CreateFixedGpuIndexArray(iGraphicsDriverGpu* apGpuDriver, eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndices, tU32 anMaxVertexIndex, eArrayUsage aUsage);
iGpuBuffer* GetIndexArrayGpuBuffer(iIndexArray* apVA);

Ptr<iDataTable> GpuFunctionDT_Load(const achar* aURL, iHString* ahspTarget);
cString GpuFunctionDT_GetSourceText(ain<nn<iDataTable>> aDT);
Ptr<iFile> GpuFunctionDT_GetSourceData(ain<nn<iDataTable>> aDT);

tFixedGpuPipelineId GetFixedGpuPipelineId(
  eGpuPixelFormat aRT0Format,
  eGpuPixelFormat aDSFormat,
  tFVF aFVF,
  eBlendMode aBlendMode,
  eCompiledStates aRS,
  eCompiledStates aDS,
  const iGpuFunction* apFuncVertex,
  const iGpuFunction* apFuncPixel);

Ptr<iGpuPipeline> CreateFixedGpuPipeline(
  iGraphicsDriverGpu* apGpuDriver,
  tFixedGpuPipelineId aPipelineId,
  iGpuFunction* apFuncVertex,
  iGpuFunction* apFuncPixel);

tBool DrawOperationSubmitGpuDrawCall(
  iGpuCommandEncoder* apCmdEncoder,
  iDrawOperation* apDrawOp);

Ptr<iFixedGpuPipelines> CreateFixedGpuPipelines(iGraphicsDriver* apGpuDriver);

Ptr<iGpuStream> CreateGpuStream(
  ain<nn<iGraphicsDriverGpu>> apDriver,
  tGpuBufferUsageFlags aUsageFlags,
  tU32 aBlockAlignment = 0,
  tU32 aChunkSize = 65536,
  tU32 aMaxChunks = 1024);

tBool UpdateGpuStreamToVertexBuffer(iGpuStream* apStream, iGpuCommandEncoder* apEncoder, const tPtr apData, tU32 anSize, tU32 anBinding);
tBool UpdateGpuStreamToIndexBuffer(iGpuStream* apStream, iGpuCommandEncoder* apEncoder, const tPtr apData, tU32 anSize, eGpuIndexType aIndexType);
tBool UpdateGpuStreamToUniformBuffer(iGpuStream* apStream, iGpuCommandEncoder* apEncoder, const tPtr apData, tU32 anSize, tU32 anBinding);

}
#endif // __GDRV_GPU_H_40BE24B3_D4BE_3B4B_A652_DE433A111C36__
