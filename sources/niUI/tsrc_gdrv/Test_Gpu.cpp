#include "stdafx.h"
#include "FGDRV.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

using namespace ni;

namespace {

struct sFGpu_Base : public sFGDRV_Base {
  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGDRV_Base::OnInit(testResults_),eFalse);
    QPtr<iGraphicsDriverGpu> driverGpu = _graphics->GetDriver();
    CHECK_RET(niIsOK(driverGpu),eFalse);
    _driverGpu = AsNN(driverGpu.raw_ptr());
    return eTrue;
  }
};

struct sFGpu_Triangle : public sFGpu_Base {
  typedef sVertexPA tVertexFmt;

  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);

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
TEST_CLASS(FGpu,Triangle);

struct sFGpu_TriangleViewport : public sFGpu_Triangle {
  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());

    niLet w = _graphicsContext->GetWidth();
    niLet h = _graphicsContext->GetHeight();
    niLet w2 = w/2;
    niLet h2 = h/2;

    // Red, TL
    cmdEncoder->SetViewport(Recti(0,0,w2,h2));
    gpuContext->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(0,0,(tF32)w,(tF32)h),
      0xFFFF0000,1.0f);
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    // Green, TR
    cmdEncoder->SetViewport(Recti(w2,0,w2,h2));
    gpuContext->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(0,0,(tF32)w,(tF32)h),
      0xFF00FF00,1.0f);
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    // Blue, BR
    cmdEncoder->SetViewport(Recti(w2,h2,w2,h2));
    gpuContext->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(0,0,(tF32)w,(tF32)h),
      0xFF0000FF,1.0f);
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    // White, BL
    cmdEncoder->SetViewport(Recti(0,h2,w2,h2));
    gpuContext->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(0,0,(tF32)w,(tF32)h),
      0xFFFFFFFF,1.0f);
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    return eTrue;
  }
};
TEST_CLASS(FGpu,TriangleViewport);

struct sFGpu_TriangleScissor : public sFGpu_Triangle {
  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());

    niLet w = _graphicsContext->GetWidth();
    niLet h = _graphicsContext->GetHeight();
    cmdEncoder->SetScissorRect(Recti(w/3,h/3,w/4,h/4));
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    return eTrue;
  }
};
TEST_CLASS(FGpu,TriangleScissor);

struct sFGpu_TriangleViewportScissor : public sFGpu_Triangle {
  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());

    niLet w = _graphicsContext->GetWidth();
    niLet h = _graphicsContext->GetHeight();
    niLet w2 = w/2; niLet h2 = h/2;

    // Red, TL
    cmdEncoder->SetViewport(Recti(0,0,w2,h2));
    cmdEncoder->SetScissorRect(Recti(0,0,w2,h2));
    gpuContext->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(0,0,(tF32)w,(tF32)h),
      0xFFFF0000,1.0f);
    cmdEncoder->SetScissorRect(Recti(w/6,h/6,w/8,h/8));
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    // Green, TR
    cmdEncoder->SetViewport(Recti(w2,0,w2,h2));
    cmdEncoder->SetScissorRect(Recti(w2,0,w2,h2));
    gpuContext->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(0,0,(tF32)w,(tF32)h),
      0xFF00FF00,1.0f);
    cmdEncoder->SetScissorRect(Recti(w2+(w/6),h/6,w/8,h/8));
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    // Blue, BR
    cmdEncoder->SetViewport(Recti(w2,h2,w2,h2));
    cmdEncoder->SetScissorRect(Recti(w2,h2,w2,h2));
    gpuContext->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(0,0,(tF32)w,(tF32)h),
      0xFF0000FF,1.0f);
    cmdEncoder->SetScissorRect(Recti(w2+(w/6),h2+(h/6),w/8,h/8));
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    // White, BL
    cmdEncoder->SetViewport(Recti(0,h2,w2,h2));
    cmdEncoder->SetScissorRect(Recti(0,h2,w2,h2));
    gpuContext->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(0,0,(tF32)w,(tF32)h),
      0xFFFFFFFF,1.0f);
    cmdEncoder->SetScissorRect(Recti(w/6,h2+(h/6),w/8,h/8));
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);

    return eTrue;
  }
};
TEST_CLASS(FGpu,TriangleViewportScissor);

