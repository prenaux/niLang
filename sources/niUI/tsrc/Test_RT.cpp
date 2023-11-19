#include "stdafx.h"
#include <niLang/Math/MathRect.h>

using namespace ni;

struct FRT {
};

struct RTBase : public ni::cWidgetSinkImpl<> {
  struct TopLeftCanvas : public ni::cWidgetSinkImpl<> {
    RTBase* _parent;
    TopLeftCanvas(RTBase* aParent) : _parent(aParent) {}
    tBool __stdcall OnSinkAttached() niImpl {
      Ptr<iImage> ptrImage = mpWidget->GetGraphics()->CreateImage(512,512);
      niAssert(ptrImage.IsOK());
      Ptr<iGraphicsContext> ptrGC = mpWidget->GetGraphics()->CreateImageGraphicsContext(ptrImage);
      niAssert(ptrGC.IsOK());
      Ptr<iCanvas> ptrCanvas = mpWidget->GetGraphics()->CreateCanvas(ptrGC,NULL);
      niAssert(ptrCanvas.IsOK());
      mpWidget->SetCanvas(ptrCanvas);
      return eTrue;
    }
    tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
      _parent->_topLeftCanvas = apCanvas;
      _parent->_DrawTopLeft(apCanvas);
      if (_parent->mbRedrawEveryFrame) {
        mpWidget->Redraw(); // we want to redraw every frame...
      }
      return eFalse;
    }
  };

  struct BotLeftCanvas : public ni::cWidgetSinkImpl<> {
    RTBase* _parent;
    BotLeftCanvas(RTBase* aParent) : _parent(aParent) {}
    tBool __stdcall OnSinkAttached() niImpl {
      Ptr<iImage> ptrImage = mpWidget->GetGraphics()->CreateImage(512,512);
      niAssert(ptrImage.IsOK());
      Ptr<iGraphicsContext> ptrGC = mpWidget->GetGraphics()->CreateImageGraphicsContext(ptrImage);
      niAssert(ptrGC.IsOK());
      Ptr<iCanvas> ptrCanvas = mpWidget->GetGraphics()->CreateCanvas(ptrGC,NULL);
      niAssert(ptrCanvas.IsOK());
      mpWidget->SetCanvas(ptrCanvas);
      return eTrue;
    }
    tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
      _parent->_botLeftCanvas = apCanvas;
      _parent->_DrawBotLeft(apCanvas);
      if (_parent->mbRedrawEveryFrame) {
        mpWidget->Redraw(); // we want to redraw every frame...
      }
      return eFalse;
    }
  };

  struct RightCanvas : public ni::cWidgetSinkImpl<> {
    RTBase* _parent;
    RightCanvas(RTBase* aParent) : _parent(aParent) {}
    tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
      _parent->_DrawRight(apCanvas);
      return eFalse;
    }
  };

  TEST_CONSTRUCTOR(RTBase) {
    niDebugFmt(("RTBase::RTBase"));
    mbAnimated = eFalse;
    mbRedrawEveryFrame = eTrue;
  }
  ~RTBase() {
    niDebugFmt(("RTBase::~RTBase"));
  }

  tBool mbRedrawEveryFrame;

  WeakPtr<iCanvas> _topLeftCanvas;
  WeakPtr<iCanvas> _botLeftCanvas;

  tBool __stdcall OnSinkAttached() niImpl {
    Ptr<iWidget> right = mpWidget->GetUIContext()->CreateWidgetRaw("MyCanvas",mpWidget);
    right->AddSink(niNew RightCanvas(this));
    right->SetRelativeSize(Vec2f(0.5f,1));
    right->SetDockStyle(eWidgetDockStyle_DockRight);
    right->SetIgnoreInput(eTrue);

    Ptr<iWidget> topLeft = mpWidget->GetUIContext()->CreateWidgetRaw("MyCanvas",mpWidget);
    topLeft->AddSink(niNew TopLeftCanvas(this));
    topLeft->SetRelativeSize(Vec2f(0.5f,0.5f));
    topLeft->SetDockStyle(eWidgetDockStyle_DockTop);
    topLeft->SetIgnoreInput(eTrue);

    Ptr<iWidget> botLeft = mpWidget->GetUIContext()->CreateWidgetRaw("MyCanvas",mpWidget);
    botLeft->AddSink(niNew BotLeftCanvas(this));
    botLeft->SetRelativeSize(Vec2f(0.5f,0.5f));
    botLeft->SetDockStyle(eWidgetDockStyle_DockTop);
    botLeft->SetIgnoreInput(eTrue);
    return eTrue;
  }

  tBool mbAnimated;
  tF64 mfAnimationTime;
  void _ToggleAnimation() {
    mbAnimated = !mbAnimated;
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

  Ptr<iOverlay> _topRightOverlay;
  Ptr<iOverlay> _botRightOverlay;

  static tBool DrawFPSRects(iCanvas* apCanvas, iFont* apFont) {
    apCanvas->SetMatrix(sMatrixf::Identity());
    apCanvas->BlitText(
      apFont, Rectf(0,0,0,0), eFontFormatFlags_Border,
      niFmt("Ren: %s, FPS: %.2f - %.2f",
            apCanvas->GetGraphicsContext()->GetDriver()->GetName(),
            ni::GetLang()->GetAverageFrameRate(),
            ni::GetLang()->GetFrameRate()));

    sMatrixf t,u;
    const sRectf rect = ni::sRectf(apCanvas->GetViewport().GetWidth() - 30 - 15,
                                   20,
                                   30.0f, 30.0f);
    apCanvas->SetMatrix(
      MatrixRotationPivot(
        u, ni::MatrixRotationZ<tF32>(t,ni::WrapRad(tF32(ni::GetLang()->GetFrameNumber())/100.0f)), Vec3f(rect.GetCenter().x,rect.GetCenter().y,0)));
    apCanvas->BlitFill(rect, 0xFFFF00FF);
    const sRectf rect2 = ni::sRectf(apCanvas->GetViewport().GetWidth() - 30 - 30 - 25,
                                    20,
                                    30.0f, 30.0f);
    apCanvas->SetMatrix(
      MatrixRotationPivot(
        u, ni::MatrixRotationZ<tF32>(t,(tF32)ni::WrapRad(ni::GetLang()->TimerInSeconds())), Vec3f(rect2.GetCenter().x,rect2.GetCenter().y,0)));
    apCanvas->BlitFill(rect2, 0xFF00FFFF);
    apCanvas->SetMatrix(sMatrixf::Identity());
    return eTrue;
  }

  void _DrawRight(iCanvas* apCanvas) {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF336699);

    QPtr<iCanvas> topLeftCanvas = _topLeftCanvas;
    if (topLeftCanvas.IsOK()) {
      Ptr<iTexture> leftTexture = topLeftCanvas->GetGraphicsContext()->GetRenderTarget(0);
      CHECK_RETURN_IF_FAILED(leftTexture.IsOK());
      if (!_topRightOverlay.IsOK()) {
        _topRightOverlay = mpWidget->GetGraphics()->CreateOverlayTexture(leftTexture);
      }
      else {
        _topRightOverlay->GetMaterial()->SetChannelTexture(
            eMaterialChannel_Base, leftTexture);
      }
    }

    QPtr<iCanvas> botLeftCanvas = _botLeftCanvas;
    if (botLeftCanvas.IsOK()) {
      Ptr<iTexture> leftTexture = botLeftCanvas->GetGraphicsContext()->GetRenderTarget(0);
      CHECK_RETURN_IF_FAILED(leftTexture.IsOK());
      if (!_botRightOverlay.IsOK()) {
        _botRightOverlay = mpWidget->GetGraphics()->CreateOverlayTexture(leftTexture);
      }
      else {
        _botRightOverlay->GetMaterial()->SetChannelTexture(
            eMaterialChannel_Base, leftTexture);
      }
    }

    sMatrixf m,t;
    if (mbAnimated) {
      mfAnimationTime += ni::GetLang()->GetFrameTime();
    }

    if (_topRightOverlay.IsOK()) {
      MatrixIdentity(m);
      const sRectf rect(100,50,200,200);
      const sVec3f center = { rect.GetCenter().x, rect.GetCenter().y, 0 };
      m *= MatrixTranslation(t,-center);
      m *= MatrixRotationZ<tF32>(
          t,ni::FMod<tF32>((tF32)mfAnimationTime/3.0f,niPif*2.0f));
      m *= MatrixTranslation(t,center);
      apCanvas->SetMatrix(m);
      apCanvas->BlitOverlay(rect,_topRightOverlay);
    }
    if (_botRightOverlay.IsOK()) {
      MatrixIdentity(m);
      const sRectf rect(100,300,200,200);
      const sVec3f center = { rect.GetCenter().x, rect.GetCenter().y, 0 };
      m *= MatrixTranslation(t,-center);
      m *= MatrixRotationZ<tF32>(
          t,ni::FMod<tF32>((tF32)mfAnimationTime/3.0f,niPif*2.0f));
      m *= MatrixTranslation(t,center);
      apCanvas->SetMatrix(m);
      apCanvas->BlitOverlay(rect,_botRightOverlay);
    }

    DrawFPSRects(apCanvas,mpWidget->GetFont());
  }

  virtual void _DrawTopLeft(iCanvas* apCanvas) = 0;
  virtual void _DrawBotLeft(iCanvas* apCanvas) = 0;
};

