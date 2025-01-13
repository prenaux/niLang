#include "stdafx.h"
#include "FGDRV.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"
#include <niLang/Math/MathLib.h>

//
// TODO (1/18):
// - [ ] p0: Sphere intersection shader, visualize the sphere's normal?
// - [ ] p0: Visualize: triangles with barycentric coordinate
// - [ ] p1: Checkerboard floor
// - [ ] p1: Four reflective sphere on checkerboard floor (white, red, green, blue spheres)
// - [ ] p1: One reflective sphere on checkerboard floor
// - [ ] p1: Ref PT cornell box white-furnace sky
// - [ ] p1: Ref PT cornell box with one light on the ceiling - one emissive quad should be the light source
// - [ ] p1: Visualize: base color
// - [ ] p1: Visualize: normals
// - [ ] p1: Visualize: worldspace position
// - [ ] p2: Add dynamic primitive AS
// - [ ] p2: Compact static primitives AS
// - [ ] p2: Textured cube
// - [ ] p2: Visualize: tex coordinates
// - [x] p0: FRay-Instances: Multiple instances, four triangles (one per instance), rotating - rebuilt every frame
// - [x] p0: FRay-TriangleQuad: Multiple geometries, two triangles and a quad
// - [x] p0: Visualize: one colour per instance index & custom instance id
//

using namespace ni;

namespace {

struct sFRay_Base : public sFGDRV_Base {
  typedef sVertexPA tVertexTri;
  tU32 _numTriVB = 0;
  tU32 _numTriIB = 0;
  tU32 _numQuadVB = 0;
  tU32 _numQuadIB = 0;

  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);
  NN<iGraphicsDriverRay> _driverRay = niDeferredInit(NN<iGraphicsDriverRay>);

  // Display resources
  NN<iGpuBuffer> _displayVABuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _displayIABuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _displayVertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _displayPixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _displayPipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _textureEarth = niDeferredInit(NN<iTexture>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGDRV_Base::OnInit(testResults_),eFalse);

    {
      QPtr<iGraphicsDriverGpu> driverGpu = _graphics->GetDriver();
      CHECK_RET(niIsOK(driverGpu),eFalse);
      _driverGpu = AsNN(driverGpu.raw_ptr());
    }

    {
      QPtr<iGraphicsDriverRay> driverRay = _graphics->GetDriver();
      CHECK_RET(niIsOK(driverRay),eFalse);
      _driverRay = AsNN(driverRay.raw_ptr());
    }

    // Setup display quad
    _displayVABuffer = niCheckNN(
      _displayVABuffer,
      _driverGpu->CreateGpuBuffer(
        _H("RayDisplay_VA"),
        sizeof(tVertexCanvas)*4,
        eGpuBufferMemoryMode_Shared,
        eGpuBufferUsageFlags_Vertex),
      eFalse);
    tVertexCanvas* verts = (tVertexCanvas*)_displayVABuffer->Lock(0, _displayVABuffer->GetSize(), eLock_Discard);
    niCheck(verts != nullptr, eFalse);
    verts[0] = {{ -0.8f,  0.8f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {0.0f,0.0f}}; // TL
    verts[1] = {{  0.8f,  0.8f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {1.0f,0.0f}}; // TR
    verts[2] = {{  0.8f, -0.8f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {1.0f,1.0f}}; // BR
    verts[3] = {{ -0.8f, -0.8f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {0.0f,1.0f}}; // BL
    _displayVABuffer->Unlock();

    _displayIABuffer = niCheckNN(
      _displayIABuffer,
      _driverGpu->CreateGpuBuffer(
        _H("RayDisplay_IA"),
        sizeof(tU32)*6,
        eGpuBufferMemoryMode_Shared,
        eGpuBufferUsageFlags_Index),
      eFalse);
    tU32* inds = (tU32*)_displayIABuffer->Lock(0, _displayIABuffer->GetSize(), eLock_Discard);
    niCheck(inds != nullptr, eFalse);
    inds[0] = 0; inds[1] = 1; inds[2] = 2;
    inds[3] = 2; inds[4] = 3; inds[5] = 0;
    _displayIABuffer->Unlock();

    // Setup display pipeline
    _displayVertexGpuFun = niCheckNN(_displayVertexGpuFun,_driverGpu->CreateGpuFunction(
      eGpuFunctionType_Vertex,_H("test/gpufunc/texture_vs.gpufunc.xml")),eFalse);
    _displayPixelGpuFun = niCheckNN(_displayPixelGpuFun,_driverGpu->CreateGpuFunction(
      eGpuFunctionType_Pixel,_H("test/gpufunc/texture_ps.gpufunc.xml")),eFalse);

    NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
    pipelineDesc->SetFVF(tVertexCanvas::eFVF);
    pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
    pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
    pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_displayVertexGpuFun);
    pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_displayPixelGpuFun);
    _displayPipeline = niCheckNN(_displayPipeline, _driverGpu->CreateGpuPipeline(_H("RayDisplay_Pipeline"),pipelineDesc), eFalse);

    {
      Ptr<iFile> fp;
      fp = _graphics->OpenBitmapFile("test/tex/earth_d.jpg");
      _textureEarth = AsNN(_graphics->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default));
    }

    return eTrue;
  }

