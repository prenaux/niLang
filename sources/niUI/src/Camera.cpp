// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include <niLang/Math/MathCurves.h>
#include <niLang/Math/MathFPU.h>
#include <niLang/Math/MathUtils.h>
#include <niLang/Math/MathMatrix.h>
#include "API/niUI/ICamera.h"
#include "Graphics.h"

#define CAMDIRTY_MOVE     niBit(0)  // View matrix changed
#define CAMDIRTY_VIEW     niBit(1)  // View matrix changed
#define CAMDIRTY_PROJ     niBit(2)  // Projection parameter changed
#define CAMDIRTY_ALL      (CAMDIRTY_MOVE|CAMDIRTY_VIEW|CAMDIRTY_PROJ)
#define CAMDIRTY_UPDATING niBit(3)
#define CAMMOVE_TARGET    niBit(0)
#define CAMMOVE_ORBIT   niBit(1)

#define CAMUPDATE() if (mnDirtyFlags && niFlagIsNot(mnDirtyFlags,CAMDIRTY_UPDATING)) { niThis(cCamera)->_UpdateCamera(); }

#define CAMMOVEDIRTY()                                                  \
  if (mMoveType != eCameraMoveType_None && mvMove != ni::sVec3f::Zero()) \
    mnDirtyFlags |= CAMDIRTY_MOVE;

#define CAMVIEWDIRTY()                          \
  mnDirtyFlags |= CAMDIRTY_VIEW;

#define CAMPROJDIRTY()                          \
  mnDirtyFlags |= CAMDIRTY_PROJ;

static const ni::sVec2f _kvMinOrthoRectSize = { 10.0f, 10.0f };

