// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "Font.h"
#include "API/niUI/Utils/AABB.h"
#include <niLang/Math/MathMatrix.h>
#include <niLang/Math/MathTriangle.h>
#include <niLang/STL/sort.h>
#include "API/niUI/IImage.h"

#define TRACE_BUFFER_CACHE(X) // niDebugFmt(X)
#include "API/niUI/Utils/BufferCache.h"

// #define USE_BUFFER_CACHE_RING_BUFFER
// #define USE_BUFFER_CACHE_BASE_VERTEX_INDEX

namespace ni {
template struct BufferCacheVertex<tVertexCanvas>;
}

static const sVec2f _vTLTex = {0,0};
static const sVec2f _vTRTex = {1,0};
static const sVec2f _vBRTex = {1,1};
static const sVec2f _vBLTex = {0,1};

#ifdef USE_BUFFER_CACHE_RING_BUFFER
#define GET_BUFFER_CACHE_VERTEX() auto& bufferCacheVertex = mptrBufferCacheVertex[mnCurrentBufferCache];
#define GET_BUFFER_CACHE_INDEX() auto& bufferCacheIndex = mptrBufferCacheIndex[mnCurrentBufferCache];
static const tU32 _knRingBufferSize = 1024;
#else
#define GET_BUFFER_CACHE_VERTEX() auto& bufferCacheVertex = mptrBufferCacheVertex;
#define GET_BUFFER_CACHE_INDEX() auto& bufferCacheIndex = mptrBufferCacheIndex;
#endif

static const tU32 _knBufferCacheInitSize = 1024;

enum CANVAS_FLAGS {
  CANVAS_FLAGS_BakeTransform = niBit(1),
  CANVAS_FLAGS_LineConstantScreenSize = niBit(2),
  CANVAS_FLAGS_Blitting = niBit(3),
  CANVAS_FLAGS_BBRightSet = niBit(4),
  CANVAS_FLAGS_BBUpSet = niBit(5),
};

struct sGraphicsCanvasStates {
  Ptr<iMaterial> mptrMaterial;
  tU32 mnColorA;
  tF32 mfLineSize;
  tBillboardModeFlags mBBMode;
  sVec3f mvBBUp, mvBBRight, mvBBCenter;
  sMatrixf mMatrix;
  sVec3f   mvNormal;
  tU32     mnFlags;
  sGraphicsCanvasStates() {
    mnFlags = CANVAS_FLAGS_LineConstantScreenSize;
    mnColorA = 0xFFFFFFFF;
    mvNormal = sVec3f::YAxis();
    mfLineSize = 1.0f;
    mptrMaterial = NULL;
    mMatrix = sMatrixf::Identity();
    mBBMode = eBillboardModeFlags_Disabled;
    mvBBUp = sVec3f::YAxis();
    mvBBRight = sVec3f::XAxis();
    mvBBCenter = sVec3f::Zero();
  }
};

//--------------------------------------------------------------------------------------------
//
//  Line quad
//
//--------------------------------------------------------------------------------------------

static __forceinline void ProjectLineQuad(sVec3f view[4], sVec3f proj[4], const sMatrixf& amtxProj, const sRectf& aVP) {
  niLoop(j,4) {
    sVec4f t;
    VecProjectRHW(t,view[j],amtxProj,aVP);
    proj[j] = (sVec3f&)t;
  }
}

struct sLine {
  tU32 axis;
  tF32 dot;
  tF32 projArea;
  sVec3f view[4];
  sVec3f proj[4];
  sVec3f lineNormal;
  sVec3f projNormal;
  void ComputeProjArea(const sMatrixf& amtxProj, const sRectf& aVP) {
    niLoop(j,4) {
      sVec4f t;
      VecProjectRHW(t,view[j],amtxProj,aVP);
      proj[j] = (sVec3f&)t;
    }
    projArea = Abs(TriangleAreaAndNormal(projNormal,proj[0],proj[1],proj[2]));
  }
};

static inline tBool ComputeLineScreenQuad(
    const sVec3f& avStart, const sVec3f& avEnd, tF32 afStartSize, tF32 afEndSize,
    tPtr apVerts,
    tU32 anStride,
    tBool abScreenSize,
    const tF32 afMinScreenSize,
    const sMatrixf& amtxWorldView,
    const sMatrixf& amtxInvWorldView,
    const sMatrixf& amtxProj,
    const sMatrixf& amtxWVP,
    const sRectf& aVP,
    const sPlanef* apClipPlanes,
    tU32 anNumClipPlanes,
    tBool abCullDiscard,
    tBool abClipForSizeOnly)
{
  tBool bRet = eTrue;
  sVec3f vClippedStart = avStart;
  sVec3f vClippedEnd = avEnd;

  if (apClipPlanes && anNumClipPlanes) {
    niLoop(i,anNumClipPlanes) {
      // use start/end size as epsilon so the line is visible when extruded as-well
      eClassify clStart = ClassifyPoint(apClipPlanes[i],vClippedStart,afStartSize);
      eClassify clEnd = ClassifyPoint(apClipPlanes[i],vClippedEnd,afEndSize);
      if (clStart == eClassify_Back && clEnd == eClassify_Back) {
        if (abCullDiscard)
          return eFalse;
        bRet = eFalse;
      }
      if (clStart == eClassify_Back && clEnd == eClassify_Front) {
        PlaneIntersectLine(apClipPlanes[i],vClippedStart,vClippedEnd,&vClippedStart);
      }
      else if (clStart == eClassify_Front && clEnd == eClassify_Back) {
        PlaneIntersectLine(apClipPlanes[i],vClippedStart,vClippedEnd,&vClippedEnd);
      }
    }
  }

  sVec3f* pTL = ((sVec3f*)(apVerts+(0*anStride)));
  sVec3f* pTR = ((sVec3f*)(apVerts+(1*anStride)));
  sVec3f* pBR = ((sVec3f*)(apVerts+(2*anStride)));
  sVec3f* pBL = ((sVec3f*)(apVerts+(3*anStride)));

  // Compute size factors
  tF32 begSize = afStartSize;
  tF32 endSize = afEndSize;
  if (abScreenSize) {
    begSize = ComputePixelWorldSize(ni::Max(afMinScreenSize,afStartSize),
                                    vClippedStart,amtxWorldView,amtxProj,aVP);
    endSize = ComputePixelWorldSize(ni::Max(afMinScreenSize,afEndSize),
                                    vClippedEnd,amtxWorldView,amtxProj,aVP);
  }

  sVec3f vPos1, vPos2;
  if (abClipForSizeOnly) {
    VecTransformCoord(vPos1,avStart,amtxWorldView);
    VecTransformCoord(vPos2,avEnd,amtxWorldView);
  }
  else {
    VecTransformCoord(vPos1,vClippedStart,amtxWorldView);
    VecTransformCoord(vPos2,vClippedEnd,amtxWorldView);
  }

  // used as a radius (so twice...)
  begSize *= 0.5f;
  endSize *= 0.5f;

  const sVec3f vAxis[3] = {
    {1,0,0},
    {0,1,0},
    {0,0,1}
  };
  sVec3f vDir = vPos1 - vPos2;
  VecNormalize(vDir);

  sLine lines[3];
  niLoop(i,niCountOf(lines)) {
    sLine& l = lines[i];
    l.axis = i;
    l.dot = Abs(VecDot(vDir,vAxis[i]));
    VecCross(l.lineNormal,vDir,vAxis[i]);
    l.view[0] = vPos1 + ( l.lineNormal * begSize); // TL
    l.view[1] = vPos2 + ( l.lineNormal * endSize); // TR
    l.view[2] = vPos2 + (-l.lineNormal * endSize); // BR
    l.view[3] = vPos1 + (-l.lineNormal * begSize); // BL
    l.ComputeProjArea(amtxProj,aVP);
  }
  astl::stable_sort(lines+0,lines+3, [](const sLine& a, const sLine& b) {
    return a.projArea > b.projArea;
  });

  {
    sLine& l = lines[0];
    const sVec3f* view = l.view;
    VecTransformCoord(*pTL,view[0],amtxInvWorldView);
    VecTransformCoord(*pTR,view[1],amtxInvWorldView);
    VecTransformCoord(*pBR,view[2],amtxInvWorldView);
    VecTransformCoord(*pBL,view[3],amtxInvWorldView);
  }
  return bRet;
}

enum class ePolygonClass {
  NotConvex,
  NotConvexDegenerate,
  ConvexDegenerate,
  ConvexCCW,
  ConvexCW
};

// Determine the type of polygon
template <typename T>
__forceinline ePolygonClass ClassifyPolygon2(const T* pVert, int nvert)
{
#define CONVEX_CMP(delta)                                           \
  ( (delta[0] > 0) ? -1 : /* x coord diff, second pt > first pt */  \
    (delta[0] < 0) ?  1 : /* x coord diff, second pt < first pt */  \
    (delta[1] > 0) ? -1 : /* x coord same, second pt > first pt */  \
    (delta[1] < 0) ?  1 : /* x coord same, second pt > first pt */  \
    0 )     /* second pt equals first point */

#define CONVEX_POINT_DELTA(delta, pprev, pcur)                       \
  /* Given a previous point 'pprev', read a new point into 'pcur' */  \
      /* and return delta in 'delta'.           */                    \
      pcur = pVert[iread++].ptr();                                    \
      delta[0] = pcur[0] - pprev[0];                                  \
      delta[1] = pcur[1] - pprev[1];                                  \

#define CONVEX_CROSS(p, q) ((p[0] * q[1]) - (p[1] * q[0]));

#define CONVEX_CHECK_TRIPLE                                       \
  if ((thisDir = CONVEX_CMP(dcur)) == -curDir) {                  \
    ++dirChanges;                                                 \
    /* The following line will optimize for polygons that are  */ \
    /* not convex because of classification condition 4,       */ \
    /* otherwise, this will only slow down the classification. */ \
    /* if ( dirChanges > 2 ) return NotConvex;                 */ \
  }                                                               \
  curDir = thisDir;                                               \
  cross = CONVEX_CROSS(dprev, dcur);                              \
  if (cross > 0) {                                                \
    if (angleSign == -1)                                          \
      return ePolygonClass::NotConvex;                            \
    angleSign = 1;                                                \
  }                                                               \
  else if (cross < 0) {                                           \
    if (angleSign == 1)                                           \
      return ePolygonClass::NotConvex;                            \
    angleSign = -1;                                               \
  }                                                               \
  pSecond = pThird;   /* Remember ptr to current point. */        \
  dprev[0] = dcur[0]; /* Remember current delta.    */            \
  dprev[1] = dcur[1];                                             \

  int curDir, thisDir, dirChanges = 0, angleSign = 0;
  const tF32 *pSecond, *pThird, *pSaveSecond;
  tF32 dprev[2], dcur[2], cross;

  // Get different point, return if less than 3 diff points.
  if (nvert < 3) {
    return ePolygonClass::ConvexDegenerate;
  }

  int iread = 1;
  for(;;) {
    CONVEX_POINT_DELTA(dprev, pVert[0], pSecond);
    if (dprev[0] || dprev[1])
      break;
    // Check if out of points. Check here to avoid slowing down cases without
    // repeated points.
    if (iread >= nvert)
      return ePolygonClass::ConvexDegenerate;
  }

  pSaveSecond = pSecond;
  curDir = CONVEX_CMP(dprev); // Find initial direction
  while ( iread < nvert ) {
    // Get different point, break if no more points
    CONVEX_POINT_DELTA(dcur, pSecond, pThird );
    if (dcur[0] == 0.0 && dcur[1] == 0.0)
      continue;

    CONVEX_CHECK_TRIPLE; /// Check current three points
  }

  // Must check for direction changes from last vertex back to first
  pThird = pVert[0].ptr(); // Prepare for 'CONVEX_CHECK_TRIPLE'
  dcur[0] = pThird[0] - pSecond[0];
  dcur[1] = pThird[1] - pSecond[1];
  if (CONVEX_CMP(dcur)) {
    CONVEX_CHECK_TRIPLE;
  }

  // and check for direction changes back to second vertex
  dcur[0] = pSaveSecond[0] - pSecond[0];
  dcur[1] = pSaveSecond[1] - pSecond[1];
  CONVEX_CHECK_TRIPLE; // Don't care about 'pThird' now

  // Decide on polygon type given accumulated status
  if (dirChanges > 2)
    return (angleSign ? ePolygonClass::NotConvex : ePolygonClass::NotConvexDegenerate);

  if (angleSign > 0)
    return ePolygonClass::ConvexCCW;
  if (angleSign < 0)
    return ePolygonClass::ConvexCW;
  return ePolygonClass::ConvexDegenerate;

#undef CONVEX_CMP
#undef CONVEX_POINT_DELTA
#undef CONVEX_CROSS
#undef CONVEX_CHECK_TRIPLE
}

