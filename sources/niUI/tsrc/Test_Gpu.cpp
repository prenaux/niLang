#include "stdafx.h"
#include <niLang/Math/MathRect.h>
#include <niUI/IGpu.h>
#include <niLang/Utils/DataTableUtils.h>

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
      tHStringPtr hspPath = _H("test/gpufunc/triangle_vs.gpufunc.xml");
      niLet dtVertex = niCheckNN(dtVertex,LoadDataTable(niHStr(hspPath)),eFalse);
      _vertexGpuFun = niCheckNN(_vertexGpuFun, _driverGpu->CreateGpuFunction(eGpuFunctionType_Vertex,hspPath,dtVertex),eFalse);
    }
    {
      tHStringPtr hspPath = _H("test/gpufunc/triangle_ps.gpufunc.xml");
      niLet dtPixel = niCheckNN(dtPixel,LoadDataTable(niHStr(hspPath)),eFalse);
      _pixelGpuFun = niCheckNN(_pixelGpuFun, _driverGpu->CreateGpuFunction(eGpuFunctionType_Pixel,hspPath,dtPixel),eFalse);
    }
    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(pipelineDesc), eFalse);
    }
    return eTrue;
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
    apCanvas->Flush(); // submit in the current command encoder

    QPtr<iGraphicsContextGpu> gpuContext = mpWidget->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->BeginCommandEncoder());
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
      tHStringPtr hspPath = _H("test/gpufunc/triangle_vs.gpufunc.xml");
      niLet dtVertex = niCheckNN(dtVertex,LoadDataTable(niHStr(hspPath)),eFalse);
      _vertexGpuFun = niCheckNN(_vertexGpuFun, _driverGpu->CreateGpuFunction(eGpuFunctionType_Vertex,hspPath,dtVertex),eFalse);
    }
    {
      tHStringPtr hspPath = _H("test/gpufunc/triangle_ps.gpufunc.xml");
      niLet dtPixel = niCheckNN(dtPixel,LoadDataTable(niHStr(hspPath)),eFalse);
      _pixelGpuFun = niCheckNN(_pixelGpuFun, _driverGpu->CreateGpuFunction(eGpuFunctionType_Pixel,hspPath,dtPixel),eFalse);
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(pipelineDesc), eFalse);
    }

    return eTrue;
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);
    apCanvas->Flush(); // submit in the current command encoder

    QPtr<iGraphicsContextGpu> gpuContext = mpWidget->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->BeginCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,eGpuIndexType_U32,6,0);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuSquare);

struct GpuTexture : public GpuCanvasBase {
  typedef sVertexPAT1 tVertexFmt;
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
      verts[0] = {{ -0.5f,   0.5f, 0.0f}, 0xFFFF0000, {0.0f,0.0f}}; // Red
      verts[1] = {{  0.5f,   0.5f, 0.0f}, 0xFF00FF00, {1.0f,0.0f}}; // Green
      verts[2] = {{  0.5f,  -0.5f, 0.0f}, 0xFF0000FF, {1.0f,1.0f}}; // Blue
      verts[3] = {{ -0.5f,  -0.5f, 0.0f}, 0xFFFFFFFF, {0.0f,1.0f}}; // White
      _vaBuffer->Unlock();
    }
    {
      _iaBuffer = niCheckNN(
        _iaBuffer,
        _driverGpu->CreateGpuBuffer(
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
      tHStringPtr hspPath = _H("test/gpufunc/texture_vs.gpufunc.xml");
      niLet dtVertex = niCheckNN(dtVertex,LoadDataTable(niHStr(hspPath)),eFalse);
      _vertexGpuFun = niCheckNN(
        _vertexGpuFun,
        _driverGpu->CreateGpuFunction(eGpuFunctionType_Vertex,hspPath,dtVertex),eFalse);
    }
    {
      tHStringPtr hspPath = _H("test/gpufunc/texture_ps.gpufunc.xml");
      niLet dtPixel = niCheckNN(dtPixel,LoadDataTable(niHStr(hspPath)),eFalse);
      _pixelGpuFun = niCheckNN(
        _pixelGpuFun,
        _driverGpu->CreateGpuFunction(eGpuFunctionType_Pixel,hspPath,dtPixel),eFalse);
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(pipelineDesc), eFalse);
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
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->BeginCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0);
    cmdEncoder->SetTexture(_texture, 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 1);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,eGpuIndexType_U32,6,0);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuTexture);

