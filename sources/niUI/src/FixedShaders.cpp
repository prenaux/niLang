// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#include "FixedShaders.h"
#include <niLang/Math/MathMatrix.h>

struct sFixedShaderConstant;

#define GET_FPS_VEC4_PARAMS                     \
  sVec4f& aOut,                                 \
    sFixedShaderConstant* c,                    \
    iDrawOperation* apDrawOp,                   \
    iGraphicsContext* apContext,                \
    const sMaterialDesc* apMat,                 \
    const sFixedStatesDesc* apFixedStates,      \
    const tBool isFlippedRT

#define GET_FPS_MAT4_PARAMS                     \
  sMatrixf& aOut,                               \
    sFixedShaderConstant* c,                    \
    iDrawOperation* apDrawOp,                   \
    iGraphicsContext* apContext,                \
    const sMaterialDesc* apMat,                 \
    const sFixedStatesDesc* apFixedStates,      \
    const tBool isFlippedRT

typedef void (*tpfnGetFpsVec4Const)(GET_FPS_VEC4_PARAMS);
typedef void (*tpfnGetFpsMat4Const)(GET_FPS_MAT4_PARAMS);

struct sFixedShaderConstant {
  tHStringPtr       hspName;
  eShaderRegisterType     regType;
  tU32          nMaxSize;
  tU32          nItemIndex;   // sub material / light index
  tU32          nSubItemIndex;  // texture index for material
  tU32          nConstantBufferIndex;
  tpfnGetFpsVec4Const     getVector;
  tpfnGetFpsMat4Const     getMatrix;
  sFixedShaderConstant() : getVector(NULL), getMatrix(NULL) {}
};

static tBool FPS_GetVector(GET_FPS_VEC4_PARAMS)
{
  if (c->getVector) {
    c->getVector(
        aOut,c,
        apDrawOp,
        apContext,
        apMat,
        apFixedStates,
        isFlippedRT);
    return eTrue;
  }
  return eFalse;
}

static tBool FPS_GetMatrix(GET_FPS_MAT4_PARAMS)
{
  if (c->getMatrix) {
    c->getMatrix(
        aOut,c,
        apDrawOp,
        apContext,
        apMat,
        apFixedStates,
        isFlippedRT);
    return eTrue;
  }
  return eFalse;
}

_HDecl(Error);  // error texture

_HDecl(fpsCameraViewMatrix);
_HDecl(fpsCameraInvViewMatrix);
_HDecl(fpsCameraProjectionMatrix);
_HDecl(fpsCameraInvProjectionMatrix);
_HDecl(fpsCameraViewProjectionMatrix);
_HDecl(fpsCameraInvViewProjectionMatrix);

_HDecl(fpsWorldMatrix);
_HDecl(fpsViewMatrix);
_HDecl(fpsProjectionMatrix);
_HDecl(fpsInvWorldMatrix);
_HDecl(fpsWorldViewMatrix);
_HDecl(fpsInvWorldViewMatrix);
_HDecl(fpsInvViewMatrix);
_HDecl(fpsViewProjectionMatrix);
_HDecl(fpsInvViewProjectionMatrix);
_HDecl(fpsWorldViewProjectionMatrix);
_HDecl(fpsInvWorldViewProjectionMatrix);
_HDecl(fpsInvProjectionMatrix);

_HDecl(fpsTextureMatrix0);
_HDecl(fpsTextureMatrix1);
_HDecl(fpsWorldScale);
_HDecl(fpsCameraViewPosition);
_HDecl(fpsCameraViewForward);
_HDecl(fpsCameraViewUp);
_HDecl(fpsCameraViewRight);
_HDecl(fpsCameraClipPlanes);
_HDecl(fpsViewPosition);
_HDecl(fpsViewForward);
_HDecl(fpsViewUp);
_HDecl(fpsViewRight);
_HDecl(fpsWorldCameraViewPosition);
_HDecl(fpsWorldCameraViewForward);
_HDecl(fpsWorldCameraViewUp);
_HDecl(fpsWorldCameraViewRight);
_HDecl(fpsWorldViewPosition);
_HDecl(fpsWorldViewForward);
_HDecl(fpsWorldViewUp);
_HDecl(fpsWorldViewRight);
_HDecl(fpsScreenRes);
_HDecl(fpsViewport);
_HDecl(fpsViewportRes);
_HDecl(fpsViewportTexCoos);
_HDecl(fpsViewportClampTexCoos);
_HDecl(fpsTime);

