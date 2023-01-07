#ifndef __POLYGON_40138691_H__
#define __POLYGON_40138691_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "AABB.h"
#include <niLang/Math/MathUtils.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
class cPolygon : public cMemImpl
{
 public:
  //! Constructor.
  inline cPolygon(tFVF aFVF = eFVF_Position);
  //! Copy constructor.
  inline cPolygon(const cPolygon& aRight);
  //! Destructor.
  inline ~cPolygon();

  //! Zero class members.
  inline void __stdcall ZeroMembers();

  //! Return eTrue if the polygon is ok.
  //! \remark check if the FVF has a position and that the vertex size is greater than
  //!     12 and smaller than niMaxVertexSize.
  inline tBool __stdcall IsOK() const;

  //########################################################################################
  //! \name Vertices
  //########################################################################################
  //! @{

  //! Set the FVF.
  //! \remark This remove all vertices in the polygon.
  inline void __stdcall SetFVF(tFVF aFVF);
  //! Get the FVF.
  inline tFVF __stdcall GetFVF() const;
  //! Check if the passed polygon's FVF is the same as this polygon's FVF.
  inline tBool __stdcall IsSameFVF(const cPolygon* aPoly) const;
  //! Get the vertex size.
  inline tU16 __stdcall GetVertexSize() const;
  //! Remove all vertices of the polygon.
  inline void __stdcall ClearVertices();
  //! Reserve memory for the specified number of vertices.
  inline void __stdcall ReserveVertices(tU32 aunNumVert);
  //! Get the number of vertices in the polygon.
  inline tU16 __stdcall GetNumVertices() const;
  //! Set the number of vertices
  inline void __stdcall SetNumVertices(tU16 anNum);
  //! Create a vertices file.
  inline iFile* __stdcall CreateVerticesFile() const;
  //! Add vertices to the polygon.
  inline void __stdcall AddVertices(const tPtr apVerts, tU16 aunNumVert = 1);
  //! Add vertices to the polygon.
  inline void __stdcall AddVertices(const cFVFDescription& aFVF, const tPtr apVerts, tU16 aunNumVert = 1);
  //! Remove a vertex of the polygon.
  //! \param anVert is the index of the vertex to remove, if eInvalidHandle remove the last vertex.
  inline void __stdcall RemoveVertex(tU32 anVert = eInvalidHandle);
  //! Swap the content of two vertex.
  inline void __stdcall SwapVertices(tU32 i, tU32 j);
  //! Get the vertex at the given index.
  inline tPtr __stdcall GetVertex(tU32 nVert) const;
  //! Get the first vertex.
  inline tPtr __stdcall GetFirstVertex() const;
  //! Get the next vertex.
  inline tPtr __stdcall GetNextVertex(tPtr apVert) const;
  //! Check if this is the end vertex.
  //! \remark The end vertex don't contain valid data and reading it could crash the application.
  inline tBool __stdcall IsEndVertex(tPtr apVert) const;
  //! Get the position at the given index.
  inline sVec3f* __stdcall GetPosition(tU32 nVert) const;
  //! Get the first position.
  inline sVec3f* __stdcall GetFirstPosition() const;
  //! Get the next position.
  inline sVec3f* __stdcall GetNextPosition(sVec3f* apVert) const;
  //! Check if this is the end position.
  //! \remark The end position don't contain valid data and reading it could crash the application.
  inline tBool __stdcall IsEndPosition(sVec3f* apVert) const;
  //! Return eTrue if their's no vertex in the polygon.
  inline tBool __stdcall IsEmpty() const;
  //! Return eTrue if the polygon is valid, aka has at least 3 vertex.
  inline tBool __stdcall IsValidPolygon() const;
  //! return eTrue if a vertex with the given position is already in the list
  inline tBool __stdcall Exists(const sVec3f& pos, const tF32 epsilon);
  inline tBool __stdcall Exists(const sVec3f& pos) { return Exists(pos,niEpsilon5); }
  //! @}

  //########################################################################################
  //! \name Misc infos
  //########################################################################################
  //! @{

  //! Get the polygon center of mass.
  inline sVec3f __stdcall GetCOM() const;
  //! Get the polygon area.
  inline tF32 __stdcall GetArea() const;
  //! @}

  //########################################################################################
  //! \name Plane
  //########################################################################################
  //! @{

  //! Compute the plane.
  inline tBool __stdcall ComputePlane();
  //! Compute the plane using a given normal from a polygon which is coplanar.
  inline tBool __stdcall ComputePlaneNormal(const sVec3f& vNormal);
  //! Set the value of the plane with the given plane
  inline void __stdcall SetPlane(const sPlanef& plane);
  //! Get the polygon's plane
  inline sPlanef __stdcall GetPlane() const;
  //! @}

  //########################################################################################
  //! \name Intersection/Classification
  //########################################################################################
  //! @{

  //! Classify along a plane.
  inline eClassify __stdcall Classify(const sPlanef& Plane, const tF32 epsilon) const;
  //! Classify along a plane.
  inline eClassify __stdcall Classify(const sPlanef& Plane) const { return Classify(Plane,niEpsilon5); }
  //! Return eTrue if the specified polygon is the same as this polygon
  inline tBool __stdcall IsEqual(const cPolygon* b) const;
  //! Return eTrue if the specified point is inside the polygon.
  inline tBool __stdcall Intersect(const sVec3f& pt, const tF32 fEpsilon = niEpsilon5) const;
  inline tBool __stdcall IntersectPoint(const sVec3f& pt) const { return Intersect(pt,niEpsilon5); }
  //! Return eTrue if the specified AABB intersect the polygon.
  inline tBool __stdcall Intersect(const cAABBf& aAABB, const tF32 fEpsilon = niEpsilon5) const;
  inline tBool __stdcall IntersectAABB(const sVec3f& avMin, const sVec3f& avMax) const { return Intersect(cAABBf(avMin,avMax),niEpsilon5); }
  //! @}

