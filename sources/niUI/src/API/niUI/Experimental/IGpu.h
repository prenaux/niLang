#pragma once
#ifndef __IGPU_H_990FD559_373C_7E4E_8C0C_63859580DB23__
#define __IGPU_H_990FD559_373C_7E4E_8C0C_63859580DB23__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include <niLang/IDeviceResource.h>
#include <niLang/IHString.h>
#include "../FVF.h"

namespace ni {

enum eLock;
struct iDataTable;

/** \addtogroup niUI
 * @{
 */

//! GPU Buffer memory mode
enum eGpuBufferMemoryMode
{
  //! Default shared memory between CPU & GPU, best for dynamic resources
  //! that are updated frequently by the CPU.
  eGpuBufferMemoryMode_Shared = 0,
  //! Memory only accessible by GPU, optimal for static resources
  //! that won't change after initialization.
  //! \remark Lock() with private buffers always fails and return nullptr. You should use CreateBufferFromData to initialize them.
  eGpuBufferMemoryMode_Private = 1,
  //! Defines a synchronized memory pair for a resource, with one copy in
  //! system memory and another in video memory. Managed resources benefit
  //! from fast CPU and GPU access to each copy of the resource, with minimal
  //! API calls needed to synchronize these copies.
  //! \remark Lock() returns a pointer to the system memory, Unlock synchronizes it with the GPU.
  //! \remark To copy the GPU version of the buffer to the system memory you must use iGraphicsDriverGpu::BlitGpuBufferToSystemMemory().
  eGpuBufferMemoryMode_Managed = 2,

  //! \internal
  eGpuBufferMemoryMode_Last niMaybeUnused = 3,
  //! \internal
  eGpuBufferMemoryMode_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Buffer usage flags
enum eGpuBufferUsageFlags
{
  //! Can be used as vertex buffer
  eGpuBufferUsageFlags_Vertex = niBit(0),
  //! Can be used as index buffer
  eGpuBufferUsageFlags_Index = niBit(1),
  //! Can be used as uniform buffer
  eGpuBufferUsageFlags_Uniform = niBit(2),
  //! Can be used as storage buffer
  eGpuBufferUsageFlags_Storage = niBit(3),
  //! Can be used as indirect buffer
  eGpuBufferUsageFlags_Indirect = niBit(4),
  //! Source for transfer operations
  eGpuBufferUsageFlags_TransferSrc = niBit(5),
  //! Destination for transfer operations
  eGpuBufferUsageFlags_TransferDst = niBit(6),

  //! \internal
  eGpuBufferUsageFlags_Last niMaybeUnused = 8,
  //! \internal
  eGpuBufferUsageFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Buffer usage flags type. \see ni::eGpuBufferUsageFlags
typedef tU32 tGpuBufferUsageFlags;

//! GPU Buffer interface.
struct iGpuBuffer : public iDeviceResource
{
  niDeclareInterfaceUUID(iGpuBuffer,0x64a75ff7,0x630a,0xfa48,0x8c,0x23,0xc3,0xb6,0x51,0x9c,0x04,0x71);

  //##########################################################################
  //! \name General
  //##########################################################################
  //! @{

  //! Get the size of the buffer.
  //! {Property}
  virtual tU32 __stdcall GetSize() const = 0;
  //! Get the memory mode of the buffer.
  //! {Property}
  virtual eGpuBufferMemoryMode __stdcall GetMemoryMode() const = 0;
  //! Get the usage flags of the buffer.
  //! {Property}
  virtual tGpuBufferUsageFlags __stdcall GetUsageFlags() const = 0;
  //! @}

  //##########################################################################
  //! \name Memory Operations
  //##########################################################################
  //! @{

  //! Lock buffer memory for CPU access.
  //! \param anOffset is the offset in the buffer data.
  //! \param anSize is the size to lock.
  //! \param aLock specify to locking mode.
  //! \return a pointer to the buffer data.
  virtual tPtr __stdcall Lock(tU32 anOffset, tU32 anSize, eLock aLock) = 0;
  //! Unlock buffer memory.
  virtual tBool __stdcall Unlock() = 0;
  //! Return true if the buffer is locked.
  //! {Property}
  virtual tBool __stdcall GetIsLocked() const = 0;
  //! @}
};

//! GPU Function type
enum eGpuFunctionType
{
  //! Vertex function
  eGpuFunctionType_Vertex = 0,
  //! Fragment/Pixel function
  eGpuFunctionType_Pixel = 1,
  //! Compute function
  eGpuFunctionType_Compute = 2,
  //! Mesh function
  eGpuFunctionType_Mesh = 3,
  //! Task/Amplification function
  eGpuFunctionType_Task = 4,
  //! Ray generation function
  eGpuFunctionType_RayGeneration = 5,
  //! Ray intersection function
  eGpuFunctionType_RayIntersection = 6,
  //! Ray any hit function
  eGpuFunctionType_RayAnyHit = 7,
  //! Ray closest hit function
  eGpuFunctionType_RayClosestHit = 8,
  //! Ray miss function
  eGpuFunctionType_RayMiss = 9,

