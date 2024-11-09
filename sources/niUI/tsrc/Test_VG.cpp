#include "stdafx.h"

#include <niLang/Math/MathRect.h>
#include "../src/API/niUI/IGraphics.h"
#define agg_real ni::tF32

using namespace ni;

struct FVG {
};

struct VGBase : public ni::cWidgetSinkImpl<> {
  struct MainCanvas : public ni::cWidgetSinkImpl<> {
    VGBase* _parent;
    MainCanvas(VGBase* aParent) : _parent(aParent) {}
    tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
      _parent->_DrawMain(apCanvas);
      return eFalse;
    }
  };

  Ptr<iVGPath> mptrPath;

  TEST_CONSTRUCTOR(VGBase) {
    niDebugFmt(("VGBase::VGBase"));
    mbWireframe = eFalse;
    mbAnimated = eFalse;
    mbRedrawEveryFrame = eTrue;
  }
  ~VGBase() {
    niDebugFmt(("VGBase::~VGBase"));
  }

  tBool mbRedrawEveryFrame;

  WeakPtr<iCanvas> _topLeftCanvas;
  WeakPtr<iCanvas> _botLeftCanvas;

  tBool __stdcall OnSinkAttached() niOverride {
    mptrPath = mpWidget->GetGraphics()->CreateVGPath();

    Ptr<iWidget> main = mpWidget->GetUIContext()->CreateWidgetRaw("MyCanvas",mpWidget);
    main->AddSink(niNew MainCanvas(this));
    main->SetDockStyle(eWidgetDockStyle_DockFill);
    main->SetIgnoreInput(eTrue);
    return eTrue;
  }

  tBool mbWireframe;
  void _ToggleWireframe() {
    mbWireframe = !mbWireframe;
  }

  tBool mbAnimated;
  tF32 mfAnimationTime;
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
    else if (aKey == eKey_W) {
      _ToggleWireframe();
    }
    return eFalse;
  }

  virtual void _DrawMain(iCanvas* apCanvas) = 0;
};

struct VGLine : public VGBase {
  TEST_CONSTRUCTOR_BASE(VGLine,VGBase) {
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), ~0);

    // Set wireframe state, use W to toggle
    apCanvas->GetVGMaterial()->SetRasterizerStates(
      mbWireframe ?
      eCompiledStates_RS_Wireframe :
      eCompiledStates_RS_NoCullingFilled);

    iVGStyle* style = apCanvas->GetVGStyle();
    // mptrStyle->SetFill(eFalse);
    style->SetFillColor(Vec3f(0,1,0));
    style->SetStrokeColor(Vec3f(1,0,0));
    style->SetStrokeWidth(15.0f);
    style->SetLineJoin(eVGLineJoin_Round);
    style->SetInnerJoin(eVGInnerJoin_Jag);
    mptrPath->Clear();
    mptrPath->MoveTo(50,50);
    mptrPath->LineTo(150,150);
    mptrPath->LineTo(250,200);
    mptrPath->LineTo(250,500);

    apCanvas->DrawPath(mptrPath);
  }
};
TEST_FIXTURE_WIDGET(FVG,VGLine);

struct PieChart : public VGBase {
  TEST_CONSTRUCTOR_BASE(PieChart,VGBase) {
  }