  //########################################################################################
  //! \name Splitting
  //########################################################################################
  //! @{

  //! Split along a plane.
  inline void __stdcall Split(const sPlanef& Plane, cPolygon* pFront, cPolygon* pBack, tBool abKeepCoplanar, tF32 epsilon) const;
  inline void __stdcall Split(const sPlanef& Plane, cPolygon* pFront, cPolygon* pBack, tBool abKeepCoplanar) const {
    Split(Plane,static_cast<cPolygon*>(pFront),static_cast<cPolygon*>(pBack),abKeepCoplanar,niEpsilon5);
  }
  //! Clip the polygon against the given set of planes. The planes are supposted to form a convex hull.
  //! \return eTrue if the resulting fragement is valid, else eFalse.
  inline tBool __stdcall Clip(const sPlanef* apPlanes, tU32 aulNumPlanes, cPolygon& aResult, tBool abKeepBack, tBool abKeepCoplanar, tF32 epsilon) const;
  inline tBool __stdcall Clip(const tVec4fCVec* avPlanes, cPolygon* apResult, tBool abKeepBack, tBool abKeepCoplanar) const {
    if (!niIsOK(avPlanes))
      return eFalse;
    return Clip(avPlanes->_Data(),avPlanes->size(),
                *static_cast<cPolygon*>(apResult),abKeepBack,abKeepCoplanar,niEpsilon5);
  }
  //! @}

  //########################################################################################
  //! \name Misc operations
  //########################################################################################
  //! @{

  //! Generate vertices for the given plane the size is fWidth.
  inline tBool __stdcall GenerateBaseForPlane(const sPlanef& Plane, const tF32 fWidth = 1024.0f, tBool bCCW = eFalse);
  //! Generate vertices for the given plane the size is defined by the specified AABB.
  inline tBool __stdcall GenerateBaseForPlaneAABB(const sPlanef& Plane, const sVec3f& avMin, const sVec3f& avMax, tBool bCCW = eFalse);
  //! Sort the vertices in clockwise order.
  //! - require a correct normal in the plane, so it must be set with SetPlane() before.
  //! - recompute the polygon's plane.
  inline void __stdcall SortVerts();
  //! Reverse the polygon facing.
  inline void __stdcall Reverse();
  //! Return the closest point to the specified point that lay on an edge of this polygon.
  inline sVec3f& __stdcall ClosestPointOnPerimeter(sVec3f& Out, const sVec3f& pt,
                                                      sVec3f* pEA = NULL, sVec3f* pEB = NULL,
                                                      tBool* pbEdgeFlag = NULL) const;
  //! Check if the polygon is degenerate.
  inline tBool __stdcall IsDegenerate() const;
  //! @}

  //########################################################################################
  //! \name Material and Flags
  //########################################################################################
  //! @{

  //! Set the polygon's material.
  inline void __stdcall SetMaterial(tU32 aunMatID);
  //! Get the polygon's material.
  inline tU32 __stdcall GetMaterial() const;
  //! Set the polygon's flags.
  inline void __stdcall SetFlags(tU32 anFlags);
  //! Get the polygon's flags.
  inline tU32 __stdcall GetFlags() const;
  //! @}

  //########################################################################################
  //! \name Creation & Copy
  //########################################################################################
  //! @{

  //! Copy only the vertices of the specified polygon.
  //! \remark The FVF of the passed polygon can be different of this polygon.
  inline void __stdcall CopyVertices(const cPolygon* apPoly);
  //! Copy the specified polygon in this polygon.
  inline void __stdcall Copy(const cPolygon* apPoly);
  //! Create a polygon with the same FVF as this polygon.
  inline cPolygon* __stdcall Create() const;
  //! Create a copy of this polygon.
  inline cPolygon* __stdcall Clone() const;
  //! @}

  inline  tBool __stdcall SetVertexPosition(ni::tU32 anIndex, const sVec3f& pos) {
    if (anIndex >= GetNumVertices()) return eFalse;
    *GetPosition(anIndex) = pos;
    return eTrue;
  }
  inline sVec3f __stdcall GetVertexPosition(ni::tU32 anIndex) const {
    if (anIndex >= GetNumVertices()) return sVec3f::Zero();
    return *GetPosition(anIndex);
  }

  inline tBool operator == (const cPolygon& aRight) const
  {
    return
        mvVertices == aRight.mvVertices &&
        munNumVertices == aRight.munNumVertices &&
        munMaterial == aRight.munMaterial &&
        mFVF == aRight.mFVF &&
        munVertexSize == aRight.munVertexSize &&
        mPlane == aRight.mPlane;
  }

 private:
  //! Vertices memory.
  astl::vector<tU8> mvVertices;
  //! Num vertices.
  tU16  munNumVertices;
  //! Material ID.
  tU32  munMaterial;
  //! Flags.
  tU32  mnFlags;
  //! FVF.
  tFVF  mFVF;
  //! Vertex size.
  tU16  munVertexSize;
  //! Polygon's plane.
  sPlanef mPlane;

 public:
  //! Get the object type ID.
  inline const achar* __stdcall GetSerializeObjectTypeID() const { return NULL; }

