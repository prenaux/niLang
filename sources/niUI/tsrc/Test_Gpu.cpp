#include "stdafx.h"
#include <niLang/Math/MathRect.h>
#include <niUI/IGpu.h>
#include <niLang/Utils/DataTableUtils.h>
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

using namespace ni;

namespace {

struct FGpu {
};

struct GpuCanvasBase : public ni::cWidgetSinkImpl<> {
  struct MainGpuCanvas : public ni::cWidgetSinkImpl<> {
    WeakPtr<GpuCanvasBase> _parent;
    MainGpuCanvas(GpuCanvasBase* aParent) : _parent(aParent) {}
    tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
      QPtr<GpuCanvasBase> p = _parent;
      p->_DrawMain(apCanvas);
      return eFalse;
    }
  };

  TEST_CONSTRUCTOR(GpuCanvasBase) {
    niDebugFmt(("GpuCanvasBase::GpuCanvasBase"));
  }
  ~GpuCanvasBase() {
    niDebugFmt(("GpuCanvasBase::~GpuCanvasBase"));
  }

  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);

  tBool __stdcall OnSinkAttached() niImpl {
    QPtr<iGraphicsDriverGpu> driverGpu = mpWidget->GetUIContext()->GetGraphics()->GetDriver();
    CHECK_RET(niIsOK(driverGpu),eFalse);
    _driverGpu = AsNN(driverGpu.raw_ptr());

    CHECK_RET(_Init(),eFalse);

    Ptr<iWidget> main = mpWidget->GetUIContext()->CreateWidgetRaw("MyCanvas",mpWidget);
    main->AddSink(niNew MainGpuCanvas(this));
    main->SetDockStyle(eWidgetDockStyle_DockFill);
    main->SetIgnoreInput(eTrue);

    niLog(Info,niFmt("Animation: %z.",mbAnimated));
    return eTrue;
  }

  tBool mbAnimated = eTrue;
  tF64 mfAnimationTime = 0.0;
  void _ToggleAnimation() {
    mbAnimated = !mbAnimated;
    niLog(Info,niFmt("Toggled animation: %z.",mbAnimated));
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niOverride {
    if (mbAnimated) {
      mfAnimationTime += ni::GetLang()->GetFrameTime();
    }
    return eFalse;
  }

  tBool __stdcall OnLeftClickDown(const sVec2f& avMousePos, const sVec2f& avNCMousePos) niImpl {
    _ToggleAnimation();
    return eFalse;
  }
  tBool __stdcall OnKeyDown(eKey aKey, tU32 aKeyMod) niImpl {
    if (aKey == eKey_Space) {
      _ToggleAnimation();
    }
    return eFalse;
  }

  virtual tBool _Init() { return eTrue; }
  virtual void _DrawMain(iCanvas* apCanvas) = 0;
};

struct GpuClear : public GpuCanvasBase {
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);

  TEST_CONSTRUCTOR_BASE(GpuClear,GpuCanvasBase) {
    //_vaBuffer = AsNN(_driverGpu->
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    // ... That's not really a clear operation ...
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuClear);

struct GpuTriangle : public GpuCanvasBase {
  typedef sVertexPA tVertexFmt;

  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  TEST_CONSTRUCTOR_BASE(GpuTriangle,GpuCanvasBase) {
  }

  tBool _Init() niImpl {
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
      // center of the screen is at 0.0
      // left x = -1,  right x =  1
      //  top y =  1, bottom y = -1
      verts[0] = {{ -0.5f,  -0.5f, 0.0f}, 0xFFFF0000}; // Red
      verts[1] = {{  0.5f,  -0.5f, 0.0f}, 0xFF00FF00}; // Green
      verts[2] = {{  0.0f,   0.5f, 0.0f}, 0xFF0000FF}; // Blue
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

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
    apCanvas->Flush(); // submit in the current command encoder

    QPtr<iGraphicsContextGpu> gpuContext = mpWidget->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->Draw(eGraphicsPrimitiveType_TriangleList,3,0);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuTriangle);

struct GpuSquare : public GpuCanvasBase {
  typedef sVertexPA tVertexFmt;

  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  TEST_CONSTRUCTOR_BASE(GpuSquare,GpuCanvasBase) {
  }

