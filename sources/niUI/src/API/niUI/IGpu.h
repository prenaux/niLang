#pragma once
#ifndef __IGPU_H_990FD559_373C_7E4E_8C0C_63859580DB23__
#define __IGPU_H_990FD559_373C_7E4E_8C0C_63859580DB23__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>
#include <niLang/IDeviceResource.h>
#include <niLang/IHString.h>
#include <niLang/Math/MathRect.h>
#include "FVF.h"
#include "GraphicsEnum.h"
#include "IAccelerationStructure.h"
#include "GpuEnum.h"

namespace ni {

struct iDataTable;
struct iTexture;

/** \addtogroup niUI
 * @{
 */

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

//! GPU Function interface.
struct iGpuFunction : public iDeviceResource
{
  niDeclareInterfaceUUID(iGpuFunction,0xa9fb33ce,0x9b16,0x4e4e,0xa9,0x33,0x53,0x82,0x07,0x42,0x0d,0x92);

  //! Get the function's id.
  //! {Property}
  virtual tU32 __stdcall GetFunctionId() const = 0;

  //! Get the function's type.
  //! {Property}
  virtual eGpuFunctionType __stdcall GetFunctionType() const = 0;

  //! Get the function's bind type.
  //! {Property}
  virtual eGpuFunctionBindType __stdcall GetFunctionBindType() const = 0;

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
  virtual Ptr<iGpuBlendMode> __stdcall Clone() const = 0;

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
  eGpuPixelFormat    mColorFormats[4];
  eGpuPixelFormat    mDepthFormat;
  tFVF               mFVF;
  tIntPtr            mhRS;
  tIntPtr            mhDS;
  Ptr<iGpuBlendMode> mptrBlendMode;
  Ptr<iGpuFunction>  mptrFuncs[eGpuFunctionType_Last];
};

//! Gpu pipeline desc interface.
struct iGpuPipelineDesc : public iUnknown
{
  niDeclareInterfaceUUID(iGpuPipelineDesc,0xbd30dcb2,0x8309,0xb84a,0xa5,0x9f,0xf4,0xa1,0x08,0x6d,0xc9,0x05);

  //! Copy another pipeline.
  virtual tBool __stdcall Copy(const iGpuPipelineDesc* apPipe) = 0;
  //! Clone this pipeline.
  virtual Ptr<iGpuPipelineDesc> __stdcall Clone() const = 0;

  //! Set a color format.
  //! {Property}
  virtual tBool __stdcall SetColorFormat(tU32 anIndex, eGpuPixelFormat aFormat) = 0;
  //! Get a color format.
  //! {Property}
  virtual eGpuPixelFormat __stdcall GetColorFormat(tU32 anIndex) const = 0;

  //! Set the depth format.
  //! {Property}
  virtual tBool __stdcall SetDepthFormat(eGpuPixelFormat aFormat) = 0;
  //! Get the depth format.
  //! {Property}
  virtual eGpuPixelFormat __stdcall GetDepthFormat() const = 0;

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

  //! Set the blend mode. Set to nullptr to disable.
  //! {Property}
  virtual tBool __stdcall SetBlendMode(iGpuBlendMode* apBlendMode) = 0;
  //! Get the blend mode.
  //! {Property}
  virtual iGpuBlendMode* __stdcall GetBlendMode() const = 0;

  //! Get the pipeline description structure pointer.
  //! {Property}
  virtual tPtr __stdcall GetDescStructPtr() const = 0;
};

struct iGpuPipeline : public iDeviceResource
{
  niDeclareInterfaceUUID(iGpuPipeline,0x4298b417,0xddeb,0xbf4d,0x82,0x5c,0x12,0x5b,0x3e,0x0b,0x5b,0x05);

  //! Get the gpu pipeline description.
  //! {Property}
  virtual const iGpuPipelineDesc* __stdcall GetDesc() const = 0;
};

//! Ray function table interface
//! \remark Defines the complete set of functions used in a ray tracing pipeline
struct iRayGpuFunctionTable : public iUnknown {
  niDeclareInterfaceUUID(iRayGpuFunctionTable,0xce0a2620,0xc4bc,0xef11,0xb1,0xc9,0x23,0xa8,0x14,0x62,0x0a,0xaf);

  //! Set the ray generation function.
  //! \remark Only one ray generation function is allowed per pipeline.
  virtual tBool __stdcall SetRayGenFunction(iGpuFunction* apFunction) = 0;
  //! Set the miss function
  //! \remark Optional, at most one miss function per pipeline.
  virtual tBool __stdcall SetMissFunction(iGpuFunction* apFunction) = 0;

