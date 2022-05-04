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
  Ptr<iFont> font = graphics->LoadFont(_H("fas"));
  CHECK_EQUAL(font.IsOK(), eTrue);

  niDebugFmt(("... font %s loaded, %s", font->GetName(), font->GetFamilyName()));
  const tBool isFontAwesomeFontFamily = _ASTR(niHStr(font->GetFamilyName())).icontains("Font Awesome");
  CHECK(isFontAwesomeFontFamily);

  auto glyphs = font->EnumGlyphs();
  CHECK(glyphs.IsOK());

  niDebugFmt(("... font %s has %d glyphs", font->GetName(), glyphs->size()));
  {
    int c = 0;
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
  CHECK_EQUAL(osFonts.IsOK(), eTrue);
  CHECK(osFonts->size() > 0);

  niDebugFmt(("... osFonts: found %d", osFonts->size()));
  for (auto& osFont : *osFonts) {
    niDebugFmt(("... osFont: %s = %s",
                osFont.first, osFont.second));
  }
}
