#include "stdafx.h"
#include <niUI/IGpu.h>
#include <niUI/IRay.h>
#include "../tsrc_gdrv/MakeTestRayGeometry.h"
#include "../../../data/test/gpufunc/TestGpuFuncs.hpp"

#if !defined niOSX
namespace _ {

using namespace ni;

//
// TODO (1/1):
// - [ ] p0: Ray trace a rotating cube with the camera moving around
//  - https://youtu.be/kYb4Io7TQp4?si=loEyfkB7XRrjFn-e&t=19
// - [ ] p0: Ray trace with bary centric coordinates
// - [ ] p0: Ray trace a sphere with a custom intersection test
// - [ ] p0: Ray trace a SDF with a union or difference between two SDF
//  - https://www.youtube.com/watch?v=BNZtUB7yhX4
//  - https://jamie-wong.com/2016/07/15/ray-marching-signed-distance-functions/
//  - https://danielchasehooper.com/posts/code-animated-rick/
//  - https://github.com/sungiant/sdf
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

//----------------------------------------------------------------------------
//
// Section: RayTracerBase
//
//----------------------------------------------------------------------------
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

  struct sGeometry {
    NN<iDrawOperation> _drawOp;

    sGeometry(
      ain<nn<iDrawOperation>> aDop
    ) : _drawOp(aDop)
    {}
  };
  astl::vector<sGeometry> _geoms;