  sVec2f getCoordinatesForPercent(tF32 percent) {
    tF32 x = ni::Cos(2.0f * niPif * percent);
    tF32 y = ni::Sin(2.0f * niPif * percent);
    return Vec2f(x,y);
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), ~0);

    // Set wireframe state, use W to toggle
    apCanvas->GetVGMaterial()->SetRasterizerStates(
      mbWireframe ?
      eCompiledStates_RS_Wireframe :
      eCompiledStates_RS_NoCullingFilled);

    iVGStyle* style = apCanvas->GetVGStyle();
    // style->SetFill(eFalse);
    style->SetStrokeColor(Vec3f(1,0,0));
    // style->SetStrokeWidth(15.0f);
    style->SetFillColor(Vec3f(0,1,0));
    style->SetLineJoin(eVGLineJoin_Round);
    style->SetInnerJoin(eVGInnerJoin_Jag);
    mptrPath->Clear();

    const sVec2f origin = Vec2f(55,55);
    const tF32 size = 50;
    {
      sVec2f start = getCoordinatesForPercent(0) * size + origin;
      mptrPath->MoveTo(start.x,start.y);
    }

    tF32 currentPercent = 0;
    {
      currentPercent += 0.2f;
      sVec2f end = getCoordinatesForPercent(currentPercent) * size + origin;
      mptrPath->ArcTo(size,size, 0, 0, 1, end.x, end.y);
      mptrPath->LineTo(origin.x, origin.y);
      mptrPath->MoveTo(end.x,end.y);
    }

    {
      currentPercent += 0.3f;
      sVec2f end = getCoordinatesForPercent(currentPercent) * size + origin;
      mptrPath->ArcTo(size,size, 0, 0, 1, end.x, end.y);
      mptrPath->LineTo(origin.x, origin.y);
      mptrPath->MoveTo(end.x,end.y);
    }

    {
      currentPercent += 0.05f;
      sVec2f end = getCoordinatesForPercent(currentPercent) * size + origin;
      mptrPath->ArcTo(size,size, 0, 0, 1, end.x, end.y);
      mptrPath->LineTo(origin.x, origin.y);
      mptrPath->MoveTo(end.x,end.y);
    }

    {
      currentPercent += 0.45f;
      sVec2f end = getCoordinatesForPercent(currentPercent) * size + origin;
      mptrPath->ArcTo(size,size, 0, 0, 1, end.x, end.y);
      mptrPath->LineTo(origin.x, origin.y);
      mptrPath->MoveTo(end.x,end.y);
    }

    mptrPath->Scale(Vec2f(5,5));
    apCanvas->DrawPath(mptrPath);
  }
};
TEST_FIXTURE_WIDGET(FVG,PieChart);

struct HeartSVGPath : public VGBase {
  TEST_CONSTRUCTOR_BASE(HeartSVGPath,VGBase) {
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), ~0);

    // Set wireframe state, use W to toggle
    apCanvas->GetVGMaterial()->SetRasterizerStates(
      mbWireframe ?
      eCompiledStates_RS_Wireframe :
      eCompiledStates_RS_NoCullingFilled);

    iVGStyle* style = apCanvas->GetVGStyle();
    style->SetFill(eFalse);
    style->SetStrokeColor(Vec3f(1,0,0));
    style->SetStrokeWidth(5.0f);
    style->SetFillColor(Vec3f(0,1,0));
    style->SetLineJoin(eVGLineJoin_Round);
    style->SetInnerJoin(eVGInnerJoin_Jag);

    mptrPath->Clear();
    mptrPath->AddSVGPath(
      " M 10,30"
      " A 20,20 0,0,1 50,30"
      " A 20,20 0,0,1 90,30"
      " Q 90,60 50,90"
      " Q 10,60 10,30 z");
    mptrPath->Scale(Vec2f(5,5));

    apCanvas->DrawPath(mptrPath);
  }
};
TEST_FIXTURE_WIDGET(FVG,HeartSVGPath);

struct Icons : public VGBase {
  TEST_CONSTRUCTOR_BASE(Icons,VGBase) {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    VGBase::OnSinkAttached();
    mpWidget->GetGraphics()->RegisterSystemFonts();
    Ptr<iFont> fontIcons = mpWidget->GetGraphics()->LoadFont(_H("fas"));
    CHECK(fontIcons.IsOK());
    Ptr<tU32CMap> glyphs = fontIcons->EnumGlyphs();
    CHECK(glyphs->size() > 1000);
#if 0
    for (auto& glyph : *glyphs) {
      niDebugFmt(("... icon: index: %d, codepoint: 0x%x, name: %s",
                  glyph.first, glyph.second,
                  fontIcons->GetGlyphName(glyph.first)));
    }
#endif
    return eTrue;
  }