struct GpuTexAlpha : public GpuCanvasBase {
  typedef sVertexPAT1 tVertexFmt;
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);
  NN<iTexture> _texture = niDeferredInit(NN<iTexture>);

  struct sTestUniforms {
    alignas(16) tF32 alphaTest = 0.0f;
    alignas(16) sVec4f materialColor = Vec4f(0,1,1,1);
  };
  static_assert(sizeof(sTestUniforms) == (sizeof(float)*8));
  NN<iGpuBuffer> _uBuffer = niDeferredInit(NN<iGpuBuffer>);

  TEST_CONSTRUCTOR_BASE(GpuTexAlpha,GpuCanvasBase) {
  }

  tBool _Init() niImpl {
    {
      _vaBuffer = niCheckNN(
        _vaBuffer,
        _driverGpu->CreateGpuBuffer(
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
      verts[0] = {{ -0.5f,   0.5f, 0.0f}, 0xFFFF0000, {0.0f,0.0f}}; // Red
      verts[1] = {{  0.5f,   0.5f, 0.0f}, 0xFF00FF00, {1.0f,0.0f}}; // Green
      verts[2] = {{  0.5f,  -0.5f, 0.0f}, 0xFF0000FF, {1.0f,1.0f}}; // Blue
      verts[3] = {{ -0.5f,  -0.5f, 0.0f}, 0xFFFFFFFF, {0.0f,1.0f}}; // White
      _vaBuffer->Unlock();
    }
    {
      _iaBuffer = niCheckNN(
        _iaBuffer,
        _driverGpu->CreateGpuBuffer(
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
      _uBuffer = niCheckNN(
        _uBuffer,
        _driverGpu->CreateGpuBuffer(
          sizeof(sTestUniforms),
          eGpuBufferMemoryMode_Shared,
          eGpuBufferUsageFlags_Uniform),
        eFalse);
      sTestUniforms* uBuffer = (sTestUniforms*)_uBuffer->Lock(0, _uBuffer->GetSize(), eLock_Discard);
      niCheck(uBuffer != nullptr, eFalse);
      uBuffer->alphaTest = 1.0f;
      uBuffer->materialColor = sVec4f::One();
      _uBuffer->Unlock();
    }

    {
      tHStringPtr hspPath = _H("test/gpufunc/texture_alphatest_vs.gpufunc.xml");
      niLet dtVertex = niCheckNN(dtVertex,LoadDataTable(niHStr(hspPath)),eFalse);
      _vertexGpuFun = niCheckNN(
        _vertexGpuFun,
        _driverGpu->CreateGpuFunction(eGpuFunctionType_Vertex,hspPath,dtVertex),eFalse);
    }
    {
      tHStringPtr hspPath = _H("test/gpufunc/texture_alphatest_ps.gpufunc.xml");
      niLet dtPixel = niCheckNN(dtPixel,LoadDataTable(niHStr(hspPath)),eFalse);
      _pixelGpuFun = niCheckNN(
        _pixelGpuFun,
        _driverGpu->CreateGpuFunction(eGpuFunctionType_Pixel,hspPath,dtPixel),eFalse);
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexFmt::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_pixelGpuFun);
      _pipeline = niCheckNN(_pipeline, _driverGpu->CreateGpuPipeline(pipelineDesc), eFalse);
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

    sTestUniforms* uBuffer = (sTestUniforms*)_uBuffer->Lock(0, _uBuffer->GetSize(), eLock_Discard);
    niCheck(uBuffer != nullptr,;);
    {
      niLet cosTime = ni::Cos<tF32>((tF32)mfAnimationTime * 2.0f) * 0.5f + 0.5f;
      uBuffer->materialColor = sVec4f::One() * ((1.0f-cosTime)+0.1f) * 3.0f;
      uBuffer->alphaTest = cosTime;
    }
    _uBuffer->Unlock();

    QPtr<iGraphicsContextGpu> gpuContext = mpWidget->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->BeginCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0);
    cmdEncoder->SetUniformBuffer(_uBuffer, 0, 0);
    cmdEncoder->SetTexture(_texture, 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 1);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,eGpuIndexType_U32,6,0);
  }
};
TEST_FIXTURE_WIDGET(FGpu,GpuTexAlpha);

}
