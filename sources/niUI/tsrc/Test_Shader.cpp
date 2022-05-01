#include "stdafx.h"
#include <niLang/Math/MathRect.h>

using namespace ni;

static const tF32 kfRunSpeed = 256.0f;
static const tF32 kfNormalSpeed = 64.0f;

struct FShader {
};

struct ShaderBase : public ni::cWidgetSinkImpl<> {
  tBool kbTextures;
  tBool mbAnimated;

  TEST_CONSTRUCTOR(ShaderBase) {
    niDebugFmt(("ShaderBase::ShaderBase"));

    kbTextures = (!ni::GetLang()->HasProperty("tests.Textures") ||
                  ni::GetLang()->GetProperty("tests.Textures").Long());

    mbAnimated = (!ni::GetLang()->HasProperty("tests.Animated") ||
                  ni::GetLang()->GetProperty("tests.Animated").Long());

    mbCameraInput = eTrue;
    mbMouseLook = eFalse;
  }
  ~ShaderBase() {
    niDebugFmt(("ShaderBase::~ShaderBase"));
  }

  Ptr<iGraphics> mptrGraphics;

  Ptr<iCamera> mptrCamera;
  tBool        mbCameraInput;
  tBool        mbMouseLook;
  sVec2f    mPrevMousePos;

  tBool        mbHasInput;
  sVec3f    mMove;
  sVec2f    mLook;

  sMatrixf                   mmtxPrevView;
  sMatrixf                   mmtxPrevProj;

  Ptr<iTexture>        _earthBase;
  Ptr<iTexture>        _earthBump;
  Ptr<iTexture>        _earthSpec;
  Ptr<iTexture>        _earthClouds;
  Ptr<iTexture>        _rustSteel;
  Ptr<iTexture>        _tree;

  Ptr<iDrawOperationSet> _drawOpSet;

  tBool __stdcall OnSinkAttached() niOverride {
    CHECK(_InitializeCamera());

    _drawOpSet = mpWidget->GetGraphics()->CreateDrawOperationSet();

    return eTrue;
  }

  tF32 mfAnimationTime;
  void _ToggleAnimation() {
    mbAnimated = !mbAnimated;
  }

  tBool __stdcall _InitializeCamera() {
    mptrCamera = mpWidget->GetGraphics()->CreateCamera();
    mpWidget->SetStyle(mpWidget->GetStyle()|eWidgetStyle_HoldFocus);
    mpWidget->SetFocus();
    mbMouseLook = eFalse;
    return eTrue;
  }

  void _UpdateCamera() {
    if (!mbHasInput && mMove == sVec3f::Zero())
      return;

    const tF32 dt = ni::GetLang()->GetFrameTime();
    const tF32 speed = ((mpWidget->GetUIContext()->GetInputModifiers()&eUIInputModifier_Shift) ?
                        kfRunSpeed : kfNormalSpeed);
    mptrCamera->MoveForward(mMove.z * speed * dt);
    mptrCamera->MoveUp(mMove.y * speed * dt);
    mptrCamera->MoveSidewards(mMove.x * speed * dt);
    mLook = Vec2f(0,0);
    mbHasInput = eFalse;
  }

  tBool __stdcall OnWheel(tF32 afWheel, const sVec2f& avAbsMousePos) niOverride {
    if (mbMouseLook && mbCameraInput) {
      mptrCamera->MoveForward(
          afWheel*0.2f*
          ((mpWidget->GetUIContext()->GetInputModifiers()&eUIInputModifier_Shift) ?
           kfRunSpeed : kfNormalSpeed));
    }
    return eFalse;
  }

  tBool __stdcall OnLeftClickDown(const sVec2f& avMP, const sVec2f& avNCMP) {
    if (mbCameraInput) {
      mbMouseLook = eTrue;
      mPrevMousePos = avNCMP + mpWidget->GetAbsolutePosition();
      mpWidget->SetCapture(eTrue);
    }
    return eFalse;
  }
  tBool __stdcall OnNCLeftClickDown(const sVec2f& avMP, const sVec2f& avNCMP) {
    return OnLeftClickDown(avMP,avNCMP);
  }
  tBool __stdcall OnLeftClickUp(const sVec2f& avMP, const sVec2f& avNCMP) {
    if (mbCameraInput) {
      mbMouseLook = eFalse;
      mpWidget->SetCapture(eFalse);
    }
    return eFalse;
  }
  tBool __stdcall OnNCLeftClickUp(const sVec2f& avMP, const sVec2f& avNCMP) {
    return OnLeftClickUp(avMP,avNCMP);
  }