struct sFGpu_Square : public sFGpu_Base {
  typedef sVertexPA tVertexFmt;

  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niImpl {
    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);

    {
      _vaBuffer = niCheckNN(
        _vaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuSquare_VA"),
          sizeof(tVertexFmt)*4,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex),
        eFalse);
      tVertexFmt* verts = (tVertexFmt*)_vaBuffer->Lock(0, _vaBuffer->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      verts[0] = {{ -0.5f,   0.5f, 0.0f}, 0xFFFF0000}; // Red, TL
      verts[1] = {{  0.5f,   0.5f, 0.0f}, 0xFF00FF00}; // Green, TR
      verts[2] = {{  0.5f,  -0.5f, 0.0f}, 0xFF0000FF}; // Blue, BR
      verts[3] = {{ -0.5f,  -0.5f, 0.0f}, 0xFFFFFFFF}; // White, BL
      _vaBuffer->Unlock();
    }
    {
      _iaBuffer = niCheckNN(
        _iaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuSquare_IA"),
          sizeof(tU32)*6,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Index),
        eFalse);
      tU32* inds = (tU32*)_iaBuffer->Lock(0, _iaBuffer->GetSize(), eLock_Discard);
      niCheck(inds != nullptr, eFalse);
      inds[0] = 0; inds[1] = 1; inds[2] = 2;
      inds[3] = 2; inds[4] = 3; inds[5] = 0;
      _iaBuffer->Unlock();
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
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(_H("GpuSquare_Pipeline"),pipelineDesc), eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,Square);

struct sFGpu_Texture : public sFGpu_Base {
  typedef tVertexCanvas tVertexFmt;
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _texture = niDeferredInit(NN<iTexture>);

  tBool OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);

    {
      _vaBuffer = niCheckNN(
        _vaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuTexture_VA"),
          sizeof(tVertexFmt)*4,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex),
        eFalse);
      tVertexFmt* verts = (tVertexFmt*)_vaBuffer->Lock(0, _vaBuffer->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      verts[0] = {{ -0.5f,   0.5f, 0.0f}, sVec3f::YAxis(), 0xFFFF0000, {0.0f,0.0f}}; // Red, TL
      verts[1] = {{  0.5f,   0.5f, 0.0f}, sVec3f::YAxis(), 0xFF00FF00, {1.0f,0.0f}}; // Green, TR
      verts[2] = {{  0.5f,  -0.5f, 0.0f}, sVec3f::YAxis(), 0xFF0000FF, {1.0f,1.0f}}; // Blue, BR
      verts[3] = {{ -0.5f,  -0.5f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {0.0f,1.0f}}; // White, BL
      _vaBuffer->Unlock();
    }
    {
      _iaBuffer = niCheckNN(
        _iaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuTexture_IA"),
          sizeof(tU32)*6,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Index),
        eFalse);
      tU32* inds = (tU32*)_iaBuffer->Lock(0, _iaBuffer->GetSize(), eLock_Discard);
      niCheck(inds != nullptr, eFalse);
      inds[0] = 0; inds[1] = 1; inds[2] = 2;
      inds[3] = 2; inds[4] = 3; inds[5] = 0;
      _iaBuffer->Unlock();
    }

    {
      _vertexGpuFun = niCheckNN(_vertexGpuFun,_driverGpu->CreateGpuFunction(
          eGpuFunctionType_Vertex,_H("test/gpufunc/texture_vs.gpufunc.xml")),eFalse);
      _pixelGpuFun = niCheckNN(_pixelGpuFun,_driverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel,_H("test/gpufunc/texture_ps.gpufunc.xml")),eFalse);
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(_H("GpuTexture_Pipeline"),pipelineDesc), eFalse);
    }

    {
      Ptr<iFile> fp;
      fp = _graphics->OpenBitmapFile("test/tex/earth_d.jpg");
      _texture = AsNN(_graphics->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default));
    }

    return eTrue;
  }

  tBool OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetTexture(_texture, 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,Texture);

struct sFGpu_TexAlphaBase : public sFGpu_Base {
  typedef tVertexCanvas tVertexFmt;
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _texture = niDeferredInit(NN<iTexture>);

