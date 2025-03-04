#include "stdafx.h"
#include <niUI/Utils/RayUtils.h>
#include "../../../data/test/nish/TestGpuFuncs.hpp"

#if !defined niOSX
namespace _ {

using namespace ni;

static const tF32 kfRunSpeed = 256.0f;
static const tF32 kfNormalSpeed = 64.0f;

struct FRayTracer {
};

typedef sVertexPNT1 tVertexRay;

//----------------------------------------------------------------------------
//
// Section: RayTracerBase
//
//----------------------------------------------------------------------------
struct RayTracerBase : public ni::cWidgetSinkImpl<> {
  tBool _noTextures;
  tBool _animated = eTrue;
  tF64 _animationTime = 0.0;
  tF32 _pingpongTime = 0.0;

  Ptr<iGraphics> _graphics;

  Ptr<iCamera> _camera;
  tBool _cameraInput;
  tBool _mouseLook;
  sVec2f _prevMousePos;

  tBool  _hasInput;
  sVec3f _cameraMove;
  sVec2f _cameraLook;

  sMatrixf _prevViewMtx;
  sMatrixf _prevProjMtx;

  astl::vector<NN<iTexture>> _textures;
  tU32 _selectedTexture = 0;

  struct sGeometry {
    NN<iDrawOperation> _drawOp;
    const sMatrixf _startMatrix;
    Ptr<iRayPrimitives> _rayPrims;
    tU32 _instIndex = eInvalidHandle;

    sGeometry(ain<nn<iDrawOperation>> aDop)
        : _drawOp(aDop)
        , _startMatrix(aDop->GetMatrix())
    {
    }
  };
  astl::vector<sGeometry> _geoms;

  tU32 _numTriVB = 0;
  tU32 _numTriIB = 0;
  tU32 _numQuadVB = 0;
  tU32 _numQuadIB = 0;

  NN<iGraphicsDriverGpu> _driverGpu = niDeferredInit(NN<iGraphicsDriverGpu>);
  NN<iGraphicsDriverRay> _driverRay = niDeferredInit(NN<iGraphicsDriverRay>);

  unn<sDisplayQuad> _display = niDeferredInit(unn<sDisplayQuad>);

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
      _display = niMakeUNN(MakeDisplayQuad(_driverGpu,ahspRayqueryGpufuncPath));
    }

