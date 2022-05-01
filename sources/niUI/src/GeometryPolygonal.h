#ifndef __GEOMETRYPOLYGONAL_33714422_H__
#define __GEOMETRYPOLYGONAL_33714422_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/STLVectorTypes.h>

//////////////////////////////////////////////////////////////////////////////////////////////
//! Geometry subset.
struct sGeometrySubset : public cIUnknownImpl<iGeometrySubset,eIUnknownImplFlags_Default>
{
  //! ID of the subset.
  tU32  ulID;
  //! First index of the subset.
  tU32  ulFirstIndex;
  //! Num indices in the subset.
  tU32  ulNumIndices;
  //! Material associated with this subset.
  tU32  ulMaterial;

  //! Constructor.
  sGeometrySubset()
  {
    ulID = eInvalidHandle;
    ulFirstIndex = 0;
    ulNumIndices = 0;
    ulMaterial = 0;
  }

  //! Return eTrue if the subset is valid and can be used for rendering, else return eFalse.
  //! \remark Optimized geometry creation generates valid subsets.
  tBool __stdcall IsOK() const { return ulNumIndices != 0; }
  //! Comparator operator.
  tBool __stdcall operator == (const sGeometrySubset& aS) const
  {
    return ulID == aS.ulID && ulFirstIndex == aS.ulFirstIndex && ulNumIndices == aS.ulNumIndices && ulMaterial == aS.ulMaterial;
  }

  void __stdcall SetID(tU32 anID) { ulID = anID; }
  tU32 __stdcall GetID() const { return ulID; }
  void __stdcall SetFirstIndex(tU32 anFirstIndex) { ulFirstIndex = anFirstIndex; }
  tU32 __stdcall GetFirstIndex() const { return ulFirstIndex; }
  void __stdcall SetNumIndices(tU32 anNumIndices) { ulNumIndices = anNumIndices; }
  tU32 __stdcall GetNumIndices() const { return ulNumIndices; }
  void __stdcall SetMaterial(tU32 anMaterial) { ulMaterial = anMaterial; }
  tU32 __stdcall GetMaterial() const { return ulMaterial; }
};

typedef Ptr<sGeometrySubset>  tGeometrySubsetPtr;
typedef astl::vector<tGeometrySubsetPtr> tGeometrySubsetVec;

//////////////////////////////////////////////////////////////////////////////////////////////
// cGeometryPolygonal declaration.

//! Polygonal geometry implementation.
class cGeometryPolygonal : public cIUnknownImpl<iGeometry>
{
  niBeginClass(cGeometryPolygonal);

 public:
  //! Face.
  struct sFace
  {
    union {
      struct {
        tU32 ulA, ulB, ulC;
      };
      tU32 ulV[3];
    };
    sPlanef     Plane;
    tBool     bVisible; // used for the silouette generation
    tU32      nSubset;
    tBool     bIsDegenerate;
    sFace() {
      Plane = sPlanef::Zero();
      bVisible = eFalse;
      nSubset = eInvalidHandle;
      bIsDegenerate = eFalse;
    }
  };

  typedef astl::vector<sFace> tFaceVec;
  typedef tFaceVec::iterator      tFaceVecIt;
  typedef tFaceVec::const_iterator  tFaceVecCIt;

 public:
  //! Constructor.
  cGeometryPolygonal(iGraphics* pGraphics, tU32 aulNumVertices, tU32 aulNumIndices, tGeometryCreateFlags aFlags, const cFVFDescription& aFVFDesc);
  //! Constructor.
  cGeometryPolygonal(iGraphics* pGraphics, iVertexArray* apVA, iIndexArray* apIA);
  //! Destructor.
  ~cGeometryPolygonal();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  void __stdcall Invalidate();

  eGeometryType __stdcall GetType() const { return eGeometryType_Polygonal; }

  //// iGeometry ///////////////////////
  //! Get the constant vertex array of this geometry.
  iVertexArray* __stdcall GetVertexArray() const;
  //! Get the constant index array of this geometry.
  iIndexArray* __stdcall GetIndexArray() const;