  virtual tBool InitUniformBuffer() = 0;
  virtual void UpdateUniformBuffer(ain<nn<iGpuCommandEncoder>> aCmdEncoder) = 0;

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niImpl {
    _clearTimer = -1.0; // fixed clear color

    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);

    {
      niLetMut wnd = _window;
      // Make a square window so that when we're rotating the square it doesnt
      // look too stretched. We're drawing it directly in clip space which is
      // [-1;1] from the left to the right of our application window.
      wnd->SetClientSize(Vec2i(500,500));
      wnd->CenterWindow();
    }

    {
      _vaBuffer = niCheckNN(
        _vaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuTexAlpha_VA"),
          sizeof(tVertexFmt)*4,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex),
        eFalse);
      tVertexFmt* verts = (tVertexFmt*)_vaBuffer->Lock(0, _vaBuffer->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      verts[0] = {{ -0.5f,   0.5f, 0.0f}, sVec3f::YAxis(), 0xFFFF0000, {0.0f,0.0f}}; // Red, TL
      verts[1] = {{  0.5f,   0.5f, 0.0f}, sVec3f::YAxis(), 0xFF00FF00, {3.0f,0.0f}}; // Green, TR
      verts[2] = {{  0.5f,  -0.5f, 0.0f}, sVec3f::YAxis(), 0xFF0000FF, {3.0f,3.0f}}; // Blue, BR
      verts[3] = {{ -0.5f,  -0.5f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {0.0f,3.0f}}; // White, BL
      _vaBuffer->Unlock();
    }
    {
      _iaBuffer = niCheckNN(
        _iaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuTexAlpha_IA"),
          sizeof(tU32)*6,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Index),
        eFalse);
      tU32* inds = (tU32*)_iaBuffer->Lock(0, _iaBuffer->GetSize(), eLock_Discard);
      niCheck(inds != nullptr, eFalse);
      inds[0] = 0; inds[1] = 1; inds[2] = 2;
      inds[3] = 2; inds[4] = 3; inds[5] = 0;
      _iaBuffer->Unlock();
    }

    niCheck(InitUniformBuffer(),eFalse);

    {
      _vertexGpuFun = niCheckNN(_vertexGpuFun,_driverGpu->CreateGpuFunction(
        eGpuFunctionType_Vertex,_H("test/gpufunc/texture_alphatest_vs.gpufunc.xml")),eFalse);
      _pixelGpuFun = niCheckNN(_pixelGpuFun,_driverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel,_H("test/gpufunc/texture_alphatest_ps.gpufunc.xml")),eFalse);
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(_H("GpuTexAlpha_Pipeline"),pipelineDesc), eFalse);
    }

    {
      Ptr<iFile> fp;
      fp = _graphics->OpenBitmapFile("test/tex/tree.dds");
      _texture = AsNN(_graphics->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default));
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    UpdateUniformBuffer(cmdEncoder);
    cmdEncoder->SetTexture(_texture, 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_SmoothMirror, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
    return eTrue;
  }
};

struct sFGpu_TexAlphaUBuffer : public sFGpu_TexAlphaBase {
  NN<iGpuBuffer> _uBuffer = niDeferredInit(NN<iGpuBuffer>);

  sFGpu_TexAlphaUBuffer() {
  }

  virtual tBool BeforePaint(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGpu_TexAlphaBase::BeforePaint(testResults_),eFalse);
    return eTrue;
  }

  virtual tBool InitUniformBuffer() niOverride {
    TestGpuFuncs_TestUniforms ubInit;
    _uBuffer = niCheckNN(
      _uBuffer,
      _driverGpu->CreateGpuBufferFromDataRaw(
        _H("GpuTexAlpha_TestUniforms"),
        (tPtr)&ubInit,
        sizeof(TestGpuFuncs_TestUniforms),
        eGpuBufferMemoryMode_Shared,
        eGpuBufferUsageFlags_Uniform),
      eFalse);
    return eTrue;
  }
  virtual void UpdateUniformBuffer(ain<nn<iGpuCommandEncoder>> aCmdEncoder) niOverride {
    niLetMut uBuffer = (TestGpuFuncs_TestUniforms*)_uBuffer->Lock(
      0, _uBuffer->GetSize(), eLock_Discard);
    niCheck(uBuffer != nullptr,;);
    {
      uBuffer->materialColor = sVec4f::One() * ((1.0f-_pingpongTime)+0.1f) * 3.0f;
      uBuffer->alphaRef = _pingpongTime - 0.05f; // 0.05 so that we have a full "no alpha test" mode
      MatrixRotationZ(uBuffer->mtxWVP,WrapRad((tF32)_animationTime*0.1f));
    }
    _uBuffer->Unlock();
    aCmdEncoder->SetUniformBuffer(_uBuffer, 0, 0);
  }
};
TEST_CLASS(FGpu,TexAlphaUBuffer);

