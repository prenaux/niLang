//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------


#include <stdio.h>
#include "agg_font_freetype.h"
#include "agg_bitset_iterator.h"
#include "agg_renderer_scanline.h"

#if niMinFeatures(20)
#include <niUI/IVGPath.h>
#endif

namespace agg
{
#if niMinFeatures(20)
struct sVGPathStorage
{
  ni::Ptr<ni::iVGPath>  mptrPath;

 public:
  typedef ni::tF32  value_type;

  sVGPathStorage(ni::iVGPath* apPath) {
    mptrPath = apPath;
  }

  void move_to(ni::tF32 x, ni::tF32 y) {
    mptrPath->MoveTo(x,y);
  }

  void line_to(ni::tF32 x, ni::tF32 y) {
    mptrPath->LineTo(x,y);
  }

  void curve3(ni::tF32 x_ctrl, ni::tF32 y_ctrl, ni::tF32 x_to,   ni::tF32 y_to) {
    mptrPath->Curve3(x_ctrl,y_ctrl,x_to,y_to);
  }

  void curve3(ni::tF32 x_to, ni::tF32 y_to) {
    mptrPath->Curve3Prev(x_to,y_to);
  }

  void curve4(ni::tF32 x_ctrl1, ni::tF32 y_ctrl1, ni::tF32 x_ctrl2, ni::tF32 y_ctrl2, ni::tF32 x_to, ni::tF32 y_to) {
    mptrPath->Curve4(x_ctrl1,y_ctrl1,x_ctrl2,y_ctrl2,x_to,y_to);
  }

  void curve4(ni::tF32 x_ctrl2, ni::tF32 y_ctrl2, ni::tF32 x_to, ni::tF32 y_to) {
    mptrPath->Curve4Prev(x_ctrl2,y_ctrl2,x_to,y_to);
  }

  void close_polygon() {
    mptrPath->ClosePolygon();
  }
};
#endif // niMinFeatures

//------------------------------------------------------------------------------
//
// This code implements the AUTODIN II polynomial
// The variable corresponding to the macro argument "crc" should
// be an unsigned long.
// Oroginal code  by Spencer Garrett <srg@quick.com>
//

// generated using the AUTODIN II polynomial
//   x^32 + x^26 + x^23 + x^22 + x^16 +
//   x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + 1
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------
static inline int dbl_to_plain_fx(agg_real d)
{
  return int(d * 65536.0);
}

//------------------------------------------------------------------------
static inline agg_real int26p6_to_dbl(int p)
{
  return agg_real(agg_real(p) / 64.0);
}

//------------------------------------------------------------------------
static inline int dbl_to_int26p6(agg_real p)
{
  return int(p * 64.0 + 0.5);
}


//------------------------------------------------------------------------
template<class PathStorage>
bool decompose_ft_outline(const FT_Outline& outline,
                          bool flip_y,
                          const trans_affine& mtx,
                          PathStorage& path)
{
  typedef typename PathStorage::value_type value_type;

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
    path.move_to(value_type(dbl_to_int26p6(x1)),
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
            path.line_to(value_type(dbl_to_int26p6(x1)),
                         value_type(dbl_to_int26p6(y1)));
            //path.line_to(conv(point->x), flip_y ? -conv(point->y) : conv(point->y));
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
                path.curve3(value_type(dbl_to_int26p6(x1)),
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
              path.curve3(value_type(dbl_to_int26p6(x1)),
                          value_type(dbl_to_int26p6(y1)),
                          value_type(dbl_to_int26p6(x2)),
                          value_type(dbl_to_int26p6(y2)));

              //path.curve3(conv(v_control.x),
              //            flip_y ? -conv(v_control.y) : conv(v_control.y),
              //            conv(v_middle.x),
              //            flip_y ? -conv(v_middle.y) : conv(v_middle.y));

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
            path.curve3(value_type(dbl_to_int26p6(x1)),
                        value_type(dbl_to_int26p6(y1)),
                        value_type(dbl_to_int26p6(x2)),
                        value_type(dbl_to_int26p6(y2)));

            //path.curve3(conv(v_control.x),
            //            flip_y ? -conv(v_control.y) : conv(v_control.y),
            //            conv(v_start.x),
            //            flip_y ? -conv(v_start.y) : conv(v_start.y));
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
              path.curve4(value_type(dbl_to_int26p6(x1)),
                          value_type(dbl_to_int26p6(y1)),
                          value_type(dbl_to_int26p6(x2)),
                          value_type(dbl_to_int26p6(y2)),
                          value_type(dbl_to_int26p6(x3)),
                          value_type(dbl_to_int26p6(y3)));

              //path.curve4(conv(vec1.x),
              //            flip_y ? -conv(vec1.y) : conv(vec1.y),
              //            conv(vec2.x),
              //            flip_y ? -conv(vec2.y) : conv(vec2.y),
              //            conv(vec.x),
              //            flip_y ? -conv(vec.y) : conv(vec.y));
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
            path.curve4(value_type(dbl_to_int26p6(x1)),
                        value_type(dbl_to_int26p6(y1)),
                        value_type(dbl_to_int26p6(x2)),
                        value_type(dbl_to_int26p6(y2)),
                        value_type(dbl_to_int26p6(x3)),
                        value_type(dbl_to_int26p6(y3)));

            //path.curve4(conv(vec1.x),
            //            flip_y ? -conv(vec1.y) : conv(vec1.y),
            //            conv(vec2.x),
            //            flip_y ? -conv(vec2.y) : conv(vec2.y),
            //            conv(v_start.x),
            //            flip_y ? -conv(v_start.y) : conv(v_start.y));
            goto Close;
          }
      }
    }

