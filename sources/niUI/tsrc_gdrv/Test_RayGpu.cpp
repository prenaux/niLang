#include "stdafx.h"
#include "FGDRV.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

//
// TODO:
// - [ ] p0: FRayGpu-TriangleSquare: Multiple geometries & instances, one triangle and a square side by sidea
// - [ ] p0: FRayGpu-UpdateInstance: Triangle & square rotating
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
//

using namespace ni;

namespace {

struct sFRayGpu_Base : public sFGDRV_Base {
  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);

  // Display resources
  NN<iGpuBuffer> _displayVABuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _displayIABuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _displayVertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _displayPixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _displayPipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _textureEarth = niDeferredInit(NN<iTexture>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGDRV_Base::OnInit(testResults_),eFalse);
    QPtr<iGraphicsDriverGpu> driverGpu = _graphics->GetDriver();
    CHECK_RET(niIsOK(driverGpu),eFalse);
    _driverGpu = AsNN(driverGpu.raw_ptr());
    CHECK_RET(_graphics->GetDriver()->GetCaps(eGraphicsCaps_IRayGpu) != 0,eFalse);

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

// clear ; ham pass1 && ham Run_Test_niUI_GDRV FIXTURE=FRayGpu,Triangle A2=-Drenderer=Vulkan BUILD=da
struct sFRayGpu_Triangle : public sFRayGpu_Base {
  typedef sVertexPA tVertexFmt;

  // Primitive and acceleration structures
  NN<iGpuBuffer> _triangleVB = niDeferredInit(NN<iGpuBuffer>);
  NN<iAccelerationStructurePrimitives> _primitiveAS = niDeferredInit(NN<iAccelerationStructurePrimitives>);
  NN<iAccelerationStructureInstances> _instanceAS = niDeferredInit(NN<iAccelerationStructureInstances>);

  // Ray tracing pipeline and shaders
  NN<iGpuFunction> _rayGenFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayMissFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayHitFun = niDeferredInit(NN<iGpuFunction>);
  NN<iRayGpuFunctionTable> _rayFuncTable = niDeferredInit(NN<iRayGpuFunctionTable>);
  NN<iRayGpuPipeline> _rayPipeline = niDeferredInit(NN<iRayGpuPipeline>);
  NN<iTexture> _rayOutputImage = niDeferredInit(NN<iTexture>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRayGpu_Base::OnInit(testResults_),eFalse);

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
      _rayFuncTable = niCheckNN(_rayFuncTable, _driverGpu->CreateRayFunctionTable(), eFalse);
      _rayFuncTable->SetRayGenFunction(_rayGenFun);
      _rayFuncTable->SetMissFunction(_rayMissFun);

      // Add hit group for triangles
      niLet hitGroupId = _rayFuncTable->AddHitGroup(
        _H("triangle"),
        eRayGpuFunctionGroupType_Triangles,
        _rayHitFun,
        nullptr, // No any-hit shader
        nullptr  // No intersection shader (using built-in triangle intersection)
      );
      niCheck(hitGroupId != eInvalidHandle, eFalse);

      _rayPipeline = niCheckNN(_rayPipeline,
        _driverGpu->CreateRayPipeline(_H("RayTriangle_Pipeline"), _rayFuncTable),
        eFalse);
    }

    // Create vertex buffer with triangle geometry
    {
      _triangleVB = niCheckNN(
        _triangleVB,
        _driverGpu->CreateGpuBuffer(
          _H("RayTriangle_VB"),
          sizeof(tVertexFmt)*3,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex|
          eGpuBufferUsageFlags_AccelerationStructureBuildInput),
        eFalse);

      tVertexFmt* verts = (tVertexFmt*)_triangleVB->Lock(0, _triangleVB->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      verts[0] = {{  0.0f,   0.5f, 0.3f}, 0xFFFF0000}; // Red, TC
      verts[1] = {{  0.5f,  -0.5f, 0.3f}, 0xFF00FF00}; // Green, BR
      verts[2] = {{ -0.5f,  -0.5f, 0.3f}, 0xFF0000FF}; // Blue, BL
      _triangleVB->Unlock();
    }

    // Create acceleration structure
    {
      _primitiveAS = niCheckNN(
        _primitiveAS,
        _driverGpu->CreateAccelerationStructurePrimitives(_H("RayTriangle_AS")),
        eFalse);

      niCheck(_primitiveAS->AddTriangles(
        _triangleVB,0,sizeof(tVertexFmt),3,
        sMatrixf::Identity(),
        eAccelerationStructurePrimitiveFlags_Opaque,
        0), eFalse);

      _instanceAS = niCheckNN(
        _instanceAS,
        _driverGpu->CreateAccelerationStructureInstances(
          _H("RayTriangle_InstanceAS")),
        eFalse);
      niCheck(_instanceAS->AddInstance(
        _primitiveAS,
        sMatrixf::Identity(), // Transform
        0,                    // Instance ID
        0xFF,                // Mask
        0,                   // Hit group offset
        eAccelerationStructureInstanceFlags_None), eFalse);
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
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());

    // Build/update acceleration structure
    cmdEncoder->BuildAccelerationStructure(_primitiveAS);
    cmdEncoder->BuildAccelerationStructure(_instanceAS);

    // Launch ray tracing, one ray per pixel
    cmdEncoder->DispatchRays(_rayPipeline,_rayOutputImage);

    DisplayTexture(cmdEncoder,_rayOutputImage);

    return eTrue;
  }
};
TEST_CLASS(FRayGpu,Triangle);