  //! \internal
  eGpuFunctionType_Last niMaybeUnused = 10,
  //! \internal
  eGpuFunctionType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Function interface.
struct iGpuFunction : public iDeviceResource
{
  niDeclareInterfaceUUID(iGpuFunction,0xa9fb33ce,0x9b16,0x4e4e,0xa9,0x33,0x53,0x82,0x07,0x42,0x0d,0x92);

  //! Get the gpu function's datatable.
  //! {Property}
  //! \remark The function's datatable describes the complete gpu function's interface:
  //! - Input/Output interface (attribute locations and types)
  //! - Resource bindings (textures, samplers, uniform buffers)
  //! - Platform-specific compiled shader code
  //! - Platform-specific binding decorations (descriptor sets, register bindings)
  //! This data is used to:
  //! - Create gpu pipelines
  //! - Setup resource binding infrastructure (descriptor sets, root signatures)
  //! - Validate resource bindings at runtime
  virtual iDataTable* __stdcall GetDataTable() const = 0;
};

//! GPU Pipeline color format
enum eGpuPipelineColorFormat
{
  eGpuPipelineColorFormat_None = 0,
  eGpuPipelineColorFormat_BGRA8 = 1,
  eGpuPipelineColorFormat_RGBA8 = 2,
  eGpuPipelineColorFormat_RGBA16F = 3,
  eGpuPipelineColorFormat_R16F = 4,
  eGpuPipelineColorFormat_R32F = 5,

  //! \internal
  eGpuPipelineColorFormat_Last niMaybeUnused = 6,
  //! \internal
  eGpuPipelineColorFormat_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Pipeline depth format
enum eGpuPipelineDepthFormat
{
  eGpuPipelineDepthFormat_None = 0,
  eGpuPipelineDepthFormat_D32 = 1,
  eGpuPipelineDepthFormat_D16 = 2,
  eGpuPipelineDepthFormat_D24S8 = 3,

  //! \internal
  eGpuPipelineDepthFormat_Last niMaybeUnused = 4,
  //! \internal
  eGpuPipelineDepthFormat_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Blend factor
enum eGpuBlendFactor
{
  //! (0, 0, 0, 0)
  eGpuBlendFactor_Zero = 0,
  //! (1, 1, 1, 1)
  eGpuBlendFactor_One = 1,
  //! (Rs, Gs, Bs, As)
  eGpuBlendFactor_SrcColor = 2,
  //! (1-Rs, 1-Gs, 1-Bs, 1-As)
  eGpuBlendFactor_InvSrcColor = 3,
  //! (As, As, As, As)
  eGpuBlendFactor_SrcAlpha = 4,
  //! (1-As, 1-As, 1-As, 1-As)
  eGpuBlendFactor_InvSrcAlpha = 5,
  //! (Ad, Ad, Ad, Ad)
  eGpuBlendFactor_DstAlpha = 6,
  //! (1-Ad, 1-Ad, 1-Ad, 1-Ad)
  eGpuBlendFactor_InvDstAlpha = 7,
  //! (Rd, Gd, Bd, Ad)
  eGpuBlendFactor_DstColor = 8,
  //! (1-Rd, 1-Gd, 1-Bd, 1-Ad)
  eGpuBlendFactor_InvDstColor = 9,
  //! (f, f, f, 1) where f = min(As, 1-Ad)
  eGpuBlendFactor_SrcAlphaSat = 10,
  //! Constant blend color
  eGpuBlendFactor_BlendColorConstant = 11,
  //! 1 - Constant blend color
  eGpuBlendFactor_InvBlendColorConstant = 12,

  //! \internal
  eGpuBlendFactor_Last niMaybeUnused = 13,
  //! \internal
  eGpuBlendFactor_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Blend operation
enum eGpuBlendOp {
  eGpuBlendOp_Add = 0,
  eGpuBlendOp_Subtract = 1,
  eGpuBlendOp_ReverseSubtract = 2,
  eGpuBlendOp_Min = 3,
  eGpuBlendOp_Max = 4,