struct RT1 : public RTBase {
  TEST_CONSTRUCTOR_BASE(RT1,RTBase) {
  }

  void _DrawTopLeft(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);

    const tF32 cellSize = ni::Floor(
        (ni::Min(apCanvas->GetViewport().GetWidth(),apCanvas->GetViewport().GetHeight())-20) / 10.f);
    niLoop(i,10) {
      niLoop(j,10) {
        const sRectf rect = ni::sRectf((tF32)i * cellSize + 10,
                                       (tF32)j * cellSize + 10,
                                       cellSize, cellSize);
        if (i == 0 && j == 0) {
          apCanvas->BlitFill(rect, 0xFF990000);
        }
        else if (i == 9 && j == 0) {
          apCanvas->BlitFill(rect, 0xFF009900);
        }
        else if ((i+j)&1) {
          apCanvas->BlitFill(rect, 0);
        }
        else {
          apCanvas->BlitFillAlpha(rect, 0x99FFFFFF);
        }
        apCanvas->BlitText(
            mpWidget->GetFont(), rect, 0, niFmt("A%d",(i + j*10)));
      }
    }
    apCanvas->Flush();
    apCanvas->GetGraphicsContext()->Display(eGraphicsDisplayFlags_Skip,sRecti::Null());
  }

  void _DrawBotLeft(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF669933);

    const tF32 cellSize = ni::Floor(
        (ni::Min(apCanvas->GetViewport().GetWidth(),apCanvas->GetViewport().GetHeight())-20) / 10.f);
    niLoop(i,10) {
      niLoop(j,10) {
        const sRectf rect = ni::sRectf((tF32)i * cellSize + 10,
                                       (tF32)j * cellSize + 10,
                                       cellSize, cellSize);
        if (i == 0 && j == 0) {
          apCanvas->BlitFill(rect, 0xFF990000);
        }
        else if (i == 9 && j == 0) {
          apCanvas->BlitFill(rect, 0xFF009900);
        }
        else if ((i+j)&1) {
          apCanvas->BlitFill(rect, 0);
        }
        else {
          apCanvas->BlitFillAlpha(rect, 0x99FFFFFF);
        }
        apCanvas->BlitText(
            mpWidget->GetFont(), rect, 0, niFmt("B%d",(i + j*10)));
      }
    }
    apCanvas->Flush();
    apCanvas->GetGraphicsContext()->Display(eGraphicsDisplayFlags_Skip,sRecti::Null());
  }
};
TEST_FIXTURE_WIDGET(FRT,RT1);