  tBool __stdcall OnMouseMove(const sVec2f& avMP, const sVec2f& avNCMP) {
    if (mbMouseLook) {
      const sVec2f newPos = (avNCMP + mpWidget->GetAbsolutePosition());
      sVec2f deltaMove = newPos - mPrevMousePos;
      mPrevMousePos = newPos;
      mptrCamera->AddPitch(-deltaMove.y / 300.0f);
      mptrCamera->AddYaw(-deltaMove.x / 300.0f);
    }
    return eFalse;
  }
  tBool __stdcall OnNCMouseMove(const sVec2f& avMP, const sVec2f& avNCMP) {
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
        mMove.z = 1.0;
        mbHasInput = true;
        break;
      }
        /// Backward ///
      case eKey_Down:
      case eKey_S: {
        mMove.z = -1.0;
        mbHasInput = true;
        break;
      }
        /// Strafe Left ///
      case eKey_Left:
      case eKey_A: {
        mMove.x = -1.0;
        mbHasInput = true;
        break;
      }
        /// Strafe Right ///
      case eKey_Right:
      case eKey_D: {
        mMove.x = 1.0;
        mbHasInput = true;
        break;
      }
        /// Move Up ///
      case eKey_PgUp:
      case eKey_R: {
        mMove.y = 1.0;
        mbHasInput = true;
        break;
      }
        /// Move Down ///
      case eKey_PgDn:
      case eKey_F: {
        mMove.y = -1.0;
        mbHasInput = true;
        break;
      }
    }
    return eFalse;
  }

  tBool __stdcall OnKeyUp(eKey aKey, tU32 aKeyMod) niOverride {
    switch (aKey) {
      case eKey_Up:
      case eKey_Down:
      case eKey_W:
      case eKey_S:
        mMove.z = 0.0;
        break;
      case eKey_Left:
      case eKey_Right:
      case eKey_A:
      case eKey_D:
        mMove.x = 0.0;
        break;
      case eKey_PgUp:
      case eKey_PgDn:
      case eKey_R:
      case eKey_F:
        mMove.y = 0.0;
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

    if (mbAnimated) {
      mfAnimationTime += ni::GetLang()->GetFrameTime();
    }

    const sVec4f vTime = Vec4f(
      ni::GetLang()->GetTotalFrameTime(),
      ni::GetLang()->GetTotalFrameTime(),
      ni::Sin(ni::GetLang()->GetTotalFrameTime()),
      ni::GetLang()->GetFrameTime()
    );

    _drawOpSet->Draw(gc, mptrCamera->GetFrustum());

    ptrFS->SetCameraViewMatrix(wasViewMatrix);
    ptrFS->SetCameraProjectionMatrix(wasProjectionMatrix);

    cString str = niFmt("Driver: %s, POS: %s, TARGET: %s, UP: %s, Animated: %d\n",
                        mpWidget->GetGraphics()->GetDriver()->GetName(),
                        cString(mptrCamera->GetPosition()).Chars(),
                        cString(mptrCamera->GetTarget()).Chars(),
                        cString(mptrCamera->GetTargetUp()).Chars(),
                        mbAnimated);
    apCanvas->BlitText(
        mpWidget->GetFont(),
        sRectf(5,5),
        eFontFormatFlags_Border,
        str.Chars());

    return eFalse;
  }

  virtual tBool __stdcall LoadEarthTextures() {
    Ptr<iFile> fp;
    fp = mpWidget->GetGraphics()->OpenBitmapFile(_A("test/tex/earth_d.jpg"));
    _earthBase = mpWidget->GetGraphics()->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),mpWidget->GetGraphics()->LoadBitmap(fp),eTextureFlags_Default);
    if (!_earthBase.IsOK()) return eFalse;
    fp = mpWidget->GetGraphics()->OpenBitmapFile(_A("test/tex/earth_b.jpg"));
    _earthBump = mpWidget->GetGraphics()->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),mpWidget->GetGraphics()->LoadBitmap(fp),eTextureFlags_Default);
    if (!_earthBump.IsOK()) return eFalse;
    fp = mpWidget->GetGraphics()->OpenBitmapFile(_A("test/tex/earth_s.jpg"));
    _earthSpec = mpWidget->GetGraphics()->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),mpWidget->GetGraphics()->LoadBitmap(fp),eTextureFlags_Default);
    if (!_earthSpec.IsOK()) return eFalse;
    fp = mpWidget->GetGraphics()->OpenBitmapFile(_A("test/tex/earth_clouds_d.jpg"));
    _earthClouds = mpWidget->GetGraphics()->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),mpWidget->GetGraphics()->LoadBitmap(fp),eTextureFlags_Default);
    if (!_earthClouds.IsOK()) return eFalse;
    fp = mpWidget->GetGraphics()->OpenBitmapFile(_A("test/tex/tree.dds"));
    _tree = mpWidget->GetGraphics()->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),mpWidget->GetGraphics()->LoadBitmap(fp),eTextureFlags_Default);
    if (!_tree.IsOK()) return eFalse;
    fp = mpWidget->GetGraphics()->OpenBitmapFile(_A("test/tex/rust_steel.jpg"));
    _rustSteel = mpWidget->GetGraphics()->CreateTextureFromBitmap(
        _H(fp->GetSourcePath()),mpWidget->GetGraphics()->LoadBitmap(fp),eTextureFlags_Default);
    if (!_rustSteel.IsOK()) return eFalse;
    return eTrue;
  }

  Ptr<iMaterial> CreateMaterial(iTexture* apTex) {
    Ptr<iMaterial> mat = mpWidget->GetGraphics()->CreateMaterial();
    mat->SetDepthStencilStates(eCompiledStates_DS_DepthTestAndWrite);
    if (kbTextures && apTex) {
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
      mat->SetFlags(mat->GetFlags()|eMaterialFlags_Translucent);
      mat->SetFlags(mat->GetFlags()|eMaterialFlags_Transparent);
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

  tBool CreateCube(const sVec3f& avCenter, iTexture* apTex, tBool abCW = eTrue, tBool abAlpha = eFalse, tF32 afSize = 10.0f) {

    Ptr<iGeometry> g = mpWidget->GetGraphics()->CreateGeometryPolygonalCube(
        eGeometryCreateFlags_Static,eFVF_Position|eFVF_Tex1|eFVF_Normal,
        sVec3f::Zero(),afSize*2.0f,abCW,0xFFFFFFFF,sMatrixf::Identity());

    Ptr<iMaterial> mat = CreateMaterial(apTex);
    if (kbTextures && apTex) {
      if (abAlpha && apTex->GetPixelFormat()->GetNumABits()) {
        mat->SetFlags(mat->GetFlags()|eMaterialFlags_Transparent);
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

  tBool InitScene(tBool abBoxed = eFalse) {

    const tBool bMinimalScene = !!ni::GetLang()->GetProperty("tests.minimal_scene").Long();

    if (!LoadEarthTextures()) return eFalse;

    if (!CreateCube(Vec3(  0.0f,-130.0f,100.0f),_rustSteel,eTrue,eFalse,100.0f))
      return eFalse;

    if (!bMinimalScene) {
      if (!CreateCube(Vec3(-25.0f,-20.0f,100.0f),_rustSteel)) return eFalse;
      if (!CreateCube(Vec3(-30.0f,-10.0f,110.0f),_earthClouds)) return eFalse;
      if (!CreateCube(Vec3(-30.0f, 10.0f,105.0f),_rustSteel)) return eFalse;
      if (!CreateCube(Vec3( 25.0f,-20.0f,100.0f),_earthBase)) return eFalse;
      if (!CreateCube(Vec3( 30.0f,-10.0f,110.0f),_earthBase)) return eFalse;
    }

    if (!CreateCube(Vec3( 30.0f, 10.0f,105.0f),_earthBase)) return eFalse;
    if (!CreateCube(Vec3(  0.0f,-20.0f,75.0f),_tree,eTrue,eTrue)) return eFalse;

    if (abBoxed) {
      if (!CreateCube(Vec3( 0.0f,-232.0f,0.0f),_rustSteel,eFalse,eFalse,500)) return eFalse;
      if (!CreateSphere(Vec3(-200.0f,-60.0f,-200.0f),_earthBase,35.0f,eTrue)) return eFalse;
      if (!CreateSphere(Vec3( 200.0f,-60.0f,-200.0f),_earthBase,35.0f,eTrue)) return eFalse;
      if (!CreateSphere(Vec3( 200.0f,-60.0f, 200.0f),_earthBase,35.0f,eTrue)) return eFalse;
      if (!CreateSphere(Vec3(-200.0f,-60.0f, 200.0f),_earthBase,35.0f,eTrue)) return eFalse;
    }

    {
      const tU32 sphereCount = bMinimalScene ? 1 : 24;
      const tU32 stacks = bMinimalScene ? 1 : 2;
      const tF32 sphereDistance = 65.0f;
      tF32 sphereRad = 0.0f;

      niLoop(j,stacks) {
        niLoop(i,sphereCount) {
          sphereRad += (tF32)2.0f*niPif/(tF32)sphereCount;
          if (!bMinimalScene && ((i+j)&1) == 0)
            continue;
          sVec3f p;
          p.y = -20.0f + (tF32)j*20.0f;
          p.x = ni::Sin(sphereRad)*sphereDistance;
          p.z = ni::Cos(sphereRad)*sphereDistance;
          if (!CreateSphere(p+Vec3(0.0f,0.0f,100.0f),_earthBase,8.0f)) return eFalse;
        }
      }
    }

    return eTrue;
  }
};

struct Fixed : public ShaderBase {
  TEST_CONSTRUCTOR_BASE(Fixed,ShaderBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    CHECK(ShaderBase::OnSinkAttached());
    CHECK(InitScene());
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    ShaderBase::OnPaint(avMousePos,apCanvas);
    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FShader,Fixed);
