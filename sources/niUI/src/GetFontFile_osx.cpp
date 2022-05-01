#include "stdafx.h"

#if defined niOSX

#include "GetFontFile.h"
#include <niLang/Utils/FileEnum.h>

/*
  See: http://support.apple.com/kb/HT1642

  Fonts included with Mac OS X 10.5 Leopard are installed in one of these folders:

  /Library/Fonts
  /System/Library/Fonts
 */

///////////////////////////////////////////////
tBool GetAllFontFiles(tFontFileLst& aLst) {
  const cString fontDir = GetFontsDirectory() + _ASTR("*.ttf");
  // niDebugFmt((_A("GATHERING FONTS: %s"),fontDir.Chars()));
  ni::FindFile findFiles;
  if (findFiles.First(fontDir.Chars())) {
    do {
      sFontFile item;
      cPath path(findFiles.FileName());
      item.strFile = path.GetFile();
      path.SetExtension(NULL);
      item.strDisp = path.GetFile();
      aLst.push_back(item);
      // niDebugFmt((_A("GOT FONT: %s, %s"),item.strFile.Chars(),item.strDisp.Chars()));
    } while (findFiles.Next());
  }
  return !aLst.empty();
}

///////////////////////////////////////////////
cString GetFontsDirectory() {
  // TODO: This works, but really the bulk of the fonts on the macOS aren't
  //       there, so its a bit garbage... I think we need to use some CF API
  //       to get the list of fonts which means modifying this API a fair bit.
  return "/System/Library/Fonts/";
}

#endif