    return eTrue;
  }

  void _ToggleAnimation() {
    _animated = !_animated;
    niLog(Info,niFmt("Toggled animation: %z.",_animated));
  }

  void _ResetCamera() {
    // TODO: Kinda wierd starting point because it aligns with the tiny
    // default triangle which was setup for NDC. Will reset to something
    // simpler once we have the full geom.
    _camera->SetPosition(Vec3f(-2.5f,-0.5f,-3.0f));
    _camera->SetTarget(_camera->GetPosition() + Vec3f(0,0,1));
    _camera->SetTargetUp(Vec3f(0,1,0));
  }

  tF32 _GetCameraSpeed() const {
    const tF32 speed = ((mpWidget->GetUIContext()->GetInputModifiers()&eUIInputModifier_Shift) ?
                        kfRunSpeed : kfNormalSpeed);
    return speed;
  }

  tBool __stdcall _InitializeCamera() {
    _camera = mpWidget->GetGraphics()->CreateCamera();
    mpWidget->SetStyle(mpWidget->GetStyle()|eWidgetStyle_HoldFocus);
    mpWidget->SetFocus();
    _ResetCamera();
    _mouseLook = eFalse;
    return eTrue;
  }

  void _UpdateCamera() {
    if (!_hasInput && _cameraMove == sVec3f::Zero())
      return;

    const tF32 dt = (tF32)ni::GetLang()->GetFrameTime();
    const tF32 speed = _GetCameraSpeed();
    _camera->MoveForward(_cameraMove.z * speed * dt);
    _camera->MoveUp(_cameraMove.y * speed * dt);
    _camera->MoveSidewards(_cameraMove.x * speed * dt);
    _cameraLook = Vec2f(0,0);
    _hasInput = eFalse;
  }

  tBool __stdcall OnWheel(tF32 afWheel, const sVec2f& avAbsMousePos) niOverride {
    if (_mouseLook && _cameraInput) {
      _camera->MoveForward(
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
      _camera->AddPitch(-deltaMove.y / 300.0f);
      _camera->AddYaw(-deltaMove.x / 300.0f);
    }
    return eFalse;
  }
  tBool __stdcall OnNCMouseMove(const sVec2f& avMP, const sVec2f& avNCMP) niOverride {
    return OnMouseMove(avMP,avNCMP);
  }

  tBool __stdcall OnKeyDown(eKey aKey, tU32 aKeyMod) niOverride {
    switch (aKey) {
      case eKey_T: {
        _selectedTexture = (_selectedTexture+1)%(tU32)_textures.size();
        break;
      };

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
    ptrFS->SetCameraViewMatrix(_camera->GetViewMatrix());
    ptrFS->SetCameraProjectionMatrix(_camera->GetProjectionMatrix());

    _camera->SetViewport(apCanvas->GetViewport().ToFloat());
		_camera->SetFov(niPif/3.0f);

    _UpdateCamera();

    if (_animated) {
      this->_animationTime += ni::GetLang()->GetFrameTime();
      this->_pingpongTime = ni::Cos<tF32>((tF32)_animationTime * 2.0f) * 0.5f + 0.5f;
    }

    const sVec4f vTime = Vec4f(
      (tF32)ni::GetLang()->GetTotalFrameTime(),
      (tF32)ni::GetLang()->GetTotalFrameTime(),
      (tF32)ni::Sin(ni::GetLang()->GetTotalFrameTime()),
      (tF32)ni::GetLang()->GetFrameTime()
    );

    iFrustum* frustum = _camera->GetFrustum();
    niLoop(i,_geoms.size()) {
      niVar& geom = _geoms[i];
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
                        cString(_camera->GetPosition()).Chars(),
                        cString(_camera->GetTarget()).Chars(),
                        cString(_camera->GetTargetUp()).Chars(),
                        _animated,
                        _camera->GetViewMatrix(),
                        _camera->GetProjectionMatrix());
    apCanvas->BlitText(
        mpWidget->GetFont(),
        sRectf(5,5),
        eFontFormatFlags_Border,
        str.Chars());

    return eFalse;
  }

  tBool LoadTextures() {
    niTry {
      {
        NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/earth_d.jpg"));
        _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
          _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default)));
      }

      {
        NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/glass.tga"));
        _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
          _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default)));
      }

      {
        NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/rust_steel.jpg"));
        _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
          _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default)));
      }

      {
        NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/earth_lights.jpg"));
        _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
          _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default)));
      }

      {
        NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/earth_clouds_d.jpg"));
        _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
          _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default)));
      }

      {
        NN<iFile> fp = AsNN(_graphics->OpenBitmapFile("test/tex/church2k.dds"));
        _textures.emplace_back(AsNN(_graphics->CreateTextureFromBitmap(
          _H(fp->GetSourcePath()),_graphics->LoadBitmap(fp),eTextureFlags_Default)));
      }
    } niCatch(ni::iPanicException,e) {
      return eFalse;
    }

    return eTrue;
  }

  tU32 _GetTextureIndex(iTexture* apTexture) const {
    tU32 r = _graphics->GetTextureDeviceResourceManager()->GetIndexFromResource(apTexture);
    if (r == eInvalidHandle)
      return 0;
    return r;
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

  tBool CreatePolySphere(const sVec3f& avCenter, iTexture* apTex, tF32 afSize = 10.0f, tBool abForceTranslucent = eFalse) {
    Ptr<iGeometry> g = mpWidget->GetGraphics()->CreateGeometryPolygonalSphere(
      eGeometryCreateFlags_Static,tVertexRay::eFVF,
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

  tBool CreatePolyCube(
    ain<sVec3f> avCenter, ain<sVec2f> aRot, iTexture* apTex,
    tBool abCW = eTrue, tBool abAlpha = eFalse, tF32 afSize = 10.0f)
  {
    Ptr<iGeometry> g = mpWidget->GetGraphics()->CreateGeometryPolygonalCube(
      eGeometryCreateFlags_Static,tVertexRay::eFVF,
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
    drawOp->SetMatrix(MatrixRotationY(aRot.y) * MatrixRotationX(aRot.x) * MatrixTranslation(avCenter));
    _geoms.emplace_back(sGeometry(drawOp));
    return eTrue;
  }

  tBool AddScenePolyGround() {
    niLog(Info,"AddSceneGround Begin");
    niCheck(CreatePolyCube(Vec3(0.0f,-130.0f,100.0f),sVec2f::Zero(),nullptr,eTrue,eFalse,100.0f), eFalse);
    niLog(Info,"AddSceneGround End");
    return eTrue;
  }

  tBool AddSceneSevenPolyBoxes() {
    niLog(Info,"AddSceneManyBoxes Begin");

    // left column, bottom cube, flip it so that we can easily tell if we're displaying the normals in the right space
    niCheck(CreatePolyCube(Vec3(-25.0f,-20.0f,100.0f),sVec2f(0.0f,niPif),nullptr),eFalse);
    // left column, middle
    niCheck(CreatePolyCube(Vec3(-30.0f,-10.0f,110.0f),sVec2f::Zero(),nullptr),eFalse);
    // left column, top
    niCheck(CreatePolyCube(Vec3(-30.0f, 10.0f,105.0f),sVec2f(-niPif/6.0f,0.0f),nullptr),eFalse);

    // right column
    niCheck(CreatePolyCube(Vec3( 25.0f,-20.0f,100.0f),sVec2f::Zero(),nullptr),eFalse);
    niCheck(CreatePolyCube(Vec3( 30.0f,-10.0f,110.0f),sVec2f::Zero(),nullptr),eFalse);
    niCheck(CreatePolyCube(Vec3( 30.0f, 10.0f,105.0f),Vec2f(0.0f,niPif/4.0f),nullptr),eFalse);

    // front cube
    niCheck(CreatePolyCube(Vec3(  0.0f,-25.0f,75.0f),Vec2f(niPif/8.0f,niPif+(niPif/4.0f)),nullptr,eTrue,eTrue),eFalse);

    niLog(Info,"AddSceneManyBoxes End");
    return eTrue;
  }

  tBool AddSceneFourPolySpheres() {
    niLog(Info,"AddSceneFourSpheres Begin");
    niCheck(CreatePolySphere(Vec3(-75.0f,-20.0f,150.0f),nullptr,20.0f,eTrue),eFalse);
    niCheck(CreatePolySphere(Vec3( 75.0f,-20.0f,150.0f),nullptr,20.0f,eTrue),eFalse);
    niCheck(CreatePolySphere(Vec3( 75.0f,-20.0f,200.0f),nullptr,20.0f,eTrue),eFalse);
    niCheck(CreatePolySphere(Vec3(-75.0f,-20.0f,200.0f),nullptr,20.0f,eTrue),eFalse);
    niLog(Info,"AddSceneFourSpheres End");
    return eTrue;
  }

  tBool AddSceneManyPolySpheres(const tU32 sphereCount = 25, const tU32 stacks = 2) {
    niLog(Info,"AddSceneManyPolySpheres Begin");
    const tF32 sphereDistance = 65.0f;
    tF32 sphereRad = 0.0f;

    niLoop(j,stacks) {
      niLoop(i,sphereCount) {
        sphereRad += (tF32)2.0f*niPif/(tF32)sphereCount;
        sVec3f p;
        p.y = -20.0f + (tF32)j*20.0f;
        p.x = ni::Sin(sphereRad)*sphereDistance;
        p.z = ni::Cos(sphereRad)*sphereDistance;
        niCheck(CreatePolySphere(p+Vec3(0.0f,0.0f,100.0f),nullptr,8.0f),eFalse);
      }
    }

    niLog(Info,"AddSceneManyPolySpheres End");
    return eTrue;
  }

  tBool AddSceneRotatingCube() {
    niLog(Info,"AddSceneRotatingCube Begin");

    niCheck(CreatePolyCube(Vec3(  20.0f,-15.0f,50.0f),Vec2f(0.0f,0.0f),nullptr,eTrue,eTrue),eFalse);

    niLog(Info,"AddSceneRotatingCube End");
    return eTrue;
  }

  tBool AddRayTriangle(ain<nn<iRayBuildEncoder>> aBuildEncoder, ain<nn<iRayInstancesDesc>> aInstDesc, tU32 anInstIndex, ain<sVec3f> aOffset = sVec3f::Zero()) {
    niLog(Info,"AddRayTriangle Begin");
    niLet prDesc = niCheckNN(
      prDesc,
      _driverRay->CreateRayTrianglePrimitivesDesc(HFmt("%s_RayTrianglePrimitivesDesc",m_testName)),
      eFalse);

    niLet triangleVB = MakeTriVB(_driverGpu,++_numTriVB,1.0f,Vec3f(aOffset.x,aOffset.y,aOffset.z+0.3f));
    niCheck(prDesc->AddTriangles(
      triangleVB,0,sizeof(tVertexTri),3,
      sMatrixf::Identity(),
      eRayPrimitiveFlags_Opaque,
      0), eFalse);

    niLet primitiveAS = niCheckNN(primitiveAS, aBuildEncoder->BuildRayTrianglePrimitives(
      HFmt("%s_RayTrianglePrimitives",m_testName),prDesc), eFalse);

    niCheck(aInstDesc->AddInstance(
      primitiveAS,
      sMatrixf::Identity(),
      anInstIndex,
      0xFF, 0, // mask, hitgroup offset
      eRayInstanceFlags_None), eFalse);

    niLog(Info,"AddRayTriangle End");
    return eTrue;
  }

  tBool AddRayGeoms(ain<nn<iRayBuildEncoder>> aBuildEncoder, ain<nn<iRayInstancesDesc>> aInstDesc, ain<tU32> anInstIndex) {
    niCheck(!_geoms.empty(),eFalse);
    niLog(Info,"AddRayGeoms Begin");

    niVar instIndex = anInstIndex;
    niLoop(i,_geoms.size()) {
      niVar& geom = _geoms[i];
      nn<iDrawOperation> dop = geom._drawOp;

      if (!geom._rayPrims.has_value()) {
        niLet fvfDesc = cFVFDescription(dop->GetVertexArray()->GetFVF());
        NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
        NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));

        niLet prDesc = niCheckNN(
          prDesc,
          _driverRay->CreateRayTrianglePrimitivesDesc(
            HFmt("%s_RayTrianglePrimitiveDesc_%d",m_testName,i)),
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

        geom._rayPrims = niCheckNN(geom._rayPrims, aBuildEncoder->BuildRayTrianglePrimitives(
          HFmt("%s_Prim_%d",m_testName,i),prDesc), eFalse);
      }

      niCheck(aInstDesc->AddInstance(
        geom._rayPrims,
        dop->GetMatrix(),
        instIndex++,
        0xFF,0,
        eRayInstanceFlags_None), eFalse);
    }

    niLog(Info,"AddRayGeoms End");
    return eTrue;
  }

  tBool UpdateRayGeomsTransforms(ain<nn<iRayBuildEncoder>> aBuildEncoder, ain<nn<iRayInstancesDesc>> aInstDesc) {
    niCheck(_geoms.size() == aInstDesc->GetNumInstances(), eFalse);

    niLoop(i,_geoms.size()) {
      niLet& geom = _geoms[i];
      nn<iDrawOperation> dop = geom._drawOp;
      niCheck(aInstDesc->UpdateInstanceTransform(i,dop->GetMatrix()), eFalse);
    }

    return eTrue;
  }

};

//----------------------------------------------------------------------------
//
// Section: Triangle
//
//----------------------------------------------------------------------------
struct Triangle : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);

  TEST_CONSTRUCTOR_BASE(Triangle,RayTracerBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_instindex_ps.gpufunc.xml")));

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      niCheck(AddRayTriangle(buildEncoder,instDesc,1),eFalse);

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,Triangle);

//----------------------------------------------------------------------------
//
// Section: VisInstIndex
//
//----------------------------------------------------------------------------
struct VisInstIndex : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);

  TEST_CONSTRUCTOR_BASE(VisInstIndex,RayTracerBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_instindex_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      tU32 instIndex = 1;
      niCheck(AddRayTriangle(buildEncoder,instDesc,instIndex++),eFalse);
      niCheck(AddRayTriangle(buildEncoder,instDesc,instIndex++,Vec3f(0.3f,0.0f,0.1f)),eFalse);
      niCheck(AddRayGeoms(buildEncoder,instDesc,instIndex++),eFalse);

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,VisInstIndex);

