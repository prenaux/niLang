#pragma once
#ifndef __IACCELERATIONSTRUCTURE_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
#define __IACCELERATIONSTRUCTURE_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
#include <niLang/Types.h>
#include <niLang/IDeviceResource.h>
#include <niLang/IHString.h>

namespace ni {

struct iVertexArray;
struct iIndexArray;

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

  //! Get the structure type
  virtual eAccelerationStructureType __stdcall GetType() const = 0;

  //! Add triangle geometry to primitive acceleration structure.
  //! \param apVertices Vertex buffer containing geometry vertices.
  //! \param apIndices Index buffer containing triangle indices.
  //! \param aTransform Transform matrix applied to vertices.
  //! \param aFlags Geometry flags affecting ray traversal behavior. see ni::eAccelerationGeometryFlags.
  //! \param anHitGroupIndex Index of hit group to use for this geometry.
  //! \return eTrue if geometry was added successfully.
  virtual tBool __stdcall AddTriangles(
    iVertexArray* apVertices,
    iIndexArray* apIndices,
    const sMatrixf& aTransform,
    tAccelerationGeometryFlags aFlags = eAccelerationGeometryFlags_None,
    tU32 anHitGroupIndex = 0) = 0;

  //! Add instance to instance acceleration structure.
  //! \param apBLAS Base acceleration structure to instance.
  //! \param aTransform Instance transform matrix.
  //! \param anInstanceID Custom ID accessible in hit shaders.
  //! \param aFlags Instance flags affecting traversal behavior. \see ni::eAccelerationInstanceFlags
  //! \param anInstanceMask Visibility mask for ray filtering.
  //! \param anHitGroupOffset Index of the hit group. The return value from iRayGpuFunctionTable::AddHitGroup().
  //! \return eTrue if instance was added successfully.
  //! \remark Hit group offset selects which pipeline hit group to use for this instance.
  //! \remark The instance mask allows filtering during ray traversal.
  virtual tBool __stdcall AddInstance(
    iAccelerationStructure* apBLAS,
    const sMatrixf& aTransform,
    tU32 anInstanceID,
    tAccelerationInstanceFlags aFlags = eAccelerationInstanceFlags_None,
    tU8 anInstanceMask = 0xFF,
    tU32 anHitGroupOffset = 0) = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __IACCELERATIONSTRUCTURE_H_168A30FF_C2BC_EF11_B76A_7FBF5918B21E__
