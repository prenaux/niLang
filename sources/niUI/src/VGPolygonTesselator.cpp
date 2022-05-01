// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(20)

#include "Graphics.h"
#include "VGPolygonTesselator.h"

///////////////////////////////////////////////
cVGPolygonTesselator::cVGPolygonTesselator() :
    mvFinalVerts(tVec2fCVec::Create()),
    mbBegan(eFalse),
    mnNumContour(0),
    mbEvenOdd(eFalse)
{
}

///////////////////////////////////////////////
cVGPolygonTesselator::~cVGPolygonTesselator() {
}

///////////////////////////////////////////////
tBool __stdcall cVGPolygonTesselator::AddVertexF32(tF32 x, tF32 y) {
  if (!mbBegan) return eFalse;
  mvPolyVerts.push_back(Vec2f(x,y));
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cVGPolygonTesselator::AddVertexF64(tF64 x, tF64 y) {
  if (!mbBegan) return eFalse;
  mvPolyVerts.push_back(Vec2f(tF32(x),tF32(y)));
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cVGPolygonTesselator::AddVertex(const sVec2f& vertex) {
  if (!mbBegan) return eFalse;
  mvPolyVerts.push_back(vertex);
  return eTrue;
}

///////////////////////////////////////////////
tSize __stdcall cVGPolygonTesselator::GetNumVertices() const {
  return mvPolyVerts.size();
}

///////////////////////////////////////////////
tBool __stdcall cVGPolygonTesselator::SubmitContour() {
  if (!mbBegan || !mvPolyVerts.size())
    return eFalse;
#ifdef USE_GPC
  gpc_vertex_list verts;
  verts.num_vertices = mvPolyVerts.size();
  // TODO: This memory allocation should be removed
  verts.vertex = niTMalloc(gpc_vertex,verts.num_vertices);
  for (tI32 i = 0; i < verts.num_vertices; ++i) {
    verts.vertex[i].x = mvPolyVerts[i].x;
    verts.vertex[i].y = mvPolyVerts[i].y;
  }
  gpc_add_contour(&mGPCPoly,&verts,mbEvenOdd?((mnNumContour%2)==0):0);
  niFree(verts.vertex);
#else
  // TODO: handle mbEvenOdd
  tessAddContour(mTess, 2, mvPolyVerts.data(), sizeof(sVec2f), mvPolyVerts.size());
#endif
  ++mnNumContour;
  // TODO: This malloc/free should be removed
  mvPolyVerts.clear();
  mvPolyVerts.reserve(1024);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cVGPolygonTesselator::BeginPolygon(tBool abEvenOdd) {
  if (mbBegan) return eFalse;
  mbEvenOdd = abEvenOdd;
  mnNumContour = 0;
#ifdef USE_GPC
  mGPCPoly.num_contours = 0;
  mGPCPoly.hole= NULL;
  mGPCPoly.contour= NULL;
#else
  mTess = tessNewTess(NULL);
#endif
  mbBegan = eTrue;
  mvPolyVerts.clear();
  mvPolyVerts.reserve(1024);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cVGPolygonTesselator::EndPolygon() {
  if (!mbBegan) return eFalse;
  mvFinalVerts->clear();
  mvFinalVerts->reserve(1024);
#ifdef USE_GPC
  if (mGPCPoly.num_contours) {
    gpc_tristrip tris;
    gpc_polygon_to_tristrip(&mGPCPoly,&tris);
    for (tI32 ts = 0; ts < tris.num_strips; ++ts) {
      gpc_vertex_list& verts = tris.strip[ts];
      mvFinalVerts->reserve(mvFinalVerts->size()+(3*(verts.num_vertices-2)));
      niLoop(i,verts.num_vertices-2) {
        mvFinalVerts->push_back(
            Vec2f(tF32(verts.vertex[i+0].x),tF32(verts.vertex[i+0].y)));
        if (i&1) {
          mvFinalVerts->push_back(
              Vec2f(tF32(verts.vertex[i+2].x),tF32(verts.vertex[i+2].y)));
          mvFinalVerts->push_back(
              Vec2f(tF32(verts.vertex[i+1].x),tF32(verts.vertex[i+1].y)));
        }
        else {
          mvFinalVerts->push_back(
              Vec2f(tF32(verts.vertex[i+1].x),tF32(verts.vertex[i+1].y)));
          mvFinalVerts->push_back(
              Vec2f(tF32(verts.vertex[i+2].x),tF32(verts.vertex[i+2].y)));
        }
      }
    }
    gpc_free_tristrip(&tris);
  }
  gpc_free_polygon(&mGPCPoly);
#else
  if (mTess) {
    if (mnNumContour) {
      const int nvp = 3; // number of vertex per polygon (3 for triangles)
#ifdef USE_LIBTESS_OPTIMIZE_OUTPUT_TRIANGLES
      // First combine contours and then triangulate, this removes unnecessary inner vertices.
      if (tessTesselate(mTess, TESS_WINDING_POSITIVE, TESS_BOUNDARY_CONTOURS, 0, 0, 0))
      {
        const float* verts = tessGetVertices(mTess);
        // const int* vinds = tessGetVertexIndices(mTess);
        // const int nverts = tessGetVertexCount(mTess);
        const int* elems = tessGetElements(mTess);
        const int nelems = tessGetElementCount(mTess);

        for (int i = 0; i < nelems; ++i) {
          const int b = elems[i*2];
          const int n = elems[i*2+1];
          tessAddContour(mTess, 2, &verts[b*2], sizeof(float)*2, n);
        }

        if (tessTesselate(mTess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0)) {
          const float* verts = tessGetVertices(mTess);
          // const int* vinds = tessGetVertexIndices(mTess);
          // const int nverts = tessGetVertexCount(mTess);
          const int* elems = tessGetElements(mTess);
          const int nelems = tessGetElementCount(mTess);
          for (int i = 0; i < nelems; ++i)
          {
            const int* p = &elems[i*nvp];
            for (int j = 0; j < nvp && p[j] != TESS_UNDEF; ++j) {
              mvFinalVerts->push_back(Vec2f(verts[p[j]*2], verts[p[j]*2+1]));
            }
          }
        }
      }
#else
      if (tessTesselate(mTess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, NULL)) {
        const float* verts = tessGetVertices(mTess);
        const int* vinds = tessGetVertexIndices(mTess);
        const int* elems = tessGetElements(mTess);
        const int nverts = tessGetVertexCount(mTess);
        const int nelems = tessGetElementCount(mTess);
        for (int i = 0; i < nelems; ++i)
        {
          const int* p = &elems[i*nvp];
          for (int j = 0; j < nvp && p[j] != TESS_UNDEF; ++j) {
            mvFinalVerts->push_back(Vec2f(verts[p[j]*2], verts[p[j]*2+1]));
          }
        }
      }
#endif
    }
    tessDeleteTess(mTess);
    mTess = NULL;
  }
#endif
  mbBegan = eFalse;
  return eTrue;
}

///////////////////////////////////////////////
const tVec2fCVec* __stdcall cVGPolygonTesselator::GetTesselatedVertices() const {
  return mvFinalVerts;
}

Ptr<iVGPolygonTesselator> __stdcall cGraphics::CreateVGPolygonTesselator() {
  return niNew cVGPolygonTesselator();
}

#endif // niMinFeatures
