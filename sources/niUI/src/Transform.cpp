// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Transform.h"
#include <niLang/Math/MathMatrix.h>
#include <niLang/Math/MathUtils.h>

static const tU32 _kMaxPushStackSize = 64;

template <typename T>
void _RemoveScaling(sMatrix<T>& aMatrix, const sVec3<T>& avScale) {
  sVec3<T> vIS;
  VecInverse(vIS,avScale);
  sMatrix<T> mtxScale;
  MatrixMultiply(aMatrix, MatrixScaling(mtxScale,vIS), aMatrix);
}

template <typename T>
void _AddScaling(sMatrix<T>& aMatrix, const sVec3<T>& avScale) {
  sMatrix<T> mtxScale;
  MatrixMultiply(aMatrix, MatrixScaling(mtxScale,avScale), aMatrix);
}

void _GetParentMatrix(sMatrixd& mtxParentWorld,
                      const iTransform* apParent,
                      tU32 aFlags,
                      tBool abRemoveParentScale)
{
  // dont inherit rotation
  if (niFlagIsNot(aFlags,eTransformFlags_InheritRotation)) {
    sVec3d parentPos;
    CopyArray(parentPos.ptr(), apParent->GetWorldPosition().ptr(), 3);
    sVec3d pos = {0,0,0};
    if (niFlagIs(aFlags,eTransformFlags_InheritPositionX)) {
      pos.x = parentPos.x;
    }
    if (niFlagIs(aFlags,eTransformFlags_InheritPositionY)) {
      pos.y = parentPos.y;
    }
    if (niFlagIs(aFlags,eTransformFlags_InheritPositionZ)) {
      pos.z = parentPos.z;
    }
    MatrixTranslation(mtxParentWorld,pos);
  }
  // inherit rotation
  else {
    CopyArray(mtxParentWorld.ptr(), apParent->GetWorldMatrix().ptr(), 16);
    // always remove the scaling factor if present, scaling inheritance applied in ::DoUpdate
    if (abRemoveParentScale && niFlagTest(apParent->GetFlags(),eTransformInternalFlags_UseScale)) {
      sVec3d parentScale;
      CopyArray(parentScale.ptr(), apParent->GetScale().ptr(), 3);
      _RemoveScaling(mtxParentWorld,parentScale);
    }
    // apply inherit position
    if (!niFlagIs(aFlags,eTransformFlags_InheritPositionX))
      mtxParentWorld._41 = 0;
    if (!niFlagIs(aFlags,eTransformFlags_InheritPositionY))
      mtxParentWorld._42 = 0;
    if (!niFlagIs(aFlags,eTransformFlags_InheritPositionZ))
      mtxParentWorld._43 = 0;
  }
}

///////////////////////////////////////////////
cTransform::cTransform(iTransform* pParent)
{
  mnFlags = eTransformFlags_InheritAll;
  this->SetParent(pParent);
  Identity();
  _SetDirty();
}

///////////////////////////////////////////////
cTransform::~cTransform()
{
  Invalidate();
}

///////////////////////////////////////////////
void __stdcall cTransform::Invalidate()
{
  SetParent(NULL);
}

///////////////////////////////////////////////
tBool cTransform::IsOK() const
{
  return eTrue;
}

///////////////////////////////////////////////
void cTransform::Identity()
{
  mmtxWorld = sMatrixf::Identity();
  mmtxLocal = sMatrixf::Identity();
  mvScale = sVec3f::One();
  mnFlags &= ~eTransformInternalFlags_UseScale;
  _SetDirty();
}

///////////////////////////////////////////////
tU16 __stdcall cTransform::SetDirty() {
  _SetDirty();
  return mnSyncCounter-1;
}

///////////////////////////////////////////////
iTransform* __stdcall cTransform::Clone() const
{
  cTransform* pNew = niNew cTransform();
  pNew->Copy(this);
  return pNew;
}

