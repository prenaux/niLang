// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "Font.h"
#include "FontTTF.h"
#include "GetFontFile.h"
#include "Image.h"
#include "TextMetric.h"

_HDecl(Fallback);
_HDecl(Default);

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
iFont* cGraphics::CreateFont(iHString* ahspName, iFile* apFile)
{
  niCheckIsOK(ahspName,NULL);
  niCheckIsOK(apFile,NULL);

  Ptr<cFont> ptrFont = niNew cFont(this);
  if (!ptrFont->_CreateFromFile(ahspName,apFile)) {
    niError(_A("Can't load the font."));
    return NULL;
  }

  return ptrFont->CreateFontInstance(NULL);
}

///////////////////////////////////////////////
iFont* cGraphics::CreateFontFromBitmap(iHString* ahspName, iBitmap2D* apBmp)
{
  niCheckIsOK(ahspName,NULL);
  niCheckIsOK(apBmp,NULL);

  Ptr<cFont> ptrFont = niNew cFont(this);
  if (!ptrFont->_CreateFromBitmap(ahspName,apBmp)) {
    niError(_A("Can't load the font."));
    return NULL;
  }

  return ptrFont->CreateFontInstance(NULL);
}

///////////////////////////////////////////////
iFont* cGraphics::CreateFont8x8(iHString* ahspName)
{
  niCheckIsOK(ahspName,NULL);

  Ptr<cFont> ptrFont = niNew cFont(this);
  if (!ptrFont->_Create8x8(ahspName)) {
    niError(_A("Can't create the font."));
    return NULL;
  }

  return ptrFont->CreateFontInstance(NULL);
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::RegisterSystemFonts()
{
  if (mbRegisteredSystemFonts)
    return eTrue;

  mbRegisteredSystemFonts = eTrue;

  Ptr<iFont> font = CreateFont8x8(_H("8x8"));
  RegisterSystemFont(font->GetName(),NULL,font);

#define REGISTER_FONT(NAME,PATH) {                                      \
    Ptr<iFont> font = LoadFont(_H(PATH));                               \
    if (font.IsOK()) {                                                  \
      RegisterSystemFont(_H(NAME),NULL,font);                           \
    }                                                                   \
    else {                                                              \
      niWarning(niFmt("Couldn't load font '%s' with '%s'.", NAME, PATH)); \
    }                                                                   \
  }

  // Regular Fonts
  REGISTER_FONT("Sans", "niUI://fonts/Roboto-Regular.ttf");
  REGISTER_FONT("Sans Italic", "niUI://fonts/Roboto-Italic.ttf");
  REGISTER_FONT("Sans Bold", "niUI://fonts/Roboto-Bold.ttf");
  REGISTER_FONT("Sans Bold Italic", "niUI://fonts/Roboto-BoldItalic.ttf");
  REGISTER_FONT("Serif", "niUI://fonts/PlayfairDisplay-Regular.ttf");
  REGISTER_FONT("Serif Italic", "niUI://fonts/PlayfairDisplay-Italic.ttf");
  REGISTER_FONT("Serif Bold", "niUI://fonts/PlayfairDisplay-Bold.ttf");
  REGISTER_FONT("Serif Bold Italic", "niUI://fonts/PlayfairDisplay-BoldItalic.ttf");
  REGISTER_FONT("Monospace", "niUI://fonts/NotoMono-Regular.ttf");
  REGISTER_FONT("Fallback", "niUI://fonts/NotoSansCJKsc-Regular.otf");
  REGISTER_FONT("Default", "Sans");
  REGISTER_FONT("Default Italic", "Sans Italic");
  REGISTER_FONT("Default Bold", "Sans Bold");
  REGISTER_FONT("Default Bold Italic", "Sans Bold Italic");
  REGISTER_FONT("Code", "Monospace");

  // Icon Fonts
  REGISTER_FONT("fab", "niUI://fonts/fab-400.otf");
  REGISTER_FONT("far", "niUI://fonts/far-400.otf");
  REGISTER_FONT("fas", "niUI://fonts/fas-900.otf");

#undef REGISTER_FONT
  return eTrue;
}