_HDecl(fpsExprMatrix0);
_HDecl(fpsExprMatrix1);
_HDecl(fpsExprTex0);
_HDecl(fpsExprTex1);

#define HD_MAP_CONSTS(NAME)                     \
  _HDecl(fpsMaterial##NAME##Color);             \
  _HDecl(fpsMaterial##NAME##Res);

HD_MAP_CONSTS(Base)
HD_MAP_CONSTS(Opacity)
HD_MAP_CONSTS(Bump)
HD_MAP_CONSTS(Specular)
HD_MAP_CONSTS(Environment)
HD_MAP_CONSTS(Ambient)
HD_MAP_CONSTS(Emissive)
HD_MAP_CONSTS(Refraction)
HD_MAP_CONSTS(T0)
HD_MAP_CONSTS(T1)
HD_MAP_CONSTS(T2)
HD_MAP_CONSTS(T3)
HD_MAP_CONSTS(T4)
HD_MAP_CONSTS(T5)
HD_MAP_CONSTS(T6)
HD_MAP_CONSTS(T7)

enum eFPSMatrix
{
  eFPSMatrix_CameraView = 0,
  eFPSMatrix_CameraProj = 1,
  eFPSMatrix_World = 2,
  eFPSMatrix_View = 3,
  eFPSMatrix_Proj = 4,
  eFPSMatrix_CameraViewProj = 5,
  eFPSMatrix_CameraInvViewProj = 6,
  eFPSMatrix_CameraInvView = 7,
  eFPSMatrix_CameraInvProj = 8,
  eFPSMatrix_WorldView = 9,
  eFPSMatrix_WorldViewProj = 10,
  eFPSMatrix_ViewProj = 11,
  eFPSMatrix_InvWorldView = 12,
  eFPSMatrix_InvWorldViewProj = 13,
  eFPSMatrix_InvViewProj = 14,
  eFPSMatrix_InvWorld = 15,
  eFPSMatrix_InvView = 16,
  eFPSMatrix_InvProj = 17,
  eFPSMatrix_Last = 18
};

#define FIXEDSTATE(STATE) apFixedStates->m##STATE

#define FIXEDSTATE_PROJ(STATE)                                          \
  (isFlippedRT ? MatrixFlipProjectionY(apFixedStates->m##STATE) : apFixedStates->m##STATE)

#define EVALEXPR(MAT,WHICH,TYPE,DEFAULT)                                \
  ((MAT->mptrExpressions[eMaterialExpression_##WHICH].IsOK()) ?         \
   (MAT->mptrExpressions[eMaterialExpression_##WHICH]->Eval().IsOK() ?         \
    MAT->mptrExpressions[eMaterialExpression_##WHICH]->GetEvalResult()->Get##TYPE() : (DEFAULT)) : \
   (DEFAULT))

#define FPS_GET_DECL(NAME) get_##NAME

#define FPS_MATRIX(NAME) FPS_GetMatrix(eFPSMatrix_##NAME,apDrawOp,apFixedStates,isFlippedRT)

static tIntPtr _fpsMulCache[eFPSMatrix_Last] = {0};

static __forceinline sMatrixf _MatrixMul(eFPSMatrix aMatrix, const sMatrixf& A, const sMatrixf& B) {
  return A * B;
}
static __forceinline sMatrixf _MatrixMul(eFPSMatrix aMatrix, const sMatrixf& A, const sMatrixf& B, const sMatrixf& C) {
  return A * B * C;
}
static __forceinline sMatrixf _MatrixInv(eFPSMatrix aMatrix, const sMatrixf& A) {
  sMatrixf r;
  return MatrixInverse(r,A);
}
static __forceinline sMatrixf _MatrixMulInv(eFPSMatrix aMatrix, const sMatrixf& A, const sMatrixf& B) {
  sMatrixf r, t = A * B;
  return MatrixInverse(r,t);
}
static __forceinline sMatrixf _MatrixMulInv(eFPSMatrix aMatrix, const sMatrixf& A, const sMatrixf& B, const sMatrixf& C) {
  sMatrixf r, t = A * B * C;
  return MatrixInverse(r,t);
}

///////////////////////////////////////////////
static inline sMatrixf FPS_GetMatrix(
    eFPSMatrix aMatrix,
    iDrawOperation* apDrawOp,
    const sFixedStatesDesc* apFixedStates,
    const tBool isFlippedRT)
{
  switch (aMatrix)
  {
    case eFPSMatrix_CameraView:
      return FIXEDSTATE(CameraViewMatrix);
    case eFPSMatrix_CameraProj:
      return FIXEDSTATE_PROJ(CameraProjectionMatrix);
    case eFPSMatrix_View:
      return FIXEDSTATE(ViewMatrix);
    case eFPSMatrix_Proj:
      return FIXEDSTATE_PROJ(ProjectionMatrix);
    case eFPSMatrix_World:
      return apDrawOp->GetMatrix();
    case eFPSMatrix_CameraViewProj:
      return _MatrixMul(aMatrix,FIXEDSTATE(CameraViewMatrix),FIXEDSTATE_PROJ(CameraProjectionMatrix));
    case eFPSMatrix_CameraInvViewProj:
      return _MatrixMulInv(aMatrix,FIXEDSTATE(CameraViewMatrix),FIXEDSTATE_PROJ(CameraProjectionMatrix));
    case eFPSMatrix_CameraInvView:
      return _MatrixInv(aMatrix,FIXEDSTATE(CameraViewMatrix));
    case eFPSMatrix_CameraInvProj:
      return _MatrixInv(aMatrix,FIXEDSTATE_PROJ(CameraProjectionMatrix));
    case eFPSMatrix_WorldView:
      return _MatrixMul(aMatrix,apDrawOp->GetMatrix(),FIXEDSTATE(ViewMatrix));
    case eFPSMatrix_WorldViewProj:
      return _MatrixMul(aMatrix,apDrawOp->GetMatrix(),FIXEDSTATE(ViewMatrix),FIXEDSTATE_PROJ(ProjectionMatrix));
    case eFPSMatrix_ViewProj:
      return _MatrixMul(aMatrix,FIXEDSTATE(ViewMatrix),FIXEDSTATE_PROJ(ProjectionMatrix));
    case eFPSMatrix_InvWorld:
      return _MatrixInv(aMatrix,apDrawOp->GetMatrix());
    case eFPSMatrix_InvView:
      return _MatrixInv(aMatrix,FIXEDSTATE(ViewMatrix));
    case eFPSMatrix_InvProj:
      return _MatrixInv(aMatrix,FIXEDSTATE_PROJ(ProjectionMatrix));
    case eFPSMatrix_InvViewProj:
      return _MatrixMulInv(aMatrix,FIXEDSTATE(ViewMatrix),FIXEDSTATE_PROJ(ProjectionMatrix));
    case eFPSMatrix_InvWorldView:
      return _MatrixMulInv(aMatrix,apDrawOp->GetMatrix(),FIXEDSTATE(ViewMatrix));
    case eFPSMatrix_InvWorldViewProj:
      return _MatrixMulInv(aMatrix,apDrawOp->GetMatrix(),FIXEDSTATE(ViewMatrix),FIXEDSTATE_PROJ(ProjectionMatrix));
  }
  return sMatrixf::Identity();
}

static void FPS_GET_DECL(fpsMaterialChannelColorDiffuseModulate)(GET_FPS_VEC4_PARAMS) {
  if (apMat->mChannels[c->nSubItemIndex].mTexture.IsOK()) {
    const tU32 flags = apMat->mFlags;
    if (flags & eMaterialFlags_DiffuseModulate) {
      aOut = apMat->mChannels[c->nSubItemIndex].mColor;
      // DiffuseModulate of the alpha is only active for translucent
      // material. So that its visually coherent on platforms that do alpha
      // blending of the backbuffer, like WebGL canvas on Safari.
      if (!(flags & eMaterialFlags_Translucent)) {
        aOut.w = 1;
      }
    }
    else {
      aOut = sVec4f::One();
    }
  }
  else {
    aOut = apMat->mChannels[c->nSubItemIndex].mColor;
  }
}

static void FPS_GET_DECL(fpsMaterialChannelColorPassthrough)(GET_FPS_VEC4_PARAMS) {
  aOut = apMat->mChannels[c->nSubItemIndex].mColor;
}

static void FPS_GET_DECL(fpsMaterialChannelRes)(GET_FPS_VEC4_PARAMS) {
  sVec4f res = sVec4f::One();
  iTexture* pTex = apMat->mChannels[c->nSubItemIndex].mTexture;
  if (pTex) {
    res.x = tF32(pTex->GetWidth());
    res.x = res.x > 0.0f ? 1.0f/res.x : 1.0f;
    res.y = tF32(pTex->GetHeight());
    res.y = res.y > 0.0f ? 1.0f/res.y : 1.0f;
    if (pTex->GetDepth()) {
      res.z = tF32(pTex->GetDepth());
      res.z = res.z > 0.0f ? 1.0f/res.z : 1.0f;
    }
    else {
      res.z = -res.x;
      res.w = -res.y;
    }
  }
  aOut = res;
}

static void FPS_GET_DECL(fpsTime)(GET_FPS_VEC4_PARAMS) {
  const tF64 time = ni::GetLang()->GetTotalFrameTime();
  const tF64 frameTime = ni::GetLang()->GetFrameTime();
  const sVec4f vTime = {
    (ni::tF32)time,
    (ni::tF32)time,
    ni::Sin((ni::tF32)time),
    (ni::tF32)frameTime
  };
  aOut = vTime;
}

static void FPS_GET_DECL(fpsExprTex0)(GET_FPS_VEC4_PARAMS) {
  aOut = EVALEXPR(apMat,Tex0,Vec4,sVec4f::Zero());
}
static void FPS_GET_DECL(fpsExprTex1)(GET_FPS_VEC4_PARAMS) {
  aOut = EVALEXPR(apMat,Tex1,Vec4,sVec4f::Zero());
}
static void FPS_GET_DECL(fpsWorldScale)(GET_FPS_VEC4_PARAMS) {
  sVec3f v; VecTransformNormal(
      v,VecNormalize(v,sVec3f::One()),FPS_MATRIX(World));
  // xyz: scale.xyz
  // w: normal sign, use to flip normals when doing double sided rendering
  aOut = Vec4(v.x,v.y,v.z,1.0f);
}
static void FPS_GET_DECL(fpsWorldViewPosition)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  VecTransformNormal(
      t,
      MatrixGetTranslation(t,FPS_MATRIX(View)),
      FPS_MATRIX(InvWorld));
  aOut = Vec4(t);
}
static void FPS_GET_DECL(fpsWorldViewForward)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  VecTransformNormal(
      t,
      MatrixGetForward(t,FPS_MATRIX(View)),
      FPS_MATRIX(InvWorld));
  aOut = Vec4(t);
}
static void FPS_GET_DECL(fpsWorldViewUp)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  VecTransformNormal(
      t,
      MatrixGetUp(t,FPS_MATRIX(View)),
      FPS_MATRIX(InvWorld));
  aOut = Vec4(t);
}
static void FPS_GET_DECL(fpsWorldViewRight)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  VecTransformNormal(
      t,
      MatrixGetRight(t,FPS_MATRIX(View)),
      FPS_MATRIX(InvWorld));
  aOut = Vec4(t);
}
static void FPS_GET_DECL(fpsWorldCameraViewPosition)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  VecTransformNormal(
      t,
      MatrixGetTranslation(t,FPS_MATRIX(CameraView)),
      FPS_MATRIX(InvWorld));
  aOut = Vec4(t);
}
static void FPS_GET_DECL(fpsWorldCameraViewForward)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  VecTransformNormal(
      t,
      MatrixGetForward(t,FPS_MATRIX(CameraView)),
      FPS_MATRIX(InvWorld));
  aOut = Vec4(t);
}
static void FPS_GET_DECL(fpsWorldCameraViewUp)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  VecTransformNormal(
      t,
      MatrixGetUp(t,FPS_MATRIX(CameraView)),
      FPS_MATRIX(InvWorld));
  aOut = Vec4(t);
}
static void FPS_GET_DECL(fpsWorldCameraViewRight)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  VecTransformNormal(
      t,
      MatrixGetRight(t,FPS_MATRIX(CameraView)),
      FPS_MATRIX(InvWorld));
  aOut = Vec4(t);
}
static void FPS_GET_DECL(fpsViewPosition)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  aOut = Vec4(MatrixGetProjectedTranslation(t,FPS_MATRIX(View)));
}
static void FPS_GET_DECL(fpsViewForward)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  aOut = Vec4(MatrixGetForward(t,FPS_MATRIX(View)));
}
static void FPS_GET_DECL(fpsViewUp)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  aOut = Vec4(MatrixGetUp(t,FPS_MATRIX(View)));
}
static void FPS_GET_DECL(fpsViewRight)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  aOut = Vec4(MatrixGetRight(t,FPS_MATRIX(View)));
}
static void FPS_GET_DECL(fpsCameraViewPosition)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  aOut = Vec4(MatrixGetProjectedTranslation(t,FPS_MATRIX(CameraView)));
}
static void FPS_GET_DECL(fpsCameraViewForward)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  aOut = Vec4(MatrixGetForward(t,FPS_MATRIX(CameraView)));
}
static void FPS_GET_DECL(fpsCameraViewUp)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  aOut = Vec4(MatrixGetUp(t,FPS_MATRIX(CameraView)));
}
static void FPS_GET_DECL(fpsCameraViewRight)(GET_FPS_VEC4_PARAMS) {
  sVec3f t;
  aOut = Vec4(MatrixGetRight(t,FPS_MATRIX(CameraView)));
}
static void FPS_GET_DECL(fpsCameraClipPlanes)(GET_FPS_VEC4_PARAMS) {
  sMatrixf mtxVP = FPS_MATRIX(CameraViewProj);
  sVec4f n, f;
  // Near clipping plane
  n.x = -(mtxVP._14 + mtxVP._13);
  n.y = -(mtxVP._24 + mtxVP._23);
  n.z = -(mtxVP._34 + mtxVP._33);
  n.w = -(mtxVP._44 + mtxVP._43);
  PlaneNormalize(n);
  // Far clipping plane
  f.x = -(mtxVP._14 - mtxVP._13);
  f.y = -(mtxVP._24 - mtxVP._23);
  f.z = -(mtxVP._34 - mtxVP._33);
  f.w = -(mtxVP._44 - mtxVP._43);
  PlaneNormalize(f);
  sVec4f v;
  v.x = n.w;
  v.y = -f.w;
  v.z = ni::FInvert(v.x);
  v.w = ni::FInvert(v.y);
  aOut = v;
  //        niDebugFmt((_A("CLIPPLANES: near=%f, far=%f (%s)\n"),
  //              v.x,v.y,_ASZ(v)));
}
static void FPS_GET_DECL(fpsScreenRes)(GET_FPS_VEC4_PARAMS) {
  iTexture* pTex = apContext->GetRenderTarget(0);
  niAssert(niIsOK(pTex));
  tF32 fW = tF32(pTex->GetWidth());
  tF32 fH = tF32(pTex->GetHeight());
  aOut = Vec4(1.0f/fW,1.0f/fH,fW,fH);
}
static void FPS_GET_DECL(fpsViewport)(GET_FPS_VEC4_PARAMS) {
  sRectf vp = apContext->GetViewport().ToFloat();
  aOut = Vec4(vp.Left(),vp.Top(),vp.GetWidth(),vp.GetHeight());
}
static void FPS_GET_DECL(fpsViewportTexCoos)(GET_FPS_VEC4_PARAMS) {
  iTexture* pTex = apContext->GetRenderTarget(0);
  niAssert(niIsOK(pTex));
  tF32 fW = tF32(pTex->GetWidth());
  tF32 fH = tF32(pTex->GetHeight());
  sRectf vp = apContext->GetViewport().ToFloat();
  aOut = Vec4(vp.x/fW,vp.y/fH,vp.z/fW,vp.w/fH);
}
static void FPS_GET_DECL(fpsViewportClampTexCoos)(GET_FPS_VEC4_PARAMS) {
  iTexture* pTex = apContext->GetRenderTarget(0);
  niAssert(niIsOK(pTex));
  tF32 fW = tF32(pTex->GetWidth());
  tF32 fH = tF32(pTex->GetHeight());
  sRectf vp = apContext->GetViewport().ToFloat();
  sVec2f border = { 1.0f/fW, 1.0f/fH };
  aOut = Vec4(vp.x/fW+border.x,vp.y/fH+border.y,vp.z/fW-border.x,vp.w/fH-border.y);
}
static void FPS_GET_DECL(fpsViewportRes)(GET_FPS_VEC4_PARAMS) {
  iTexture* pTex = apContext->GetRenderTarget(0);
  niAssert(niIsOK(pTex));
  sRectf vp = apContext->GetViewport().ToFloat();
  tF32 fRTW = tF32(pTex->GetWidth());
  tF32 fRTH = tF32(pTex->GetHeight());
  tF32 fVPX = vp.x*(1.0f/fRTW);
  tF32 fVPY = vp.y*(1.0f/fRTH);
  tF32 fVPW = vp.GetWidth();
  tF32 fVPH = vp.GetHeight();
  aOut = Vec4(
      (fVPW/fRTW),
      (fVPH/fRTH),
      fVPX+0.5f/fRTW,
      fVPY+0.5f/fRTH
                 );
}
static void FPS_GET_DECL(fpsTextureMatrix0)(GET_FPS_MAT4_PARAMS) {
  if (apMat->mptrExpressions[eMaterialExpression_Tex0].IsOK()) {
    aOut = EVALEXPR(apMat,Tex0,Matrix,sMatrixf::Identity());
  }
  else {
    aOut = sMatrixf::Identity();
  }
}
static void FPS_GET_DECL(fpsTextureMatrix1)(GET_FPS_MAT4_PARAMS) {
  if (apMat->mptrExpressions[eMaterialExpression_Tex1].IsOK()) {
    aOut = EVALEXPR(apMat,Tex1,Matrix,sMatrixf::Identity());
  }
  else {
    aOut = sMatrixf::Identity();
  }
}

