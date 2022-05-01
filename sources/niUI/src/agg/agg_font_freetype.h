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
//
// See implementation agg_font_freetype.cpp
//
//----------------------------------------------------------------------------

#ifndef AGG_FONT_FREETYPE_INCLUDED
#define AGG_FONT_FREETYPE_INCLUDED

#include "../FreetypeWrapper.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYSTEM_H
#include FT_GLYPH_H
#include FT_MODULE_H

#include "agg_scanline_storage_aa.h"
#include "agg_scanline_storage_bin.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_path_storage_integer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_conv_curve.h"
#include "agg_conv_contour.h"
#include "agg_trans_affine.h"
#include "agg_path_storage.h"

#if niMinFeatures(20)
namespace ni {
struct iVGPath;
}
#endif

namespace agg
{

//---------------------------------------------------------glyph_data_type
enum glyph_data_type
{
  glyph_data_invalid = 0,  //-----glyph_data_invalid
  glyph_data_mono    = 1,  //-----glyph_data_mono
  glyph_data_gray8   = 2,  //-----glyph_data_gray8
  glyph_data_outline = 3   //-----glyph_data_outline
};

//---------------------------------------------------------glyph_rendering
enum glyph_rendering
{
  glyph_ren_native_mono,    //-----glyph_ren_native_mono
  glyph_ren_native_gray8,   //-----glyph_ren_native_gray8
  glyph_ren_outline,        //-----glyph_ren_outline
  glyph_ren_agg_mono,       //-----glyph_ren_agg_mono
  glyph_ren_agg_gray8       //-----glyph_ren_agg_gray8
};

//-----------------------------------------------font_engine_freetype_base
class font_engine_freetype_base
{
 public:
  //--------------------------------------------------------------------
  typedef serialized_scanlines_adaptor_aa<int8u>    gray8_adaptor_type;
  typedef serialized_scanlines_adaptor_bin          mono_adaptor_type;
  typedef scanline_storage_aa8                      scanlines_aa_type;
  typedef scanline_storage_bin                      scanlines_bin_type;

  //--------------------------------------------------------------------
  ~font_engine_freetype_base();
  font_engine_freetype_base(bool flag32, unsigned max_faces = 32);

  // Set font parameters
  //--------------------------------------------------------------------
  void resolution(unsigned dpi);
  bool load_font(ni::sMyFTLibrary* apLib, const char* font_name, unsigned face_index,
                 glyph_rendering ren_type, ni::iFile* apFile);
  void set_glyph_rendering(glyph_rendering ren_type);
  glyph_rendering get_glyph_rendering() const;
  bool attach(const char* file_name);
  bool char_map(FT_Encoding map);
  bool height(agg_real h);
  bool width(agg_real w);
  void hinting(bool h);
  void flip_y(bool f);
  void transform(const trans_affine& affine);

  // Set Gamma
  //--------------------------------------------------------------------
  template<class GammaF> void gamma(const GammaF& f)
  {
    m_rasterizer.gamma(f);
  }

  // Accessors
  //--------------------------------------------------------------------
  int         last_error()   const { return m_last_error; }
  unsigned    resolution()   const { return m_resolution; }
  const char* name()         const { return m_name;       }
  unsigned    num_faces()    const;
  FT_Encoding char_map()     const { return m_char_map;   }
  agg_real      height()       const { return agg_real(agg_real(m_height) / 64.0);    }
  agg_real      width()        const { return agg_real(agg_real(m_width) / 64.0);     }
  agg_real      ascender()     const;
  agg_real      descender()    const;
  bool        hinting()      const { return m_hinting;    }
  bool        flip_y()       const { return m_flip_y;     }


  // Interface mandatory to implement for font_cache_manager
  //--------------------------------------------------------------------
  //const char*     font_signature() const { return m_signature;    }
  int             change_stamp()   const { return m_change_stamp; }

  bool      has_path() const;
#if niMinFeatures(20)
  bool        get_glyph_path(
      unsigned glyph_code,
      bool flip_y,
      bool hinting,
      const trans_affine& mtx,
      ni::iVGPath* apPath,
      agg_real* advance_x, agg_real* advance_y,
      agg_real* real_width, agg_real* real_height);
#endif
  bool            prepare_glyph(unsigned glyph_code);
  unsigned        glyph_index() const { return m_glyph_index; }
  unsigned        data_size()   const { return m_data_size;   }
  glyph_data_type data_type()   const { return m_data_type;   }
  const rect_i&   bounds()      const { return m_bounds;      }
  agg_real        advance_x()   const { return m_advance_x;   }
  agg_real        advance_y()   const { return m_advance_y;   }
  agg_real    real_width()  const { return m_real_width;  }
  agg_real    real_height() const { return m_real_height; }
  void            write_glyph_to(int8u* data) const;
  bool            add_kerning(unsigned first, unsigned second,
                              agg_real* x, agg_real* y);
  bool            get_kerning(unsigned first, unsigned second,
                              agg_real* x, agg_real* y);