  //! Add a hit function group.
  //! \param ahspName Name of the hit group. Used for debugging.
  //! \param aType Type of hit group (triangles or procedural).
  //! \param apClosestHit Closest hit function.
  //! \param apAnyHit Optional any-hit function.
  //! \param apIntersection Optional intersection function for procedural geometry.
  //! \return Hit group ID, or eInvalidHandle if failed.
  //! \remark Hit groups are immutable once the pipeline is created.
  virtual tU32 __stdcall AddHitGroup(
    iHString* ahspName,
    eRayGpuFunctionGroupType aType,
    iGpuFunction* apClosestHit,
    iGpuFunction* apAnyHit = nullptr,
    iGpuFunction* apIntersection = nullptr) = 0;
};

//! Ray tracing gpu pipeline interface
//! \remark Ray execution order: Generation -> [Intersection -> Any Hit] -> Closest Hit or Miss
struct iRayGpuPipeline : public iDeviceResource {
  niDeclareInterfaceUUID(iRayGpuPipeline,0x54c330ff,0xc3bc,0xef11,0x9b,0xeb,0x2d,0x23,0xa2,0x57,0x56,0x3e);

  //! Get ray generation function
  virtual iGpuFunction* __stdcall GetRayGenFunction() const = 0;
  //! Get miss function
  virtual iGpuFunction* __stdcall GetMissFunction() const = 0;
  //! Get function table
  virtual iRayGpuFunctionTable* __stdcall GetFunctionTable() const = 0;
};

struct iGpuCommandEncoder : public iUnknown {
  niDeclareInterfaceUUID(iGpuCommandEncoder,0x055a196d,0x4ae9,0x7648,0xa8,0x6e,0x5e,0x90,0xaf,0xf2,0x16,0xce);

  //##########################################################################
  //! \name Resource Bindings
  //##########################################################################
  //! @{

  //! Binds a GPU pipeline to the command encoder.
  virtual void __stdcall SetPipeline(iGpuPipeline* apPipeline) = 0;

  //! Binds a vertex buffer to the specified binding slot.
  //! \param apBuffer The GPU buffer containing vertex data
  //! \param anOffset Byte offset into the buffer
  //! \param anBinding The binding slot index
  virtual void __stdcall SetVertexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) = 0;

  //! Binds an index buffer for indexed draw calls.
  //! \param apBuffer The GPU buffer containing index data
  //! \param anOffset Byte offset into the buffer
  virtual void __stdcall SetIndexBuffer(iGpuBuffer* apBuffer, tU32 anOffset, eGpuIndexType aIndexType) = 0;

  //! Binds a uniform buffer to the specified binding slot.
  //! \param apBuffer The GPU buffer containing uniform data
  //! \param anOffset Byte offset into the buffer
  //! \param anBinding The binding slot index
  virtual void __stdcall SetUniformBuffer(iGpuBuffer* apBuffer, tU32 anOffset, tU32 anBinding) = 0;

  //! Binds a texture to the specified binding slot.
  //! \param apTexture The texture to bind
  //! \param anBinding The binding slot index
  virtual void __stdcall SetTexture(iTexture* apTexture, tU32 anBinding) = 0;

  //! Binds a sampler state to the specified binding slot.
  //! \param ahSS Handle to the sampler state
  //! \param anBinding The binding slot index
  virtual void __stdcall SetSamplerState(tIntPtr ahSS, tU32 anBinding) = 0;
  //! @}

  //##########################################################################
  //! \name Streamed Resources
  //##########################################################################
  //! @{

  //! Stream vertex data through an internally managed GPU stream.
  //! \param apData Pointer to the vertex data to upload
  //! \param anSize Size of the data in bytes
  //! \param anBinding Vertex buffer binding slot index
  //! \return eTrue on success, eFalse if the upload or binding fails
  virtual tBool __stdcall StreamVertexBuffer(const tPtr apData, tU32 anSize, tU32 anBinding) = 0;

  //! Stream index data through an internally managed GPU stream.
  //! \param apData Pointer to the index data to upload
  //! \param anSize Size of the data in bytes
  //! \param aIndexType Type of indices (16 or 32-bit)
  //! \return eTrue on success, eFalse if the upload or binding fails
  virtual tBool __stdcall StreamIndexBuffer(const tPtr apData, tU32 anSize, eGpuIndexType aIndexType) = 0;

  //! Stream uniform data through an internally managed GPU stream.
  //! \param apData Pointer to the uniform data to upload
  //! \param anSize Size of the data in bytes
  //! \param anBinding Uniform buffer binding slot index
  //! \return eTrue on success, eFalse if the upload or binding fails
  virtual tBool __stdcall StreamUniformBuffer(const tPtr apData, tU32 anSize, tU32 anBinding) = 0;
  //! @}

  //##########################################################################
  //! \name Dynamic States
  //##########################################################################
  //! @{

  //! Sets the polygon offset for depth bias.
  //! \param avOffset Vector containing the constant and slope factors
  virtual void __stdcall SetPolygonOffset(const sVec2f& avOffset) = 0;

  //! Sets the scissor test rectangle.
  //! \param aRect Rectangle defining the scissor region
  virtual void __stdcall SetScissorRect(const sRecti& aRect) = 0;

  //! Sets the viewport dimensions.
  //! \param aRect Rectangle defining the viewport region
  virtual void __stdcall SetViewport(const sRecti& aRect) = 0;

