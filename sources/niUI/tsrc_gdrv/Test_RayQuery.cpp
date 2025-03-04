#include "stdafx.h"
#include "FGDRV.h"
#include "../../../data/test/nish/TestGpuFuncs.hpp"
#include <niLang/Math/MathLib.h>
#include <niUI/Utils/AABB.h>
#include <niUI/Utils/RayUtils.h>

using namespace ni;

namespace _ {

//----------------------------------------------------------------------------
//
// Section: sFRayQuery_Base
//
//----------------------------------------------------------------------------
struct sFRayQuery_Base : public sFGDRV_Base {
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
      eGpuFunctionType_Vertex,_H("test/nish/gpu/texture_vs.gpufunc.xml")),eFalse);
    _displayPixelGpuFun = niCheckNN(_displayPixelGpuFun,_driverGpu->CreateGpuFunction(
      eGpuFunctionType_Pixel,_H("test/nish/gpu/texture_ps.gpufunc.xml")),eFalse);

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
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,0,1,0,0,6);
  }
};

//----------------------------------------------------------------------------
//
// Section: Triangle
//
//----------------------------------------------------------------------------
struct sFRayQuery_Triangle : public sFRayQuery_Base {
  NN<iRayInstances> _instanceAS = niDeferredInit(NN<iRayInstances>);
  NN<iGpuFunction> _triangleRayQueryFun = niDeferredInit(NN<iGpuFunction>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRayQuery_Base::OnInit(testResults_),eFalse);

    // Create ray tracing shaders
    {
      _triangleRayQueryFun = niCheckNN(_triangleRayQueryFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel, _H("test/nish/rayquery/triangle_rayquery_ps.gpufunc.xml")), eFalse);
      CHECK_EQUAL(eGpuFunctionType_Pixel,
                  _triangleRayQueryFun->GetFunctionType());
      CHECK_EQUAL(eGpuFunctionBindType_FixedRayInstances,
                  _triangleRayQueryFun->GetFunctionBindType());
    }

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      // Add a triangle
      {
        niLet prDesc = niCheckNN(
          prDesc,
          _driverRay->CreateRayTrianglePrimitivesDesc(HFmt("%s_RayTrianglePrimitivesDesc",m_testName)),
          eFalse);

        niLet triangleVB = MakeTriVB(_driverGpu,++_numTriVB,1.0f,Vec3f(0,0,0.3f));
        niCheck(prDesc->AddTriangles(
          triangleVB,0,sizeof(tVertexTri),3,
          sMatrixf::Identity(),
          eRayPrimitiveFlags_Opaque,
          0), eFalse);

        niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayTrianglePrimitives(
          HFmt("%s_RayTrianglePrimitives",m_testName),prDesc), eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          1,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);
      }

      _instanceAS = niCheckNN(_instanceAS, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    // Recreate the display pipeline with our shader
    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexCanvas::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_displayVertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_triangleRayQueryFun);
      _displayPipeline = niCheckNN(_displayPipeline, _driverGpu->CreateGpuPipeline(
        HFmt("%s_DisplayPipeline",m_testName),pipelineDesc), eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());

    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)_graphicsContext->GetWidth();
    u.rtHeight = (tF32)_graphicsContext->GetHeight();
    u.cameraFarClipPlane = 10000.0f;
    sMatrixf camView = MatrixLookAtLH(
      Vec3f(0,0,-1.0f),
      Vec3f(0,0,0),
      Vec3f(0,1,0)
    );
    sMatrixf camProj = MatrixPerspectiveFovLH(
      u.rtWidth/u.rtHeight,niRadf(45.0f),0.001f,u.cameraFarClipPlane);
    u.cameraInvView = MatrixInverse(camView);
    u.cameraInvViewProj = MatrixInverse(camView * camProj);

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QueryInterface<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_instanceAS);

    DisplayTexture(gpuEncoder,nullptr);
    return eTrue;
  }
};
TEST_CLASS(FRayQuery,Triangle);

