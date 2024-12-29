// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"

#include <niLang/STL/utils.h>
#include "API/niUI/Utils/AABB.h"

//--------------------------------------------------------------------------------------------
//
//  Draw Operation implementation.
//
//--------------------------------------------------------------------------------------------

//! Draw operation implementation.
class cDrawOperation : public ImplRC<iDrawOperation>
{
  niBeginClass(cDrawOperation);

 public:
  //! Constructor.
  cDrawOperation();
  //! Destructor.
  ~cDrawOperation();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  //// iDrawOperation ///////////////////////////
  iDrawOperationSet* __stdcall GetDrawOperationSet() const;
  tBool __stdcall Copy(const iDrawOperation* apDO);
  tBool __stdcall GetIsCompiled() const { return eTrue; }
  iDrawOperation* __stdcall Clone() const;
  void __stdcall SetPriority(tU32 anPriority);
  tU32 __stdcall GetPriority() const;
  void __stdcall SetVertexArray(iVertexArray* apVertexArray);
  iVertexArray* __stdcall GetVertexArray() const;
  void __stdcall SetIndexArray(iIndexArray* apIndexArray);
  iIndexArray* __stdcall GetIndexArray() const;
  void __stdcall SetFirstIndex(tU32 anIndex);
  tU32 __stdcall GetFirstIndex() const;
  void __stdcall SetNumIndices(tU32 anNumIndices);
  tU32 __stdcall GetNumIndices() const;
  void __stdcall SetBaseVertexIndex(tU32 anBaseVertexIndex);
  tU32 __stdcall GetBaseVertexIndex() const;
  void __stdcall SetLocalBoundingVolume(iBoundingVolume* apBV);
  iBoundingVolume* __stdcall GetLocalBoundingVolume() const;
  iBoundingVolume* __stdcall GetBoundingVolume() const;
  sVec3f __stdcall GetCenter() const;
  void __stdcall SetMatrix(const sMatrixf& aMatrix);
  sMatrixf __stdcall GetMatrix() const;
  void __stdcall SetMaterial(iMaterial* apMaterial);
  iMaterial* __stdcall GetMaterial() const;
  tFVF __stdcall GetFVF() const;
  tBool __stdcall SetFlags(tDrawOperationFlags aType);
  tDrawOperationFlags __stdcall GetFlags() const;

  ///////////////////////////////////////////////
  void __stdcall SetPrimitiveType(eGraphicsPrimitiveType aPrim) {
    if (mptrIndexArray.IsOK()) return;
    mPrimitiveType = aPrim;
  }
  eGraphicsPrimitiveType __stdcall GetPrimitiveType() const {
    return mPrimitiveType;
  }
  //// iDrawOperation ///////////////////////////

 private:
  tU32      mnPriority;
  Ptr<iVertexArray> mptrVertexArray;
  Ptr<iIndexArray>  mptrIndexArray;
  tU32      mnFirstIndex;
  tU32      mnNumIndices;
  tU32      mnBaseVertexIndex;
  eGraphicsPrimitiveType  mPrimitiveType;
  Ptr<iBoundingVolume>  mptrLocalBV;
  Ptr<iBoundingVolume>  mptrWorldBV;
  sMatrixf        mMatrix;
  Ptr<iMaterial>  mptrMaterial;
  sVec3f       mvCenter;
  tDrawOperationFlags   mFlags;

  niEndClass(cDrawOperation);
};

///////////////////////////////////////////////
cDrawOperation::cDrawOperation()
{
  mnFirstIndex = 0;
  mnNumIndices = 0;
  mnBaseVertexIndex = 0;
  mMatrix = sMatrixf::Identity();
  mFlags = 0;
  mPrimitiveType = eGraphicsPrimitiveType_TriangleList;
  mptrLocalBV = niNew cBoundingVolumeAABB();
  mptrLocalBV->SetMin(-sVec3f::One());
  mptrLocalBV->SetMax(+sVec3f::One());
  mptrWorldBV = mptrLocalBV->Clone();
}