  tBool _Init() niImpl {
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
      // center of the screen is at 0.0
      // left x = -1,  right x =  1
      //  top y =  1, bottom y = -1
      // this a regular "math grid" with the x axis going right and y axis going up
      verts[0] = {{ -0.5f,   0.5f, 0.0f}, 0xFFFF0000}; // Red
      verts[1] = {{  0.5f,   0.5f, 0.0f}, 0xFF00FF00}; // Green
      verts[2] = {{  0.5f,  -0.5f, 0.0f}, 0xFF0000FF}; // Blue
      verts[3] = {{ -0.5f,  -0.5f, 0.0f}, 0xFFFFFFFF}; // White
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

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
    apCanvas->Flush(); // submit in the current command encoder

    QPtr<iGraphicsContextGpu> gpuContext = mpWidget->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuSquare);

struct GpuTexture : public GpuCanvasBase {
  typedef tVertexCanvas tVertexFmt;
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _texture = niDeferredInit(NN<iTexture>);

  TEST_CONSTRUCTOR_BASE(GpuTexture,GpuCanvasBase) {
  }

  tBool _Init() niImpl {
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
      // center of the screen is at 0.0
      // left x = -1,  right x =  1
      //  top y =  1, bottom y = -1
      // this a regular "math grid" with the x axis going right and y axis going up
      verts[0] = {{ -0.5f,   0.5f, 0.0f}, sVec3f::YAxis(), 0xFFFF0000, {0.0f,0.0f}}; // Red
      verts[1] = {{  0.5f,   0.5f, 0.0f}, sVec3f::YAxis(), 0xFF00FF00, {1.0f,0.0f}}; // Green
      verts[2] = {{  0.5f,  -0.5f, 0.0f}, sVec3f::YAxis(), 0xFF0000FF, {1.0f,1.0f}}; // Blue
      verts[3] = {{ -0.5f,  -0.5f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {0.0f,1.0f}}; // White
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
      fp = mpWidget->GetGraphics()->OpenBitmapFile("test/tex/earth_d.jpg");
      _texture = AsNN(mpWidget->GetGraphics()->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),mpWidget->GetGraphics()->LoadBitmap(fp),eTextureFlags_Default));
    }

    return eTrue;
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
    apCanvas->Flush(); // submit in the current command encoder

    QPtr<iGraphicsContextGpu> gpuContext = mpWidget->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetTexture(_texture, 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 1);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuTexture);

struct GpuTexAlphaBase : public GpuCanvasBase {
  typedef tVertexCanvas tVertexFmt;
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _texture = niDeferredInit(NN<iTexture>);

  TEST_CONSTRUCTOR_BASE(GpuTexAlphaBase,GpuCanvasBase) {
  }

  virtual tBool InitUniformBuffer() = 0;
  virtual void UpdateUniformBuffer(ain<nn<iGpuCommandEncoder>> aCmdEncoder) = 0;

