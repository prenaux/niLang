#ifndef __RAYUTILS_H_D7E1ED73_ACC5_4013_A7E9_B849E4A2A1F9__
#define __RAYUTILS_H_D7E1ED73_ACC5_4013_A7E9_B849E4A2A1F9__

#include <niCC.h>
#include <niUI.h>
#include <niUI/nish/niUIGpuFuncs.hpp>

namespace ni {

using namespace astl;

typedef sVertexPNAT1 tVertexRay;
typedef sVertexPA tVertexTri;

template <typename T, typename IBUFFER_T>
struct sAutoLockBuffer {
  NN<IBUFFER_T> _buffer;
  astl::span<T> _span;

  sAutoLockBuffer(ain_nn<IBUFFER_T> aBuffer, ain<tU32> anOffset,
                  ain<tU32> anMaybeSize, ain<eLock> aFlags)
      : _buffer(aBuffer)
  {
    tU32 lockSize =
      (anMaybeSize == eInvalidHandle) ? aBuffer->GetSize() : anMaybeSize;
    const tU32 elementSize = sizeof(T);
    const tU32 numElements = lockSize / elementSize;
    niPanicAssertMsg((numElements >= 1) && (lockSize >= elementSize),
                     "Must lock at least one element.");
    T* data = reinterpret_cast<T*>(_buffer->Lock(anOffset, lockSize, aFlags));
    niPanicAssertMsg(data != nullptr, "Cant lock buffer.");
    _span = astl::span<T>(data, numElements);
  }

  ~sAutoLockBuffer()
  {
    if (!_span.empty()) {
      _buffer->Unlock();
    }
  }

  T* data() const
  {
    return _span.data();
  }

  T* operator->() const
  {
    return data();
  }

  T& operator[](tU32 anIdx) const
  {
    return _span[anIdx];
  }

  bool empty() const
  {
    return _span.empty();
  }

  tU32 size() const
  {
    return (tU32)_span.size();
  }