  void DisplayTexture(iGpuCommandEncoder* cmdEncoder, iTexture* texture) {
    cmdEncoder->SetPipeline(_displayPipeline);
    cmdEncoder->SetVertexBuffer(_displayVABuffer, 0, 0);
    cmdEncoder->SetTexture(texture, 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 0);
    cmdEncoder->SetIndexBuffer(_displayIABuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
  }

  NN<iGpuBuffer> MakeTriVB(tF32 afSize, ain<sVec3f> aPos) {
    niLet triVB = AsNN(_driverGpu->CreateGpuBuffer(
        HFmt("TriVB_%d",_numTriVB++),
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

  NN<iGpuBuffer> MakeTriIB(tU32 aNumTris) {
    niLet triIB = AsNN(_driverGpu->CreateGpuBuffer(
        HFmt("TriIB_%d",_numTriIB++),
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
  NN<iGpuBuffer> MakeQuadVB(tF32 afSize, ain<sVec3f> aPos) {
    // Create vertex buffer with quad geometry
    niLet quadVB = AsNN(_driverGpu->CreateGpuBuffer(
      HFmt("QuadVB_%d",_numQuadVB++),
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

  NN<iGpuBuffer> MakeQuadIB(tU32 aNumQuads) {
    niLet quadIB = AsNN(_driverGpu->CreateGpuBuffer(
        HFmt("QuadIB_%d",_numQuadIB++),
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
};

// clear ; ham pass1 && ham Run_Test_niUI_GDRV FIXTURE=FRay,Triangle A2=-Drenderer=Vulkan BUILD=da
struct sFRay_Triangle : public sFRay_Base {

  // Ray tracing instances, pipeline and shaders
  NN<iRayInstances> _instanceAS = niDeferredInit(NN<iRayInstances>);
  NN<iGpuFunction> _rayGenFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayMissFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayHitFun = niDeferredInit(NN<iGpuFunction>);
  NN<iRayFunctionTable> _rayFuncTable = niDeferredInit(NN<iRayFunctionTable>);
  NN<iRayPipeline> _rayPipeline = niDeferredInit(NN<iRayPipeline>);
  NN<iTexture> _rayOutputImage = niDeferredInit(NN<iTexture>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRay_Base::OnInit(testResults_),eFalse);

    // Create ray tracing shaders
    {
      _rayGenFun = niCheckNN(_rayGenFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayGeneration, _H("test/rayfunc/triangle_rgen.gpufunc.xml")), eFalse);

      _rayMissFun = niCheckNN(_rayMissFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayMiss, _H("test/rayfunc/triangle_rmiss.gpufunc.xml")), eFalse);

      _rayHitFun = niCheckNN(_rayHitFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayClosestHit, _H("test/rayfunc/triangle_rchit.gpufunc.xml")), eFalse);
    }

    // Create ray tracing pipeline
    {
      _rayFuncTable = niCheckNN(_rayFuncTable, _driverRay->CreateRayFunctionTable(), eFalse);
      _rayFuncTable->SetRayGenFunction(_rayGenFun);
      _rayFuncTable->SetMissFunction(_rayMissFun);

      // Add hit group for triangles
      niLet hitGroupId = _rayFuncTable->AddHitGroup(
        _H("triangle"),
        eRayFunctionGroupType_Triangles,
        _rayHitFun,
        nullptr, // No any-hit shader
        nullptr  // No intersection shader (using built-in triangle intersection)
      );
      niCheck(hitGroupId != eInvalidHandle, eFalse);

      _rayPipeline = niCheckNN(_rayPipeline,
        _driverRay->CreateRayPipeline(_H("RayTriangle_Pipeline"), _rayFuncTable),
        eFalse);
    }

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);

      niLet prDesc = niCheckNN(
        prDesc,
        _driverRay->CreateRayPrimitivesDesc(_H("RayPrimitivesDesc_Triangle")),
        eFalse);

      // Add a triangle
      niLet triangleVB = MakeTriVB(1.0f,Vec3f(0,0,0.3f));
      niCheck(prDesc->AddTriangles(
        triangleVB,0,sizeof(tVertexTri),3,
        sMatrixf::Identity(),
        eRayPrimitiveFlags_Opaque,
        0), eFalse);

      niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayPrimitives(
        _H("RayPrimitives_Triangle"),prDesc), eFalse);

      {
        niLet instDesc = niCheckNN(
          instDesc,
          _driverRay->CreateRayInstancesDesc(_H("RayInstancesDesc_Triangle")),
          eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          0,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);

        _instanceAS = niCheckNN(_instanceAS, buildEncoder->BuildRayInstances(
          _H("RayInstances_Triangle"),instDesc), eFalse);
      }
    }

    // Create our output image
    {
      _rayOutputImage = niCheckNN(_rayOutputImage,_graphics->CreateTexture(
        _H("rayOutputImage"),eBitmapType_2D,"R8G8B8A8",0,
        256,256,0,eTextureFlags_RenderTarget),eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    NN<iRayCommandEncoder> rayEncoder = AsNN(QueryInterface<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_instanceAS);
    rayEncoder->SetRayOutputImage(_rayOutputImage);
    rayEncoder->SetRayPipeline(_rayPipeline);
    rayEncoder->DispatchRays(_rayOutputImage->GetWidth(),_rayOutputImage->GetHeight(),1);

    DisplayTexture(gpuEncoder,_rayOutputImage);
    return eTrue;
  }
};
TEST_CLASS(FRay,Triangle);

struct sFRay_Quad : public sFRay_Base {
  typedef sVertexPA tVertexTri;

  // Ray tracing pipeline and shaders
  NN<iRayInstances> _instanceAS = niDeferredInit(NN<iRayInstances>);
  NN<iGpuFunction> _rayGenFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayMissFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayHitFun = niDeferredInit(NN<iGpuFunction>);
  NN<iRayFunctionTable> _rayFuncTable = niDeferredInit(NN<iRayFunctionTable>);
  NN<iRayPipeline> _rayPipeline = niDeferredInit(NN<iRayPipeline>);
  NN<iTexture> _rayOutputImage = niDeferredInit(NN<iTexture>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRay_Base::OnInit(testResults_),eFalse);

    // Create ray tracing shaders
    {
      _rayGenFun = niCheckNN(_rayGenFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayGeneration, _H("test/rayfunc/triangle_rgen.gpufunc.xml")), eFalse);

      _rayMissFun = niCheckNN(_rayMissFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayMiss, _H("test/rayfunc/triangle_rmiss.gpufunc.xml")), eFalse);

      _rayHitFun = niCheckNN(_rayHitFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayClosestHit, _H("test/rayfunc/triangle_rchit.gpufunc.xml")), eFalse);
    }

    // Create ray tracing pipeline
    {
      _rayFuncTable = niCheckNN(_rayFuncTable, _driverRay->CreateRayFunctionTable(), eFalse);
      _rayFuncTable->SetRayGenFunction(_rayGenFun);
      _rayFuncTable->SetMissFunction(_rayMissFun);

      // Add hit group for quads
      niLet hitGroupId = _rayFuncTable->AddHitGroup(
        _H("quad"),
        eRayFunctionGroupType_Triangles,
        _rayHitFun,
        nullptr, // No any-hit shader
        nullptr  // No intersection shader (using built-in quad intersection)
      );
      niCheck(hitGroupId != eInvalidHandle, eFalse);

      _rayPipeline = niCheckNN(_rayPipeline,
        _driverRay->CreateRayPipeline(_H("RayQuad_Pipeline"), _rayFuncTable),
        eFalse);
    }

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);

      niLet prDesc = niCheckNN(
        prDesc,
        _driverRay->CreateRayPrimitivesDesc(_H("RayPrimitivesDesc_Quad")),
        eFalse);

      niLet quadVB = MakeQuadVB(1.0f, Vec3f(0,0,0.3f));
      niLet quadIB = MakeQuadIB(1);
      niCheck(prDesc->AddTrianglesIndexed(
        quadVB,0,sizeof(tVertexTri),4,
        quadIB,0,eGpuIndexType_U32,6,
        sMatrixf::Identity(),
        eRayPrimitiveFlags_Opaque,
        0), eFalse);

      niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayPrimitives(
        _H("RayPrimitives_Triangle"),prDesc), eFalse);

      {
        niLet instDesc = niCheckNN(
          instDesc,
          _driverRay->CreateRayInstancesDesc(_H("RayInstancesDesc_Quad")),
          eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          0,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);

        _instanceAS = niCheckNN(_instanceAS, buildEncoder->BuildRayInstances(
          _H("RayInstances_Quad"),instDesc), eFalse);
      }
    }

    // Create our output image
    {
      _rayOutputImage = niCheckNN(_rayOutputImage,_graphics->CreateTexture(
        _H("rayOutputImage"),eBitmapType_2D,"R8G8B8A8",0,
        256,256,0,eTextureFlags_RenderTarget),eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    NN<iRayCommandEncoder> rayEncoder = AsNN(QueryInterface<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_instanceAS);
    rayEncoder->SetRayOutputImage(_rayOutputImage);
    rayEncoder->SetRayPipeline(_rayPipeline);
    rayEncoder->DispatchRays(_rayOutputImage->GetWidth(),_rayOutputImage->GetHeight(),1);

    DisplayTexture(gpuEncoder,_rayOutputImage);
    return eTrue;
  }
};
TEST_CLASS(FRay,Quad);

struct sFRay_TriangleQuad : public sFRay_Base {
  typedef sVertexPA tVertexTri;

  // Ray tracing pipeline and shaders
  NN<iRayInstances> _instanceAS = niDeferredInit(NN<iRayInstances>);
  NN<iGpuFunction> _rayGenFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayMissFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayHitFun = niDeferredInit(NN<iGpuFunction>);
  NN<iRayFunctionTable> _rayFuncTable = niDeferredInit(NN<iRayFunctionTable>);
  NN<iRayPipeline> _rayPipeline = niDeferredInit(NN<iRayPipeline>);
  NN<iTexture> _rayOutputImage = niDeferredInit(NN<iTexture>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRay_Base::OnInit(testResults_),eFalse);

    // Create ray tracing shaders
    {
      _rayGenFun = niCheckNN(_rayGenFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayGeneration, _H("test/rayfunc/triangle_rgen.gpufunc.xml")), eFalse);

      _rayMissFun = niCheckNN(_rayMissFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayMiss, _H("test/rayfunc/triangle_rmiss.gpufunc.xml")), eFalse);

      _rayHitFun = niCheckNN(_rayHitFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayClosestHit, _H("test/rayfunc/triangle_rchit.gpufunc.xml")), eFalse);
    }