  //! Serialize the object.
  inline tSize __stdcall Serialize(iFile* apFile, eSerializeMode aMode)
  {
    switch(aMode)
    {
      case eSerializeMode_Read:
      case eSerializeMode_ReadRaw:
        {
          tI64 nPos = apFile->Tell();
          mFVF = (eFVF)apFile->ReadLE32();
          munVertexSize = apFile->ReadLE16();
          munMaterial = apFile->ReadLE32();
          mnFlags = apFile->ReadLE32();
          munNumVertices = apFile->ReadLE16();
          apFile->ReadF32Array(mPlane.ptr(),4);
          mvVertices.resize(GetNumVertices()*GetVertexSize());
          apFile->ReadRaw((void*)GetFirstVertex(), GetNumVertices()*GetVertexSize());
          return (tSize)(apFile->Tell()-nPos);
        }
      case eSerializeMode_Write:
      case eSerializeMode_WriteRaw:
        {
          tSize nSize = 0;
          nSize += apFile->WriteLE32(mFVF);
          nSize += apFile->WriteLE16(munVertexSize);
          nSize += apFile->WriteLE32(munMaterial);
          nSize += apFile->WriteLE32(mnFlags);
          nSize += apFile->WriteLE16(munNumVertices);
          nSize += apFile->WriteF32Array(mPlane.ptr(),4);
          nSize += apFile->WriteRaw((void*)GetFirstVertex(), GetNumVertices()*GetVertexSize());
          return nSize;
        }
    }

    niAssertUnreachable("Unreachable code.");
    return eInvalidHandle;
  }
};

///////////////////////////////////////////////
//! Constructor.
inline cPolygon::cPolygon(tFVF aFVF)
{
  ZeroMembers();
  SetFVF(aFVF);
}

///////////////////////////////////////////////
//! Copy constructor.
inline cPolygon::cPolygon(const cPolygon& aRight)
{
  Copy(&aRight);
}

///////////////////////////////////////////////
//! Destructor.
inline cPolygon::~cPolygon()
{
  mvVertices.clear();
}

///////////////////////////////////////////////
//! Zero class members.
inline void cPolygon::ZeroMembers()
{
  munNumVertices = 0;
  munMaterial = 0;
  mnFlags = 0;
  mFVF = eFVF(0);
  munVertexSize = 0;
  mPlane.SetNormal(sVec3f::YAxis());
}

///////////////////////////////////////////////
//! Return eTrue if the polygon is ok.
//! \remark check if the FVF has a position and that the vertex size is greater than 12 and smaller than niMaxVertexSize.
inline tBool __stdcall cPolygon::IsOK() const
{
  return niFlagTest(mFVF,eFVF_Position) && munVertexSize && munVertexSize < kMaxVertexSize;
}

///////////////////////////////////////////////
//! Set the FVF.
inline void __stdcall cPolygon::SetFVF(tFVF aFVF)
{
  ClearVertices();
  mFVF = aFVF;
  munVertexSize = FVFGetStride(mFVF);
}

///////////////////////////////////////////////
//! Get the FVF.
inline tFVF __stdcall cPolygon::GetFVF() const
{
  return mFVF;
}

///////////////////////////////////////////////
//! Check if the passed polygon's FVF is the same as this polygon's FVF.
inline tBool __stdcall cPolygon::IsSameFVF(const cPolygon* aPoly) const
{
  return mFVF == aPoly->GetFVF() && munVertexSize == aPoly->GetVertexSize();
}

///////////////////////////////////////////////
//! Get the vertex size.
inline tU16 __stdcall cPolygon::GetVertexSize() const
{
  return munVertexSize;
}

///////////////////////////////////////////////
//! Remove all vertices of the polygon.
inline void __stdcall cPolygon::ClearVertices()
{
  munNumVertices = 0;
  mvVertices.clear();
}

///////////////////////////////////////////////
//! Reserve memory for the specified number of vertices.
inline void __stdcall cPolygon::ReserveVertices(tU32 aunNumVert)
{
  mvVertices.reserve((mvVertices.size()+aunNumVert)*munVertexSize);
}

///////////////////////////////////////////////
//! Add vertices to the polygon.
inline void __stdcall cPolygon::AddVertices(const tPtr apVerts, tU16 aunNumVert)
{
  niAssert(aunNumVert);

  mvVertices.resize((GetNumVertices()+aunNumVert)*munVertexSize);

  if (apVerts)
  {
    memcpy((void*)&mvVertices[GetNumVertices()*munVertexSize], apVerts, aunNumVert*munVertexSize);
  }

  munNumVertices += aunNumVert;
}

///////////////////////////////////////////////
//! Add vertices to the polygon.
inline void __stdcall cPolygon::AddVertices(const cFVFDescription& aFVF, const tPtr apVerts, tU16 aunNumVert)
{
  niAssert(apVerts != NULL);
  niAssert(aunNumVert);

  mvVertices.resize((GetNumVertices()+aunNumVert)*munVertexSize);

  if (mFVF == aFVF.GetFVF() && GetVertexSize() == aFVF.GetStride())
  {
    memcpy((void*)&mvVertices[GetNumVertices()*munVertexSize], apVerts, aunNumVert*munVertexSize);
  }
  else
  {
    FVFCopy(&mvVertices[GetNumVertices()*munVertexSize], cFVFDescription(mFVF), apVerts, aFVF, aunNumVert);
  }

  munNumVertices += aunNumVert;
}