  //! Set duplication indices informations.
  void __stdcall SetDuplicationIndices(tU32 ulOriginalVertices, tU32 ulIndices, const tU32* pIndices);
  //! Get the number of duplication indices.
  tU32 __stdcall GetNumDuplicationIndices() const;
  //! Get the number of indices before duplication.
  tU32 __stdcall GetNumOriginalVertices() const;
  //! Get the duplication indices array.
  const tU32* __stdcall GetDuplicationIndices() const;

  //! Generate the specified things.
  tBool __stdcall Generate(eGeometryGenerate aGenerate, tF32 fEpsilon = niEpsilon4);
  //! Get the number of face contained in the mesh.
  tU32 __stdcall GetNumFaces() const;
  //! Set the faces subset's IDs.
  void __stdcall SetFacesSubsetsIDs(const tU32* apIDs);
  //! Get the faces subsets IDs.
  const tU32* __stdcall GetFacesSubsetsIDs() const;
  //! Get the adjacency array.
  const tU32* __stdcall GetAdjacencyArray() const;

  //! Get the number of subsets.
  tU32 __stdcall GetNumSubsets() const;
  //! Get the subset at the specified index.
  iGeometrySubset* __stdcall GetSubset(tU32 aulIdx) const;
  //! Get the index of the subset with the specified ID.
  tU32 __stdcall GetSubsetIndex(tU32 aulID) const;
  //! Add a subset.
  iGeometrySubset* __stdcall AddSubset(tU32 anID, tU32 anFirstIndex, tU32 anNumIndices, tU32 anMaterial);
  //! Remove the subset at the given index.
  tBool __stdcall RemoveSubset(tU32 aulIdx);

  //! Create a copy of this geometry.
  iGeometry* __stdcall Clone(tGeometryCreateFlags aFlags, tFVF aFVF);

  //! Set the complete geometry in a draw operation set.
  tBool __stdcall SetDrawOpSet(iDrawOperationSet* apDrawOpSet);
  //! Set the specified draw operation to draw the specified subset.
  tBool __stdcall SetDrawOp(iDrawOperation* apDrawOp, tU32 aulSubsetIdx);

  //! Optimize the mesh.
  tBool __stdcall Optimize(tGeometryOptimizeFlags aFlags);
  //// iGeometry ///////////////////////

  void _GenerateAdjacencyArrays(cFVFStream& astmVerts, tFaceVec& avFaces, tF32 afEpsilon);
  void _GenerateNormals(cFVFStream& astmVerts, tFaceVec& avFaces, tBool bFlat);
  static tBool _GetVAUsage(tGeometryCreateFlags aFlags, eArrayUsage* aVAUsage);
  static tBool _GetIAUsage(tGeometryCreateFlags aFlags, eArrayUsage* aIAUsage);

  tU32 _OptimizeRemoveDegenerates(tGeometryOptimizeFlags aFlags, tFaceVec& avFaces);
  tU32 _OptimizeUnusedVertices(tGeometryOptimizeFlags aFlags, tFaceVec& avFaces);
  tU32 _OptimizeDuplicateVertices(tGeometryOptimizeFlags aFlags, tFaceVec& avFaces);
  tBool _OptimizeRebuildIndexArray(tGeometryOptimizeFlags aFlags, tFaceVec& avFaces);

  tBool _BuildFaces(tFaceVec& avFaces);

 private:
  //! Sanity status.
  tBool mbOK;

  Ptr<iGraphics>    mptrGraphics;
  Ptr<iVertexArray>   mptrVA;
  Ptr<iIndexArray>    mptrIA;
  tU32Vec       mvFacesAdjacency;
  tU32Vec       mvFacesSubsetsIDs;
  tU32        mulNumOriginalVertices;
  tU32Vec       mvDuplicationIndices;
  tGeometrySubsetVec  mvSubsets;

  niEndClass(cGeometryPolygonal);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __GEOMETRYPOLYGONAL_33714422_H__
