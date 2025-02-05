#include "stdafx.h"
#include <niLang/Math/MathRect.h>

using namespace ni;

//
// TODO (1/1):
// - [ ] p0: Ray trace with bary centric coordinates
// - [ ] p1: Checkerboard floor
// - [ ] p1: Four reflective sphere on checkerboard floor (white, red, green, blue spheres)
// - [ ] p1: One reflective sphere on checkerboard floor
// - [ ] p1: Ref PT cornell box white-furnace sky
// - [ ] p1: Ref PT cornell box with one light on the ceiling - one emissive quad should be the light source
// - [ ] p1: Visualize: base color
// - [ ] p1: Visualize: normals
// - [ ] p1: Visualize: worldspace position
//

static const tF32 kfRunSpeed = 256.0f;
static const tF32 kfNormalSpeed = 64.0f;

struct FRayTracer {
};

struct RayTracerBase : public ni::cWidgetSinkImpl<> {
  tBool _noTextures;
  tBool _animated;
  tF64 _animTime = 0.0;

  Ptr<iGraphics> _graphics;

  Ptr<iCamera> mptrCamera;
  tBool _cameraInput;
  tBool _mouseLook;
  sVec2f _prevMousePos;

  tBool  _hasInput;
  sVec3f _cameraMove;
  sVec2f _cameraLook;

  sMatrixf _prevViewMtx;
  sMatrixf _prevProjMtx;

  Ptr<iDrawOperationSet> _drawOpSet;