static void FPS_GET_DECL(fpsCameraViewMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(CameraView);
}
static void FPS_GET_DECL(fpsCameraInvViewMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(CameraInvView);
}
static void FPS_GET_DECL(fpsCameraProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(CameraProj);
}
static void FPS_GET_DECL(fpsCameraInvProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(CameraInvProj);
}
static void FPS_GET_DECL(fpsCameraViewProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(CameraViewProj);
}
static void FPS_GET_DECL(fpsCameraInvViewProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(CameraInvViewProj);
}
static void FPS_GET_DECL(fpsWorldMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(World);
}
static void FPS_GET_DECL(fpsViewMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(View);
}
static void FPS_GET_DECL(fpsProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(Proj);
}
static void FPS_GET_DECL(fpsInvWorldMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(InvWorld);
}
static void FPS_GET_DECL(fpsWorldViewMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(WorldView);
}
static void FPS_GET_DECL(fpsInvWorldViewMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(InvWorldView);
}
static void FPS_GET_DECL(fpsInvViewMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(InvView);
}
static void FPS_GET_DECL(fpsViewProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(ViewProj);
}
static void FPS_GET_DECL(fpsInvViewProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(InvViewProj);
}
static void FPS_GET_DECL(fpsWorldViewProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(WorldViewProj);
}
static void FPS_GET_DECL(fpsInvWorldViewProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(InvWorldViewProj);
}
static void FPS_GET_DECL(fpsInvProjectionMatrix)(GET_FPS_MAT4_PARAMS) {
  aOut = FPS_MATRIX(InvProj);
}
static void FPS_GET_DECL(fpsExprMatrix0)(GET_FPS_MAT4_PARAMS) {
  aOut = EVALEXPR(apMat,Matrix0,Matrix,sMatrixf::Identity());
}
static void FPS_GET_DECL(fpsExprMatrix1)(GET_FPS_MAT4_PARAMS) {
  aOut = EVALEXPR(apMat,Matrix1,Matrix,sMatrixf::Identity());
}

