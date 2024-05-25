// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "GeometryQuadric.h"

struct sPrimitiveVertex {
  sVec3f pos;
  sVec3f normal;
  sVec2f tex;
  tU32    color;
};

typedef astl::vector<sPrimitiveVertex>  tPrimitiveVertexVec;
typedef tPrimitiveVertexVec::iterator tPrimitiveVertexVecIt;
typedef tPrimitiveVertexVec::const_iterator tPrimitiveVertexVecCIt;

struct sQuadStrip {
  sPrimitiveVertex  mCurrentVertex;
  tPrimitiveVertexVec mvVertices;
  tBool mbInvertWinding;

  sQuadStrip() {
    mvVertices.reserve(64);
    mbInvertWinding = eFalse;
  }
  ~sQuadStrip() {
  }
  void Position(tF32 x, tF32 y, tF32 z) {
    mCurrentVertex.pos = Vec3(x,y,z);
    mvVertices.push_back(mCurrentVertex);
  }
  void Normal(tF32 x, tF32 y, tF32 z) {
    mCurrentVertex.normal = Vec3(x,z,y);
  }
  void TexCoo(tF32 x, tF32 y) {
    mCurrentVertex.tex = Vec2(x,y);
  }
  void Color(tU32 anColor) {
    mCurrentVertex.color = anColor;
  }
  tU32 GetNumVertices() const {
    return mvVertices.size();
  }
  tU32 GetNumQuads() const {
    return (GetNumVertices()-2)/2;
  }
  tU32 GetNumTriangles() const {
    return GetNumVertices()-2;
  }
  tU32 GetQuadFirstVertexIndex(tU32 anQuadIndex) const {
    niAssert(anQuadIndex < GetNumQuads());
    return anQuadIndex*2;
  }
  const sPrimitiveVertex& GetVertex(tU32 anIndex) const {
    niAssert(anIndex < GetNumVertices());
    return mvVertices[anIndex];
  }
  void InvertWinding() {
    mbInvertWinding = !mbInvertWinding;
  }
  void OutputVertices(const cFVFDescription& fvfDesc, cFVFStream& fvfStream, cFVFVertex& fvfVertex,
                      tIndex*& pInds, tU32& nCount,
                      const sMatrixf& amtxUV, tBool abCW) const
  {
    if (mbInvertWinding)
      abCW = !abCW;
    tU32 i;
    for (i = 0; i < GetNumVertices(); ++i)
    {
      const sPrimitiveVertex& v = GetVertex(i);

      fvfVertex.Set(fvfStream.Current());
      if (fvfDesc.HasPosition()) {
        fvfVertex.Position() = Vec3f(v.pos.x,v.pos.z,v.pos.y);
      }
      if (fvfDesc.HasNormal()) {
        fvfVertex.Normal() = (abCW ? 1.0f : -1.0f) * v.normal;
        VecNormalize(fvfVertex.Normal());
      }
      niLoop(k,fvfDesc.GetNumTexCoos()) {
        if (fvfDesc.GetTexCooDim(k) >= 2) {
          VecTransformCoord(fvfVertex.TexCoo2(k), v.tex, amtxUV);
        }
      }
      if (fvfDesc.HasColorA())  {
        fvfVertex.ColorA() = v.color;
      }
      fvfStream.Next();
    }

    for (i = 0; i < GetNumQuads(); ++i)
    {
      tU32 nVI = GetQuadFirstVertexIndex(i);
      if (!abCW) {
        // first triangle
        pInds[0] = nCount+nVI+0;
        pInds[1] = nCount+nVI+1;
        pInds[2] = nCount+nVI+2;
        // second triangle
        pInds[3] = nCount+nVI+1;
        pInds[4] = nCount+nVI+3;
        pInds[5] = nCount+nVI+2;
      }
      else {
        // first triangle
        pInds[0] = nCount+nVI+2;
        pInds[1] = nCount+nVI+1;
        pInds[2] = nCount+nVI+0;
        // second triangle
        pInds[3] = nCount+nVI+2;
        pInds[4] = nCount+nVI+3;
        pInds[5] = nCount+nVI+1;
      }
      pInds += 6;
    }
    nCount += GetNumVertices();
  }
};

typedef astl::vector<sQuadStrip>    tQuadStripVec;
typedef tQuadStripVec::iterator     tQuadStripVecIt;
typedef tQuadStripVec::const_iterator tQuadStripVecCIt;

void QuadricDiskSweepQuads(
    tQuadStripVec& vQuads,
    tF32 innerRadius, tF32 outerRadius,
    tI32 slices, tI32 loops,
    tF32 startAngle, tF32 sweepAngle,
    tBool abCW, tU32 aulColor,
    const sMatrixf& amtxUV,
    tU32& nNumTris, tU32& nNumVerts,
    tU8 anAxis, sVec3f avOffset, tBool abInvertWinding);

tBool FillGeometryFromQuads(iGeometry* apGeom,
                            const tQuadStripVec& avQuads, tU32 anNumVerts, tU32 anNumTris,
                            const sMatrixf& amtxUV, tBool abCW)
{
  if (apGeom->GetVertexArray()->GetNumVertices() != anNumVerts) {
    return eFalse;
  }
  if (apGeom->GetIndexArray()->GetNumIndices() != anNumTris*3) {
    return eFalse;
  }

  cFVFDescription fvfDesc(apGeom->GetVertexArray()->GetFVF());
  cFVFStream fvfStream(&fvfDesc,apGeom->GetVertexArray()->Lock(0,0,eLock_Normal),anNumVerts);
  cFVFVertex fvfVertex(&fvfDesc);
  tIndex* pInds = (tIndex*)apGeom->GetIndexArray()->Lock(0,0,eLock_Normal);
  tU32 nCount = 0;

  for (tQuadStripVecCIt it = avQuads.begin(); it != avQuads.end(); ++it) {
    it->OutputVertices(fvfDesc,fvfStream,fvfVertex,pInds,nCount,amtxUV,abCW);
  }

  apGeom->GetVertexArray()->Unlock();
  apGeom->GetIndexArray()->Unlock();
  return eTrue;
}