///////////////////////////////////////////////
//! Remove a vertex of the polygon.
//! \param anVert is the index of the vertex to remove, if eInvalidHandle remove the last vertex.
inline void __stdcall cPolygon::RemoveVertex(tU32 anVert)
{
  niAssert(anVert == eInvalidHandle || anVert < GetNumVertices());

  tU32 unVertexOffset = GetVertexSize()*((anVert == eInvalidHandle)?munNumVertices-1:anVert);
  mvVertices.erase(
    mvVertices.begin()+unVertexOffset,
    mvVertices.begin()+unVertexOffset+munVertexSize);
  --munNumVertices;
}

///////////////////////////////////////////////
//! Swap the content of two vertex.
inline void __stdcall cPolygon::SwapVertices(tU32 i, tU32 j)
{
  niAssert(i < GetNumVertices());
  niAssert(j < GetNumVertices());

  static tU8 tempV[kMaxVertexSize];
  tPtr vertI = GetVertex(i);
  tPtr vertJ = GetVertex(j);

  memcpy((void*)tempV, (void*)vertI, munVertexSize);
  memcpy((void*)vertI, (void*)vertJ, munVertexSize);
  memcpy((void*)vertJ, (void*)tempV, munVertexSize);
}

///////////////////////////////////////////////
//! Get the number of vertices in the polygon.
inline tU16 __stdcall cPolygon::GetNumVertices() const
{
  return munNumVertices;
}

///////////////////////////////////////////////
//! Get the vertex at the given index.
inline tPtr __stdcall cPolygon::GetVertex(tU32 nVert) const
{
  niAssert(nVert < GetNumVertices());
  return GetFirstVertex()+(nVert*GetVertexSize());
}

///////////////////////////////////////////////
//! Set the number of vertices
inline void __stdcall cPolygon::SetNumVertices(tU16 anNum)
{
  if (munNumVertices == anNum) return;
  munNumVertices = anNum;
  mvVertices.resize(munNumVertices*munVertexSize);
}

///////////////////////////////////////////////
//! Create a vertices file.
inline iFile* __stdcall cPolygon::CreateVerticesFile() const
{
  return ni::CreateFileMemory((tPtr)&mvVertices[0],mvVertices.size(),eFalse,NULL);
}

///////////////////////////////////////////////
//! Get the first vertex.
inline tPtr __stdcall cPolygon::GetFirstVertex() const
{
  return tPtr(mvVertices.data());
}

///////////////////////////////////////////////
//! Get the next vertex.
inline tPtr __stdcall cPolygon::GetNextVertex(tPtr apVert) const
{
  return (tPtr)(apVert+munVertexSize);
}

///////////////////////////////////////////////
//! Check if this is the end vertex.
inline tBool __stdcall cPolygon::IsEndVertex(tPtr apVert) const
{
  return tPtr(apVert) == tPtr(mvVertices.data()+mvVertices.size());
}

///////////////////////////////////////////////
//! Get the position at the given index.
inline sVec3f* __stdcall cPolygon::GetPosition(tU32 nVert) const
{
  niAssert(nVert < GetNumVertices());
  return reinterpret_cast<sVec3f*>(GetFirstVertex()+(nVert*GetVertexSize()));
}

///////////////////////////////////////////////
//! Get the first vertex.
inline sVec3f* __stdcall cPolygon::GetFirstPosition() const
{
  return reinterpret_cast<sVec3f*>(tPtr(mvVertices.data()));
}

///////////////////////////////////////////////
//! Get the next vertex.
inline sVec3f* __stdcall cPolygon::GetNextPosition(sVec3f* apVert) const
{
  return reinterpret_cast<sVec3f*>(tPtr(apVert)+munVertexSize);
}

///////////////////////////////////////////////
//! Check if this is the end vertex.
inline tBool __stdcall cPolygon::IsEndPosition(sVec3f* apVert) const
{
  return tPtr(apVert) == tPtr(mvVertices.data()+mvVertices.size());
}

///////////////////////////////////////////////
//! Return eTrue if their's no vertex in the polygon.
inline tBool __stdcall cPolygon::IsEmpty() const
{
  return mvVertices.empty();
}

///////////////////////////////////////////////
//! Return eTrue if the polygon is valid, aka has at least 3 vertex.
inline tBool __stdcall cPolygon::IsValidPolygon() const
{
  return munNumVertices >= 3;
}

///////////////////////////////////////////////
// return eTrue if a vertex with the given position is already in the list
inline tBool __stdcall cPolygon::Exists(const sVec3f& aPos, const tF32 epsilon)
{
  for (tPtr itV = GetFirstVertex(); !IsEndVertex(itV); itV = GetNextVertex(itV))
  {
    sVec3f* pos = reinterpret_cast<sVec3f*>(itV);
    if (ni::Abs(pos->x-aPos.x) <= epsilon &&
        ni::Abs(pos->y-aPos.y) <= epsilon &&
        ni::Abs(pos->z-aPos.z) <= epsilon)
      return eTrue;
  }

  return eFalse;
}

///////////////////////////////////////////////
//! Get the polygon center of mass.
inline sVec3f cPolygon::GetCOM() const
{
  niAssert(IsValidPolygon());
  sVec3f vCOM = { 0, 0, 0 };
  for (sVec3f* itV = GetFirstPosition(); !IsEndPosition(itV); itV = GetNextPosition(itV))
  {
    vCOM.x += itV->x;
    vCOM.y += itV->y;
    vCOM.z += itV->z;
  }
  vCOM /= tF32(GetNumVertices());
  return vCOM;
}