struct sFixedShaderConstants : public Impl_HeapAlloc {
  sFixedShaderConstants();
  ~sFixedShaderConstants();
  tBool InitializeShaderConstants(iShaderConstants* apBuffer) const;

  typedef astl::hstring_hash_map<sFixedShaderConstant> tMap;
  tMap mmapConstants;
};

static __forceinline sFixedShaderConstants* _GetFPSConstants() {
  static sFixedShaderConstants* _fpsConstants = NULL;
  if (!_fpsConstants) {
    _fpsConstants = niNew sFixedShaderConstants();
  }
  return _fpsConstants;
}

static sFixedShaderConstant* FPS_GetConstant(iHString* ahspName)
{
  const sFixedShaderConstants* fpsConsts = _GetFPSConstants();
  sFixedShaderConstants::tMap::const_iterator cit =
      fpsConsts->mmapConstants.find(ahspName);
  if (cit == fpsConsts->mmapConstants.end())
    return NULL;
  return const_cast<sFixedShaderConstant*>(&cit->second);
}

#define DEF_FPSCONST(NAME,REGTYPE,MAXSIZE,CONSTTYPE,ITEMINDEX)  { \
    sFixedShaderConstant c;                                               \
    c.hspName = _HC(NAME);                                        \
    c.regType = eShaderRegisterType_Const##REGTYPE;               \
    c.nMaxSize = MAXSIZE;                                         \
    c.nItemIndex = ITEMINDEX;                                     \
    c.nSubItemIndex = 0;                                          \
    c.get##CONSTTYPE = get_##NAME;                                \
    astl::upsert(mmapConstants,c.hspName,c);                 \
  }

