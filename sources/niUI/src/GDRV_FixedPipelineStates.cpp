// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include <niLang/Utils/DTS.h>
#include <niLang/Math/MathMatrix.h>
#include "API/niUI_ModuleDef.h"

//--------------------------------------------------------------------------------------------
//
//  FixedStates
//
//--------------------------------------------------------------------------------------------

class cFixedStates : public ImplRC<iFixedStates>,
                     public sFixedStatesDesc {
  niBeginClass(cFixedStates);

 public:
  const tF32 mfOrthoProjectionOffset; // for SetOrthoMatrices

  cFixedStates(tF32 afOrthoProjectionOffset)
      : mfOrthoProjectionOffset(afOrthoProjectionOffset)
  {
    mCameraViewMatrix = sMatrixf::Identity();
    mCameraProjectionMatrix = sMatrixf::Identity();
    mViewMatrix = sMatrixf::Identity();
    mProjectionMatrix = sMatrixf::Identity();
  }
  ~cFixedStates() {
    Invalidate();
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cFixedStates);
    return eTrue;
  }

  void __stdcall Invalidate() {
  }

  ///////////////////////////////////////////////
  virtual tPtr __stdcall GetDescStructPtr() const {
    return (tPtr)niStaticCast(const sFixedStatesDesc*,this);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Copy(const iFixedStates* apStates) {
    niCheckSilent(niIsOK(apStates),eFalse);
    SetOnlyCameraViewMatrix(apStates->GetCameraViewMatrix());
    SetOnlyCameraProjectionMatrix(apStates->GetCameraProjectionMatrix());
    SetViewMatrix(apStates->GetViewMatrix());
    SetProjectionMatrix(apStates->GetProjectionMatrix());
    return eTrue;
  }
  ///////////////////////////////////////////////
  virtual iFixedStates* __stdcall Clone() const {
    cFixedStates* pNew = niNew cFixedStates(mfOrthoProjectionOffset);
    pNew->Copy(this);
    return pNew;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCameraViewMatrix(const sMatrixf& aVal) {
    mCameraViewMatrix = aVal;
    SetViewMatrix(mCameraViewMatrix);
  }
  virtual void __stdcall SetOnlyCameraViewMatrix(const sMatrixf& aVal) {
    mCameraViewMatrix = aVal;
  }
  virtual sMatrixf __stdcall GetCameraViewMatrix() const {
    return mCameraViewMatrix;
  }
  virtual sMatrixf __stdcall GetCameraInvViewMatrix() const {
    sMatrixf retMtx;
    return MatrixInverse(retMtx,mCameraViewMatrix);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetCameraProjectionMatrix(const sMatrixf& aVal) {
    mCameraProjectionMatrix = aVal;
    SetProjectionMatrix(mCameraProjectionMatrix);
  }
  virtual void __stdcall SetOnlyCameraProjectionMatrix(const sMatrixf& aVal) {
    mCameraProjectionMatrix = aVal;
  }
  virtual sMatrixf __stdcall GetCameraProjectionMatrix() const {
    return mCameraProjectionMatrix;
  }
  virtual sMatrixf __stdcall GetCameraInvProjectionMatrix() const {
    sMatrixf retMtx;
    return MatrixInverse(retMtx,mCameraProjectionMatrix);
  }

  ///////////////////////////////////////////////
  virtual sMatrixf __stdcall GetCameraViewProjectionMatrix() const {
    return mCameraViewMatrix*mCameraProjectionMatrix;
  }

  ///////////////////////////////////////////////
  virtual sMatrixf __stdcall GetCameraInvViewProjectionMatrix() const {
    sMatrixf retMtx;
    return MatrixInverse(retMtx,GetCameraViewProjectionMatrix());
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetViewMatrix(const sMatrixf& aVal) {
    mViewMatrix = aVal;
  }
  virtual sMatrixf __stdcall GetViewMatrix() const {
    return mViewMatrix;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetProjectionMatrix(const sMatrixf& aVal) {
    mProjectionMatrix = aVal;
  }
  virtual sMatrixf __stdcall GetProjectionMatrix() const {
    return mProjectionMatrix;
  }

  ///////////////////////////////////////////////
  virtual sMatrixf __stdcall GetInvViewMatrix() const {
    sMatrixf retMtx;
    return MatrixInverse(retMtx,mViewMatrix);
  }

  ///////////////////////////////////////////////
  virtual sMatrixf __stdcall GetViewProjectionMatrix() const {
    return mViewMatrix*mProjectionMatrix;
  }

  ///////////////////////////////////////////////
  virtual sMatrixf __stdcall GetInvViewProjectionMatrix() const {
    sMatrixf retMtx;
    return MatrixInverse(retMtx,mViewMatrix*mProjectionMatrix);
  }

  ///////////////////////////////////////////////
  virtual sMatrixf __stdcall GetInvProjectionMatrix() const {
    sMatrixf retMtx;
    return MatrixInverse(retMtx,mProjectionMatrix);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetLookAtMatrices(tBool abSetCameraMatrices,
                                           const sVec3f& avEye, const sVec3f& avAt, const sVec3f& avUp,
                                           tF32 afFovY, tF32 afAspect, tF32 afNear, tF32 afFar)
  {
    sMatrixf mtxView, mtxProj;
    MatrixLookAtLH(mtxView, avEye, avAt, avUp);
    MatrixPerspectiveFovLH(mtxProj, afFovY, afAspect, afNear, afFar);
    if (abSetCameraMatrices) {
      SetCameraViewMatrix(mtxView);
      SetCameraProjectionMatrix(mtxProj);
    }
    else {
      SetViewMatrix(mtxView);
      SetProjectionMatrix(mtxProj);
    }
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetOrthoMatrices(tBool abSetCameraMatrices, const sRectf& arectViewport, tF32 afNear, tF32 afFar) {
    sRectf rect(0,0,arectViewport.GetWidth(),arectViewport.GetHeight());
    sVec2f origin = Vec2((tF32)arectViewport.Left()+mfOrthoProjectionOffset,(tF32)arectViewport.Top()+mfOrthoProjectionOffset);
    rect -= origin;

    sMatrixf mtxView = sMatrixf::Identity();
    sMatrixf mtxProj;
    MatrixOrthoOffCenterLH(mtxProj,rect.Left(),rect.Right(),rect.Bottom(),rect.Top(),afNear,afFar);

    if (abSetCameraMatrices) {
      SetCameraViewMatrix(mtxView);
      SetCameraProjectionMatrix(mtxProj);
    }
    else {
      SetViewMatrix(mtxView);
      SetProjectionMatrix(mtxProj);
    }
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) {
    _DTS_INIT(apDT,aFlags,eFalse);
    _DTS_MATRIX("camera_view_matrix", mCameraViewMatrix);
    _DTS_MATRIX("camera_proj_matrix", mCameraProjectionMatrix);
    _DTS_MATRIX("view_matrix", mViewMatrix);
    _DTS_MATRIX("proj_matrix", mProjectionMatrix);
    return ni::eTrue;
  }

  niEndClass(cFixedStates);
};

//--------------------------------------------------------------------------------------------
//
//  cGraphics
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
iFixedStates* __stdcall cGraphics::CreateFixedStates() {
  CHECKDRIVER(NULL);
  return niNew cFixedStates(ultof(GetDriverCaps(eGraphicsCaps_OrthoProjectionOffset)));
}