///////////////////////////////////////////////
tBool __stdcall cTransform::Copy(const iTransform* apSrc)
{
  if (!niIsOK(apSrc)) return eFalse;
  mnFlags = apSrc->GetFlags();
  mptrParent = apSrc->GetParent();
  mmtxLocal = apSrc->GetLocalMatrix();;
  mmtxWorld = apSrc->GetWorldMatrix();
  mvScale = apSrc->GetScale();
  _SetDirty();
  return eTrue;
}

///////////////////////////////////////////////
void cTransform::SetWorldMatrix(const sMatrixf& aMatrix)
{
  sMatrixd newWorld;
  CopyArray(newWorld.ptr(), aMatrix.ptr(), 16);
  sMatrixd curLocal, newLocal;
  CopyArray(curLocal.ptr(), mmtxLocal.ptr(), 16);
  newLocal = curLocal;
  // Deduct local matrix
  if (GetParent()) {
    sMatrixd mtxParent;
    _GetParentMatrix(mtxParent,GetParent(),mnFlags,eTrue);
    sMatrixd mtxInvParent;
    MatrixMultiply(newLocal, newWorld, MatrixInverse(mtxInvParent,mtxParent));
  }
  else {
    newLocal = newWorld;
  }
  if (niFlagTest(mnFlags,eTransformInternalFlags_UseScale)) {
    sVec3d thisScale;
    CopyArray(thisScale.ptr(), mvScale.ptr(), 3);
    _RemoveScaling(newLocal,thisScale);
  }
  if (curLocal != newLocal) {
    CopyArray(mmtxLocal.ptr(), newLocal.ptr(), 16);
    _SetDirty();
  }
}

///////////////////////////////////////////////
void cTransform::_UpdateWorldMatrix()
{
  if (mptrParent.IsOK())
  {
    // Don't try to be clever here - for example by referencing the parent's mnSyncCounter directly - GetSyncCounter() also check the parent of the parent, etc...
    const tU16 parentSyncCounter = mptrParent->GetSyncCounter();
    if (niFlagIs(mnFlags,eTransformInternalFlags_Dirty)) {
    }
    else if (mnParentSyncCounter != parentSyncCounter) {
      ++mnSyncCounter;
    }
    else {
      return;
    }

    mnParentSyncCounter = parentSyncCounter;

    sVec3d parentScale = sVec3d::One();
    sMatrixd mtxParentWorld, mtxLocal, mtxWorld;
    _GetParentMatrix(mtxParentWorld,mptrParent,mnFlags,eTrue);
    CopyArray(mtxLocal.ptr(), mmtxLocal.ptr(), 16);
    CopyArray(mtxWorld.ptr(), mmtxWorld.ptr(), 16);

    if (niFlagTest(mnFlags,eTransformFlags_InheritScale)) {
      CopyArray(parentScale.ptr(), mptrParent->GetScale().ptr(), 3);
    }
    if (niFlagTest(mnFlags,eTransformInternalFlags_UseScale)) {
      sVec3d thisScale;
      CopyArray(thisScale.ptr(), mvScale.ptr(), 3);
      MatrixRotationPivotAndScale(mtxWorld,mtxLocal,sVec3d::Zero(),thisScale*parentScale);
      mtxWorld *= mtxParentWorld;
    }
    else {
      MatrixMultiply(mtxWorld, mtxLocal, mtxParentWorld);
    }

    CopyArray(mmtxWorld.ptr(), mtxWorld.ptr(), 16);
  }
  else {
    if (!niFlagIs(mnFlags,eTransformInternalFlags_Dirty)) {
      return;
    }

    if (niFlagTest(mnFlags,eTransformInternalFlags_UseScale)) {
      MatrixRotationPivotAndScale(mmtxWorld,mmtxLocal,sVec3f::Zero(),mvScale);
    }
    else {
      mmtxWorld = mmtxLocal;  // no parent, world and local matrix are the same
    }
  }

  niFlagOff(mnFlags,eTransformInternalFlags_Dirty);
}

///////////////////////////////////////////////
tU16 __stdcall cTransform::GetFlags() const {
  return mnFlags;
}
void __stdcall cTransform::SetFlags(tU16 anFlags) {
  mnFlags = anFlags;
}