  tBool _Init() niImpl {
    {
      niLetMut wnd = UnitTest::GetTestAppContext()->_window;
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
      // center of the screen is at 0.0
      // left x = -1,  right x =  1
      //  top y =  1, bottom y = -1
      // this a regular "math grid" with the x axis going right and y axis going up
      verts[0] = {{ -0.5f,   0.5f, 0.0f}, sVec3f::YAxis(), 0xFFFF0000, {0.0f,0.0f}}; // Red
      verts[1] = {{  0.5f,   0.5f, 0.0f}, sVec3f::YAxis(), 0xFF00FF00, {3.0f,0.0f}}; // Green
      verts[2] = {{  0.5f,  -0.5f, 0.0f}, sVec3f::YAxis(), 0xFF0000FF, {3.0f,3.0f}}; // Blue
      verts[3] = {{ -0.5f,  -0.5f, 0.0f}, sVec3f::YAxis(), 0xFFFFFFFF, {0.0f,3.0f}}; // White
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
      fp = mpWidget->GetGraphics()->OpenBitmapFile("test/tex/tree.dds");
      _texture = AsNN(mpWidget->GetGraphics()->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),mpWidget->GetGraphics()->LoadBitmap(fp),eTextureFlags_Default));
    }

    return eTrue;
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
    apCanvas->Flush(); // submit in the current command encoder
    QPtr<iGraphicsContextGpu> gpuContext = mpWidget->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    UpdateUniformBuffer(cmdEncoder);
    cmdEncoder->SetTexture(_texture, 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_SmoothMirror, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,6,0);
  }
};

struct GpuTexAlphaUBuffer : public GpuTexAlphaBase {
  NN<iGpuBuffer> _uBuffer = niDeferredInit(NN<iGpuBuffer>);

  TEST_CONSTRUCTOR_BASE(GpuTexAlphaUBuffer,GpuTexAlphaBase) {
  }

  virtual tBool InitUniformBuffer() {
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
  virtual void UpdateUniformBuffer(ain<nn<iGpuCommandEncoder>> aCmdEncoder) {
    niLetMut uBuffer = (TestGpuFuncs_TestUniforms*)_uBuffer->Lock(
      0, _uBuffer->GetSize(), eLock_Discard);
    niCheck(uBuffer != nullptr,;);
    {
      niLet cosTime = ni::Cos<tF32>((tF32)mfAnimationTime * 2.0f) * 0.5f + 0.5f;
      uBuffer->materialColor = sVec4f::One() * ((1.0f-cosTime)+0.1f) * 3.0f;
      uBuffer->alphaRef = cosTime - 0.05f; // 0.05 so that we have a full "no alpha test" mode
      MatrixRotationZ(uBuffer->mtxWVP,WrapRad((tF32)mfAnimationTime*0.1f));
    }
    _uBuffer->Unlock();
    aCmdEncoder->SetUniformBuffer(_uBuffer, 0, 0);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuTexAlphaUBuffer);

struct GpuTexAlphaStream : public GpuTexAlphaBase {
  TEST_CONSTRUCTOR_BASE(GpuTexAlphaStream,GpuTexAlphaBase) {
  }

  virtual tBool InitUniformBuffer() {
    return eTrue;
  }
  virtual void UpdateUniformBuffer(ain<nn<iGpuCommandEncoder>> aCmdEncoder) {
    TestGpuFuncs_TestUniforms u;
    niLet cosTime = ni::Cos<tF32>((tF32)mfAnimationTime * 2.0f) * 0.5f + 0.5f;
    u.materialColor = sVec4f::One() * ((1.0f-cosTime)+0.1f) * 3.0f;
    u.alphaRef = cosTime - 0.05f;
    MatrixRotationZ(u.mtxWVP,WrapRad((tF32)mfAnimationTime*0.1f));
    aCmdEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuTexAlphaStream);

struct GpuClearRects : public GpuCanvasBase {
  TEST_CONSTRUCTOR_BASE(GpuClearRects,GpuCanvasBase) {
  }

  tBool _Init() niImpl {
    return eTrue;
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFFFFFFFF);
    apCanvas->Flush(); // submit in the current command encoder

    QPtr<iGraphicsContextGpu> gpuContext = mpWidget->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());

    niLetMut ctx = mpWidget->GetGraphicsContext();
    niLetMut gpuCtx = niCheckNN(gpuCtx,QueryInterface<iGraphicsContextGpu>(ctx),;);
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
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuClearRects);

}