    path.close_polygon();

 Close:
    first = last + 1;
  }

  return true;
}



//------------------------------------------------------------------------
template<class Scanline, class ScanlineStorage>
void decompose_ft_bitmap_mono(const FT_Bitmap& bitmap,
                              int x, int y,
                              bool flip_y,
                              Scanline& sl,
                              ScanlineStorage& storage)
{
  int i;
  const int8u* buf = (const int8u*)bitmap.buffer;
  int pitch = bitmap.pitch;
  sl.reset(x, x + bitmap.width);
  storage.prepare();
  if(flip_y)
  {
    buf += bitmap.pitch * (bitmap.rows - 1);
    y += bitmap.rows;
    pitch = -pitch;
  }
  for(i = 0; i < bitmap.rows; i++)
  {
    sl.reset_spans();
    bitset_iterator bits(buf, 0);
    int j;
    for(j = 0; j < bitmap.width; j++)
    {
      if(bits.bit()) sl.add_cell(x + j, cover_full);
      ++bits;
    }
    buf += pitch;
    if(sl.num_spans())
    {
      sl.finalize(y - i - 1);
      storage.render(sl);
    }
  }
}



//------------------------------------------------------------------------
template<class Rasterizer, class Scanline, class ScanlineStorage>
void decompose_ft_bitmap_gray8(const FT_Bitmap& bitmap,
                               int x, int y,
                               bool flip_y,
                               Rasterizer& ras,
                               Scanline& sl,
                               ScanlineStorage& storage)
{
  int i, j;
  const int8u* buf = (const int8u*)bitmap.buffer;
  int pitch = bitmap.pitch;
  sl.reset(x, x + bitmap.width);
  storage.prepare();
  if(flip_y)
  {
    buf += bitmap.pitch * (bitmap.rows - 1);
    y += bitmap.rows;
    pitch = -pitch;
  }
  for(i = 0; i < bitmap.rows; i++)
  {
    sl.reset_spans();
    const int8u* p = buf;
    for(j = 0; j < bitmap.width; j++)
    {
      if(*p) sl.add_cell(x + j, ras.apply_gamma(*p));
      ++p;
    }
    buf += pitch;
    if(sl.num_spans())
    {
      sl.finalize(y - i - 1);
      storage.render(sl);
    }
  }
}













//------------------------------------------------------------------------
font_engine_freetype_base::~font_engine_freetype_base()
{
  unsigned i;
  for(i = 0; i < m_num_faces; ++i)
  {
    delete [] m_face_names[i];
    FT_Done_Face(m_faces[i]);
  }
  delete [] m_face_names;
  delete [] m_faces;
  //delete [] m_signature;
}