//----------------------------------------------------------------------------
//
// Section: IntSphere
//
//----------------------------------------------------------------------------
struct sFRayQuery_IntSphere : public sFRayQuery_Base {
  NN<iRayInstances> _instanceAS = niDeferredInit(NN<iRayInstances>);
  NN<iGpuFunction> _triangleRayQueryFun = niDeferredInit(NN<iGpuFunction>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFRayQuery_Base::OnInit(testResults_),eFalse);

    // Create ray tracing shaders
    {
      _triangleRayQueryFun = niCheckNN(_triangleRayQueryFun, _driverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel, _H("test/nish/rayquery/sphere_rayquery_ps.gpufunc.xml")), eFalse);
      CHECK_EQUAL(eGpuFunctionType_Pixel,
                  _triangleRayQueryFun->GetFunctionType());
      CHECK_EQUAL(eGpuFunctionBindType_FixedRayInstances,
                  _triangleRayQueryFun->GetFunctionBindType());
    }

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      // Add a triangle
      {
        niLet prDesc = niCheckNN(
          prDesc,
          _driverRay->CreateRayTrianglePrimitivesDesc(HFmt("%s_RayTrianglePrimitivesDesc",m_testName)),
          eFalse);

        niLet triangleVB = MakeTriVB(_driverGpu,++_numTriVB,1.0f,Vec3f(0,0,0.3f));
        niCheck(prDesc->AddTriangles(
          triangleVB,0,sizeof(tVertexTri),3,
          sMatrixf::Identity(),
          eRayPrimitiveFlags_Opaque,
          0), eFalse);

        niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayTrianglePrimitives(
          HFmt("%s_RayTrianglePrimitives",m_testName),prDesc), eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          1,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);
      }

      // Create a procedural AABB for the sphere
      {
        niLet prDesc = niCheckNN(
          prDesc,
          _driverRay->CreateRayProceduralPrimitivesDesc(HFmt("%s_RayProceduralPrimitivesDesc",m_testName)),
          eFalse);

        niLet aabb = cAABBf(
          Vec3f(-0.5f,-0.5f,-0.5f),
          Vec3f(0.5f,0.5f,0.5f));

        niLet aabbBuffer = niCheckNN(
          aabbBuffer,
          _driverGpu->CreateGpuBufferFromDataRaw(
            HFmt("%s_SphereAABB",m_testName),
            (tPtr)&aabb,
            sizeof(cAABBf),
            eGpuBufferMemoryMode_Shared,
            eGpuBufferUsageFlags_RayBuildInput),
          eFalse);

        niCheck(prDesc->AddAABBs(
          aabbBuffer,0,sizeof(cAABBf),1,
          sMatrixf::Identity(),
          eRayPrimitiveFlags_Opaque,
          0), eFalse);

        niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayProceduralPrimitives(
          HFmt("%s_RayProceduralPrimitives",m_testName),prDesc), eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          2,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);
      }

      _instanceAS = niCheckNN(_instanceAS, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    // Recreate the display pipeline with our shader
    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexCanvas::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_displayVertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_triangleRayQueryFun);
      _displayPipeline = niCheckNN(_displayPipeline, _driverGpu->CreateGpuPipeline(
        HFmt("%s_DisplayPipeline",m_testName),pipelineDesc), eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());

    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)_graphicsContext->GetWidth();
    u.rtHeight = (tF32)_graphicsContext->GetHeight();
    u.cameraFarClipPlane = 10000.0f;
    sMatrixf camView = MatrixLookAtLH(
      Vec3f(0,0,-1.0f),
      Vec3f(0,0,0),
      Vec3f(0,1,0)
    );
    sMatrixf camProj = MatrixPerspectiveFovLH(
      u.rtWidth/u.rtHeight,niRadf(45.0f),0.001f,u.cameraFarClipPlane);
    u.cameraInvView = MatrixInverse(camView);
    u.cameraInvViewProj = MatrixInverse(camView * camProj);

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QueryInterface<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_instanceAS);

    DisplayTexture(gpuEncoder,nullptr);
    return eTrue;
  }
};
TEST_CLASS(FRayQuery,IntSphere);

}
