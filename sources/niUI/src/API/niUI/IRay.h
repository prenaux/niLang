#pragma once
#ifndef __IRAY_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
#define __IRAY_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
#include <niLang/Types.h>
#include <niLang/IDeviceResource.h>
#include <niLang/IHString.h>
#include "GpuEnum.h"

namespace ni {

struct iGpuBuffer;
struct iGpuCommandEncoder;

/** \addtogroup niUI
 * @{
 */

//! Acceleration structure primitive flags
enum eRayPrimitiveFlags {
  //! No special properties
  eRayPrimitiveFlags_None = 0,
  //! Primitive is fully opaque, allows skipping any-hit functions
  eRayPrimitiveFlags_Opaque = niBit(0),
  //! \internal
  eRayPrimitiveFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
//! \see ni::eRayPrimitiveFlags
typedef tU32 tRayPrimitiveFlags;

//! Acceleration structure instance flags
enum eRayInstanceFlags {
  //! No special instance properties
  eRayInstanceFlags_None = 0,
  //! Disable face culling for this instance
  eRayInstanceFlags_DisableCulling = niBit(0),
  //! Invert face culling for this instance
  eRayInstanceFlags_FlipCulling = niBit(1),
  //! Force instance to be opaque, skipping any-hit functions
  eRayInstanceFlags_ForceOpaque = niBit(2),
  //! Force instance to be non-opaque, always running any-hit functions
  eRayInstanceFlags_ForceNonOpaque = niBit(3),
  //! \internal
  eRayInstanceFlags_ForceDWORD = 0xFFFFFFFF
};
//! \see ni::eRayInstanceFlags
typedef tU32 tRayInstanceFlags;

//! Acceleration structure primitives description interface
struct iRayPrimitivesDesc : public iDeviceResource {
  niDeclareInterfaceUUID(iRayPrimitivesDesc,0x2d88524d,0x3d15,0x455c,0x9d,0x13,0x3f,0x95,0xf9,0x0b,0x86,0x19);

  //! Get the number of primitives in the builder.
  //! {Property}
  virtual tU32 __stdcall GetNumPrimitives() const = 0;

  //! Add non-indexed triangle geometry to primitive acceleration structure.
  //! \remark Valid only for eRayType_Primitives
  virtual tBool __stdcall AddTriangles(
    iGpuBuffer* apVertices,
    tU32 anVertexOffset,
    tU32 anVertexStride,
    tU32 anVertexCount,
    const sMatrixf& aTransform,
    tRayPrimitiveFlags aFlags,
    tU32 anHitGroup) = 0;

  //! Add indexed triangle geometry to primitive acceleration structure.
  //! \remark Valid only for eRayType_Primitives
  virtual tBool __stdcall AddTrianglesIndexed(
    iGpuBuffer* apVertices,
    tU32 anVertexOffset,
    tU32 anVertexStride,
    tU32 anVertexCount,
    iGpuBuffer* apIndices,
    tU32 anIndexOffset,
    eGpuIndexType anIndexType,
    tU32 anIndexCount,
    const sMatrixf& aTransform,
    tRayPrimitiveFlags aFlags,
    tU32 anHitGroup) = 0;

  //! Add procedural geometry to primitive acceleration structure using axis-aligned bounding boxes.
  //! \remarks The hit group must include an intersection shader for the procedural geometry.
  //! \remarks AABB is an array of ni::cAABBf
  //! \remark Valid only for eRayType_Primitives
  virtual tBool __stdcall AddProceduralAABBs(
    iGpuBuffer* apAABBs,
    tU32 anAABBOffset,
    tU32 anAABBStride,
    tU32 anAABBCount,
    const sMatrixf& aTransform,
    tRayPrimitiveFlags aFlags,
    tU32 anHitGroup) = 0;
};

//! Ray primitives acceleration structure interface
struct iRayPrimitives : public iDeviceResource {
  niDeclareInterfaceUUID(iRayPrimitives,0x53349815,0xa1dc,0x4334,0x9b,0x70,0x0a,0x0d,0xf3,0x5d,0x71,0xb7);
};

//! Acceleration structure instances description interface
struct iRayInstancesDesc : public iDeviceResource {
  niDeclareInterfaceUUID(iRayInstancesDesc,0x4224e75a,0xaef7,0x44f9,0xb3,0x0c,0x77,0xfd,0x2e,0x9a,0xb3,0x68);

  //! Get the number of instances in the builder.
  //! {Property}
  virtual tU32 __stdcall GetNumInstances() const = 0;