//------------------------------------------------------------------------
font_engine_freetype_base::font_engine_freetype_base(bool flag32,
                                                     unsigned max_faces) :
    m_flag32(flag32),
    m_change_stamp(0),
    m_last_error(0),
    m_name(0),
    m_name_len(256-16-1),
    m_face_index(0),
    m_char_map(FT_ENCODING_NONE),
    //m_signature(new char [256+256-16]),
    m_height(0),
    m_width(0),
    m_hinting(true),
    m_flip_y(false),
    m_faces(new FT_Face [max_faces]),
    m_face_names(new char* [max_faces]),
    m_num_faces(0),
    m_max_faces(max_faces),
    m_cur_face(0),
    m_resolution(0),
    m_glyph_rendering(glyph_ren_native_gray8),
    m_glyph_index(0),
    m_data_size(0),
    m_data_type(glyph_data_invalid),
    m_bounds(1,1,0,0),
    m_advance_x(0.0),
    m_advance_y(0.0),
    m_bearing_x(0.0),
    m_bearing_y(0.0),
    m_path16(),
    m_path32(),
    m_curves16(m_path16),
    m_curves32(m_path32),
    m_scanline_aa(),
    m_scanline_bin(),
    m_scanlines_aa(),
    m_scanlines_bin(),
    m_rasterizer()
{
  m_curves16.approximation_scale(4.0);
  m_curves32.approximation_scale(4.0);
}



//------------------------------------------------------------------------
void font_engine_freetype_base::resolution(unsigned dpi)
{
  m_resolution = dpi;
  update_char_size();
}


//------------------------------------------------------------------------
int font_engine_freetype_base::find_face(const char* face_name) const
{
  unsigned i;
  for(i = 0; i < m_num_faces; ++i)
  {
    if(strcmp(face_name, m_face_names[i]) == 0) return i;
  }
  return -1;
}


//------------------------------------------------------------------------
agg_real font_engine_freetype_base::ascender() const
{
  if(m_cur_face)
  {
    return m_cur_face->ascender * height() / m_cur_face->height;
  }
  return 0.0;
}

//------------------------------------------------------------------------
agg_real font_engine_freetype_base::descender() const
{
  if(m_cur_face)
  {
    return m_cur_face->descender * height() / m_cur_face->height;
  }
  return 0.0;
}


//------------------------------------------------------------------------
bool font_engine_freetype_base::load_font(ni::sMyFTLibrary* apLib,
                                          const char* font_name,
                                          unsigned face_index,
                                          glyph_rendering ren_type,
                                          ni::iFile* apFile)
{
  mptrLibrary = apLib;

  bool ret = false;

  m_last_error = 0;

  int idx = find_face(font_name);
  if(idx >= 0)
  {
    m_cur_face = m_faces[idx];
    m_name     = m_face_names[idx];
  }
  else
  {
    if(m_num_faces >= m_max_faces)
    {
      delete [] m_face_names[0];
      FT_Done_Face(m_faces[0]);
      memcpy(m_faces,
             m_faces + 1,
             (m_max_faces - 1) * sizeof(FT_Face));
      memcpy(m_face_names,
             m_face_names + 1,
             (m_max_faces - 1) * sizeof(char*));
      m_num_faces = m_max_faces - 1;
    }

    if (apFile)
    {
      m_faces[m_num_faces] = MyFT_Face_Open(mptrLibrary->mLibrary, apFile);
      m_last_error = (m_faces[m_num_faces]==0)?1:0;
    }
    else
    {
      m_last_error = FT_New_Face(mptrLibrary->mLibrary,
                                 font_name,
                                 face_index,
                                 &m_faces[m_num_faces]);
    }

    if(m_last_error == 0)
    {
      m_face_names[m_num_faces] = new char [strlen(font_name) + 1];
      strcpy(m_face_names[m_num_faces], font_name);
      m_cur_face = m_faces[m_num_faces];
      m_name     = m_face_names[m_num_faces];
      ++m_num_faces;
    }
    else
    {
      m_face_names[m_num_faces] = 0;
      m_cur_face = 0;
      m_name = 0;
    }
  }


  if(m_last_error == 0)
  {
    ret = true;
    set_glyph_rendering(ren_type);
  }

  return ret;
}