///////////////////////////////////////////////
//! Get the polygon area.
inline tF32 cPolygon::GetArea() const
{
  niAssert(IsValidPolygon());

  tF32 xyArea = 0.0f, yzArea = 0.0f, zxArea = 0.0f;
  sVec3f *p0 = reinterpret_cast<sVec3f*>(GetVertex(GetNumVertices()-1));
  sVec3f *p1;

  for (sVec3f* itV = GetFirstPosition(); !IsEndPosition(itV); itV = GetNextPosition(itV))
  {
    p1 = itV;
    xyArea += (p0->y + p1->y) * (p1->x - p0->x) / 2.0f;
    yzArea += (p0->z + p1->z) * (p1->y - p0->y) / 2.0f;
    zxArea += (p0->x + p1->x) * (p1->z - p0->z) / 2.0f;
    p0 = p1;
  }

  return sqrtf(xyArea * xyArea + yzArea * yzArea + zxArea * zxArea);
}

///////////////////////////////////////////////
//! Compute the plane.
inline tBool __stdcall cPolygon::ComputePlane()
{
  niAssert(IsValidPolygon());

  sVec3f *vi, *vj;

  sVec3f normal = {0,0,0};
  for (tU32 i = 0, j; i < GetNumVertices(); ++i) {
    j = (i+1)%GetNumVertices();
    vi = GetPosition(i);
    vj = GetPosition(j);
    normal.x += tF32(vi->y - vj->y) * tF32(vi->z + vj->z);
    normal.y += tF32(vi->z - vj->z) * tF32(vi->x + vj->x);
    normal.z += tF32(vi->x - vj->x) * tF32(vi->y + vj->y);
  }
  VecNormalize(normal);

  mPlane.SetDist(-PlaneDotNormal(mPlane, GetCOM()));
  mPlane.SetNormal(normal);
  return eTrue;
}

///////////////////////////////////////////////
//! Compute the plane using a given normal from a polygon which is coplanar.
inline tBool __stdcall cPolygon::ComputePlaneNormal(const sVec3f& vNormal)
{
  niAssert(IsValidPolygon());
  PlaneFromPointNormal(mPlane, GetCOM(), vNormal);
  return eTrue;
}

///////////////////////////////////////////////
//! Set the value of the plane with the given plane
inline void __stdcall cPolygon::SetPlane(const sPlanef& plane)
{
  mPlane = plane;
}

///////////////////////////////////////////////
//! Get the polygon's plane
inline sPlanef __stdcall cPolygon::GetPlane() const
{
  return mPlane;
}

///////////////////////////////////////////////
//! Classify along a plane.
inline eClassify __stdcall cPolygon::Classify(const sPlanef& aPlane, const tF32 epsilon) const
{
  tBool bFront = eFalse, bBack = eFalse;
  for (sVec3f* itV = GetFirstPosition(); !IsEndPosition(itV); itV = GetNextPosition(itV))
  {
    tF32 fDist = PlaneDotCoord(aPlane, *itV);
    if (fDist > epsilon)
    {
      if (bBack)
        return eClassify_Spanned;
      bFront = eTrue;
    }
    else if (fDist < -epsilon)
    {
      if (bFront)
        return eClassify_Spanned;
      bBack = eTrue;
    }
  }

  if (bFront)
    return eClassify_Front;
  else if (bBack)
    return eClassify_Back;

  return eClassify_Coplanar;

  //  tU16 InFront = 0, Behind = 0, Coplanar = 0;
  //  for (sVec3f* it = GetFirstPosition(); !IsEndPosition(it); it = GetNextPosition(it))
  //  {
  //    tF32 fDist = PlaneDotCoord(aPlane, *it);
  //    if (fDist < -epsilon)
  //      ++Behind;
  //    else if (fDist > epsilon)
  //      ++InFront;
  //    else
  //    {
  //      ++InFront;
  //      ++Behind;
  //      ++Coplanar;
  //    }
  //  }
  //
  //  if (Coplanar == munNumVertices) return eClassify_Coplanar;
  //  if (InFront == munNumVertices) return eClassify_Front;
  //  if (Behind == munNumVertices) return eClassify_Back;
  //  return eClassify_Spanned;
}

///////////////////////////////////////////////
//! Return eTrue if the specified polygon is the same as this polygon
inline tBool __stdcall cPolygon::IsEqual(const cPolygon* apB) const
{
  const cPolygon& b = (const cPolygon&)*apB;
  if (GetNumVertices() != b.GetNumVertices())
    return eFalse;

  if (GetPlane() != b.GetPlane())
    return eFalse;

  for (sVec3f* pA = GetFirstPosition(), *pB = b.GetFirstPosition();
       !IsEndPosition(pA);
       pA = GetNextPosition(pA), pB = GetNextPosition(pB))
  {
    if (*pA != *pB)
      return eFalse;
  }

  return eTrue;
}

///////////////////////////////////////////////
//! Return eTrue if the specified point is inside the polygon.
inline tBool __stdcall cPolygon::Intersect(const sVec3f& pt, const tF32 fEpsilon) const
{
  niAssert(IsValidPolygon());

  int pos = 0;
  int neg = 0;

  const sVec3f planeNormal = mPlane.GetNormal();
  sVec3f* j = GetPosition(1);

  for (sVec3f* i = GetFirstPosition(); !IsEndPosition(i); i = GetNextPosition(i), j = GetNextPosition(j))
  {
    if (IsEndPosition(j)) j = GetFirstPosition();

    // Generate a normal for this edge
    sVec3f n;
    VecCross<tF32>(n, (*j-*i), planeNormal);

    // Which side of this edge-plane is the point?
    tF32 fHalfPlane = VecDot(pt,n) - VecDot(*i,n);

    // Keep track of positives & negatives (but not zeros -- which means it's on the edge)
    if (fHalfPlane > fEpsilon)   pos++;
    else if (fHalfPlane < -fEpsilon)  neg++;
  }

  // If they're ALL positive, or ALL negative, then it's inside
  if (!pos || !neg) return eTrue;

  // Must not be inside, because some were pos and some were neg
  return eFalse;
}

