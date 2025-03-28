#include "stdafx.h"
#include "FGDRV.h"
#include "nish/TestGpuFuncs.hpp"
#include <niLang/Math/MathLib.h>

using namespace ni;

namespace _ {

//----------------------------------------------------------------------------
//
// Section: sFBindless_Base
//
//----------------------------------------------------------------------------
struct sFBindless_Base : public sFGDRV_Base {
  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);
  astl::vector<NN<iTexture>> _textures;
  tU32 _selectedTexture = 0;

  niFn(tBool) OnInit(UnitTest::TestResults& testResults_) niOverride
  {
    CHECK_RET(sFGDRV_Base::OnInit(testResults_), eFalse);
    QPtr<iGraphicsDriverGpu> driverGpu = _graphics->GetDriver();
    CHECK_RET(niIsOK(driverGpu), eFalse);
    _driverGpu = AsNN(driverGpu.raw_ptr());

    {
      NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/earth_d.jpg"));
      _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()), _graphics->LoadBitmap(fp),
        eTextureFlags_Default)));
    }

    {
      NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/glass.tga"));
      _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()), _graphics->LoadBitmap(fp),
        eTextureFlags_Default)));
    }

    {
      NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/rust_steel.jpg"));
      _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()), _graphics->LoadBitmap(fp),
        eTextureFlags_Default)));
    }

    {
      NN<iFile> fp =
        AsNN(_graphics->OpenBitmapFile("test/tex/earth_lights.jpg"));
      _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()), _graphics->LoadBitmap(fp),
        eTextureFlags_Default)));
    }

    {
      NN<iFile> fp =
        AsNN(_graphics->OpenBitmapFile("test/tex/earth_clouds_d.jpg"));
      _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()), _graphics->LoadBitmap(fp),
        eTextureFlags_Default)));
    }

    return eTrue;
  }

  tU32 _GetTextureIndex(iTexture* apTexture) const
  {
    tU32 r = _graphics->GetTextureDeviceResourceManager()->GetIndexFromResource(
      apTexture);
    if (r == eInvalidHandle)
      return 0;
    return r;
  }

  tBool OnKeyDown(tU32 anKey, tU32 anKeyMod) niOverride
  {
    switch (anKey) {
    case eKey_T: {
      _selectedTexture = (_selectedTexture + 1) % (tU32)_textures.size();
      break;
    };
    default: {
      return eFalse;
    }
    }
    return eTrue;
  }
};

//----------------------------------------------------------------------------
//
// Section: Utils
//
//----------------------------------------------------------------------------
static Ptr<iGpuBuffer> MakeSquareVB(ain<nn<iGraphicsDriverGpu>> aDriver,
                                    const sVec2f& aPos, tF32 afSize = 0.5f)
{
  Ptr<iGpuBuffer> buffer = aDriver->CreateGpuBuffer(
    _H("GpuTexture_VA"), sizeof(tVertexCanvas) * 4, eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Vertex);
  niCheck(buffer.IsOK(), nullptr);

  tVertexCanvas* verts =
    (tVertexCanvas*)buffer->Lock(0, buffer->GetSize(), eLock_Discard);
  niCheck(verts != nullptr, nullptr);
  verts[0] = { { aPos.x - afSize, aPos.y + afSize, 0.0f },
               sVec3f::YAxis(),
               0xFFFFFFFF,
               { 0.0f, 0.0f } }; // TL
  verts[1] = { { aPos.x + afSize, aPos.y + afSize, 0.0f },
               sVec3f::YAxis(),
               0xFFFFFFFF,
               { 1.0f, 0.0f } }; // TR
  verts[2] = { { aPos.x + afSize, aPos.y - afSize, 0.0f },
               sVec3f::YAxis(),
               0xFFFFFFFF,
               { 1.0f, 1.0f } }; // BR
  verts[3] = { { aPos.x - afSize, aPos.y - afSize, 0.0f },
               sVec3f::YAxis(),
               0xFFFFFFFF,
               { 0.0f, 1.0f } }; // BL
  buffer->Unlock();

  return buffer;
}

