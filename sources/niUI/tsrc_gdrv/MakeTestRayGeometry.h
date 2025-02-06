#pragma once
#ifndef __MAKETESTRAYGEOMETRY_H_31778F7A_CDAF_4F90_9AAF_3D6002C6B3F9__
#define __MAKETESTRAYGEOMETRY_H_31778F7A_CDAF_4F90_9AAF_3D6002C6B3F9__

namespace ni {

typedef sVertexPA tVertexTri;

inline NN<iGpuBuffer> MakeTriVB(ain<nn<iGraphicsDriverGpu>> aGpu, tU32 anId, tF32 afSize, ain<sVec3f> aPos) {
  niLet triVB = AsNN(aGpu->CreateGpuBuffer(
    HFmt("TriVB_%d",anId),
    sizeof(tVertexTri)*3,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Vertex|
    eGpuBufferUsageFlags_RayBuildInput));
  {
    niLet w = afSize/2.0f;
    tVertexTri* verts = (tVertexTri*)triVB->Lock(0, triVB->GetSize(), eLock_Discard);
    niPanicAssert(verts != nullptr);
    // Red, TC
    verts[0] = {{ aPos.x, w+aPos.y, aPos.z}, 0xFFFF0000};
    // Green, BR
    verts[1] = {{ w+aPos.x, -w+aPos.y, aPos.z}, 0xFF00FF00};
    // Blue, BL
    verts[2] = {{ -w+aPos.x, -w+aPos.y, aPos.z}, 0xFF0000FF};
    triVB->Unlock();
  }
  return triVB;
}

inline NN<iGpuBuffer> MakeTriIB(ain<nn<iGraphicsDriverGpu>> aGpu, tU32 anId, tU32 aNumTris) {
  niLet triIB = AsNN(aGpu->CreateGpuBuffer(
    HFmt("TriIB_%d",anId),
    sizeof(tU32)*aNumTris*6,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Index|
    eGpuBufferUsageFlags_RayBuildInput));
  tU32 baseIndex = 0;
  {
    tU32* inds = (tU32*)triIB->Lock(0, triIB->GetSize(), eLock_Discard);
    niPanicAssert(inds != nullptr);
    niLoop(i,aNumTris) {
      inds[0] = baseIndex+0;
      inds[1] = baseIndex+1;
      inds[2] = baseIndex+2;
      inds += 3;
      baseIndex += 3;
    }
    triIB->Unlock();
  }
  return triIB;
}

// 25 degree-ish rotated quad
inline NN<iGpuBuffer> MakeQuadVB(ain<nn<iGraphicsDriverGpu>> aGpu, tU32 anId, tF32 afSize, ain<sVec3f> aPos) {
  // Create vertex buffer with quad geometry
  niLet quadVB = AsNN(aGpu->CreateGpuBuffer(
    HFmt("QuadVB_%d",anId),
    sizeof(tVertexTri)*4,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Vertex|
    eGpuBufferUsageFlags_RayBuildInput));
  {
    tVertexTri* verts = (tVertexTri*)quadVB->Lock(0, quadVB->GetSize(), eLock_Discard);
    niPanicAssert(verts != nullptr);
    // Red, TL
    verts[0] = {{ -0.35f*afSize+aPos.x, 0.6f*afSize+aPos.y, aPos.z}, 0xFFFF0000};
    // Green, TR
    verts[1] = {{  0.6f*afSize+aPos.x, 0.35f*afSize+aPos.y, aPos.z}, 0xFF00FF00};
    // Blue, BR
    verts[2] = {{  0.35f*afSize+aPos.x, -0.6f*afSize+aPos.y, aPos.z}, 0xFF0000FF};
    // White, BL
    verts[3] = {{ -0.6f*afSize+aPos.x, -0.35f*afSize+aPos.y, aPos.z}, 0xFFFFFFFF};
    quadVB->Unlock();
  }
  return quadVB;
}

inline NN<iGpuBuffer> MakeQuadIB(ain<nn<iGraphicsDriverGpu>> aGpu, tU32 anId, tU32 aNumQuads) {
  niLet quadIB = AsNN(aGpu->CreateGpuBuffer(
    HFmt("QuadIB_%d",anId),
    sizeof(tU32)*aNumQuads*6,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Index|
    eGpuBufferUsageFlags_RayBuildInput));
  tU32 baseIndex = 0;
  {
    tU32* inds = (tU32*)quadIB->Lock(0, quadIB->GetSize(), eLock_Discard);
    niPanicAssert(inds != nullptr);
    niLoop(i,aNumQuads) {
      inds[0] = baseIndex+0;
      inds[1] = baseIndex+1;
      inds[2] = baseIndex+2;
      inds[3] = baseIndex+2;
      inds[4] = baseIndex+3;
      inds[5] = baseIndex+0;
      inds += 6;
      baseIndex += 4;
    }
    quadIB->Unlock();
  }
  return quadIB;
}

}
#endif // __MAKETESTRAYGEOMETRY_H_31778F7A_CDAF_4F90_9AAF_3D6002C6B3F9__
