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
// Image transformations with filtering. Span generator base class
//
//----------------------------------------------------------------------------
#ifndef AGG_SPAN_IMAGE_FILTER_INCLUDED
#define AGG_SPAN_IMAGE_FILTER_INCLUDED

#include "agg_basics.h"
#include "agg_image_filters.h"
#include "agg_span_interpolator_linear.h"

namespace agg
{

//-------------------------------------------------------span_image_filter
template<class Source, class Interpolator> class span_image_filter
{
 public:
  typedef Source source_type;
  typedef Interpolator interpolator_type;

  //--------------------------------------------------------------------
  span_image_filter() {}
  span_image_filter(source_type& src,
                    interpolator_type& interpolator,
                    const image_filter_lut* filter) :
      m_src(&src),
      m_interpolator(&interpolator),
      m_filter(filter),
      m_dx_dbl(0.5),
      m_dy_dbl(0.5),
      m_dx_int(image_subpixel_scale / 2),
      m_dy_int(image_subpixel_scale / 2)
  {}
  void attach(source_type& v) { m_src = &v; }

  //--------------------------------------------------------------------
  source_type& source()            { return *m_src; }
  const  source_type& source()      const { return *m_src; }
  const  image_filter_lut& filter() const { return *m_filter; }
  int    filter_dx_int()            const { return m_dx_int; }
  int    filter_dy_int()            const { return m_dy_int; }
  agg_real filter_dx_dbl()            const { return m_dx_dbl; }
  agg_real filter_dy_dbl()            const { return m_dy_dbl; }

  //--------------------------------------------------------------------
  void interpolator(interpolator_type& v)  { m_interpolator = &v; }
  void filter(const image_filter_lut& v)   { m_filter = &v; }
  void filter_offset(agg_real dx, agg_real dy)
  {
    m_dx_dbl = dx;
    m_dy_dbl = dy;
    m_dx_int = iround(dx * image_subpixel_scale);
    m_dy_int = iround(dy * image_subpixel_scale);
  }
  void filter_offset(agg_real d) { filter_offset(d, d); }

  //--------------------------------------------------------------------
  interpolator_type& interpolator() { return *m_interpolator; }

  //--------------------------------------------------------------------
  void prepare() {}

  //--------------------------------------------------------------------
 private:
  source_type*            m_src;
  interpolator_type*      m_interpolator;
  const image_filter_lut* m_filter;
  agg_real   m_dx_dbl;
  agg_real   m_dy_dbl;
  unsigned m_dx_int;
  unsigned m_dy_int;
};




//==============================================span_image_resample_affine
template<class Source>
class span_image_resample_affine :
      public span_image_filter<Source, span_interpolator_linear<trans_affine> >
{
 public:
  typedef Source source_type;
  typedef span_interpolator_linear<trans_affine> interpolator_type;
  typedef span_image_filter<source_type, interpolator_type> base_type;

  //--------------------------------------------------------------------
  span_image_resample_affine() :
      m_scale_limit(200.0),
      m_blur_x(1.0),
      m_blur_y(1.0)
  {}

  //--------------------------------------------------------------------
  span_image_resample_affine(source_type& src,
                             interpolator_type& inter,
                             const image_filter_lut& filter) :
      base_type(src, inter, &filter),
      m_scale_limit(200.0),
      m_blur_x(1.0),
      m_blur_y(1.0)
  {}


  //--------------------------------------------------------------------
  int  scale_limit() const { return uround(m_scale_limit); }
  void scale_limit(int v)  { m_scale_limit = v; }

  //--------------------------------------------------------------------
  agg_real blur_x() const { return m_blur_x; }
  agg_real blur_y() const { return m_blur_y; }
  void blur_x(agg_real v) { m_blur_x = v; }
  void blur_y(agg_real v) { m_blur_y = v; }
  void blur(agg_real v) { m_blur_x = m_blur_y = v; }

  //--------------------------------------------------------------------
  void prepare()
  {
    agg_real scale_x;
    agg_real scale_y;

    base_type::interpolator().transformer().scaling_abs(&scale_x, &scale_y);

    m_rx     = image_subpixel_scale;
    m_ry     = image_subpixel_scale;
    m_rx_inv = image_subpixel_scale;
    m_ry_inv = image_subpixel_scale;

    scale_x *= m_blur_x;
    scale_y *= m_blur_y;

    if(scale_x * scale_y > m_scale_limit)
    {
      scale_x = scale_x * m_scale_limit / (scale_x * scale_y);
      scale_y = scale_y * m_scale_limit / (scale_x * scale_y);
    }

    if(scale_x > 1.0001)
    {
      if(scale_x > m_scale_limit) scale_x = m_scale_limit;
      m_rx     = uround(    scale_x * agg_real(image_subpixel_scale));
      m_rx_inv = uround(1.0/scale_x * agg_real(image_subpixel_scale));
    }

    if(scale_y > 1.0001)
    {
      if(scale_y > m_scale_limit) scale_y = m_scale_limit;
      m_ry     = uround(    scale_y * agg_real(image_subpixel_scale));
      m_ry_inv = uround(1.0/scale_y * agg_real(image_subpixel_scale));
    }
  }

 protected:
  int m_rx;
  int m_ry;
  int m_rx_inv;
  int m_ry_inv;

 private:
  agg_real m_scale_limit;
  agg_real m_blur_x;
  agg_real m_blur_y;
};



//=====================================================span_image_resample
template<class Source, class Interpolator>
class span_image_resample :
      public span_image_filter<Source, Interpolator>
{
 public:
  typedef Source source_type;
  typedef Interpolator interpolator_type;
  typedef span_image_filter<source_type, interpolator_type> base_type;

  //--------------------------------------------------------------------
  span_image_resample() :
      m_scale_limit(20),
      m_blur_x(image_subpixel_scale),
      m_blur_y(image_subpixel_scale)
  {}

  //--------------------------------------------------------------------
  span_image_resample(source_type& src,
                      interpolator_type& inter,
                      const image_filter_lut& filter) :
      base_type(src, inter, &filter),
      m_scale_limit(20),
      m_blur_x(image_subpixel_scale),
      m_blur_y(image_subpixel_scale)
  {}

  //--------------------------------------------------------------------
  int  scale_limit() const { return m_scale_limit; }
  void scale_limit(int v)  { m_scale_limit = v; }

  //--------------------------------------------------------------------
  agg_real blur_x() const { return agg_real(m_blur_x) / agg_real(image_subpixel_scale); }
  agg_real blur_y() const { return agg_real(m_blur_y) / agg_real(image_subpixel_scale); }
  void blur_x(agg_real v) { m_blur_x = uround(v * agg_real(image_subpixel_scale)); }
  void blur_y(agg_real v) { m_blur_y = uround(v * agg_real(image_subpixel_scale)); }
  void blur(agg_real v)   { m_blur_x =
        m_blur_y = uround(v * agg_real(image_subpixel_scale)); }

 protected:
  int m_scale_limit;
  int m_blur_x;
  int m_blur_y;
};




}

#endif