class cCamera : public ni::cIUnknownImpl<ni::iCamera,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cCamera);

 public:
  ///////////////////////////////////////////////
  cCamera() : mFrustum(cGraphics::_CreateFrustum()) {
    ZeroMembers();
    mFrustum->SetNumPlanes(6);
    _SetMoveType(mMoveType);
  }

  ///////////////////////////////////////////////
  ~cCamera() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
    mProjection = ni::eCameraProjectionType_Perspective;
    mfAspect = 0.0f;
    mfFov = niRadf(45);
    mfNearPlane = 1.0f;
    mfFarPlane = 1e6f;
    mfOrthoSize = 100.0f;
    mmtxView = ni::sMatrixf::Identity();
    mmtxProj = ni::sMatrixf::Identity();
    mvMove = ni::sVec3f::Zero();
    mnDirtyFlags = CAMDIRTY_ALL;
    mnMoveMode = 0;
    mMoveType = eCameraMoveType_Fly;
    mvPos = ni::sVec3f::Zero();
    mvRayStart = ni::sVec3f::Zero();
    mvTarget = ni::sVec3f::ZAxis();
    mvTargetUp = ni::sVec3f::YAxis();
    mViewport = ni::sRectf(0.0f,0.0f,1.0f,1.0f);
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cCamera);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Copy(const iCamera* apSrc) {
    niPanicAssert(apSrc != NULL);
    const cCamera* pSrc = static_cast<const cCamera*>(apSrc);
    this->mProjection = pSrc->mProjection;
    this->mViewport = pSrc->mViewport;
    this->mfAspect = pSrc->mfAspect;
    this->mfFov = pSrc->mfFov;
    this->mfNearPlane = pSrc->mfNearPlane;
    this->mfFarPlane = pSrc->mfFarPlane;
    this->mvPos = pSrc->mvPos;
    this->mvTarget = pSrc->mvTarget;
    this->mvTargetUp = pSrc->mvTargetUp;
    this->mvMove = pSrc->mvMove;
    this->mnMoveMode = pSrc->mnMoveMode;
    this->mMoveType = pSrc->mMoveType;
    this->mFrustum->Copy(pSrc->mFrustum);
    this->mmtxView = pSrc->mmtxView;
    this->mmtxProj = pSrc->mmtxProj;
    this->mnDirtyFlags = pSrc->mnDirtyFlags;
  }

  ///////////////////////////////////////////////
  virtual iCamera* __stdcall Clone() const {
    cCamera* pNew = niNew cCamera();
    pNew->Copy(this);
    return pNew;
  }

  ///////////////////////////////////////////////
  virtual ni::sMatrixf __stdcall GetViewMatrix() const {
    CAMUPDATE();
    return mmtxView;
  }
  virtual ni::sMatrixf __stdcall GetProjectionMatrix() const {
    CAMUPDATE();
    return mmtxProj;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetViewMatrix(const sMatrixf& aViewMatrix) {
    mmtxView = aViewMatrix;
    CAMVIEWDIRTY();
  }
  virtual void __stdcall SetProjectionMatrix(const sMatrixf& aProjMatrix) {
    mmtxProj = aProjMatrix;
    CAMPROJDIRTY();
  }

  ///////////////////////////////////////////////
  virtual ni::iFrustum* __stdcall GetFrustum() const {
    CAMUPDATE();
    return mFrustum;
  }

  ///////////////////////////////////////////////
  void _UpdateCamera() {
    if (!mnDirtyFlags)
      return;

    ni::tU32 nNewDirtyFlags = 0;
    niFlagOn(mnDirtyFlags,CAMDIRTY_UPDATING);

    if ((mMoveType != eCameraMoveType_SetMatrix) &&
        (niFlagIs(mnDirtyFlags,CAMDIRTY_VIEW) || niFlagIs(mnDirtyFlags,CAMDIRTY_MOVE)))
    {
      if (niFlagIs(mnDirtyFlags,CAMDIRTY_VIEW)) {
        MatrixLookAtLH(mmtxView,mvPos,mvTarget,mvTargetUp);
      }

      if ((mMoveType != eCameraMoveType_None) && niFlagIs(mnDirtyFlags,CAMDIRTY_MOVE)) {
        ni::sVec3f vMoveDelta = ni::sVec3f::Zero();

        if (niFlagIs(mnMoveMode,CAMMOVE_ORBIT)) {
          ni::sVec3f F;
          MatrixGetForward(F,mmtxView);
          ni::sVec3f oldPos = mvPos;
          OrbitUp(mvMove.y);
          OrbitSidewards(mvMove.x);
          vMoveDelta = mvPos-oldPos;
          vMoveDelta += F*mvMove.z;
          mvPos = oldPos;
        }
        else {
          ni::sVec3f F,R,U;
          MatrixGetForward(F,mmtxView);
          MatrixGetRight(R,mmtxView);
          MatrixGetUp(U,mmtxView);
          vMoveDelta = R*mvMove.x + U*mvMove.y + F*mvMove.z;
        }
        mvPos += vMoveDelta;

        if (niFlagIs(mnMoveMode,CAMMOVE_ORBIT)) {
          niFlagOn(nNewDirtyFlags,CAMDIRTY_VIEW);
        }
        else if (niFlagIs(mnMoveMode,CAMMOVE_TARGET)) {
          mvTarget += vMoveDelta;
          niFlagOn(nNewDirtyFlags,CAMDIRTY_VIEW);
        }


        mvMove.Set(0.0f);

        MatrixSetProjectedTranslation(mmtxView,mvPos);
      }
    }

    if ((mProjection != eCameraProjectionType_SetMatrix) && niFlagIs(mnDirtyFlags,CAMDIRTY_PROJ)) {
      const ni::tF32 aspectRatio = GetAbsoluteAspect();
      if (mProjection == ni::eCameraProjectionType_Orthogonal) {
        MatrixOrthoLH(mmtxProj,mfOrthoSize*aspectRatio,mfOrthoSize,mfNearPlane,mfFarPlane);
      }
      else if (mProjection == ni::eCameraProjectionType_Perspective) {
        MatrixPerspectiveFovLH(mmtxProj, mfFov,
                               aspectRatio,
                               mfNearPlane, mfFarPlane);
      }
    }

    if (mnDirtyFlags) {
      mFrustum->ExtractPlanes(mmtxView*mmtxProj);
    }

    niFlagOff(mnDirtyFlags,CAMDIRTY_UPDATING);
    mnDirtyFlags = nNewDirtyFlags;
  }

  ///////////////////////////////////////////////
  virtual ni::sVec3f __stdcall GetRay(ni::tF32 afX, ni::tF32 afY, const ni::sRectf& aRect) {
    CAMUPDATE();
    ni::sRectf rect = aRect;
    const ni::tF64 aspectRatio = _ComputeAbsoluteAspect(mfAspect,aRect);
    if (mProjection == ni::eCameraProjectionType_Orthogonal) {
      // Perspective cameras
      ni::sVec3d pos, dest;
      ni::sMatrixd mtxV,mtxP,mtxVP;
      CopyArray(mtxV.ptr(),mmtxView.ptr(),16);
      // recompute perspective projection with safe clip planes for rays...
      MatrixOrthoLH(mtxP,(tF64)mfOrthoSize*aspectRatio,(tF64)mfOrthoSize,
                    (tF64)mfNearPlane,
                    (tF64)mfNearPlane+10000.0);
      mtxVP = mtxV*mtxP;
      ni::VecUnproject(pos,ni::Vec3<ni::tF64>(afX,afY,0.0),
                       (tF64)rect.GetLeft(),(tF64)rect.GetTop(),
                       (tF64)rect.GetWidth(),(tF64)rect.GetHeight(),
                       mtxVP);
      ni::VecUnproject(dest,ni::Vec3<ni::tF64>(afX,afY,10.0),
                       (tF64)rect.GetLeft(),(tF64)rect.GetTop(),
                       (tF64)rect.GetWidth(),(tF64)rect.GetHeight(),
                       mtxVP);
      dest = dest-pos;
      VecNormalize(dest);
      ni::sVec3f r;
      CopyArray(r.ptr(),dest.ptr(),3);
      CopyArray(mvRayStart.ptr(),pos.ptr(),3);
      // niDebugFmt(("... GETRAY(%s,%s,%s): %s -> %s", afX, afY, aRect, mvRayStart, r));
      return r;
    }
    else {
      mvRayStart = GetPosition();
      // Perspective cameras
      ni::sVec3d pos;
      CopyArray(pos.ptr(),mvRayStart.ptr(),3);
      ni::sVec3d dest;
      ni::sMatrixd mtxV,mtxP,mtxVP;
      CopyArray(mtxV.ptr(),mmtxView.ptr(),16);
      // CopyArray(mtxP.ptr(),mmtxProj.ptr(),16);
      // recompute perspective projection with safe clip planes for rays...
      MatrixPerspectiveFovLH(mtxP, (tF64)mfFov, aspectRatio,
                             (tF64)mfNearPlane,
                             (tF64)mfNearPlane+10000.0);
      mtxVP = mtxV*mtxP;
      ni::VecUnproject(dest,ni::Vec3<ni::tF64>(afX,afY,1.0),
                       (tF64)rect.GetLeft(),(tF64)rect.GetTop(),
                       (tF64)rect.GetWidth(),(tF64)rect.GetHeight(),
                       mtxVP);
      dest = dest-pos;
      VecNormalize(dest);
      ni::sVec3f r;
      CopyArray(r.ptr(),dest.ptr(),3);
      return r;
    }
  }

  ///////////////////////////////////////////////
  virtual ni::sVec3f __stdcall GetScreenPosition(const ni::sVec3f& avPos, const ni::sRectf& aRect) const {
    return ni::GetScreenPosition(avPos,mmtxView*mmtxProj,aRect);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFromWorldMatrix(const ni::sMatrixf& aMatrix, tBool abSetUp) {
    mvMove = ni::sVec3f::Zero();
    MatrixGetTranslation(mvPos,aMatrix);

    if (niFlagIs(mnMoveMode,CAMMOVE_TARGET)) {
      // Use double precision here, the inverse will degenerate in some cases if we dont
      sMatrixd dMtx, dMtxI;
      CopyArray(dMtx.ptr(),aMatrix.ptr(),16);
      tF64 d = 0;
      MatrixInverse(dMtxI,dMtx,&d);
      sVec3d F, U;
      MatrixGetForward(F,dMtxI);
      VecNormalize(F);
      MatrixGetUp(U,dMtxI);
      VecNormalize(U);
      // Compute the final target position
      CopyArray(mvTarget.ptr(),F.ptr(),3);
      mvTarget = mvPos + mvTarget;
      if (abSetUp)
        CopyArray(mvTargetUp.ptr(),U.ptr(),3);
    }

    CAMVIEWDIRTY();
  }
  virtual ni::sMatrixf __stdcall GetWorldMatrix() const {
    CAMUPDATE();
    sMatrixf mtx = sMatrixf::Identity();
    MatrixSetTranslation(mtx,GetPosition());
    sMatrixf mtxInverse;
    MatrixInverse(mtxInverse,mmtxView);
    sVec3f f,r,u;
    MatrixSetForward(mtx,VecNormalize(MatrixGetForward(f,mtxInverse)));
    MatrixSetRight(mtx,VecNormalize(MatrixGetRight(r,mtxInverse)));
    MatrixSetUp(mtx,VecNormalize(MatrixGetUp(u,mtxInverse)));
    return mtx;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetProjection(eCameraProjectionType aProjectionType) {
    if (mProjection == aProjectionType)
      return;
    mProjection = aProjectionType;
    CAMPROJDIRTY();
    CAMVIEWDIRTY();
  }
  virtual eCameraProjectionType __stdcall GetProjection() const {
    CAMUPDATE();
    return mProjection;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetViewport(const ni::sRectf& aRect) {
    if (mViewport == aRect) return;
    mViewport = aRect;
    ni::sVec2f center = mViewport.GetCenter();
    if (mViewport.GetWidth() < _kvMinOrthoRectSize.x) {
      mViewport.SetWidth(_kvMinOrthoRectSize.x);
    }
    if (mViewport.GetHeight() < _kvMinOrthoRectSize.y) {
      mViewport.SetHeight(_kvMinOrthoRectSize.y);
    }
    mViewport.SetCenter(center);
    CAMPROJDIRTY();
  }
  virtual ni::sRectf __stdcall GetViewport() const {
    return mViewport;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFov(ni::tF32 fFOV) {
    if (mfFov == fFOV) return;
    mfFov = fFOV;
    CAMPROJDIRTY();
  }
  virtual ni::tF32 __stdcall GetFov() const {
    return mfFov;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetAspect(ni::tF32 fAspect) {
    if (mfAspect == fAspect) return;
    mfAspect = fAspect;
    CAMPROJDIRTY();
  }
  virtual ni::tF32 __stdcall GetAspect() const {
    return mfAspect;
  }

  ///////////////////////////////////////////////
  static inline ni::tF32 __stdcall _ComputeAbsoluteAspect(const tF32 afAspect, const sRectf& aVP) {
    return (afAspect > 0) ? afAspect : ni::FDiv((ni::tF32)aVP.GetWidth(),(ni::tF32)aVP.GetHeight());
  }
  virtual ni::tF32 __stdcall GetAbsoluteAspect() const {
    return _ComputeAbsoluteAspect(mfAspect,mViewport);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetNearClipPlane(ni::tF32 afD) {
    if (afD == mfNearPlane) return;
    mfNearPlane = afD;
    CAMPROJDIRTY();
  }
  virtual ni::tF32 __stdcall GetNearClipPlane() const {
    return mfNearPlane;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetFarClipPlane(ni::tF32 afD) {
    afD = ni::Min(afD,1e6f);
    if (afD == mfFarPlane) return;
    mfFarPlane = afD;
    CAMPROJDIRTY();
  }
  virtual ni::tF32 __stdcall GetFarClipPlane() const {
    return mfFarPlane;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetOrthoSize(tF32 afSize) {
    mfOrthoSize = afSize;
    CAMPROJDIRTY();
  }
  virtual ni::tF32 __stdcall GetOrthoSize() const {
    return mfOrthoSize;
  }
  virtual sRectf __stdcall GetOrthogonalRect() const {
    sRectf r = sRectf(0,0,1,1);
    r.x = mvPos.x;
    r.y = mvPos.y;
    r.SetHeight(mfOrthoSize);
    r.SetWidth(mfOrthoSize*GetAbsoluteAspect());
    return r;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetPosition(const ni::sVec3f& vPos) {
    if (vPos == mvPos) return;
    if (niFlagIs(mnMoveMode,CAMMOVE_TARGET)) {
      mvTarget += vPos-mvPos;
    }
    mvPos = vPos;
    CAMVIEWDIRTY();
  }
  virtual ni::sVec3f __stdcall GetPosition() const {
    CAMUPDATE();
    return mvPos;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTarget(const ni::sVec3f& avTarget) {
    if (mvTarget == avTarget) return;
    mvTarget = avTarget;
    CAMVIEWDIRTY();
  }
  virtual ni::sVec3f __stdcall GetTarget() const {
    CAMUPDATE();
    return mvTarget;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTargetUp(const ni::sVec3f& avUp) {
    VecNormalize(mvTargetUp,avUp);
    CAMVIEWDIRTY();
  }
  virtual ni::sVec3f __stdcall GetTargetUp() const {
    CAMUPDATE();
    return mvTargetUp;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTargetDistance(ni::tF32 afDist) {
    SetTarget(mvPos + GetTargetDirection()*afDist);
  }
  virtual ni::tF32 __stdcall GetTargetDistance() const {
    CAMUPDATE();
    return ni::VecLength(mvTarget-mvPos);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetPositionDistance(ni::tF32 afDist) {
    SetPosition(mvTarget - GetTargetDirection()*afDist);
  }
  virtual ni::tF32 __stdcall GetPositionDistance() const {
    return GetTargetDistance();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetTargetDirection(const ni::sVec3f& avDir) {
    sVec3f nDir;
    VecNormalize(nDir,avDir);
    SetTarget(GetPosition()+(nDir*GetTargetDistance()));
  }
  virtual ni::sVec3f __stdcall GetTargetDirection() const {
    CAMUPDATE();
    sVec3f ret;
    return ni::VecNormalize(ret,mvTarget-mvPos);
  }

  ///////////////////////////////////////////////
  void __stdcall _SetMoveType(ni::eCameraMoveType aMode)
  {
    niFlagOff(mnMoveMode,CAMMOVE_ORBIT);
    niFlagOff(mnMoveMode,CAMMOVE_TARGET);
    switch (aMode) {
      default:
      case ni::eCameraMoveType_None:
      case ni::eCameraMoveType_Fly:
        niFlagOn(mnMoveMode,CAMMOVE_TARGET);
        break;
      case ni::eCameraMoveType_FlyFixedTarget:
        break;
      case ni::eCameraMoveType_Orbit:
        niFlagOn(mnMoveMode,CAMMOVE_ORBIT);
        break;
    }
    mvMove = ni::sVec3f::Zero();
    CAMVIEWDIRTY();
    CAMMOVEDIRTY();
    CAMUPDATE();
  }
  virtual void __stdcall SetMoveType(eCameraMoveType aMode) {
    if (aMode == mMoveType) return;
    mMoveType = aMode;
    _SetMoveType(mMoveType);
  }
  virtual eCameraMoveType __stdcall GetMoveType() const {
    return mMoveType;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall MoveSidewards(ni::tF32 delta) {
    if (delta == 0.0f)  return;
    mvMove.x += delta;
    CAMMOVEDIRTY();
  }
  virtual void __stdcall MoveUp(ni::tF32 delta) {
    if (delta == 0.0f)  return;
    mvMove.y += delta;
    CAMMOVEDIRTY();
  }
  virtual void __stdcall MoveForward(ni::tF32 delta) {
    if (delta == 0.0f)  return;
    mvMove.z += delta;
    CAMMOVEDIRTY();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetMove(const ni::sVec3f& avMove) {
    if (avMove == mvMove)  return;
    mvMove = avMove;
    CAMMOVEDIRTY();
  }
  virtual ni::sVec3f __stdcall GetMove() const {
    return mvMove;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall RotateTarget(ni::sVec3f avAxis, ni::tF32 afAngle) {
    if (afAngle == 0.0f) return;
    CAMUPDATE();
    ni::sMatrixf mtxR;
    MatrixRotationAxis(mtxR,VecNormalize(avAxis),afAngle);
    mvTarget -= mvPos;  // to camera space
    VecTransformNormal(mvTarget,mvTarget,mtxR);
    //VecTransformNormal(mvTargetUp,mvTargetUp,mtxR);
    mvTarget += mvPos;  // to world space
    CAMVIEWDIRTY();
  }
  virtual void __stdcall RotateTargetUp(ni::sVec3f avAxis, ni::tF32 afAngle) {
    if (afAngle == 0.0f) return;
    CAMUPDATE();
    ni::sMatrixf mtxR;
    MatrixRotationAxis(mtxR,avAxis,afAngle);
    VecTransformNormal(mvTargetUp,mvTargetUp,mtxR);
    CAMVIEWDIRTY();
  }

  ///////////////////////////////////////////////
  virtual void __stdcall AddYaw(ni::tF32 a) {
    if (a == 0.0f) return;
    RotateTarget(ni::sVec3f::YAxis(),a);
  }
  virtual void __stdcall AddPitch(ni::tF32 a) {
    if (a == 0.0f) return;
    ni::sVec3f vAxis = GetRight();
    vAxis.y = 0.0f;
    RotateTarget(vAxis,a);
  }
  virtual void __stdcall AddRoll(ni::tF32 a) {
    if (a == 0.0f) return;
    ni::sVec3f vAxis = GetForward();
    vAxis.y = 0.0f;
    RotateTargetUp(vAxis,a);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall OrbitUp(ni::tF32 afA) {
    VecOrbitPitch(mvPos,mvTarget,mvPos,-afA);
    CAMVIEWDIRTY();
  }
  virtual void __stdcall OrbitSidewards(ni::tF32 afA) {
    VecOrbitYaw(mvPos,mvTarget,mvPos,afA);
    CAMVIEWDIRTY();
  }

  ///////////////////////////////////////////////
  virtual ni::sVec3f __stdcall GetForward() const {
    CAMUPDATE();
    ni::sVec3f r;
    return ni::MatrixGetForward(r,mmtxView);
  }
  virtual ni::sVec3f __stdcall GetRight() const {
    CAMUPDATE();
    ni::sVec3f r;
    return ni::MatrixGetRight(r,mmtxView);
  }
  virtual ni::sVec3f __stdcall GetUp() const {
    CAMUPDATE();
    ni::sVec3f r;
    return ni::MatrixGetUp(r,mmtxView);
  }

  ///////////////////////////////////////////////
  virtual ni::sVec3f __stdcall GetRayStart() const {
    return mvRayStart;
  }

 private:
  ni::eCameraProjectionType mProjection;
  ni::sRectf                mViewport;
  ni::tF32                  mfAspect, mfFov;
  ni::tF32                  mfNearPlane;
  ni::tF32                  mfFarPlane;
  ni::tF32                  mfOrthoSize;
  ni::sVec3f                mvMove;
  ni::tU8                   mnMoveMode;
  ni::eCameraMoveType       mMoveType;
  ni::tU32                  mnDirtyFlags;
  ni::Nonnull<ni::iFrustum> mFrustum;
  ni::sMatrixf              mmtxView;
  ni::sMatrixf              mmtxProj;
  ni::sVec3f                mvPos;
  ni::sVec3f                mvTarget;
  ni::sVec3f                mvTargetUp;
  ni::sVec3f                mvRayStart;

  niEndClass(cCamera);
};

niExportFunc(iUnknown*) New_niUI_Camera(const Var&, const Var&) {
  return niNew cCamera();
}