  //! \internal
  eGpuBlendOp_Last niMaybeUnused = 5,
  //! \internal
  eGpuBlendOp_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! GPU Blend mode description
struct sGpuBlendModeDesc {
  eGpuBlendOp     mOp;
  eGpuBlendFactor mSrcRGB;
  eGpuBlendFactor mSrcAlpha;
  eGpuBlendFactor mDstRGB;
  eGpuBlendFactor mDstAlpha;
};

//! GPU Blend mode interface
struct iGpuBlendMode : public iUnknown {
  niDeclareInterfaceUUID(iGpuBlendMode,0x979bda0b,0x8876,0x294f,0x83,0xde,0xad,0x0b,0xdc,0x15,0x98,0x33);

  //! Copy another blend mode.
  virtual tBool __stdcall Copy(const iGpuBlendMode* apBlend) = 0;
  //! Clone this blend mode.
  virtual iGpuBlendMode* __stdcall Clone() const = 0;

  //! Set the blend operation.
  //! {Property}
  virtual tBool __stdcall SetOp(eGpuBlendOp aOp) = 0;
  //! Get the blend operation.
  //! {Property}
  virtual eGpuBlendOp __stdcall GetOp() const = 0;

  //! Set the source RGB blend factor.
  //! {Property}
  virtual tBool __stdcall SetSrcRGB(eGpuBlendFactor aFactor) = 0;
  //! Get the source RGB blend factor.
  //! {Property}
  virtual eGpuBlendFactor __stdcall GetSrcRGB() const = 0;

  //! Set the source alpha blend factor.
  //! {Property}
  virtual tBool __stdcall SetSrcAlpha(eGpuBlendFactor aFactor) = 0;
  //! Get the source alpha blend factor.
  //! {Property}
  virtual eGpuBlendFactor __stdcall GetSrcAlpha() const = 0;

  //! Set the destination RGB blend factor.
  //! {Property}
  virtual tBool __stdcall SetDstRGB(eGpuBlendFactor aFactor) = 0;
  //! Get the destination RGB blend factor.
  //! {Property}
  virtual eGpuBlendFactor __stdcall GetDstRGB() const = 0;

  //! Set the destination alpha blend factor.
  //! {Property}
  virtual tBool __stdcall SetDstAlpha(eGpuBlendFactor aFactor) = 0;
  //! Get the destination alpha blend factor.
  //! {Property}
  virtual eGpuBlendFactor __stdcall GetDstAlpha() const = 0;

  //! Get the blend mode description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;
};

//! Gpu pipeline storage description.
struct sGpuPipelineDesc {
  eGpuPipelineColorFormat mColorFormats[4];
  eGpuPipelineDepthFormat mDepthFormat;
  tFVF                  mFVF;
  tIntPtr               mhRS;
  tIntPtr               mhDS;
  Ptr<iGpuFunction>     mptrFuncs[eGpuFunctionType_Last];
  Ptr<iGpuBlendMode>    mptrBlendMode;
};

//! Gpu pipeline interface.
struct iGpuPipeline : public iUnknown
{
  niDeclareInterfaceUUID(iGpuPipeline,0xbd30dcb2,0x8309,0xb84a,0xa5,0x9f,0xf4,0xa1,0x08,0x6d,0xc9,0x05);

  //! Copy another pipeline.
  virtual tBool __stdcall Copy(const iGpuPipeline* apPipe) = 0;
  //! Clone this pipeline.
  virtual Ptr<iGpuPipeline> __stdcall Clone() const = 0;
  //! Return whether the depth-stencil states are compiled (read-only)
  //! {Property}
  virtual tBool __stdcall GetIsCompiled() const = 0;

  //! Set a color format.
  //! {Property}
  virtual tBool __stdcall SetColorFormat(tU32 anIndex, eGpuPipelineColorFormat aFormat) = 0;
  //! Get a color format.
  //! {Property}
  virtual eGpuPipelineColorFormat __stdcall GetColorFormat(tU32 anIndex) const = 0;

  //! Set the depth format.
  //! {Property}
  virtual tBool __stdcall SetDepthFormat(eGpuPipelineDepthFormat aFormat) = 0;
  //! Get the depth format.
  //! {Property}
  virtual eGpuPipelineDepthFormat __stdcall GetDepthFormat() const = 0;

  //! Set the vertex format.
  //! {Property}
  virtual tBool __stdcall SetFVF(tFVF aFVF) = 0;
  //! Get the vertex format.
  //! {Property}
  virtual tFVF __stdcall GetFVF() const = 0;

