#pragma once
#ifndef __IACCELERATIONSTRUCTURE_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
#define __IACCELERATIONSTRUCTURE_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
#include <niLang/Types.h>
#include <niLang/IDeviceResource.h>
#include <niLang/IHString.h>
#include "GpuEnum.h"

namespace ni {

struct iGpuBuffer;

/** \addtogroup niUI
 * @{
 */

//! Acceleration structure type
enum eAccelerationStructureType {
  //! Contains primitive geometry data such as triangle meshes or procedural shapes.
  //! This is the "template" geometry that can be instanced multiple times.
  //! \remark Equivalent to Bottom Level AS (BLAS) in Vulkan/DX12 terms.
  eAccelerationStructureType_Primitive = 0,
  //! Contains instances of primitive or instance acceleration structures, each with
  //! its own transform and properties. Can be nested to create complex hierarchies.
  //! \remark Equivalent to Top Level AS (TLAS) in Vulkan/DX12 terms.
  eAccelerationStructureType_Instance = 1,
  //! \internal
  eAccelerationStructureType_Last niMaybeUnused = 2,
  //! \internal
  eAccelerationStructureType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Acceleration structure geometry flags
enum eAccelerationGeometryFlags {
  //! No special geometry properties
  eAccelerationGeometryFlags_None = 0,
  //! Geometry is fully opaque, allows skipping any-hit functions
  eAccelerationGeometryFlags_Opaque = niBit(0),
  //! \internal
  eAccelerationGeometryFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};
//! \see ni::eAccelerationGeometryFlags
typedef tU32 tAccelerationGeometryFlags;

//! Acceleration structure instance flags
enum eAccelerationInstanceFlags {
  //! No special instance properties
  eAccelerationInstanceFlags_None = 0,
  //! Disable face culling for this instance
  eAccelerationInstanceFlags_DisableCulling = niBit(0),
  //! Invert face culling for this instance
  eAccelerationInstanceFlags_FlipCulling = niBit(1),
  //! Force instance to be opaque, skipping any-hit functions
  eAccelerationInstanceFlags_ForceOpaque = niBit(2),
  //! Force instance to be non-opaque, always running any-hit functions
  eAccelerationInstanceFlags_ForceNonOpaque = niBit(3),
  //! \internal
  eAccelerationInstanceFlags_ForceDWORD = 0xFFFFFFFF
};
//! \see ni::eAccelerationInstanceFlags
typedef tU32 tAccelerationInstanceFlags;

//! Acceleration structure interface
struct iAccelerationStructure : public iDeviceResource {
  niDeclareInterfaceUUID(iAccelerationStructure,0xb2bfe248,0xc3bc,0xef11,0xb5,0x4b,0x0b,0x0d,0xaf,0xd3,0xbe,0xd1);

  //! Get the acceleration structure type.
  //! {Property}
  virtual eAccelerationStructureType __stdcall GetType() const = 0;

  //! Add non-indexed triangle geometry to primitive acceleration structure.
  virtual tBool __stdcall AddTriangles(
    iGpuBuffer* apVertices,
    tU32 anVertexOffset,
    tU32 anVertexStride,
    tU32 anVertexCount,
    const sMatrixf& aTransform,
    tAccelerationGeometryFlags aFlags,
    tU32 anHitGroup) = 0;

  //! Add indexed triangle geometry to primitive acceleration structure.
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
    tAccelerationGeometryFlags aFlags,
    tU32 anHitGroup) = 0;

  //! Add procedural geometry to primitive acceleration structure using axis-aligned bounding boxes.
  //! \remarks The hit group must include an intersection shader for the procedural geometry.
  //! \remarks AABB format: float[6] = {minX,minY,minZ,maxX,maxY,maxZ}, \see ni::cAABBf
  virtual tBool __stdcall AddProceduralAABBs(
    iGpuBuffer* apAABBs,
    tU32 anAABBOffset,
    tU32 anAABBStride,
    tU32 anAABBCount,
    const sMatrixf& aTransform,
    tAccelerationGeometryFlags aFlags,
    tU32 anHitGroup) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IACCELERATIONSTRUCTURE_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
