#include "stdafx.h"
#include "FGDRV.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

using namespace ni;

namespace {

struct sFGpu_Base : public sFGDRV_Base {
  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);

  virtual tBool OnInit(UnitTest::TestResults& testResults_) override {
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

  virtual tBool OnInit(UnitTest::TestResults& testResults_) niImpl {
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

  virtual tBool OnPaint(UnitTest::TestResults& testResults_) override {
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
  virtual tBool OnPaint(UnitTest::TestResults& testResults_) niImpl {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetViewport(Recti(
      _graphicsContext->GetWidth()/3,_graphicsContext->GetHeight()/3,
      _graphicsContext->GetWidth()/4,_graphicsContext->GetHeight()/4));
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,TriangleViewport);

struct sFGpu_TriangleScissor : public sFGpu_Triangle {
  virtual tBool OnPaint(UnitTest::TestResults& testResults_) niImpl {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetScissorRect(Recti(
      _graphicsContext->GetWidth()/3,_graphicsContext->GetHeight()/3,
      _graphicsContext->GetWidth()/4,_graphicsContext->GetHeight()/4));
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);
    return eTrue;
  }
};
TEST_CLASS(FGpu,TriangleScissor);

struct sFGpu_Square : public sFGpu_Base {
  typedef sVertexPA tVertexFmt;

  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  virtual tBool OnInit(UnitTest::TestResults& testResults_) niImpl {
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

  virtual tBool OnPaint(UnitTest::TestResults& testResults_) niImpl {
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

  virtual tBool OnInit(UnitTest::TestResults& testResults_) niImpl {
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

  virtual tBool OnPaint(UnitTest::TestResults& testResults_) niImpl {
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

  virtual tBool OnInit(UnitTest::TestResults& testResults_) niImpl {
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

  virtual tBool OnPaint(UnitTest::TestResults& testResults_) niImpl {
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

  virtual tBool BeforePaint(UnitTest::TestResults& testResults_) override {
    CHECK_RET(sFGpu_TexAlphaBase::BeforePaint(testResults_),eFalse);
    return eTrue;
  }

  virtual tBool InitUniformBuffer() override {
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
  virtual void UpdateUniformBuffer(ain<nn<iGpuCommandEncoder>> aCmdEncoder) override {
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
  virtual tBool OnInit(UnitTest::TestResults& testResults_) niImpl {
    CHECK_RET(sFGpu_Base::OnInit(testResults_),eFalse);
    return eTrue;
  }

  virtual tBool OnPaint(UnitTest::TestResults& testResults_) niImpl {
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

}