//----------------------------------------------------------------------------
//
// Section: ManyPolySpheresInstIndex
//
//----------------------------------------------------------------------------
struct ManyPolySpheresInstIndex : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);

  TEST_CONSTRUCTOR_BASE(ManyPolySpheresInstIndex,RayTracerBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_instindex_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneFourPolySpheres());
    CHECK(AddSceneManyPolySpheres(25,2));

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      niCheck(AddRayTriangle(buildEncoder,instDesc,1),eFalse);
      niCheck(AddRayGeoms(buildEncoder,instDesc,2),eFalse);

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,ManyPolySpheresInstIndex);

//----------------------------------------------------------------------------
//
// Section: VisBary
//
//----------------------------------------------------------------------------
struct VisBary : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);

  TEST_CONSTRUCTOR_BASE(VisBary,RayTracerBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_bary_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      niCheck(AddRayTriangle(buildEncoder,instDesc,1),eFalse);
      niCheck(AddRayGeoms(buildEncoder,instDesc,2),eFalse);

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,VisBary);

//----------------------------------------------------------------------------
//
// Section: VisPrimIndex
//
//----------------------------------------------------------------------------
struct VisPrimIndex : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);

  TEST_CONSTRUCTOR_BASE(VisPrimIndex,RayTracerBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_primindex_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      niCheck(AddRayTriangle(buildEncoder,instDesc,1),eFalse);
      niCheck(AddRayGeoms(buildEncoder,instDesc,2),eFalse);

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,VisPrimIndex);