//------------------------------------------------------------------------
void font_engine_freetype_base::set_glyph_rendering(glyph_rendering ren_type)
{
  switch(ren_type)
  {
    case glyph_ren_native_mono:
      m_glyph_rendering = glyph_ren_native_mono;
      break;

    case glyph_ren_native_gray8:
      m_glyph_rendering = glyph_ren_native_gray8;
      break;

    case glyph_ren_outline:
      if(FT_IS_SCALABLE(m_cur_face))
      {
        m_glyph_rendering = glyph_ren_outline;
      }
      else
      {
        m_glyph_rendering = glyph_ren_native_gray8;
      }
      break;

    case glyph_ren_agg_mono:
      if (FT_IS_SCALABLE(m_cur_face)) {
        m_glyph_rendering = glyph_ren_agg_mono;
      }
      else {
        m_glyph_rendering = glyph_ren_native_mono;
      }
      break;

    case glyph_ren_agg_gray8:
      if (FT_IS_SCALABLE(m_cur_face)) {
        m_glyph_rendering = glyph_ren_agg_gray8;
      }
      else {
        m_glyph_rendering = glyph_ren_native_gray8;
      }
      break;
  }
}

//------------------------------------------------------------------------
glyph_rendering font_engine_freetype_base::get_glyph_rendering() const
{
  return m_glyph_rendering;
}

//------------------------------------------------------------------------
bool font_engine_freetype_base::attach(const char* file_name)
{
  if(m_cur_face)
  {
    m_last_error = FT_Attach_File(m_cur_face, file_name);
    return m_last_error == 0;
  }
  return false;
}

//------------------------------------------------------------------------
unsigned font_engine_freetype_base::num_faces() const
{
  if(m_cur_face)
  {
    return m_cur_face->num_faces;
  }
  return 0;
}

//------------------------------------------------------------------------
bool font_engine_freetype_base::char_map(FT_Encoding char_map)
{
  if(m_cur_face)
  {
    m_last_error = FT_Select_Charmap(m_cur_face, m_char_map);
    if(m_last_error == 0) {
      return true;
    }
  }
  return false;
}

//------------------------------------------------------------------------
bool font_engine_freetype_base::height(agg_real h)
{
  m_height = int(h * 64.0);
  if(m_cur_face)
  {
    update_char_size();
    return true;
  }
  return false;
}

//------------------------------------------------------------------------
bool font_engine_freetype_base::width(agg_real w)
{
  m_width = int(w * 64.0);
  if(m_cur_face)
  {
    update_char_size();
    return true;
  }
  return false;
}

//------------------------------------------------------------------------
void font_engine_freetype_base::hinting(bool h)
{
  m_hinting = h;
}

//------------------------------------------------------------------------
void font_engine_freetype_base::flip_y(bool f)
{
  m_flip_y = f;
}

//------------------------------------------------------------------------
void font_engine_freetype_base::transform(const trans_affine& affine)
{
  m_affine = affine;
}

//------------------------------------------------------------------------
void font_engine_freetype_base::update_char_size()
{
  if(m_cur_face)
  {
    if(m_resolution)
    {
      FT_Set_Char_Size(m_cur_face,
                       m_width,       // char_width in 1/64th of points
                       m_height,      // char_height in 1/64th of points
                       m_resolution,  // horizontal device resolution
                       m_resolution); // vertical device resolution
    }
    else
    {
      FT_Set_Pixel_Sizes(m_cur_face,
                         m_width >> 6,    // pixel_width
                         m_height >> 6);  // pixel_height
    }
  }
}

//------------------------------------------------------------------------
bool font_engine_freetype_base::has_path() const
{
  return FT_IS_SCALABLE(m_cur_face);
}

#if niMinFeatures(20)
//------------------------------------------------------------------------
bool font_engine_freetype_base::get_glyph_path(unsigned glyph_code,
                                               bool flip_y,
                                               bool hinting,
                                               const trans_affine& mtx,
                                               ni::iVGPath* apPath,
                                               agg_real* advance_x, agg_real* advance_y,
                                               agg_real* real_width, agg_real* real_height)
{
  unsigned glyph_index = FT_Get_Char_Index(m_cur_face, glyph_code);
  if (m_glyph_index == 0)
    return false;

  unsigned last_error = FT_Load_Glyph(m_cur_face, glyph_index,
                                      hinting ? FT_LOAD_DEFAULT : FT_LOAD_NO_HINTING);
  //                                     hinting ? FT_LOAD_FORCE_AUTOHINT : FT_LOAD_NO_HINTING);
  if (last_error == 0)
  {
    if (niIsOK(apPath) && FT_IS_SCALABLE(m_cur_face)) {
      sVGPathStorage path(apPath);
      decompose_ft_outline(m_cur_face->glyph->outline,flip_y,mtx,path);
    }
    if (advance_x)
      *advance_x = int26p6_to_dbl(m_cur_face->glyph->advance.x);
    if (advance_y)
      *advance_y = int26p6_to_dbl(m_cur_face->glyph->advance.y);
    if (real_width)
      *real_width = int26p6_to_dbl(m_cur_face->glyph->metrics.width);
    if (real_height)
      *real_height = int26p6_to_dbl(m_cur_face->glyph->metrics.height);
    return true;
  }

  return false;
}
#endif