///////////////////////////////////////////////
tU16 __stdcall cTransform::GetSyncCounter() const {
  niThis(cTransform)->_UpdateWorldMatrix();
  return mnSyncCounter;
}
void __stdcall cTransform::SetSyncCounter(tU16 anSyncCounter) {
  mnSyncCounter = anSyncCounter;
}

///////////////////////////////////////////////
void cTransform::SetParent(iTransform* pParent)
{
  niCheck(pParent != this,;);
  if (mptrParent != pParent) {
    mptrParent = niGetIfOK(pParent);
    if (mptrParent.IsOK()) {
      mnParentSyncCounter = ((cTransform*)pParent)->mnSyncCounter-1;
    }
  }
}

///////////////////////////////////////////////
iTransform* cTransform::GetParent() const
{
  return mptrParent;
}

///////////////////////////////////////////////
sMatrixf cTransform::GetWorldMatrix() const
{
  niThis(cTransform)->_UpdateWorldMatrix();
  return mmtxWorld;
}

///////////////////////////////////////////////
void cTransform::SetLocalMatrix(const sMatrixf& aMatrix)
{
  mmtxLocal = aMatrix;
  _SetDirty();
}

///////////////////////////////////////////////
sMatrixf cTransform::GetLocalMatrix() const
{
  return mmtxLocal;
}

///////////////////////////////////////////////
void cTransform::MultiplyWorldMatrix(const sMatrixf& aMatrix)
{
  niThis(cTransform)->_UpdateWorldMatrix();
  sMatrixf mtx;
  SetWorldMatrix(MatrixMultiply(mtx,mmtxWorld,aMatrix));
}

///////////////////////////////////////////////
void cTransform::PreMultiplyWorldMatrix(const sMatrixf& aMatrix)
{
  niThis(cTransform)->_UpdateWorldMatrix();
  sMatrixf mtx;
  SetWorldMatrix(MatrixMultiply(mtx,aMatrix,mmtxWorld));
}

///////////////////////////////////////////////
void cTransform::LookAt(const sVec3f& avLookAt, const sVec3f& avUp) {

  sVec3d worldPos, lookat, up;
  CopyArray(worldPos.ptr(), this->GetWorldPosition().ptr(), 3);
  CopyArray(lookat.ptr(), avLookAt.ptr(), 3);
  CopyArray(up.ptr(), avUp.ptr(), 3);

  ni::sMatrixd mtxTmp1, mtxTmp2;
  MatrixInverse(
    mtxTmp1,
    MatrixLookAtLH(
      mtxTmp2,
      worldPos, // eye position
      lookat, // target position
      up)); // up vector

  ni::sMatrixf mtxf;
  CopyArray(mtxf.ptr(), mtxTmp1.ptr(), 16);

  this->SetWorldRotation(mtxf);

}

///////////////////////////////////////////////
void cTransform::MultiplyLocalMatrix(const sMatrixf& aMatrix)
{
  MatrixMultiply(mmtxLocal,mmtxLocal,aMatrix);
  _SetDirty();
}

///////////////////////////////////////////////
void cTransform::PreMultiplyLocalMatrix(const sMatrixf& aMatrix)
{
  MatrixMultiply(mmtxLocal,aMatrix,mmtxLocal);
  _SetDirty();
}

///////////////////////////////////////////////
void cTransform::SetWorldPosition(const sVec3f& v)
{
  niThis(cTransform)->_UpdateWorldMatrix();
  sMatrixf mtx = mmtxWorld;
  MatrixSetTranslation(mtx,v);
  SetWorldMatrix(mtx);
}

///////////////////////////////////////////////
sVec3f cTransform::GetWorldPosition() const
{
  niThis(cTransform)->_UpdateWorldMatrix();
  sVec3f temp;
  return MatrixGetTranslation(temp,mmtxWorld);
}

///////////////////////////////////////////////
void cTransform::SetLocalPosition(const sVec3f& v)
{
  MatrixSetTranslation(mmtxLocal, v);
  _SetDirty();
}

///////////////////////////////////////////////
sVec3f cTransform::GetLocalPosition() const
{
  sVec3f t;
  return MatrixGetTranslation(t,mmtxLocal);
}