//----------------------------------------------------------------------------
//
// Section: VisTex0
//
//----------------------------------------------------------------------------
struct VisTex0 : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);
  astl::vector<NN<iGpuBuffer>> _instDataBuffers;

  TEST_CONSTRUCTOR_BASE(VisTex0,RayTracerBase) {
  }

  tU32 AddInstData(ain<TestGpuFuncs_RayInstanceData> aInstData) {
    niLet instDataBuffer = niCheckNN(instDataBuffer, _driverGpu->CreateGpuBuffer(
      HFmt("instData_%s_%d",m_testName,_instDataBuffers.size()),
      sizeof(aInstData),
      eGpuBufferMemoryMode_Shared,
      eGpuBufferUsageFlags_Storage), eInvalidHandle);
    niVar locked = (niDeclTypeBase(aInstData)*)instDataBuffer->Lock(
      0, instDataBuffer->GetSize(), eLock_Discard);
    *locked = aInstData;
    instDataBuffer->Unlock();
    _instDataBuffers.emplace_back(instDataBuffer);
    return _driverGpu->GetStorageBufferDeviceResourceManager()->
        GetIndexFromResource(instDataBuffer);
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_tex0_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      {
        // 0,0 is invalid it should result in an error color
        TestGpuFuncs_RayInstanceData instData;
        instData.vbIndex = 0;
        instData.ibIndex = 0;
        tU32 triInstData = AddInstData(instData);
        niCheck(AddRayTriangle(buildEncoder,instDesc,triInstData),eFalse);
        niDebugFmt(("... triInstData: %d", triInstData));
      }

      {
        tU32 firstInstIndex = eInvalidHandle;
        niLoop(i,_geoms.size()) {
          niVar& geom = _geoms[i];
          nn<iDrawOperation> dop = geom._drawOp;
          NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));
          NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
          TestGpuFuncs_RayInstanceData instData;
          instData.vbIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
              GetIndexFromResource(vaBuffer);
          instData.ibIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
              GetIndexFromResource(iaBuffer);
          niDebugFmt(("... geom[%d]: vbIndex: %d, ibIndex: %d",
                      i,instData.vbIndex,instData.ibIndex));
          geom._instIndex = AddInstData(instData);
          if (firstInstIndex == eInvalidHandle) {
            firstInstIndex = geom._instIndex;
          }
        }
        niDebugFmt(("... firstInstIndex: %d", firstInstIndex));
        niCheck(AddRayGeoms(buildEncoder,instDesc,firstInstIndex),eFalse);
      }

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,VisTex0);