    // Create ray tracing pipeline
    {
      _rayFuncTable = niCheckNN(_rayFuncTable, _driverRay->CreateRayFunctionTable(), eFalse);
      _rayFuncTable->SetRayGenFunction(_rayGenFun);
      _rayFuncTable->SetMissFunction(_rayMissFun);

      // Add hit group for quads
      niLet hitGroupId = _rayFuncTable->AddHitGroup(
        _H("quad"),
        eRayFunctionGroupType_Triangles,
        _rayHitFun,
        nullptr, // No any-hit shader
        nullptr  // No intersection shader (using built-in quad intersection)
      );
      niCheck(hitGroupId != eInvalidHandle, eFalse);

      _rayPipeline = niCheckNN(_rayPipeline,
        _driverRay->CreateRayPipeline(_H("RayQuad_Pipeline"), _rayFuncTable),
        eFalse);
    }

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);

      niLet prDesc = niCheckNN(
        prDesc,
        _driverRay->CreateRayPrimitivesDesc(_H("RayPrimitivesDesc_Quad")),
        eFalse);

      {
        niLet triangleVB = MakeTriVB(0.5f,Vec3f(-0.25f,0.35f,0.3f));
        niCheck(prDesc->AddTriangles(
          triangleVB,0,sizeof(tVertexTri),3,
          sMatrixf::Identity(),
          eRayPrimitiveFlags_Opaque,
          0), eFalse);
      }

      {
        niLet triangleVB = MakeTriVB(0.5f,Vec3f(-0.25f,-0.35f,0.3f));
        niLet triangleIB = MakeTriIB(1);
        niCheck(prDesc->AddTrianglesIndexed(
          triangleVB,0,sizeof(tVertexTri),3,
          triangleIB,0,eGpuIndexType_U32,3,
          sMatrixf::Identity(),
          eRayPrimitiveFlags_Opaque,
          0), eFalse);
      }

      {
        niLet quadVB = MakeQuadVB(0.5f, Vec3f(0.25f,0,0.3f));
        niLet quadIB = MakeQuadIB(1);
        niCheck(prDesc->AddTrianglesIndexed(
          quadVB,0,sizeof(tVertexTri),4,
          quadIB,0,eGpuIndexType_U32,6,
          sMatrixf::Identity(),
          eRayPrimitiveFlags_Opaque,
          0), eFalse);
      }

      niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayPrimitives(
        _H("RayPrimitives_Triangle"),prDesc), eFalse);

      {
        niLet instDesc = niCheckNN(
          instDesc,
          _driverRay->CreateRayInstancesDesc(_H("RayInstancesDesc_Quad")),
          eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          0,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);

        _instanceAS = niCheckNN(_instanceAS, buildEncoder->BuildRayInstances(
          _H("RayInstances_Quad"),instDesc), eFalse);
      }
    }

    // Create our output image
    {
      _rayOutputImage = niCheckNN(_rayOutputImage,_graphics->CreateTexture(
        _H("rayOutputImage"),eBitmapType_2D,"R8G8B8A8",0,
        256,256,0,eTextureFlags_RenderTarget),eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    NN<iRayCommandEncoder> rayEncoder = AsNN(QueryInterface<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_instanceAS);
    rayEncoder->SetRayOutputImage(_rayOutputImage);
    rayEncoder->SetRayPipeline(_rayPipeline);
    rayEncoder->DispatchRays(_rayOutputImage->GetWidth(),_rayOutputImage->GetHeight(),1);

    DisplayTexture(gpuEncoder,_rayOutputImage);
    return eTrue;
  }
};
TEST_CLASS(FRay,TriangleQuad);