iGeometry* CreateGeometryFromQuads(iGraphics* apGraphics, const tQuadStripVec& avQuads, tU32 anNumVerts, tU32 anNumTris,
                                            tGeometryCreateFlags aFlags, tFVF aFVF, const sMatrixf& amtxUV, tBool abCW)
{
  // generate the geometry
  iGeometry* pGeom = apGraphics->CreateGeometryPolygonal(anNumVerts,anNumTris*3,aFlags,aFVF);
  if (!niIsOK(pGeom)) {
    niSafeRelease(pGeom);
    return NULL;
  }

  if (!FillGeometryFromQuads(pGeom,avQuads,anNumVerts,anNumTris,amtxUV,abCW)) {
    niSafeRelease(pGeom);
    return NULL;
  }

  //  pGeom->Generate(eGeometryGenerate(eGeometryGenerate_Normals|
  //                    eGeometryGenerate_TextureSpaces));
  pGeom->Optimize(eGeometryOptimizeFlags_RemoveDegenerates|
                  eGeometryOptimizeFlags_RemoveDuplicateVertices|
                  eGeometryOptimizeFlags_RemoveUnusedVertices);

  pGeom->AddSubset(0,0,pGeom->GetIndexArray()->GetNumIndices(),0);
  return pGeom;
}

iGeometry*
QuadricCylinder(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
                tF32 baseRadius, tF32 topRadius,
                tF32 height, tI32 slices, tI32 stacks,
                tBool abCW, tU32 aulColor, const sMatrixf& amtxUV, tBool abCap, tBool abCentered)
{
  tF32 da, r, dr, dz;
  tF32 z, nz;
  tI32 i, j;

  da = 2.0f * niPif / (tF32)slices;
  dr = (topRadius - baseRadius) / (tF32)stacks;
  dz = height / (tF32)stacks;
  nz = (baseRadius - topRadius) / height; /* Z component of normal vectors */

  tF32 heightOffset = abCentered ? -(height*0.5f) : 0.0f;

  tQuadStripVec vQuads;

  tU32 nNumTris = 0;
  tU32 nNumVerts = 0;
  tF32 ds = 1.0f / (tF32)slices;
  tF32 dt = 1.0f / (tF32)stacks;
  tF32 t = 0.0f;
  z = 0.0f;
  r = baseRadius;
  for (j = 0; j < stacks; ++j)
  {
    tF32 s = 0.0;
    vQuads.push_back(sQuadStrip());
    sQuadStrip& strip = vQuads.back();
    for (i = 0; i <= slices; i++)
    {
      tF32 x, y;
      if (i == slices) {
        x = sinf(0.0f);
        y = cosf(0.0f);
      }
      else {
        x = ni::Sin((tF32)i * da);
        y = ni::Cos((tF32)i * da);
      }

      strip.Color(aulColor);
      strip.Normal(x, y, nz);
      strip.TexCoo(s, -t);
      strip.Position(x * r, y * r, z + heightOffset);

      strip.Color(aulColor);
      strip.Normal(x, y, nz);
      strip.TexCoo(s, -(t + dt));
      strip.Position(x * (r + dr), y * (r + dr), z + dz + heightOffset);

      s += ds;
    }     /* for slices */
    r += dr;
    t += dt;
    z += dz;
    nNumVerts += strip.GetNumVertices();
    nNumTris += strip.GetNumTriangles();
  }       /* for stacks */

  if (abCap) {
    if (baseRadius > niEpsilon2) {
      QuadricDiskSweepQuads(vQuads,0.0f,baseRadius,slices,stacks,0,ni2Pif,abCW,aulColor,amtxUV,nNumTris,nNumVerts,'z',Vec3<tF32>(0,0,0+heightOffset),eTrue);
    }
    if (topRadius > niEpsilon2) {
      QuadricDiskSweepQuads(vQuads,0.0f,topRadius,slices,stacks,0,ni2Pif,abCW,aulColor,amtxUV,nNumTris,nNumVerts,'z',Vec3<tF32>(0,0,height+heightOffset),eFalse);
    }
  }

  iGeometry* pGeom = CreateGeometryFromQuads(apGraphics,vQuads,nNumVerts,nNumTris,aFlags,aFVF,amtxUV,abCW);
  if (!niIsOK(pGeom)) {
    niError(_A("Can't create destination geometry."));
    return NULL;
  }

  return pGeom;
}