struct sFGpu_TexAlphaUStream : public sFGpu_TexAlphaBase {
  virtual tBool InitUniformBuffer() {
    return eTrue;
  }
  virtual void UpdateUniformBuffer(ain<nn<iGpuCommandEncoder>> aCmdEncoder) {
    TestGpuFuncs_TestUniforms u;
    u.materialColor = sVec4f::One() * ((1.0f-_pingpongTime)+0.1f) * 3.0f;
    u.alphaRef = _pingpongTime - 0.05f;
    MatrixRotationZ(u.mtxWVP,WrapRad((tF32)_animationTime*0.1f));
    aCmdEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);
  }
};
TEST_CLASS(FGpu,TexAlphaUStream);

struct sFGpu_ClearRects : public sFGpu_Base {
  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niImpl {
    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);
    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    niLetMut ctx = _graphicsContext;
    niLetMut gpuCtx = niCheckNN(gpuCtx,QueryInterface<iGraphicsContextGpu>(ctx),eFalse);
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuCtx->GetCommandEncoder());

    niLet w = ctx->GetWidth();
    niLet h = ctx->GetHeight();

    // Clear rects
    niLet border = 1;
    gpuCtx->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(border,border,
            (tF32)w-(2*border),
            (tF32)h-(2*border)),
      0xFF333333, 0.0f);

    gpuCtx->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(50,50,
            (tF32)w-100,
            (tF32)h-100),
      0xFF00FF00, 0.0f);

    cmdEncoder->SetViewport(Recti(w/2,h/2,w/4,h/4));
    gpuCtx->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(50,50,
            (tF32)w-100,
            (tF32)h-100),
      0xFF0000FF, 0.0f);

    cmdEncoder->SetViewport(Recti(0,0,w,h));
    gpuCtx->ClearBuffersRect(
      eClearBuffersFlags_ColorDepthStencil,
      Rectf(50,50,500,500), 0xFFFF0000, 0.0f);
    return eTrue;
  }
};
TEST_CLASS(FGpu,ClearRects);

struct sFGpu_RenderTarget : public sFGpu_Texture {
  NN<iTexture> _rtTex = niDeferredInit(NN<iTexture>);
  NN<iGraphicsContext> _rtGC = niDeferredInit(NN<iGraphicsContext>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niImpl {
    CHECK_RET(sFGpu_Texture::OnInit(testResults_),eFalse);
    _rtTex = niCheckNN(_rtTex,_graphics->CreateTexture(
      _H("rtTex"),eBitmapType_2D,"R8G8B8A8",0,
      256,256,0,eTextureFlags_RenderTarget),eFalse);
    _rtGC = niCheckNN(_rtGC,_graphics->CreateContextForRenderTargets(
      _rtTex,nullptr,nullptr,nullptr,nullptr),eFalse);
    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    {
      QPtr<iGraphicsContextGpu> gpuRT = _rtGC;
      niPanicAssert(gpuRT.IsOK());
      _rtGC->ClearBuffers(eClearBuffersFlags_ColorDepthStencil,0xFFFF0000,1.0f,0);
      NN<iGpuCommandEncoder> rtEncoder = AsNN(gpuRT->GetCommandEncoder());
      rtEncoder->SetPipeline(_pipeline);
      rtEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
      rtEncoder->SetTexture(nullptr, 0); // bind the "white texture"
      rtEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 0);
      rtEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
      rtEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
      _rtGC->Display(eGraphicsDisplayFlags_Skip,sRecti::Null());
    }

    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetTexture(_rtTex, 0); // bind the render target
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 1);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,RenderTarget);