//----------------------------------------------------------------------------
//
// Section: VisNormalsObj
//
//----------------------------------------------------------------------------
struct VisNormalsObj : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);
  astl::vector<NN<iGpuBuffer>> _instDataBuffers;

  TEST_CONSTRUCTOR_BASE(VisNormalsObj,RayTracerBase) {
  }

  tU32 AddInstData(ain<TestGpuFuncs_RayInstanceData> aInstData) {
    niLet instDataBuffer = niCheckNN(instDataBuffer, _driverGpu->CreateGpuBuffer(
      HFmt("instData_%s_%d",m_testName,_instDataBuffers.size()),
      sizeof(aInstData),
      eGpuBufferMemoryMode_Shared,
      eGpuBufferUsageFlags_Storage), eInvalidHandle);
    niVar locked = (niDeclTypeBase(aInstData)*)instDataBuffer->Lock(
      0, instDataBuffer->GetSize(), eLock_Discard);
    *locked = aInstData;
    instDataBuffer->Unlock();
    _instDataBuffers.emplace_back(instDataBuffer);
    return _driverGpu->GetStorageBufferDeviceResourceManager()->
        GetIndexFromResource(instDataBuffer);
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_normals_obj_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      {
        // 0,0 is invalid it should result in an error color
        TestGpuFuncs_RayInstanceData instData;
        instData.vbIndex = 0;
        instData.ibIndex = 0;
        tU32 triInstData = AddInstData(instData);
        niCheck(AddRayTriangle(buildEncoder,instDesc,triInstData),eFalse);
        niDebugFmt(("... triInstData: %d", triInstData));
      }

      {
        tU32 firstInstIndex = eInvalidHandle;
        niLoop(i,_geoms.size()) {
          niVar& geom = _geoms[i];
          nn<iDrawOperation> dop = geom._drawOp;
          NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));
          NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
          TestGpuFuncs_RayInstanceData instData;
          instData.vbIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
              GetIndexFromResource(vaBuffer);
          instData.ibIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
              GetIndexFromResource(iaBuffer);
          niDebugFmt(("... geom[%d]: vbIndex: %d, ibIndex: %d",
                      i,instData.vbIndex,instData.ibIndex));
          geom._instIndex = AddInstData(instData);
          if (firstInstIndex == eInvalidHandle) {
            firstInstIndex = geom._instIndex;
          }
        }
        niDebugFmt(("... firstInstIndex: %d", firstInstIndex));
        niCheck(AddRayGeoms(buildEncoder,instDesc,firstInstIndex),eFalse);
      }

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,VisNormalsObj);

