#include "stdafx.h"
#include "FGDRV.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

//
// TODO:
// - [ ] p0: FRay-TriangleSquare: Multiple geometries & instances, one triangle and a square side by sidea
// - [ ] p0: FRay-UpdateInstance: Triangle & square rotating
// - [ ] p0: Sphere intersection shader
// - [ ] p0: Visualize: one colour per instance (use hashToColor / rainbowColor to generate that from the instance id)
// - [ ] p0: Visualize: triangles with barycentric coordinate
// - [ ] p1: Checkerboard floor
// - [ ] p1: Four reflective sphere on checkerboard floor (white, red, green, blue spheres)
// - [ ] p1: One reflective sphere on checkerboard floor
// - [ ] p1: Ref PT cornell box white-furnace sky
// - [ ] p1: Ref PT cornell box with one light on the ceiling - one emissive square should be the light source
// - [ ] p1: Visualize: base color
// - [ ] p1: Visualize: normals
// - [ ] p1: Visualize: worldspace position
// - [ ] p2: Textured cube
// - [ ] p2: Visualize: tex coordinates
// - [ ] p2: Compact static primitives AS
// - [ ] p2: Add dynamic primitive AS
//

using namespace ni;

namespace {

struct sFRay_Base : public sFGDRV_Base {
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
};

// clear ; ham pass1 && ham Run_Test_niUI_GDRV FIXTURE=FRay,Triangle A2=-Drenderer=Vulkan BUILD=da
struct sFRay_Triangle : public sFRay_Base {
  typedef sVertexPA tVertexFmt;

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

    // Create vertex buffer with triangle geometry
    niLet triangleVB = niCheckNN(
      triangleVB,
      _driverGpu->CreateGpuBuffer(
        _H("RayTriangle_VB"),
        sizeof(tVertexFmt)*3,
        eGpuBufferMemoryMode_Shared,
        eGpuBufferUsageFlags_Vertex|
        eGpuBufferUsageFlags_RayBuildInput),
      eFalse);
    {
      tVertexFmt* verts = (tVertexFmt*)triangleVB->Lock(0, triangleVB->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      verts[0] = {{  0.0f,   0.5f, 0.3f}, 0xFFFF0000}; // Red, TC
      verts[1] = {{  0.5f,  -0.5f, 0.3f}, 0xFF00FF00}; // Green, BR
      verts[2] = {{ -0.5f,  -0.5f, 0.3f}, 0xFF0000FF}; // Blue, BL
      triangleVB->Unlock();
    }

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);

      niLet prDesc = niCheckNN(
        prDesc,
        _driverRay->CreateRayPrimitivesDesc(_H("RayPrimitivesDesc_Triangle")),
        eFalse);

      niCheck(prDesc->AddTriangles(
        triangleVB,0,sizeof(tVertexFmt),3,
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

struct sFRay_Square : public sFRay_Base {
  typedef sVertexPA tVertexFmt;

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

      // Add hit group for squares
      niLet hitGroupId = _rayFuncTable->AddHitGroup(
        _H("square"),
        eRayFunctionGroupType_Triangles,
        _rayHitFun,
        nullptr, // No any-hit shader
        nullptr  // No intersection shader (using built-in square intersection)
      );
      niCheck(hitGroupId != eInvalidHandle, eFalse);

      _rayPipeline = niCheckNN(_rayPipeline,
        _driverRay->CreateRayPipeline(_H("RaySquare_Pipeline"), _rayFuncTable),
        eFalse);
    }

    // Create vertex buffer with square geometry
    niLet squareVB = niCheckNN(
      squareVB,
      _driverGpu->CreateGpuBuffer(
        _H("RaySquare_VB"),
        sizeof(tVertexFmt)*4,
        eGpuBufferMemoryMode_Shared,
        eGpuBufferUsageFlags_Vertex|
        eGpuBufferUsageFlags_RayBuildInput),
      eFalse);
    {

      tVertexFmt* verts = (tVertexFmt*)squareVB->Lock(0, squareVB->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      // 25 degree-ish rotated square
      verts[0] = {{ -0.35f,  0.6f, 0.3f}, 0xFFFF0000}; // Red, TL
      verts[1] = {{  0.6f,   0.35f, 0.3f}, 0xFF00FF00}; // Green, TR
      verts[2] = {{  0.35f, -0.6f, 0.3f}, 0xFF0000FF}; // Blue, BR
      verts[3] = {{ -0.6f,  -0.35f, 0.3f}, 0xFFFFFFFF}; // White, BL
      squareVB->Unlock();
    }

    niLet squareIB = niCheckNN(
      squareIB,
      _driverGpu->CreateGpuBuffer(
        _H("RaySquare_IB"),
        sizeof(tU32)*6,
        eGpuBufferMemoryMode_Shared,
        eGpuBufferUsageFlags_Index|
        eGpuBufferUsageFlags_RayBuildInput),
      eFalse);
    {
      tU32* inds = (tU32*)squareIB->Lock(0, squareIB->GetSize(), eLock_Discard);
      niCheck(inds != nullptr, eFalse);
      inds[0] = 0; inds[1] = 1; inds[2] = 2;
      inds[3] = 2; inds[4] = 3; inds[5] = 0;
      squareIB->Unlock();
    }

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);

      niLet prDesc = niCheckNN(
        prDesc,
        _driverRay->CreateRayPrimitivesDesc(_H("RayPrimitivesDesc_Square")),
        eFalse);

      niCheck(prDesc->AddTrianglesIndexed(
        squareVB,0,sizeof(tVertexFmt),4,
        squareIB,0,eGpuIndexType_U32,6,
        sMatrixf::Identity(),
        eRayPrimitiveFlags_Opaque,
        0), eFalse);

      niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayPrimitives(
        _H("RayPrimitives_Triangle"),prDesc), eFalse);

      {
        niLet instDesc = niCheckNN(
          instDesc,
          _driverRay->CreateRayInstancesDesc(_H("RayInstancesDesc_Square")),
          eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          0,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);

        _instanceAS = niCheckNN(_instanceAS, buildEncoder->BuildRayInstances(
          _H("RayInstances_Square"),instDesc), eFalse);
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
TEST_CLASS(FRay,Square);

}