  FT_Face get_face(ni::tU32 anIndex) const { return m_faces[anIndex]; }
  FT_Face get_cur_face() const { return m_cur_face; }

 public:
  font_engine_freetype_base(const font_engine_freetype_base&);
  const font_engine_freetype_base& operator = (const font_engine_freetype_base&);

  void update_char_size();
  int  find_face(const char* face_name) const;

  bool            m_flag32;
  int             m_change_stamp;
  int             m_last_error;
  char*           m_name;
  unsigned        m_name_len;
  unsigned        m_face_index;
  FT_Encoding     m_char_map;
  //char*           m_signature;
  unsigned        m_height;
  unsigned        m_width;
  bool            m_hinting;
  bool            m_flip_y;
  ni::tMyFTLibraryPtr mptrLibrary;
  FT_Face*        m_faces;      // A pool of font faces
  char**          m_face_names;
  unsigned        m_num_faces;
  unsigned        m_max_faces;
  FT_Face         m_cur_face;  // handle to the current face object
  int             m_resolution;
  glyph_rendering m_glyph_rendering;
  unsigned        m_glyph_index;
  unsigned        m_data_size;
  glyph_data_type m_data_type;
  rect_i          m_bounds;
  agg_real        m_advance_x;
  agg_real        m_advance_y;
  agg_real        m_bearing_x;
  agg_real        m_bearing_y;
  agg_real    m_real_width;
  agg_real    m_real_height;
  trans_affine    m_affine;

  path_storage_integer<int16, 6>              m_path16;
  path_storage_integer<int32, 6>              m_path32;
  conv_curve<path_storage_integer<int16, 6> > m_curves16;
  conv_curve<path_storage_integer<int32, 6> > m_curves32;
  scanline_u8              m_scanline_aa;
  scanline_bin             m_scanline_bin;
  scanlines_aa_type        m_scanlines_aa;
  scanlines_bin_type       m_scanlines_bin;
  rasterizer_scanline_aa<> m_rasterizer;
};




//------------------------------------------------font_engine_freetype_int16
// This class uses values of type int16 (10.6 format) for the vector cache.
// The vector cache is compact, but when rendering glyphs of height
// more that 200 there integer overflow can occur.
//
class font_engine_freetype_int16 : public font_engine_freetype_base
{
 public:
  typedef serialized_integer_path_adaptor<int16, 6>     path_adaptor_type;
  typedef font_engine_freetype_base::gray8_adaptor_type gray8_adaptor_type;
  typedef font_engine_freetype_base::mono_adaptor_type  mono_adaptor_type;
  typedef font_engine_freetype_base::scanlines_aa_type  scanlines_aa_type;
  typedef font_engine_freetype_base::scanlines_bin_type scanlines_bin_type;

  font_engine_freetype_int16(unsigned max_faces = 32) :
      font_engine_freetype_base(false, max_faces) {}
};

//------------------------------------------------font_engine_freetype_int32
// This class uses values of type int32 (26.6 format) for the vector cache.
// The vector cache is twice larger than in font_engine_freetype_int16,
// but it allows you to render glyphs of very large sizes.
//
class font_engine_freetype_int32 : public font_engine_freetype_base
{
 public:
  typedef serialized_integer_path_adaptor<int32, 6>     path_adaptor_type;
  typedef font_engine_freetype_base::gray8_adaptor_type gray8_adaptor_type;
  typedef font_engine_freetype_base::mono_adaptor_type  mono_adaptor_type;
  typedef font_engine_freetype_base::scanlines_aa_type  scanlines_aa_type;
  typedef font_engine_freetype_base::scanlines_bin_type scanlines_bin_type;

  font_engine_freetype_int32(unsigned max_faces = 32) :
      font_engine_freetype_base(true, max_faces) {}
};


}

typedef agg::font_engine_freetype_int32           tAGGFontEngine;

#endif