//----------------------------------------------------------------------------
//
// Section: VisNormalsWorld
//
//----------------------------------------------------------------------------
struct VisNormalsWorld : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);
  astl::vector<NN<iGpuBuffer>> _instDataBuffers;

  TEST_CONSTRUCTOR_BASE(VisNormalsWorld,RayTracerBase) {
  }

  tU32 AddInstData(ain<TestGpuFuncs_RayInstanceData> aInstData) {
    niLet instDataBuffer = niCheckNN(instDataBuffer, _driverGpu->CreateGpuBuffer(
      HFmt("instData_%s_%d",m_testName,_instDataBuffers.size()),
      sizeof(aInstData),
      eGpuBufferMemoryMode_Shared,
      eGpuBufferUsageFlags_Storage), eInvalidHandle);
    niVar locked = (niDeclTypeBase(aInstData)*)instDataBuffer->Lock(
      0, instDataBuffer->GetSize(), eLock_Discard);
    *locked = aInstData;
    instDataBuffer->Unlock();
    _instDataBuffers.emplace_back(instDataBuffer);
    return _driverGpu->GetStorageBufferDeviceResourceManager()->
        GetIndexFromResource(instDataBuffer);
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_normals_world_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      {
        // 0,0 is invalid it should result in an error color
        TestGpuFuncs_RayInstanceData instData;
        instData.vbIndex = 0;
        instData.ibIndex = 0;
        tU32 triInstData = AddInstData(instData);
        niCheck(AddRayTriangle(buildEncoder,instDesc,triInstData),eFalse);
        niDebugFmt(("... triInstData: %d", triInstData));
      }

      {
        tU32 firstInstIndex = eInvalidHandle;
        niLoop(i,_geoms.size()) {
          niVar& geom = _geoms[i];
          nn<iDrawOperation> dop = geom._drawOp;
          NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));
          NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
          TestGpuFuncs_RayInstanceData instData;
          instData.vbIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
              GetIndexFromResource(vaBuffer);
          instData.ibIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
              GetIndexFromResource(iaBuffer);
          niDebugFmt(("... geom[%d]: vbIndex: %d, ibIndex: %d",
                      i,instData.vbIndex,instData.ibIndex));
          geom._instIndex = AddInstData(instData);
          if (firstInstIndex == eInvalidHandle) {
            firstInstIndex = geom._instIndex;
          }
        }
        niDebugFmt(("... firstInstIndex: %d", firstInstIndex));
        niCheck(AddRayGeoms(buildEncoder,instDesc,firstInstIndex),eFalse);
      }

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,VisNormalsWorld);

//----------------------------------------------------------------------------
//
// Section: VisPosWorld
//
//----------------------------------------------------------------------------
struct VisPosWorld : public RayTracerBase {
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);
  astl::vector<NN<iGpuBuffer>> _instDataBuffers;

  TEST_CONSTRUCTOR_BASE(VisPosWorld,RayTracerBase) {
  }

  tU32 AddInstData(ain<TestGpuFuncs_RayInstanceData> aInstData) {
    niLet instDataBuffer = niCheckNN(instDataBuffer, _driverGpu->CreateGpuBuffer(
      HFmt("instData_%s_%d",m_testName,_instDataBuffers.size()),
      sizeof(aInstData),
      eGpuBufferMemoryMode_Shared,
      eGpuBufferUsageFlags_Storage), eInvalidHandle);
    niVar locked = (niDeclTypeBase(aInstData)*)instDataBuffer->Lock(
      0, instDataBuffer->GetSize(), eLock_Discard);
    *locked = aInstData;
    instDataBuffer->Unlock();
    _instDataBuffers.emplace_back(instDataBuffer);
    return _driverGpu->GetStorageBufferDeviceResourceManager()->
        GetIndexFromResource(instDataBuffer);
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_pos_world_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());

    // Create acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      niLet instDesc = niCheckNN(
        instDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);

      {
        // 0,0 is invalid it should result in an error color
        TestGpuFuncs_RayInstanceData instData;
        instData.vbIndex = 0;
        instData.ibIndex = 0;
        tU32 triInstData = AddInstData(instData);
        niCheck(AddRayTriangle(buildEncoder,instDesc,triInstData),eFalse);
        niDebugFmt(("... triInstData: %d", triInstData));
      }

      {
        tU32 firstInstIndex = eInvalidHandle;
        niLoop(i,_geoms.size()) {
          niVar& geom = _geoms[i];
          nn<iDrawOperation> dop = geom._drawOp;
          NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));
          NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
          TestGpuFuncs_RayInstanceData instData;
          instData.vbIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
              GetIndexFromResource(vaBuffer);
          instData.ibIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
              GetIndexFromResource(iaBuffer);
          niDebugFmt(("... geom[%d]: vbIndex: %d, ibIndex: %d",
                      i,instData.vbIndex,instData.ibIndex));
          geom._instIndex = AddInstData(instData);
          if (firstInstIndex == eInvalidHandle) {
            firstInstIndex = geom._instIndex;
          }
        }
        niDebugFmt(("... firstInstIndex: %d", firstInstIndex));
        niCheck(AddRayGeoms(buildEncoder,instDesc,firstInstIndex),eFalse);
      }

      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances",m_testName),instDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,VisPosWorld);

