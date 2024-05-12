#ifndef __FREETYPE_33032706_H__
#define __FREETYPE_33032706_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang.h>

#define FT2_BUILD_LIBRARY
#include <ft2build.h>
#include FT_FREETYPE_H

// Mostly working, but the text is too low, the baseline has never been
// considered accurately before so our ad-hoc notion of center mess up the
// "right way".
// #define FONT_TTF_USE_EXACT_RATIO

#define FT_FAILED(x)  ((x) != 0)
#define FT_SUCCEEDED(x) ((x) == 0)

#define FT_ERR_OK   0
#define FT_ERR_ERROR  -1

ni::cString MyFT_GetErrorString(int error);
niExportFunc(struct FT_FaceRec_*) MyFT_Face_Open(struct FT_LibraryRec_* aLib, ni::iFile* apFile);

namespace ni {

struct sMyFTLibrary : public ni::ImplRC<ni::iUnknown>
{
  sMyFTLibrary();
  ~sMyFTLibrary();
  struct FT_LibraryRec_* mLibrary;
};
typedef ni::Ptr<sMyFTLibrary> tMyFTLibraryPtr;

} // end of namespace ni

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __FREETYPE_33032706_H__
