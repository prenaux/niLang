// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include <niLang/Utils/Trace.h>
#include "Graphics.h"
#include "Font.h"
#include "FontTTF.h"
#include "Image.h"
#include "TextMetric.h"
#include <niLang/Utils/UnitSnap.h>
#include "API/niUI/IVGPath.h"
#include "freetype/ftglyph.h"
#include <niLang/STL/pair.h>

using namespace ni;

niDeclareModuleTrace_(niUI,TraceFontErrors);
#define _TraceFontErrors(FMT) niModuleTrace_(niUI,TraceFontErrors,FMT);

//////////////////////////////////////////////////////////////////////////////////////////////
static sPropertyBool _dumpGlyphs("niUI.FontTTF.DumpGlyphs",eFalse);

#define TTF_KERNING
// #define TTF_PREMULALPHA
#define TTF_PAGE_SIZE 512
#define TTF_GLYPH_PADDING 1
#define TTF_USE_MIPMAPS
#define TTF_COMPUTE_MIPMAPS_PER_PAGE
// #define TTF_SELECT_CHARMAP

#define TTFGlyphFlags_DistanceField (1ULL<<63ULL)

static __forceinline tGlyphHash TTFGlyphIndexHash(tU64 GLYPHINDEX, tU64 SIZE, tBool DISTANCEFIELD) {
  return ((GLYPHINDEX)?(((DISTANCEFIELD)?TTFGlyphFlags_DistanceField:0ULL)|((SIZE)<<32ULL)|(GLYPHINDEX)):0ULL);
}

// this is a good compromise between quality and generation time
const int SDF_scaler = 4;
static tU8 get_SDF_radial(
  const tU8* fontmap,
  int w, int h,
  int x, int y,
  int max_radius);

static cFontTTF* _GetFallback(const cFontTTF* apThis, cGraphics* apGraphics) {
  cFont* pFallback = apGraphics->_GetFallbackFont();
  if ((pFallback) && (pFallback->mptrTTF.IsOK()) && (pFallback->mptrTTF.ptr() != apThis)) {
    return pFallback->mptrTTF;
  }
  return NULL;
}

static cString GetTTFFaceName(FT_Face pFace) {
  cString ret = cString(pFace->family_name);
  if (strcmp(pFace->style_name,"Regular") != 0) {
    ret += _A(" ");
    ret += cString(pFace->style_name);
  }
  return ret;
}

static cString GetTTFFileFontName(sMyFTLibrary *apLibrary, iFile* apFile) {
  FT_Face pFace = MyFT_Face_Open(apLibrary->mLibrary,apFile);
  if (pFace == NULL) {
    niError(niFmt(_A("Can't load font from file '%s'."),apFile->GetSourcePath()));
    return AZEROSTR;
  }
  cString ret = GetTTFFaceName(pFace);
  FT_Done_Face(pFace);
  return ret;
}

static void GetTTFFontVMetrics(FT_Face aFace, int* height, int *ascent, int *descent) {
  *height = aFace->height;
  *ascent = aFace->ascender;
  *descent = aFace->descender;
}
static float GetTTFPixelHeightScale(FT_Face aFont, float size) {
  return size / (aFont->ascender - aFont->descender);
}

static tBool SetTTFPixelSize(FT_Face aFace, tU32 anPixelSize, tU32* apLastKerningPixelSize) {
  if (apLastKerningPixelSize && (anPixelSize == *apLastKerningPixelSize)) {
    return eTrue;
  }
  FT_Error ftError = FT_Set_Pixel_Sizes(aFace, 0, (FT_UInt)((float)anPixelSize * (float)aFace->units_per_EM / (float)(aFace->ascender - aFace->descender)));
  if (ftError)
    return eFalse;
  return eTrue;
}