//----------------------------------------------------------------------------
//
// Section: LitCube
//
//----------------------------------------------------------------------------
struct LitCube : public RayTracerBase {
  NN<iRayInstancesDesc> _rayInstsDesc = niDeferredInit(NN<iRayInstancesDesc>);
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);
  astl::vector<NN<iGpuBuffer>> _instDataBuffers;

  TEST_CONSTRUCTOR_BASE(LitCube,RayTracerBase) {
  }

  tU32 AddInstData(ain<TestGpuFuncs_RayInstanceData> aInstData) {
    niLet instDataBuffer = niCheckNN(instDataBuffer, _driverGpu->CreateGpuBuffer(
      HFmt("instData_%s_%d",m_testName,_instDataBuffers.size()),
      sizeof(aInstData),
      eGpuBufferMemoryMode_Shared,
      eGpuBufferUsageFlags_Storage), eInvalidHandle);
    niVar locked = (niDeclTypeBase(aInstData)*)instDataBuffer->Lock(
      0, instDataBuffer->GetSize(), eLock_Discard);
    *locked = aInstData;
    instDataBuffer->Unlock();
    _instDataBuffers.emplace_back(instDataBuffer);
    return _driverGpu->GetStorageBufferDeviceResourceManager()->
        GetIndexFromResource(instDataBuffer);
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_lit_cube_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());
    CHECK(AddSceneRotatingCube());

    // Init the instance data
    niLoop(i,_geoms.size()) {
      niVar& geom = _geoms[i];
      nn<iDrawOperation> dop = geom._drawOp;
      NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));
      NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
      TestGpuFuncs_RayInstanceData instData;
      instData.vbIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
          GetIndexFromResource(vaBuffer);
      instData.ibIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
          GetIndexFromResource(iaBuffer);
      niDebugFmt(("... geom[%d]: vbIndex: %d, ibIndex: %d",
                  i,instData.vbIndex,instData.ibIndex));
      geom._instIndex = AddInstData(instData);
    }
    niDebugFmt(("... _geoms[0]._instIndex: %d", _geoms[0]._instIndex));

    // Create the acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      _rayInstsDesc = niCheckNN(
        _rayInstsDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);
      niCheck(AddRayGeoms(buildEncoder,_rayInstsDesc,_geoms[0]._instIndex),eFalse);
      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances_Frame%d",m_testName,ni::GetLang()->GetFrameNumber()),_rayInstsDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      {
        niVar& lastGeom = _geoms.back();
        lastGeom._drawOp->SetMatrix(
          MatrixRotationY(WrapRad((tF32)_animationTime * 2.0f)) *
          MatrixRotationZ(WrapRad((tF32)_animationTime * 0.5f)) *
          MatrixTranslation(MatrixGetTranslation(lastGeom._startMatrix)));
      }
      niCheck(UpdateRayGeomsTransforms(buildEncoder,_rayInstsDesc),eFalse);
      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances_Frame%d",m_testName,ni::GetLang()->GetFrameNumber()),_rayInstsDesc), eFalse);
    }

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,LitCube);

//----------------------------------------------------------------------------
//
// Section: LitTexturedCube
//
//----------------------------------------------------------------------------
struct LitTexturedCube : public RayTracerBase {
  NN<iRayInstancesDesc> _rayInstsDesc = niDeferredInit(NN<iRayInstancesDesc>);
  NN<iRayInstances> _rayInsts = niDeferredInit(NN<iRayInstances>);
  astl::vector<NN<iGpuBuffer>> _instDataBuffers;

  TEST_CONSTRUCTOR_BASE(LitTexturedCube,RayTracerBase) {
  }