  tU32 _numTriVB = 0;
  tU32 _numTriIB = 0;
  tU32 _numQuadVB = 0;
  tU32 _numQuadIB = 0;

  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);
  NN<iGraphicsDriverRay> _driverRay = niDeferredInit(NN<iGraphicsDriverRay>);

  NN<iGpuBuffer> _displayVABuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuBuffer> _displayIABuffer = niDeferredInit(NN<iGpuBuffer>);
  NN<iGpuFunction> _displayVertexGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuFunction> _displayRayqueryGpuFun = niDeferredInit(NN<iGpuFunction>);
  NN<iGpuPipeline> _displayPipeline = niDeferredInit(NN<iGpuPipeline>);

  TEST_CONSTRUCTOR(RayTracerBase) {
    _animated = ni::GetProperty("tests.Animated","true").Bool();
    _noTextures = ni::GetProperty("tests.NoTextures","false").Bool();
    _cameraInput = eTrue;
    _mouseLook = eFalse;
  }
  ~RayTracerBase() {
  }

  virtual tBool __stdcall OnSinkAttached(iHString* ahspRayqueryGpufuncPath) {
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

      _displayRayqueryGpuFun = niCheckNN(_displayRayqueryGpuFun,_driverGpu->CreateGpuFunction(
        eGpuFunctionType_Pixel,ahspRayqueryGpufuncPath),eFalse);

      NN<iGpuPipelineDesc> pipelineDesc = niCheckNN(pipelineDesc, _driverGpu->CreateGpuPipelineDesc(), eFalse);
      pipelineDesc->SetFVF(tVertexCanvas::eFVF);
      pipelineDesc->SetColorFormat(0,eGpuPixelFormat_BGRA8);
      pipelineDesc->SetDepthFormat(eGpuPixelFormat_D32);
      pipelineDesc->SetFunction(eGpuFunctionType_Vertex,_displayVertexGpuFun);
      pipelineDesc->SetFunction(eGpuFunctionType_Pixel,_displayRayqueryGpuFun);
      _displayPipeline = niCheckNN(_displayPipeline, _driverGpu->CreateGpuPipeline(_H("RayDisplay_Pipeline"),pipelineDesc), eFalse);
    }
    return eTrue;
  }

  void _ToggleAnimation() {
    _animated = !_animated;
  }

  void _ResetCamera() {
    // TODO: Kinda wierd starting point because it aligns with the tiny
    // default triangle which was setup for NDC. Will reset to something
    // simpler once we have the full geom.
    mptrCamera->SetPosition(Vec3f(-2.5f,-0.5f,-3.0f));
    mptrCamera->SetTarget(mptrCamera->GetPosition() + Vec3f(0,0,1));
    mptrCamera->SetTargetUp(Vec3f(0,1,0));
  }

  tF32 _GetCameraSpeed() const {
    const tF32 speed = ((mpWidget->GetUIContext()->GetInputModifiers()&eUIInputModifier_Shift) ?
                        kfRunSpeed : kfNormalSpeed);
    return speed;
  }

  tBool __stdcall _InitializeCamera() {
    mptrCamera = mpWidget->GetGraphics()->CreateCamera();
    mpWidget->SetStyle(mpWidget->GetStyle()|eWidgetStyle_HoldFocus);
    mpWidget->SetFocus();
    _ResetCamera();
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
    const tF32 speed = _GetCameraSpeed();
    mptrCamera->MoveForward(_cameraMove.z * speed * dt);
    mptrCamera->MoveUp(_cameraMove.y * speed * dt);
    mptrCamera->MoveSidewards(_cameraMove.x * speed * dt);
    _cameraLook = Vec2f(0,0);
    _hasInput = eFalse;
  }

  tBool __stdcall OnWheel(tF32 afWheel, const sVec2f& avAbsMousePos) niOverride {
    if (_mouseLook && _cameraInput) {
      mptrCamera->MoveForward(
          afWheel*0.2f*_GetCameraSpeed());
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
        _ResetCamera();
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

    iFrustum* frustum = mptrCamera->GetFrustum();
    niLoop(i,_geoms.size()) {
      niLet& geom = _geoms[i];
      nn<iDrawOperation> dop = geom._drawOp;
      if (frustum && dop->GetBoundingVolume()) {
        if (dop->GetBoundingVolume()->IntersectFrustum(NULL,frustum) ==
            eIntersectionResult_None) {
          continue; // skip this draw op...
        }
      }
      gc->DrawOperation(dop);
    }

    ptrFS->SetCameraViewMatrix(wasViewMatrix);
    ptrFS->SetCameraProjectionMatrix(wasProjectionMatrix);

    cString str = niFmt("Driver: %s, POS: %s, TARGET: %s, UP: %s, Animated: %d, VIEW: %s, PROJ: %s\n",
                        mpWidget->GetGraphics()->GetDriver()->GetName(),
                        cString(mptrCamera->GetPosition()).Chars(),
                        cString(mptrCamera->GetTarget()).Chars(),
                        cString(mptrCamera->GetTargetUp()).Chars(),
                        _animated,
                        mptrCamera->GetViewMatrix(),
                        mptrCamera->GetProjectionMatrix());
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

    niLet drawOp = niCheckNN(drawOp,mpWidget->GetGraphics()->CreateDrawOperation(),eFalse);
    drawOp->SetVertexArray(g->GetVertexArray());
    drawOp->SetIndexArray(g->GetIndexArray());
    drawOp->SetMaterial(mat);
    drawOp->GetLocalBoundingVolume()->SetCenter(sVec3f::Zero());
    drawOp->GetLocalBoundingVolume()->SetRadius(afSize);
    drawOp->SetMatrix(MatrixTranslation(avCenter));
    _geoms.emplace_back(sGeometry(drawOp));
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

    niLet drawOp = niCheckNN(drawOp,mpWidget->GetGraphics()->CreateDrawOperation(),eFalse);
    drawOp->SetVertexArray(g->GetVertexArray());
    drawOp->SetIndexArray(g->GetIndexArray());
    drawOp->SetMaterial(mat);
    drawOp->GetLocalBoundingVolume()->SetCenter(sVec3f::Zero());
    drawOp->GetLocalBoundingVolume()->SetRadius(afSize);
    drawOp->SetMatrix(MatrixTranslation(avCenter));
    _geoms.emplace_back(sGeometry(drawOp));
    return eTrue;
  }

  tBool InitSceneOneBox(tBool abBoxed = eFalse) {
    niCheck(CreateCube(Vec3(0.0f,-130.0f,100.0f),nullptr,eTrue,eFalse,100.0f), eFalse);
    return eTrue;
  }

  tBool InitSceneManyBoxes(tBool abBoxed = eFalse) {
    niCheck(CreateCube(Vec3(0.0f,-130.0f,100.0f),nullptr,eTrue,eFalse,100.0f), eFalse);

    niCheck(CreateCube(Vec3(-25.0f,-20.0f,100.0f),nullptr),eFalse);
    niCheck(CreateCube(Vec3(-30.0f,-10.0f,110.0f),nullptr),eFalse);
    niCheck(CreateCube(Vec3(-30.0f, 10.0f,105.0f),nullptr),eFalse);
    niCheck(CreateCube(Vec3( 25.0f,-20.0f,100.0f),nullptr),eFalse);
    niCheck(CreateCube(Vec3( 30.0f,-10.0f,110.0f),nullptr),eFalse);

    niCheck(CreateCube(Vec3( 30.0f, 10.0f,105.0f),nullptr),eFalse);
    niCheck(CreateCube(Vec3(  0.0f,-15.0f,75.0f),nullptr,eTrue,eTrue),eFalse);
    return eTrue;
  }

};