  tU32 _numTriVB = 0;
  tU32 _numTriIB = 0;
  tU32 _numQuadVB = 0;
  tU32 _numQuadIB = 0;

  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);
  NN<iGraphicsDriverRay> _driverRay = niDeferredInit(NN<iGraphicsDriverRay>);

  NN<iGpuBuffer> _displayVABuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _displayIABuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _displayVertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _displayPixelGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _displayPipeline = niDeferredInit(NN<iGpuPipeline>);

  TEST_CONSTRUCTOR(RayTracerBase) {
    _animated = ni::GetProperty("tests.Animated","true").Bool();
    _noTextures = ni::GetProperty("tests.NoTextures","false").Bool();
    _cameraInput = eTrue;
    _mouseLook = eFalse;
  }
  ~RayTracerBase() {
  }

  tBool __stdcall OnSinkAttached() niOverride {
    CHECK(_InitializeCamera());

    _graphics = mpWidget->GetGraphics();

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
    {
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
    }

    _drawOpSet = mpWidget->GetGraphics()->CreateDrawOperationSet();
    return eTrue;
  }

  void _ToggleAnimation() {
    _animated = !_animated;
  }

  tBool __stdcall _InitializeCamera() {
    mptrCamera = mpWidget->GetGraphics()->CreateCamera();
    mpWidget->SetStyle(mpWidget->GetStyle()|eWidgetStyle_HoldFocus);
    mpWidget->SetFocus();
    _mouseLook = eFalse;
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

  void _UpdateCamera() {
    if (!_hasInput && _cameraMove == sVec3f::Zero())
      return;

    const tF32 dt = (tF32)ni::GetLang()->GetFrameTime();
    const tF32 speed = ((mpWidget->GetUIContext()->GetInputModifiers()&eUIInputModifier_Shift) ?
                        kfRunSpeed : kfNormalSpeed);
    mptrCamera->MoveForward(_cameraMove.z * speed * dt);
    mptrCamera->MoveUp(_cameraMove.y * speed * dt);
    mptrCamera->MoveSidewards(_cameraMove.x * speed * dt);
    _cameraLook = Vec2f(0,0);
    _hasInput = eFalse;
  }

  tBool __stdcall OnWheel(tF32 afWheel, const sVec2f& avAbsMousePos) niOverride {
    if (_mouseLook && _cameraInput) {
      mptrCamera->MoveForward(
          afWheel*0.2f*
          ((mpWidget->GetUIContext()->GetInputModifiers()&eUIInputModifier_Shift) ?
           kfRunSpeed : kfNormalSpeed));
    }
    return eFalse;
  }

  tBool __stdcall OnLeftClickDown(const sVec2f& avMP, const sVec2f& avNCMP) niOverride {
    if (_cameraInput) {
      _mouseLook = eTrue;
      _prevMousePos = avNCMP + mpWidget->GetAbsolutePosition();
      mpWidget->SetCapture(eTrue);
    }
    return eFalse;
  }
  tBool __stdcall OnNCLeftClickDown(const sVec2f& avMP, const sVec2f& avNCMP) niOverride {
    return OnLeftClickDown(avMP,avNCMP);
  }
  tBool __stdcall OnLeftClickUp(const sVec2f& avMP, const sVec2f& avNCMP) niOverride {
    if (_cameraInput) {
      _mouseLook = eFalse;
      mpWidget->SetCapture(eFalse);
    }
    return eFalse;
  }
  tBool __stdcall OnNCLeftClickUp(const sVec2f& avMP, const sVec2f& avNCMP) niOverride {
    return OnLeftClickUp(avMP,avNCMP);
  }

  tBool __stdcall OnMouseMove(const sVec2f& avMP, const sVec2f& avNCMP) niOverride {
    if (_mouseLook) {
      const sVec2f newPos = (avNCMP + mpWidget->GetAbsolutePosition());
      sVec2f deltaMove = newPos - _prevMousePos;
      _prevMousePos = newPos;
      mptrCamera->AddPitch(-deltaMove.y / 300.0f);
      mptrCamera->AddYaw(-deltaMove.x / 300.0f);
    }
    return eFalse;
  }
  tBool __stdcall OnNCMouseMove(const sVec2f& avMP, const sVec2f& avNCMP) niOverride {
    return OnMouseMove(avMP,avNCMP);
  }

  tBool __stdcall OnKeyDown(eKey aKey, tU32 aKeyMod) niOverride {
    switch (aKey) {
      case eKey_Space: {
        _ToggleAnimation();
        break;
      }

      case eKey_F4: {
        mptrCamera->SetPosition(Vec3f(0,0,0));
        mptrCamera->SetTarget(mptrCamera->GetPosition() + Vec3f(0,0,1));
        mptrCamera->SetTargetUp(Vec3f(0,1,0));
        break;
      }

        /// Forward ///
      case eKey_Up:
      case eKey_W: {
        _cameraMove.z = 1.0;
        _hasInput = true;
        break;
      }
        /// Backward ///
      case eKey_Down:
      case eKey_S: {
        _cameraMove.z = -1.0;
        _hasInput = true;
        break;
      }
        /// Strafe Left ///
      case eKey_Left:
      case eKey_A: {
        _cameraMove.x = -1.0;
        _hasInput = true;
        break;
      }
        /// Strafe Right ///
      case eKey_Right:
      case eKey_D: {
        _cameraMove.x = 1.0;
        _hasInput = true;
        break;
      }
        /// Move Up ///
      case eKey_PgUp:
      case eKey_R: {
        _cameraMove.y = 1.0;
        _hasInput = true;
        break;
      }
        /// Move Down ///
      case eKey_PgDn:
      case eKey_F: {
        _cameraMove.y = -1.0;
        _hasInput = true;
        break;
      }

        // we dont care about the other keys
      default:
        break;
    }
    return eFalse;
  }

  tBool __stdcall OnKeyUp(eKey aKey, tU32 aKeyMod) niOverride {
    switch (aKey) {
      case eKey_Up:
      case eKey_Down:
      case eKey_W:
      case eKey_S:
        _cameraMove.z = 0.0;
        break;
      case eKey_Left:
      case eKey_Right:
      case eKey_A:
      case eKey_D:
        _cameraMove.x = 0.0;
        break;
      case eKey_PgUp:
      case eKey_PgDn:
      case eKey_R:
      case eKey_F:
        _cameraMove.y = 0.0;
        break;
        // we dont care about the other keys
      default:
        break;
    }
    return eFalse;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niOverride {
    Ptr<iGraphicsContext> gc = apCanvas->GetGraphicsContext();
    gc->ClearBuffers(eClearBuffersFlags_ColorDepthStencil,0,1,0);

    Ptr<iFixedStates> ptrFS = gc->GetFixedStates();
    const sMatrixf wasViewMatrix = ptrFS->GetViewMatrix();
    const sMatrixf wasProjectionMatrix = ptrFS->GetProjectionMatrix();
    ptrFS->SetCameraViewMatrix(mptrCamera->GetViewMatrix());
    ptrFS->SetCameraProjectionMatrix(mptrCamera->GetProjectionMatrix());

    mptrCamera->SetViewport(apCanvas->GetViewport().ToFloat());
		mptrCamera->SetFov(niPif/3.0f);

    _UpdateCamera();

    if (_animated) {
      _animTime += ni::GetLang()->GetFrameTime();
    }

    const sVec4f vTime = Vec4f(
      (tF32)ni::GetLang()->GetTotalFrameTime(),
      (tF32)ni::GetLang()->GetTotalFrameTime(),
      (tF32)ni::Sin(ni::GetLang()->GetTotalFrameTime()),
      (tF32)ni::GetLang()->GetFrameTime()
    );

    _drawOpSet->Draw(gc, mptrCamera->GetFrustum());

    ptrFS->SetCameraViewMatrix(wasViewMatrix);
    ptrFS->SetCameraProjectionMatrix(wasProjectionMatrix);

    cString str = niFmt("Driver: %s, POS: %s, TARGET: %s, UP: %s, Animated: %d\n",
                        mpWidget->GetGraphics()->GetDriver()->GetName(),
                        cString(mptrCamera->GetPosition()).Chars(),
                        cString(mptrCamera->GetTarget()).Chars(),
                        cString(mptrCamera->GetTargetUp()).Chars(),
                        _animated);
    apCanvas->BlitText(
        mpWidget->GetFont(),
        sRectf(5,5),
        eFontFormatFlags_Border,
        str.Chars());

    return eFalse;
  }

  virtual tBool __stdcall LoadTextures() {
    return eTrue;
  }

  Ptr<iMaterial> CreateMaterial(iTexture* apTex) {
    Ptr<iMaterial> mat = mpWidget->GetGraphics()->CreateMaterial();
    mat->SetDepthStencilStates(eCompiledStates_DS_DepthTestAndWrite);
    mat->SetRasterizerStates(eCompiledStates_RS_Filled);
    if (_noTextures && apTex) {
      mat->SetChannelTexture(eMaterialChannel_Base,apTex);
    }
    else {
      mat->SetFlags(eMaterialFlags_Vertex);
    }
    return mat;
  }

  tBool CreateSphere(const sVec3f& avCenter, iTexture* apTex, tF32 afSize = 10.0f, tBool abForceTranslucent = eFalse) {

    Ptr<iGeometry> g = mpWidget->GetGraphics()->CreateGeometryPolygonalSphere(
        eGeometryCreateFlags_Static,eFVF_Position|eFVF_Tex1|eFVF_Normal,
        afSize,16,16,eTrue,0xFFFFFFFF,sMatrixf::Identity());

    Ptr<iMaterial> mat = CreateMaterial(apTex);
    if (abForceTranslucent) {
      mat->SetFlags(
        mat->GetFlags()|eMaterialFlags_Translucent|eMaterialFlags_Transparent);
    }

    Ptr<iDrawOperation> drawOp = mpWidget->GetGraphics()->CreateDrawOperation();
    drawOp->SetVertexArray(g->GetVertexArray());
    drawOp->SetIndexArray(g->GetIndexArray());
    drawOp->SetMaterial(mat);
    sMatrixf mtx;
    drawOp->GetLocalBoundingVolume()->SetCenter(sVec3f::Zero());
    drawOp->GetLocalBoundingVolume()->SetRadius(afSize);
    drawOp->SetMatrix(MatrixTranslation(mtx,avCenter));
    _drawOpSet->Insert(drawOp);
    return eTrue;
  }

  tBool CreateCube(const sVec3f& avCenter, iTexture* apTex,
                   tBool abCW = eTrue, tBool abAlpha = eFalse, tF32 afSize = 10.0f)
  {
    Ptr<iGeometry> g = mpWidget->GetGraphics()->CreateGeometryPolygonalCube(
        eGeometryCreateFlags_Static,eFVF_Position|eFVF_Tex1|eFVF_Normal,
        sVec3f::Zero(),afSize*2.0f,abCW,0xFFFFFFFF,sMatrixf::Identity());

    Ptr<iMaterial> mat = CreateMaterial(apTex);
    if (_noTextures && apTex) {
      if (abAlpha && apTex->GetPixelFormat()->GetNumABits()) {
        mat->SetFlags(mat->GetFlags()|eMaterialFlags_Transparent);
        mat->SetChannelColor(eMaterialChannel_Opacity,Vec4f(1,1,1,0.1f));
      }
    }

    Ptr<iDrawOperation> drawOp = mpWidget->GetGraphics()->CreateDrawOperation();
    drawOp->SetVertexArray(g->GetVertexArray());
    drawOp->SetIndexArray(g->GetIndexArray());
    drawOp->SetMaterial(mat);
    sMatrixf mtx;
    drawOp->GetLocalBoundingVolume()->SetCenter(sVec3f::Zero());
    drawOp->GetLocalBoundingVolume()->SetRadius(afSize);
    drawOp->SetMatrix(MatrixTranslation(mtx,avCenter));
    _drawOpSet->Insert(drawOp);
    return eTrue;
  }

  tBool InitSceneOneBox(tBool abBoxed = eFalse) {
    niCheck(CreateCube(Vec3(0.0f,-130.0f,100.0f),nullptr,eTrue,eFalse,100.0f), eFalse);
    return eTrue;
  }

};

struct Triangle : public RayTracerBase {
  TEST_CONSTRUCTOR_BASE(Triangle,RayTracerBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached());
    CHECK(InitSceneOneBox());
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());

    // NN<iRayCommandEncoder> rayEncoder = AsNN(QueryInterface<iRayCommandEncoder>(gpuEncoder));
    // rayEncoder->SetRayInstances(_instanceAS);

    DisplayTexture(gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,Triangle);
