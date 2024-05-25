#ifndef __IGEOMETRY_1210155_H__
#define __IGEOMETRY_1210155_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "FVF.h"
#include "IVertexArray.h"
#include "IIndexArray.h"

namespace ni {

struct iDrawOperation;
struct iDrawOperationSet;

/** \addtogroup niUI
 * @{
 */

//! Geometry generation.
enum eGeometryGenerate
{
  //! Generate adjacency informations.
  eGeometryGenerate_Adjacency = niBit(0),
  //! Generate flat normals.
  eGeometryGenerate_FlatNormals = niBit(1),
  //! Generate the normals (overrirde flat normals computation).
  eGeometryGenerate_Normals = niBit(2),
  //! \internal
  eGeometryGenerate_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Geometry types.
enum eGeometryType
{
  //! Polygonal geometry.
  eGeometryType_Polygonal = 0,
  //! Patch geometry.
  eGeometryType_Patch = 1,
  //! Progressive geometry.
  eGeometryType_Progressive = 2,
  //! \internal
  eGeometryType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Geometry creation flags.
enum eGeometryCreateFlags
{
  //! The vertex array usage is dynamic.
  eGeometryCreateFlags_VADynamic = niBit(0),
  //! The vertex array usage is dynamic read write.
  eGeometryCreateFlags_VADynamicReadWrite = niBit(1),
  //! The vertex array usage is static.
  eGeometryCreateFlags_VAStatic = niBit(2),
  //! The vertex array usage is system memory.
  eGeometryCreateFlags_VASystemMemory = niBit(3),
  //! The index array usage is dynamic.
  eGeometryCreateFlags_IADynamic = niBit(4),
  //! The index array usage is dynamic read write.
  eGeometryCreateFlags_IADynamicReadWrite = niBit(5),
  //! The index array usage is static.
  eGeometryCreateFlags_IAStatic = niBit(6),
  //! The index array usage is system memory.
  eGeometryCreateFlags_IASystemMemory = niBit(7),
  //! The vertex array and index array usage is dynamic.
  eGeometryCreateFlags_Dynamic = eGeometryCreateFlags_VADynamic|eGeometryCreateFlags_IADynamic,
  //! The vertex array and index array usage is dynamic read write.
  eGeometryCreateFlags_DynamicReadWrite = eGeometryCreateFlags_VADynamicReadWrite|eGeometryCreateFlags_IADynamicReadWrite,
  //! The vertex array and index array usage is static.
  eGeometryCreateFlags_Static = eGeometryCreateFlags_VAStatic|eGeometryCreateFlags_IAStatic,
  //! The vertex array and index array usage is system memory.
  eGeometryCreateFlags_SystemMemory = eGeometryCreateFlags_VASystemMemory|eGeometryCreateFlags_IASystemMemory,
  //! \internal
  eGeometryCreateFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Geometry creation flags type.
typedef tU32    tGeometryCreateFlags;

//! Geometry optimization flags.
enum eGeometryOptimizeFlags
{
  //! Remove degenerate triangles.
  eGeometryOptimizeFlags_RemoveDegenerates = niBit(0),
  //! Remove unused vertices.
  eGeometryOptimizeFlags_RemoveUnusedVertices = niBit(1),
  //! Remove duplicate vertices.
  eGeometryOptimizeFlags_RemoveDuplicateVertices = niBit(2),
  //! Default.
  eGeometryOptimizeFlags_Default = eGeometryOptimizeFlags_RemoveDegenerates|eGeometryOptimizeFlags_RemoveUnusedVertices,
  //! Default.
  eGeometryOptimizeFlags_All = eGeometryOptimizeFlags_RemoveDegenerates|eGeometryOptimizeFlags_RemoveUnusedVertices|eGeometryOptimizeFlags_RemoveDuplicateVertices,
  //! Force DWORD.
  eGeometryOptimizeFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Optimize the geometry. \see ni::eGeometryOptimizeFlags
typedef tU32 tGeometryOptimizeFlags;

///////////////////////////////////////////////////////////////////////////////////////////////
//! Geometry subset interface.
struct iGeometrySubset : public iUnknown
{
  niDeclareInterfaceUUID(iGeometrySubset,0x0f4ea1e3,0x4c09,0x44b8,0xa0,0x4d,0x95,0x4c,0x72,0xa8,0x60,0x75)
  //! Set the ID of the subset.
  //! {Property}
  virtual void __stdcall SetID(tU32 anID) = 0;
  //! Get the ID of the subset.
  //! {Property}
  virtual tU32 __stdcall GetID() const = 0;
  //! Set the first index of the subset.
  //! {Property}
  virtual void __stdcall SetFirstIndex(tU32 anFirstIndex) = 0;
  //! Get the first index of the subset.
  //! {Property}
  virtual tU32 __stdcall GetFirstIndex() const = 0;
  //! Set the number of indices of the subset.
  //! {Property}
  virtual void __stdcall SetNumIndices(tU32 anNumIndices) = 0;
  //! Get the number of indices of the subset.
  //! {Property}
  virtual tU32 __stdcall GetNumIndices() const = 0;
  //! Set the material ID of the subset.
  //! {Property}
  virtual void __stdcall SetMaterial(tU32 anMaterial) = 0;
  //! Get the material ID of the subset.
  //! {Property}
  virtual tU32 __stdcall GetMaterial() const = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Geometry base interface.
struct iGeometry : public iUnknown
{
  niDeclareInterfaceUUID(iGeometry,0x95cfa251,0x70ed,0x4f5d,0x94,0x22,0x23,0xfa,0xb6,0xc0,0xc7,0xce)