static __forceinline void EnsureConvexLineQuad(sVec3f view[4], sVec3f proj[4], const sMatrixf& amtxProj, const sRectf& aVP) {
  ProjectLineQuad(view,proj,amtxProj,aVP);
  ePolygonClass cl = ClassifyPolygon2(proj,4);
  if (cl != ePolygonClass::ConvexCW && cl != ePolygonClass::ConvexCCW) {
    ni::Swap(proj[0],proj[3]);
    ni::Swap(view[0],view[3]);
  }
}

///////////////////////////////////////////////
static __forceinline tBool DoComputeScreenQuad(
  iGraphicsContext* apContext,
  const sRectf& aVP,
  const sMatrixf& aWorldMatrix,
  const sVec3f& avStart, const sVec3f& avEnd,
  tF32 afStartSize, tF32 afEndSize,
  tVertexCanvas* apVerts, tBool abScreenSize,
  tBool abIsStrip, const sVec3f* apStripStart)
{
  const sMatrixf mtxView = apContext->GetFixedStates()->GetViewMatrix();
  const sMatrixf mtxProj = apContext->GetFixedStates()->GetProjectionMatrix();

  sMatrixf mtxWorldView = aWorldMatrix*mtxView;
  sMatrixf mtxInvWorldView;
  MatrixInverse(mtxInvWorldView,mtxWorldView);

  sMatrixf mtxWVP = mtxWorldView*mtxProj;
  sPlanef vPlanes[6];
  ExtractFrustumPlanes(vPlanes,mtxWVP);

  if (abIsStrip) {
    tBool bVisible = ComputeLineScreenQuad(
      avStart, avEnd, afStartSize, afEndSize,
      tPtr(apVerts), sizeof(*apVerts),
      abScreenSize,
      1.5f,
      mtxWorldView,
      mtxInvWorldView,
      mtxProj,
      mtxWVP,
      aVP,
      vPlanes,6,
      eFalse,
      eTrue);
    if (apStripStart) {
      // make sure the quad is convex...
      apVerts[0].pos = apStripStart[0];
      apVerts[3].pos = apStripStart[1];
      sVec3f view[4];
      niLoop(i,4) { view[i] = apVerts[i].pos; }
      sVec3f proj[4];
      ProjectLineQuad(view,proj,mtxWVP,aVP);
      EnsureConvexLineQuad(view,proj,mtxWVP,aVP);
      niLoop(i,4) { apVerts[i].pos = view[i]; }
    }
    return bVisible;
  }
  else
  {
    return ComputeLineScreenQuad(
      avStart, avEnd, afStartSize, afEndSize,
      tPtr(apVerts), sizeof(*apVerts),
      abScreenSize,
      1.5f,
      mtxWorldView,
      mtxInvWorldView,
      mtxProj,
      mtxWVP,
      aVP,
      vPlanes,niCountOf(vPlanes),
      eTrue,
      eFalse);
  }
}

static __forceinline void _GetCirclePointXZ(sVec3f& aOut, const sVec3f& avCenter, const sVec2f& avRadius, tF32 afCurrent) {
  aOut.x = avCenter.x + (ni::Cos(afCurrent) * avRadius.x);
  aOut.z = avCenter.z + (ni::Sin(afCurrent) * avRadius.y);
  aOut.y = avCenter.y;
}

static __forceinline void _GetCirclePointYZ(sVec3f& aOut, const sVec3f& avCenter, const sVec2f& avRadius, tF32 afCurrent) {
  aOut.y = avCenter.y + (ni::Cos(afCurrent) * avRadius.y);
  aOut.z = avCenter.z + (ni::Sin(afCurrent) * avRadius.x);
  aOut.x = avCenter.x;
}

static __forceinline void _GetCirclePointXY(sVec3f& aOut, const sVec3f& avCenter, const sVec2f& avRadius, tF32 afCurrent) {
  aOut.x = avCenter.x + (ni::Cos(afCurrent) * avRadius.x);
  aOut.y = avCenter.y + (ni::Sin(afCurrent) * avRadius.y);
  aOut.z = avCenter.z;
}

//--------------------------------------------------------------------------------------------
//
//  VGPathDraw
//
//--------------------------------------------------------------------------------------------
#define VG_SS_DEFAULT_FILTER eCompiledStates_SS_PointClamp

struct sCanvasVGPathTesselatedRenderer : public ImplRC<iVGPathTesselatedRenderer> {
  iCanvas* _canvas; // raw pointer, owned by the Canvas

  Ptr<iVGStyle> mptrVGStyle;

  Ptr<iVGTransform>       mVGTransforms[eVGTransform_Last];

  Ptr<iMaterial>          mptrVGMaterial;
  tIntPtr                 mhSS_MirrorPoint;
  tIntPtr                 mhSS_MirrorBilinear;
  tIntPtr                 mhSS_WhiteBorderPoint;
  tIntPtr                 mhSS_WhiteBorderBilinear;

  tIntPtr mnBeginAddPath_PrevImageSig;
  tBool mbAddPathPolygons_TexGen;
  tU32 mnAddPathPolygons_VertexColor;
  sVec2f mvAddPathPolygons_TexGen_Scale;
  sVec2f mvAddPathPolygons_TexGen_Translation;

  sCanvasVGPathTesselatedRenderer(iCanvas* apCanvas) : _canvas(apCanvas) {
    iGraphics* g = apCanvas->GetGraphicsContext()->GetGraphics();

    mptrVGStyle = g->CreateVGStyle();
    for (tU32 i = 0; i < eVGTransform_Last; ++i) {
      mVGTransforms[i] = CreateVGTransform();
    }

    // Material
    mptrVGMaterial = g->CreateMaterial();
    mptrVGMaterial->SetRasterizerStates(eCompiledStates_RS_NoCullingFilledScissor);
    mptrVGMaterial->SetDepthStencilStates(eCompiledStates_DS_NoDepthTest);
    mptrVGMaterial->SetFlags(
        mptrVGMaterial->GetFlags()|
        eMaterialFlags_NoLighting|
        eMaterialFlags_DoubleSided|
        eMaterialFlags_Vertex);
    mptrVGMaterial->SetChannelSamplerStates(
        eMaterialChannel_Base,VG_SS_DEFAULT_FILTER);

    Ptr<iSamplerStates> ss = g->CreateSamplerStates();
    ss->SetFilter(eSamplerFilter_Point);
    ss->SetWrapS(eSamplerWrap_Mirror);
    ss->SetWrapT(eSamplerWrap_Mirror);
    ss->SetWrapR(eSamplerWrap_Mirror);
    mhSS_MirrorPoint = g->CompileSamplerStates(ss);

    ss->SetFilter(eSamplerFilter_Smooth);
    ss->SetWrapS(eSamplerWrap_Mirror);
    ss->SetWrapT(eSamplerWrap_Mirror);
    ss->SetWrapR(eSamplerWrap_Mirror);
    mhSS_MirrorBilinear = g->CompileSamplerStates(ss);

    ss->SetFilter(eSamplerFilter_Point);
    ss->SetWrapS(eSamplerWrap_Border);
    ss->SetWrapT(eSamplerWrap_Border);
    ss->SetWrapR(eSamplerWrap_Border);
    ss->SetBorderColor(sColor4f::White());
    mhSS_WhiteBorderPoint = g->CompileSamplerStates(ss);

    ss->SetFilter(eSamplerFilter_Smooth);
    ss->SetWrapS(eSamplerWrap_Border);
    ss->SetWrapT(eSamplerWrap_Border);
    ss->SetWrapR(eSamplerWrap_Border);
    ss->SetBorderColor(sColor4f::White());
    mhSS_WhiteBorderBilinear = g->CompileSamplerStates(ss);
  }