iGeometry*
QuadricSphere(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
              tF32 radius, tI32 slices, tI32 stacks,
              tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  tF32 rho, drho, theta, dtheta;
  tF32 x, y, z;
  tF32 s, t, ds, dt;
  tI32 i, j, imin, imax;

  drho = niPif / (tF32) stacks;
  dtheta = 2.0f * niPif / (tF32) slices;

  // texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
  // t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
  // cannot use triangle fan on texturing (s coord. at top/bottom tip varies)

  ds = 1.0f / (tF32)slices;
  dt = 1.0f / (tF32)stacks;
  t = 1.0f;     // because loop now runs from 0
  imin = 0;
  imax = stacks;

  tQuadStripVec vQuads;
  tU32 nNumTris = 0;
  tU32 nNumVerts = 0;
  // draw intermediate stacks as quad strips
  for (i = imin; i < imax; i++) {
    rho = (tF32)i * drho;
    vQuads.push_back(sQuadStrip());
    sQuadStrip& strip = vQuads.back();
    s = 0.0;
    for (j = 0; j <= slices; j++) {
      theta = (j == slices) ? 0.0f : (tF32)j * dtheta;
      x = -sin(theta) * sin(rho);
      y = cos(theta) * sin(rho);
      z = cos(rho);
      strip.Color(aulColor);
      strip.Normal(x, y, z);
      strip.TexCoo(s, -t);
      strip.Position(x * radius, y * radius, z * radius);
      x = -sin(theta) * sin(rho + drho);
      y = cos(theta) * sin(rho + drho);
      z = cos(rho + drho);
      strip.Color(aulColor);
      strip.Normal(x, y, z);
      strip.TexCoo(s, dt - t);
      s += ds;
      strip.Position(x * radius, y * radius, z * radius);
    }
    t -= dt;

    nNumVerts += strip.GetNumVertices();
    nNumTris += strip.GetNumTriangles();
  }

  iGeometry* pGeom = CreateGeometryFromQuads(apGraphics,vQuads,nNumVerts,nNumTris,aFlags,aFVF,amtxUV,abCW);
  if (!niIsOK(pGeom)) {
    niError(_A("Can't create destination geometry."));
    return NULL;
  }

  return pGeom;
}

void QuadricDiskSweepQuads(tQuadStripVec& vQuads, tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 loops, tF32 startAngle, tF32 sweepAngle,
                           tBool abCW, tU32 aulColor, const sMatrixf& amtxUV, tU32& nNumTris, tU32& nNumVerts, tU8 anAxis, sVec3f avOffset, tBool abInvertWinding)
{
  tI32 loop, slice;
  tF32 radius, delta_radius;
  tF32 angle, delta_angle, x, y;
  delta_radius = (outerRadius - innerRadius) / (tF32)loops;
  delta_angle = (sweepAngle / (tF32)slices);
  radius = innerRadius;
  for (loop = 0; loop < loops; loop++)
  {
    vQuads.push_back(sQuadStrip());
    sQuadStrip& strip = vQuads.back();
    angle = startAngle;
    for (slice = 0; slice <= slices; slice++)
    {
      strip.Color(aulColor);
      strip.Normal(0,0,abCW?1.0f:-1.0f);
      strip.TexCoo(0,0);
      x = (radius + delta_radius) * sinf(angle);
      y = ((radius + delta_radius) * cosf(angle));
      switch (anAxis) {
        case 'x': case 'X': strip.Position(0+avOffset.x, y+avOffset.y, x+avOffset.z); break;
        case 'y': case 'Y': strip.Position(x+avOffset.x, 0+avOffset.y, y+avOffset.z); break;
        case 'z': case 'Z': strip.Position(x+avOffset.x, y+avOffset.y, 0+avOffset.z); break;
      }
      strip.Color(aulColor);
      strip.Normal(0,0,abCW?1.0f:-1.0f);
      strip.TexCoo(0,0);
      x = radius * sin(angle);
      y = radius * cos(angle);
      switch (anAxis) {
        case 'x': case 'X': strip.Position(0+avOffset.x, y+avOffset.y, x+avOffset.z); break;
        case 'y': case 'Y': strip.Position(x+avOffset.x, 0+avOffset.y, y+avOffset.z); break;
        case 'z': case 'Z': strip.Position(x+avOffset.x, y+avOffset.y, 0+avOffset.z); break;
      }
      angle += delta_angle;
    }
    radius += delta_radius;
    nNumVerts += strip.GetNumVertices();
    nNumTris += strip.GetNumTriangles();
    if (abInvertWinding) {
      strip.InvertWinding();
    }
  }
}

iGeometry*
QuadricDiskSweep(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
                 tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 loops, tF32 startAngle, tF32 sweepAngle,
                 tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  tQuadStripVec vQuads;

  tU32 nNumTris = 0;
  tU32 nNumVerts = 0;

  QuadricDiskSweepQuads(vQuads,innerRadius,outerRadius,slices,loops,startAngle,sweepAngle,abCW,aulColor,amtxUV,nNumTris,nNumVerts,'z',sVec3f::Zero(),eFalse);

  iGeometry* pGeom = CreateGeometryFromQuads(apGraphics,vQuads,nNumVerts,nNumTris,aFlags,aFVF,amtxUV,abCW);
  if (!niIsOK(pGeom)) {
    niError(_A("Can't create destination geometry."));
    return NULL;
  }

  return pGeom;
}

tBool
QuadricDiskSweepEx(iGeometry* apGeom,
                   tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 loops, tF32 startAngle, tF32 sweepAngle,
                   tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  tQuadStripVec vQuads;
  tU32 nNumTris = 0;
  tU32 nNumVerts = 0;
  tI32 loop, slice;
  tF32 radius, delta_radius;
  tF32 angle, delta_angle;
  delta_radius = (outerRadius - innerRadius) / (tF32)loops;
  delta_angle = (sweepAngle / (tF32)slices);
  radius = innerRadius;
  for (loop = 0; loop < loops; loop++)
  {
    vQuads.push_back(sQuadStrip());
    sQuadStrip& strip = vQuads.back();
    angle = startAngle;
    for (slice = 0; slice <= slices; slice++)
    {
      strip.Color(aulColor);
      strip.Normal(0,0,abCW?1.0f:-1.0f);
      strip.TexCoo(0,0);
      strip.Position((radius + delta_radius) * sinf(angle), (radius + delta_radius) * cosf(angle), 0.0f);
      strip.Color(aulColor);
      strip.Normal(0,0,abCW?1.0f:-1.0f);
      strip.TexCoo(0,0);
      strip.Position(radius * sin(angle), radius * cos(angle), 0.0f);
      angle += delta_angle;
    }
    radius += delta_radius;
    nNumVerts += strip.GetNumVertices();
    nNumTris += strip.GetNumTriangles();
  }

  if (!FillGeometryFromQuads(apGeom,vQuads,nNumVerts,nNumTris,amtxUV,abCW)) {
    niError(_A("Can't fill the destination geometry."));
    return eFalse;
  }

  return eTrue;
}