///////////////////////////////////////////////
//! Return eTrue if the specified AABB intersect the polygon.
inline tBool __stdcall cPolygon::Intersect(const cAABBf& aAABB, const tF32 fEpsilon) const
{
  cAABBf polyAABB;
  {
    niLoop(i,this->GetNumVertices()) {
      polyAABB.SetPoint(this->GetVertexPosition(i));
    }
  }
  if (!polyAABB.Intersect(aAABB,fEpsilon))
    return eFalse;

  sPlanef aabbPlanes[6];
  aAABB.GetFacesPlanes(aabbPlanes,eTrue);

  for (tU32 i = 0; i < 6; ++i) {
    if (Classify(aabbPlanes[i], fEpsilon) == eClassify_Front)
      return eFalse;
  }

  return eTrue;
}

///////////////////////////////////////////////
//! Split along a plane.
inline void __stdcall cPolygon::Split(const sPlanef& Plane, cPolygon* pFront, cPolygon* pBack, tBool abKeepCoplanar, tF32 epsilon) const
{
  niAssert(!pFront || (mFVF == pFront->GetFVF() && munVertexSize == pFront->GetVertexSize()));
  niAssert(!pBack  || (mFVF == pBack->GetFVF()  && munVertexSize == pBack->GetVertexSize()));

  if (!pFront && !pBack) return;

  sVec3f *itA, *itB;

  astl::vector<eClassify>   vCl;
  vCl.reserve(GetNumVertices());

  {
    tU16 usInFront = 0, usBack = 0, usCoplanar = 0;
    for (itA = GetFirstPosition(); !IsEndPosition(itA); itA = GetNextPosition(itA))
    {
      vCl.push_back(ClassifyPoint(Plane,*itA,epsilon));
      if (vCl.back() == eClassify_Front)    ++usInFront;
      else if (vCl.back() == eClassify_Back)  ++usBack;
      else                  ++usCoplanar;
    }
    if (usInFront == GetNumVertices())
    {
      if (pFront)
        *pFront = *this;
      return;
    }
    else if (usBack == GetNumVertices())
    {
      if (pBack)
        *pBack = *this;
      return;
    }
    else if (usCoplanar == GetNumVertices())
    {
      if (abKeepCoplanar)
      {
        if (pFront)
          *pFront = *this;
        if (pBack)
          *pBack = *this;
      }
      return;
    }
  }

  astl::vector<tU8> newVert;
  newVert.resize(munVertexSize);

  eClassify clA, clB;
  itA = GetFirstPosition();
  itB = GetNextPosition(itA);
  for (tU32 i = 0; i < GetNumVertices(); ++i)
  {
    clA = vCl[i];
    clB = vCl[(i+1)%GetNumVertices()];

    if (clA == eClassify_Front)
    {
      if (pFront) pFront->AddVertices(tPtr(itA));
    }
    else if (clA == eClassify_Back)
    {
      if (pBack)  pBack->AddVertices(tPtr(itA));
    }
    else /*if (clA == eClassify_Coplanar)*/
    {
      if (pFront) pFront->AddVertices(tPtr(itA));
      if (pBack)  pBack->AddVertices(tPtr(itA));
    }

    if ((clA == eClassify_Front && clB == eClassify_Back) ||
        (clA == eClassify_Back && clB == eClassify_Front))
    {
      tF32 fPercent = 0.0f;
      PlaneIntersectLine(Plane, *itA, *itB, (sVec3f*)NULL, &fPercent);
      FVFLerp(GetFVF(), (tPtr)&newVert[0], (tPtr)itA, (tPtr)itB, fPercent);
      if (pBack)  pBack->AddVertices(tPtr(&newVert[0]));
      if (pFront) pFront->AddVertices(tPtr(&newVert[0]));
    }

    itA = GetNextPosition(itA);
    itB = GetNextPosition(itA);
    if (IsEndPosition(itB)) itB = GetFirstPosition();
  }

  if (pFront && pFront->IsValidPolygon())
  {
    pFront->SetPlane(mPlane);
    pFront->SetMaterial(GetMaterial());
    pFront->SetFlags(GetFlags());
  }

  if (pBack && pBack->IsValidPolygon())
  {
    pBack->SetPlane(mPlane);
    pBack->SetMaterial(GetMaterial());
    pBack->SetFlags(GetFlags());
  }
}

///////////////////////////////////////////////
//! Clip the polygon against the given set of planes. The planes are supposted to form a convex hull.
inline tBool __stdcall cPolygon::Clip(const sPlanef* apPlanes, tU32 aulNumPlanes, cPolygon& aResult, tBool abKeepBack, tBool abKeepCoplanar, tF32 epsilon) const
{
  cPolygon *pA, *pB;
  cPolygon polyA = *this;
  cPolygon polyB(GetFVF());
  polyB.ReserveVertices(16);

  pA = &polyA;
  pB = &polyB;

  for (tU32 i = 0; i < aulNumPlanes; ++i)
  {
    pB->ClearVertices();
    if (abKeepBack) pA->Split(apPlanes[i], NULL, pB, abKeepCoplanar, epsilon);
    else      pA->Split(apPlanes[i], pB, NULL, abKeepCoplanar, epsilon);
    Swap(pA,pB);
    if (!pA->IsValidPolygon())
      break;
  }

  aResult.Copy(pA);
  return aResult.IsValidPolygon();
}

