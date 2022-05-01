// GetFontFile.h
//
// Copyright (C) 2001 Hans Dietrich
//
// This software is released into the public domain.
// You are free to use it in any way you like.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef GETFONTFILE_H
#define GETFONTFILE_H

struct sFontFile {
  cString strFile;
  cString strDisp;
};
typedef astl::list<sFontFile> tFontFileLst;
typedef tFontFileLst::iterator  tFontFileLstIt;
typedef tFontFileLst::const_iterator  tFontFileLstCIt;

ni::tBool GetAllFontFiles(tFontFileLst& aLst);
ni::cString GetFontsDirectory();

#endif //GETFONTFILE_H