iGeometry*
QuadricDisk(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
            tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 loops,
            tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  return QuadricDiskSweep(apGraphics, aFlags, aFVF, innerRadius, outerRadius, slices, loops, 0.0, ni2Pif, abCW, aulColor, amtxUV);
}


//--------------------------------------------------------------------------------------------
//
//  Geosphere
//
//--------------------------------------------------------------------------------------------
struct Mesh {
  Mesh(tPtr apVerts, tU32 anNumVerts, tU32 anVertStride, tIndex* apInds, tU32 anNumFaces) {
    _verts = apVerts;
    _nNumVerts = anNumVerts;
    _nVertStride = anVertStride;
    _inds = apInds;
    _nNumFaces = anNumFaces;
  }

  sVec3f& vertRef(int idx) const {
    return *((sVec3f*)(_verts+(idx*_nVertStride)));
  }

  void setVert(int i, const sVec3f& v) {
    vertRef(i) = v;
  }

  void setVert(int i, float x, float y, float z) {
    vertRef(i) = Vec3(x,y,z);
  }

  const sVec3f& getVert(int i) const {
    return vertRef(i);
  }

  int getNumVerts() const {
    return _nNumVerts;
  }

  int getNumFaces() const {
    return _nNumFaces;
  }

  void setFaceVerts(int fi, tIndex a, tIndex b, tIndex c) {
    _inds[fi*3+0] = a;
    _inds[fi*3+1] = b;
    _inds[fi*3+2] = c;
  }

  sVec3<tIndex> getFaceVerts(int fi) {
    return Vec3<tIndex>(
        _inds[fi*3+0],
        _inds[fi*3+1],
        _inds[fi*3+2]);
  }

 private:
  tU32    _nNumVerts;
  tU32    _nVertStride;
  tPtr    _verts;
  tU32    _nNumFaces;
  tIndex*   _inds;
};

// CONSTRUCTING THE MESH:

// To construct a geodesic sphere, we take a tetrahedron, subdivide each face into
// segs^2 faces, and project the vertices onto the sphere of the correct radius.

// This subdivision produces 3 kinds of vertices: 4 "corner" vertices, which are the
// original tetrahedral vertices; "edge" vertices, those that lie on the tetrahedron's
// edges, and "face" vertices.  There are 6 edges with (segs-1) verts on each, and
// 4 faces with (segs-1)*(segs-2)/2 verts.

// We construct these vertices in this order: the first four are the corner vertices.
// Then we use spherical interpolation to place edge vertices along each edge.
// Finally, we use the same interpolation to produce face vertices between the edge
// vertices.

void SinCos(const float a, float* s, float* c) {
  if (s) *s = ni::Sin(a);
  if (c) *c = ni::Cos(a);
}


// Assumed in the following function: the vertices have the same radius, or
// distance from the origin, and they have nontrivial cross product.

void SphericalInterpolate (Mesh& amesh, int v1, int v2, int *current, int num)
{
  int i;
  float theta, theta1, theta2, sn, cs, rad;
  sVec3f a, b, c;

  if (num<2) { return; }
  a=amesh.getVert (v1);
  b=amesh.getVert (v2);
  rad = VecDot (a, a);
  if (rad==0) {
    for (i=1; i<num; i++) amesh.setVert ((*current)++, a);
    return;
  }
  cs = VecDot (a, b) / rad;
  cs = ni::Clamp(cs, -1.0f, 1.0f);
  theta = (float) acos (cs);
  sn = ni::Sin (theta);

  for (i=1; i<num; i++) {
    theta1 = (theta*(tF32)i)/(tF32)num;
    theta2 = (theta*(tF32)(num-i))/(tF32)num;
    c = (a*ni::Sin(theta2) + b*ni::Sin(theta1))/sn;
    amesh.setVert ((*current)++, c);
  }
}

// The order we produced the verts in makes for difficult face construction.  Thus
// we use the following functions to find the index of the vertex in a given row,
// given column, of a given basetype face.

// (Each face is viewed as a triangular grid of vertices.  The orientation of the
// triangle is such that it "points up", so rows are "horizontal" while columns
// are diagonal: column 0 is the left edge of the triangle.)

int tetra_find_vert_index (int s, int f, int r, int c)
{           // segs, face, row, column.
  if (r==0) {
    if (f<3) { return 0; }
    else { return 1; }
  }

  if (c==0) {
    if (r==s) {
      if (f<3) { return f+1; }
      else { return 3; }
    }
    if (f<3) { return 4+(s-1)*f+r-1; }
    else { return 4+(s-1)*4+r-1; }
  }

  if (c==r) {
    if (r==s) {
      if (f<3) { return (f+1)%3+1; }
      else { return 2; }
    }
    else {
      if (f<3) { return 4+(s-1)*((f+1)%3)+r-1; }
      else { return 4+(s-1)*3+r-1; }
    }
  }

  if (r==s) {
    switch (f) {
      case 0: return 4+(s-1)*3 + c-1;
      case 1: return 4+(s-1)*5 + c-1;
      case 2: return 4+(s-1)*4 + s-1-c;
      case 3: return 4+(s-1)*5 + s-1-c;
    }
  }

  return 4 + (s-1)*6 + f*(s-1)*(s-2)/2 + (r-2)*(r-1)/2 + c-1;
}