struct sTest_DepthTest {
  typedef sVertexPA tVertexFmt;

  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  niFn(tBool) OnInit(
    UnitTest::TestResults& testResults_,
    ain<nn<iGraphicsDriverGpu>> aDriverGpu)
  {
    {
      _vaBuffer = niCheckNN(
        _vaBuffer,
        aDriverGpu->CreateGpuBuffer(
          _H("GpuSquare_VA"),
          sizeof(tVertexFmt)*12,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex),
        eFalse);
      tVertexFmt* verts = (tVertexFmt*)_vaBuffer->Lock(0, _vaBuffer->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);

      // depth goes from 0.0f (near/front) to 1.0f (far/back)
      niLet znear = 0.0f + 0.1f;
      niLet zfar = 1.0f - 0.1f;
      niLet zmid = ((zfar-znear)/2.0f) + znear;

      tU32 i = 0;

      // middle
      verts[i++] = {{ -0.5f,   0.5f, zmid}, 0xFFFF0000}; // Red, TL
      verts[i++] = {{  0.5f,   0.5f, zmid}, 0xFF00FF00}; // Green, TR
      verts[i++] = {{  0.5f,  -0.5f, zmid}, 0xFF0000FF}; // Blue, BR
      verts[i++] = {{ -0.5f,  -0.5f, zmid}, 0xFFFFFFFF}; // White, BL

      // front - purple - bottom right of middle
      verts[i++]  = {{ -0.3f + 0.5f,   0.3f - 0.5f, znear}, 0xFF800080}; // TL
      verts[i++]  = {{  0.3f + 0.5f,   0.3f - 0.5f, znear}, 0xFF800080}; // TR
      verts[i++] = {{  0.3f + 0.5f,  -0.3f - 0.5f, znear}, 0xFF800080}; // BR
      verts[i++] = {{ -0.3f + 0.5f,  -0.3f - 0.5f, znear}, 0xFF800080}; // BL

      // back - yellow - top left of middle
      verts[i++] = {{ -0.3f - 0.5f,   0.3f + 0.5f, zfar}, 0xFFFFFF00}; // TL
      verts[i++] = {{  0.3f - 0.5f,   0.3f + 0.5f, zfar}, 0xFFFFFF00}; // TR
      verts[i++] = {{  0.3f - 0.5f,  -0.3f + 0.5f, zfar}, 0xFFFFFF00}; // BR
      verts[i++] = {{ -0.3f - 0.5f,  -0.3f + 0.5f, zfar}, 0xFFFFFF00}; // BL

      _vaBuffer->Unlock();
    }
    {
      _iaBuffer = niCheckNN(
        _iaBuffer,
        aDriverGpu->CreateGpuBuffer(
          _H("GpuSquare_IA"),
          sizeof(tU32)*18,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Index),
        eFalse);
      tU32* inds = (tU32*)_iaBuffer->Lock(0, _iaBuffer->GetSize(), eLock_Discard);
      niCheck(inds != nullptr, eFalse);
      inds[0] = 0; inds[1] = 1; inds[2] = 2;
      inds[3] = 2; inds[4] = 3; inds[5] = 0;
      inds[6] = 4; inds[7] = 5; inds[8] = 6;
      inds[9] = 6; inds[10] = 7; inds[11] = 4;
      inds[12] = 8; inds[13] = 9; inds[14] = 10;
      inds[15] = 10; inds[16] = 11; inds[17] = 8;
      _iaBuffer->Unlock();
    }

    {
      _vertexGpuFun = niCheckNN(_vertexGpuFun, aDriverGpu->CreateGpuFunction(
        eGpuFunctionType_Vertex,_H("test/gpufunc/triangle_vs.gpufunc.xml")),eFalse);
      _pixelGpuFun = niCheckNN(_pixelGpuFun, aDriverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel,_H("test/gpufunc/triangle_ps.gpufunc.xml")),eFalse);
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, aDriverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      pipelineDesc->SetDepthStencilStates(eCompiledStates_DS_DepthTestAndWrite);
      _pipeline = niCheckNN(_pipeline, aDriverGpu->CreateGpuPipeline(_H("GpuSquare_Pipeline"),pipelineDesc), eFalse);
    }

