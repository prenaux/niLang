//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
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
//
// Class gsv_text
//
//----------------------------------------------------------------------------

#ifndef AGG_GSV_TEXT_INCLUDED
#define AGG_GSV_TEXT_INCLUDED

#include "agg_array.h"
#include "agg_conv_stroke.h"
#include "agg_conv_transform.h"

namespace agg
{


//---------------------------------------------------------------gsv_text
//
// See Implementation agg_gsv_text.cpp
//
class gsv_text
{
  enum status
  {
    initial,
    next_char,
    start_glyph,
    glyph
  };

 public:
  gsv_text();

  void font(const void* font);
  void flip(bool flip_y) { m_flip = flip_y; }
  void load_font(const char* file);
  void size(agg_real height, agg_real width=0.0);
  void space(agg_real space);
  void line_space(agg_real line_space);
  void start_point(agg_real x, agg_real y);
  void text(const char* text);

  void rewind(unsigned path_id);
  unsigned vertex(agg_real* x, agg_real* y);

 private:
  // not supposed to be copied
  gsv_text(const gsv_text&);
  const gsv_text& operator = (const gsv_text&);

  int16u value(const int8u* p) const
  {
    int16u v;
    if(m_big_endian)
    {
      *(int8u*)&v      = p[1];
      *((int8u*)&v + 1) = p[0];
    }
    else
    {
      *(int8u*)&v      = p[0];
      *((int8u*)&v + 1) = p[1];
    }
    return v;
  }

 private:
  agg_real          m_x;
  agg_real          m_y;
  agg_real          m_start_x;
  agg_real          m_width;
  agg_real          m_height;
  agg_real          m_space;
  agg_real          m_line_space;
  char            m_chr[2];
  char*           m_text;
  pod_array<char> m_text_buf;
  char*           m_cur_chr;
  const void*     m_font;
  pod_array<char> m_loaded_font;
  status          m_status;
  bool            m_big_endian;
  bool            m_flip;
  int8u*          m_indices;
  int8*           m_glyphs;
  int8*           m_bglyph;
  int8*           m_eglyph;
  agg_real          m_w;
  agg_real          m_h;
};




//--------------------------------------------------------gsv_text_outline
template<class Transformer = trans_affine> class gsv_text_outline
{
 public:
  gsv_text_outline(gsv_text& text, const Transformer& trans) :
      m_polyline(text),
      m_trans(m_polyline, trans)
  {
  }

  void width(agg_real w)
  {
    m_polyline.width(w);
  }

  void transformer(const Transformer* trans)
  {
    m_trans->transformer(trans);
  }

  void rewind(unsigned path_id)
  {
    m_trans.rewind(path_id);
    m_polyline.line_join(round_join);
    m_polyline.line_cap(round_cap);
  }

  unsigned vertex(agg_real* x, agg_real* y)
  {
    return m_trans.vertex(x, y);
  }

 private:
  conv_stroke<gsv_text> m_polyline;
  conv_transform<conv_stroke<gsv_text>, Transformer> m_trans;
};



}


#endif