struct sFRay_InstancesBase : public sFRay_Base {
  typedef sVertexPA tVertexTri;

  const tHStringPtr _rchitPath;

  // Ray tracing pipeline and shaders
  NN<iGpuFunction> _rayGenFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayMissFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayHitFun = niDeferredInit(NN<iGpuFunction>);
  NN<iRayFunctionTable> _rayFuncTable = niDeferredInit(NN<iRayFunctionTable>);
  NN<iRayPipeline> _rayPipeline = niDeferredInit(NN<iRayPipeline>);
  NN<iTexture> _rayOutputImage = niDeferredInit(NN<iTexture>);

  // Instances
  struct sInstanceDef {
    sVec3f _pos;
  };
  astl::array<sInstanceDef,4> _instances = {
    Vec3f(-0.25f,0.35f,0.35f),
    Vec3f( 0.25f,0.35f,0.35f),
    Vec3f(-0.25f,-0.35f,0.35f),
    Vec3f( 0.25f,-0.35f,0.35f),
  };
  NN<iRayBuildEncoder> _rayBuildEncoder = niDeferredInit(NN<iRayBuildEncoder>);
  NN<iRayInstancesDesc> _instancesDesc = niDeferredInit(NN<iRayInstancesDesc>);

  sFRay_InstancesBase(iHString* ahspRChitPath)
      : _rchitPath(ahspRChitPath)
  {}

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRay_Base::OnInit(testResults_),eFalse);

    // Create ray tracing shaders
    {
      _rayGenFun = niCheckNN(_rayGenFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayGeneration, _H("test/rayfunc/triangle_rgen.gpufunc.xml")), eFalse);

      _rayMissFun = niCheckNN(_rayMissFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayMiss, _H("test/rayfunc/triangle_rmiss.gpufunc.xml")), eFalse);

      _rayHitFun = niCheckNN(_rayHitFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_RayClosestHit, _rchitPath), eFalse);
    }

    // Create ray tracing pipeline
    {
      _rayFuncTable = niCheckNN(_rayFuncTable, _driverRay->CreateRayFunctionTable(), eFalse);
      _rayFuncTable->SetRayGenFunction(_rayGenFun);
      _rayFuncTable->SetMissFunction(_rayMissFun);

      // Add hit group for quads
      niLet hitGroupId = _rayFuncTable->AddHitGroup(
        _H("quad"),
        eRayFunctionGroupType_Triangles,
        _rayHitFun,
        nullptr, // No any-hit shader
        nullptr  // No intersection shader (using built-in quad intersection)
      );
      niCheck(hitGroupId != eInvalidHandle, eFalse);

      _rayPipeline = niCheckNN(_rayPipeline,
        _driverRay->CreateRayPipeline(_H("RayQuad_Pipeline"), _rayFuncTable),
        eFalse);
    }

    // Create acceleration structure
    {
      _rayBuildEncoder = niCheckNN(_rayBuildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);

      niLet prDesc = niCheckNN(
        prDesc,
        _driverRay->CreateRayPrimitivesDesc(_H("RayPrimitivesDesc_Quad")),
        eFalse);

      niLet triangleVB = MakeTriVB(0.5f,sVec3f::Zero());
      niCheck(prDesc->AddTriangles(
        triangleVB,0,sizeof(tVertexTri),3,
        sMatrixf::Identity(),
        eRayPrimitiveFlags_Opaque,
        0), eFalse);
      niLet primitiveAS = niCheckNN(primitiveAS, _rayBuildEncoder->BuildRayPrimitives(
        _H("RayPrimitives_Triangle"),prDesc), eFalse);

      _instancesDesc = niCheckNN(
        _instancesDesc,
        _driverRay->CreateRayInstancesDesc(_H("RayInstancesDesc_Quad")),
        eFalse);

      niLoop(i,_instances.size()) {
        sMatrixf mtx = sMatrixf::Identity();
        mtx = MatrixTranslation(_instances[i]._pos);
        niCheck(_instancesDesc->AddInstance(
          primitiveAS,
          mtx,                  // Transform
          i*10,                 // Custom Instance Id
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);
      }
    }

    // Create our output image
    {
      _rayOutputImage = niCheckNN(_rayOutputImage,_graphics->CreateTexture(
        _H("rayOutputImage"),eBitmapType_2D,"R8G8B8A8",0,
        256,256,0,eTextureFlags_RenderTarget),eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    NN<iRayCommandEncoder> rayEncoder = AsNN(QueryInterface<iRayCommandEncoder>(gpuEncoder));

    niLoop(i,_instances.size()) {
      niPanicAssert(_instancesDesc->UpdateInstanceTransform(
        i,MatrixRotationZ(WrapRad(_animationTime)) *
        MatrixTranslation(_instances[i]._pos)));
    }
    niLet instanceAS = niCheckNN(instanceAS, _rayBuildEncoder->BuildRayInstances(
      nullptr,_instancesDesc), eFalse);

    rayEncoder->SetRayInstances(instanceAS);
    rayEncoder->SetRayOutputImage(_rayOutputImage);
    rayEncoder->SetRayPipeline(_rayPipeline);
    rayEncoder->DispatchRays(_rayOutputImage->GetWidth(),_rayOutputImage->GetHeight(),1);

    DisplayTexture(gpuEncoder,_rayOutputImage);
    return eTrue;
  }
};

struct sFRay_Instances : public sFRay_InstancesBase {
  sFRay_Instances()
      : sFRay_InstancesBase(_H("test/rayfunc/triangle_rchit.gpufunc.xml"))
  {}
};
TEST_CLASS(FRay,Instances);

struct sFRay_InstancesIndex : public sFRay_InstancesBase {
  sFRay_InstancesIndex()
      : sFRay_InstancesBase(_H("test/rayfunc/triangle_instanceindex_rchit.gpufunc.xml"))
  {}
};
TEST_CLASS(FRay,InstancesIndex);

struct sFRay_InstancesId : public sFRay_InstancesBase {
  sFRay_InstancesId()
      : sFRay_InstancesBase(_H("test/rayfunc/triangle_instanceid_rchit.gpufunc.xml"))
  {}
};
TEST_CLASS(FRay,InstancesId);

}