    return eTrue;
  }

  niFn(tBool) OnPaint(
    UnitTest::TestResults& testResults_,
    ain<nn<iGraphicsContextGpu>> gpuContext
  ) {
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,18,0);
    return eTrue;
  }
};

struct sFGpu_DepthTest : public sFGpu_Base {
  sTest_DepthTest _testDepthTest;

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niImpl {
    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);
    CHECK_RET(_testDepthTest.OnInit(testResults_, _driverGpu),eFalse);
    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    CHECK_RET(_testDepthTest.OnPaint(testResults_,gpuContext.non_null()),eFalse);
    return eTrue;
  }
};
TEST_CLASS(FGpu,DepthTest);

struct sFGpu_DepthTestRenderTarget : public sFGpu_Texture {
  sTest_DepthTest _testDepthTest;
  NN<iTexture> _rtTex = niDeferredInit(NN<iTexture>);
  NN<iTexture> _dsTex = niDeferredInit(NN<iTexture>);
  NN<iGraphicsContext> _rtGC = niDeferredInit(NN<iGraphicsContext>);

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niImpl {
    CHECK_RET(sFGpu_Texture::OnInit(testResults_),eFalse);
    CHECK_RET(_testDepthTest.OnInit(testResults_, _driverGpu),eFalse);
    _rtTex = niCheckNN(_rtTex,_graphics->CreateTexture(
      _H("rtTex"),eBitmapType_2D,"R8G8B8A8",0,
      256,256,0,eTextureFlags_RenderTarget),eFalse);
    _dsTex = niCheckNN(_rtTex,_graphics->CreateTexture(
      _H("dsTex"),eBitmapType_2D,"D32",0,
      256,256,0,eTextureFlags_DepthStencil),eFalse);
    _rtGC = niCheckNN(_rtGC,_graphics->CreateContextForRenderTargets(
      _rtTex,nullptr,nullptr,nullptr,_dsTex),eFalse);
    return eTrue;
  }

  niFn(tBool) OnPaint(UnitTest::TestResults& testResults_) niImpl {
    {
      QPtr<iGraphicsContextGpu> gpuRT = _rtGC;
      niPanicAssert(gpuRT.IsOK());
      _rtGC->ClearBuffers(eClearBuffersFlags_ColorDepthStencil,0xFFFF0000,1.0f,0);
      CHECK_RET(_testDepthTest.OnPaint(testResults_,gpuRT.non_null()),eFalse);
      _rtGC->Display(eGraphicsDisplayFlags_Skip,sRecti::Null());
    }

    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetTexture(_rtTex, 0); // bind the render target as texture
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,DepthTestRenderTarget);

// Note: This is kind of shitty but does the job for now. Resize the window to
// see the different mipmaps, it should change color as mipmaps.dds has a
// different color for each mipmap level. Ideally we'd show all the mip levels
// like a "texture viewer" of sort, that requires a new shader that I cant be
// bothered to write right now.
struct sFGpu_TextureMipMaps : public sFGpu_Base {
  typedef tVertexCanvas tVertexFmt;

  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _texture = niDeferredInit(NN<iTexture>);
  tF32 _scale = 1.0f;

  tBool OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);