///////////////////////////////////////////////
cDrawOperation::~cDrawOperation()
{
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cDrawOperation::IsOK() const
{
  niClassIsOK(cDrawOperation);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cDrawOperation::Copy(const iDrawOperation* apDO)
{
  SetPriority(apDO->GetPriority());
  SetVertexArray(apDO->GetVertexArray());
  SetIndexArray(apDO->GetIndexArray());
  SetFirstIndex(apDO->GetFirstIndex());
  SetNumIndices(apDO->GetNumIndices());
  SetBaseVertexIndex(apDO->GetBaseVertexIndex());
  SetLocalBoundingVolume(apDO->GetLocalBoundingVolume());
  SetMatrix(apDO->GetMatrix());
  SetMaterial(apDO->GetMaterial());
  return eTrue;
}

///////////////////////////////////////////////
iDrawOperation* __stdcall cDrawOperation::Clone() const
{
  iDrawOperation* pNew = niNew cDrawOperation();
  pNew->Copy(this);
  return pNew;
}

///////////////////////////////////////////////
void __stdcall cDrawOperation::SetPriority(tU32 anPriority)
{
  mnPriority = anPriority;
}

///////////////////////////////////////////////
tU32 __stdcall cDrawOperation::GetPriority() const
{
  return mnPriority;
}

///////////////////////////////////////////////
//! Set the vertex array.
void __stdcall cDrawOperation::SetVertexArray(iVertexArray* apVertexArray)
{
  mptrVertexArray = niGetIfOK(apVertexArray);
}

///////////////////////////////////////////////
//! Get the vertex array.
iVertexArray* __stdcall cDrawOperation::GetVertexArray() const
{
  return mptrVertexArray;
}

///////////////////////////////////////////////
//! Set the index array.
void __stdcall cDrawOperation::SetIndexArray(iIndexArray* apIndexArray)
{
  mptrIndexArray = niGetIfOK(apIndexArray);
  if (mptrIndexArray.IsOK()) {
    mPrimitiveType = (eGraphicsPrimitiveType)mptrIndexArray->GetPrimitiveType();
  }
}

///////////////////////////////////////////////
//! Get the index array.
iIndexArray* __stdcall cDrawOperation::GetIndexArray() const
{
  return mptrIndexArray;
}

///////////////////////////////////////////////
//! Set the first index.
void __stdcall cDrawOperation::SetFirstIndex(tU32 anIndex)
{
  mnFirstIndex = anIndex;
}

///////////////////////////////////////////////
//! Get the first index.
tU32 __stdcall cDrawOperation::GetFirstIndex() const
{
  return mnFirstIndex;
}

///////////////////////////////////////////////
//! Set the number of indices.
void __stdcall cDrawOperation::SetNumIndices(tU32 anNumIndices)
{
  mnNumIndices = anNumIndices;
}

///////////////////////////////////////////////
//! Get the number of indices.
tU32 __stdcall cDrawOperation::GetNumIndices() const
{
  return mnNumIndices;
}

///////////////////////////////////////////////
//! Set the base vertex index.
void __stdcall cDrawOperation::SetBaseVertexIndex(tU32 anBaseVertexIndex)
{
  mnBaseVertexIndex = anBaseVertexIndex;
}

///////////////////////////////////////////////
//! Get the base vertex index.
tU32 __stdcall cDrawOperation::GetBaseVertexIndex() const
{
  return mnBaseVertexIndex;
}

///////////////////////////////////////////////
void __stdcall cDrawOperation::SetLocalBoundingVolume(iBoundingVolume* apBV)
{
  mptrLocalBV = apBV;
  if (!mptrLocalBV.IsOK()) {
    mptrLocalBV = niNew cBoundingVolumeAABB();
    mptrLocalBV->SetCenter(sVec3f::Zero());
    mptrLocalBV->SetSize(sVec3f::One());
  }
  if (!mptrWorldBV.IsOK()) {
    mptrWorldBV = mptrLocalBV->Clone();
  }
  else {
    mptrWorldBV->Copy(mptrLocalBV);
  }
  mptrWorldBV->Transform(mMatrix);
}

///////////////////////////////////////////////
iBoundingVolume* __stdcall cDrawOperation::GetLocalBoundingVolume() const
{
  return mptrLocalBV;
}

///////////////////////////////////////////////
iBoundingVolume* __stdcall cDrawOperation::GetBoundingVolume() const
{
  return mptrWorldBV;
}

///////////////////////////////////////////////
//! Set the matrix.
void __stdcall cDrawOperation::SetMatrix(const sMatrixf& aMatrix)
{
  mMatrix = aMatrix;
  mptrWorldBV->Copy(mptrLocalBV);
  mptrWorldBV->Transform(mMatrix);
}

///////////////////////////////////////////////
//! Get the matrix.
sMatrixf __stdcall cDrawOperation::GetMatrix() const
{
  return mMatrix;
}

///////////////////////////////////////////////
void cDrawOperation::SetMaterial(iMaterial* apMaterial)
{
  mptrMaterial = apMaterial;
}

///////////////////////////////////////////////
//! Get the material.
iMaterial* __stdcall cDrawOperation::GetMaterial() const
{
  return mptrMaterial;
}

///////////////////////////////////////////////
//! Get the center position transformed by the matirx.
sVec3f __stdcall cDrawOperation::GetCenter() const
{
  return GetBoundingVolume()->GetCenter();
}

///////////////////////////////////////////////
tFVF __stdcall cDrawOperation::GetFVF() const
{
  tFVF fvf = 0;
  if (!fvf && mptrVertexArray.IsOK())
    fvf = mptrVertexArray->GetFVF();
  return fvf;
}

///////////////////////////////////////////////
tBool __stdcall cDrawOperation::SetFlags(tDrawOperationFlags aFlags)
{
  mFlags = aFlags;
  return eTrue;
}

///////////////////////////////////////////////
tDrawOperationFlags __stdcall cDrawOperation::GetFlags() const
{
  return mFlags;
}

//--------------------------------------------------------------------------------------------
//
//  Draw Operation Set
//
//--------------------------------------------------------------------------------------------

typedef astl::vector<Ptr<iDrawOperation> >   tDrawOperationLst;
typedef tDrawOperationLst::iterator          tDrawOperationLstIt;
typedef tDrawOperationLst::const_iterator    tDrawOperationLstCIt;

// typedef astl::list<Ptr<iDrawOperation> > tDrawOperationLst;
// typedef tDrawOperationLst::iterator      tDrawOperationLstIt;
// typedef tDrawOperationLst::const_iterator  tDrawOperationLstCIt;

//////////////////////////////////////////////////////////////////////////////////////////////
// cDrawOperationSet declaration.

//! Draw operation set implementation.
class cDrawOperationSet : public ImplRC<iDrawOperationSet>
{
  niBeginClass(cDrawOperationSet);

 public:
  //! Constructor.
  cDrawOperationSet(iGraphics* apGraphics) {
    mCurrentDrawOp = eInvalidHandle;
  }
  //! Destructor.
  ~cDrawOperationSet() {
    Invalidate();
  }

  //! Sanity check.
  tBool __stdcall IsOK() const {
    niClassIsOK(cDrawOperationSet);
    return eTrue;
  }

  void __stdcall Invalidate() {
    mvDrawOps.clear();
    mCurrentDrawOp = eInvalidHandle;
  }

  //// iDrawOperationSet ////////////////////////
  tBool __stdcall GetIsEmpty() const {
    return !!mvDrawOps.empty();
  }
  void __stdcall Clear() {
    mvDrawOps.clear();
    mCurrentDrawOp = eInvalidHandle;
  }
  iDrawOperation* __stdcall Insert(iDrawOperation* apDO) {
    if (!niIsOK(apDO)) return NULL;
    mvDrawOps.push_back(apDO);
    mCurrentDrawOp = eInvalidHandle;
    return apDO;
  }
  tBool __stdcall InsertSet(const iDrawOperationSet* apSet) {
    if (!niIsOK(apSet)) return eFalse;
    iDrawOperationSet* s = const_cast<iDrawOperationSet*>(apSet);
    for (iDrawOperation* dop = s->Begin(); !s->IsEnd(); dop = s->Next()) {
      mvDrawOps.push_back(dop);
    }
    mCurrentDrawOp = eInvalidHandle;
    return eTrue;
  }

  tU32 __stdcall GetNumDrawOperations() const {
    return (tU32)mvDrawOps.size();
  }

  iDrawOperation* __stdcall Begin() {
    if (mvDrawOps.empty()) return NULL;
    mCurrentDrawOp = 0;
    return mvDrawOps[mCurrentDrawOp].ptr();
  }
  iDrawOperation* __stdcall Next() {
    const size_t numDrawOps = mvDrawOps.size();
    if (mCurrentDrawOp < numDrawOps) {
      ++mCurrentDrawOp;
      if (mCurrentDrawOp >= numDrawOps) {
        mCurrentDrawOp = eInvalidHandle;
        return NULL;
      }
      else {
        return mvDrawOps[mCurrentDrawOp].ptr();
      }
    }
    else {
      return NULL;
    }
  }
  tBool __stdcall IsEnd() const {
    return (mCurrentDrawOp >= mvDrawOps.size());
  }
  iDrawOperation* __stdcall GetCurrent() const {
    return (mCurrentDrawOp >= mvDrawOps.size()) ? NULL : mvDrawOps[mCurrentDrawOp].ptr();
  }

  tU32 __stdcall Draw(iGraphicsContext* apContext, iFrustum* apFrustum) {
    niCheckSilent(niIsOK(apContext),0);
    tU32 c = 0;
    niLoopit(tDrawOperationLst::const_iterator,it,mvDrawOps) {
      iDrawOperation* dop = *it;
      if (apFrustum && dop->GetBoundingVolume()) {
        if (dop->GetBoundingVolume()->IntersectFrustum(NULL,apFrustum) ==
            eIntersectionResult_None) {
          continue; // skip this draw op...
        }
      }
      apContext->DrawOperation(dop);
      ++c;
    }
    return c;
  }

  tU32 __stdcall XDraw(const sMatrixf& aMatrix, iGraphicsContext* apContext, iFrustum* apFrustum) {
    niCheckSilent(niIsOK(apContext),0);
    tU32 c = 0;
    niLoopit(tDrawOperationLst::const_iterator,it,mvDrawOps) {
      iDrawOperation* dop = *it;
      dop->SetMatrix(aMatrix);
      if (apFrustum && dop->GetBoundingVolume()) {
        if (dop->GetBoundingVolume()->IntersectFrustum(NULL,apFrustum) ==
            eIntersectionResult_None) {
          continue; // skip this draw op...
        }
      }
      apContext->DrawOperation(dop);
      ++c;
    }
    return c;
  }
  //// iDrawOperationSet ////////////////////////

 private:
  tDrawOperationLst   mvDrawOps;
  tU32 mCurrentDrawOp;

  niEndClass(cDrawOperationSet);
};

//--------------------------------------------------------------------------------------------
//
//  cGraphics
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
iDrawOperation* __stdcall cGraphics::CreateDrawOperation() {
  return niNew cDrawOperation();
}

///////////////////////////////////////////////
iDrawOperationSet* __stdcall cGraphics::CreateDrawOperationSet() {
  return niNew cDrawOperationSet(this);
}