//----------------------------------------------------------------------------
//
// Section: Triangle
//
//----------------------------------------------------------------------------
struct Triangle : public RayTracerBase {
  NN<iRayInstances> _instanceAS = niDeferredInit(NN<iRayInstances>);

  TEST_CONSTRUCTOR_BASE(Triangle,RayTracerBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(_H("test/gpufunc/triangle_rayquery_ps.gpufunc.xml")));

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(_H("RayQueryInstancesDesc_Triangle")),
        eFalse);

      // Add a triangle
      {
        niLet prDesc = niCheckNN(
          prDesc,
          _driverRay->CreateRayTrianglePrimitivesDesc(_H("RayQueryTrianglePrimitivesDesc_Triangle")),
          eFalse);

        niLet triangleVB = MakeTriVB(_driverGpu,++_numTriVB,1.0f,Vec3f(0,0,0.3f));
        niCheck(prDesc->AddTriangles(
          triangleVB,0,sizeof(tVertexTri),3,
          sMatrixf::Identity(),
          eRayPrimitiveFlags_Opaque,
          0), eFalse);

        niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayTrianglePrimitives(
          _H("RayTrianglePrimitives_Triangle"),prDesc), eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          0,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);
      }

      // Add all draw operations
      {
        niLoop(i,_geoms.size()) {
          niLet& geom = _geoms[i];
          nn<iDrawOperation> dop = geom._drawOp;
          niLet fvfDesc = cFVFDescription(dop->GetVertexArray()->GetFVF());
          NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
          NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));

          niLet prDesc = niCheckNN(
            prDesc,
            _driverRay->CreateRayTrianglePrimitivesDesc(HFmt("%s_PrimDesc_%d",m_testName,i)),
            eFalse);

          const tU32 firstInd = dop->GetFirstIndex();
          tU32 numInds = dop->GetNumIndices();
          if (!numInds) {
            numInds = dop->GetIndexArray()->GetNumIndices()-firstInd;
          }
          niCheck(prDesc->AddTrianglesIndexed(
            vaBuffer,
            dop->GetBaseVertexIndex()*fvfDesc.GetStride(),
            fvfDesc.GetStride(),
            dop->GetVertexArray()->GetNumVertices(),
            iaBuffer,
            firstInd*sizeof(tU32),
            eGpuIndexType_U32,
            numInds,
            sMatrixf::Identity(),
            eRayPrimitiveFlags_Opaque,
            0), eFalse);

          niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayTrianglePrimitives(
            HFmt("%s_Prim_%d",m_testName,i),prDesc), eFalse);

          niCheck(instDesc->AddInstance(
            primitiveAS,
            dop->GetMatrix(),     // Transform
            0,                    // Instance ID
            0xFF,                 // Mask
            0,                    // Hit group offset
            eRayInstanceFlags_None), eFalse);
        }
      }

      _instanceAS = niCheckNN(_instanceAS, buildEncoder->BuildRayInstances(
        _H("RayInstances_Triangle"),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetViewport().GetWidth();
    u.rtHeight = (tF32)apCanvas->GetViewport().GetHeight();
    u.cameraInvView = MatrixInverse(mptrCamera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(mptrCamera->GetViewMatrix() * mptrCamera->GetProjectionMatrix());
    u.cameraFarClipPlane = mptrCamera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_instanceAS);

    DisplayTexture(gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,Triangle);

//----------------------------------------------------------------------------
//
// Section: ManyBoxes
//
//----------------------------------------------------------------------------
struct ManyBoxes : public RayTracerBase {
  NN<iRayInstances> _instanceAS = niDeferredInit(NN<iRayInstances>);

  TEST_CONSTRUCTOR_BASE(ManyBoxes,RayTracerBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(_H("test/gpufunc/triangle_rayquery_ps.gpufunc.xml")));
    CHECK(InitSceneManyBoxes());

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(_H("RayQueryInstancesDesc_ManyBoxes")),
        eFalse);

      // Add a triangle
      {
        niLet prDesc = niCheckNN(
          prDesc,
          _driverRay->CreateRayTrianglePrimitivesDesc(_H("RayQueryManyBoxesPrimitivesDesc_ManyBoxes")),
          eFalse);

        niLet triangleVB = MakeTriVB(_driverGpu,++_numTriVB,1.0f,Vec3f(0,0,0.3f));
        niCheck(prDesc->AddTriangles(
          triangleVB,0,sizeof(tVertexTri),3,
          sMatrixf::Identity(),
          eRayPrimitiveFlags_Opaque,
          0), eFalse);

        niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayTrianglePrimitives(
          _H("RayTrianglePrimitives_ManyBoxes"),prDesc), eFalse);

        niCheck(instDesc->AddInstance(
          primitiveAS,
          sMatrixf::Identity(), // Transform
          0,                    // Instance ID
          0xFF,                 // Mask
          0,                    // Hit group offset
          eRayInstanceFlags_None), eFalse);
      }

      // Add all draw operations
      {
        niLoop(i,_geoms.size()) {
          niLet& geom = _geoms[i];
          nn<iDrawOperation> dop = geom._drawOp;
          niLet fvfDesc = cFVFDescription(dop->GetVertexArray()->GetFVF());
          NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
          NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));

          niLet prDesc = niCheckNN(
            prDesc,
            _driverRay->CreateRayTrianglePrimitivesDesc(HFmt("%s_PrimDesc_%d",m_testName,i)),
            eFalse);

          const tU32 firstInd = dop->GetFirstIndex();
          tU32 numInds = dop->GetNumIndices();
          if (!numInds) {
            numInds = dop->GetIndexArray()->GetNumIndices()-firstInd;
          }
          niCheck(prDesc->AddTrianglesIndexed(
            vaBuffer,
            dop->GetBaseVertexIndex()*fvfDesc.GetStride(),
            fvfDesc.GetStride(),
            dop->GetVertexArray()->GetNumVertices(),
            iaBuffer,
            firstInd*sizeof(tU32),
            eGpuIndexType_U32,
            numInds,
            sMatrixf::Identity(),
            eRayPrimitiveFlags_Opaque,
            0), eFalse);

          niLet primitiveAS = niCheckNN(primitiveAS, buildEncoder->BuildRayTrianglePrimitives(
            HFmt("%s_Prim_%d",m_testName,i),prDesc), eFalse);

          niCheck(instDesc->AddInstance(
            primitiveAS,
            dop->GetMatrix(),     // Transform
            0,                    // Instance ID
            0xFF,                 // Mask
            0,                    // Hit group offset
            eRayInstanceFlags_None), eFalse);
        }
      }

      _instanceAS = niCheckNN(_instanceAS, buildEncoder->BuildRayInstances(
        _H("RayInstances_ManyBoxes"),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetViewport().GetWidth();
    u.rtHeight = (tF32)apCanvas->GetViewport().GetHeight();
    u.cameraInvView = MatrixInverse(mptrCamera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(mptrCamera->GetViewMatrix() * mptrCamera->GetProjectionMatrix());
    u.cameraFarClipPlane = mptrCamera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_instanceAS);

    DisplayTexture(gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,ManyBoxes);

}
#endif