//------------------------------------------------------------------------
bool font_engine_freetype_base::prepare_glyph(unsigned glyph_code)
{
  m_glyph_index = FT_Get_Char_Index(m_cur_face, glyph_code);
  if (m_glyph_index == 0)
    return false;

  m_last_error = FT_Load_Glyph(m_cur_face,
                               m_glyph_index,
                               m_hinting ? FT_LOAD_DEFAULT : FT_LOAD_NO_HINTING);
  //                                     m_hinting ? FT_LOAD_FORCE_AUTOHINT : FT_LOAD_NO_HINTING);
  if(m_last_error == 0)
  {
    switch(m_glyph_rendering)
    {
      case glyph_ren_native_mono:
        m_last_error = FT_Render_Glyph(m_cur_face->glyph, FT_RENDER_MODE_MONO);
        if(m_last_error == 0)
        {
          decompose_ft_bitmap_mono(m_cur_face->glyph->bitmap,
                                   m_cur_face->glyph->bitmap_left,
                                   m_flip_y ? -m_cur_face->glyph->bitmap_top :
                                   m_cur_face->glyph->bitmap_top,
                                   m_flip_y,
                                   m_scanline_bin,
                                   m_scanlines_bin);
          m_bounds.x1 = m_scanlines_bin.min_x();
          m_bounds.y1 = m_scanlines_bin.min_y();
          m_bounds.x2 = m_scanlines_bin.max_x();
          m_bounds.y2 = m_scanlines_bin.max_y();
          m_data_size = m_scanlines_bin.byte_size();
          m_data_type = glyph_data_mono;
          m_advance_x = int26p6_to_dbl(m_cur_face->glyph->advance.x);
          m_advance_y = int26p6_to_dbl(m_cur_face->glyph->advance.y);
          m_bearing_x = int26p6_to_dbl(m_cur_face->glyph->metrics.horiBearingX);
          m_bearing_y = int26p6_to_dbl(m_cur_face->glyph->metrics.vertBearingY);
          m_real_width = int26p6_to_dbl(m_cur_face->glyph->metrics.width);
          m_real_height = int26p6_to_dbl(m_cur_face->glyph->metrics.height);
          return true;
        }
        break;


      case glyph_ren_native_gray8:
        m_last_error = FT_Render_Glyph(m_cur_face->glyph, FT_RENDER_MODE_NORMAL);
        if(m_last_error == 0)
        {
          decompose_ft_bitmap_gray8(m_cur_face->glyph->bitmap,
                                    m_cur_face->glyph->bitmap_left,
                                    m_flip_y ? -m_cur_face->glyph->bitmap_top :
                                    m_cur_face->glyph->bitmap_top,
                                    m_flip_y,
                                    m_rasterizer,
                                    m_scanline_aa,
                                    m_scanlines_aa);
          m_bounds.x1 = m_scanlines_aa.min_x();
          m_bounds.y1 = m_scanlines_aa.min_y();
          m_bounds.x2 = m_scanlines_aa.max_x();
          m_bounds.y2 = m_scanlines_aa.max_y();
          m_data_size = m_scanlines_aa.byte_size();
          m_data_type = glyph_data_gray8;
          m_advance_x = int26p6_to_dbl(m_cur_face->glyph->advance.x);
          m_advance_y = int26p6_to_dbl(m_cur_face->glyph->advance.y);
          m_bearing_x = int26p6_to_dbl(m_cur_face->glyph->metrics.horiBearingX);
          m_bearing_y = int26p6_to_dbl(m_cur_face->glyph->metrics.vertBearingY);
          m_real_width = int26p6_to_dbl(m_cur_face->glyph->metrics.width);
          m_real_height = int26p6_to_dbl(m_cur_face->glyph->metrics.height);
          return true;
        }
        break;


      case glyph_ren_outline:
        if(m_last_error == 0)
        {
          if(m_flag32)
          {
            m_path32.remove_all();
            if(decompose_ft_outline(m_cur_face->glyph->outline,
                                    m_flip_y,
                                    m_affine,
                                    m_path32))
            {
              rect_d bnd  = m_path32.bounding_rect();
              m_data_size = m_path32.byte_size();
              m_data_type = glyph_data_outline;
              m_bounds.x1 = int(floor(bnd.x1));
              m_bounds.y1 = int(floor(bnd.y1));
              m_bounds.x2 = int(ceil(bnd.x2));
              m_bounds.y2 = int(ceil(bnd.y2));
              m_advance_x = int26p6_to_dbl(m_cur_face->glyph->advance.x);
              m_advance_y = int26p6_to_dbl(m_cur_face->glyph->advance.y);
              m_bearing_x = int26p6_to_dbl(m_cur_face->glyph->metrics.horiBearingX);
              m_bearing_y = int26p6_to_dbl(m_cur_face->glyph->metrics.vertBearingY);
              m_real_width = int26p6_to_dbl(m_cur_face->glyph->metrics.width);
              m_real_height = int26p6_to_dbl(m_cur_face->glyph->metrics.height);
              m_affine.transform(&m_advance_x, &m_advance_y);
              return true;
            }
          }
          else
          {
            m_path16.remove_all();
            if(decompose_ft_outline(m_cur_face->glyph->outline,
                                    m_flip_y,
                                    m_affine,
                                    m_path16))
            {
              rect_d bnd  = m_path16.bounding_rect();
              m_data_size = m_path16.byte_size();
              m_data_type = glyph_data_outline;
              m_bounds.x1 = int(floor(bnd.x1));
              m_bounds.y1 = int(floor(bnd.y1));
              m_bounds.x2 = int(ceil(bnd.x2));
              m_bounds.y2 = int(ceil(bnd.y2));
              m_advance_x = int26p6_to_dbl(m_cur_face->glyph->advance.x);
              m_advance_y = int26p6_to_dbl(m_cur_face->glyph->advance.y);
              m_bearing_x = int26p6_to_dbl(m_cur_face->glyph->metrics.horiBearingX);
              m_bearing_y = int26p6_to_dbl(m_cur_face->glyph->metrics.vertBearingY);
              m_real_width = int26p6_to_dbl(m_cur_face->glyph->metrics.width);
              m_real_height = int26p6_to_dbl(m_cur_face->glyph->metrics.height);
              m_affine.transform(&m_advance_x, &m_advance_y);
              return true;
            }
          }
        }
        return false;

      case glyph_ren_agg_mono:
        if(m_last_error == 0)
        {
          m_rasterizer.reset();
          if(m_flag32)
          {
            m_path32.remove_all();
            decompose_ft_outline(m_cur_face->glyph->outline,
                                 m_flip_y,
                                 m_affine,
                                 m_path32);
            m_rasterizer.add_path(m_curves32);
          }
          else
          {
            m_path16.remove_all();
            decompose_ft_outline(m_cur_face->glyph->outline,
                                 m_flip_y,
                                 m_affine,
                                 m_path16);
            m_rasterizer.add_path(m_curves16);
          }
          m_scanlines_bin.prepare(); // Remove all
          render_scanlines(m_rasterizer, m_scanline_bin, m_scanlines_bin);
          m_bounds.x1 = m_scanlines_bin.min_x();
          m_bounds.y1 = m_scanlines_bin.min_y();
          m_bounds.x2 = m_scanlines_bin.max_x();
          m_bounds.y2 = m_scanlines_bin.max_y();
          m_data_size = m_scanlines_bin.byte_size();
          m_data_type = glyph_data_mono;
          m_advance_x = int26p6_to_dbl(m_cur_face->glyph->advance.x);
          m_advance_y = int26p6_to_dbl(m_cur_face->glyph->advance.y);
          m_bearing_x = int26p6_to_dbl(m_cur_face->glyph->metrics.horiBearingX);
          m_bearing_y = int26p6_to_dbl(m_cur_face->glyph->metrics.vertBearingY);
          m_real_width = int26p6_to_dbl(m_cur_face->glyph->metrics.width);
          m_real_height = int26p6_to_dbl(m_cur_face->glyph->metrics.height);
          m_affine.transform(&m_advance_x, &m_advance_y);
          return true;
        }
        return false;


      case glyph_ren_agg_gray8:
        if(m_last_error == 0)
        {
          m_rasterizer.reset();
          if(m_flag32)
          {
            m_path32.remove_all();
            decompose_ft_outline(m_cur_face->glyph->outline,
                                 m_flip_y,
                                 m_affine,
                                 m_path32);
            m_rasterizer.add_path(m_curves32);
          }
          else
          {
            m_path16.remove_all();
            decompose_ft_outline(m_cur_face->glyph->outline,
                                 m_flip_y,
                                 m_affine,
                                 m_path16);
            m_rasterizer.add_path(m_curves16);
          }
          m_scanlines_aa.prepare(); // Remove all
          render_scanlines(m_rasterizer, m_scanline_aa, m_scanlines_aa);
          m_bounds.x1 = m_scanlines_aa.min_x();
          m_bounds.y1 = m_scanlines_aa.min_y();
          m_bounds.x2 = m_scanlines_aa.max_x();
          m_bounds.y2 = m_scanlines_aa.max_y();
          m_data_size = m_scanlines_aa.byte_size();
          m_data_type = glyph_data_gray8;
          m_advance_x = int26p6_to_dbl(m_cur_face->glyph->advance.x);
          m_advance_y = int26p6_to_dbl(m_cur_face->glyph->advance.y);
          m_bearing_x = int26p6_to_dbl(m_cur_face->glyph->metrics.horiBearingX);
          m_bearing_y = int26p6_to_dbl(m_cur_face->glyph->metrics.vertBearingY);
          m_real_width = int26p6_to_dbl(m_cur_face->glyph->metrics.width);
          m_real_height = int26p6_to_dbl(m_cur_face->glyph->metrics.height);
          m_affine.transform(&m_advance_x, &m_advance_y);
          return true;
        }
        return false;
    }
  }
  return false;
}