///////////////////////////////////////////////
Ptr<tStringCMap> __stdcall cGraphics::EnumOSFonts() const {
  Ptr<tStringCMap> fontsMap = tStringCMap::Create();

  tFontFileLst lst;
  GetAllFontFiles(lst);

  cString fontDir = GetFontsDirectory();
  for (tFontFileLstIt it = lst.begin(); it != lst.end(); ++it) {
    cPath path;
    path.SetDirectory(fontDir.Chars());
    path.SetFile(it->strFile.Chars());
    if (!ni::GetRootFS()->FileExists(path.GetPath().Chars(),eFileAttrFlags_AllFiles))
      continue;
    fontsMap->insert(astl::make_pair(it->strDisp,path.GetPath()));
  }

  return fontsMap;
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::RegisterSystemFont(iHString* ahspName, iHString* ahspFilePath, iFont* apFont)
{
  if (HStringIsEmpty(ahspName)) {
    niError("Can't register a system font without a name.");
    return eFalse;
  }

  if (mmapSystemFonts.HasKey(ahspName)) {
    sSystemFont& sf = mmapSystemFonts.GetItem(ahspName);
    sf.hspFilePath = ahspFilePath;
    sf.ptrFont = apFont;
  }
  else {
    sSystemFont sf;
    sf.hspName = ahspName;
    sf.ptrFont = apFont;
    if (HStringIsNotEmpty(ahspFilePath)) {
      cPath path(niHStr(ahspFilePath));
      sf.hspFileName = _H(path.GetFile());
      sf.hspFilePath = _H(path.GetPath());
    }
    mmapSystemFonts.Set(sf.hspName,sf);
  }

  if (apFont) {
    niLog(Info,niFmt("Registered System font object '%s' ('%s', '%s')", ahspName, apFont->GetFilePath(), apFont->GetName()));
  }
  else {
    niLog(Info,niFmt("Registered System font file '%s' ('%s')", ahspName, ahspFilePath));
  }

  if (ahspName == _HC(Fallback)) {
    mptrFallbackFont = (cFont*)apFont;
  }
  return eTrue;
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetNumSystemFonts() const {
  return (tU32)mmapSystemFonts.GetSize();
}
iHString* __stdcall cGraphics::GetSystemFontName(tU32 anIndex) const {
  niCheckSilent(anIndex < mmapSystemFonts.GetSize(),NULL);
  return mmapSystemFonts.GetItem(anIndex).hspName;
}
iHString* __stdcall cGraphics::GetSystemFontFilePath(tU32 anIndex) const {
  niCheckSilent(anIndex < mmapSystemFonts.GetSize(),NULL);
  return mmapSystemFonts.GetItem(anIndex).hspFilePath;
}
iHString* __stdcall cGraphics::GetSystemFontFileName(tU32 anIndex) const {
  niCheckSilent(anIndex < mmapSystemFonts.GetSize(),NULL);
  return mmapSystemFonts.GetItem(anIndex).hspFileName;
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetNumFonts() const {
  return (tU32)mmapBaseFonts.GetSize();
}

///////////////////////////////////////////////
iFont* __stdcall cGraphics::GetFontFromIndex(tU32 anIndex) const {
  if (anIndex >= mmapBaseFonts.GetSize())
    return NULL;
  return mmapBaseFonts.GetItem(anIndex);
}

///////////////////////////////////////////////
iFont* __stdcall cGraphics::GetFontFromName(iHString* ahspName) const {

  // look in already loaded fonts
  iFont* const* baseFont = mmapBaseFonts.GetItemPtr(ahspName);
  if (baseFont != NULL) {
    return *baseFont;
  }

  // look in system fonts
  sSystemFont const* systemFont = mmapSystemFonts.GetItemPtr(ahspName);
  if (systemFont != NULL) {
    return systemFont->ptrFont;
  }

  return NULL;
}

///////////////////////////////////////////////
iFont* __stdcall cGraphics::FindFont(iHString* ahspName, tFontLoadFlags aFlags) const {
  if (ni::HStringIsEmpty(ahspName))
    return NULL;

  {
    iFont* pFontFromName = GetFontFromName(ahspName);
    if (pFontFromName) {
      return pFontFromName;
    }
  }

  cPath pathFont;
  astl::vector<cString> vNames;
  StringSplit(_ASTR(niHStr(ahspName)),_A(","),&vNames);
  niLoop(i,vNames.size()) {
    cString strName = vNames[i];
    strName.Normalize();
    if (!strName.contains(_A("."))) {
      if (niFlagIs(aFlags,eFontLoadFlags_Bold)) {
        if (strName.find(_A("Bold")) == cString::npos)
          strName += _A(" Bold");
      }
      if (niFlagIs(aFlags,eFontLoadFlags_Italic)) {
        if (strName.find(_A("Italic")) == cString::npos)
          strName += _A(" Italic");
      }
    }

    tHStringPtr hspName = _H(strName);

    // Look in the loaded fonts
    Ptr<iRegex> regex = ni::CreateFilePatternRegex(niHStr(hspName));

    // Look if the font is already loaded
    niLoop(i,mmapBaseFonts.GetSize()) {
      iFont* f = mmapBaseFonts.GetItem(i);
      if (f &&
          (f->GetName() == hspName ||
           f->GetFilePath() == hspName ||
           regex->DoesMatch(niHStr(f->GetName()))))
      {
        // found the font, return it
        return f;
      }
    }
  }

  return NULL;
}

///////////////////////////////////////////////
iFont* __stdcall cGraphics::LoadFont(iHString* ahspName) {
  return LoadFontEx(ahspName,eFontLoadFlags_Default);
}
iFont* __stdcall cGraphics::LoadFontEx(iHString* ahspName, tFontLoadFlags aFlags)
{
  if (ni::HStringIsEmpty(ahspName)) {
    ahspName = _HC(Fallback);
  }

  {
    iFont* f = GetFontFromName(ahspName);
    if (f) {
      return f->CreateFontInstance(NULL);
    }
  }

  cPath pathFont;
  astl::vector<cString> vNames;
  StringSplit(_ASTR(niHStr(ahspName)),_A(","),&vNames);
  niLoop(i,vNames.size()) {
    cString strName = vNames[i];
    strName.Normalize();

    if (!strName.contains(_A("."))) {
      if (niFlagIs(aFlags,eFontLoadFlags_Bold)) {
        if (strName.find(_A("Bold")) == cString::npos)
          strName += _A(" Bold");
      }
      if (niFlagIs(aFlags,eFontLoadFlags_Italic)) {
        if (strName.find(_A("Italic")) == cString::npos)
          strName += _A(" Italic");
      }
    }

    tHStringPtr hspName = _H(strName);

    // Look in the loaded fonts
    Ptr<iRegex> regex = ni::CreateFilePatternRegex(niHStr(hspName));

    // Look if the font is already loaded
    niLoop(i,mmapBaseFonts.GetSize()) {
      iFont* f = mmapBaseFonts.GetItem(i);
      if (f &&
          (f->GetName() == hspName ||
           f->GetFilePath() == hspName ||
           regex->DoesMatch(niHStr(f->GetName()))))
      {
        // found the font, return it
        return f;
      }
    }

    // Font is not loaded, look in the system fonts
    niLoop(i,mmapSystemFonts.GetSize()) {
      const sSystemFont& sf = mmapSystemFonts.GetItem(i);
      if (sf.hspName == hspName ||
          sf.hspFileName == hspName ||
          sf.hspFilePath == hspName ||
          regex->DoesMatch(niHStr(sf.hspName)))
      {
        if (sf.ptrFont.IsOK()) {
          return sf.ptrFont->CreateFontInstance(NULL);
        }
        else {
          pathFont = niHStr(sf.hspFilePath);
          break;
        }
      }
    }

    // Didn't find a font with the name so we'll just use the name straigh away as path
    if (pathFont.GetPath().IsEmpty()) {
      pathFont = niHStr(hspName);
    }

    // If the resource exist we found it, get to the loading part
    if (ni::GetLang()->URLExists(pathFont.GetPath().Chars())) {
      break;
    }

    // Resource not found, reset the path and process the next name
    pathFont = AZEROSTR;
  }

  // using a while loop so I can break without using a goto, note that we
  // return at the end if everything when fine so the loop will be excuted
  // only once no matter what
  while (!pathFont.GetPath().IsEmpty()) {
    Ptr<iFile> ptrFile = ni::GetLang()->URLOpen(pathFont.GetPath().Chars());
    if (!ptrFile.IsOK()) {
      niWarning(niFmt(_A("Font '%s', can't open font file '%s'."),
                      niHStr(ahspName),
                      pathFont.GetPath().Chars()));
      break;
    }
    Ptr<iFont> ptrFont = this->CreateFont(ahspName,ptrFile);
    if (!ptrFont.IsOK()) {
      niWarning(niFmt(_A("Font '%s', can't create font from file '%s'."),
                      niHStr(ahspName),
                      pathFont.GetPath().Chars()));
      break;
    }
    return ptrFont.GetRawAndSetNull();
  }

  // look for default
  Ptr<iFont> defaultFont = FindFont(_HC(Fallback),aFlags);
  if (defaultFont.IsOK()) {
    return defaultFont->CreateFontInstance(NULL);
  }
  else if ((aFlags&eFontLoadFlags_Bold) || (aFlags&eFontLoadFlags_Italic)) {
    // try without any attributes...
    defaultFont = FindFont(_HC(Fallback),0);
    if (defaultFont.IsOK()) {
      return defaultFont->CreateFontInstance(NULL);
    }
  }

  // look for 8x8
  defaultFont = GetFontFromName(_H("8x8"));
  if (defaultFont.IsOK()) {
    return defaultFont->CreateFontInstance(NULL);
  }

  // create 8x8
  defaultFont = CreateFont8x8(_H("8x8"));
  return defaultFont.GetRawAndSetNull();
}

///////////////////////////////////////////////
void cGraphics::_RegisterBaseFont(cFont* apFont) {
  niAssert(!apFont->mptrParentFont.IsOK());
  niLog(Info,niFmt("Registered Base Font '%s' (family: %s, style: %s, filename: %s, filepath: %s)",
                   apFont->GetName(),
                   apFont->GetFamilyName(),
                   apFont->GetStyleName(),
                   apFont->GetFilePath()));
  mmapBaseFonts.Set(apFont->GetName(),apFont);
}
tBool cGraphics::_UnregisterBaseFont(iFont* apFont) {
  niLoop(i,mmapBaseFonts.GetSize()) {
    if (mmapBaseFonts.GetItem(i) == apFont) {
      niAssert(!((cFont*)apFont)->mptrParentFont.IsOK());
      niLog(Info,niFmt("Unregistered Base Font '%s' (family: %s, style: %s, filename: %s, filepath: %s)",
                       apFont->GetName(),
                       apFont->GetFamilyName(),
                       apFont->GetStyleName(),
                       apFont->GetFilePath()));
      mmapBaseFonts.EraseIndex(i);
      return eTrue;
    }
  }
  return eFalse;
}
cFont* __stdcall cGraphics::_GetFallbackFont() const {
  return const_cast<cFont*>(this->mptrFallbackFont.ptr());
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::ClearAllFontCaches() {
  niLoop(i,mmapBaseFonts.GetSize()) {
    iFont* f = mmapBaseFonts.GetItem(i);
    if (f) {
      f->ClearCache();
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetFontCacheID() const {
  return mnFontCacheID;
}

//////////////////////////////////////////////////////////////////////////////////////////////
static tU8 _DefaultFontData[256*8] =
{
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, /* Char(00) NUL */ // NULL is white and to be used as background color
  0x7E,0x81,0xA5,0x81,0xBD,0x99,0x81,0x7E, /* Char(01) SOH */
  0x7E,0xFF,0xDB,0xFF,0xC3,0xE7,0xFF,0x7E, /* Char(02) STX */
  0x6C,0xFE,0xFE,0xFE,0x7C,0x38,0x10,0x00, /* Char(03) ETX */
  0x10,0x38,0x7C,0xFE,0x7C,0x38,0x10,0x00, /* Char(04) EOT */
  0x38,0x7C,0x38,0xFE,0xFE,0x7C,0x38,0x7C, /* Char(05) ENQ */
  0x10,0x10,0x38,0x7C,0xFE,0x7C,0x38,0x7C, /* Char(06) ACK */
  0x00,0x00,0x18,0x3C,0x3C,0x18,0x00,0x00, /* Char(07) BELL */
  0xFF,0xFF,0xE7,0xC3,0xC3,0xE7,0xFF,0xFF, /* Char(08) BACKSPACE */
  0x00,0x3C,0x66,0x42,0x42,0x66,0x3C,0x00, /* Char(09) TAB */
  0xFF,0xC3,0x99,0xBD,0xBD,0x99,0xC3,0xFF, /* Char(0A) LF */
  0x0F,0x07,0x0F,0x7D,0xCC,0xCC,0xCC,0x78, /* Char(0B) VT */
  0x3C,0x66,0x66,0x66,0x3C,0x18,0x7E,0x18, /* Char(0C) FF */
  0x3F,0x33,0x3F,0x30,0x30,0x70,0xF0,0xE0, /* Char(0D) CR */
  0x7F,0x63,0x7F,0x63,0x63,0x67,0xE6,0xC0, /* Char(0E) SO */
  0x99,0x5A,0x3C,0xE7,0xE7,0x3C,0x5A,0x99, /* Char(0F) SI */
  0x80,0xE0,0xF8,0xFE,0xF8,0xE0,0x80,0x00, /* Char(10) DLE */
  0x02,0x0E,0x3E,0xFE,0x3E,0x0E,0x02,0x00, /* Char(11) DC1 */
  0x18,0x3C,0x7E,0x18,0x18,0x7E,0x3C,0x18, /* Char(12) DC2 */
  0x66,0x66,0x66,0x66,0x66,0x00,0x66,0x00, /* Char(13) DC3 */
  0x7F,0xDB,0xDB,0x7B,0x1B,0x1B,0x1B,0x00, /* Char(14) DC4 */
  0x3E,0x63,0x38,0x6C,0x6C,0x38,0xCC,0x78, /* Char(15) NAK */
  0x00,0x00,0x00,0x00,0x7E,0x7E,0x7E,0x00, /* Char(16) SYN */
  0x18,0x3C,0x7E,0x18,0x7E,0x3C,0x18,0xFF, /* Char(17) ETB */
  0x18,0x3C,0x7E,0x18,0x18,0x18,0x18,0x00, /* Char(18) CAN */
  0x18,0x18,0x18,0x18,0x7E,0x3C,0x18,0x00, /* Char(19) EM */
  0x00,0x18,0x0C,0xFE,0x0C,0x18,0x00,0x00, /* Char(1A) SUB */
  0x00,0x30,0x60,0xFE,0x60,0x30,0x00,0x00, /* Char(1B) ESC */
  0x00,0x00,0xC0,0xC0,0xC0,0xFE,0x00,0x00, /* Char(1C) FS */
  0x00,0x24,0x66,0xFF,0x66,0x24,0x00,0x00, /* Char(1D) GS */
  0x00,0x18,0x3C,0x7E,0xFF,0xFF,0x00,0x00, /* Char(1E) RS */
  0x00,0xFF,0xFF,0x7E,0x3C,0x18,0x00,0x00, /* Char(1F) US */
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* Char(20) SPACE */
  0x30,0x78,0x78,0x78,0x30,0x00,0x30,0x00, /* Char(21) ! */
  0x6C,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00, /* Char(22) " */
  0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00, /* Char(23) # */
  0x30,0x7C,0xC0,0x78,0x0C,0xF8,0x30,0x00, /* Char(24) $ */
  0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00, /* Char(25) % */
  0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00, /* Char(26) & */
  0x60,0x60,0xC0,0x00,0x00,0x00,0x00,0x00, /* Char(27) ' */
  0x18,0x30,0x60,0x60,0x60,0x30,0x18,0x00, /* Char(28) ( */
  0x60,0x30,0x18,0x18,0x18,0x30,0x60,0x00, /* Char(29) ) */
  0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00, /* Char(2A) * */
  0x00,0x30,0x30,0xFC,0x30,0x30,0x00,0x00, /* Char(2B) + */
  0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60, /* Char(2C) , */
  0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0x00, /* Char(2D) - */
  0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00, /* Char(2E) . */
  0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00, /* Char(2F) / */
  0x7C,0xC6,0xCE,0xDE,0xF6,0xE6,0x7C,0x00, /* Char(30) 0 */
  0x30,0x70,0x30,0x30,0x30,0x30,0xFC,0x00, /* Char(31) 1 */
  0x78,0xCC,0x0C,0x38,0x60,0xCC,0xFC,0x00, /* Char(32) 2 */
  0x78,0xCC,0x0C,0x38,0x0C,0xCC,0x78,0x00, /* Char(33) 3 */
  0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x1E,0x00, /* Char(34) 4 */
  0xFC,0xC0,0xF8,0x0C,0x0C,0xCC,0x78,0x00, /* Char(35) 5 */
  0x38,0x60,0xC0,0xF8,0xCC,0xCC,0x78,0x00, /* Char(36) 6 */
  0xFC,0xCC,0x0C,0x18,0x30,0x30,0x30,0x00, /* Char(37) 7 */
  0x78,0xCC,0xCC,0x78,0xCC,0xCC,0x78,0x00, /* Char(38) 8 */
  0x78,0xCC,0xCC,0x7C,0x0C,0x18,0x70,0x00, /* Char(39) 9 */
  0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x00, /* Char(3A) : */
  0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x60, /* Char(3B) ; */
  0x18,0x30,0x60,0xC0,0x60,0x30,0x18,0x00, /* Char(3C) < */
  0x00,0x00,0xFC,0x00,0x00,0xFC,0x00,0x00, /* Char(3D) = */
  0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00, /* Char(3E) > */
  0x78,0xCC,0x0C,0x18,0x30,0x00,0x30,0x00, /* Char(3F) ? */
  0x7C,0xC6,0xDE,0xDE,0xDE,0xC0,0x78,0x00, /* Char(40) @ */
  0x30,0x78,0xCC,0xCC,0xFC,0xCC,0xCC,0x00, /* Char(41) A */
  0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00, /* Char(42) B */
  0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00, /* Char(43) C */
  0xF8,0x6C,0x66,0x66,0x66,0x6C,0xF8,0x00, /* Char(44) D */
  0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0x00, /* Char(45) E */
  0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0x00, /* Char(46) F */
  0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00, /* Char(47) G */
  0xCC,0xCC,0xCC,0xFC,0xCC,0xCC,0xCC,0x00, /* Char(48) H */
  0x78,0x30,0x30,0x30,0x30,0x30,0x78,0x00, /* Char(49) I */
  0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00, /* Char(4A) J */
  0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00, /* Char(4B) K */
  0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00, /* Char(4C) L */
  0xC6,0xEE,0xFE,0xFE,0xD6,0xC6,0xC6,0x00, /* Char(4D) M */
  0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00, /* Char(4E) N */
  0x38,0x6C,0xC6,0xC6,0xC6,0x6C,0x38,0x00, /* Char(4F) O */
  0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00, /* Char(50) P */
  0x78,0xCC,0xCC,0xCC,0xDC,0x78,0x1C,0x00, /* Char(51) Q */
  0xFC,0x66,0x66,0x7C,0x6C,0x66,0xE6,0x00, /* Char(52) R */
  0x78,0xCC,0xE0,0x70,0x1C,0xCC,0x78,0x00, /* Char(53) S */
  0xFC,0x30,0x30,0x30,0x30,0x30,0x30,0x00, /* Char(54) T */
  0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFC,0x00, /* Char(55) U */
  0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x30,0x00, /* Char(56) V */
  0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00, /* Char(57) W */
  0xC6,0xC6,0x6C,0x38,0x38,0x6C,0xC6,0x00, /* Char(58) X */
  0xCC,0xCC,0xCC,0x78,0x30,0x30,0x78,0x00, /* Char(59) Y */
  0xFE,0x06,0x0C,0x18,0x30,0x60,0xFE,0x00, /* Char(5A) Z */
  0x78,0x60,0x60,0x60,0x60,0x60,0x78,0x00, /* Char(5B) [ */
  0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00, /* Char(5C) '\' */
  0x78,0x18,0x18,0x18,0x18,0x18,0x78,0x00, /* Char(5D) ] */
  0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00, /* Char(5E) ^ */
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF, /* Char(5F) _ */
  0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00, /* Char(60) ` */
  0x00,0x00,0x78,0x0C,0x7C,0xCC,0x76,0x00, /* Char(61) a */
  0xE0,0x60,0x60,0x7C,0x66,0x66,0xDC,0x00, /* Char(62) b */
  0x00,0x00,0x78,0xCC,0xC0,0xCC,0x78,0x00, /* Char(63) c */
  0x1C,0x0C,0x0C,0x7C,0xCC,0xCC,0x76,0x00, /* Char(64) d */
  0x00,0x00,0x78,0xCC,0xFC,0xC0,0x78,0x00, /* Char(65) e */
  0x38,0x6C,0x60,0xF0,0x60,0x60,0xF0,0x00, /* Char(66) f */
  0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0xF8, /* Char(67) g */
  0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0x00, /* Char(68) h */
  0x30,0x00,0x70,0x30,0x30,0x30,0x78,0x00, /* Char(69) i */
  0x0C,0x00,0x0C,0x0C,0x0C,0xCC,0xCC,0x78, /* Char(6A) j */
  0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0x00, /* Char(6B) k */
  0x70,0x30,0x30,0x30,0x30,0x30,0x78,0x00, /* Char(6C) l */
  0x00,0x00,0xCC,0xFE,0xFE,0xD6,0xC6,0x00, /* Char(6D) m */
  0x00,0x00,0xF8,0xCC,0xCC,0xCC,0xCC,0x00, /* Char(6E) n */
  0x00,0x00,0x78,0xCC,0xCC,0xCC,0x78,0x00, /* Char(6F) o */
  0x00,0x00,0xDC,0x66,0x66,0x7C,0x60,0xF0, /* Char(70) p */
  0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x1E, /* Char(71) q */
  0x00,0x00,0xDC,0x76,0x66,0x60,0xF0,0x00, /* Char(72) r */
  0x00,0x00,0x7C,0xC0,0x78,0x0C,0xF8,0x00, /* Char(73) s */
  0x10,0x30,0x7C,0x30,0x30,0x34,0x18,0x00, /* Char(74) t */
  0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x76,0x00, /* Char(75) u */
  0x00,0x00,0xCC,0xCC,0xCC,0x78,0x30,0x00, /* Char(76) v */
  0x00,0x00,0xC6,0xD6,0xFE,0xFE,0x6C,0x00, /* Char(77) w */
  0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00, /* Char(78) x */
  0x00,0x00,0xCC,0xCC,0xCC,0x7C,0x0C,0xF8, /* Char(79) y */
  0x00,0x00,0xFC,0x98,0x30,0x64,0xFC,0x00, /* Char(7A) z */
  0x1C,0x30,0x30,0xE0,0x30,0x30,0x1C,0x00, /* Char(7B) */
  0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00, /* Char(7C) | */
  0xE0,0x30,0x30,0x1C,0x30,0x30,0xE0,0x00, /* Char(7D) } */
  0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00, /* Char(7E) ~ */
  0x00,0x10,0x38,0x6C,0xC6,0xC6,0xFE,0x00, /* Char(7F) DEL */
  0x78,0xCC,0xC0,0xCC,0x78,0x18,0x0C,0x78, /* Char(80) */
  0x00,0xCC,0x00,0xCC,0xCC,0xCC,0x7E,0x00, /* Char(81) */
  0x1C,0x00,0x78,0xCC,0xFC,0xC0,0x78,0x00, /* Char(82) */
  0x7E,0xC3,0x3C,0x06,0x3E,0x66,0x3F,0x00, /* Char(83) */
  0xCC,0x00,0x78,0x0C,0x7C,0xCC,0x7E,0x00, /* Char(84) */
  0xE0,0x00,0x78,0x0C,0x7C,0xCC,0x7E,0x00, /* Char(85) */
  0x30,0x30,0x78,0x0C,0x7C,0xCC,0x7E,0x00, /* Char(86) */
  0x00,0x00,0x78,0xC0,0xC0,0x78,0x0C,0x38, /* Char(87) */
  0x7E,0xC3,0x3C,0x66,0x7E,0x60,0x3C,0x00, /* Char(88) */
  0xCC,0x00,0x78,0xCC,0xFC,0xC0,0x78,0x00, /* Char(89) */
  0xE0,0x00,0x78,0xCC,0xFC,0xC0,0x78,0x00, /* Char(8A) */
  0xCC,0x00,0x70,0x30,0x30,0x30,0x78,0x00, /* Char(8B) */
  0x7C,0xC6,0x38,0x18,0x18,0x18,0x3C,0x00, /* Char(8C) */
  0xE0,0x00,0x70,0x30,0x30,0x30,0x78,0x00, /* Char(8D) */
  0xC6,0x38,0x6C,0xC6,0xFE,0xC6,0xC6,0x00, /* Char(8E) */
  0x30,0x30,0x00,0x78,0xCC,0xFC,0xCC,0x00, /* Char(8F) */
  0x1C,0x00,0xFC,0x60,0x78,0x60,0xFC,0x00, /* Char(90) */
  0x00,0x00,0x7F,0x0C,0x7F,0xCC,0x7F,0x00, /* Char(91) */
  0x3E,0x6C,0xCC,0xFE,0xCC,0xCC,0xCE,0x00, /* Char(92) */
  0x78,0xCC,0x00,0x78,0xCC,0xCC,0x78,0x00, /* Char(93) */
  0x00,0xCC,0x00,0x78,0xCC,0xCC,0x78,0x00, /* Char(94) */
  0x00,0xE0,0x00,0x78,0xCC,0xCC,0x78,0x00, /* Char(95) */
  0x78,0xCC,0x00,0xCC,0xCC,0xCC,0x7E,0x00, /* Char(96) */
  0x00,0xE0,0x00,0xCC,0xCC,0xCC,0x7E,0x00, /* Char(97) */
  0x00,0xCC,0x00,0xCC,0xCC,0x7C,0x0C,0xF8, /* Char(98) */
  0xC3,0x18,0x3C,0x66,0x66,0x3C,0x18,0x00, /* Char(99) */
  0xCC,0x00,0xCC,0xCC,0xCC,0xCC,0x78,0x00, /* Char(9A) */
  0x18,0x18,0x7E,0xC0,0xC0,0x7E,0x18,0x18, /* Char(9B) */
  0x38,0x6C,0x64,0xF0,0x60,0xE6,0xFC,0x00, /* Char(9C) */
  0xCC,0xCC,0x78,0xFC,0x30,0xFC,0x30,0x30, /* Char(9D) */
  0xF8,0xCC,0xCC,0xFA,0xC6,0xCF,0xC6,0xC7, /* Char(9E) */
  0x0E,0x1B,0x18,0x3C,0x18,0x18,0xD8,0x70, /* Char(9F) */
  0x1C,0x00,0x78,0x0C,0x7C,0xCC,0x7E,0x00, /* Char(A0) */
  0x38,0x00,0x70,0x30,0x30,0x30,0x78,0x00, /* Char(A1) */
  0x00,0x1C,0x00,0x78,0xCC,0xCC,0x78,0x00, /* Char(A2) */
  0x00,0x1C,0x00,0xCC,0xCC,0xCC,0x7E,0x00, /* Char(A3) */
  0x00,0xF8,0x00,0xF8,0xCC,0xCC,0xCC,0x00, /* Char(A4) */
  0xFC,0x00,0xCC,0xEC,0xFC,0xDC,0xCC,0x00, /* Char(A5) */
  0x3C,0x6C,0x6C,0x3E,0x00,0x7E,0x00,0x00, /* Char(A6) */
  0x38,0x6C,0x6C,0x38,0x00,0x7C,0x00,0x00, /* Char(A7) */
  0x30,0x00,0x30,0x60,0xC0,0xCC,0x78,0x00, /* Char(A8) */
  0x00,0x00,0x00,0xFC,0xC0,0xC0,0x00,0x00, /* Char(A9) */
  0x00,0x00,0x00,0xFC,0x0C,0x0C,0x00,0x00, /* Char(AA) */
  0xC3,0xC6,0xCC,0xDE,0x33,0x66,0xCC,0x0F, /* Char(AB) */
  0xC3,0xC6,0xCC,0xDB,0x37,0x6F,0xCF,0x03, /* Char(AC) */
  0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x00, /* Char(AD) */
  0x00,0x33,0x66,0xCC,0x66,0x33,0x00,0x00, /* Char(AE) */
  0x00,0xCC,0x66,0x33,0x66,0xCC,0x00,0x00, /* Char(AF) */
  0x22,0x88,0x22,0x88,0x22,0x88,0x22,0x88, /* Char(B0) */
  0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA, /* Char(B1) */
  0xDB,0x77,0xDB,0xEE,0xDB,0x77,0xDB,0xEE, /* Char(B2) */
  0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, /* Char(B3) */
  0x18,0x18,0x18,0x18,0xF8,0x18,0x18,0x18, /* Char(B4) */
  0x18,0x18,0xF8,0x18,0xF8,0x18,0x18,0x18, /* Char(B5) */
  0x36,0x36,0x36,0x36,0xF6,0x36,0x36,0x36, /* Char(B6) */
  0x00,0x00,0x00,0x00,0xFE,0x36,0x36,0x36, /* Char(B7) */
  0x00,0x00,0xF8,0x18,0xF8,0x18,0x18,0x18, /* Char(B8) */
  0x36,0x36,0xF6,0x06,0xF6,0x36,0x36,0x36, /* Char(B9) */
  0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36, /* Char(BA) */
  0x00,0x00,0xFE,0x06,0xF6,0x36,0x36,0x36, /* Char(BB) */
  0x36,0x36,0xF6,0x06,0xFE,0x00,0x00,0x00, /* Char(BC) */
  0x36,0x36,0x36,0x36,0xFE,0x00,0x00,0x00, /* Char(BD) */
  0x18,0x18,0xF8,0x18,0xF8,0x00,0x00,0x00, /* Char(BE) */
  0x00,0x00,0x00,0x00,0xF8,0x18,0x18,0x18, /* Char(BF) */
  0x18,0x18,0x18,0x18,0x1F,0x00,0x00,0x00, /* Char(C0) */
  0x18,0x18,0x18,0x18,0xFF,0x00,0x00,0x00, /* Char(C1) */
  0x00,0x00,0x00,0x00,0xFF,0x18,0x18,0x18, /* Char(C2) */
  0x18,0x18,0x18,0x18,0x1F,0x18,0x18,0x18, /* Char(C3) */
  0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00, /* Char(C4) */
  0x18,0x18,0x18,0x18,0xFF,0x18,0x18,0x18, /* Char(C5) */
  0x18,0x18,0x1F,0x18,0x1F,0x18,0x18,0x18, /* Char(C6) */
  0x36,0x36,0x36,0x36,0x37,0x36,0x36,0x36, /* Char(C7) */
  0x36,0x36,0x37,0x30,0x3F,0x00,0x00,0x00, /* Char(C8) */
  0x00,0x00,0x3F,0x30,0x37,0x36,0x36,0x36, /* Char(C9) */
  0x36,0x36,0xF7,0x00,0xFF,0x00,0x00,0x00, /* Char(CA) */
  0x00,0x00,0xFF,0x00,0xF7,0x36,0x36,0x36, /* Char(CB) */
  0x36,0x36,0x37,0x30,0x37,0x36,0x36,0x36, /* Char(CC) */
  0x00,0x00,0xFF,0x00,0xFF,0x00,0x00,0x00, /* Char(CD) */
  0x36,0x36,0xF7,0x00,0xF7,0x36,0x36,0x36, /* Char(CE) */
  0x18,0x18,0xFF,0x00,0xFF,0x00,0x00,0x00, /* Char(CF) */
  0x36,0x36,0x36,0x36,0xFF,0x00,0x00,0x00, /* Char(D0) */
  0x00,0x00,0xFF,0x00,0xFF,0x18,0x18,0x18, /* Char(D1) */
  0x00,0x00,0x00,0x00,0xFF,0x36,0x36,0x36, /* Char(D2) */
  0x36,0x36,0x36,0x36,0x3F,0x00,0x00,0x00, /* Char(D3) */
  0x18,0x18,0x1F,0x18,0x1F,0x00,0x00,0x00, /* Char(D4) */
  0x00,0x00,0x1F,0x18,0x1F,0x18,0x18,0x18, /* Char(D5) */
  0x00,0x00,0x00,0x00,0x3F,0x36,0x36,0x36, /* Char(D6) */
  0x36,0x36,0x36,0x36,0xFF,0x36,0x36,0x36, /* Char(D7) */
  0x18,0x18,0xFF,0x18,0xFF,0x18,0x18,0x18, /* Char(D8) */
  0x18,0x18,0x18,0x18,0xF8,0x00,0x00,0x00, /* Char(D9) */
  0x00,0x00,0x00,0x00,0x1F,0x18,0x18,0x18, /* Char(DA) */
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, /* Char(DB) */
  0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF, /* Char(DC) */
  0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0, /* Char(DD) */
  0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F, /* Char(DE) */
  0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00, /* Char(DF) */
  0x00,0x00,0x76,0xDC,0xC8,0xDC,0x76,0x00, /* Char(E0) */
  0x00,0x78,0xCC,0xF8,0xCC,0xF8,0xC0,0xC0, /* Char(E1) */
  0x00,0xFC,0xCC,0xC0,0xC0,0xC0,0xC0,0x00, /* Char(E2) */
  0x00,0xFE,0x6C,0x6C,0x6C,0x6C,0x6C,0x00, /* Char(E3) */
  0xFC,0xCC,0x60,0x30,0x60,0xCC,0xFC,0x00, /* Char(E4) */
  0x00,0x00,0x7E,0xD8,0xD8,0xD8,0x70,0x00, /* Char(E5) */
  0x00,0x66,0x66,0x66,0x66,0x7C,0x60,0xC0, /* Char(E6) */
  0x00,0x76,0xDC,0x18,0x18,0x18,0x18,0x00, /* Char(E7) */
  0xFC,0x30,0x78,0xCC,0xCC,0x78,0x30,0xFC, /* Char(E8) */
  0x38,0x6C,0xC6,0xFE,0xC6,0x6C,0x38,0x00, /* Char(E9) */
  0x38,0x6C,0xC6,0xC6,0x6C,0x6C,0xEE,0x00, /* Char(EA) */
  0x1C,0x30,0x18,0x7C,0xCC,0xCC,0x78,0x00, /* Char(EB) */
  0x00,0x00,0x7E,0xDB,0xDB,0x7E,0x00,0x00, /* Char(EC) */
  0x06,0x0C,0x7E,0xDB,0xDB,0x7E,0x60,0xC0, /* Char(ED) */
  0x38,0x60,0xC0,0xF8,0xC0,0x60,0x38,0x00, /* Char(EE) */
  0x78,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x00, /* Char(EF) */
  0x00,0xFC,0x00,0xFC,0x00,0xFC,0x00,0x00, /* Char(F0) */
  0x30,0x30,0xFC,0x30,0x30,0x00,0xFC,0x00, /* Char(F1) */
  0x60,0x30,0x18,0x30,0x60,0x00,0xFC,0x00, /* Char(F2) */
  0x18,0x30,0x60,0x30,0x18,0x00,0xFC,0x00, /* Char(F3) */
  0x0E,0x1B,0x1B,0x18,0x18,0x18,0x18,0x18, /* Char(F4) */
  0x18,0x18,0x18,0x18,0x18,0xD8,0xD8,0x70, /* Char(F5) */
  0x30,0x30,0x00,0xFC,0x00,0x30,0x30,0x00, /* Char(F6) */
  0x00,0x76,0xDC,0x00,0x76,0xDC,0x00,0x00, /* Char(F7) */
  0x38,0x6C,0x6C,0x38,0x00,0x00,0x00,0x00, /* Char(F8) */
  0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00, /* Char(F9) */
  0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00, /* Char(FA) */
  0x0F,0x0C,0x0C,0x0C,0xEC,0x6C,0x3C,0x1C, /* Char(FB) */
  0x78,0x6C,0x6C,0x6C,0x6C,0x00,0x00,0x00, /* Char(FC) */
  0x70,0x18,0x30,0x60,0x78,0x00,0x00,0x00, /* Char(FD) */
  0x00,0x00,0x3C,0x3C,0x3C,0x3C,0x00,0x00, /* Char(FE) */
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, /* Char(FF) */
};

///////////////////////////////////////////////
// Assumes that the destination bitmap is R8G8B8A8
static inline void _Draw8BitFontChar(const tU8* apFontData, iBitmap2D* apBmp, tU32 aX, tU32 aY, tU32 aC) {
  const tU8* fontData = apFontData+(aC<<3);
  const tU32 pitch = apBmp->GetPitch();
  const tPtr bmpData = apBmp->GetData();
  for (tU32 y = 0; y < 8; ++y) {
    tU8* dst = bmpData + (y+aY)*pitch + (aX * 4);
    tU32 i = 0x80;
    tU32 d = *(fontData++);
    for (tU32 x = 0; x < 8; ++x, i >>= 1, dst += 4) {
      dst[0] = dst[1] = dst[2] = //255;
      dst[3] = (d & i) ? 255 : 0;
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Font interface implementation

///////////////////////////////////////////////
cFont::cFont(cGraphics* apGraphics) {
  mpwGraphics.Swap(apGraphics);
  mStates.NewMaterial(apGraphics);
}

///////////////////////////////////////////////
tBool cFont::_CreateFromFile(iHString* ahspName, iFile* pFile)
{
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  niCheck(ptrGraphics.IsOK(),eFalse);

  cPath filePath = pFile->GetSourcePath();
  mhspFilePath = _H(filePath.GetPath());
  mhspFileName = _H(filePath.GetFile());

  const cString ext = filePath.GetExtension();
  // Load TTF font
  if (ext.IEq("ttf") ||
      ext.IEq("ttc") ||
      ext.IEq("otf") ||
      ext.IEq("otc") ||
      ext.IEq("pfa") ||
      ext.IEq("pfb"))
  {
    mptrTTF = niNew cFontTTF(ptrGraphics,ptrGraphics->GetFTLibrary(),pFile);
    if (!mptrTTF.IsOK()) {
      niError(_A("Can't create TTF font."));
      return eFalse;
    }
    mhspName = _H(mptrTTF->GetName());
    mhspFamilyName = _H(mptrTTF->GetName());
    mhspStyleName = _H(mptrTTF->GetName());
    SetSizeAndResolution(sVec2f::Zero(),mptrTTF->GetMaxCharSize(),1);
  }
  // Load a bitmap font
  else {
    QPtr<iBitmap2D> ptrBmp = ptrGraphics->LoadBitmap(pFile);
    if (!ptrBmp.ptr()) {
      niError(niFmt(_A("Can't load 2d bitmap from '%s'."),
                    pFile->GetSourcePath()));
      return eFalse;
    }

    cPath path(pFile->GetSourcePath());
    path.SetExtension(AZEROSTR);
    tHStringPtr hspName = _H(path.GetFile());
    if (!_CreateFromBitmap(hspName,ptrBmp)) {
      niError(_A("Can't create font from the loaded bitmap."));
      return eFalse;
    }
  }

  if (!HStringIsEmpty(ahspName)) {
    mhspName = ahspName;
  }

  ptrGraphics->_RegisterBaseFont(this);
  return eTrue;
}

///////////////////////////////////////////////
tBool cFont::_CreateFromBitmap(iHString* ahspName, iBitmap2D* pBmp)
{
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  niCheck(ptrGraphics.IsOK(),eFalse);
  niCheckIsOK(pBmp,eFalse);

  mhspName = ahspName;

  mptrImage = ptrGraphics->CreateImageFromBitmap(pBmp);
  niCheckIsOK(mptrImage,eFalse);

  Image_SetDebugName(mptrImage,niFmt(_A("FONT_%s"),niHStr(ahspName)));

  ptrGraphics->_RegisterBaseFont(this);
  return eTrue;
}

///////////////////////////////////////////////
tBool cFont::_Create8x8(iHString* ahspName)
{
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  niCheck(ptrGraphics.IsOK(),eFalse);

  mhspName = _H("8x8");
  mhspFilePath = NULL;
  mhspFileName = NULL;

  mptrImage = ptrGraphics->CreateImage(128,128);
  if (!mptrImage.IsOK()) {
    niError(_A("Can't create image."));
    return eFalse;
  }

  Image_SetDebugName(mptrImage,niFmt(_A("FNTDEFAULT")));
  Ptr<iBitmap2D> ptrBmp = mptrImage->GrabBitmap(eImageUsage_Target,sRecti::Null());
  niCheckIsOK(ptrBmp,eFalse);

  ptrBmp->Clear();
  for (tU32 c = 0; c < 256; ++c) {
    _Draw8BitFontChar(_DefaultFontData, ptrBmp, (c&0xF)<<3, (c>>4)<<3, c);
  }

  SetFiltering(eFalse);
  mStates.SetResolution(8);
  SetSizeAndResolution(sVec2f::Zero(),8,1);

  if (!HStringIsEmpty(ahspName)) {
    mhspName = ahspName;
  }

  ptrGraphics->_RegisterBaseFont(this);
  return eTrue;
}

///////////////////////////////////////////////
cFont::~cFont()
{
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  if (ptrGraphics.IsOK() && !mptrParentFont.IsOK()) {
    ptrGraphics->_UnregisterBaseFont(this);
  }
}

///////////////////////////////////////////////
void __stdcall cFont::Invalidate() {
}

///////////////////////////////////////////////
tBool cFont::IsOK() const {
  return mptrTTF.IsOK() || mptrImage.IsOK();
}

///////////////////////////////////////////////
iHString* __stdcall cFont::GetFilePath() const {
  return mhspFilePath;
}
iHString* __stdcall cFont::GetName() const {
  return mhspName;
}
iHString* __stdcall cFont::GetFamilyName() const {
  return mhspFamilyName;
}
iHString* __stdcall cFont::GetStyleName() const {
  return mhspStyleName;
}

///////////////////////////////////////////////
tBool __stdcall cFont::GetIsInstance() const {
  return mptrParentFont.IsOK();
}
tBool __stdcall cFont::GetIsFixedResolution() const {
  return mptrTTF.IsOK() ? eFalse : eTrue;
}
tBool __stdcall cFont::GetHasKerning() const {
  return mptrTTF.IsOK() ? mptrTTF->GetHasKerning() :eFalse;
}
tU32 __stdcall cFont::GetMaxResolution() const {
  return mptrTTF.IsOK() ? mptrTTF->GetMaxCharSize() : mptrImage->GetWidth()/16;
}

///////////////////////////////////////////////
tU32 cFont::GetTextureWidth() const {
  return mptrTTF.IsOK() ? mptrTTF->GetPageSize() : mptrImage->GetWidth();
}
tU32 cFont::GetTextureHeight() const {
  return mptrTTF.IsOK() ? mptrTTF->GetPageSize() : mptrImage->GetHeight();
}

///////////////////////////////////////////////
iFont* cFont::CreateFontInstance(iMaterial* apMaterial) {
  niCheck(this->IsOK(),NULL);
  niCheck(mStates.mptrMaterial.IsOK(),NULL);
  QPtr<cGraphics> ptrGraphics = mpwGraphics;
  niCheck(ptrGraphics.IsOK(),NULL);

  Ptr<cFont> ptrNewFont = niNew cFont(ptrGraphics);
  ptrNewFont->mptrParentFont = this;
  ptrNewFont->mhspName = this->mhspName;
  ptrNewFont->mhspFamilyName = this->mhspFamilyName;
  ptrNewFont->mhspStyleName = this->mhspStyleName;
  ptrNewFont->mhspFilePath = this->mhspFilePath;
  ptrNewFont->mhspFileName = this->mhspFileName;
  ptrNewFont->mptrImage = this->mptrImage;
  ptrNewFont->mptrTTF = this->mptrTTF;
  ptrNewFont->mStates = this->mStates;

  if (niIsOK(apMaterial)) {
    ptrNewFont->mStates.mptrMaterial = apMaterial;
    ptrNewFont->mStates.mbMaterialInstanced = eTrue;
  }
  else {
    if (ptrNewFont->mStates.mbMaterialInstanced) {
      ptrNewFont->mStates.mptrMaterial = ptrNewFont->mStates.mptrMaterial;
      ptrNewFont->mStates.mbMaterialInstanced = eTrue;
    }
    else {
      ptrNewFont->mStates.mptrMaterial = ptrNewFont->mStates.mptrMaterial->Clone();
      ptrNewFont->mStates.mbMaterialInstanced = eFalse;
    }
    ptrNewFont->mStates.mptrMaterial->SetChannelTexture(eMaterialChannel_Base,NULL);
  }
  return ptrNewFont.GetRawAndSetNull();
}

///////////////////////////////////////////////
void cFont::SetTabSize(tU32 ulNumChar) {
  mStates.mfTabSize = (tF32)ulNumChar;
}
tU32 cFont::GetTabSize() const {
  return (tU32)mStates.mfTabSize;
}

///////////////////////////////////////////////
void cFont::SetFiltering(tBool abFiltering)
{
  mStates.mbFiltering = abFiltering;
}

///////////////////////////////////////////////
tBool cFont::GetFiltering() const
{
  return mStates.mbFiltering;
}

///////////////////////////////////////////////
void cFont::SetDistanceField(tBool abDistanceField)
{
  mStates.mbDistanceField = abDistanceField;
}

///////////////////////////////////////////////
tBool cFont::GetDistanceField() const
{
  return mStates.mbDistanceField;
}

///////////////////////////////////////////////
void cFont::SetColor(tU32 anColor)
{
  mStates.mnColor = anColor;
}

///////////////////////////////////////////////
tU32 cFont::GetColor() const
{
  return mStates.mnColor;
}

///////////////////////////////////////////////
void __stdcall cFont::SetBlendMode(eBlendMode aMode) {
  mStates.mBlendMode = aMode;
}

///////////////////////////////////////////////
eBlendMode __stdcall cFont::GetBlendMode() const {
  return mStates.mBlendMode;
}

///////////////////////////////////////////////
void __stdcall cFont::SetSizeAndResolution(const sVec2f& avSize, const tU32 anResolution, const tF32 afContentsScale) {
  if (!this->GetIsFixedResolution()) {
    niAssert(niIsOK(mptrTTF));
    mStates.SetResolution(ni::Clamp((tU32)((tF32)anResolution*afContentsScale),2,GetMaxResolution()));
  }

  // Set the size
  if (avSize == sVec2f::Zero()) {
    mStates.mvSize = Vec2f(mStates.mnResolution,mStates.mnResolution) / afContentsScale;
  }
  else {
    mStates.mvSize = avSize;
  }

  mStates.mfYSign = ni::Sign(mStates.mvSize.y);
  // preserve +, turn - into + (-1*-1)
  mStates.mvSize.x *= ni::Sign(mStates.mvSize.x);
  // preserve +, turn - into + (-1*-1)
  mStates.mvSize.y *= mStates.mfYSign;
}

sVec2f __stdcall cFont::GetSize() const {
  return Vec2f(mStates.GetWidth(), mStates.GetHeight()*mStates.mfYSign);
}
tU32 __stdcall cFont::GetResolution() const {
  return mStates.mnResolution;
}

///////////////////////////////////////////////
float cFont::GetCharWidth(tU32 c) const {
  if (c == 0) {
    return 0;
  }
  return GetCharRectWidth(GetCharTexCoo(' '));
}
float cFont::GetCharHeight(tU32 c) const
{
  if (c == 0) {
    return 0;
  }
  return GetCharRectHeight(GetCharTexCoo(' '));
}

///////////////////////////////////////////////
tF32 __stdcall cFont::GetMaxCharWidth() const {
  return mStates.GetWidth();
}
tF32 __stdcall cFont::GetMaxCharHeight() const {
  return mStates.GetHeight();
}

///////////////////////////////////////////////
tF32 __stdcall cFont::GetAverageCharWidth() const {
  return GetCharWidth('X');
}
tF32 __stdcall cFont::GetAverageCharHeight() const {
  return GetCharHeight('X');
}

///////////////////////////////////////////////
tF32 __stdcall cFont::GetAscent() const {
  if (!mptrTTF.IsOK()) {
    return GetHeight();
  }
  return mptrTTF->GetAscender(mStates.GetHeight());
}
tF32 __stdcall cFont::GetDescent() const {
  if (!mptrTTF.IsOK()) {
    return 0;
  }
  return mptrTTF->GetDescender(mStates.GetHeight());
}
tF32 __stdcall cFont::GetHeight() const {
  return mStates.GetHeight();
}

///////////////////////////////////////////////
tU32 __stdcall cFont::GetFirstChar() const {
  return 0;
}
tU32 __stdcall cFont::GetLastChar() const {
  // Bitmap font is max 256 characters atm (16x16 grid)...
  return mptrTTF.IsOK() ? 0xFFFF : 0xFF;
}

///////////////////////////////////////////////
void __stdcall cFont::SetLineSpacing(const tF32 afSpacing) {
  mStates.mfLineSpacing = afSpacing;
}
tF32 __stdcall cFont::GetLineSpacing() const {
  return mStates.mfLineSpacing;
}
tF32 __stdcall cFont::GetLineHeight() const {
  return mStates.GetHeight() * mStates.mfLineSpacing;
}

///////////////////////////////////////////////
sRectf __stdcall cFont::ComputeTextSize(const sRectf& aRect, const achar* aaszText, tFontFormatFlags aFormat) {
  astl::vector<sTextLineMetric> tm;
  sRectf bb = TextLayout_Compute(
    this, aRect,
    aaszText, ni::StrLen(aaszText),
    &aFormat, tm);
  return bb;
}

///////////////////////////////////////////////
sRectf __stdcall cFont::DrawText(ni::iCanvas* apCanvas, const sRectf& aRect, tF32 afZ, const achar* aaszText, tFontFormatFlags aFlags) {
  const sRectf baseClipRect = aRect;
  astl::vector<sTextLineMetric> tm;
  sRectf bb = TextLayout_Compute(
    this, aRect,
    aaszText, ni::StrLen(aaszText),
    &aFlags, tm);
  if (tm.empty()) {
    return bb;
  }

  const tI32 numPasses = niFlagIs(aFlags,eFontFormatFlags_Border) ? 9 : 1;
  niLoop(pass,numPasses) {
    sRectf clipRect = baseClipRect;
    tF32 offsetX = 0, offsetY = 0;
    tU32 color = this->GetColor();
    if (pass == (numPasses-1)) {
      // last pass doesnt have any offset
    }
    else {
      static const sVec2f offsets[8] = {
        { -1, -1 },
        {  1, -1 },
        {  1,  1 },
        { -1,  1 },
        { -1,  0 },
        {  1,  0 },
        {  0,  1 },
        {  0, -1 },
      };
      color = ULColorBuild(0,0,0,ULColorGetA(color));
      if (numPasses <= 5) {
        offsetX = offsets[pass+4].x;
        offsetY = offsets[pass+4].y;
      }
      else {
        offsetX = offsets[pass].x;
        offsetY = offsets[pass].y;
      }
      clipRect.Move(Vec2f(offsetX, offsetY));
    }
    TextLayout_PushToCanvas(
      tm.data(), (tU32)tm.size(),
      apCanvas, this,
      color, Vec3f(offsetX, offsetY, afZ),
      niFlagIs(aFlags,eFontFormatFlags_ClipH) ? &clipRect : NULL,
      niFlagIs(aFlags,eFontFormatFlags_ClipV) ? &clipRect : NULL);
  }

  if (niFlagIs(aFlags,eFontFormatFlags_ClipH)) {
    bb.x = ni::Min(bb.x,baseClipRect.x);
    bb.z = ni::Min(bb.z,baseClipRect.z);
  }
  if (niFlagIs(aFlags,eFontFormatFlags_ClipV)) {
    bb.y = ni::Min(bb.y,baseClipRect.y);
    bb.w = ni::Min(bb.w,baseClipRect.w);
  }

  // for debugging, draw bounding boxes over each text segments
#if 0
  niLoop(i,tm.size()) {
    apCanvas->BlitRect(tm[i].rect, ~0);
  }
  apCanvas->BlitRect(bb, 0xFF00FFFF);
#endif

  return bb;
}

///////////////////////////////////////////////
tBool __stdcall cFont::CacheText(const achar* aaszText)
{
  if (mptrTTF.IsOK()) {
    StrCharIt itText(aaszText);
    while (!itText.is_end())  {
      const tU32 ch = itText.next();
      mptrTTF->RenderCodepoint(ch,mStates.mnResolution,mStates.mbDistanceField);
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cFont::CacheRange(tU32 anFirst, tU32 anLast)
{
  if (mptrTTF.IsOK()) {
    for (tU32 i = anFirst; i <= anLast; ++i) {
      mptrTTF->RenderCodepoint(i,mStates.mnResolution,mStates.mbDistanceField);
    }
  }
  return eTrue;
}

///////////////////////////////////////////////
tBool cFont::UpdateMaterial(tBool abUpdateMaterialStates) {
  iMaterial* pMat = mStates.mptrMaterial;
  if (!pMat)
    return eFalse;

  pMat->SetChannelTexture(eMaterialChannel_Base,GetTexture());
  if (abUpdateMaterialStates) {
    pMat->SetBlendMode(mStates.mBlendMode);
    switch (mStates.mBlendMode) {
      case eBlendMode_NoBlending:
        // Solid drawing
        // pMat->SetFlags(pMat->GetFlags()&(~eMaterialFlags_Translucent));
        // pMat->SetChannelColor(eMaterialChannel_Opacity,Vec4<tF32>(1,1,1,0.3f));
        break;
      default:
        // Translucent drawing
        // pMat->SetFlags(pMat->GetFlags()|eMaterialFlags_Translucent);
        // pMat->SetChannelColor(eMaterialChannel_Opacity,Vec4<tF32>(1,1,1,1.0f/255.0f));
        break;
    }
    pMat->SetChannelSamplerStates(
        eMaterialChannel_Base,
        mStates.mbFiltering ?
        eCompiledStates_SS_SharpClamp :
        eCompiledStates_SS_PointClamp);
  }
  return eTrue;
}

///////////////////////////////////////////////
const sFontGlyph* cFont::GetGlyph(tU32 anChar, sFontGlyph* apTmpStorage) const {
  if (mptrTTF.IsOK()) {
    return mptrTTF->RenderCodepoint(anChar,mStates.mnResolution,mStates.mbDistanceField);
  }
  else {
    // 16x16 grid of glyphs, max 256 characters
    if (anChar >= 0xFF)
      anChar = '?';
    apTmpStorage->SetGlyphInBitmap(anChar, GetSize());
    apTmpStorage->img = mptrImage;
    return apTmpStorage;
  }
}

///////////////////////////////////////////////
tF32 cFont::GetCharRectWidth(const sRectf& rectTexCoo) const
{
  return mStates.GetWidth()*
      rectTexCoo.GetWidth()*GetTextureWidth()*
      mStates.mfInvResolution;
}

///////////////////////////////////////////////
tF32 cFont::GetCharRectHeight(const sRectf& rectTexCoo) const
{
  return mStates.GetHeight()*
      rectTexCoo.GetHeight()*GetTextureHeight()*
      mStates.mfInvResolution;
}

///////////////////////////////////////////////
sRectf __stdcall cFont::GetCharTexCoo(tU32 c) const
{
  sFontGlyph tg;
  return GetGlyph(c,&tg)->texCoo;
}

///////////////////////////////////////////////
iTexture* __stdcall cFont::GetTexture() {
  if (!mptrImage.IsOK()) return NULL;
  iTexture* pTex = mptrImage->GrabTexture(eImageUsage_Source,sRecti::Null());
  mStates.mptrMaterial->SetChannelTexture(eMaterialChannel_Base,pTex);
  return pTex;
}

///////////////////////////////////////////////
iMaterial* __stdcall cFont::GetMaterial() const {
  return mStates.mptrMaterial;
}

///////////////////////////////////////////////
tF32 __stdcall cFont::GetKerning(tU32 anPrevChar, tU32 anChar) const{
  if (anPrevChar && mptrTTF.IsOK()) {
    const tF32 scale = ni::FDiv(mStates.GetWidth(),(tF32)mStates.mnResolution);
    return (tF32)mptrTTF->GetKerning(mStates.mnResolution,anPrevChar,anChar) * scale;
  }
  else {
    return 0;
  }
}

///////////////////////////////////////////////
tF32 __stdcall cFont::GetAdvance(tU32 anChar) const {
  if (mptrTTF.IsOK()) {
    const tF32 scale = ni::FDiv(mStates.GetWidth(),(tF32)mStates.mnResolution);
    return mptrTTF->GetAdvanceCodepoint(mStates.mnResolution, mStates.mbDistanceField, anChar) * scale;
  }
  else {
    return mStates.GetWidth();
  }
}

///////////////////////////////////////////////
iTexture* __stdcall cFont::GetCharTexture(tU32 anChar) const {
  sFontGlyph tg;
  const sFontGlyph* g = GetGlyph(anChar,&tg);
  if (!g->img.IsOK())
    return NULL;
  return g->img->GrabTexture(eImageUsage_Source,sRecti::Null());
}
iBitmap2D* __stdcall cFont::GetCharBitmap(tU32 anChar) const {
  sFontGlyph tg;
  const sFontGlyph* g = GetGlyph(anChar,&tg);
  if (!g->img.IsOK())
    return NULL;
  return g->img->GrabBitmap(eImageUsage_Source,sRecti::Null());
}

///////////////////////////////////////////////
sVec2i __stdcall cFont::BlitCharEx(iBitmap2D* apDestBmp, tI32 anX, tI32 anY, tI32 anW, tI32 anH, tU32 anChar, eBlendMode aBlendMode) {
  niCheckIsOK(apDestBmp,sVec2i::Zero());

  sFontGlyph tg;
  const sFontGlyph* g = GetGlyph(anChar,&tg);
  if (!g->img.IsOK())
    return sVec2i::Zero();

  Ptr<iBitmap2D> pSrcBmp = g->img->GrabBitmap(eImageUsage_Source,sRecti::Null());
  niCheckIsOK(pSrcBmp,sVec2i::Zero());

  anX += g->lead.x;
  anY += g->lead.y;
  const sRecti rectSrc {
    (tI32)(g->texCoo.x * (float)pSrcBmp->GetWidth()),
    (tI32)(g->texCoo.y * (float)pSrcBmp->GetHeight()),
    (tI32)(g->texCoo.z * (float)pSrcBmp->GetWidth()),
    (tI32)(g->texCoo.w * (float)pSrcBmp->GetHeight())
  };
  sColor4f colFront = {ULColorGetRf(mStates.mnColor),
                       ULColorGetGf(mStates.mnColor),
                       ULColorGetBf(mStates.mnColor),
                       ULColorGetAf(mStates.mnColor)};
  // sColor4f colBack = {0,0,0,0};

  const sVec2i charSize = {
    anW<=0 ? rectSrc.GetWidth() : anW,
    anH<=0 ? rectSrc.GetHeight() : anH
  };
  if (anChar > 32) {
    if (charSize.x > 0 && charSize.y > 0 && charSize != rectSrc.GetSize()) {
      if (aBlendMode != eBlendMode_NoBlending) {
        apDestBmp->BlitAlphaStretch(pSrcBmp,
                                    rectSrc.Left(),
                                    rectSrc.Top(),
                                    anX,
                                    anY,
                                    rectSrc.GetWidth(),
                                    rectSrc.GetHeight(),
                                    charSize.x,
                                    charSize.y,
                                    colFront,
                                    sColor4f::Zero(),
                                    eBlendMode_Translucent);
      }
      else {
        apDestBmp->BlitStretch(pSrcBmp,
                               rectSrc.Left(),
                               rectSrc.Top(),
                               anX,
                               anY,
                               rectSrc.GetWidth(),
                               rectSrc.GetHeight(),
                               charSize.x,
                               charSize.y);
      }
    }
    else {
      if (aBlendMode != eBlendMode_NoBlending) {
        apDestBmp->BlitAlpha(pSrcBmp,
                             rectSrc.Left(),
                             rectSrc.Top(),
                             anX,
                             anY,
                             rectSrc.GetWidth(),
                             rectSrc.GetHeight(),
                             ePixelFormatBlit_Normal,
                             colFront,
                             sColor4f::Zero(),
                             eBlendMode_Translucent);
      }
      else {
        apDestBmp->Blit(pSrcBmp,
                        rectSrc.Left(),
                        rectSrc.Top(),
                        anX,
                        anY,
                        rectSrc.GetWidth(),
                        rectSrc.GetHeight(),
                        ePixelFormatBlit_Normal);
      }
    }
  }

  return charSize;
}

///////////////////////////////////////////////
tBool __stdcall cFont::ClearCache() {
  if (mptrTTF.IsOK()) {
    mptrTTF->ClearCache();
    if (mStates.mptrMaterial.IsOK()) {
      mStates.mptrMaterial->SetChannelTexture(eMaterialChannel_Base,NULL);
    }
    QPtr<cGraphics> ptrGraphics = mpwGraphics;
    if (ptrGraphics.IsOK()) {
      ++ptrGraphics->mnFontCacheID;
    }
    return eTrue;
  }
  return eFalse;
}
tIntPtr __stdcall cFont::GetCacheID() const {
  if (mptrTTF.IsOK()) {
    return mptrTTF->GetCacheID();
  }
  return 0;
}

///////////////////////////////////////////////
Ptr<tU32CMap> __stdcall cFont::EnumGlyphs() const {
  if (mptrTTF.IsOK()) {
    return mptrTTF->EnumGlyphs();
  }
  else {
    return NULL;
  }
}

///////////////////////////////////////////////
tU32 __stdcall cFont::GetGlyphIndexFromName(const achar* aaszName) const {
  FT_Face face = mptrTTF.IsOK() ? mptrTTF->GetFTFace() : NULL;
  if (!face && FT_HAS_GLYPH_NAMES(face))
    return 0;

  FT_UInt gindex = FT_Get_Name_Index(face, (char*)aaszName);
  return gindex;
}

///////////////////////////////////////////////
tU32 __stdcall cFont::GetGlyphIndexFromCodepoint(const tU32 anCodepoint) const {
  FT_Face face = mptrTTF.IsOK() ? mptrTTF->GetFTFace() : NULL;
  if (!face)
    return 0;

  FT_UInt gindex = FT_Get_Char_Index(face, anCodepoint);
  return gindex;
}

///////////////////////////////////////////////
cString __stdcall cFont::GetGlyphName(const tU32 anGlyphIndex) const {
  FT_Face face = mptrTTF.IsOK() ? mptrTTF->GetFTFace() : NULL;
  if (!face && FT_HAS_GLYPH_NAMES(face))
    return "";

  char buffer[2048] = {0};
  FT_Error error = FT_Get_Glyph_Name(face, anGlyphIndex, buffer, sizeof(buffer));
  if (!error) {
    return buffer;
  }
  else {
    return "";
  }
}

///////////////////////////////////////////////
sVec2f __stdcall cFont::GetGlyphPath(iVGPath* apPath, tU32 anGlyphIndex, const sVec2f& avOffset, const tF32 afScale) const
{
  if (mptrTTF.IsOK()) {
    return mptrTTF->GetGlyphPath(apPath, anGlyphIndex, mStates.mnResolution, avOffset, afScale);
  }
  else {
    return sVec2f::Zero();
  }
}

///////////////////////////////////////////////
tU32 __stdcall cFont::GetGlyphCodePointFromName(const achar* aaszName) const {
  const tU32 glyphIndex = this->GetGlyphIndexFromName(aaszName);
  if (glyphIndex) {
    const Ptr<tU32CMap> glyphs = this->EnumGlyphs();
    if (glyphs.IsOK()) {
      tU32CMap::const_iterator it = glyphs->find(glyphIndex);
      if (it != glyphs->end()) {
        return it->second;
      }
    }
  }
  return 0;
}
