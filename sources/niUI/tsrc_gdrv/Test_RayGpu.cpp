#include "stdafx.h"
#include "FGDRV.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

using namespace ni;

namespace {

struct sFRayGpu_Base : public sFGDRV_Base {
  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGDRV_Base::OnInit(testResults_),eFalse);
    QPtr<iGraphicsDriverGpu> driverGpu = _graphics->GetDriver();
    CHECK_RET(niIsOK(driverGpu),eFalse);
    _driverGpu = AsNN(driverGpu.raw_ptr());
    CHECK_RET(_graphics->GetDriver()->GetCaps(eGraphicsCaps_IRayGpu) != 0,eFalse);
    return eTrue;
  }
};

// clear ; ham pass1 && ham Run_Test_niUI_GDRV FIXTURE=FRayGpu,Triangle A2=-Drenderer=Vulkan BUILD=da
struct sFRayGpu_Triangle : public sFRayGpu_Base {
  typedef sVertexPA tVertexFmt;

  NN<iGpuBuffer> _vertexBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iAccelerationStructure> _primitiveAS = niDeferredInit(NN<iAccelerationStructure>);
  NN<iAccelerationStructure> _instanceAS = niDeferredInit(NN<iAccelerationStructure>);

  // Ray tracing pipeline and shaders
  NN<iGpuFunction> _rayGenFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayMissFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _rayHitFun = niDeferredInit(NN<iGpuFunction>);
  NN<iRayGpuFunctionTable> _rayFuncTable = niDeferredInit(NN<iRayGpuFunctionTable>);
  NN<iRayGpuPipeline> _rayPipeline = niDeferredInit(NN<iRayGpuPipeline>);
  NN<iTexture> _rayOutputImage = niDeferredInit(NN<iTexture>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRayGpu_Base::OnInit(testResults_),eFalse);

    // Create vertex buffer with triangle geometry
    {
      _vertexBuffer = niCheckNN(
        _vertexBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("RayTriangle_VB"),
          sizeof(tVertexFmt)*3,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex|
          eGpuBufferUsageFlags_AccelerationStructureBuildInput),
        eFalse);

      tVertexFmt* verts = (tVertexFmt*)_vertexBuffer->Lock(0, _vertexBuffer->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      verts[0] = {{  0.0f,   0.5f, 0.0f}, 0xFFFF0000}; // Red, TC
      verts[1] = {{  0.5f,  -0.5f, 0.0f}, 0xFF00FF00}; // Green, BR
      verts[2] = {{ -0.5f,  -0.5f, 0.0f}, 0xFF0000FF}; // Blue, BL
      _vertexBuffer->Unlock();
    }

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

    // Create acceleration structure
    {
      _primitiveAS = niCheckNN(
        _primitiveAS,
        _driverGpu->CreateAccelerationStructure(
          _H("RayTriangle_AS"),
          eAccelerationStructureType_Primitive),
        eFalse);

      niCheck(_primitiveAS->AddTriangles(
        _vertexBuffer,0,sizeof(tVertexFmt),3,
        sMatrixf::Identity(),
        eAccelerationGeometryFlags_Opaque,
        0),
        eFalse);
    }

    {
      _instanceAS = niCheckNN(
        _instanceAS,
        _driverGpu->CreateAccelerationStructure(
          _H("RayTriangle_InstanceAS"),
          eAccelerationStructureType_Instance),
        eFalse);

      niCheck(_instanceAS->AddInstance(
        _primitiveAS,
        sMatrixf::Identity(), // Transform
        0,                    // Instance ID
        0xFF,                // Mask
        0,                   // Hit group offset
        eAccelerationInstanceFlags_None),
              eFalse);
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

    return eTrue;
  }
};
TEST_CLASS(FRayGpu,Triangle);

}