  //! Set the rasterizer states.
  //! {Property}
  virtual tBool __stdcall SetRasterizerStates(tIntPtr aHandle) = 0;
  //! Get the rasterizer states.
  //! {Property}
  virtual tIntPtr __stdcall GetRasterizerStates() const = 0;

  //! Set the depth stencil states.
  //! {Property}
  virtual tBool __stdcall SetDepthStencilStates(tIntPtr aHandle) = 0;
  //! Get the depth stencil states.
  //! {Property}
  virtual tIntPtr __stdcall GetDepthStencilStates() const = 0;

  //! Set a function.
  //! {Property}
  virtual tBool __stdcall SetFunction(eGpuFunctionType aType, iGpuFunction* apFunc) = 0;
  //! Get a function.
  //! {Property}
  virtual iGpuFunction* __stdcall GetFunction(eGpuFunctionType aType) const = 0;

  //! Set the blend factors (constants)
  //! {Property}
  virtual void __stdcall SetBlendFactors(const sColor4f& aFactors) = 0;
  //! Get the blend factors (constants)
  //! {Property}
  virtual sColor4f __stdcall GetBlendFactors() const = 0;

  //! Get the pipeline description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;
};

//! GPU-specific graphics context interface.
struct iGraphicsContextGpu : public iUnknown
{
  niDeclareInterfaceUUID(iGraphicsContextGpu,0x0d2ffd6c,0x887d,0x3d46,0xaa,0x18,0x12,0x3b,0x27,0x29,0x54,0xe0);

  //##########################################################################
  //! \name Dynamic states
  //##########################################################################
  //! @{

  //! Set the polygon offset.
  //! \param avOffset Vec2(x: units scale factor, y: depth slope factor)
  //! {Property}
  virtual void __stdcall SetPolygonOffset(const sVec2f& avOffset) = 0;
  //! Get the polygon offset.
  //! {Property}
  virtual sVec2f __stdcall GetPolygonOffset() const = 0;

  //! Set the stencil reference value.
  //! {Property}
  virtual void __stdcall SetStencilReference(tI32 aRef) = 0;
  //! Get the stencil reference value.
  //! {Property}
  virtual tI32 __stdcall GetStencilReference() const = 0;

  //! Set the stencil mask.
  //! {Property}
  virtual void __stdcall SetStencilMask(tU32 aMask) = 0;
  //! Get the stencil mask.
  //! {Property}
  virtual tU32 __stdcall GetStencilMask() const = 0;

  //! Set the blend color constant
  //! {Property}
  virtual void __stdcall SetBlendColorConstant(const sColor4f& aColor) = 0;
  //! Get the blend color constant
  //! {Property}
  virtual sColor4f __stdcall GetBlendColorConstant() const = 0;
  //! @}
};

//! GPU-specific graphics driver interface.
//! \remark WIP/Placeholder
struct iGraphicsDriverGpu : public iUnknown
{
  niDeclareInterfaceUUID(iGraphicsDriverGpu,0xe689ee7e,0xf674,0x1541,0x9a,0x2b,0xb3,0x06,0x55,0x7e,0x6f,0x09);

  //! Create a new GPU buffer.
  virtual Ptr<iGpuBuffer> __stdcall CreateBuffer(tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) = 0;
  //! Create a new GPU buffer with initial data.
  virtual Ptr<iGpuBuffer> __stdcall CreateBufferFromData(iFile* apFile, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) = 0;
  //! Create a new GPU buffer with initial raw data.
  //! {NoAutomation}
  virtual Ptr<iGpuBuffer> __stdcall CreateBufferFromDataRaw(tPtr apData, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) = 0;

  //! Create a new GPU function.
  virtual Ptr<iGpuFunction> __stdcall CreateFunction(iHString* ahspName, const achar* aaszSource, const achar* aaszEntryPoint, eGpuFunctionType aType) = 0;

  //! Create a new GPU pipeline description.
  virtual Ptr<iGpuPipeline> __stdcall CreatePipeline() = 0;

  //! Compile a GPU pipeline description into a driver handle.
  //! \return Handle to the compiled pipeline, 0 if compilation failed.
  virtual tIntPtr __stdcall CompilePipeline(iGpuPipeline* apDesc) = 0;

  //! Create a new GPU blend mode description.
  virtual Ptr<iGpuBlendMode> __stdcall CreateBlendMode() = 0;

  //! Synchronize a managed resource from the GPU to the CPU memory.
  virtual tBool BlitBufferToSystemMemory(iGpuBuffer* apBuffer) = 0;
};

}
#endif // __IGPU_H_990FD559_373C_7E4E_8C0C_63859580DB23__