///////////////////////////////////////////////
void cTransform::Translate(const sVec3f& v)
{
  sVec3f t;
  sVec3f vNewPos = MatrixGetTranslation(t,mmtxLocal)+v;
  SetLocalPosition(vNewPos);
}

///////////////////////////////////////////////
void cTransform::PreTranslate(const sVec3f& v)
{
  sMatrixf t;
  MatrixMultiply(mmtxLocal, MatrixTranslation(t,v), mmtxLocal);
  _SetDirty();
}

///////////////////////////////////////////////
void cTransform::SetWorldRotation(const sMatrixf& aMatrix)
{
  niThis(cTransform)->_UpdateWorldMatrix();
  MatrixSetRotation(mmtxWorld, aMatrix);
  if (niFlagIs(mnFlags,eTransformInternalFlags_UseScale)) {
    _AddScaling(mmtxWorld,mvScale);
  }
  SetWorldMatrix(mmtxWorld);
}

///////////////////////////////////////////////
void cTransform::SetLocalRotation(const sMatrixf& aMatrix)
{
  MatrixSetRotation(mmtxLocal, aMatrix);
  _SetDirty();
}

///////////////////////////////////////////////
void cTransform::Rotate(const sMatrixf& aMatrix)
{
  MatrixRotate(mmtxLocal, mmtxLocal, aMatrix);
  _SetDirty();
}

///////////////////////////////////////////////
void cTransform::PreRotate(const sMatrixf& aMatrix)
{
  MatrixRotate(mmtxLocal, aMatrix, mmtxLocal);
  _SetDirty();
}

///////////////////////////////////////////////
void cTransform::SetScale(const sVec3f& aScale)
{
  if (mvScale == aScale) return;
  mvScale = VecZeroToEpsilon(aScale);
  niFlagOnIf(mnFlags, eTransformInternalFlags_UseScale, mvScale != sVec3f::One());
  _SetDirty();
}

///////////////////////////////////////////////
sVec3f cTransform::GetScale() const
{
  return mvScale;
}

///////////////////////////////////////////////
iTransform* __stdcall cTransform::CreatePreOffsetTransform() {
  Ptr<iTransform> ptrThisParent = GetParent();
  Ptr<iTransform> ptrOffset = niNew cTransform(ptrThisParent);
  SetParent(ptrOffset);
  return ptrOffset.GetRawAndSetNull();
}

///////////////////////////////////////////////
iTransform* __stdcall cTransform::CreatePostOffsetTransform() {
  Ptr<iTransform> ptrOffset = niNew cTransform(this);
  return ptrOffset.GetRawAndSetNull();
}

///////////////////////////////////////////////
sVec3f __stdcall cTransform::GetRight() const
{
  sVec3f v;
  return VecNormalize(MatrixGetRight(v,GetWorldMatrix()));
}

///////////////////////////////////////////////
sVec3f __stdcall cTransform::GetUp() const
{
  sVec3f v;
  return VecNormalize(MatrixGetUp(v,GetWorldMatrix()));
}

///////////////////////////////////////////////
sVec3f __stdcall cTransform::GetForward() const
{
  sVec3f v;
  return VecNormalize(MatrixGetForward(v,GetWorldMatrix()));
}

///////////////////////////////////////////////
sVec3f __stdcall cTransform::GetInvRight() const
{
  sVec3f v;
  sMatrixf mtx;
  return VecNormalize(MatrixGetRight(v,MatrixInverse(mtx,GetWorldMatrix())));
}

///////////////////////////////////////////////
sVec3f __stdcall cTransform::GetInvUp() const
{
  sVec3f v;
  sMatrixf mtx;
  return VecNormalize(MatrixGetUp(v,MatrixInverse(mtx,GetWorldMatrix())));
}

///////////////////////////////////////////////
sVec3f __stdcall cTransform::GetInvForward() const
{
  sVec3f v;
  sMatrixf mtx;
  return VecNormalize(MatrixGetForward(v,MatrixInverse(mtx,GetWorldMatrix())));
}