//------------------------------------------------------------------------
void font_engine_freetype_base::write_glyph_to(int8u* data) const
{
  if(data && m_data_size)
  {
    switch(m_data_type)
    {
      default: return;
      case glyph_data_mono:    m_scanlines_bin.serialize(data); break;
      case glyph_data_gray8:   m_scanlines_aa.serialize(data);  break;
      case glyph_data_outline:
        if(m_flag32)
        {
          m_path32.serialize(data);
        }
        else
        {
          m_path16.serialize(data);
        }
        break;
      case glyph_data_invalid: break;
    }
  }
}



//------------------------------------------------------------------------
bool font_engine_freetype_base::add_kerning(unsigned first, unsigned second, agg_real* x, agg_real* y)
{
  if(m_cur_face && first && second && FT_HAS_KERNING(m_cur_face))
  {
    FT_Vector delta;
    FT_Get_Kerning(m_cur_face, first, second,
                   FT_KERNING_DEFAULT, &delta);
    agg_real dx = int26p6_to_dbl(delta.x);
    agg_real dy = int26p6_to_dbl(delta.y);
    if(m_glyph_rendering == glyph_ren_outline ||
       m_glyph_rendering == glyph_ren_agg_mono ||
       m_glyph_rendering == glyph_ren_agg_gray8)
    {
      m_affine.transform_2x2(&dx, &dy);
    }
    *x += dx;
    *y += dy;

    return true;
  }
  return false;
}


//------------------------------------------------------------------------
bool font_engine_freetype_base::get_kerning(unsigned first, unsigned second,
                                            agg_real* x, agg_real* y)
{
  if(m_cur_face && first && second && FT_HAS_KERNING(m_cur_face))
  {
    FT_Vector delta;
    FT_Get_Kerning(m_cur_face, first, second,
                   FT_KERNING_DEFAULT, &delta);
    *x = int26p6_to_dbl(delta.x);
    *y = int26p6_to_dbl(delta.y);
    return true;
  }
  return false;
}
}