  tU32 size_bytes() const
  {
    return (tU32)_span.size_bytes();
  }
};

template <typename T, IsNonNullType IBUFFER_T>
inline auto AutoLockBufferReadOnly(IBUFFER_T&& aBuffer, tU32 anOffset = 0,
                                   tU32 anSize = eInvalidHandle)
{
  using BufferType = typename remove_cvref_t<IBUFFER_T>::element_type;
  return sAutoLockBuffer<const T, BufferType>(std::forward<IBUFFER_T>(aBuffer),
                                              anOffset, anSize, eLock_ReadOnly);
}

template <typename T, IsNonNullType IBUFFER_T>
inline auto AutoLockBufferReadWrite(IBUFFER_T&& aBuffer, tU32 anOffset = 0,
                                    tU32 anSize = eInvalidHandle)
{
  using BufferType = typename remove_cvref_t<IBUFFER_T>::element_type;
  return sAutoLockBuffer<T, BufferType>(std::forward<IBUFFER_T>(aBuffer),
                                        anOffset, anSize, eLock_Normal);
}

template <typename T, IsNonNullType IBUFFER_T>
inline auto AutoLockBufferDiscard(IBUFFER_T&& aBuffer, tU32 anOffset = 0,
                                  tU32 anSize = eInvalidHandle)
{
  using BufferType = typename remove_cvref_t<IBUFFER_T>::element_type;
  return sAutoLockBuffer<T, BufferType>(std::forward<IBUFFER_T>(aBuffer),
                                        anOffset, anSize, eLock_Discard);
}

inline NN<iGpuBuffer> MakeTriVB(ain_nn<iGraphicsDriverGpu> aGpu, tU32 anId,
                                tF32 afSize, ain<sVec3f> aPos)
{
  niLet triVB = AsNN(aGpu->CreateGpuBuffer(
    HFmt("TriVB_%d", anId), sizeof(tVertexTri) * 3, eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Vertex | eGpuBufferUsageFlags_RayBuildInput));
  {
    niLet w = afSize / 2.0f;
    niLet verts = AutoLockBufferDiscard<tVertexTri>(triVB);
    // Red, TC
    verts[0] = { { aPos.x, w + aPos.y, aPos.z }, 0xFFFF0000 };
    // Green, BR
    verts[1] = { { w + aPos.x, -w + aPos.y, aPos.z }, 0xFF00FF00 };
    // Blue, BL
    verts[2] = { { -w + aPos.x, -w + aPos.y, aPos.z }, 0xFF0000FF };
  }
  return triVB;
}

inline NN<iGpuBuffer> MakeTriIB(ain<nn<iGraphicsDriverGpu>> aGpu, tU32 anId,
                                tU32 aNumTris)
{
  niLet triIB = AsNN(aGpu->CreateGpuBuffer(
    HFmt("TriIB_%d", anId), sizeof(tU32) * aNumTris * 6,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Index | eGpuBufferUsageFlags_RayBuildInput));
  tU32 baseIndex = 0;
  {
    niLet inds = AutoLockBufferDiscard<tU32>(triIB);
    niLoop (i, aNumTris) {
      niLet indsBase = i * 3;
      inds[indsBase + 0] = baseIndex + 0;
      inds[indsBase + 1] = baseIndex + 1;
      inds[indsBase + 2] = baseIndex + 2;
      baseIndex += 3;
    }
  }
  return triIB;
}

// 25 degree-ish rotated quad
inline NN<iGpuBuffer> MakeQuadVB(ain<nn<iGraphicsDriverGpu>> aGpu, tU32 anId,
                                 tF32 afSize, ain<sVec3f> aPos)
{
  // Create vertex buffer with quad geometry
  niLet quadVB = AsNN(aGpu->CreateGpuBuffer(
    HFmt("QuadVB_%d", anId), sizeof(tVertexTri) * 4,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Vertex | eGpuBufferUsageFlags_RayBuildInput));
  {
    niLet verts = AutoLockBufferDiscard<tVertexTri>(quadVB);
    // Red, TL
    verts[0] = { { -0.35f * afSize + aPos.x, 0.6f * afSize + aPos.y, aPos.z },
                 0xFFFF0000 };
    // Green, TR
    verts[1] = { { 0.6f * afSize + aPos.x, 0.35f * afSize + aPos.y, aPos.z },
                 0xFF00FF00 };
    // Blue, BR
    verts[2] = { { 0.35f * afSize + aPos.x, -0.6f * afSize + aPos.y, aPos.z },
                 0xFF0000FF };
    // White, BL
    verts[3] = { { -0.6f * afSize + aPos.x, -0.35f * afSize + aPos.y, aPos.z },
                 0xFFFFFFFF };
  }
  return quadVB;
}

inline NN<iGpuBuffer> MakeQuadIB(ain<nn<iGraphicsDriverGpu>> aGpu, tU32 anId,
                                 tU32 aNumQuads)
{
  niLet quadIB = AsNN(aGpu->CreateGpuBuffer(
    HFmt("QuadIB_%d", anId), sizeof(tU32) * aNumQuads * 6,
    eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Index | eGpuBufferUsageFlags_RayBuildInput));
  tU32 baseIndex = 0;
  {
    niLet inds = AutoLockBufferDiscard<tU32>(quadIB);
    niLoop (i, aNumQuads) {
      niLet indsBase = i * 6;
      inds[indsBase + 0] = baseIndex + 0;
      inds[indsBase + 1] = baseIndex + 1;
      inds[indsBase + 2] = baseIndex + 2;
      inds[indsBase + 3] = baseIndex + 2;
      inds[indsBase + 4] = baseIndex + 3;
      inds[indsBase + 5] = baseIndex + 0;
      baseIndex += 4;
    }
  }
  return quadIB;
}

inline Ptr<iRayPrimitives> CreateRayPrimsFromDop(
  ain<nn<iHString>> aName, ain<nn<iGraphicsDriverRay>> aDriverRay,
  ain<nn<iRayBuildEncoder>> aBuildEncoder, ain<nn<iDrawOperation>> aDop)
{
  niCheck(tVertexRay::eFVF == aDop->GetVertexArray()->GetFVF(), nullptr);

  niLet fvfDesc = cFVFDescription(aDop->GetVertexArray()->GetFVF());
  NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(aDop->GetVertexArray()));
  NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(aDop->GetIndexArray()));

  niLet prDesc = niCheckNN(prDesc,
                           aDriverRay->CreateRayTrianglePrimitivesDesc(
                             HFmt("%s_RayTriPrimsDesc", aName)),
                           nullptr);

  niLet firstInd = aDop->GetFirstIndex();
  niVar numInds = aDop->GetNumIndices();
  if (!numInds) {
    numInds = aDop->GetIndexArray()->GetNumIndices() - firstInd;
  }
  niCheck(prDesc->AddTrianglesIndexed(
            vaBuffer, aDop->GetBaseVertexIndex() * fvfDesc.GetStride(),
            fvfDesc.GetStride(), aDop->GetVertexArray()->GetNumVertices(),
            iaBuffer, firstInd * sizeof(tU32), eGpuIndexType_U32, numInds,
            sMatrixf::Identity(), eRayPrimitiveFlags_Opaque, 0),
          nullptr);

  return aBuildEncoder->BuildRayTrianglePrimitives(HFmt("%s_RayPrims", aName),
                                                   prDesc);
}

// A quad in NDC coordinates. [-1;1]
struct sDisplayQuad {
  NN<iGpuFunction> _displayVertexGpuFun;
  NN<iGpuFunction> _displayPixelGpuFun;
  NN<iGpuPipeline> _displayPipeline;
};