  tU32 AddInstData(ain<TestGpuFuncs_RayInstanceData> aInstData) {
    niLet instDataBuffer = niCheckNN(instDataBuffer, _driverGpu->CreateGpuBuffer(
      HFmt("instData_%s_%d",m_testName,_instDataBuffers.size()),
      sizeof(aInstData),
      eGpuBufferMemoryMode_Shared,
      eGpuBufferUsageFlags_Storage), eInvalidHandle);
    niVar locked = (niDeclTypeBase(aInstData)*)instDataBuffer->Lock(
      0, instDataBuffer->GetSize(), eLock_Discard);
    *locked = aInstData;
    instDataBuffer->Unlock();
    _instDataBuffers.emplace_back(instDataBuffer);
    return _driverGpu->GetStorageBufferDeviceResourceManager()->
        GetIndexFromResource(instDataBuffer);
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(RayTracerBase::OnSinkAttached(
      _H("niUI://nish/raytracer/raytracer_lit_textured_cube_ps.gpufunc.xml")));
    CHECK(AddScenePolyGround());
    CHECK(AddSceneSevenPolyBoxes());
    CHECK(AddSceneFourPolySpheres());
    CHECK(AddSceneRotatingCube());
    CHECK(LoadTextures());

    // Init the instance data
    niLoop(i,_geoms.size()) {
      niVar& geom = _geoms[i];
      nn<iDrawOperation> dop = geom._drawOp;
      NN<iGpuBuffer> iaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetIndexArray()));
      NN<iGpuBuffer> vaBuffer = AsNN(QPtr<iGpuBuffer>(dop->GetVertexArray()));
      TestGpuFuncs_RayInstanceData instData;
      instData.vbIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
          GetIndexFromResource(vaBuffer);
      instData.ibIndex = _driverGpu->GetStorageBufferDeviceResourceManager()->
          GetIndexFromResource(iaBuffer);
      if (i == _geoms.size()-1) {
        instData.texIndex = _GetTextureIndex(_textures[_selectedTexture]);
      }
      niDebugFmt(("... geom[%d]: vbIndex: %d, ibIndex: %d",
                  i,instData.vbIndex,instData.ibIndex));
      geom._instIndex = AddInstData(instData);
    }
    niDebugFmt(("... _geoms[0]._instIndex: %d", _geoms[0]._instIndex));

    // Create the acceleration structure
    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      _rayInstsDesc = niCheckNN(
        _rayInstsDesc,
        _driverRay->CreateRayInstancesDesc(HFmt("%s_RayInstancesDesc",m_testName)),
        eFalse);
      niCheck(AddRayGeoms(buildEncoder,_rayInstsDesc,_geoms[0]._instIndex),eFalse);
      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances_Frame%d",m_testName,ni::GetLang()->GetFrameNumber()),_rayInstsDesc), eFalse);
    }

    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    RayTracerBase::OnPaint(avMousePos,apCanvas);

    QPtr<iGraphicsContextGpu> gpuContext = apCanvas->GetGraphicsContext();
    niPanicAssert(gpuContext.IsOK());

    {
      niLet buildEncoder = niCheckNN(buildEncoder,_driverRay->CreateRayBuildEncoder(),eFalse);
      {
        niVar& lastGeom = _geoms.back();
        lastGeom._drawOp->SetMatrix(
          MatrixRotationY(WrapRad((tF32)_animationTime * 2.0f)) *
          MatrixRotationZ(WrapRad((tF32)_animationTime * 0.5f)) *
          MatrixTranslation(MatrixGetTranslation(lastGeom._startMatrix)));

        niVar& lastInstData = _instDataBuffers.back();
        TestGpuFuncs_RayInstanceData* pLastInstData =
            (TestGpuFuncs_RayInstanceData*)lastInstData->Lock(
              0, sizeof(TestGpuFuncs_RayInstanceData), eLock_Normal);
        niCheck(pLastInstData != nullptr, eFalse);
        pLastInstData->texIndex = _GetTextureIndex(_textures[_selectedTexture]);
        lastInstData->Unlock();
      }
      niCheck(UpdateRayGeomsTransforms(buildEncoder,_rayInstsDesc),eFalse);
      _rayInsts = niCheckNN(_rayInsts, buildEncoder->BuildRayInstances(
        HFmt("%s_RayInstances_Frame%d",m_testName,ni::GetLang()->GetFrameNumber()),_rayInstsDesc), eFalse);
    }

    NN<iGpuCommandEncoder> gpuEncoder = AsNN(gpuContext->GetCommandEncoder());
    TestGpuFuncs_RayUniforms u;
    u.rtWidth = (tF32)apCanvas->GetGraphicsContext()->GetWidth();
    u.rtHeight = (tF32)apCanvas->GetGraphicsContext()->GetHeight();
    u.cameraInvView = MatrixInverse(_camera->GetViewMatrix());
    u.cameraInvViewProj = MatrixInverse(_camera->GetViewMatrix() * _camera->GetProjectionMatrix());
    u.cameraFarClipPlane = _camera->GetFarClipPlane();
    gpuEncoder->StreamUniformBuffer((tPtr)&u,sizeof(u),0);

    NN<iRayCommandEncoder> rayEncoder = AsNN(QPtr<iRayCommandEncoder>(gpuEncoder));
    rayEncoder->SetRayInstances(_rayInsts);

    DisplayTexture(*_display,gpuEncoder,nullptr);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FRayTracer,LitTexturedCube);

}
#endif