    // Create quad scaled larger to show mips
    {
      _vaBuffer = niCheckNN(
        _vaBuffer,
        _driverGpu->CreateGpuBuffer(_H("GpuTextureMip_VA"),
                                    sizeof(tVertexFmt)*4,
                                    eGpuBufferMemoryMode_Shared,
                                    eGpuBufferUsageFlags_Vertex),
        eFalse);

      tVertexFmt* verts = (tVertexFmt*)_vaBuffer->Lock(0, _vaBuffer->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      verts[0] = {{ -2.0f,  2.0f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {0.0f,0.0f}};
      verts[1] = {{  2.0f,  2.0f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {8.0f,0.0f}};
      verts[2] = {{  2.0f, -2.0f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {8.0f,8.0f}};
      verts[3] = {{ -2.0f, -2.0f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {0.0f,8.0f}};
      _vaBuffer->Unlock();
    }

    {
      _iaBuffer = niCheckNN(
        _iaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuTexture_IA"),
          sizeof(tU32)*6,
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Index),
        eFalse);
      tU32* inds = (tU32*)_iaBuffer->Lock(0, _iaBuffer->GetSize(), eLock_Discard);
      niCheck(inds != nullptr, eFalse);
      inds[0] = 0; inds[1] = 1; inds[2] = 2;
      inds[3] = 2; inds[4] = 3; inds[5] = 0;
      _iaBuffer->Unlock();
    }

    {
      _vertexGpuFun = niCheckNN(_vertexGpuFun,_driverGpu->CreateGpuFunction(
          eGpuFunctionType_Vertex,_H("test/gpufunc/texture_vs.gpufunc.xml")),eFalse);
      _pixelGpuFun = niCheckNN(_pixelGpuFun,_driverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel,_H("test/gpufunc/texture_ps.gpufunc.xml")),eFalse);
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(_H("GpuTexture_Pipeline"),pipelineDesc), eFalse);
    }

    {
      _texture = niCheckNN(
        _texture,
        _graphics->CreateTextureFromRes(
          _H("test/tex/mipmaps.dds"),
          nullptr,
          eTextureFlags_Default),
      eFalse);
    }

    return eTrue;
  }

  tBool OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());

    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer,0,0);
    cmdEncoder->SetTexture(_texture, 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_SmoothRepeat, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,TextureMipMaps);

struct sFGpu_TextureCube : public sFGpu_Base {
  typedef sVertexP tVertexFmt;
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _cubemap = niDeferredInit(NN<iTexture>);

  tBool OnInit(UnitTest::TestResults& testResults_) niOverride {
    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);

    // Create cube vertices (standard skybox cube)
    {
      const tF32 s = 10.0f;
      const tVertexFmt kCubeVertices[] = {
        {{-s,  s, -s}}, {{-s, -s, -s}}, {{ s, -s, -s}}, {{ s,  s, -s}},  // front
        {{ s,  s,  s}}, {{ s, -s,  s}}, {{-s, -s,  s}}, {{-s,  s,  s}},  // back
        {{-s,  s,  s}}, {{-s, -s,  s}}, {{-s, -s, -s}}, {{-s,  s, -s}},  // left
        {{ s,  s, -s}}, {{ s, -s, -s}}, {{ s, -s,  s}}, {{ s,  s,  s}},  // right
        {{-s,  s,  s}}, {{-s,  s, -s}}, {{ s,  s, -s}}, {{ s,  s,  s}},  // top
        {{-s, -s, -s}}, {{-s, -s,  s}}, {{ s, -s,  s}}, {{ s, -s, -s}}   // bottom
      };

      _vaBuffer = niCheckNN(
        _vaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuTextureCube_VA"),
          sizeof(kCubeVertices),
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Vertex),
        eFalse);

      tVertexFmt* verts = (tVertexFmt*)_vaBuffer->Lock(0, _vaBuffer->GetSize(), eLock_Discard);
      niCheck(verts != nullptr, eFalse);
      memcpy(verts, kCubeVertices, sizeof(kCubeVertices));
      _vaBuffer->Unlock();
    }

    // Create indices for the cube
    {
      const tU32 kCubeIndices[] = {
        0,1,2, 2,3,0,       // front
        4,5,6, 6,7,4,       // back
        8,9,10, 10,11,8,    // left
        12,13,14, 14,15,12, // right
        16,17,18, 18,19,16, // top
        20,21,22, 22,23,20  // bottom
      };

      _iaBuffer = niCheckNN(
        _iaBuffer,
        _driverGpu->CreateGpuBuffer(
          _H("GpuTextureCube_IA"),
          sizeof(kCubeIndices),
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Index),
        eFalse);

      tU32* inds = (tU32*)_iaBuffer->Lock(0, _iaBuffer->GetSize(), eLock_Discard);
      niCheck(inds != nullptr, eFalse);
      memcpy(inds, kCubeIndices, sizeof(kCubeIndices));
      _iaBuffer->Unlock();
    }

    // Load cubemap