int hemi_tetra_find_vert_index (int s, int f, int r, int c)
{               // segs, face, row, column.
  if (r==0) return 0;

  if (c==0) {
    if (r==s) return f+1;
    return 4+(s-1)*f+r-1;
  }

  if (c==r) {
    if (r==s) return (f+1)%3+1;
    return 4+(s-1)*((f+1)%3)+r-1;
  }

  if (r==s) return 4 + (s-1)*(f+3) + c-1;

  return 4 + (s-1)*6 + f*(s-1)*(s-2)/2 + (r-2)*(r-1)/2 + c-1;
}

int octa_find_vert_index (int s, int f, int r, int c)
{           // segs, face, row, column.

  if (r==0) { // Top corner of face
    if (f<4) return 0;
    return 5;
  }

  if (r==s) {
    if (((f<4)&&(c==0)) || ((f>3)&&(c==r))) return f%4+1;
    if (((f>3)&&(c==0)) || ((f<4)&&(c==r))) return (f+1)%4+1;
    if (f<4) return 6+(s-1)*(8+f)+c-1;
    return 6+(s-1)*(4+f)+s-1-c;
  }

  if (c==0) {  // r is between 0 and s.
    if (f<4) return 6+(s-1)*f + r-1;
    return 6+(s-1)*((f+1)%4+4) + r-1;
  }

  if (c==r) {
    if (f<4) return 6+(s-1)*((f+1)%4) + r-1;
    return 6+(s-1)*f + r-1;
  }

  return 6 + (s-1)*12 + f*(s-1)*(s-2)/2 + (r-1)*(r-2)/2 + c-1;
}

int hemi_octa_find_vert_index (int s, int f, int r, int c)
{               // segs, face, row, column.
  if (r==0) return 0;

  if (r==s) { // Bottom row.
    if (c==0) return f+1;
    if (c==r) return (f+1)%4+1;
    return 5+(s-1)*(4+f)+c-1;
  }

  if (c==0) return 5+(s-1)*f + r-1;
  if (c==r) return 5+(s-1)*((f+1)%4) + r-1;

  return 5 + (s-1)*8 + f*(s-1)*(s-2)/2 + (r-1)*(r-2)/2 + c-1;
}

int icosa_find_vert_index (int s, int f, int r, int c)
{             // segs, face, row, column

  if (r==0) { // Top corner of face
    if (f<5) { return 0; }
    if (f>14) { return 11; }
    return f-4;
  }

  if ((r==s) && (c==0)) { // Lower left corner of face
    if (f<5) { return f+1; }
    if (f<10) { return (f+4)%5 + 6; }
    if (f<15) { return (f+1)%5 + 1; }
    return (f+1)%5 + 6;
  }

  if ((r==s) && (c==s)) { // Lower right corner
    if (f<5) { return (f+1)%5+1; }
    if (f<10) { return f+1; }
    if (f<15) { return f-9; }
    return f-9;
  }

  if (r==s) { // Bottom edge
    if (f<5) { return 12 + (5+f)*(s-1) + c-1; }
    if (f<10) { return 12 + (20+(f+4)%5)*(s-1) + c-1; }
    if (f<15) { return 12 + (f-5)*(s-1) + s-1-c; }
    return 12 + (5+f)*(s-1) + s-1-c;
  }

  if (c==0) { // Left edge
    if (f<5) { return 12 + f*(s-1) + r-1; }
    if (f<10) { return 12 + (f%5+15)*(s-1) + r-1; }
    if (f<15) { return 12 + ((f+1)%5+15)*(s-1) + s-1-r; }
    return 12 + ((f+1)%5+25)*(s-1) + r-1;
  }

  if (c==r) { // Right edge
    if (f<5) { return 12 + ((f+1)%5)*(s-1) + r-1; }
    if (f<10) { return 12 + (f%5+10)*(s-1) + r-1; }
    if (f<15) { return 12 + (f%5+10)*(s-1) + s-1-r; }
    return 12 + (f%5+25)*(s-1) + r-1;
  }

  // Not an edge or corner.
  return 12 + 30*(s-1) + f*(s-1)*(s-2)/2 + (r-1)*(r-2)/2 + c-1;
}

int hemi_icosa_find_vert_index (int s, int f, int r, int c)
{               // segs, face, row, column
  int frow = f/5, f0=f%5, f1=(f+1)%5;
  int grow = f/10, g0=f%10, g9=(f+9)%10;
  int parity = f%2, h0=g0/2, h1=(h0+1)%5, h4=(h0+4)%5;

  if (r==0) { // Top corner of face
    switch (grow) {
      case 0: return frow ? f0+11 : 0;
      case 1: return h0 + 1;
      case 2: return g0 + 16;
      case 3: return parity ? h0+11 : h0+6;
    }
  }

  if ((r==s) && (c==0)) { // Lower left corner of face
    switch (grow) {
      case 0: return frow ? f1+1 : f0+1;
      case 1: return parity ? h0+6 : h4+11;
      case 2: return parity ? h1+6 : h0+11;
      case 3: return g9+16;
    }
  }

  if ((r==s) && (c==s)) { // Lower right corner
    switch (grow) {
      case 0: return frow ? f0+1 : f1+1;
      case 1: return parity ? h0+11 : h0+6;
      case 2: return parity ? h0+11 : h0+6;
      case 3: return g0+16;
    }
  }

  if (r==s) { // Bottom edge
    switch (grow) {
      case 0:
        if (frow) return 26 + (5+f0)*(s-1) + s-1-c;
        return 26 + (5+f0)*(s-1) + c-1;
      case 1:
        if (parity) return 26 + (h0+25)*(s-1) + c-1;
        return 26 + (h0+30)*(s-1) + c-1;
      case 2:
        if (parity) return 26 + (h1+30)*(s-1) + s-1-c;
        return 26 + (h0+25)*(s-1) + s-1-c;
      case 3:     return 26 + (g9+55)*(s-1) + c-1;
    }
  }

  if (c==0) { // Left edge
    switch (grow) {
      case 0:
        if (frow) return 26 + (20+f1)*(s-1) + s-1-r;
        return 26 + f0*(s-1) + r-1;
      case 1:
        if (parity) return 26 + (h0+10)*(s-1) + r-1;
        return 26 + (h0+20)*(s-1) + r-1;
      case 2:
        if (parity) return 26 + (38+h0*4)*(s-1) + s-1-r;
        return 26 + (36+h0*4)*(s-1) + s-1-r;
      case 3:     return 26 + (g9*2+36)*(s-1) + r-1;
    }
  }

  if (c==r) { // Right edge
    switch (grow) {
      case 0:
        if (frow) return 26 + (15+f0)*(s-1) + s-1-r;
        return 26 + f1*(s-1) + r-1;
      case 1:
        if (parity) return 26 + (h0+15)*(s-1) + r-1;
        return 26 + (h0+10)*(s-1) + r-1;
      case 2:
        if (parity) return 26 + (37+h0*4)*(s-1) + s-1-r;
        return 26 + (35+h0*4)*(s-1) + s-1-r;
      case 3:     return 26 + (g0*2+35)*(s-1) + r-1;
    }
  }

  // Not an edge or corner.
  return 26 + 65*(s-1) + f*(s-1)*(s-2)/2 + (r-1)*(r-2)/2 + c-1;
}

