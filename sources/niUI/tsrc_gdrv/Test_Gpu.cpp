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
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 1);
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

struct sFGpu_TexAlphaStream : public sFGpu_TexAlphaBase {
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
TEST_CLASS(FGpu,TexAlphaStream);

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
      _H("rtTex"),eBitmapType_2D,"R8G8B8A8",1,
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
      rtEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 1);
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

struct sFGpu_DepthTest : public sFGpu_Base {
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
        _driverGpu->CreateGpuBuffer(
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
      pipelineDesc->SetDepthStencilStates(eCompiledStates_DS_DepthTestAndWrite);
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
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,18,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,DepthTest);

}