struct RT2 : public RTBase {
  TEST_CONSTRUCTOR_BASE(RT2,RTBase) {
  }

  void _DrawTopLeft(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF996633);

    apCanvas->BlitText(
        mpWidget->GetFont(),
        apCanvas->GetViewport().ToFloat(),
        eFontFormatFlags_Right,
        "No Depth Buffer");

    apCanvas->ResetStates();
    apCanvas->GetGraphicsContext()->GetFixedStates()->SetLookAtMatrices(
        eTrue, Vec3f(0,40,-50), Vec3f(0,10,0), Vec3f(0,1,0),
        niPif / 2,
		ni::FDiv((tF32)apCanvas->GetGraphicsContext()->GetWidth(),
				 (tF32)apCanvas->GetGraphicsContext()->GetHeight()),
		0.1f, 1e6f);
    apCanvas->SetLineSize(3.0f);
    apCanvas->WireframeAABB(Vec3f(-10,-10,-10), Vec3f(10,30,10));
    apCanvas->SetColorA(0xFF000099);
    apCanvas->WireframeAABB(Vec3f(-10,-25,-10), Vec3f(10,-15,10));
    apCanvas->SetColorA(0xFF990000);
    apCanvas->WireframeAABB(Vec3f(-10,35,-10), Vec3f(10,45,10));