  //! Geometry type.
  //! {Property}
  virtual eGeometryType __stdcall GetType() const = 0;

  //########################################################################################
  //! \name Vertex and index array.
  //########################################################################################
  //! @{

  //! Get the constant vertex array of this geometry.
  //! {Property}
  virtual iVertexArray* __stdcall GetVertexArray() const = 0;
  //! Get the constant index array of this geometry.
  //! {Property}
  virtual iIndexArray* __stdcall GetIndexArray() const = 0;
  //! @}

  //########################################################################################
  //! \name Generation and Extra data
  //########################################################################################
  //! @{

  //! Generate the specified things.
  //! \param  aGenerate indicate what to generate.
  //! \param fEpsilon: specifies that vertices that differ in position by less than
  //!     epsilon should be treated as coincident.
  //! \return eFalse if generation failed, else eTrue.
  virtual tBool __stdcall Generate(eGeometryGenerate aGenerate, tF32 fEpsilon = niEpsilon4) = 0;
  //! Get the number of face contained in the mesh.
  //! {Property}
  virtual tU32 __stdcall GetNumFaces() const = 0;
  //! Set the faces subset's IDs.
  //! \param  apIDs is an array of subset id, it must have a size of GetNumFaces().
  //! \remark this is used by the rendering optimizer to generate subsets.
  virtual void __stdcall SetFacesSubsetsIDs(const tU32* apIDs) = 0;
  //! Get the faces subsets IDs.
  //! \return NULL if the no faces subset's IDs have been set, else return an array
  //!     of GetNumFaces() subset's IDs.
  //! {NoAutomation}
  virtual const tU32* __stdcall GetFacesSubsetsIDs() const = 0;
  //! Get the adjacency array.
  //! \return NULL if the adjacency array hasn't been generated.
  //! \remark The size of the adjacency array is 3*GetNumFaces()
  //! {NoAutomation}
  virtual const tU32* __stdcall GetAdjacencyArray() const = 0;
  //! @}

  //########################################################################################
  //! \name Subsets
  //########################################################################################
  //! @{

  //! Get the number of subsets.
  //! {Property}
  virtual tU32 __stdcall GetNumSubsets() const = 0;
  //! Get the subset at the specified index.
  //! \return NULL if the index is invalid, else return a pointer to the subset at the
  //!     specified index.
  //! {Property}
  virtual iGeometrySubset* __stdcall GetSubset(tU32 aulIdx) const = 0;
  //! Get the index of the subset with the specified ID.
  //! \return eInvalidHandle if a subset with the specified ID doesn't exists, else return
  //!     the index of the first subset found that has the given ID.
  //! {Property}
  virtual tU32 __stdcall GetSubsetIndex(tU32 aulID) const = 0;
  //! Add a subset.
  virtual iGeometrySubset* __stdcall AddSubset(tU32 anID, tU32 anFirstIndex, tU32 anNumIndices, tU32 anMaterial) = 0;
  //! Remove the subset at the given index.
  //! \return eFalse if the subset index is invalid, else remove the subset and return eTrue.
  virtual tBool __stdcall RemoveSubset(tU32 aulIdx) = 0;
  //! @}

  //########################################################################################
  //! \name Clonning.
  //########################################################################################
  //! @{

  //! Create a copy of this geometry.
  //! \param  aFlags are the creation flags of the new geometry.
  //! \param aFVF: if not NULL it's used as the new vertex format to use for the
  //!     new geometry.
  //! \return NULL if the new geometry can't be created, else return an instance of a new
  //!     geometry.
  virtual iGeometry* __stdcall Clone(tGeometryCreateFlags aFlags, tFVF aFVF = 0) = 0;
  //! @}

  //########################################################################################
  //! \name Rendering
  //########################################################################################
  //! @{

  //! Set the specified draw operation to draw the specified subset.
  virtual tBool __stdcall SetDrawOp(iDrawOperation* apDrawOp, tU32 aulSubsetIdx) = 0;
  //! @}

  //########################################################################################
  //! \name Optimization
  //########################################################################################
  //! @{

  //! Optimize the mesh.
  virtual tBool __stdcall Optimize(tGeometryOptimizeFlags aFlags) = 0;
  //! @}
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! Geometry modifier interface.
struct iGeometryModifier : public iUnknown
{
  niDeclareInterfaceUUID(iGeometryModifier,0x26ce1e18,0xd0d7,0x46e6,0xbd,0x9f,0xc2,0x05,0x76,0x29,0xdc,0xa6)

  //! Set the geometry to modify.
  //! \return eFalse if the given geometry can't be modified, else return eTrue.
  //! \remark This may initialize some internal data specific to the modifer and
  //!     to the geometry that's gonna be modified. So this function is considered
  //!     as slow. That's why a modifier should be created for each geometry.
  //! \remark If SetGeometry() is recalled with the same geometry as set, to indicate
  //!     that some data of the geometry have been modified.
  //! {Property}
  virtual tBool __stdcall SetGeometry(iGeometry* pGeometry) = 0;
  //! Get the constant geometry that this modifier modifies.
  //! {Property}
  virtual iGeometry* __stdcall GetGeometry() const = 0;

  //! Return the modified geometry.
  //! \return NULL if the modified geometry has not been generated correctly, else
  //!     a pointer to a valid geometry instance.
  //! {Property}
  virtual iGeometry* __stdcall GetModifiedGeometry() const = 0;

  //! Update the modified geometry.
  //! \remark This function really do the modification. The modification is done only
  //!     if it's necessary.
  //! \remark Should be called after a parameter is changed or the input geometry is changed.
  virtual tBool __stdcall Update() = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IGEOMETRY_1210155_H__