#define DEF_FPSCONST2(NAME,BASENAME,REGTYPE,MAXSIZE,CONSTTYPE,ITEMINDEX)  { \
    sFixedShaderConstant c;                                                     \
    c.hspName = _H(#NAME);                                              \
    c.regType = eShaderRegisterType_Const##REGTYPE;                     \
    c.nMaxSize = MAXSIZE;                                               \
    c.nItemIndex = ITEMINDEX;                                           \
    c.nSubItemIndex = 0;                                                \
    c.get##CONSTTYPE = get_##BASENAME;                                  \
    astl::upsert(mmapConstants,c.hspName,c);                       \
  }

#define DEF_FPSCONST3(NAME,BASENAME,REGTYPE,MAXSIZE,CONSTTYPE,ITEMINDEX,SUBITEMINDEX,GETTER)  { \
    sFixedShaderConstant c;                                                     \
    c.hspName = _H(#NAME);                                              \
    c.regType = eShaderRegisterType_Const##REGTYPE;                     \
    c.nMaxSize = MAXSIZE;                                               \
    c.nItemIndex = ITEMINDEX;                                           \
    c.nSubItemIndex = SUBITEMINDEX;                                     \
    c.getVector = get_##GETTER;                                         \
    astl::upsert(mmapConstants,c.hspName,c);                       \
  }