  //! Called when begining to render a path.
  virtual void __stdcall BeginAddPath(const iVGStyle* apStyle, tBool abStroke) {
    iVGPaint* apPaint = (abStroke)?apStyle->GetStrokePaint():apStyle->GetFillPaint();
    tF32 fOpacity = apStyle->GetOpacity();
    mbAddPathPolygons_TexGen = eFalse;

    switch (apPaint->GetType()) {
    case eVGPaintType_Gradient:
    case eVGPaintType_Image:
      {
        mbAddPathPolygons_TexGen = eTrue;
        mnAddPathPolygons_VertexColor = ~0;
        const iVGTransform* pTransform = mVGTransforms[
          abStroke?eVGTransform_StrokePaint:eVGTransform_FillPaint];
        Ptr<iVGImage> ptrImage;

        sVec2f vTrans = {0,0};
        sVec2f vScale = {1,1};
        eVGWrapType wrapType;
        eVGImageFilter imageFilter;
        if (apPaint->GetType() == eVGPaintType_Gradient) {
          QPtr<iVGPaintGradient> pPaintGradient(apPaint);
          ptrImage = pPaintGradient->GetGradientImage();
          wrapType = pPaintGradient->GetWrapType();
          imageFilter = eVGImageFilter_Bilinear;
        }
        else {
          const iVGPaintImage* pPaintImage = niStaticCast(const iVGPaintImage*,apPaint);
          ptrImage = pPaintImage->GetImage();
          wrapType = pPaintImage->GetWrapType();
          imageFilter = pPaintImage->GetFilterType();
        }

        const tIntPtr nImageSig = (tIntPtr)ptrImage.ptr();
        if (mnBeginAddPath_PrevImageSig != nImageSig) {
          _canvas->Flush();
          mnBeginAddPath_PrevImageSig = nImageSig;
        }

        mvAddPathPolygons_TexGen_Scale.x = (1.0f/tF32(ptrImage->GetWidth()))*vScale.x;
        mvAddPathPolygons_TexGen_Scale.y = (1.0f/tF32(ptrImage->GetHeight()))*vScale.y;
        mvAddPathPolygons_TexGen_Translation = vTrans;

        mptrVGMaterial->SetChannelColor(eMaterialChannel_Base,Vec4f(1,1,1,fOpacity));

        tHandle hSS = 0;
        if (wrapType == eVGWrapType_Pad)  {
          //                     sColor4f backColor = apPaint->GetColor();
          //                     ss->SetBorderColor(backColor);
          hSS = (imageFilter==eVGImageFilter_Point)?
              mhSS_WhiteBorderPoint:mhSS_WhiteBorderBilinear;
        }
        else if (wrapType == eVGWrapType_Mirror)  {
          hSS = (imageFilter==eVGImageFilter_Point)?
              mhSS_MirrorPoint:mhSS_MirrorBilinear;
        }
        else if (wrapType == eVGWrapType_Clamp)  {
          hSS = (imageFilter==eVGImageFilter_Point)?
              eCompiledStates_SS_PointClamp:eCompiledStates_SS_SmoothClamp;
        }
        else /*if (wrapType == eVGWrapType_Repeat)*/  {
          hSS = (imageFilter==eVGImageFilter_Point)?
              eCompiledStates_SS_PointRepeat:eCompiledStates_SS_SmoothRepeat;
        }
        mptrVGMaterial->SetChannelSamplerStates(eMaterialChannel_Base,hSS);

        Ptr<iTexture> ptrTex = ptrImage->GrabTexture(eVGImageUsage_Source,sRecti::Null());
        mptrVGMaterial->SetChannelTexture(eMaterialChannel_Base,ptrTex);
        mptrVGMaterial->SetBlendMode(eBlendMode_Translucent);
        _canvas->SetMaterial(mptrVGMaterial);
        break;
      }
    default:
    case eVGPaintType_Solid:
      {
        if (mnBeginAddPath_PrevImageSig != 0) {
          _canvas->Flush();
          mnBeginAddPath_PrevImageSig = 0;
        }

        sColor4f vertexColor = (abStroke) ?
            apStyle->GetStrokeColor4() :
            apStyle->GetFillColor4();
        vertexColor.w *= apStyle->GetOpacity();

#if 1
        // we have the clear the ChannelTexture if there is any
        mptrVGMaterial->SetChannelTexture(eMaterialChannel_Base, NULL);
        mptrVGMaterial->SetBlendMode(eBlendMode_Translucent);
        _canvas->SetMaterial(mptrVGMaterial);
#else
        _IMSetMaterial(NULL,ni::FuzzyEqual(vertexColor.w,1.0f,0.001f) ?
                       eBlendMode_NoBlending : eBlendMode_Translucent);
#endif

        mnAddPathPolygons_VertexColor = ULColorBuild(vertexColor);
        break;
      }
    }
  }

  //! Called when end adding a path.
  virtual void __stdcall EndAddPath(const iVGStyle* apStyle, tBool abStroke) {
  }

  //! Called to request the approximation scale.
  virtual tF32 __stdcall GetPathApproximationScale(const iVGStyle* apStyle) const {
    return apStyle->GetTesselatorApproximationScale();
  }