static Ptr<iBitmap2D> RenderGlyphIndexBitmap(iGraphics* apGraphics, FT_Face aFace,
                                             tU32 glyphIndex, tU32 anPixelSize,
                                             float* advance,
                                             sVec2f* lead,
                                             sVec2i* dim,
                                             tF32 ascent)
{
  if (!SetTTFPixelSize(aFace, anPixelSize, NULL)) {
    return 0;
  }

  // Load the font with hinting. We don't allow autohinting as its generally
  // of fairly poor quality, blurry text looks better than bad spacing...
  FT_Error ftError = FT_Load_Glyph(aFace, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT);
  if (ftError) {
    // try without any hinting...
    ftError = FT_Load_Glyph(aFace, glyphIndex, FT_LOAD_RENDER | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
    if (ftError) {
      _TraceFontErrors(("Font '%s', can't load glyph '%d'. FTERROR: %s.",
                        GetTTFFaceName(aFace), glyphIndex,
                        MyFT_GetErrorString(ftError).Chars()));
    }
    return 0;
  }

  FT_GlyphSlot ftGlyph = aFace->glyph;
  lead->x = ftGlyph->bitmap_left;
  lead->y = -ftGlyph->bitmap_top + ascent - 1;
  dim->x = ftGlyph->bitmap.width;
  dim->y = ftGlyph->bitmap.rows;
  *advance = (float)ftGlyph->advance.x / 64.0f;
  if (*advance <= 1) {
    *advance = dim->x + 1;
  }
  // niDebugFmt(("... BuildTTFGlyphBitmap: %s, c: %c (%d), adv: %g, dim: %s, lead: %s",
              // GetTTFFaceName(aFace),
              // anChar, anChar,
              // *advance,
              // *dim,
              // *lead));

  Ptr<iBitmap2D> bmp;
  if (ftGlyph->bitmap.width > 0) {
    bmp = apGraphics->CreateBitmap2D(ftGlyph->bitmap.width, ftGlyph->bitmap.rows, "A8");
    memcpy(bmp->GetData(), ftGlyph->bitmap.buffer, ftGlyph->bitmap.width*ftGlyph->bitmap.rows);
  }

  return bmp;
}

#ifdef TTF_KERNING
static int GetTTFGlyphKernAdvance(FT_Face aFace, tU32 anPixelSize, tU32* apLastKerningPixelSize, int glyph1, int glyph2)
{
  if (!SetTTFPixelSize(aFace,anPixelSize,apLastKerningPixelSize)) {
    return 0;
  }
  FT_Vector ftKerning;
  int prev = FT_Get_Char_Index(aFace, glyph1);
  int curr = FT_Get_Char_Index(aFace, glyph2);
  FT_Get_Kerning(aFace, prev, curr, FT_KERNING_DEFAULT, &ftKerning);
  return (int)((ftKerning.x + 32) >> 6);  // Round up and convert to integer
}
#endif

static __forceinline agg_real int26p6_to_dbl(int p) {
  return (agg_real)(p / 64.0);
}
static __forceinline int dbl_to_int26p6(agg_real p) {
  return int(p * 64.0 + 0.5);
}

static bool FTOutlineToPath(const FT_Outline& outline,
                            bool flip_y,
                            const agg::trans_affine& mtx,
                            iVGPath* path)
{
  typedef tF32 value_type;

  FT_Vector   v_last;
  FT_Vector   v_control;
  FT_Vector   v_start;
  agg_real x1, y1, x2, y2, x3, y3;

  FT_Vector*  point;
  FT_Vector*  limit;
  char*       tags;

  int   n;         // index of contour in outline
  int   first;     // index of first point in contour
  char  tag;       // current point's state

  first = 0;

  for(n = 0; n < outline.n_contours; n++)
  {
    int  last;  // index of last point in contour

    last  = outline.contours[n];
    limit = outline.points + last;

    v_start = outline.points[first];
    v_last  = outline.points[last];

    v_control = v_start;

    point = outline.points + first;
    tags  = outline.tags  + first;
    tag   = FT_CURVE_TAG(tags[0]);

    // A contour cannot start with a cubic control point!
    if(tag == FT_CURVE_TAG_CUBIC) return false;

    // check first point to determine origin
    if( tag == FT_CURVE_TAG_CONIC)
    {
      // first point is conic control.  Yes, this happens.
      if(FT_CURVE_TAG(outline.tags[last]) == FT_CURVE_TAG_ON)
      {
        // start at last point if it is on the curve
        v_start = v_last;
        limit--;
      }
      else
      {
        // if both first and last points are conic,
        // start at their middle and record its position
        // for closure
        v_start.x = (v_start.x + v_last.x) / 2;
        v_start.y = (v_start.y + v_last.y) / 2;

        v_last = v_start;
      }
      point--;
      tags--;
    }

    x1 = int26p6_to_dbl(v_start.x);
    y1 = int26p6_to_dbl(v_start.y);
    if(flip_y) y1 = -y1;
    mtx.transform(&x1, &y1);
    path->MoveTo(value_type(dbl_to_int26p6(x1)),
                 value_type(dbl_to_int26p6(y1)));

    while(point < limit)
    {
      point++;
      tags++;

      tag = FT_CURVE_TAG(tags[0]);
      switch(tag)
      {
      case FT_CURVE_TAG_ON:  // emit a single line_to
        {
          x1 = int26p6_to_dbl(point->x);
          y1 = int26p6_to_dbl(point->y);
          if(flip_y) y1 = -y1;
          mtx.transform(&x1, &y1);
          path->LineTo(value_type(dbl_to_int26p6(x1)),
                       value_type(dbl_to_int26p6(y1)));
          continue;
        }

      case FT_CURVE_TAG_CONIC:  // consume conic arcs
        {
          v_control.x = point->x;
          v_control.y = point->y;

       Do_Conic:
          if(point < limit)
          {
            FT_Vector vec;
            FT_Vector v_middle;

            point++;
            tags++;
            tag = FT_CURVE_TAG(tags[0]);

            vec.x = point->x;
            vec.y = point->y;

            if(tag == FT_CURVE_TAG_ON)
            {
              x1 = int26p6_to_dbl(v_control.x);
              y1 = int26p6_to_dbl(v_control.y);
              x2 = int26p6_to_dbl(vec.x);
              y2 = int26p6_to_dbl(vec.y);
              if(flip_y) { y1 = -y1; y2 = -y2; }
              mtx.transform(&x1, &y1);
              mtx.transform(&x2, &y2);
              path->Curve3(value_type(dbl_to_int26p6(x1)),
                           value_type(dbl_to_int26p6(y1)),
                           value_type(dbl_to_int26p6(x2)),
                           value_type(dbl_to_int26p6(y2)));
              continue;
            }

            if(tag != FT_CURVE_TAG_CONIC) return false;

            v_middle.x = (v_control.x + vec.x) / 2;
            v_middle.y = (v_control.y + vec.y) / 2;

            x1 = int26p6_to_dbl(v_control.x);
            y1 = int26p6_to_dbl(v_control.y);
            x2 = int26p6_to_dbl(v_middle.x);
            y2 = int26p6_to_dbl(v_middle.y);
            if(flip_y) { y1 = -y1; y2 = -y2; }
            mtx.transform(&x1, &y1);
            mtx.transform(&x2, &y2);
            path->Curve3(value_type(dbl_to_int26p6(x1)),
                         value_type(dbl_to_int26p6(y1)),
                         value_type(dbl_to_int26p6(x2)),
                         value_type(dbl_to_int26p6(y2)));
            v_control = vec;
            goto Do_Conic;
          }

          x1 = int26p6_to_dbl(v_control.x);
          y1 = int26p6_to_dbl(v_control.y);
          x2 = int26p6_to_dbl(v_start.x);
          y2 = int26p6_to_dbl(v_start.y);
          if(flip_y) { y1 = -y1; y2 = -y2; }
          mtx.transform(&x1, &y1);
          mtx.transform(&x2, &y2);
          path->Curve3(value_type(dbl_to_int26p6(x1)),
                       value_type(dbl_to_int26p6(y1)),
                       value_type(dbl_to_int26p6(x2)),
                       value_type(dbl_to_int26p6(y2)));
          goto Close;
        }

      default:  // FT_CURVE_TAG_CUBIC
        {
          FT_Vector vec1, vec2;

          if(point + 1 > limit || FT_CURVE_TAG(tags[1]) != FT_CURVE_TAG_CUBIC)
          {
            return false;
          }

          vec1.x = point[0].x;
          vec1.y = point[0].y;
          vec2.x = point[1].x;
          vec2.y = point[1].y;

          point += 2;
          tags  += 2;

          if(point <= limit)
          {
            FT_Vector vec;

            vec.x = point->x;
            vec.y = point->y;

            x1 = int26p6_to_dbl(vec1.x);
            y1 = int26p6_to_dbl(vec1.y);
            x2 = int26p6_to_dbl(vec2.x);
            y2 = int26p6_to_dbl(vec2.y);
            x3 = int26p6_to_dbl(vec.x);
            y3 = int26p6_to_dbl(vec.y);
            if(flip_y) { y1 = -y1; y2 = -y2; y3 = -y3; }
            mtx.transform(&x1, &y1);
            mtx.transform(&x2, &y2);
            mtx.transform(&x3, &y3);
            path->Curve4(value_type(dbl_to_int26p6(x1)),
                         value_type(dbl_to_int26p6(y1)),
                         value_type(dbl_to_int26p6(x2)),
                         value_type(dbl_to_int26p6(y2)),
                         value_type(dbl_to_int26p6(x3)),
                         value_type(dbl_to_int26p6(y3)));
            continue;
          }

          x1 = int26p6_to_dbl(vec1.x);
          y1 = int26p6_to_dbl(vec1.y);
          x2 = int26p6_to_dbl(vec2.x);
          y2 = int26p6_to_dbl(vec2.y);
          x3 = int26p6_to_dbl(v_start.x);
          y3 = int26p6_to_dbl(v_start.y);
          if(flip_y) { y1 = -y1; y2 = -y2; y3 = -y3; }
          mtx.transform(&x1, &y1);
          mtx.transform(&x2, &y2);
          mtx.transform(&x3, &y3);
          path->Curve4(value_type(dbl_to_int26p6(x1)),
                       value_type(dbl_to_int26p6(y1)),
                       value_type(dbl_to_int26p6(x2)),
                       value_type(dbl_to_int26p6(y2)),
                       value_type(dbl_to_int26p6(x3)),
                       value_type(dbl_to_int26p6(y3)));
          goto Close;
        }
      }
    }

    path->ClosePolygon();

 Close:
    first = last + 1;
  }

  return true;
}

//------------------------------------------------------------------------
static bool GetTTFGlyphPath(
  FT_Face face,
  unsigned glyph_index,
  tU32 anPixelSize,
  bool flip_y,
  const agg::trans_affine& mtx,
  ni::iVGPath* apPath)
{
  if (!SetTTFPixelSize(face, anPixelSize, NULL)) {
    niError(niFmt("Can't set pixel size '%d'.",(tI32)glyph_index));
    return false;
  }

  unsigned error = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_HINTING);
  if (error != 0) {
    niError(niFmt("Can't load glyph '%d': %d: %s.",(tI32)glyph_index,(tI32)error,MyFT_GetErrorString(error)));
    return false;
  }

  FT_Glyph glyph;
  error = FT_Get_Glyph(face->glyph, &glyph);
  if (error != 0) {
    niError(niFmt("Can't get glyph '%d': %d: %s.",(tI32)glyph_index,(tI32)error,MyFT_GetErrorString(error)));
    return false;
  }
  if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
    niError(niFmt("Glyph doesnt have an outline '%d'."));
    return false;
  }

  FT_OutlineGlyph glyphOutline = (FT_OutlineGlyph)glyph;
  if (niIsOK(apPath)) {
    FTOutlineToPath(glyphOutline->outline,flip_y,mtx,apPath);
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
cFontTTF::cFontTTF(cGraphics* apGraphics, sMyFTLibrary* apLibrary, iFile* apFile)
    : mnImageMapPageSize(
        ni::Min(TTF_PAGE_SIZE,apGraphics->GetDriverCaps(eGraphicsCaps_Texture2DMaxSize)))
{
  niAssert(niIsOK(apGraphics));

  mpGraphics = apGraphics;
  mptrFTLib = apLibrary;

  mFace = MyFT_Face_Open(mptrFTLib->mLibrary, apFile);
  if (!mFace) {
    niError(niFmt(_A("Can't load font from file '%s'."),apFile->GetSourcePath()));
    return;
  }

  // Store normalized line height. The real line height is got by multiplying
  // the lineh by font size.
  int height, ascent, descent;
  GetTTFFontVMetrics(mFace, &height, &ascent, &descent);
  mfNormalizedAscender = (float)ascent / (float)height;
  mfNormalizedDescender = (float)descent / (float)height;

  // Set a UCS2 charmap if there's one
  tBool foundUCS2CharMap = eFalse;
  for(int i = 0; i < mFace->num_charmaps; i++) {
    if (((mFace->charmaps[i]->platform_id == 0) && (mFace->charmaps[i]->encoding_id == 3)) ||
        ((mFace->charmaps[i]->platform_id == 3) && (mFace->charmaps[i]->encoding_id == 1)))
    {
      FT_Set_Charmap(mFace, mFace->charmaps[i]);
      foundUCS2CharMap = eTrue;
      break;
    }
  }

  niLog(Info,
        niFmt("Created TTF Font: '%s' (%s): isFixed: %s, hasKerning: %s, ucs2CharMap: %s, height: %d, ascent: %d, descent: %d.",
              this->GetName(), apFile->GetSourcePath(),
              this->GetIsFixedWidth() ? "yes" : "no",
              this->GetHasKerning() ? "yes" : "no",
              foundUCS2CharMap ? "yes" : "no",
              (tInt)height, (tInt)ascent, (tInt)descent));

#ifdef TTF_SELECT_CHARMAP
  FT_CharMap  foundCharmap = 0;
  for (int n = 0; n < mFace->num_charmaps; ++n) {
    FT_CharMap charmap = mFace->charmaps[n];
    const tBool selected = (charmap->encoding == FT_ENCODING_UNICODE);
    niLog(Info,
          niFmt("Font '%s' charmap%s '%s', %d, %d.", apFile->GetSourcePath(),
                selected ? " [SELECTED]" : "",
                _ASTR("").SetFourCC(niSwapBE32(charmap->encoding)), charmap->platform_id, charmap->encoding_id));
    if (selected) {
      foundCharmap = charmap;
    }
  }


  if (foundCharmap) {
    // now, select the charmap for the face object
    FT_Error error = FT_Set_Charmap(mFace, foundCharmap);
    if (error) {
      niLog(Error,
            niFmt("Font '%s' can't set charmap charmap '%s': %s.",
                  apFile->GetSourcePath(),
                  _ASTR("").SetFourCC(niSwapBE32(foundCharmap->encoding)),
                  MyFT_GetErrorString(error)));
    }
  }
#endif
}

///////////////////////////////////////////////
cFontTTF::~cFontTTF()
{
  if (mFace) {
    FT_Done_Face(mFace);
    mFace = NULL;
  }
  mptrFTLib = NULL;
}

///////////////////////////////////////////////
tBool cFontTTF::IsOK() const
{
  return mptrFTLib.IsOK() && mFace != NULL;
}

///////////////////////////////////////////////
tBool cFontTTF::GetCharSize(sVec2f& avOut, tU32 anChar, int anSize, tF32 afRealSize, tBool abDistanceField)
{
  const sFontGlyph* glyph = CacheCodepoint(anChar,anSize,abDistanceField,NULL);
  if (!glyph) {
    return eFalse;
  }
  avOut.x = glyph->dim.x;
  avOut.y = glyph->dim.y;
  return eTrue;
}

///////////////////////////////////////////////
tF32 cFontTTF::GetCharWidth(tU32 anChar, int anSize, tF32 afRealSize, tBool abDistanceField)
{
  sVec2f v = {0,0};
  GetCharSize(v,anChar,anSize,afRealSize,abDistanceField);
  return v.x;
}

///////////////////////////////////////////////
tF32 cFontTTF::GetCharHeight(tU32 anChar, int anSize, tF32 afRealSize, tBool abDistanceField)
{
  sVec2f v = {0,0};
  GetCharSize(v,anChar,anSize,afRealSize,abDistanceField);
  return v.y;
}

///////////////////////////////////////////////
tBool cFontTTF::RenderGlyph(sFontGlyph* baseGlyph, tU32 anBaseSize, tBool abDistanceField)
{
  // Already rendered / no rendering
  if (baseGlyph->img.IsOK() || !baseGlyph->bmp.IsOK()) {
    return eTrue;
  }

  // Render the glyph
  Ptr<iBitmap2D> renderedBmp;
  Ptr<iOverlay> ptrOG;
  if (abDistanceField) {
    // cache and render the scaled glyph
    sFontGlyph* glyph = CacheGlyphIndex(baseGlyph->glyphIndex, anBaseSize*SDF_scaler, eFalse);
    niAssert(glyph);
    if (!glyph) {
      return eFalse;
    }

    renderedBmp = mpGraphics->CreateBitmap2D(glyph->GetWidth()/SDF_scaler,glyph->GetHeight()/SDF_scaler,"A8");
    const tU32 nWidth = renderedBmp->GetWidth()*SDF_scaler;
    const tU32 nHeight = renderedBmp->GetHeight()*SDF_scaler;

    const int w = glyph->bmp->GetWidth();
    const int h = glyph->bmp->GetHeight();
    const int sw = w + (SDF_scaler * 4);
    const int sh = h + (SDF_scaler * 4);
    astl::vector<tU8> smooth_buf;
    smooth_buf.resize(sw*sh);
    memset(smooth_buf.data(),0,smooth_buf.size());

    //  copy the glyph into the buffer to be smoothed
    for (int j = 0; j < h; ++j) {
      const tU8* row = glyph->bmp->GetData() + (j*w);
      memcpy(smooth_buf.data() + (SDF_scaler*2+((j+SDF_scaler*2)*sw)), row, w);
    }

    //  do the SDF
    for (int j = 0; j < h; ++j) {
      tU8* row = glyph->bmp->GetData() + (j*w);
      for (int i = 0; i < w; ++i) {
        const tU8 dist = get_SDF_radial(
          smooth_buf.data(), sw, sh,
          SDF_scaler*2+i,
          SDF_scaler*2+j,
          2*SDF_scaler);
        row[i] = dist;
      }
    }
    renderedBmp->BlitStretch(glyph->bmp,0,0,0,0,glyph->bmp->GetWidth(),glyph->bmp->GetHeight(),renderedBmp->GetWidth(),renderedBmp->GetHeight());

    if (_dumpGlyphs.get()) {
      mpGraphics->SaveBitmap(niFmt("%s/%s_%d_char_%d_df0.png", ni::GetProperty("ni.dirs.logs"),
                                   GetName(),anBaseSize,baseGlyph->glyphIndex),glyph->bmp,0);
    }

    // uncache the scaled glyph, that release its memory
    UncacheGlyphIndex(baseGlyph->glyphIndex, anBaseSize*SDF_scaler, eFalse);
  }
  else {
    renderedBmp = baseGlyph->bmp;
  }

  {
    Ptr<iBitmap2D> bmp = mpGraphics->CreateBitmap2D(
      renderedBmp->GetWidth()+(TTF_GLYPH_PADDING*2),
      renderedBmp->GetHeight()+(TTF_GLYPH_PADDING*2),
      IMAGE_PIXELFORMAT);
    bmp->Clear(NULL);

    const tU32 bpp = 4; // ARGB8888
    const tU32 pitch = bmp->GetPitch();
    const tU32 destYOffset = TTF_GLYPH_PADDING;

    // niDebugFmt(("... GLYPH %3d (%c), sz:%d as:%d de:%d h:%d lead:%s : %d : %d : %g, %dx%d, [%d,%d,%d,%d]",
                // anChar, anChar,
                // anBaseSize, GetAscender(anBaseSize), GetDescender(anBaseSize), GetLineHeight(anBaseSize), baseGlyph->lead,
                // GetLineHeight(anBaseSize) + baseGlyph->y0,
                // GetAscender(anBaseSize) + baseGlyph->y0,
                // GetAdvanceCodepoint(anBaseSize,abDistanceField,0,anChar),
                // baseGlyph->GetWidth(), baseGlyph->GetHeight(),
                // baseGlyph->x0, baseGlyph->y0,
                // baseGlyph->x1, baseGlyph->y1));

    // Copy the renderedBmp (greyscale) to our destination bitmap
    niLoop(j,renderedBmp->GetHeight()) {
      tU8* dst = bmp->GetData() + ((j+destYOffset)*bmp->GetPitch()) + (TTF_GLYPH_PADDING*4);
      const tU8* src = renderedBmp->GetData() + (j*renderedBmp->GetPitch());
      niLoop(k,renderedBmp->GetWidth()) {
#ifdef TTF_PREMULALPHA
        dst[0] = dst[1] = dst[2] = src[0];
#else
        dst[0] = dst[1] = dst[2] = 255;
#endif
        dst[3] = src[0];
        dst += 4;
        src += 1;
      }
    }

    if (_dumpGlyphs.get()) {
      mpGraphics->SaveBitmap(niFmt("%s/%s_%d_char_%d_r.png", ni::GetProperty("ni.dirs.logs"),
                                   GetName(), anBaseSize, baseGlyph->glyphIndex), renderedBmp, 0);
      mpGraphics->SaveBitmap(niFmt("%s/%s_%d_char_%d.png", ni::GetProperty("ni.dirs.logs"),
                                   GetName(), anBaseSize, baseGlyph->glyphIndex), bmp, 0);
    }

#if defined TTF_USE_MIPMAPS && !defined TTF_COMPUTE_MIPMAPS_PER_PAGE
    bmp->CreateMipMaps(0,eTrue);
#endif
    ptrOG = GetImageMap()->AddImage(NULL,bmp);

    baseGlyph->bmp = NULL;
    baseGlyph->img = ptrOG->GetImage();
    baseGlyph->texCoo = ptrOG->GetMapping();
  }
  return eTrue;
}

///////////////////////////////////////////////
const sFontGlyph* cFontTTF::RenderGlyphIndex(tU32 anGlyphIndex, tU32 anBaseSize, tBool abDistanceField) {
  anBaseSize = ni::Min(anBaseSize,GetMaxCharSize());
  sFontGlyph* glyph = CacheGlyphIndex(anGlyphIndex,anBaseSize,abDistanceField);
  if (!glyph) return NULL;
  RenderGlyph(glyph,anBaseSize,abDistanceField);
  return glyph;
}

///////////////////////////////////////////////
const sFontGlyph* cFontTTF::RenderCodepoint(tU32 anCodepoint, tU32 anBaseSize, tBool abDistanceField) {
  anBaseSize = ni::Min(anBaseSize,GetMaxCharSize());
  cFontTTF* pRenderFont;
  sFontGlyph* glyph = CacheCodepoint(anCodepoint,anBaseSize,abDistanceField,&pRenderFont);
  if (!glyph) return NULL;
  pRenderFont->RenderGlyph(glyph,anBaseSize,abDistanceField);
  return glyph;
}

///////////////////////////////////////////////
cString cFontTTF::GetFamilyName() const
{
  return cString(mFace->family_name);
}

///////////////////////////////////////////////
cString cFontTTF::GetStyleName() const
{
  return cString(mFace->style_name);
}

///////////////////////////////////////////////
cString cFontTTF::GetName() const
{
  return GetTTFFaceName(mFace);
}

///////////////////////////////////////////////
tI32 cFontTTF::GetKerning(tU32 anPixelSize, tU32 anPrevChar, tU32 anChar) const {
#ifdef TTF_KERNING
  if (!GetHasKerning() || (anPrevChar <= 32) || (anChar <= 32)) {
    return 0;
  }
  return GetTTFGlyphKernAdvance(mFace, anPixelSize, &mnLastKerningPixelSize, anPrevChar, anChar);
#else
  return 0;
#endif
}
tF32 cFontTTF::GetAdvanceGlyphIndex(tU32 anPixelSize, tBool abDistanceField, tU32 anGlyphIndex) const {
  const sFontGlyph* g = niThis(cFontTTF)->CacheGlyphIndex(anGlyphIndex,anPixelSize,abDistanceField);
  if (!g) {
    return 0;
  }
  return g->xadv;
}
tF32 cFontTTF::GetAdvanceCodepoint(tU32 anPixelSize, tBool abDistanceField, tU32 anCodepoint) const {
  const sFontGlyph* g = niThis(cFontTTF)->CacheCodepoint(anCodepoint,anPixelSize,abDistanceField,NULL);
  if (!g) {
    return 0;
  }
  return g->xadv;
}

///////////////////////////////////////////////
tF32 cFontTTF::GetAscender(tF32 afRealSize) const {
  return ni::UnitSnapf(mfNormalizedAscender * afRealSize);
}
tF32 cFontTTF::GetDescender(tF32 afRealSize) const {
  return ni::UnitSnapf(mfNormalizedDescender * afRealSize);
}

///////////////////////////////////////////////
sFontGlyph* cFontTTF::CacheGlyphIndex(tU32 anGlyphIndex, tU32 anSize, tBool abDistanceField)
{
  tGlyphHash hash = TTFGlyphIndexHash(anGlyphIndex,anSize,abDistanceField);
  tGlyphCacheHMap::iterator it = mhmapGlyphCache.find(hash);
  if (it == mhmapGlyphCache.end()) {
    sFontGlyph glyph;
    glyph.glyphIndex = anGlyphIndex;
    glyph.bmp = RenderGlyphIndexBitmap(mpGraphics, mFace,
                                       anGlyphIndex, anSize,
                                       &glyph.xadv,
                                       &glyph.lead,
                                       &glyph.dim,
                                       GetAscender(anSize));
    it = astl::upsert(mhmapGlyphCache,hash,glyph);
  }
  if (!it->second.IsValidGlyph()) {
    return NULL;
  }

  return &it->second;
}


///////////////////////////////////////////////
sFontGlyph* cFontTTF::CacheCodepoint(tU32 anCodepoint, tU32 anSize, tBool abDistanceField, cFontTTF** appRenderFont)
{
  {
    tU32 glyphIndex = GetCodepointGlyphIndex(anCodepoint);
    if (glyphIndex) {
      sFontGlyph* gl = CacheGlyphIndex(glyphIndex,anSize,abDistanceField);
      if (gl) {
        if (appRenderFont)
          *appRenderFont = this;
        return gl;
      }
    }
  }

  cFontTTF* pFallback = _GetFallback(this,mpGraphics);
  if (pFallback) {
    tU32 glyphIndex = pFallback->GetCodepointGlyphIndex(anCodepoint);
    if (glyphIndex) {
      sFontGlyph* gl = pFallback->CacheGlyphIndex(glyphIndex,anSize,abDistanceField);
      if (gl) {
        if (appRenderFont)
          *appRenderFont = pFallback;
        return gl;
      }
    }
  }

  if (anCodepoint != '?') {
    return CacheCodepoint('?',anSize,abDistanceField,appRenderFont);
  }

  return NULL;
}

///////////////////////////////////////////////
tBool cFontTTF::UncacheGlyphIndex(tU32 anGlyphIndex, tU32 anSize, tBool abDistanceField)
{
  tGlyphHash hash = TTFGlyphIndexHash(anGlyphIndex,anSize,abDistanceField);
  tGlyphCacheHMap::iterator it = mhmapGlyphCache.find(hash);
  if (it != mhmapGlyphCache.end()) {
    mhmapGlyphCache.erase(it);
    return eTrue;
  }
  else {
    return eFalse;
  }
}

///////////////////////////////////////////////
iImageMap* cFontTTF::GetImageMap() {
  if (!mptrImageMap.IsOK()) {
    // Use L8A8, L8 garantees that the RGB is white, and A8 is the blending parameter.
    //
    // Note: DirectX9 uses white as default RGB if we use A8, but of course
    //       OpenGL uses black as default RGB if we use A8...
    mptrImageMap = mpGraphics->CreateImageMap(GetName().Chars(),"L8A8");
#ifdef TTF_USE_MIPMAPS
    mptrImageMap->SetPageMipMaps(1);
#ifdef TTF_COMPUTE_MIPMAPS_PER_PAGE
    mptrImageMap->SetComputeMipMapsPerPage(eTrue);
#endif
#else
    mptrImageMap->SetPageMipMaps(0);
#endif
    mptrImageMap->SetPageSize(mnImageMapPageSize);
    mptrImageMap->SetDefaultImageFilter(eTrue);
#ifdef TTF_PREMULALPHA
    mptrImageMap->SetDefaultImageBlendMode(eBlendMode_PreMulAlpha);
#else
    mptrImageMap->SetDefaultImageBlendMode(eBlendMode_Translucent);
#endif
  }
  return mptrImageMap;
}

///////////////////////////////////////////////
void cFontTTF::ClearCache() {
  if (mptrImageMap.IsOK()) {
    mptrImageMap->Invalidate();
    mptrImageMap = NULL;
  }
  mhmapGlyphCache.clear();
}

///////////////////////////////////////////////
sVec2f __stdcall cFontTTF::GetGlyphPath(iVGPath* apPath, tU32 anGlyphIndex, tU32 anSize, const sVec2f& avOffset, const tF32 afScale) const
{
  GetTTFGlyphPath(
    mFace,anGlyphIndex,anSize,true,
    // TODO: I dont know why we need to divide by two to get the right side, must be messing up something somewhere in the glyph extraction...
    agg::trans_affine_scaling(afScale/2.0f)*
    agg::trans_affine_translation(avOffset.x,avOffset.y)*
    agg::trans_affine_scaling(1/64.0f),
    apPath);
  return Vec2f(GetAdvanceGlyphIndex(anSize,eFalse,anGlyphIndex),0);
}

///////////////////////////////////////////////
Ptr<tU32CMap> __stdcall cFontTTF::EnumGlyphs() const {
  if (mEnumedGlyphs.IsOK()) {
    return mEnumedGlyphs;
  }

  mEnumedGlyphs = tU32CMap::Create();
  FT_UInt  gindex = 0;
  FT_ULong charcode = FT_Get_First_Char(mFace, &gindex);
  while (gindex != 0) {
    mEnumedGlyphs->insert(astl::make_pair((tU32)gindex,(tU32)charcode));
    // niDebugFmt(("... charcode: 0x%x, gindex: %d, name: %s", (tU32)charcode, (tU32)gindex, buffer));
    charcode = FT_Get_Next_Char(mFace, charcode, &gindex);
  }

  return mEnumedGlyphs;
}

//////////////////////////////////////////////////////////////////////////////////////////////
static tU8 get_SDF_radial(
  const tU8* fontmap,
  int w, int h,
  int x, int y,
  int max_radius)
{
  //  hideous brute force method
  float d2 = max_radius*max_radius+1.0f;
  tU8 v = fontmap[x+y*w];
  for( int radius = 1; (radius <= max_radius) && (radius*radius < d2); ++radius )
  {
    int line, lo, hi;
    //  north
    line = y - radius;
    if( (line >= 0) && (line < h) )
    {
      lo = x - radius;
      hi = x + radius;
      if( lo < 0 ) { lo = 0; }
      if( hi >= w ) { hi = w-1; }
      int idx = line * w + lo;
      for( int i = lo; i <= hi; ++i )
      {
        //  check this pixel
        if( fontmap[idx] != v )
        {
          float nx = i - x;
          float ny = line - y;
          float nd2 = nx*nx+ny*ny;
          if( nd2 < d2 )
          {
            d2 = nd2;
          }
        }
        //  move on
        ++idx;
      }
    }
    //  south
    line = y + radius;
    if( (line >= 0) && (line < h) )
    {
      lo = x - radius;
      hi = x + radius;
      if( lo < 0 ) { lo = 0; }
      if( hi >= w ) { hi = w-1; }
      int idx = line * w + lo;
      for( int i = lo; i <= hi; ++i )
      {
        //  check this pixel
        if( fontmap[idx] != v )
        {
          float nx = i - x;
          float ny = line - y;
          float nd2 = nx*nx+ny*ny;
          if( nd2 < d2 )
          {
            d2 = nd2;
          }
        }
        //  move on
        ++idx;
      }
    }
    //  west
    line = x - radius;
    if( (line >= 0) && (line < w) )
    {
      lo = y - radius + 1;
      hi = y + radius - 1;
      if( lo < 0 ) { lo = 0; }
      if( hi >= h ) { hi = h-1; }
      int idx = lo * w + line;
      for( int i = lo; i <= hi; ++i )
      {
        //  check this pixel
        if( fontmap[idx] != v )
        {
          float nx = line - x;
          float ny = i - y;
          float nd2 = nx*nx+ny*ny;
          if( nd2 < d2 )
          {
            d2 = nd2;
          }
        }
        //  move on
        idx += w;
      }
    }
    //  east
    line = x + radius;
    if( (line >= 0) && (line < w) )
    {
      lo = y - radius + 1;
      hi = y + radius - 1;
      if( lo < 0 ) { lo = 0; }
      if( hi >= h ) { hi = h-1; }
      int idx = lo * w + line;
      for( int i = lo; i <= hi; ++i )
      {
        //  check this pixel
        if( fontmap[idx] != v )
        {
          float nx = line - x;
          float ny = i - y;
          float nd2 = nx*nx+ny*ny;
          if( nd2 < d2 )
          {
            d2 = nd2;
          }
        }
        //  move on
        idx += w;
      }
    }
  }
  d2 = sqrtf( d2 );
  if( v==0 )
  {
    d2 = -d2;
  }
  d2 *= 127.5f / max_radius;
  d2 += 127.5f;
  if( d2 < 0.0 ) d2 = 0.0;
  if( d2 > 255.0 ) d2 = 255.0;
  return (tU8)(d2 + 0.5);
}

///////////////////////////////////////////////
sMyFTLibrary* __stdcall cGraphics::GetFTLibrary()
{
  if (!mptrFTLib.IsOK())
    mptrFTLib = niNew sMyFTLibrary();
  return mptrFTLib;
}