///////////////////////////////////////////////
//! Generate vertices for the given plane the size is fWidth.
inline tBool __stdcall cPolygon::GenerateBaseForPlane(const sPlanef& Plane, const tF32 fWidth, tBool bCCW)
{
  sVec3f vOrg = kvec3fZero, vRight = kvec3fZero, vUp = kvec3fZero;
  ClearVertices();

  // find the major axis
  sVec3f vA = kvec3fZero;
  if (ni::Abs(Plane.y) > ni::Abs(Plane.z))
  {
    if (ni::Abs(Plane.z) < ni::Abs(Plane.x))
      vA.z = 1.0f;
    else
      vA.x = 1.0f;
  }
  else
  {
    if (ni::Abs(Plane.y) <= ni::Abs(Plane.x))
      vA.y = 1.0f;
    else
      vA.x = 1.0f;
  }

  const sVec3f planeNormal = Plane.GetNormal();
  VecNormalize(VecCross(vUp,    vA,  planeNormal));
  VecNormalize(VecCross(vRight, vUp, planeNormal));

  vUp *= fWidth*0.5f;
  vRight *= fWidth*0.5f;

  vOrg = -Plane.GetDist() * planeNormal;

  mvVertices.resize(4*munVertexSize);
  munNumVertices = 4;
  if (bCCW)
  {
    *GetPosition(3) = vOrg - vRight + vUp;
    *GetPosition(2) = vOrg + vRight + vUp;
    *GetPosition(1) = vOrg + vRight - vUp;
    *GetPosition(0) = vOrg - vRight - vUp;
  }
  else
  {
    *GetPosition(0) = vOrg - vRight + vUp;
    *GetPosition(1) = vOrg + vRight + vUp;
    *GetPosition(2) = vOrg + vRight - vUp;
    *GetPosition(3) = vOrg - vRight - vUp;
  }

  mPlane = Plane;
  return eTrue;
}

///////////////////////////////////////////////
//! Generate vertices for the given plane the size is defined by the specified AABB.
inline tBool __stdcall cPolygon::GenerateBaseForPlaneAABB(const sPlanef& Plane, const sVec3f& avMin, const sVec3f& avMax, tBool bCCW)
{
  sVec3f vOrg = kvec3fZero, vRight = kvec3fZero, vUp = kvec3fZero;
  ClearVertices();

  // find the major axis
  sVec3f vA = kvec3fZero;
  if (ni::Abs(Plane.y) > ni::Abs(Plane.z))
  {
    if (ni::Abs(Plane.z) < ni::Abs(Plane.x))
      vA.z = 1.0f;
    else
      vA.x = 1.0f;
  }
  else
  {
    if (ni::Abs(Plane.y) <= ni::Abs(Plane.x))
      vA.y = 1.0f;
    else
      vA.x = 1.0f;
  }

  const sVec3f planeNormal = Plane.GetNormal();
  VecNormalize(VecCross(vUp,    vA,   planeNormal));
  VecNormalize(VecCross(vRight, vUp,  planeNormal));

  sVec3f CB = (avMax+avMin)/2.0f;
  tF32 fLength = VecLength(avMax-CB);
  vOrg = CB + (-PlaneDotCoord(Plane,CB) * planeNormal);

  vUp *= fLength;
  vRight *= fLength;

  mvVertices.resize(4*munVertexSize);
  munNumVertices = 4;
  if (bCCW)
  {
    *GetPosition(3) = vOrg - vRight + vUp;
    *GetPosition(2) = vOrg + vRight + vUp;
    *GetPosition(1) = vOrg + vRight - vUp;
    *GetPosition(0) = vOrg - vRight - vUp;
  }
  else
  {
    *GetPosition(0) = vOrg - vRight + vUp;
    *GetPosition(1) = vOrg + vRight + vUp;
    *GetPosition(2) = vOrg + vRight - vUp;
    *GetPosition(3) = vOrg - vRight - vUp;
  }

  mPlane = Plane;
  return eTrue;
}

///////////////////////////////////////////////
//! Sort the vertices in clockwise order.
inline void __stdcall cPolygon::SortVerts()
{
  niAssert(IsValidPolygon());

  sVec3f vCOM = GetCOM();

  // Sort vertices
  for (tU16 i = 0; i < GetNumVertices()-2; ++i)
  {
    sVec3f a;
    sPlanef p;
    tF32  fSmallestAngle  = -1;
    int   nSmallest   = -1;

    a = *GetPosition(i) - vCOM;
    VecNormalize(a);

    PlaneFromPoints<tF32>(p, *GetPosition(i), vCOM, vCOM+mPlane.GetNormal());
    PlaneNormalize(p);

    for (tU32 j = i+1; j < GetNumVertices(); ++j)
    {
      if (ClassifyPoint(p,*GetPosition(j)) == eClassify_Back)
        continue;

      sVec3f b;
      tF32 fAngle;

      b = *GetPosition(j) - vCOM;
      VecNormalize(b);

      fAngle = VecDot(a,b);
      if (fAngle > fSmallestAngle)
      {
        fSmallestAngle  = fAngle;
        nSmallest   = j;
      }
    }

    niAssertMsg(nSmallest != -1, _A("Invalid polygon."));

    // swap the vertices
    SwapVertices(nSmallest, i+1);
  }

  // Check if vertex order needs to be reversed for back-facing polygon
  sPlanef oldPlane(mPlane);
  ComputePlane();
  if (PlaneDotNormal(mPlane,oldPlane.GetNormal()) < 0)
  {
    Reverse();
  }
}