static Ptr<iGpuBuffer> MakeQuadIB(ain<nn<iGraphicsDriverGpu>> aDriver)
{
  Ptr<iGpuBuffer> buffer = aDriver->CreateGpuBuffer(
    _H("GpuTexture_IA"), sizeof(tU32) * 6, eGpuBufferMemoryMode_Shared,
    eGpuBufferUsageFlags_Index);
  niCheck(buffer.IsOK(), nullptr);

  tU32* inds = (tU32*)buffer->Lock(0, buffer->GetSize(), eLock_Discard);
  niCheck(inds != nullptr, nullptr);
  inds[0] = 0;
  inds[1] = 1;
  inds[2] = 2;
  inds[3] = 2;
  inds[4] = 3;
  inds[5] = 0;
  buffer->Unlock();

  return buffer;
}

//----------------------------------------------------------------------------
//
// Section: sFBindless_Textures
//
//----------------------------------------------------------------------------
struct sFBindless_Textures : public sFBindless_Base {
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  tBool OnInit(UnitTest::TestResults& testResults_) niOverride
  {
    CHECK_RET(sFBindless_Base::OnInit(testResults_), eFalse);

    {
      _vaBuffer = niCheckNN(
        _vaBuffer, MakeSquareVB(_driverGpu, sVec2f(0, 0), 0.5f), eFalse);
      _iaBuffer = niCheckNN(_iaBuffer, MakeQuadIB(_driverGpu), eFalse);
    }

    {
      _vertexGpuFun = niCheckNN(
        _vertexGpuFun,
        _driverGpu->CreateGpuFunction(
          eGpuFunctionType_Vertex, _H("test/nish/gpu/texture_vs.gpufunc.xml")),
        eFalse);
      _pixelGpuFun =
        niCheckNN(_pixelGpuFun,
                  _driverGpu->CreateGpuFunction(
                    eGpuFunctionType_Pixel,
                    _H("test/nish/bindless/texture_bindless_ps.gpufunc.xml")),
                  eFalse);
      //CHECK_EQUAL(eGpuFunctionBindType_Bindless,_pixelGpuFun->GetFunctionBindType());
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc =
        niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexCanvas::eFVF);
      pipelineDesc->SetColorFormat(0, eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex, _vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel, _pixelGpuFun);
      _pipeline = niCheckNN(
        _pipeline,
        _driverGpu->CreateGpuPipeline(_H("GpuTexture_Pipeline"), pipelineDesc),
        eFalse);
    }

    return eTrue;
  }

  tBool OnPaint(UnitTest::TestResults& testResults_) niOverride
  {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetTexture(_textures[_selectedTexture], 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    TestGpuFuncs_TestUniforms u;
    u.padding0 = _GetTextureIndex(_textures[_selectedTexture]);
    u.padding1 =
      _GetTextureIndex(_textures[(_selectedTexture + 1) % _textures.size()]);
    u.padding2 =
      _GetTextureIndex(_textures[(_selectedTexture + 2) % _textures.size()]);
    cmdEncoder->StreamUniformBuffer((tPtr)&u, sizeof(u), 0);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList, 0, 1, 0, 0, 6);
    return eTrue;
  }
};
TEST_CLASS(FBindless, Textures);

//----------------------------------------------------------------------------
//
// Section: sFBindless_Instances
//
//----------------------------------------------------------------------------
struct sFBindless_Instances : public sFBindless_Base {
  NN<iGpuBuffer> _vaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _iaBuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _vertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _pixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _pipeline = niDeferredInit(NN<iGpuPipeline>);

  astl::vector<NN<iGpuBuffer>> _instDataBuffers;
  tU32 _instDataIndex0 = eInvalidHandle;