inline optional<sDisplayQuad> MakeDisplayQuad(
  ain_nn<iGraphicsDriverGpu> aDriverGpu, iHString* ahspRayqueryGpufuncPath)
{
  // Setup display pipeline
  niLet displayVertexGpuFun =
    niCheckNN(displayVertexGpuFun,
              aDriverGpu->CreateGpuFunction(
                eGpuFunctionType_Vertex,
                _H("niUI://nish/fixed/fixed_blit_vs.gpufunc.xml")),
              nullopt);

  niLet displayPixelGpuFun =
    niCheckNN(displayPixelGpuFun,
              aDriverGpu->CreateGpuFunction(eGpuFunctionType_Pixel,
                                            ahspRayqueryGpufuncPath),
              nullopt);

  niLet pipelineDesc =
    niCheckNN(pipelineDesc, aDriverGpu->CreateGpuPipelineDesc(), nullopt);
  pipelineDesc->SetFVF(tVertexCanvas::eFVF);
  pipelineDesc->SetColorFormat(0, eGpuPixelFormat_BGRA8);
  pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
  pipelineDesc->SetFunction(eGpuFunctionType_Vertex, displayVertexGpuFun);
  pipelineDesc->SetFunction(eGpuFunctionType_Pixel, displayPixelGpuFun);
  niLet displayPipeline = niCheckNN(
    displayPipeline,
    aDriverGpu->CreateGpuPipeline(_H("DisplayQuad_Pipeline"), pipelineDesc),
    nullopt);

  return sDisplayQuad{
    ._displayVertexGpuFun = displayVertexGpuFun,
    ._displayPixelGpuFun = displayPixelGpuFun,
    ._displayPipeline = displayPipeline,
  };
}

inline void DisplayTexture(ain<sDisplayQuad> aThis,
                           ain_nn<iGpuCommandEncoder> cmdEncoder,
                           iTexture* texture,
                           ain<sVec2f> aTL = Vec2f(-0.75f, -0.75f),
                           ain<sVec2f> aBR = Vec2f(0.75f, 0.75f),
                           ain<tF32> aZ = 0.0f)
{
  tVertexCanvas verts[6];
  // TL
  verts[0] = {
    { aTL.x, aTL.y, aZ }, sVec3f::YAxis(), 0xFFFFFFFF, { 0.0f, 0.0f }
  };
  // TR
  verts[1] = {
    { aBR.x, aTL.y, aZ }, sVec3f::YAxis(), 0xFFFFFFFF, { 1.0f, 0.0f }
  };
  // BR
  verts[2] = {
    { aBR.x, aBR.y, aZ }, sVec3f::YAxis(), 0xFFFFFFFF, { 1.0f, 1.0f }
  };
  // BL
  verts[3] = {
    { aTL.x, aBR.y, aZ }, sVec3f::YAxis(), 0xFFFFFFFF, { 0.0f, 1.0f }
  };
  // finish 2nd triangle
  verts[4] = verts[0];
  verts[5] = verts[2];

  cmdEncoder->SetPipeline(aThis._displayPipeline);
  cmdEncoder->StreamVertexBuffer((tPtr)verts, sizeof(verts), 0);
  cmdEncoder->SetTexture(texture, 0);
  cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 0);
  cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList, 0, 1, 0, 6);
}

inline tU32 GetTextureResourceIndex(ain_nn<iGraphics> aGraphics,
                                    iTexture* apTexture)
{
  if (!apTexture)
    return 0;
  tU32 r = aGraphics->GetTextureDeviceResourceManager()->GetIndexFromResource(
    apTexture);
  return (r == eInvalidHandle) ? 0 : r;
}

inline tU32 RayGetMaterialColor(iMaterial* apMaterial,
                                tU32 anErrorColor = 0xFFFF00FF)
{
  if (!apMaterial)
    return anErrorColor;
  iTexture* tex = apMaterial->GetChannelTexture(eMaterialChannel_Base);
  sColor4f col = apMaterial->GetChannelColor(eMaterialChannel_Base);
  if (!tex || (apMaterial->GetFlags() & eMaterialFlags_DiffuseModulate)) {
    return ULColorBuild(col);
  }
  return 0xFFFFFFFF;
}

inline tU32 GetBufferResourceIndex(ain_nn<iGraphicsDriverGpu> aDriverGpu,
                                   iGpuBuffer* apGpuBuffer)
{
  if (!apGpuBuffer)
    return 0;
  tU32 r =
    aDriverGpu->GetStorageBufferDeviceResourceManager()->GetIndexFromResource(
      apGpuBuffer);
  return (r == eInvalidHandle) ? 0 : r;
}