  //! Called to add the path's polygons.
  virtual void __stdcall AddPathPolygons(iVGPolygonTesselator* apTess, const iVGStyle* apStyle, tBool abStroke) {
    const tVec2fCVec* pVerts = apTess->GetTesselatedVertices();
    if (pVerts && !pVerts->empty()) {
      _canvas->SetColorA(mnAddPathPolygons_VertexColor);
      if (mbAddPathPolygons_TexGen) {
        agg::trans_affine paintTrans = AGGGetTransform(
            mVGTransforms[abStroke?eVGTransform_StrokePaint:eVGTransform_FillPaint].ptr());

        // fit image into the bounding box
        sVec2f first = pVerts->GetFirst().mVec2f;
        tF32 min_x, max_x, min_y, max_y;
        min_x = max_x = first.x;
        min_y = max_y = first.y;
        astl::vector<sVec2f> v_transformed(pVerts->GetSize());
        niLoop(i, pVerts->GetSize()) {
          sVec2f v = pVerts->Get(i).mVec2f;
          paintTrans.transform(&v.x,&v.y);
          v_transformed[i] = v;

          min_x = Min(min_x, v.x);
          max_x = Max(max_x, v.x);

          min_y = Min(min_y, v.y);
          max_y = Max(max_y, v.y);
        }

        tF32 width = max_x - min_x;
        tF32 height = max_y - min_y;

        for (const auto& v : v_transformed) {
          tF32 tx = (v.x - min_x) / width;
          tF32 ty = (v.y - min_y) / height;
          _canvas->VertexPT(Vec3f(v.x,v.y,0.0f),Vec2f(tx,ty));
        }
      }
      else {
        for (tVec2fCVec::const_iterator it = pVerts->begin(); it != pVerts->end(); ++it) {
          _canvas->VertexP(Vec3f(it->x,it->y,0.0f));
        }
      }
    }
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
class cCanvasGraphics : public ImplRC<iCanvas,eImplFlags_Default>
{
 public:
  cCanvasGraphics(iGraphics* apGraphics, iGraphicsContext* apContext, iMaterial* apResetMaterial)
  {
#ifdef USE_BUFFER_CACHE_RING_BUFFER
    niLog(Info,"cCanvasGraphics USE_BUFFER_CACHE_RING_BUFFER");
#else
    niLog(Info,"cCanvasGraphics NO USE_BUFFER_CACHE_RING_BUFFER");
#endif
#ifdef USE_BUFFER_CACHE_BASE_VERTEX_INDEX
    niLog(Info,"cCanvasGraphics USE_BUFFER_CACHE_BASE_VERTEX_INDEX");
#else
    niLog(Info,"cCanvasGraphics NO USE_BUFFER_CACHE_BASE_VERTEX_INDEX");
#endif

    mptrGraphics = apGraphics;
    mptrContext = apContext;
    mfContentsScale = 1.0f;

    if (!niIsOK(mptrContext->GetFixedStates())) {
      mptrContext = NULL;
      niError(_A("Invalid context fixed states."));
      return;
    }

    iGraphics* g = mptrContext->GetGraphics();

    // Material
    if (niIsOK(apResetMaterial)) {
      mptrResetMaterial = apResetMaterial;
    }
    else {
      mptrResetMaterial = g->CreateMaterial();
      mptrResetMaterial->SetName(_H("CanvasDefaultMaterial"));
      mptrResetMaterial->SetBlendMode(eBlendMode_NoBlending);
      mptrResetMaterial->SetRasterizerStates(eCompiledStates_RS_NoCullingFilledScissor);
      mptrResetMaterial->SetDepthStencilStates(eCompiledStates_DS_NoDepthTest);
      mptrResetMaterial->SetFlags(eMaterialFlags_NoLighting|eMaterialFlags_DoubleSided|eMaterialFlags_Vertex);
      mptrResetMaterial->SetChannelSamplerStates(eMaterialChannel_Base, eCompiledStates_SS_SmoothClamp);
      mptrResetMaterial->SetChannelColor(eMaterialChannel_Base, sVec4f::White());
    }
    mptrDefaultMaterial = g->CreateMaterial();

    // Set the default states
    _SetDefaultStates();

    // Initialize the draw op
    mptrDrawOp = mptrContext->GetGraphics()->CreateDrawOperation();

    // Begin a batch...
    _BeginNextBatch();
  }

  ~cCanvasGraphics() {
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall IsOK() const {
    return mptrContext.IsOK();
  }

  ///////////////////////////////////////////////
  iGraphicsContext* __stdcall GetGraphicsContext() const {
    return mptrContext;
  }

  ///////////////////////////////////////////////
  void _SetDefaultStates() {
    mStates = sGraphicsCanvasStates();
    *((sMaterialDesc*)mptrDefaultMaterial->GetDescStructPtr()) = *((sMaterialDesc*)mptrResetMaterial->GetDescStructPtr());
    mStates.mptrMaterial = mptrDefaultMaterial;
  }
  void __stdcall ResetStates() {
    this->Flush();
    _SetDefaultStates();
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Flush() {
    if (_HasVertices()) {
      GET_BUFFER_CACHE_VERTEX();
      GET_BUFFER_CACHE_INDEX();
      const sVec2i verts = bufferCacheVertex->Update();
      const sVec2i inds = bufferCacheIndex->Update();
      if (!verts.y || !inds.y) {
        return eFalse;
      }
      mptrDrawOp->SetVertexArray(bufferCacheVertex->mptrVA);
      mptrDrawOp->SetIndexArray(bufferCacheIndex->mptrIA);
      mptrDrawOp->SetFirstIndex(inds.x);
      mptrDrawOp->SetNumIndices(inds.y);
#ifdef USE_BUFFER_CACHE_BASE_VERTEX_INDEX
      mptrDrawOp->SetBaseVertexIndex(verts.x);
#endif
      mptrDrawOp->SetMaterial(mStates.mptrMaterial.IsOK() ? mStates.mptrMaterial : mptrDefaultMaterial);
      mptrDrawOp->SetMatrix(mStates.mMatrix);
      mptrContext->DrawOperation(mptrDrawOp);
    }
    _BeginNextBatch();
    return eTrue;
  }

  ///////////////////////////////////////////////
  void _SetDefaultMaterial(iTexture* apTex, eBlendMode aBlendMode, tIntPtr aSamplerStates) {
    if (!aSamplerStates) {
      aSamplerStates = mptrResetMaterial->GetChannelSamplerStates(eMaterialChannel_Base);
    }
    iMaterial* m = mStates.mptrMaterial;
    sMaterialDesc* pDefaultMatDesc = (sMaterialDesc*)mptrDefaultMaterial->GetDescStructPtr();
    if (m != mptrDefaultMaterial ||
        pDefaultMatDesc->mChannels[eMaterialChannel_Base].mTexture != apTex ||
        pDefaultMatDesc->mChannels[eMaterialChannel_Base].mhSS != aSamplerStates ||
        pDefaultMatDesc->mBlendMode != aBlendMode)
    {
      this->Flush();
      pDefaultMatDesc->mChannels[eMaterialChannel_Base].mTexture = apTex;
      pDefaultMatDesc->mChannels[eMaterialChannel_Base].mhSS = aSamplerStates;
      pDefaultMatDesc->mBlendMode = aBlendMode;
      this->SetMaterial(mptrDefaultMaterial);
    }
  }
  virtual void __stdcall SetDefaultMaterial(iTexture* apTexture, eBlendMode aBlendMode, tIntPtr aSamplerStates) {
    _SetDefaultMaterial(apTexture,aBlendMode,aSamplerStates);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitFill(const sRectf& aDestRect, tU32 aColor) {
    _SetDefaultMaterial(NULL,eBlendMode_NoBlending,0);
    this->RectA(aDestRect.GetTopLeft(),
                aDestRect.GetBottomRight(),
                0.0f,aColor);
    return eFalse;
  }
  virtual tBool __stdcall BlitFillAlpha(const sRectf& aDestRect, tU32 aColor) {
    const tU32 a = ULColorGetA(aColor);
    if (a > 0) {
      _SetDefaultMaterial(NULL,(a >= 255) ? eBlendMode_NoBlending : eBlendMode_Translucent,0);
      this->RectA(aDestRect.GetTopLeft(),
                  aDestRect.GetBottomRight(),
                  0.0f,aColor);
    }
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitStretch(const sRectf& aDestRect, iTexture* apSrc, const sRectf& aSrcRect) {
    niCheckSilent(niIsOK(apSrc),eFalse);
    _SetDefaultMaterial(
        apSrc,
        eBlendMode_NoBlending,
        0);
    const sVec2f invSize = { ni::FInvert((tF32)apSrc->GetWidth()), ni::FInvert((tF32)apSrc->GetHeight()) };
    this->RectT(
        aDestRect.GetTopLeft(),
        aDestRect.GetBottomRight(),
        aSrcRect.GetTopLeft()*invSize,
        aSrcRect.GetBottomRight()*invSize,
        0.0f);
    return eTrue;
  }
  virtual tBool __stdcall BlitStretchAlpha(const sRectf& aDestRect, iTexture* apSrc, const sRectf& aSrcRect) {
    niCheckSilent(niIsOK(apSrc),eFalse);
    _SetDefaultMaterial(
        apSrc,
        eBlendMode_Translucent,
        0);
    const sVec2f invSize = { ni::FInvert((tF32)apSrc->GetWidth()), ni::FInvert((tF32)apSrc->GetHeight()) };
    this->RectT(
        aDestRect.GetTopLeft(),
        aDestRect.GetBottomRight(),
        aSrcRect.GetTopLeft()*invSize,
        aSrcRect.GetBottomRight()*invSize,
        0.0f);
    return eTrue;
  }
  virtual tBool __stdcall BlitStretchAlpha1(const sRectf& aDestRect, iTexture* apSrc, const sRectf& aSrcRect, tF32 afAlpha) {
    niCheckSilent(niIsOK(apSrc),eFalse);
    _SetDefaultMaterial(
        apSrc,
        eBlendMode_Translucent,
        0);
    const sVec2f invSize = { ni::FInvert((tF32)apSrc->GetWidth()), ni::FInvert((tF32)apSrc->GetHeight()) };
    const tU32 col = ULColorBuildf(1,1,1,afAlpha);
    this->RectTA(
        aDestRect.GetTopLeft(),
        aDestRect.GetBottomRight(),
        aSrcRect.GetTopLeft()*invSize,
        aSrcRect.GetBottomRight()*invSize,
        0.0f,
        col);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitOverlay(const sRectf& aDestRect, iOverlay* apOverlay) {
    niCheckSilent(niIsOK(apOverlay),eFalse);
    apOverlay->Draw(this,aDestRect.GetTopLeft(),aDestRect.GetSize());
    return eTrue;
  }
  virtual tBool __stdcall BlitOverlayFrame(const sRectf& aDestRect, iOverlay* apOverlay,  tRectFrameFlags aFrame) {
    niCheckSilent(niIsOK(apOverlay),eFalse);
    apOverlay->DrawFrame(this,aFrame,
                         Vec2<tF32>(aDestRect.x,aDestRect.y),
                         Vec2<tF32>(aDestRect.GetWidth(),aDestRect.GetHeight()));
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual sRectf __stdcall BlitText(iFont* apFont, const sRectf& aRect, tFontFormatFlags aFormatFlags, const achar* aaszText) {
    niCheckSilent(niIsOK(apFont),sRectf::Null());
    const sRectf& r = apFont->DrawText(
        this,
        aRect,
        0.0f,
        aaszText,
        aFormatFlags);
    return r;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitLine(const sVec2f& aStart, const sVec2f& aEnd, tU32 aColor) {
    _SetDefaultMaterial(NULL,eBlendMode_NoBlending,0);
    const sVec3f start = {aStart.x,aStart.y,0};
    const sVec3f end = {aEnd.x,aEnd.y,0};
    this->LineA(start,end,aColor);
    return eFalse;
  }
  virtual tBool __stdcall BlitLineAlpha(const sVec2f& aStart, const sVec2f& aEnd, tU32 aColor) {
    const tU32 a = ULColorGetA(aColor);
    if (a > 0) {
      _SetDefaultMaterial(NULL,(a >= 255) ? eBlendMode_NoBlending : eBlendMode_Translucent,0);
      const sVec3f start = {aStart.x,aStart.y,0};
      const sVec3f end = {aEnd.x,aEnd.y,0};
      this->LineA(start,end,aColor);
    }
    return eFalse;
  }

  ///////////////////////////////////////////////
  void _PushBlitRect(const sRectf& aRect,
                     const tF32 afLineSize,
                     const tU32 anColorLeft,
                     const tU32 anColorRight,
                     const tU32 anColorTop,
                     const tU32 anColorBottom)
  {
    const tF32 x1 = aRect.GetLeft();
    const tF32 x2 = aRect.GetRight();
    const tF32 y1 = aRect.GetTop();
    const tF32 y2 = aRect.GetBottom();
    const tF32 ls = afLineSize;
    // left side
    this->RectA(Vec2(x1,   y1),
                Vec2(x1+ls,y2),
                0.0f,anColorLeft);
    // right side
    this->RectA(Vec2(x2-ls,y1),
                Vec2(x2   ,y2),
                0.0f,anColorRight);
    // top side
    this->RectA(Vec2(x1+ls,y1),
                Vec2(x2-ls,y1+ls),
                0.0f,anColorTop);
    // bottom side
    this->RectA(Vec2(x1+ls,y2-ls),
                Vec2(x2-ls,y2),
                0.0f,anColorBottom);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BlitRect(const sRectf& aRect, tU32 anColor) {
    _SetDefaultMaterial(NULL,eBlendMode_NoBlending,0);
    _PushBlitRect(aRect,1.0f,anColor,anColor,anColor,anColor);
    return eFalse;
  }
  virtual tBool __stdcall BlitRectAlpha(const sRectf& aRect, tU32 anColor) {
    const tU32 a = ULColorGetA(anColor);
    if (a > 0) {
      _SetDefaultMaterial(NULL,(a >= 255) ? eBlendMode_NoBlending : eBlendMode_Translucent,0);
      _PushBlitRect(aRect,1.0f,anColor,anColor,anColor,anColor);
    }
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetContentsScale(tF32 afContentsScale) {
    mfContentsScale = afContentsScale;
  }
  virtual tF32 __stdcall GetContentsScale() const {
    return mfContentsScale;
  }

  ///////////////////////////////////////////////
  virtual sRectf __stdcall GetViewport() const {
    return mptrContext->GetViewport().ToFloat()/mfContentsScale;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetColorA(tU32 anColor) {
    mStates.mnColorA = anColor;
  }
  virtual tU32 __stdcall GetColorA() const {
    return mStates.mnColorA;
  }
  virtual void __stdcall SetNormal(const sVec3f& avNormal) {
    mStates.mvNormal = avNormal;
  }
  virtual sVec3f __stdcall GetNormal() const {
    return mStates.mvNormal;
  }
  virtual void __stdcall SetLineConstantScreenSize(tBool abConst) {
    niFlagOnIf(mStates.mnFlags, CANVAS_FLAGS_LineConstantScreenSize, abConst);
  }
  virtual tBool __stdcall GetLineConstantScreenSize() const {
    return (mStates.mnFlags&CANVAS_FLAGS_LineConstantScreenSize);
  }
  virtual void __stdcall SetLineSize(tF32 afSize) {
    mStates.mfLineSize = afSize;
  }
  virtual tF32 __stdcall GetLineSize() const {
    return mStates.mfLineSize;
  }
  virtual void __stdcall SetMatrix(const sMatrixf& aMatrix) {
    if (mStates.mMatrix != aMatrix) {
      this->Flush();
      mStates.mMatrix = aMatrix;
    }
  }
  virtual sMatrixf __stdcall GetMatrix() const {
    return mStates.mMatrix;
  }
  virtual void __stdcall SetMaterial(iMaterial* apMaterial) {
    iMaterial* pNewMaterial = apMaterial ? apMaterial : mptrDefaultMaterial.ptr();
    if (mStates.mptrMaterial != pNewMaterial) {
      this->Flush();
      mStates.mptrMaterial = pNewMaterial;
    }
  }
  virtual iMaterial* __stdcall GetMaterial() const {
    return mStates.mptrMaterial;
  }

  ///////////////////////////////////////////////
  __forceinline void _AddVertex(const tVertexCanvas& aV) {
    GET_BUFFER_CACHE_VERTEX();
    if (mStates.mnFlags & CANVAS_FLAGS_BakeTransform) {
      const sMatrixf& m = mStates.mMatrix;
      tVertexCanvas v = aV;
      VecTransformCoord(v.pos,aV.pos,m);
      VecTransformNormal(v.normal,aV.normal,m);
      bufferCacheVertex->Add(v);
    }
    else {
      bufferCacheVertex->Add(aV);
    }
  }
  __forceinline void _AddVertices(const tVertexCanvas* aV, tU32 anCount) {
    GET_BUFFER_CACHE_VERTEX();
    if (mStates.mnFlags & CANVAS_FLAGS_BakeTransform) {
      const sMatrixf& m = mStates.mMatrix;
      niLoop(i,anCount) {
        tVertexCanvas v = aV[i];
        VecTransformCoord(v.pos,v.pos,m);
        VecTransformNormal(v.normal,v.normal,m);
        bufferCacheVertex->Add(v);
      }
    }
    else {
      bufferCacheVertex->Add(aV,anCount);
    }
  }
  __forceinline tIndex _GetCurrentVertex() const {
    GET_BUFFER_CACHE_VERTEX();
#ifdef USE_BUFFER_CACHE_BASE_VERTEX_INDEX
    return bufferCacheVertex->GetCurrentIndex();
#else
    return bufferCacheVertex->GetCurrentEl();
#endif
  }
  __forceinline void _AddIndex(tIndex anIndex) {
    GET_BUFFER_CACHE_INDEX();
    bufferCacheIndex->Add(anIndex);
  }
  __forceinline void _AddIndices(tIndex* apIndices, tU32 anCount) {
    GET_BUFFER_CACHE_INDEX();
    bufferCacheIndex->Add(apIndices,anCount);
  }
  __forceinline tIndex _GetCurrentIndex() const {
    GET_BUFFER_CACHE_INDEX();
    return bufferCacheIndex->GetCurrentEl();
  }

  inline void _BeginNextBatch() {
#ifdef USE_BUFFER_CACHE_RING_BUFFER
    if (mnCurrentBufferCache >= _knRingBufferSize-1) {
      mnCurrentBufferCache = 0;
    }
    else {
      ++mnCurrentBufferCache;
    }
    if (!mptrBufferCacheVertex[mnCurrentBufferCache].IsOK()) {
      mptrBufferCacheVertex[mnCurrentBufferCache] = niNew BufferCacheVertex<tVertexCanvas>(mptrContext->GetGraphics(), _knBufferCacheInitSize);
    }
    if (!mptrBufferCacheIndex[mnCurrentBufferCache].IsOK()) {
      mptrBufferCacheIndex[mnCurrentBufferCache] = niNew BufferCacheIndex(mptrContext->GetGraphics(), _knBufferCacheInitSize*3);
    }
#else
    if (!mptrBufferCacheVertex.IsOK()) {
      mptrBufferCacheVertex = niNew BufferCacheVertex<tVertexCanvas>(mptrContext->GetGraphics(), _knBufferCacheInitSize);
    }
    if (!mptrBufferCacheIndex.IsOK()) {
      mptrBufferCacheIndex = niNew BufferCacheIndex(mptrContext->GetGraphics(), _knBufferCacheInitSize*3);
    }
#endif
    GET_BUFFER_CACHE_VERTEX();
    GET_BUFFER_CACHE_INDEX();
    bufferCacheVertex->Reset();
    bufferCacheIndex->Reset();
  }
  __forceinline tBool _HasVertices() const {
    return _GetCurrentVertex() > 2;
  }
  virtual tBool __stdcall GetHasVertices() const {
    return _HasVertices();
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall VertexP(const sVec3f& avPosition) {
    _AddIndex(_GetCurrentVertex());
    tVertexCanvas v;
    v.pos = avPosition;
    v.colora = mStates.mnColorA;
    v.normal = mStates.mvNormal;
    v.tex1 = sVec2f::Zero();
    _AddVertex(v);
    return eTrue;
  }
  virtual tBool __stdcall VertexPN(const sVec3f& avPosition, const sVec3f& avNormal) {
    _AddIndex(_GetCurrentVertex());
    tVertexCanvas v;
    v.pos = avPosition;
    v.colora = mStates.mnColorA;
    v.normal = mStates.mvNormal;
    v.tex1 = sVec2f::Zero();
    _AddVertex(v);
    return eTrue;
  }
  virtual tBool __stdcall VertexPT(const sVec3f& avPosition, const sVec2f& avTex) {
    _AddIndex(_GetCurrentVertex());
    tVertexCanvas v;
    v.pos = avPosition;
    v.colora = mStates.mnColorA;
    v.normal = mStates.mvNormal;
    v.tex1 = avTex;
    _AddVertex(v);
    return eTrue;
  }
  virtual tBool __stdcall VertexPTA(const sVec3f& avPosition, const sVec2f& avTex, tU32 anColorA) {
    _AddIndex(_GetCurrentVertex());
    tVertexCanvas v;
    v.pos = avPosition;
    v.colora = anColorA;
    v.normal = mStates.mvNormal;
    v.tex1 = avTex;
    _AddVertex(v);
    return eTrue;
  }
  virtual tBool __stdcall VertexPNT(const sVec3f& avPosition, const sVec3f& avNormal, const sVec2f& avTex) {
    _AddIndex(_GetCurrentVertex());
    tVertexCanvas v;
    v.pos = avPosition;
    v.colora = mStates.mnColorA;
    v.normal = avNormal;
    v.tex1 = avTex;
    _AddVertex(v);
    return eTrue;
  }
  virtual tBool __stdcall VertexPNTA(const sVec3f& avPosition, const sVec3f& avNormal, const sVec2f& avTex, tU32 anColorA) {
    _AddIndex(_GetCurrentVertex());
    tVertexCanvas v;
    v.pos = avPosition;
    v.colora = anColorA;
    v.normal = avNormal;
    v.tex1 = avTex;
    _AddVertex(v);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Rect(const sVec2f& avTL, const sVec2f& avBR, tF32 afZ) {
    tU32 nColA = GetColorA();
    return _RectTA2(avTL,avBR,
                    _vTLTex,_vBRTex,
                    afZ,
                    nColA,nColA,nColA,nColA);
  }
  virtual tBool __stdcall RectT(const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ) {
    tU32 nColA = GetColorA();
    return _RectTA2(avTL,avBR,
                    avTLTex,avBRTex,
                    afZ,
                    nColA,nColA,nColA,nColA);
  }
  virtual tBool __stdcall RectA(const sVec2f& avTL, const sVec2f& avBR, tF32 afZ, tU32 anColA) {
    return _RectTA2(avTL,avBR,
                    _vTLTex,_vBRTex,
                    afZ,
                    anColA,anColA,anColA,anColA);
  }
  virtual tBool __stdcall RectTA(const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ, tU32 anColA) {
    return _RectTA2(avTL,avBR,
                    avTLTex,avBRTex,
                    afZ,
                    anColA,anColA,anColA,anColA);
  }
  inline tBool __stdcall _RectTA2(const sVec2f& avTL, const sVec2f& avBR,
                                  const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ,
                                  tU32 anTLColA, tU32 anTRColA, tU32 anBRColA, tU32 anBLColA)
  {
    return QuadTA2(
        // TL
        Vec3<tF32>(avTL.x,avTL.y,afZ),
        avTLTex,
        anTLColA,
        // TR
        Vec3<tF32>(avBR.x,avTL.y,afZ),
        Vec2<tF32>(avBRTex.x,avTLTex.y),
        anTRColA,
        // BR
        Vec3<tF32>(avBR.x,avBR.y,afZ),
        avBRTex,
        anBRColA,
        // BL
        Vec3<tF32>(avTL.x,avBR.y,afZ),
        Vec2<tF32>(avTLTex.x,avBRTex.y),
        anBLColA);
  }
  virtual tBool __stdcall RectTA2(const sVec2f& avTL, const sVec2f& avBR,
                                  const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ,
                                  tU32 anTLColA, tU32 anTRColA, tU32 anBRColA, tU32 anBLColA)
  {
    return _RectTA2(avTL,avBR,avTLTex,avBRTex,afZ,anTLColA,anTRColA,anBRColA,anBLColA);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall QuadEx(
      const sVec3f& avTL, const sVec3f& avTLN, const sVec2f& avTLT, tU32 anTLA,
      const sVec3f& avTR, const sVec3f& avTRN, const sVec2f& avTRT, tU32 anTRA,
      const sVec3f& avBR, const sVec3f& avBRN, const sVec2f& avBRT, tU32 anBRA,
      const sVec3f& avBL, const sVec3f& avBLN, const sVec2f& avBLT, tU32 anBLA)
  {
    tVertexCanvas v;

    tU32 nFirstIndex = _GetCurrentVertex();
    tU32 nIndices[6] {
      nFirstIndex+0,
      nFirstIndex+1,
      nFirstIndex+3,
      nFirstIndex+1,
      nFirstIndex+2,
      nFirstIndex+3,
    };

    sVec3f vTL, vTR, vBL, vBR;
    if (mStates.mBBMode) {
      sVec3f vSize = avBR-avTL;
      sVec3f vPos = avTL+(vSize*0.5f);
      sVec3f vUp = mStates.mvBBUp;
      sVec3f vRight = mStates.mvBBRight;
      tF32 fLeft, fRight, fTop, fBottom;
      if (niFlagIs(mStates.mBBMode,eBillboardModeFlags_CustomCenter)) {
        sVec3f vOffset = vPos-mStates.mvBBCenter;
        fLeft = (-vSize.x/2) + vOffset.x;
        fRight = fLeft+vSize.x;
        fTop = (-vSize.y/2) + vOffset.y;
        fBottom = fTop+vSize.y;
        vPos = mStates.mvBBCenter;
      }
      else {
        fLeft = (-vSize.x/2);
        fRight = fLeft+vSize.x;
        fTop = (-vSize.y/2);
        fBottom = fTop+vSize.y;
      }
      vTL  = vPos + (vUp * fTop)    + (vRight * fLeft);
      vTR  = vPos + (vUp * fTop)    + (vRight * fRight);
      vBL  = vPos + (vUp * fBottom) + (vRight * fLeft);
      vBR  = vPos + (vUp * fBottom) + (vRight * fRight);
    }
    else {
      vTL = (avTL);
      vTR = (avTR);
      vBL = (avBL);
      vBR = (avBR);
    }

    // Top-Left
    v.pos = vTL;
    v.colora = anTLA;
    v.tex1 = avTLT;
    v.normal = avTLN;
    _AddVertex(v);

    // Top-Right
    v.pos = vTR;
    v.colora = anTRA;
    v.tex1 = avTRT;
    v.normal = avTRN;
    _AddVertex(v);

    // Bottom-Right
    v.pos = vBR;
    v.colora = anBRA;
    v.tex1 = avBRT;
    v.normal = avBRN;
    _AddVertex(v);

    // Bottom-Left
    v.pos = vBL;
    v.colora = anBLA;
    v.tex1 = avBLT;
    v.normal = avBLN;
    _AddVertex(v);

    _AddIndices(nIndices,6);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Quad(const sVec3f& avTL, const sVec3f& avTR, const sVec3f& avBR, const sVec3f& avBL)
  {
    const tU32 nA = mStates.mnColorA;
    const sVec3f& vNormal = mStates.mvNormal;
    return QuadEx(
        avTL,vNormal,_vTLTex,nA,
        avTR,vNormal,_vTRTex,nA,
        avBR,vNormal,_vBRTex,nA,
        avBL,vNormal,_vBLTex,nA);
  }
  tBool __stdcall QuadA(const sVec3f& avTL, const sVec3f& avTR, const sVec3f& avBR, const sVec3f& avBL, tU32 anColorA)
  {
    const tU32 nA = anColorA;
    const sVec3f& vNormal = mStates.mvNormal;
    return QuadEx(
        avTL,vNormal,_vTLTex,nA,
        avTR,vNormal,_vTRTex,nA,
        avBR,vNormal,_vBRTex,nA,
        avBL,vNormal,_vBLTex,nA);
  }
  virtual tBool __stdcall QuadA2(
      const sVec3f& avTL, tU32 anTLColA,
      const sVec3f& avTR, tU32 anTRColA,
      const sVec3f& avBR, tU32 anBRColA,
      const sVec3f& avBL, tU32 anBLColA)
  {
    const sVec3f& vNormal = mStates.mvNormal;
    return QuadEx(
        avTL,vNormal,_vTLTex,anTLColA,
        avTR,vNormal,_vTRTex,anTRColA,
        avBR,vNormal,_vBRTex,anBRColA,
        avBL,vNormal,_vBLTex,anBLColA);
  }
  virtual tBool __stdcall QuadT(
      const sVec3f& avTL, const sVec2f& avTLTex,
      const sVec3f& avTR, const sVec2f& avTRTex,
      const sVec3f& avBR, const sVec2f& avBRTex,
      const sVec3f& avBL, const sVec2f& avBLTex)
  {
    const tU32 nA = mStates.mnColorA;
    const sVec3f& vNormal = mStates.mvNormal;
    return QuadEx(
        avTL,vNormal,avTLTex,nA,
        avTR,vNormal,avTRTex,nA,
        avBR,vNormal,avBRTex,nA,
        avBL,vNormal,avBLTex,nA);
  }
  virtual tBool __stdcall QuadTA(
      const sVec3f& avTL, const sVec2f& avTLTex,
      const sVec3f& avTR, const sVec2f& avTRTex,
      const sVec3f& avBR, const sVec2f& avBRTex,
      const sVec3f& avBL, const sVec2f& avBLTex,
      tU32 anColorA)
  {
    const tU32 nA = anColorA;
    const sVec3f& vNormal = mStates.mvNormal;
    return QuadEx(
        avTL,vNormal,avTLTex,nA,
        avTR,vNormal,avTRTex,nA,
        avBR,vNormal,avBRTex,nA,
        avBL,vNormal,avBLTex,nA);
  }
  virtual tBool __stdcall QuadTA2(
      const sVec3f& avTL, const sVec2f& avTLTex, tU32 anTLColA,
      const sVec3f& avTR, const sVec2f& avTRTex, tU32 anTRColA,
      const sVec3f& avBR, const sVec2f& avBRTex, tU32 anBRColA,
      const sVec3f& avBL, const sVec2f& avBLTex, tU32 anBLColA)
  {
    const sVec3f& vNormal = mStates.mvNormal;
    return QuadEx(
        avTL,vNormal,avTLTex,anTLColA,
        avTR,vNormal,avTRTex,anTRColA,
        avBR,vNormal,avBRTex,anBRColA,
        avBL,vNormal,avBLTex,anBLColA);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Frame(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, tF32 afZ) {
    return FrameT(aFrame,aFrameBorder,avTL,avBR,sVec2f::Zero(),sVec2f::One(),0.0f);
  }
  virtual tBool __stdcall FrameA(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, tF32 afZ, tU32 anColA) {
    return FrameTA(aFrame,aFrameBorder,avTL,avBR,sVec2f::Zero(),sVec2f::One(),0.0f,anColA);
  }
  virtual tBool __stdcall FrameT(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ) {
    return _FrameT(aFrame,aFrameBorder,avTL,avBR,avTLTex,avBRTex,afZ,GetColorA());
  }
  virtual tBool __stdcall FrameTA(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ, tU32 anColA) {
    return _FrameT(aFrame,aFrameBorder,avTL,avBR,avTLTex,avBRTex,afZ,anColA);
  }
  virtual tBool __stdcall FrameTA2(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ, tU32 anTLColA, tU32 anTRColA, tU32 anBRColA, tU32 anBLColA) {
	  return _FrameTA2(aFrame,aFrameBorder,avTL,avBR,avTLTex,avBRTex,afZ, anTLColA, anTRColA,anBRColA, anBLColA);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Line(const sVec3f& avStart, const sVec3f& avEnd) {
    return LineEx(avStart, sVec2f::Zero(), mStates.mfLineSize, mStates.mnColorA, avEnd, sVec2f::One(), mStates.mfLineSize, mStates.mnColorA);
  }
  virtual tBool __stdcall LineA(const sVec3f& avStart, const sVec3f& avEnd, tU32 anCol) {
    return LineEx(avStart, sVec2f::Zero(), mStates.mfLineSize, anCol, avEnd, sVec2f::One(), mStates.mfLineSize, anCol);
  }
  virtual tBool __stdcall LineEx(const sVec3f& avStart, const sVec2f& avStartTex,
                                 tF32 afStartSize, tU32 anStartCol,
                                 const sVec3f& avEnd, const sVec2f& avEndTex,
                                 tF32 afEndSize, tU32 anEndCol)
  {
    tVertexCanvas v[4];
    tU32 nFirstIndex = _GetCurrentVertex();
    tU32 nIndices[12];
    nIndices[0] = nFirstIndex+0;
    nIndices[1] = nFirstIndex+1;
    nIndices[2] = nFirstIndex+3;
    nIndices[3] = nFirstIndex+1;
    nIndices[4] = nFirstIndex+2;
    nIndices[5] = nFirstIndex+3;

    // draw double sided
    nIndices[ 6] = nFirstIndex+0;
    nIndices[ 7] = nFirstIndex+3;
    nIndices[ 8] = nFirstIndex+1;
    nIndices[ 9] = nFirstIndex+1;
    nIndices[10] = nFirstIndex+3;
    nIndices[11] = nFirstIndex+2;

    tBool bVisible = DoComputeScreenQuad(
      mptrContext,
      // This must be the pixel viewport -
      // the same as used by the projection
      // matrix.
      mptrContext->GetViewport().ToFloat(),
      mStates.mMatrix,
      avStart, avEnd, afStartSize, afEndSize,
      v,
      (mStates.mnFlags&CANVAS_FLAGS_LineConstantScreenSize),
      eFalse, NULL);
    if (bVisible) {
      // Top-Left
      v[0].colora = anStartCol;
      v[0].tex1.x = avStartTex.x;
      v[0].tex1.y = avStartTex.y;
      // Top-Right
      v[1].colora = anEndCol;
      v[1].tex1.x = avEndTex.x;
      v[1].tex1.y = avStartTex.y;
      // Bottom-Right
      v[2].colora = anEndCol;
      v[2].tex1.x = avEndTex.x;
      v[2].tex1.y = avEndTex.y;
      // Bottom-Left
      v[3].colora = anStartCol;
      v[3].tex1.x = avStartTex.x;
      v[3].tex1.y = avEndTex.y;
      _AddVertices(v,4);
      _AddIndices(nIndices,niCountOf(nIndices));
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall LineGridXY(ni::tF32 aX, ni::tF32 aY, ni::tU32 anNumColumns, ni::tU32 anNumRows) niImpl {
    const ni::tF32 tx2 = (tF32)(aX * ((tF32)anNumColumns/2.0f));
    const ni::tF32 ty2 = (tF32)(aY * ((tF32)anNumRows/2.0f));
    for (ni::tF32 x = -tx2; x <= tx2; x += aX) {
      this->Line(Vec3f(x, -ty2, 0.0f), Vec3f(x, ty2, 0.0f));
    }
    for (ni::tF32 y = -ty2; y <= ty2; y += aY) {
      this->Line(Vec3f(-tx2, y, 0.0f), Vec3f(tx2, y, 0.0f));
    }
  }

  void __stdcall LineGridXZ(ni::tF32 aX, ni::tF32 aZ, ni::tU32 anNumColumns, ni::tU32 anNumRows) niImpl {
    const ni::tF32 tx2 = (tF32)(aX * ((tF32)anNumColumns/2.0f));
    const ni::tF32 tz2 = (tF32)(aZ * ((tF32)anNumRows/2.0f));
    for (ni::tF32 x = -tx2; x <= tx2; x += aX) {
      this->Line(Vec3f(x, 0.0f, -tz2), Vec3f(x, 0.0f, tz2));
    }
    for (ni::tF32 z = -tz2; z <= tz2; z += aZ) {
      this->Line(Vec3f(-tx2, 0.0f, z), Vec3f(tx2, 0.0f, z));
    }
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall WireframeCircle(const sVec3f& avCenter, tF32 afRadius, tU32 aulNumDiv = 32, ePlaneType aPlane = ePlaneType_XY) {
    return _WireframeEllipse(avCenter,Vec2(afRadius,afRadius),aulNumDiv,aPlane);
  }
  virtual tBool __stdcall WireframeEllipse(const sVec3f& avCenter, const sVec2f& avRadius, tU32 aulNumDiv = 32, ePlaneType aPlane = ePlaneType_XY) {
    return _WireframeEllipse(avCenter,avRadius,aulNumDiv,aPlane);
  }
  virtual tBool __stdcall WireframeAABB(const sVec3f& avMin, const sVec3f& avMax) {
    return _WireframeAABB(avMin,avMax);
  }
  virtual tBool __stdcall WireframeSphere(const sVec3f& aCenter, tF32 afRadius, tU32 aulNumDiv = 32) {
    return _WireframeEllipsoid(aCenter,Vec3(afRadius,afRadius,afRadius),aulNumDiv,mStates.mnColorA,mStates.mnColorA,mStates.mnColorA);
  }
  virtual tBool __stdcall WireframeSphereEx(const sVec3f& aCenter, tF32 afRadius, tU32 aulNumDiv,
                                            tU32 aulColorX, tU32 aulColorY, tU32 aulColorZ) {
    return _WireframeEllipsoid(aCenter,Vec3(afRadius,afRadius,afRadius),aulNumDiv,aulColorX,aulColorY,aulColorZ);
  }
  virtual tBool __stdcall WireframeEllipsoid(const sVec3f& aCenter, const sVec3f& avRadius, tU32 aulNumDiv = 32) {
    return _WireframeEllipsoid(aCenter,avRadius,aulNumDiv,mStates.mnColorA,mStates.mnColorA,mStates.mnColorA);
  }
  virtual tBool __stdcall WireframeEllipsoidEx(const sVec3f& aCenter, const sVec3f& avRadius, tU32 aulNumDiv,
                                               tU32 aulColorX, tU32 aulColorY, tU32 aulColorZ)
  {
    return _WireframeEllipsoid(aCenter,avRadius,aulNumDiv,aulColorX,aulColorY,aulColorZ);
  }
  virtual tBool __stdcall WireframeCone(const sVec3f& avBaseCenter, tF32 afBaseRadius, tF32 afHeight, tU32 aulNumDiv = 32, ePlaneType aPlane = ePlaneType_XY)
  {
    return _WireframeCone(avBaseCenter,afBaseRadius,afHeight,aulNumDiv,aPlane);
  }

  inline tBool __stdcall _WireframeEllipse(const sVec3f& avCenter, const sVec2f& avRadius, tU32 aulNumDiv, ePlaneType aPlane) {

    tF32 fStep = ni2Pif/tF32(aulNumDiv);
    tF32 fCurrent = 0.0f;
    sVec3f vCur, vFirst, vPrev;

    if (aPlane == ePlaneType_XZ)
    {
      _GetCirclePointXZ(vFirst,avCenter,avRadius,fCurrent);
      fCurrent += fStep;
      _GetCirclePointXZ(vCur,avCenter,avRadius,fCurrent);
      fCurrent += fStep;
      this->Line(vFirst,vCur);
      vPrev = vCur;
      for (tU32 i = 1; i < aulNumDiv; ++i, fCurrent += fStep) {
        _GetCirclePointXZ(vCur,avCenter,avRadius,fCurrent);
        this->Line(vPrev,vCur);
        vPrev = vCur;
      }
      this->Line(vPrev,vFirst);
    }
    else if (aPlane == ePlaneType_YZ)
    {
      _GetCirclePointYZ(vFirst,avCenter,avRadius,fCurrent);
      fCurrent += fStep;
      _GetCirclePointYZ(vCur,avCenter,avRadius,fCurrent);
      fCurrent += fStep;
      this->Line(vFirst,vCur);
      vPrev = vCur;
      for (tU32 i = 1; i < aulNumDiv; ++i, fCurrent += fStep) {
        _GetCirclePointYZ(vCur,avCenter,avRadius,fCurrent);
        this->Line(vPrev,vCur);
        vPrev = vCur;
      }
      this->Line(vPrev,vFirst);
    }
    else
    {
      _GetCirclePointXY(vFirst,avCenter,avRadius,fCurrent);
      fCurrent += fStep;
      _GetCirclePointXY(vCur,avCenter,avRadius,fCurrent);
      fCurrent += fStep;
      this->Line(vFirst,vCur);
      vPrev = vCur;
      for (tU32 i = 1; i < aulNumDiv; ++i, fCurrent += fStep) {
        _GetCirclePointXY(vCur,avCenter,avRadius,fCurrent);
        this->Line(vPrev,vCur);
        vPrev = vCur;
      }
      this->Line(vPrev,vFirst);
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall _WireframeAABB(const sVec3f& avMin, const sVec3f& avMax) {

    sVec3f verts[8];
    cAABBf aabb(avMin,avMax);
    aabb.GetVertices(tPtr(verts), 0, sizeof(sVec3f));
    /*
     *   vertex[x]                          Y+
     *    \                                        Z+
     *     7 <---size---> 6 (max)           |    /
     *      *------------*                  |   /
     *     /|           /|                  |  /
     *    / |          / |                  | /
     * 4 /  |       5 /  |                  |/
     *  *---+--------*   |        ----------+---------- X+
     *  |   |        |   |                 /|
     *  |   |        |   |                / |
     *  | 3 *--------+---* 2             /  |
     *  |  /         |  /               /   |
     *  | /          | /               /    |
     *  |/           |/
     *  *------------*
     *  0 (min)      1
    */
    this->Line(verts[0],verts[1]);
    this->Line(verts[1],verts[5]);
    this->Line(verts[5],verts[4]);
    this->Line(verts[4],verts[0]);

    this->Line(verts[2],verts[6]);
    this->Line(verts[6],verts[7]);
    this->Line(verts[7],verts[3]);
    this->Line(verts[3],verts[2]);

    this->Line(verts[4], verts[7]);
    this->Line(verts[5], verts[6]);
    this->Line(verts[1], verts[2]);
    this->Line(verts[0], verts[3]);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall _WireframeEllipsoid(const sVec3f& aCenter, const sVec3f& avRadius, tU32 aulNumDiv, tU32 aulColorX, tU32 aulColorY, tU32 aulColorZ) {
    const tU32 wasColorA = this->GetColorA();
    this->SetColorA(aulColorX);
    _WireframeEllipse(aCenter,Vec2(avRadius.z,avRadius.y),aulNumDiv,ePlaneType_X);
    this->SetColorA(aulColorY);
    _WireframeEllipse(aCenter,Vec2(avRadius.x,avRadius.z),aulNumDiv,ePlaneType_Y);
    this->SetColorA(aulColorZ);
    _WireframeEllipse(aCenter,Vec2(avRadius.x,avRadius.y),aulNumDiv,ePlaneType_Z);
    this->SetColorA(wasColorA);
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall _WireframeCone(const sVec3f& avBaseCenter, tF32 afBaseRadius, tF32 afHeight, tU32 aulNumDiv = 32, ePlaneType aPlane = ePlaneType_XY) {

    _WireframeEllipse(avBaseCenter,Vec2(afBaseRadius,afBaseRadius),aulNumDiv,aPlane);
    if (aPlane == ePlaneType_XZ)
    {
      this->Line(
          Vec3(avBaseCenter.x + (::cosf(0.0f) * afBaseRadius),
               avBaseCenter.z + (::sinf(0.0f) * afBaseRadius),
               avBaseCenter.y),
          Vec3(avBaseCenter.x,
               avBaseCenter.z,
               avBaseCenter.y+afHeight));

      this->Line(
          Vec3(avBaseCenter.x + (::cosf(niPif/2) * afBaseRadius),
               avBaseCenter.z + (::sinf(niPif/2) * afBaseRadius),
               avBaseCenter.y),
          Vec3(avBaseCenter.x,
               avBaseCenter.z,
               avBaseCenter.y+afHeight));

      this->Line(
          Vec3(avBaseCenter.x + (::cosf(niPif) * afBaseRadius),
               avBaseCenter.z + (::sinf(niPif) * afBaseRadius),
               avBaseCenter.y),
          Vec3(avBaseCenter.x,
               avBaseCenter.z,
               avBaseCenter.y+afHeight));

      this->Line(
          Vec3(avBaseCenter.x + (::cosf(niPif/2+niPif) * afBaseRadius),
               avBaseCenter.z + (::sinf(niPif/2+niPif) * afBaseRadius),
               avBaseCenter.y),
          Vec3(avBaseCenter.x,
               avBaseCenter.z,
               avBaseCenter.y+afHeight));
    }
    else if (aPlane == ePlaneType_YZ)
    {
      this->Line(
          Vec3(avBaseCenter.y + (::cosf(0.0f) * afBaseRadius),
               avBaseCenter.z + (::sinf(0.0f) * afBaseRadius),
               avBaseCenter.x),
          Vec3(avBaseCenter.y,
               avBaseCenter.z,
               avBaseCenter.x+afHeight));

      this->Line(
          Vec3(avBaseCenter.y + (::cosf(niPif/2) * afBaseRadius),
               avBaseCenter.z + (::sinf(niPif/2) * afBaseRadius),
               avBaseCenter.x),
          Vec3(avBaseCenter.y,
               avBaseCenter.z,
               avBaseCenter.x+afHeight));

      this->Line(
          Vec3(avBaseCenter.y + (::cosf(niPif) * afBaseRadius),
               avBaseCenter.z + (::sinf(niPif) * afBaseRadius),
               avBaseCenter.x),
          Vec3(avBaseCenter.y,
               avBaseCenter.z,
               avBaseCenter.x+afHeight));

      this->Line(
          Vec3(avBaseCenter.y + (::cosf(niPif/2+niPif) * afBaseRadius),
               avBaseCenter.z + (::sinf(niPif/2+niPif) * afBaseRadius),
               avBaseCenter.x),
          Vec3(avBaseCenter.y,
               avBaseCenter.z,
               avBaseCenter.x+afHeight));
    }
    else //if (aPlane == ePlaneType_XY)
    {
      this->Line(
          Vec3(avBaseCenter.x + (::cosf(0.0f) * afBaseRadius),
               avBaseCenter.y + (::sinf(0.0f) * afBaseRadius),
               avBaseCenter.z),
          Vec3(avBaseCenter.x,
               avBaseCenter.y,
               avBaseCenter.z+afHeight));

      this->Line(
          Vec3(avBaseCenter.x + (::cosf(niPif/2) * afBaseRadius),
               avBaseCenter.y + (::sinf(niPif/2) * afBaseRadius),
               avBaseCenter.z),
          Vec3(avBaseCenter.x,
               avBaseCenter.y,
               avBaseCenter.z+afHeight));

      this->Line(
          Vec3(avBaseCenter.x + (::cosf(niPif) * afBaseRadius),
               avBaseCenter.y + (::sinf(niPif) * afBaseRadius),
               avBaseCenter.z),
          Vec3(avBaseCenter.x,
               avBaseCenter.y,
               avBaseCenter.z+afHeight));

      this->Line(
          Vec3(avBaseCenter.x + (::cosf(niPif/2+niPif) * afBaseRadius),
               avBaseCenter.y + (::sinf(niPif/2+niPif) * afBaseRadius),
               avBaseCenter.z),
          Vec3(avBaseCenter.x,
               avBaseCenter.y,
               avBaseCenter.z+afHeight));
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall _FrameTA2(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ, tU32 anTLColA, tU32 anTRColA, tU32 anBRColA, tU32 anBLColA)
  {
    sVec2f rs;
    if (mStates.mptrMaterial.IsOK()) {
      rs.x = (tF32)mStates.mptrMaterial->GetWidth();
      rs.y = (tF32)mStates.mptrMaterial->GetHeight();
    }
    else {
      rs.x = 256.0f;
      rs.y = 256.0f;
    }
    const tF32 rl =
        (niFlagIs(aFrame,eRectFrameFlags_TopLeftCorner) ||
         niFlagIs(aFrame,eRectFrameFlags_LeftEdge) ||
         niFlagIs(aFrame,eRectFrameFlags_BottomLeftCorner)) ?
        aFrameBorder.Left() : 0;
    const tF32 rr =
        (niFlagIs(aFrame,eRectFrameFlags_TopRightCorner) ||
         niFlagIs(aFrame,eRectFrameFlags_RightEdge) ||
         niFlagIs(aFrame,eRectFrameFlags_BottomRightCorner)) ?
        aFrameBorder.Right() : 0;
    const tF32 rt =
        (niFlagIs(aFrame,eRectFrameFlags_TopLeftCorner) ||
         niFlagIs(aFrame,eRectFrameFlags_TopEdge) ||
         niFlagIs(aFrame,eRectFrameFlags_TopRightCorner)) ?
        aFrameBorder.Top() : 0;
    const tF32 rb =
        (niFlagIs(aFrame,eRectFrameFlags_BottomLeftCorner) ||
         niFlagIs(aFrame,eRectFrameFlags_BottomEdge) ||
         niFlagIs(aFrame,eRectFrameFlags_BottomRightCorner)) ?
        aFrameBorder.Bottom() : 0;
    const tF32 l = tF32(aFrameBorder.Left())/rs.x;
    const tF32 r = tF32(aFrameBorder.Right())/rs.x;
    const tF32 t = tF32(aFrameBorder.Top())/rs.y;
    const tF32 b = tF32(aFrameBorder.Bottom())/rs.y;
    const tF32 txl = avTLTex.x;
    const tF32 txr = avBRTex.x;
    const tF32 txt = avTLTex.y;
    const tF32 txb = avBRTex.y;
    const sVec2f tl = avTL;
    const sVec2f tr = Vec2(avBR.x,avTL.y);
    const sVec2f bl = Vec2(avTL.x,avBR.y);
    const sVec2f br = avBR;

    const tU32 width = avBR.x - avTL.x;
    const tU32 height = avBR.y - avTL.y;
    const tBool bColorSame = (anTLColA == anTRColA && anBRColA == anBLColA && anTLColA == anBRColA);
    tU32 trbrColor = anTLColA;
    tU32 tltrColor = anTLColA;
    tU32 tlblColor = anTLColA;
    tU32 tlbrColor = anTLColA;
    tU32 trtlColor = anTLColA;
    tU32 trblColor = anTLColA;
    tU32 brtrColor = anTLColA;
    tU32 bltlColor = anTLColA;
    tU32 bltrColor = anTLColA;
    tU32 blbrColor = anTLColA;
    tU32 brtlColor = anTLColA;
    tU32 brblColor = anTLColA;

    // List of vertices;
    // A--AB----BA--B
    // |  |      |  |
    // AC-X------Y-BD
    // |  |      |  |
    // |  |      |  |
    // CA-Z------W-DB
    // |  |      |  |
    // C--CD----DC--D
    if (!bColorSame)
    {
      tF32 rth = rt/height;
      tF32 rlw = rl/width;
      tF32 wrw = (width-rr)/width;
      tF32 hrh = (height-rb)/height;

      // AB = (A.B)
      tltrColor = ULColorLerp(anTLColA, anTRColA, rlw);
      // AC = (A.C)
      tlblColor = ULColorLerp(anTLColA, anBLColA, rth);
      // BA = (B.A)
      trtlColor = ULColorLerp(anTLColA, anTRColA, wrw);
      // BD = (B.D)
      trbrColor = ULColorLerp(anTRColA, anBRColA, rth);
      // CA = (C.A)
      bltlColor = ULColorLerp(anTLColA, anBLColA, hrh);
      // CD = (C.D)
      blbrColor = ULColorLerp(anBLColA, anBRColA, rlw);
      // DB = (D.B)
      brtrColor = ULColorLerp(anTRColA, anBRColA, hrh);
      // DC = (D.C)
      brblColor = ULColorLerp(anBLColA, anBRColA, wrw);

      // X = (AC.BD)
      tlbrColor = ULColorLerp(tlblColor, trbrColor, rlw);
      // Y = (BD.AC)
      trblColor = ULColorLerp(tlblColor, trbrColor, wrw);
      // Z = (CA.DB)
      bltrColor = ULColorLerp(bltlColor, brtrColor, rlw);
      // W = (DB.CA)
      brtlColor = ULColorLerp(bltlColor, brtrColor, wrw);
    }

    // top left
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_TopLeftCorner)) {
      this->RectTA2(tl, tl+Vec2<tF32>(rl,rt),
                    Vec2<tF32>(txl,txt),Vec2<tF32>(txl+l,txt+t),
                    0,
                    anTLColA, tltrColor, tlbrColor, tlblColor);
    }

    // top right
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_TopRightCorner)) {
      this->RectTA2(tr-Vec2<tF32>(rr,0), tr+Vec2<tF32>(0,rt),
                    Vec2<tF32>(txr-r,txt), Vec2<tF32>(txr,txt+t),
                    0,
                    trtlColor, anTRColA, trbrColor, trblColor);
    }
    // bottom left
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_BottomLeftCorner)) {
      this->RectTA2(bl-Vec2<tF32>(0,rb), bl+Vec2<tF32>(rl,0),
                    Vec2<tF32>(txl,txb-b), Vec2<tF32>(txl+l,txb),
                    0,
                    bltlColor, bltrColor, blbrColor, anBLColA);
    }
    // bottom right
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_BottomRightCorner)) {
      this->RectTA2(br-Vec2<tF32>(rr,rb), br-Vec2<tF32>(0,0),
                    Vec2<tF32>(txr-r,txb-b), Vec2<tF32>(txr,txb),
                    0,
                    brtlColor, brtrColor, anBRColA, brblColor);
    }
    // top
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_TopEdge)) {
      this->RectTA2(tl+Vec2<tF32>(rl,0), tr+Vec2<tF32>(-rr,rt),
                    Vec2<tF32>(txl+l,txt), Vec2<tF32>(txr-r,txt+t),
                    0,
                    tltrColor, trtlColor, trblColor, tlbrColor);
    }
    // left
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_LeftEdge)) {
      this->RectTA2(tl+Vec2<tF32>(0,rt), bl+Vec2<tF32>(rl,-rb),
                    Vec2<tF32>(txl,txt+t), Vec2<tF32>(txl+l,txb-b),
                    0,
                    tlblColor, tlbrColor, bltrColor, bltlColor);
    }
    // bottom
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_BottomEdge)) {
      this->RectTA2(bl+Vec2<tF32>(rl,-rb), br+Vec2<tF32>(-rr,0),
                    Vec2<tF32>(txl+l,txb-b), Vec2<tF32>(txr-r,txb),
                    0,
                    bltrColor, brtlColor, brblColor, blbrColor);
    }
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_RightEdge)) {
      // right
      this->RectTA2(tr+Vec2<tF32>(-rr,rt), br+Vec2<tF32>(0,-rb),
                    Vec2<tF32>(txr-r,txt+t), Vec2<tF32>(txr,txb-b),
                    0,
                    trblColor, trbrColor, brtrColor, brtlColor);
    }
    // center
    if (!aFrame || niFlagIs(aFrame,eRectFrameFlags_Center)) {
      this->RectTA2(tl+Vec2<tF32>(rl,rt), br-Vec2<tF32>(rr,rb),
                    Vec2<tF32>(txl+l,txt+t), Vec2<tF32>(txr-r,txb-b),
                    0,
                    tlbrColor, trblColor, brtlColor, bltrColor);
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  inline tBool __stdcall _FrameT(tRectFrameFlags aFrame, const sVec4f& aFrameBorder, const sVec2f& avTL, const sVec2f& avBR, const sVec2f& avTLTex, const sVec2f& avBRTex, tF32 afZ, tU32 anColA) {
    return _FrameTA2(aFrame, aFrameBorder, avTL, avBR, avTLTex, avBRTex, afZ, anColA, anColA, anColA, anColA);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetBillboard(tBillboardModeFlags aBB) {
    mStates.mBBMode = aBB;
    if (mStates.mBBMode) {
      if (!niFlagIs(mStates.mnFlags,CANVAS_FLAGS_BBRightSet)) {
        SetBillboardRight(MatrixGetRight(mStates.mvBBRight,mptrContext->GetFixedStates()->GetViewMatrix()));
      }
      if (!niFlagIs(mStates.mnFlags,CANVAS_FLAGS_BBUpSet)) {
        SetBillboardUp(MatrixGetUp(mStates.mvBBUp,mptrContext->GetFixedStates()->GetViewMatrix()));
      }
    }
  }
  virtual tBillboardModeFlags __stdcall GetBillboard() const {
    return mStates.mBBMode;
  }
  virtual void __stdcall SetBillboardRight(const sVec3f& avRight) {
    mStates.mvBBRight = avRight;
    niFlagOn(mStates.mnFlags,CANVAS_FLAGS_BBRightSet);
  }
  virtual sVec3f __stdcall GetBillboardRight() const {
    return mStates.mvBBRight;
  }
  virtual void __stdcall SetBillboardUp(const sVec3f& avUp) {
    mStates.mvBBUp = avUp;
    niFlagOn(mStates.mnFlags,CANVAS_FLAGS_BBUpSet);
  }
  virtual sVec3f __stdcall GetBillboardUp() const {
    return mStates.mvBBUp;
  }
  virtual void __stdcall SetBillboardCenter(const sVec3f& avCenter) {
    mStates.mvBBCenter = avCenter;
  }
  virtual sVec3f __stdcall GetBillboardCenter() const {
    return mStates.mvBBCenter;
  }

  ///////////////////////////////////////////////
  virtual iDrawOperation* __stdcall GetDrawOperation() const {
    return mptrDrawOp;
  }

  ///////////////////////////////////////////////
  inline void _CheckVGPathRenderer() {
    if (!mptrCanvasVGPathRenderer.IsOK()) {
      mptrCanvasVGPathRenderer = niNew sCanvasVGPathTesselatedRenderer(this);
    }
  }
  virtual void __stdcall DrawPath(const iVGPath* apPath) {
    niThis(cCanvasGraphics)->_CheckVGPathRenderer();
    apPath->RenderTesselated(mptrCanvasVGPathRenderer,
                             mptrCanvasVGPathRenderer->mVGTransforms[eVGTransform_Path],
                             mptrCanvasVGPathRenderer->mptrVGStyle);
  }
  virtual iMaterial* __stdcall GetVGMaterial() const {
    niThis(cCanvasGraphics)->_CheckVGPathRenderer();
    return mptrCanvasVGPathRenderer->mptrVGMaterial;
  }
  virtual iVGStyle* __stdcall GetVGStyle() const {
    niThis(cCanvasGraphics)->_CheckVGPathRenderer();
    return mptrCanvasVGPathRenderer->mptrVGStyle;
  }
  virtual iVGTransform* __stdcall GetVGTransform(eVGTransform aTransform) const {
    niCheck(aTransform < eVGTransform_Last, NULL);
    niThis(cCanvasGraphics)->_CheckVGPathRenderer();
    return mptrCanvasVGPathRenderer->mVGTransforms[aTransform];
  }

 public:
  Ptr<iGraphics>        mptrGraphics;
  Ptr<iGraphicsContext> mptrContext;
  Ptr<iMaterial>        mptrResetMaterial;
  Ptr<iMaterial>        mptrDefaultMaterial;
  tF32                  mfContentsScale;

  sGraphicsCanvasStates   mStates;
  Ptr<iDrawOperation>     mptrDrawOp;

#ifdef USE_BUFFER_CACHE_RING_BUFFER
  Ptr<BufferCacheVertex<tVertexCanvas> > mptrBufferCacheVertex[_knRingBufferSize];
  Ptr<BufferCacheIndex> mptrBufferCacheIndex[_knRingBufferSize];
  tU32 mnCurrentBufferCache = ~0;
#else
  Ptr<BufferCacheVertex<tVertexCanvas> > mptrBufferCacheVertex;
  Ptr<BufferCacheIndex> mptrBufferCacheIndex;
#endif

  Ptr<sCanvasVGPathTesselatedRenderer> mptrCanvasVGPathRenderer;
};

///////////////////////////////////////////////
iCanvas* __stdcall cGraphics::CreateCanvas(iGraphicsContext* apContext, iMaterial* apResetMaterial) {
  niCheck(niIsOK(apContext), NULL);
  return niNew cCanvasGraphics(this,apContext,apResetMaterial);
}