///////////////////////////////////////////////
sFixedShaderConstants::sFixedShaderConstants() {
  DEF_FPSCONST(fpsCameraViewMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsCameraInvViewMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsCameraProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsCameraInvProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsCameraViewProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsCameraInvViewProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsWorldMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsViewMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsInvWorldMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsWorldViewMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsInvWorldViewMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsInvViewMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsViewProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsInvViewProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsWorldViewProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsInvWorldViewProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsInvProjectionMatrix,Float,4,Matrix,0);
  DEF_FPSCONST(fpsTextureMatrix0,Float,4,Matrix,0);
  DEF_FPSCONST(fpsTextureMatrix1,Float,4,Matrix,1);
  DEF_FPSCONST(fpsWorldScale,Float,1,Vector,0);
  DEF_FPSCONST(fpsCameraViewPosition,Float,1,Vector,0);
  DEF_FPSCONST(fpsCameraViewForward,Float,1,Vector,0);
  DEF_FPSCONST(fpsCameraViewUp,Float,1,Vector,0);
  DEF_FPSCONST(fpsCameraViewRight,Float,1,Vector,0);
  DEF_FPSCONST(fpsCameraClipPlanes,Float,1,Vector,0);
  DEF_FPSCONST(fpsViewPosition,Float,1,Vector,0);
  DEF_FPSCONST(fpsViewForward,Float,1,Vector,0);
  DEF_FPSCONST(fpsViewUp,Float,1,Vector,0);
  DEF_FPSCONST(fpsViewRight,Float,1,Vector,0);
  DEF_FPSCONST(fpsWorldCameraViewPosition,Float,1,Vector,0);
  DEF_FPSCONST(fpsWorldCameraViewForward,Float,1,Vector,0);
  DEF_FPSCONST(fpsWorldCameraViewUp,Float,1,Vector,0);
  DEF_FPSCONST(fpsWorldCameraViewRight,Float,1,Vector,0);
  DEF_FPSCONST(fpsWorldViewPosition,Float,1,Vector,0);
  DEF_FPSCONST(fpsWorldViewForward,Float,1,Vector,0);
  DEF_FPSCONST(fpsWorldViewUp,Float,1,Vector,0);
  DEF_FPSCONST(fpsWorldViewRight,Float,1,Vector,0);
  DEF_FPSCONST(fpsScreenRes,Float,1,Vector,0);
  DEF_FPSCONST(fpsViewport,Float,1,Vector,0);
  DEF_FPSCONST(fpsViewportRes,Float,1,Vector,0);
  DEF_FPSCONST(fpsViewportTexCoos,Float,1,Vector,0);
  DEF_FPSCONST(fpsViewportClampTexCoos,Float,1,Vector,0);
  DEF_FPSCONST(fpsTime,Float,1,Vector,0);

  DEF_FPSCONST(fpsExprMatrix0,Float,4,Matrix,0);
  DEF_FPSCONST(fpsExprMatrix1,Float,4,Matrix,0);
  DEF_FPSCONST(fpsExprTex0,Float,1,Vector,0);
  DEF_FPSCONST(fpsExprTex1,Float,1,Vector,0);

#define DEF_MAP_CONSTS(NAME,IDX,COLORMODE)                              \
  DEF_FPSCONST3(fpsMaterial##NAME##Color##IDX,fpsMaterial##NAME##Color,Float,1,MaterialColor,IDX,eMaterialChannel_##NAME,fpsMaterialChannelColor##COLORMODE); \
  DEF_FPSCONST3(fpsMaterial##NAME##Res##IDX,fpsMaterial##NAME##Res,Float,1,MaterialVector,IDX,eMaterialChannel_##NAME,fpsMaterialChannelRes);

#define DEF_MATERIAL_CONSTS(IDX)                \
  DEF_MAP_CONSTS(Base,IDX,DiffuseModulate);     \
  DEF_MAP_CONSTS(Opacity,IDX,Passthrough);      \
  DEF_MAP_CONSTS(Bump,IDX,Passthrough);         \
  DEF_MAP_CONSTS(Specular,IDX,Passthrough);     \
  DEF_MAP_CONSTS(Environment,IDX,Passthrough);  \
  DEF_MAP_CONSTS(Ambient,IDX,Passthrough);      \
  DEF_MAP_CONSTS(Emissive,IDX,Passthrough);     \
  DEF_MAP_CONSTS(Refraction,IDX,Passthrough);   \
  DEF_MAP_CONSTS(T0,IDX,Passthrough);           \
  DEF_MAP_CONSTS(T1,IDX,Passthrough);           \
  DEF_MAP_CONSTS(T2,IDX,Passthrough);           \
  DEF_MAP_CONSTS(T3,IDX,Passthrough);           \
  DEF_MAP_CONSTS(T4,IDX,Passthrough);           \
  DEF_MAP_CONSTS(T5,IDX,Passthrough);           \
  DEF_MAP_CONSTS(T6,IDX,Passthrough);           \
  DEF_MAP_CONSTS(T7,IDX,Passthrough);

  DEF_MATERIAL_CONSTS(0);
  // DEF_MATERIAL_CONSTS(1);
  // DEF_MATERIAL_CONSTS(2);
  // DEF_MATERIAL_CONSTS(3);
}

///////////////////////////////////////////////
sFixedShaderConstants::~sFixedShaderConstants() {
}

///////////////////////////////////////////////
tBool sFixedShaderConstants::InitializeShaderConstants(iShaderConstants* apBuffer) const {
  for (sFixedShaderConstants::tMap::const_iterator it = mmapConstants.begin(); it != mmapConstants.end(); ++it) {
    const sFixedShaderConstant& c = it->second;
    niConstCast(sFixedShaderConstant&,c).nConstantBufferIndex = apBuffer->AddConstant(c.hspName,c.regType,c.nMaxSize);
  }
  return eTrue;
}

///////////////////////////////////////////////
static __forceinline tBool _UpdateShaderConstants(
  iShaderConstants* apDestConsts,
  iGraphicsContext* apContext,
  iDrawOperation* apDrawOp,
  const tBool isFlippedRT)
{
  niProfileBlock(FPSConstants_UpdateShaderConstantBuffer);

  const sMaterialDesc* pMaterial = NULL;
  if (apDrawOp->GetMaterial()) {
    pMaterial = (const sMaterialDesc*)apDrawOp->GetMaterial()->GetDescStructPtr();
  }

  const sFixedStatesDesc* pFixedStates = (const sFixedStatesDesc*)apContext->GetFixedStates()->GetDescStructPtr();

  {
    niProfileBlock(FPSConstants_Update_Uniforms);
    const sShaderConstantsDesc* shaderConstDesc = (const sShaderConstantsDesc*)apDestConsts->GetDescStructPtr();
    // Update the requested constants
    niLoopit(sShaderConstantsDesc::tConstMap::const_iterator,it,shaderConstDesc->mmapConstants) {
      sFixedShaderConstant* c = FPS_GetConstant(it->first);
      if (!c)
        continue;
      if (c->getMatrix) {
        sMatrixf v;
        c->getMatrix(
            v,c,
            apDrawOp,
            apContext,
            pMaterial,
            pFixedStates,
            isFlippedRT);
        apDestConsts->SetFloatMatrix(it->second,v);
      }
      else if (c->getVector) {
        sVec4f v;
        c->getVector(
            v,c,
            apDrawOp,
            apContext,
            pMaterial,
            pFixedStates,
            isFlippedRT);
        apDestConsts->SetFloat(it->second,v);
      }
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) FixedShaders_UpdateConstants(iGraphicsContext* apContext, iShaderConstants* apBuffer, iDrawOperation* apDrawOp)
{
  niCheck(niIsOK(apContext),eFalse);
  niCheck(niIsOK(apBuffer),eFalse);
  const tBool isFlippedRT = niFlagIs(apContext->GetRenderTarget(0)->GetFlags(),eTextureFlags_RTFlipped);
  return _UpdateShaderConstants(apBuffer,apContext,apDrawOp,isFlippedRT);
}
