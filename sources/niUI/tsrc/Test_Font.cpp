#include "stdafx.h"
#include <niLang/Utils/CrashReport.h>

using namespace ni;

struct FFont {
  QPtr<iGraphics> graphics;

  FFont() {
    niDebugFmt(("FFont::FFont"));
    niCrashReportModuleInstall();
    ni::GetLang()->InstallCrashHandler();
    graphics = niCreateInstance(niUI,Graphics,niVarNull,niVarNull);
    graphics->RegisterSystemFonts();
  }
  ~FFont() {
    niDebugFmt(("FFont::~FFont"));
  }
};

TEST_FIXTURE(FFont,IconFontBase) {
  Ptr<iFont> font = graphics->LoadFont(_H("far"));
  CHECK_EQUAL(font.IsOK(), eTrue);

  niDebugFmt(("... font %s loaded", font->GetName()));

  auto glyphs = font->EnumGlyphs();
  CHECK(glyphs.IsOK());

  // This needs to be updated when we update the regular font awesome icons font.
  CHECK_EQUAL(1850, glyphs->size());

  niDebugFmt(("... font %s has %d glyphs", font->GetName(), glyphs->size()));
  for (auto& glyph : *glyphs) {
    niDebugFmt(("... glyph: index: %d, codepoint: 0x%x, name: %s",
                glyph.first, glyph.second,
                font->GetGlyphName(glyph.first)));
  }

  tU32 glyphIndexSearchIcon = font->GetGlyphIndexFromName("search");
  CHECK_EQUAL(5, glyphIndexSearchIcon);
  CHECK_EQUAL(5, font->GetGlyphIndexFromCodepoint(0xf002));

  cString nameSearchIcon = font->GetGlyphName(glyphIndexSearchIcon);
  CHECK_EQUAL(_ASTR("search"), nameSearchIcon);
}

TEST_FIXTURE(FFont,OSFonts) {
  Ptr<tStringCMap> osFonts = graphics->EnumOSFonts();
  CHECK_EQUAL(osFonts.IsOK(), eTrue);
  CHECK(osFonts->size() > 0);

  niDebugFmt(("... osFonts: found %d", osFonts->size()));
  for (auto& osFont : *osFonts) {
    niDebugFmt(("... osFont: %s = %s",
                osFont.first, osFont.second));
  }
}