struct sFRayGpu_Square : public sFRayGpu_Base {
  typedef sVertexPA tVertexFmt;

  // Primitive and acceleration structures
  NN<iGpuBuffer> _squareVB = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _squareIB = niDeferredInit(NN<iGpuBuffer>);
  NN<iAccelerationStructurePrimitives> _primitiveAS = niDeferredInit(NN<iAccelerationStructurePrimitives>);
  NN<iAccelerationStructureInstances> _instanceAS = niDeferredInit(NN<iAccelerationStructureInstances>);

  // Ray tracing pipeline and shaders
  NN<iGpuFunction> _rayGenFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayMissFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayHitFun = niDeferredInit(NN<iGpuFunction>);
  NN<iRayGpuFunctionTable> _rayFuncTable = niDeferredInit(NN<iRayGpuFunctionTable>);
  NN<iRayGpuPipeline> _rayPipeline = niDeferredInit(NN<iRayGpuPipeline>);
  NN<iTexture> _rayOutputImage = niDeferredInit(NN<iTexture>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRayGpu_Base::OnInit(testResults_),eFalse);

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
      _rayFuncTable = niCheckNN(_rayFuncTable, _driverGpu->CreateRayFunctionTable(), eFalse);
      _rayFuncTable->SetRayGenFunction(_rayGenFun);
      _rayFuncTable->SetMissFunction(_rayMissFun);

      // Add hit group for squares
      niLet hitGroupId = _rayFuncTable->AddHitGroup(
        _H("square"),
        eRayGpuFunctionGroupType_Triangles,
        _rayHitFun,
        nullptr, // No any-hit shader
        nullptr  // No intersection shader (using built-in square intersection)
      );
      niCheck(hitGroupId != eInvalidHandle, eFalse);

      _rayPipeline = niCheckNN(_rayPipeline,
        _driverGpu->CreateRayPipeline(_H("RaySquare_Pipeline"), _rayFuncTable),
        eFalse);
    }

    // Create vertex buffer with square geometry
    {
      _squareVB = niCheckNN(
        _squareVB,
        _driverGpu->CreateGpuBuffer(
          _H("RaySquare_VB"),
          sizeof(tVertexFmt)*4,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex|
          eGpuBufferUsageFlags_AccelerationStructureBuildInput),
        eFalse);

      tVertexFmt* verts = (tVertexFmt*)_squareVB->Lock(0, _squareVB->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      // 25 degree-ish rotated square
      verts[0] = {{ -0.35f,  0.6f, 0.3f}, 0xFFFF0000}; // Red, TL
      verts[1] = {{  0.6f,   0.35f, 0.3f}, 0xFF00FF00}; // Green, TR
      verts[2] = {{  0.35f, -0.6f, 0.3f}, 0xFF0000FF}; // Blue, BR
      verts[3] = {{ -0.6f,  -0.35f, 0.3f}, 0xFFFFFFFF}; // White, BL
      _squareVB->Unlock();
    }

    {
      _squareIB = niCheckNN(
        _squareIB,
        _driverGpu->CreateGpuBuffer(
          _H("RaySquare_IB"),
          sizeof(tU32)*6,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Index|
          eGpuBufferUsageFlags_AccelerationStructureBuildInput),
        eFalse);
      tU32* inds = (tU32*)_squareIB->Lock(0, _squareIB->GetSize(), eLock_Discard);
      niCheck(inds != nullptr, eFalse);
      inds[0] = 0; inds[1] = 1; inds[2] = 2;
      inds[3] = 2; inds[4] = 3; inds[5] = 0;
      _squareIB->Unlock();
    }

    // Create acceleration structure
    {
      _primitiveAS = niCheckNN(
        _primitiveAS,
        _driverGpu->CreateAccelerationStructurePrimitives(_H("RaySquare_AS")),
        eFalse);

      niCheck(_primitiveAS->AddTrianglesIndexed(
        _squareVB,0,sizeof(tVertexFmt),4,
        _squareIB,0,eGpuIndexType_U32,6,
        sMatrixf::Identity(),
        eAccelerationStructurePrimitiveFlags_Opaque,
        0), eFalse);

      _instanceAS = niCheckNN(
        _instanceAS,
        _driverGpu->CreateAccelerationStructureInstances(
          _H("RaySquare_InstanceAS")),
        eFalse);
      niCheck(_instanceAS->AddInstance(
        _primitiveAS,
        sMatrixf::Identity(), // Transform
        0,                    // Instance ID
        0xFF,                // Mask
        0,                   // Hit group offset
        eAccelerationStructureInstanceFlags_None), eFalse);
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
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());

    // Build/update acceleration structure
    cmdEncoder->BuildAccelerationStructure(_primitiveAS);
    cmdEncoder->BuildAccelerationStructure(_instanceAS);

    // Launch ray tracing, one ray per pixel
    cmdEncoder->DispatchRays(_rayPipeline,_rayOutputImage);

    DisplayTexture(cmdEncoder,_rayOutputImage);

    return eTrue;
  }
};
TEST_CLASS(FRayGpu,Square);

}