  void _DrawMain(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xDDDDDDDD);

    // Set wireframe state, use W to toggle
    apCanvas->GetVGMaterial()->SetRasterizerStates(
      mbWireframe ?
      eCompiledStates_RS_Wireframe :
      eCompiledStates_RS_NoCullingFilled);

    iVGStyle* style = apCanvas->GetVGStyle();
    style->SetFill(eTrue);
    style->SetStrokeColor(Vec3f(1,0,0));
    style->SetStrokeWidth(2.0f);
    style->SetFillColor(Vec3f(0,1,0));
    style->SetLineJoin(eVGLineJoin_Round);
    style->SetInnerJoin(eVGInnerJoin_Jag);

    mptrPath->Clear();

#if 0
    // hearth path
    mptrPath->AddSVGPath(
      " M 10,30"
      " A 20,20 0,0,1 50,30"
      " A 20,20 0,0,1 90,30"
      " Q 90,60 50,90"
      " Q 10,60 10,30 z");
#endif

    Ptr<iFont> fontIcons = mpWidget->GetGraphics()->LoadFont(_H("fas"));
    fontIcons->SetColor(ULColorBuild(0,0,0,255));
    fontIcons->SetSizeAndResolution(Vec2f(32,32),32,mpWidget->GetUIContext()->GetContentsScale());

    const tF32 ascent = fontIcons->GetAscent();
    const tF32 descent = fontIcons->GetDescent();
    const tF32 height = fontIcons->GetHeight();
    sVec2f pos = Vec2f(50,50);
    tU32 gi = fontIcons->GetGlyphIndexFromName("magnifying-glass");
    sVec2f glyphAdvance = fontIcons->GetGlyphPath(mptrPath, gi, pos+Vec2f(0,ascent), 1.0f);

    // Draw the glyph's box
    apCanvas->BlitFill(Rectf(pos.x,pos.y,(tF32)fontIcons->GetResolution(),(tF32)fontIcons->GetResolution()), 0xCCCCCC);
    apCanvas->BlitFill(Rectf(pos.x,pos.y+height,100,1), 0);
    apCanvas->BlitFill(Rectf(pos.x,pos.y+ascent,100,1), 0xFF00FF);
    apCanvas->BlitFill(Rectf(pos.x,pos.y+glyphAdvance.y,100,1), 0xFF0000);
    apCanvas->BlitFill(Rectf(pos.x+glyphAdvance.x,pos.y,1,100), 0xFF0000);

    apCanvas->BlitText(fontIcons,
                       Rectf(5,5,0,0),
                       0,
                       "Hi \uF002\uF004\uF040!");

    Ptr<iTextObject> textObject = mpWidget->GetGraphics()->CreateTextObject(
      "<font size='32' res='32' color='black'>Hi <font name='fas' color='darkgreen'><g name='magnifying-glass'/>Foo<g name='heart'/></font> Bar!", Vec2f(500,500), mpWidget->GetUIContext()->GetContentsScale());
    textObject->DrawAt(apCanvas, sRectf::Null(), Vec3f(pos.x,pos.y+120,0));

    static tBool _once = eTrue;
    if (_once) {
      niDebugFmt(("... gi: %d, res: %s, adv: %s, path: %s",
                  gi, fontIcons->GetResolution(), glyphAdvance, mptrPath->GetSVGPath()));
      _once = eFalse;
    }
    pos.x += glyphAdvance.x;

    // mptrPath->Scale(Vec2f(50,50));

    apCanvas->DrawPath(mptrPath);
  }
};
TEST_FIXTURE_WIDGET(FVG,Icons);