  //! Add an instance.
  //! \remark Valid only for eRayType_Instances
  virtual tBool __stdcall AddInstance(
    iRayPrimitives* apAS,
    const sMatrixf& aTransform,
    tU32 anInstanceId,
    tU8 anMask,
    tU32 anHitGroup,
    tRayInstanceFlags aFlags) = 0;
};

//! Ray instances acceleration structure interface
struct iRayInstances : public iDeviceResource {
  niDeclareInterfaceUUID(iRayInstances,0xb03f4031,0xc4b2,0x48f8,0x89,0x0c,0x18,0x91,0xf2,0x52,0xad,0x73);
};

//! Ray function group type for hit functions
enum eRayFunctionGroupType {
  //! Hit group for built-in triangle intersection
  eRayFunctionGroupType_Triangles = 0,
  //! Hit group with custom intersection function
  eRayFunctionGroupType_Procedural = 1,
  //! \internal
  eRayFunctionGroupType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Ray function table interface
//! \remark Defines the complete set of functions used in a ray tracing pipeline
struct iRayFunctionTable : public iUnknown {
  niDeclareInterfaceUUID(iRayFunctionTable,0xce0a2620,0xc4bc,0xef11,0xb1,0xc9,0x23,0xa8,0x14,0x62,0x0a,0xaf);

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
    eRayFunctionGroupType aType,
    iGpuFunction* apClosestHit,
    iGpuFunction* apAnyHit = nullptr,
    iGpuFunction* apIntersection = nullptr) = 0;
};

//! Ray tracing gpu pipeline interface
//! \remark Ray execution order: Generation -> [Intersection -> Any Hit] -> Closest Hit or Miss
struct iRayPipeline : public iDeviceResource {
  niDeclareInterfaceUUID(iRayPipeline,0x54c330ff,0xc3bc,0xef11,0x9b,0xeb,0x2d,0x23,0xa2,0x57,0x56,0x3e);

  //! Get ray generation function
  virtual iGpuFunction* __stdcall GetRayGenFunction() const = 0;
  //! Get miss function
  virtual iGpuFunction* __stdcall GetMissFunction() const = 0;
  //! Get function table
  virtual iRayFunctionTable* __stdcall GetFunctionTable() const = 0;
};

struct iRayBuildEncoder : public iUnknown {
  niDeclareInterfaceUUID(iRayBuildEncoder,0xcdacd218,0xd73c,0x4713,0x85,0xb1,0xc4,0xdb,0x17,0x01,0x6f,0xe9);

  //! Build a primitives acceleration structure.
  virtual Ptr<iRayPrimitives> __stdcall BuildRayPrimitives(iHString* ahspName, iRayPrimitivesDesc* apPrimitivesDesc) = 0;

  //! Build an instances acceleration structure.
  virtual Ptr<iRayInstances> __stdcall BuildRayInstances(iHString* ahspName, iRayInstancesDesc* apInstancesDesc) = 0;
};

struct iRayCommandEncoder : public iUnknown {
  niDeclareInterfaceUUID(iRayCommandEncoder,0xcdacd218,0xd73c,0x4713,0x85,0xb1,0xc4,0xdb,0x17,0x01,0x6f,0xe9);

  //! Set the instances acceleration structure.
  virtual tBool __stdcall SetRayInstances(iRayInstances* apAS) = 0;

  //! Set the ray gpu pipeline.
  virtual tBool __stdcall SetRayPipeline(iRayPipeline* apAS) = 0;

  //! Set the output image.
  virtual tBool __stdcall SetRayOutputImage(iTexture* apOutputImage) = 0;

  //! Dispatch rays
  virtual tBool __stdcall DispatchRays(tU32 anW, tU32 anH, tU32 anD) = 0;
};

struct iGraphicsDriverRay : public iUnknown
{  niDeclareInterfaceUUID(iGraphicsDriverRay,0xc7f02f1d,0xff59,0x4696,0xa0,0x79,0x29,0x12,0xf9,0x40,0xdf,0x3c);

  //! Create ray tracing pipeline.
  virtual Ptr<iRayPipeline> __stdcall CreateRayPipeline(
    iHString* ahspName,
    iRayFunctionTable* apFunctionTable) = 0;

  //! Create ray tracing function table.
  virtual Ptr<iRayFunctionTable> __stdcall CreateRayFunctionTable() = 0;

  //! Creates a ray primitives acceleration structure description.
  virtual Ptr<iRayPrimitivesDesc> __stdcall CreateRayPrimitivesDesc(iHString* ahspName) = 0;

  //! Creates a ray instances acceleration structure description.
  virtual Ptr<iRayInstancesDesc> __stdcall CreateRayInstancesDesc(iHString* ahspName) = 0;

  //! Creates a single time ray build encoder.
  virtual Ptr<iRayBuildEncoder> __stdcall CreateRayBuildEncoder() = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IRAY_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