    apCanvas->Flush();
    apCanvas->GetGraphicsContext()->Display(eGraphicsDisplayFlags_Skip,sRecti::Null());
  }

  Ptr<iMaterial> mptrDSMaterial;

  void _DrawBotLeft(iCanvas* apCanvas) niImpl {
    apCanvas->BlitText(
        mpWidget->GetFont(),
        apCanvas->GetViewport().ToFloat(),
        eFontFormatFlags_Right,
        "With Depth Buffer");

    Ptr<iGraphicsContext> gc = apCanvas->GetGraphicsContext();
    if (!mptrDSMaterial.IsOK()) {
      mptrDSMaterial = gc->GetGraphics()->CreateMaterial();
      mptrDSMaterial->SetRasterizerStates(eCompiledStates_RS_NoCullingFilledScissor);
      mptrDSMaterial->SetDepthStencilStates(eCompiledStates_DS_DepthTestAndWrite);
      mptrDSMaterial->SetFlags(
          mptrDSMaterial->GetFlags()|
          eMaterialFlags_NoLighting|
          eMaterialFlags_DoubleSided|
          eMaterialFlags_Vertex);
      mptrDSMaterial->SetChannelSamplerStates(
          eMaterialChannel_Base,eCompiledStates_SS_PointClamp);
    }

    Ptr<iTexture> ds = apCanvas->GetGraphicsContext()->GetDepthStencil();
    CHECK_RETURN_IF_FAILED(ds.IsOK());

    gc->ClearBuffers(eClearBuffersFlags_ColorDepthStencil,0xFF669933,1.0f,0);

    apCanvas->ResetStates();
    apCanvas->SetMaterial(mptrDSMaterial);
    apCanvas->GetGraphicsContext()->GetFixedStates()->SetLookAtMatrices(
        eTrue, Vec3f(0,40,-50), Vec3f(0,10,0), Vec3f(0,1,0),
        niPif / 2,
		ni::FDiv((tF32)apCanvas->GetGraphicsContext()->GetWidth(),
			     (tF32)apCanvas->GetGraphicsContext()->GetHeight()),
		0.1f, 1e6f);
    apCanvas->SetLineSize(3.0f);
    apCanvas->WireframeAABB(Vec3f(-10,-10,-10), Vec3f(10,30,10));
    apCanvas->SetColorA(0xFF000099);
    apCanvas->WireframeAABB(Vec3f(-10,-25,-10), Vec3f(10,-15,10));
    apCanvas->SetColorA(0xFF990000);
    apCanvas->WireframeAABB(Vec3f(-10,35,-10), Vec3f(10,45,10));

    apCanvas->Flush();
    apCanvas->GetGraphicsContext()->Display(eGraphicsDisplayFlags_Skip,sRecti::Null());
  }
};
TEST_FIXTURE_WIDGET(FRT,RT2);
