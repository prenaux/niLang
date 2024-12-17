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

struct sFRayGpu_Triangle : public sFRayGpu_Base {
  typedef sVertexPA tVertexFmt;

  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRayGpu_Base::OnInit(testResults_),eFalse);

    QPtr<iGraphicsDriverGpu> driverGpu = _graphics->GetDriver();
    CHECK_RET(niIsOK(driverGpu),eFalse);
    _driverGpu = AsNN(driverGpu.raw_ptr());

    {
      _vaBuffer = niCheckNN(
        _vaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuTriangle_VA"),
          sizeof(tVertexFmt)*3,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex),
        eFalse);
      tVertexFmt* verts = (tVertexFmt*)_vaBuffer->Lock(0, _vaBuffer->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      // NDC (Normalized Device Coordinates - Clip Space):
      // - the origin (0.0,0.0) is at the center of the screen
      // - x points right
      // - y points up
      // - left x = -1,  right x =  1
      // -  top y =  1, bottom y = -1
      verts[0] = {{  0.0f,   0.5f, 0.0f}, 0xFFFF0000}; // Red, TC
      verts[1] = {{  0.5f,  -0.5f, 0.0f}, 0xFF00FF00}; // Green, BR
      verts[2] = {{ -0.5f,  -0.5f, 0.0f}, 0xFF0000FF}; // Blue, BL
      _vaBuffer->Unlock();
    }

    {
      _vertexGpuFun = niCheckNN(_vertexGpuFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_Vertex,_H("test/gpufunc/triangle_vs.gpufunc.xml")),eFalse);
      _pixelGpuFun = niCheckNN(_pixelGpuFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel,_H("test/gpufunc/triangle_ps.gpufunc.xml")),eFalse);
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(_H("GpuTriangle_Pipeline"),pipelineDesc), eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);
    return eTrue;
  }
};
TEST_CLASS(FRayGpu,Triangle);

}