  tBool OnInit(UnitTest::TestResults& testResults_) niOverride
  {
    CHECK_RET(sFBindless_Base::OnInit(testResults_), eFalse);

    {
      _vaBuffer = niCheckNN(
        _vaBuffer, MakeSquareVB(_driverGpu, sVec2f(0, 0), 0.5f), eFalse);
      _iaBuffer = niCheckNN(_iaBuffer, MakeQuadIB(_driverGpu), eFalse);

      niLoop (i, 5) {
        TestGpuFuncs_TestInstanceData instData;
        niLet instDataBuffer =
          niCheckNN(instDataBuffer,
                    _driverGpu->CreateGpuBuffer(HFmt("instData_%s", m_testName),
                                                sizeof(instData),
                                                eGpuBufferMemoryMode_Shared,
                                                eGpuBufferUsageFlags_Storage),
                    eFalse);
        instData.mtxWorld =
          MatrixTranslation(Vec3f(0.1f, 0.1f, 0.0f) * (tF32)i);
        TestGpuFuncs_TestInstanceData* locked =
          (TestGpuFuncs_TestInstanceData*)instDataBuffer->Lock(
            0, instDataBuffer->GetSize(), eLock_Discard);
        *locked = instData;
        instDataBuffer->Unlock();
        _instDataBuffers.emplace_back(instDataBuffer);
      }

      // We assume that all instance buffers are using contiguous resource
      // indices, which is guaranteed if they are indeeded created in order.
      _instDataIndex0 = _driverGpu->GetStorageBufferDeviceResourceManager()
                          ->GetIndexFromResource(_instDataBuffers[0]);
      niDebugFmt(("... _instDataIndex0: %d", _instDataIndex0));
      CHECK_NOT_EQUAL(eInvalidHandle, _instDataIndex0);
    }

    {
      _vertexGpuFun =
        niCheckNN(_vertexGpuFun,
                  _driverGpu->CreateGpuFunction(
                    eGpuFunctionType_Vertex,
                    _H("test/nish/bindless/texture_bindless_vs.gpufunc.xml")),
                  eFalse);
      CHECK_EQUAL(eGpuFunctionBindType_Bindless,
                  _vertexGpuFun->GetFunctionBindType());
      _pixelGpuFun =
        niCheckNN(_pixelGpuFun,
                  _driverGpu->CreateGpuFunction(
                    eGpuFunctionType_Pixel,
                    _H("test/nish/bindless/texture_bindless_ps.gpufunc.xml")),
                  eFalse);
      CHECK_EQUAL(eGpuFunctionBindType_Bindless,
                  _pixelGpuFun->GetFunctionBindType());
    }

    {
      NN<iGpuPipelineDesc> pipelineDesc =
        niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexCanvas::eFVF);
      pipelineDesc->SetColorFormat(0, eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex, _vertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel, _pixelGpuFun);
      _pipeline = niCheckNN(
        _pipeline,
        _driverGpu->CreateGpuPipeline(_H("GpuTexture_Pipeline"), pipelineDesc),
        eFalse);
    }

    return eTrue;
  }

  tBool OnPaint(UnitTest::TestResults& testResults_) niOverride
  {
    QPtr<iGraphicsContextGpu> gpuContext = _graphicsContext;
    niPanicAssert(gpuContext.IsOK());
    NN<iGpuCommandEncoder> cmdEncoder = AsNN(gpuContext->GetCommandEncoder());
    cmdEncoder->SetPipeline(_pipeline);
    cmdEncoder->SetVertexBuffer(_vaBuffer, 0, 0);
    cmdEncoder->SetTexture(_textures[_selectedTexture], 0);
    cmdEncoder->SetSamplerState(eCompiledStates_SS_PointRepeat, 0);
    cmdEncoder->SetIndexBuffer(_iaBuffer, 0, eGpuIndexType_U32);
    TestGpuFuncs_TestUniforms u;
    u.padding0 = _GetTextureIndex(_textures[_selectedTexture]);
    u.padding1 =
      _GetTextureIndex(_textures[(_selectedTexture + 1) % _textures.size()]);
    u.padding2 =
      _GetTextureIndex(_textures[(_selectedTexture + 2) % _textures.size()]);
    cmdEncoder->StreamUniformBuffer((tPtr)&u, sizeof(u), 0);
    cmdEncoder->DrawIndexed(eGraphicsPrimitiveType_TriangleList,
                            _instDataIndex0, _instDataBuffers.size(), 0, 0, 6);
    return eTrue;
  }
};
TEST_CLASS(FBindless, Instances);

} // namespace _