  //! Sets the stencil reference value.
  //! \param aRef The reference value to compare against
  virtual void __stdcall SetStencilReference(tI32 aRef) = 0;

  //! Sets the stencil read/write mask.
  //! \param aMask The stencil mask value
  virtual void __stdcall SetStencilMask(tU32 aMask) = 0;

  //! Sets the blend color constant.
  //! \param aColor The color to use for constant blend factor
  virtual void __stdcall SetBlendColorConstant(const sColor4f& aColor) = 0;
  //! @}

  //##########################################################################
  //! \name Draw Commands
  //##########################################################################
  //! @{

  //! Draws primitives using the current vertex buffer.
  //! \param anVertexCount Number of vertices to draw
  //! \param anFirstVertex Index of first vertex to draw
  virtual tBool __stdcall Draw(eGraphicsPrimitiveType aPrimType, tU32 anVertexCount, tU32 anFirstVertex) = 0;

  //! Draws indexed primitives using the current index buffer.
  //! \param anNumIndices Number of indices to draw
  //! \param anFirstIndex Index of first index to draw
  virtual tBool __stdcall DrawIndexed(eGraphicsPrimitiveType aPrimType, tU32 anNumIndices, tU32 anFirstIndex) = 0;
  //! @}

 //##########################################################################
  //! \name Ray Tracing Commands (WIP)
  //##########################################################################
  //! @{

  //! Build acceleration structure
  virtual tBool __stdcall BuildAccelerationStructure(iAccelerationStructure* apAS) = 0;

  //! Dispatch rays
  virtual tBool __stdcall DispatchRays(iRayGpuPipeline* apPipeline, iTexture* apOutputImage) = 0;
  //! @}
};

//! GPU-specific graphics context interface.
struct iGraphicsContextGpu : public iUnknown
{
  niDeclareInterfaceUUID(iGraphicsContextGpu,0x0d2ffd6c,0x887d,0x3d46,0xaa,0x18,0x12,0x3b,0x27,0x29,0x54,0xe0);

  //! Get the current command encoder
  virtual iGpuCommandEncoder* __stdcall GetCommandEncoder() = 0;

  virtual tBool __stdcall ClearBuffersRect(tClearBuffersFlags aFlags, const sRectf& aRect, tU32 anColor, tF32 afZ) = 0;
};

//! GPU-specific graphics driver interface.
//! \remark WIP/Placeholder
struct iGraphicsDriverGpu : public iUnknown
{
  niDeclareInterfaceUUID(iGraphicsDriverGpu,0xe689ee7e,0xf674,0x1541,0x9a,0x2b,0xb3,0x06,0x55,0x7e,0x6f,0x09);

  //! Create a new GPU buffer.
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBuffer(iHString* ahspName, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) = 0;
  //! Create a new GPU buffer with initial data.
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromData(iHString* ahspName, iFile* apFile, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) = 0;
  //! Create a new GPU buffer with initial raw data.
  //! {NoAutomation}
  virtual Ptr<iGpuBuffer> __stdcall CreateGpuBufferFromDataRaw(iHString* ahspName, tPtr apData, tU32 anSize, eGpuBufferMemoryMode aMemMode, tGpuBufferUsageFlags aUsage) = 0;

  //! Get the gpu function target.
  //! {Property}
  virtual iHString* __stdcall GetGpuFunctionTarget() const = 0;
  //! Create a new GPU function.
  virtual Ptr<iGpuFunction> __stdcall CreateGpuFunction(eGpuFunctionType aType, iHString* ahspPath) = 0;

  //! Create a new GPU pipeline description.
  virtual Ptr<iGpuPipelineDesc> __stdcall CreateGpuPipelineDesc() = 0;

  //! Create a new GPU blend mode description.
  virtual Ptr<iGpuBlendMode> __stdcall CreateGpuBlendMode() = 0;

  //! Compile a GPU pipeline description into a driver handle.
  virtual Ptr<iGpuPipeline> __stdcall CreateGpuPipeline(iHString* ahspName, const iGpuPipelineDesc* apDesc) = 0;

  //! Synchronize a managed resource from the GPU to the CPU memory.
  virtual tBool __stdcall BlitManagedGpuBufferToSystemMemory(iGpuBuffer* apBuffer) = 0;

  //! Create ray tracing pipeline
  virtual Ptr<iRayGpuPipeline> __stdcall CreateRayPipeline(
    iHString* ahspName,
    iRayGpuFunctionTable* apFunctionTable) = 0;

  //! Create ray tracing function table.
  virtual Ptr<iRayGpuFunctionTable> __stdcall CreateRayFunctionTable() = 0;

  //! Create a primitives acceleration structure
  virtual Ptr<iAccelerationStructurePrimitives> __stdcall CreateAccelerationStructurePrimitives(iHString* ahspName) = 0;

  //! Create a instances acceleration structure
  virtual Ptr<iAccelerationStructureInstances> __stdcall CreateAccelerationStructureInstances(iHString* ahspName) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IGPU_H_990FD559_373C_7E4E_8C0C_63859580DB23__