#if 1
    {
      // Face loading map for left-handed system
      struct sFaceFile {
        eBitmapCubeFace face;
        const achar* filename;
      } kFaces[] = {
        {eBitmapCubeFace_PositiveX, "test/tex/skybox_RT.jpg"},
        {eBitmapCubeFace_NegativeX, "test/tex/skybox_LF.jpg"},
        {eBitmapCubeFace_PositiveY, "test/tex/skybox_UP.jpg"},
        {eBitmapCubeFace_NegativeY, "test/tex/skybox_DN.jpg"},
        {eBitmapCubeFace_NegativeZ, "test/tex/skybox_FR.jpg"},
        {eBitmapCubeFace_PositiveZ, "test/tex/skybox_BK.jpg"},
      };

      // Load all faces first
      Ptr<iBitmap2D> faces[6];
      for (tU32 i = 0; i < 6; ++i) {
        faces[i] = (iBitmap2D*)_graphics->LoadBitmapFromRes(_H(kFaces[i].filename),nullptr);
        niCheck(niIsOK(faces[i]),eFalse);
      }

      // Create cube with size from first face
      niLet cubeSize = faces[0]->GetWidth();
      Ptr<iBitmapCube> bmpCube = _graphics->CreateBitmapCube(
        cubeSize,"R8G8B8A8");
      niCheck(niIsOK(bmpCube),eFalse);

      // Blit each face
      for (tU32 i = 0; i < 6; ++i) {
        Ptr<iBitmap2D> cubeFace = bmpCube->GetFace(kFaces[i].face);
        niCheck(niIsOK(cubeFace),eFalse);
        cubeFace->BlitStretch(faces[i],0,0,0,0,
                              faces[i]->GetWidth(),faces[i]->GetHeight(),
                              cubeFace->GetWidth(),cubeFace->GetHeight());
      }

      _cubemap = AsNN(_graphics->CreateTextureFromBitmap(
        _H("CubeMap"),
        bmpCube,
        eTextureFlags_Default));
    }
#else
    {

      _cubemap = niCheckNN(_cubemap,_graphics->CreateTextureFromRes(
        // _H("test/tex/glass_cubemap.dds"),
        _H("test/tex/Beach_cubemap_128.dds"),
        nullptr,
        eTextureFlags_Default), eFalse);
    }
#endif

    // Create shaders
    {
      _vertexGpuFun = niCheckNN(_vertexGpuFun,_driverGpu->CreateGpuFunction(
        eGpuFunctionType_Vertex,_H("test/gpufunc/skybox_vs.gpufunc.xml")),eFalse);
      _pixelGpuFun = niCheckNN(_pixelGpuFun,_driverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel,_H("test/gpufunc/skybox_ps.gpufunc.xml")),eFalse);
    }

    // Create pipeline
    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(
        pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      pipelineDesc->SetDepthStencilStates(eCompiledStates_DS_NoDepthTest);
      pipelineDesc->SetRasterizerStates(eCompiledStates_RS_NoCullingFilled);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(
        _H("GpuTextureCube_Pipeline"),pipelineDesc), eFalse);
    }

    return eTrue;
  }

  tBool OnPaint(UnitTest::TestResults& testResults_) niOverride {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());

    // Setup view matrix that only rotates (no translation for skybox)
    TestGpuFuncs_TestUniforms u;
    niLocal rotY = sMatrixf::Identity();
    niLocal rotX = sMatrixf::Identity();
    niLocal view = sMatrixf::Identity();
    niLocal persp = sMatrixf::Identity();
    MatrixRotationY(rotY,(tF32)_animationTime * 0.3f);
    MatrixRotationX(rotX,ni::Sin((tF32)_animationTime) * niPi4f);
    MatrixLookAtLH(view,sVec3f::Zero(),sVec3f::ZAxis(),sVec3f::YAxis());
    MatrixPerspectiveFovLH(
      persp,
      niRadf(110),
      ni::FDiv(
        (tF32)_graphicsContext->GetWidth(),
        (tF32)_graphicsContext->GetHeight()),
      0.1f,
      1000.0f);
    u.mtxWVP = rotY * rotX * view * persp;

    cmdEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer,0,0);
    cmdEncoder->SetTexture(_cubemap,0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_SmoothClamp,0);
    cmdEncoder->SetIndexBuffer(_iaBuffer,0,eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,36,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,TextureCube);

}