int find_vert_index (int basetype, int segs, int face, int row, int column)
{
  switch (basetype) {
    case 0: return tetra_find_vert_index (segs, face, row, column);
    case 1: return hemi_tetra_find_vert_index (segs, face, row, column);
    case 2: return octa_find_vert_index (segs, face, row, column);
    case 3: return hemi_octa_find_vert_index (segs, face, row, column);
    case 4: return icosa_find_vert_index (segs, face, row, column);
    default: return hemi_icosa_find_vert_index (segs, face, row, column);
  }
}

// Misc stuff
#define MAX_SEGMENTS  200
#define MIN_SEGMENTS  1

#define MIN_RADIUS    float(0)
#define MAX_RADIUS    float(1.0E30)

#define DEF_SEGMENTS  4
#define DEF_RADIUS    float(0.0)
#define DEF_BASETYPE  2
#define DEF_SMOOTH    1
#define DEF_HEMI    0
#define DEF_BASEPIVOT 0
#define DEF_MAPPING   0

// Now put it all together sensibly
#define EPSILON 1e-5f
void Geosphere_Count(int basetype, float radius, int segs, int hemi, tU32& nverts, tU32& nfaces, tU32& nsections) {

  basetype = basetype*2 + hemi;
  switch (basetype) {
    case 0:  nsections = 4;  break;
    case 1:  nsections = 3;  break;
    case 2:  nsections = 8;  break;
    case 3:  nsections = 4;  break;
    case 4:  nsections = 20; break;
    default: nsections = 40; break;
  }

  if (hemi) {
    nfaces = nsections * segs*segs + ((basetype==5) ? (segs*10) : (segs*nsections));
    nverts = nsections * (segs-1)*(segs-2)/2 + 1; // face verts, including origin.
    switch (basetype) {
      case 1:
        nverts += 6*(segs-1) + 4;
        break;
      case 3:
        nverts += 8*(segs-1) + 5;
        break;
      case 5:
        nverts += 65*(segs-1) + 26;
        break;
    }
  } else {
    nfaces = nsections * segs * segs;
    nverts = nfaces/2 + 2;
  }
}

