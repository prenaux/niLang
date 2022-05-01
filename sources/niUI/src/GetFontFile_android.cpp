#include "stdafx.h"

#if defined niAndroid || defined niEmbedded || defined niLinuxDesktop
#include "GetFontFile.h"
#include <niLang/Utils/FileEnum.h>

///////////////////////////////////////////////
tBool GetAllFontFiles(tFontFileLst& aLst) {
  const cString fontDir = GetFontsDirectory() + _ASTR("*.ttf");
  niDebugFmt((_A("Looking for Android fonts in '%s'"),fontDir.Chars()));
  ni::FindFile findFiles;
  if (findFiles.First(fontDir.Chars())) {
    do {
      sFontFile item;
      cPath path(findFiles.FileName());
      item.strFile = path.GetFile();
      path.SetExtension(NULL);
      item.strDisp = path.GetFile();
      aLst.push_back(item);
      niDebugFmt((_A("Found Font: %s, %s"),item.strFile.Chars(),item.strDisp.Chars()));
    } while (findFiles.Next());
  }
  return !aLst.empty();
}

///////////////////////////////////////////////
cString GetFontsDirectory() {
#if defined niAndroid
  return _A("/system/fonts/");
#elif defined niMSVC
  return niFmt(_A("%s/toolsets/android/device/system/fonts/"),ni::GetLang()->GetEnv(_A("HAM_HOME")));
#else
  cString d = ni::GetLang()->GetProperty("ni.dirs.data");
  d << "base/fonts/";
  return d;
#endif
}

#endif