inline Ptr<iGpuBuffer> CreateRayInstanceData(
  ain_nn<iHString> ahspResName, ain_nn<iGraphicsDriverGpu> aDriverGpu,
  ain<niUIGpuFuncs_RayInstanceData> aInstData)
{
  niLet instDataBuffer =
    niCheckNN(instDataBuffer,
              aDriverGpu->CreateGpuBuffer(
                HFmt("RayInstanceData_%s", ahspResName), sizeof(aInstData),
                eGpuBufferMemoryMode_Shared, eGpuBufferUsageFlags_Storage),
              nullptr);
  niVar locked =
    AutoLockBufferDiscard<niDeclTypeBase(aInstData)>(instDataBuffer);
  locked[0] = aInstData;
  instDataBuffer->Unlock();
  return instDataBuffer;
}

inline tU32 GetMaterialTextureResourceIndex(
  ain_nn<iGraphics> aGraphics, iMaterial* apMaterial,
  eMaterialChannel aCh = eMaterialChannel_Base)
{
  if (!apMaterial)
    return 0;
  return GetTextureResourceIndex(aGraphics, apMaterial->GetChannelTexture(aCh));
}

struct sRayGeometry {
  ni::NN<iRayPrimitives> rayPrims;
  ni::NN<iGpuBuffer> rayInstData;
  ni::tU32 rayInstDataIndex = eInvalidHandle;
  ni::Ptr<iMaterial> material;
  ni::Ptr<iTransform> transform;
  ni::tU32 userIndex = eInvalidHandle;

  void UpdateTexIndex(ain<tU32> anTexResIndex)
  {
    niLet lock = AutoLockBufferReadWrite<niUIGpuFuncs_RayInstanceData>(
      rayInstData.non_null());
    niLet currentTexIndex = lock->texIndex;
    niLet newTexIndex = anTexResIndex;
    if (currentTexIndex != newTexIndex) {
      niDebugFmt((
        "... sRayGeometry::InitFromDrawOp: currentTexIndex: %d -> newTexIndex: %d",
        currentTexIndex, newTexIndex));
      lock->texIndex = newTexIndex;
    }
  }

  tBool AddInstance(ain_nn<ni::iRayInstancesDesc> aRayInstsDesc,
                    ain<sMatrixf> aWorldMatrix, ain<tU32> anInstanceMask) const
  {
    return aRayInstsDesc->AddInstance(rayPrims, aWorldMatrix, rayInstDataIndex,
                                      anInstanceMask, 0,
                                      eRayInstanceFlags_None);
  }
};

inline optional<sRayGeometry> MakeRayGeometry(
  ain_nn<iHString> resName, ain_nn<iGraphicsDriverGpu> driverGpu,
  ain_nn<iGraphicsDriverRay> driverRay,
  ain_nn<iRayBuildEncoder> rayBuildEncoder, ain_nn<iDrawOperation> drawOp,
  ain<tU32> anTexResIndex, ain<tU32> anMaterialColor)
{
  NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(drawOp->GetIndexArray()));
  NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(drawOp->GetVertexArray()));

  niLet rayPrims = niCheckNN_(
    rayPrims,
    CreateRayPrimsFromDop(resName, driverRay, rayBuildEncoder, drawOp),
    niFmt("Cant create rayprims for '%s'.", resName), nullopt);

  niUIGpuFuncs_RayInstanceData instData;
  instData.vbIndex = GetBufferResourceIndex(driverGpu, vaBuffer);
  instData.ibIndex = GetBufferResourceIndex(driverGpu, iaBuffer);
  instData.firstIndex = drawOp->GetFirstIndex();
  instData.baseVertexIndex = drawOp->GetBaseVertexIndex();
  instData.texIndex = anTexResIndex;
  instData.materialColor = anMaterialColor;
  niLet rayInstData =
    niCheckNN_(rayInstData, CreateRayInstanceData(resName, driverGpu, instData),
               niFmt("Cant create rayinstdata for '%s'.", resName), nullopt);
  niLet rayInstDataIndex =
    GetBufferResourceIndex(driverGpu, rayInstData.non_null());

  niDebugFmt((
    "... MakeRayGeometry: %s, mnRayInstDataIndex: %d, vbIndex: %d, ibIndex: %d, texIndex: %d, firstIndex: %d, numTris: %d, baseVertexIndex: %d",
    resName, rayInstDataIndex, instData.vbIndex, instData.ibIndex,
    instData.texIndex, instData.firstIndex, drawOp->GetNumIndices() / 3,
    instData.baseVertexIndex));

  return sRayGeometry{ .rayPrims = rayPrims,
                       .rayInstData = rayInstData,
                       .rayInstDataIndex = rayInstDataIndex };
}

} // namespace ni
#endif // __RAYUTILS_H_D7E1ED73_ACC5_4013_A7E9_B849E4A2A1F9__