///////////////////////////////////////////////
//! Reverse the polygon facing.
inline void __stdcall cPolygon::Reverse()
{
  int j = GetNumVertices();
  for (int i = 0; i < j/2; ++i)
  {
    SwapVertices(i, j-i-1);
  }
  ComputePlane();
}

///////////////////////////////////////////////
//! Return the closest point to the specified point that lay on an edge of this polygon.
inline sVec3f& __stdcall cPolygon::ClosestPointOnPerimeter(sVec3f& vOut, const sVec3f& pt,
                                                              sVec3f* pEA, sVec3f* pEB,
                                                              tBool* pbEdgeFlag) const
{
  tBool  bFound = eFalse;
  tF32   fClosestDistance = 0.0f;
  sVec3f vClosestPoint = kvec3fZero;
  sVec3f vClosestP0, vClosestP1;

  const sVec3f *p0 = reinterpret_cast<sVec3f*>(GetVertex(GetNumVertices()-1)), *p1;
  sVec3f cp;
  tI32  index = 0, lClosestIndex = -1;
  tBool bEdgeFlag = eFalse;

  for (tPtr itV = GetFirstVertex(); !IsEndVertex(itV); itV = GetNextVertex(itV), ++index)
  {
    p1 = reinterpret_cast<sVec3f*>(itV);
    tBool bEdge;

    ClosestPointOnLineSegment(cp, *p0, *p1, pt, &bEdge);
    tF32 d = VecDistance(cp,pt);

    if (!bFound || d < fClosestDistance)
    {
      fClosestDistance = d;
      vClosestPoint = cp;
      vClosestP0 = *p0;
      vClosestP1 = *p1;
      bEdgeFlag = bEdge;
      lClosestIndex = index;
      bFound = eTrue;
    }

    p0 = p1;
  }

  if (pEA && pEB && lClosestIndex >= 0)
  {
    if (!bEdgeFlag)
    {
      tI32 a = lClosestIndex - 1; if (a < 0) a = GetNumVertices()-1;
      tI32 b = lClosestIndex + 1; if (b >= tI32(GetNumVertices())) b = 0;
      *pEA = *reinterpret_cast<sVec3f*>(GetVertex(a));
      *pEB = *reinterpret_cast<sVec3f*>(GetVertex(b));
    }
    else
    {
      *pEA = vClosestP0;
      *pEB = vClosestP1;
    }
  }

  if (pbEdgeFlag)
    *pbEdgeFlag = bEdgeFlag;

  vOut = vClosestPoint;
  return vOut;
}

///////////////////////////////////////////////
inline tBool __stdcall cPolygon::IsDegenerate() const
{
  tU32 nNumPos = GetNumVertices();
  for (tU32 i = 0; i < nNumPos; ++i)
  {
    for (tU32 j = i+1; j < nNumPos; ++j)
    {
      if (VecEqual(*GetPosition(i),*GetPosition(j)))
        return eTrue;
    }
  }
  return eFalse;
}

///////////////////////////////////////////////
//! Set the polygon's material.
inline void __stdcall cPolygon::SetMaterial(tU32 aunMatID)
{
  munMaterial = aunMatID;
}

///////////////////////////////////////////////
inline void __stdcall cPolygon::SetFlags(tU32 anFlags)
{
  mnFlags = anFlags;
}

///////////////////////////////////////////////
inline tU32 __stdcall cPolygon::GetFlags() const
{
  return mnFlags;
}

///////////////////////////////////////////////
//! Get the polygon's material.
inline tU32 __stdcall cPolygon::GetMaterial() const
{
  return munMaterial;
}

///////////////////////////////////////////////
//! Copy only the vertices of the specified polygon.
inline void __stdcall cPolygon::CopyVertices(const cPolygon* apPoly)
{
  const cPolygon& p = (const cPolygon&)*apPoly;

  munNumVertices = p.GetNumVertices();
  mvVertices.resize(munVertexSize*munNumVertices);

  if (IsSameFVF(apPoly))
  {
    memcpy((void*)&mvVertices[0], p.GetFirstVertex(), mvVertices.size());
  }
  else
  {
    FVFCopy(tPtr(&mvVertices[0]), cFVFDescription(mFVF),
            p.GetFirstVertex(), cFVFDescription(p.GetFVF()),
            p.GetNumVertices());
  }

  mPlane = p.GetPlane();
}

///////////////////////////////////////////////
//! Copy the specified polygon in this polygon.
inline void __stdcall cPolygon::Copy(const cPolygon* apRight)
{
  const cPolygon& r = (const cPolygon&)*apRight;
  mvVertices = r.mvVertices;
  munNumVertices = r.munNumVertices;
  munMaterial = r.munMaterial;
  mnFlags = r.mnFlags;
  mFVF = r.mFVF;
  munVertexSize = r.munVertexSize;
  mPlane = r.mPlane;
}

///////////////////////////////////////////////
//! Create a polygon with the same FVF as this polygon.
inline cPolygon* __stdcall cPolygon::Create() const
{
  return niNew cPolygon(mFVF);
}

///////////////////////////////////////////////
//! Create a copy of this polygon.
inline cPolygon* __stdcall cPolygon::Clone() const
{
  cPolygon* pPoly = niNew cPolygon();
  pPoly->Copy(this);
  return pPoly;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __POLYGON_40138691_H__