void Geosphere_Build(Mesh& mesh, int basetype, float radius, int segs, int hemi, const tU32 nverts, const tU32 nfaces, const tU32 nsections)
{
  int i;
  int nf=0, nv=0;
  int row, column, face, a, b, c, d;
  float subrad, subz, theta, sn, cs;

  switch (basetype) {
    case 0: // Based on tetrahedron
      // First four tetrahedral vertices
      mesh.setVert (nv++, (float)0.0, (float)0.0, radius);
      mesh.setVert (nv++, radius*(Sqrt(8.f/9.f)), (float)0.0, -radius/((float)3.));
      mesh.setVert (nv++, -radius*(Sqrt(2.f/9.f)), radius*(Sqrt(2.f/3.f)), -radius/((float)3.));
      mesh.setVert (nv++, -radius*(Sqrt(2.f/9.f)), -radius*(Sqrt(2.f/3.f)), -radius/((float)3.));

      // Edge vertices
      SphericalInterpolate (mesh, 0, 1, &nv, segs);
      SphericalInterpolate (mesh, 0, 2, &nv, segs);
      SphericalInterpolate (mesh, 0, 3, &nv, segs);
      SphericalInterpolate (mesh, 1, 2, &nv, segs);
      SphericalInterpolate (mesh, 1, 3, &nv, segs);
      SphericalInterpolate (mesh, 2, 3, &nv, segs);

      // Face vertices
      for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+i, 4+(segs-1)+i, &nv, i+1);
      for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+(segs-1)+i, 4+2*(segs-1)+i, &nv, i+1);
      for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+2*(segs-1)+i, 4+i, &nv, i+1);
      for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+4*(segs-1)+i, 4+3*(segs-1)+i, &nv, i+1);
      break;

    case 1: // Hemi-tetrahedron
      // Modified four tetrahedral verts.  (Yes, I'm making this up -- no, it's not really geodesic)
      mesh.setVert (nv++, 0.0f, 0.0f, radius);
      mesh.setVert (nv++, radius, 0.0f, 0.0f);
      mesh.setVert (nv++, -radius/2.0f, radius*(ni::Sqrt(0.75f)), 0.0f);
      mesh.setVert (nv++, -radius/2.0f, -radius*(ni::Sqrt(0.75f)), 0.0f);

      // Edge Vertices
      SphericalInterpolate (mesh, 0, 1, &nv, segs);
      SphericalInterpolate (mesh, 0, 2, &nv, segs);
      SphericalInterpolate (mesh, 0, 3, &nv, segs);
      SphericalInterpolate (mesh, 1, 2, &nv, segs);
      SphericalInterpolate (mesh, 2, 3, &nv, segs);
      SphericalInterpolate (mesh, 3, 1, &nv, segs);

      // Face vertices
      for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+i, 4+(segs-1)+i, &nv, i+1);
      for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+(segs-1)+i, 4+2*(segs-1)+i, &nv, i+1);
      for (i=1; i<segs-1; i++) SphericalInterpolate (mesh, 4+2*(segs-1)+i, 4+i, &nv, i+1);

      // Last point:
      mesh.setVert (nv++, 0.0f, 0.0f, 0.0f);

      break;

    case 2: //Based on the Octahedron
      // First 6 octahedral vertices
      mesh.setVert (nv++, (float)0, (float)0, radius);
      mesh.setVert (nv++, radius, (float)0, (float)0);
      mesh.setVert (nv++, (float)0, radius, (float)0);
      mesh.setVert (nv++, -radius, (float)0, (float)0);
      mesh.setVert (nv++, (float)0, -radius, (float)0);
      mesh.setVert (nv++, (float)0, (float)0, -radius);

      // Edge vertices
      for (face=0; face<4; face++) SphericalInterpolate (mesh, 0, face+1, &nv, segs);
      for (face=0; face<4; face++) SphericalInterpolate (mesh, 5, face+1, &nv, segs);
      for (face=0; face<4; face++) SphericalInterpolate (mesh, face+1, (face+1)%4+1, &nv, segs);

      // Face vertices
      for (face=0; face<4; face++) {
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 6+face*(segs-1)+i, 6+((face+1)%4)*(segs-1)+i, &nv, i+1);
        }
      }
      for (face=0; face<4; face++) {
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 6+((face+1)%4+4)*(segs-1)+i, 6+(face+4)*(segs-1)+i, &nv, i+1);
        }
      }
      break;

    case 3: // Hemioctahedron -- this one's perfect every time.
      // First 5 octahedral vertices
      mesh.setVert (nv++, (float)0, (float)0, radius);
      mesh.setVert (nv++, radius, (float)0, (float)0);
      mesh.setVert (nv++, (float)0, radius, (float)0);
      mesh.setVert (nv++, -radius, (float)0, (float)0);
      mesh.setVert (nv++, (float)0, -radius, (float)0);

      // Edge vertices
      for (face=0; face<4; face++) SphericalInterpolate (mesh, 0, face+1, &nv, segs);
      for (face=0; face<4; face++) SphericalInterpolate (mesh, face+1, (face+1)%4+1, &nv, segs);

      // Face vertices
      for (face=0; face<4; face++) {
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 5+face*(segs-1)+i, 5+((face+1)%4)*(segs-1)+i, &nv, i+1);
        }
      }

      // Last point:
      mesh.setVert (nv++, 0.0f, 0.0f, 0.0f);
      break;

    case 4:  // Based on the Icosahedron
      // First 12 icosahedral vertices
      mesh.setVert (nv++, (float)0, (float)0, radius);
      subz = Sqrt (.2f) * radius;
      subrad = 2*subz;
      for (face=0; face<5; face++) {
        theta = 2*niPif*(tF32)face/5;
        SinCos (theta, &sn, &cs);
        mesh.setVert (nv++, subrad*cs, subrad*sn, subz);
      }
      for (face=0; face<5; face++) {
        theta = niPif*(tF32)(2*face+1)/5.0f;
        SinCos (theta, &sn, &cs);
        mesh.setVert (nv++, subrad*cs, subrad*sn, -subz);
      }
      mesh.setVert (nv++, (float)0, (float)0, -radius);

      // Edge vertices: 6*5*(segs-1) of these.
      for (face=0; face<5; face++) SphericalInterpolate (mesh, 0, face+1, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, (face+1)%5+1, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, face+6, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, (face+4)%5+6, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+6, (face+1)%5+6, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, 11, face+6, &nv, segs);

      // Face vertices: 4 rows of 5 faces each.
      for (face=0; face<5; face++) {
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 12+face*(segs-1)+i, 12+((face+1)%5)*(segs-1)+i, &nv, i+1);
        }
      }
      for (face=0; face<5; face++) {
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 12+(face+15)*(segs-1)+i, 12+(face+10)*(segs-1)+i, &nv, i+1);
        }
      }
      for (face=0; face<5; face++) {
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 12+((face+1)%5+15)*(segs-1)+segs-2-i, 12+(face+10)*(segs-1)+segs-2-i, &nv, i+1);
        }
      }
      for (face=0; face<5; face++) {
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 12+((face+1)%5+25)*(segs-1)+i, 12+(face+25)*(segs-1)+i, &nv, i+1);
        }
      }
      break;

    case 5:  // HemiIcosahedron, based loosely on a segs-2 icosahedron
      // First 26 icosahedral vertices:
      mesh.setVert (nv++, 0.0f, 0.0f, radius);
      subz = Sqrt (.2f) * radius;
      subrad = 2*subz;
      for (face=0; face<5; face++) {
        theta = 2*niPif*(tF32)face/5.0f;
        SinCos (theta, &sn, &cs);
        mesh.setVert (face+6, subrad*cs, subrad*sn, subz);
        SphericalInterpolate (mesh, 0, face+6, &nv, 2);
      }
      nv += 5;
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+6, (face+1)%5+6, &nv, 2);
      for (face=0; face<10; face++) {
        theta = 2.0f*niPif*(tF32)face/10.0f + (niPif/10.0f);
        SinCos (theta, &sn, &cs);
        mesh.setVert (nv++, radius*cs, radius*sn, 0.0f);
      }

      // Edge vertices: 13*5*(segs-1) of these.
      for (face=0; face<5; face++) SphericalInterpolate (mesh, 0, face+1, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, (face+1)%5+1, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, face+6, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, face+11, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+1, (face+4)%5+11, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, face+6, face+11, &nv, segs);
      for (face=0; face<5; face++) SphericalInterpolate (mesh, (face+4)%5+11, face+6, &nv, segs);
      for (face=0; face<5; face++) {
        SphericalInterpolate (mesh, face+6, face*2+16, &nv, segs);
        SphericalInterpolate (mesh, face+11, face*2+16, &nv, segs);
        SphericalInterpolate (mesh, face+11, face*2+17, &nv, segs);
        SphericalInterpolate (mesh, (face+1)%5+6, face*2+17, &nv, segs);
      }
      for (face=0; face<10; face++) SphericalInterpolate (mesh, face+16, (face+1)%10+16, &nv, segs);

      // Face vertices: representing 40 faces, 8 groups of 5.
      for (face=0; face<5; face++) {  // Top 5:
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 26+face*(segs-1)+i, 26+((face+1)%5)*(segs-1)+i, &nv, i+1);
        }
      }
      for (face=0; face<5; face++) {  // Stellation of above
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 26+((face+1)%5+20)*(segs-1)+segs-2-i, 26+(face+15)*(segs-1)+segs-2-i, &nv, i+1);
        }
      }
      for (face=0; face<5; face++) {  // Rest of this level
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 26+(face+20)*(segs-1)+i, 26+(face+10)*(segs-1)+i, &nv, i+1);
        }
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 26+(face+10)*(segs-1)+i, 26+(face+15)*(segs-1)+i, &nv, i+1);
        }
      }
      for (face=0; face<10; face++) { // Downward-pointing faces in lowest level
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 26+(face*2+36)*(segs-1)+segs-2-i, 26+(face*2+35)*(segs-1)+segs-2-i, &nv, i+1);
        }
      }
      for (face=0; face<10; face++) { // Upward-pointing faces in lowest level
        int f9 = (face+9)%10;
        for (i=1; i<segs-1; i++) {
          SphericalInterpolate (mesh, 26+(f9*2+36)*(segs-1)+i, 26+(face*2+35)*(segs-1)+i, &nv, i+1);
        }
      }

      mesh.setVert (nv++, 0.0f, 0.0f, 0.0f);
      break;
  }

  for (face=0; face<nsections; face++) {
    for (row=0; row<segs; row++) {
      for (column=0; column<=row; column++) {
        a = find_vert_index (basetype, segs, face, row, column);
        b = find_vert_index (basetype, segs, face, row+1, column);
        c = find_vert_index (basetype, segs, face, row+1, column+1);
        mesh.setFaceVerts(nf, a, b, c);
        nf++;
        if (column<row) {
          d = find_vert_index (basetype, segs, face, row, column+1);
          mesh.setFaceVerts(nf, a, c, d);
          nf++;
        }
      }
    }
  }

  if (hemi) {
    int bfaces = 0, bfoffset = 0;
    switch (basetype) {
      case 1: bfaces=3, bfoffset=0; break;
      case 3: bfaces=4, bfoffset=0; break;
      case 5: bfaces=10, bfoffset=30; break;
    }
    for (face=0; face<bfaces; face++) {
      for (column=0; column<segs; column++) {
        a = find_vert_index (basetype, segs, face+bfoffset, segs, column);
        b = find_vert_index (basetype, segs, face+bfoffset, segs, column+1);
        c = nv-1;
        mesh.setFaceVerts(nf, b, a, c);
        nf++;
      }
    }
  }
}

