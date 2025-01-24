#include "stdafx.h"
#include "TestCanvasWidgetSink.h"
#include "data/A.jpg.hxx"

struct FFont {
  QPtr<iGraphics> graphics;

  FFont() {
    niDebugFmt(("FFont::FFont"));
    graphics = niCreateInstance(niUI,Graphics,niVarNull,niVarNull);
    graphics->RegisterSystemFonts();
  }
  ~FFont() {
    niDebugFmt(("FFont::~FFont"));
  }
};

TEST_FIXTURE(FFont,IconFontBase) {
  Ptr<iFont> font = graphics->LoadFont(_H("fas"));
  CHECK(font.IsOK());

  niDebugFmt(("... font %s loaded, %s", font->GetName(), font->GetFamilyName()));
  const tBool isFontAwesomeFontFamily = _ASTR(niHStr(font->GetFamilyName())).icontains("Font Awesome");
  CHECK(isFontAwesomeFontFamily);

  auto glyphs = font->EnumGlyphs();
  CHECK(glyphs.IsOK());

  niDebugFmt(("... font %s has %d glyphs", font->GetName(), glyphs->size()));
  {
    tSize c = 0;
    for (auto& glyph : *glyphs) {
      niDebugFmt(("... glyph: index: %d, codepoint: 0x%x, name: %s",
                  glyph.first, glyph.second,
                  font->GetGlyphName(glyph.first)));
#if !defined _DEBUG
      if (c++ > 10)
        break;
#endif
    }
    if (c < glyphs->size()) {
      niDebugFmt(("... more glyphs skipped ..."));
    }
  }

  // This needs to be updated when we update the regular font awesome icons font.
  CHECK_EQUAL(1388, glyphs->size());
  tU32 glyphIndexSearchIcon = font->GetGlyphIndexFromName("magnifying-glass");
  CHECK_EQUAL(391, glyphIndexSearchIcon);
  CHECK_EQUAL(391, font->GetGlyphIndexFromCodepoint(0xf002));

  cString nameSearchIcon = font->GetGlyphName(glyphIndexSearchIcon);
  CHECK_EQUAL(_ASTR("magnifying-glass"), nameSearchIcon);
}

TEST_FIXTURE(FFont,OSFonts) {
  Ptr<tStringCMap> osFonts = graphics->EnumOSFonts();
  CHECK(osFonts.IsOK());
  CHECK(osFonts->size() > 0);

  niDebugFmt(("... osFonts: found %d", osFonts->size()));
  for (auto& osFont : *osFonts) {
    niDebugFmt(("... osFont: %s = %s",
                osFont.first, osFont.second));
  }
}

struct DrawTextIcons : public TestCanvasWidgetSink {
  TEST_CONSTRUCTOR_BASE(DrawTextIcons,TestCanvasWidgetSink) {
  }

  NN<iOverlay> _bgPic = niDeferredInit(NN<iOverlay>);
  NN<iFont> _iconFont = niDeferredInit(NN<iFont>);;
  NN<iOverlay> _glyphSearch = niDeferredInit(NN<iOverlay>);
  NN<iOverlay> _glyphHeart = niDeferredInit(NN<iOverlay>);

  niFn(tBool) OnSinkAttached() niImpl {
    niLet graphics = mpWidget->GetGraphics();
    niLet fp = niCheckNN(fp,niFileOpenBin2H(A_jpg),eFalse);
    niLet bmp = niCheckNN(bmp,graphics->LoadBitmap(fp),eFalse);
    niLet tex = niCheckNN(tex,graphics->CreateTextureFromBitmap(
      _H(fp->GetSourcePath()),bmp,eTextureFlags_Default),eFalse);
    _bgPic = niCheckNN(_bgPic,graphics->CreateOverlayTexture(tex),eFalse);
    _bgPic->SetFiltering(eTrue);

    _iconFont = niCheckNN(_iconFont,graphics->LoadFont(_H("fas")),eFalse);
    _iconFont->SetSizeAndResolution(Vec2f(64,64),64,mpWidget->GetUIContext()->GetContentsScale());

    _iconFont->SetColor(0xFF0000FF);
    niLet glyphIndexSearchIcon = _iconFont->GetGlyphIndexFromName("magnifying-glass");
    _glyphSearch = niCheckNN(_glyphSearch,_iconFont->GetGlyphOverlay(glyphIndexSearchIcon),eFalse);

    _iconFont->SetColor(0xFFFF0000);
    _glyphHeart = niCheckNN(_glyphHeart,_iconFont->GetGlyphOverlay(_iconFont->GetGlyphIndexFromName("heart")),eFalse);

    return eTrue;
  }

  void PaintTest(iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFF222222);
    apCanvas->BlitOverlay(
      Rectf(5,5,_bgPic->GetSize().x,_bgPic->GetSize().y),
      _bgPic);

    apCanvas->BlitOverlay(
      Rectf(
        _bgPic->GetSize().x-_glyphSearch->GetSize().x,
        5,
        _glyphSearch->GetSize().x,_glyphSearch->GetSize().y),
      _glyphSearch);

    apCanvas->BlitOverlay(
      Rectf(
        (_bgPic->GetSize().x-_glyphHeart->GetSize().x)/2.0f,
        (_bgPic->GetSize().y-_glyphHeart->GetSize().y)/2.0f,
        _glyphHeart->GetSize().x,_glyphHeart->GetSize().y),
      _glyphHeart);

    apCanvas->Flush();
  }
};
TEST_FIXTURE_WIDGET(FFont,DrawTextIcons);