///////////////////////////////////////////////
iGeometry* GeoSphere(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
                              int type, tBool abHemi, tF32 radius, tI32 slices,
                              tBool abCW, tU32 aulColor, const sMatrixf& amtxUV)
{
  tU32 nNumVerts, nNumFaces, nNumSections;

  Geosphere_Count(type,radius,slices,eFalse,nNumVerts,nNumFaces,nNumSections);

  iGeometry* pGeom = apGraphics->CreateGeometryPolygonal(nNumVerts,nNumFaces*3,aFlags,aFVF);
  if (!niIsOK(pGeom)) {
    niError(_A("Can't create destination geometry."));
    return NULL;
  }

  cFVFDescription fvfDesc(pGeom->GetVertexArray()->GetFVF());
  cFVFStream fvfStream(&fvfDesc,pGeom->GetVertexArray()->Lock(0,0,eLock_Normal),nNumVerts);
  tIndex* pInds = (tIndex*)pGeom->GetIndexArray()->Lock(0,0,eLock_Normal);

  Mesh mesh(fvfStream.GetBase(),nNumVerts,fvfDesc.GetStride(),pInds,nNumFaces);
  Geosphere_Build(mesh,type,radius,slices,abHemi,nNumVerts,nNumFaces,nNumSections);

  if (fvfDesc.HasColorA()) {
    fvfStream.Reset();
    niLoop(i,mesh.getNumVerts()) {
      //if (fvfDesc.HasColorA())
      {
        fvfStream.Vertex().ColorA() = aulColor;
      }
      fvfStream.Next();
    }
  }

  if (!abCW) {
    // flip face winding
    niLoop(i,mesh.getNumFaces()) {
      sVec3<tIndex> f = mesh.getFaceVerts(i);
      *pInds++ = f.z;
      *pInds++ = f.y;
      *pInds++ = f.x;
    }
  }

  pGeom->GetVertexArray()->Unlock();
  pGeom->GetIndexArray()->Unlock();
  pGeom->AddSubset(0,0,pGeom->GetIndexArray()->GetNumIndices(),0);
  return pGeom;
}
