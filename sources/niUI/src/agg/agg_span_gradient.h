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

#ifndef AGG_SPAN_GRADIENT_INCLUDED
#define AGG_SPAN_GRADIENT_INCLUDED

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "agg_basics.h"
#include "agg_math.h"
#include "agg_array.h"


namespace agg
{

enum gradient_subpixel_scale_e
{
  gradient_subpixel_shift = 4,                              //-----gradient_subpixel_shift
  gradient_subpixel_scale = 1 << gradient_subpixel_shift,   //-----gradient_subpixel_scale
  gradient_subpixel_mask  = gradient_subpixel_scale - 1     //-----gradient_subpixel_mask
};



//==========================================================span_gradient
template<class ColorT,
         class Interpolator,
         class GradientF,
         class ColorF>
class span_gradient
{
 public:
  typedef Interpolator interpolator_type;
  typedef ColorT color_type;

  enum downscale_shift_e
  {
    downscale_shift = interpolator_type::subpixel_shift -
    gradient_subpixel_shift
  };

  //--------------------------------------------------------------------
  span_gradient() {}

  //--------------------------------------------------------------------
  span_gradient(interpolator_type& inter,
                const GradientF& gradient_function,
                const ColorF& color_function,
                agg_real d1, agg_real d2) :
      m_interpolator(&inter),
      m_gradient_function(&gradient_function),
      m_color_function(&color_function),
      m_d1(iround(d1 * gradient_subpixel_scale)),
      m_d2(iround(d2 * gradient_subpixel_scale))
  {}

  //--------------------------------------------------------------------
  interpolator_type& interpolator() { return *m_interpolator; }
  const GradientF& gradient_function() const { return *m_gradient_function; }
  const ColorF& color_function() const { return *m_color_function; }
  agg_real d1() const { return agg_real(m_d1) / gradient_subpixel_scale; }
  agg_real d2() const { return agg_real(m_d2) / gradient_subpixel_scale; }

  //--------------------------------------------------------------------
  void interpolator(interpolator_type& i) { m_interpolator = &i; }
  void gradient_function(const GradientF& gf) { m_gradient_function = &gf; }
  void color_function(const ColorF& cf) { m_color_function = &cf; }
  void d1(agg_real v) { m_d1 = iround(v * gradient_subpixel_scale); }
  void d2(agg_real v) { m_d2 = iround(v * gradient_subpixel_scale); }

  //--------------------------------------------------------------------
  void prepare() {}

  //--------------------------------------------------------------------
  void generate(color_type* span, int x, int y, unsigned len)
  {
    int dd = m_d2 - m_d1;
    if(dd < 1) dd = 1;
    m_interpolator->begin(x+0.5, y+0.5, len);
    do
    {
      m_interpolator->coordinates(&x, &y);
      int d = m_gradient_function->calculate(x >> downscale_shift,
                                             y >> downscale_shift, m_d2);
      d = ((d - m_d1) * (int)m_color_function->size()) / dd;
      if(d < 0) d = 0;
      if(d >= (int)m_color_function->size()) d = m_color_function->size() - 1;
      *span++ = (*m_color_function)[d];
      ++(*m_interpolator);
    }
    while(--len);
  }

 private:
  interpolator_type* m_interpolator;
  const GradientF*   m_gradient_function;
  const ColorF*      m_color_function;
  int                m_d1;
  int                m_d2;
};




//=====================================================gradient_linear_color
template<class ColorT>
struct gradient_linear_color
{
  typedef ColorT color_type;

  gradient_linear_color() {}
  gradient_linear_color(const color_type& c1, const color_type& c2,
                        unsigned size = 256) :
      m_c1(c1), m_c2(c2), m_size(size) {}

  unsigned size() const { return m_size; }
  color_type operator [] (unsigned v) const
  {
    return m_c1.gradient(m_c2, agg_real(v) / agg_real(m_size - 1));
  }

  void colors(const color_type& c1, const color_type& c2, unsigned size = 256)
  {
    m_c1 = c1;
    m_c2 = c2;
    m_size = size;
  }

  color_type m_c1;
  color_type m_c2;
  unsigned m_size;
};


//==========================================================gradient_circle
class gradient_circle
{
  // Actually the same as radial. Just for compatibility
 public:
  static AGG_INLINE int calculate(int x, int y, int)
  {
    return int(fast_sqrt(x*x + y*y));
  }
};


//==========================================================gradient_radial
class gradient_radial
{
 public:
  static AGG_INLINE int calculate(int x, int y, int)
  {
    return int(fast_sqrt(x*x + y*y));
  }
};


//========================================================gradient_radial_d
class gradient_radial_d
{
 public:
  static AGG_INLINE int calculate(int x, int y, int)
  {
    return uround(sqrt(agg_real(x)*agg_real(x) + agg_real(y)*agg_real(y)));
  }
};


//====================================================gradient_radial_focus
class gradient_radial_focus
{
 public:
  //---------------------------------------------------------------------
  gradient_radial_focus() :
      m_radius(100 * gradient_subpixel_scale),
      m_focus_x(0),
      m_focus_y(0)
  {
    update_values();
  }

  //---------------------------------------------------------------------
  gradient_radial_focus(agg_real r, agg_real fx, agg_real fy) :
      m_radius (iround(r  * gradient_subpixel_scale)),
      m_focus_x(iround(fx * gradient_subpixel_scale)),
      m_focus_y(iround(fy * gradient_subpixel_scale))
  {
    update_values();
  }

  //---------------------------------------------------------------------
  void init(agg_real r, agg_real fx, agg_real fy)
  {
    m_radius  = iround(r  * gradient_subpixel_scale);
    m_focus_x = iround(fx * gradient_subpixel_scale);
    m_focus_y = iround(fy * gradient_subpixel_scale);
    update_values();
  }

  //---------------------------------------------------------------------
  agg_real radius()  const { return agg_real(m_radius)  / gradient_subpixel_scale; }
  agg_real focus_x() const { return agg_real(m_focus_x) / gradient_subpixel_scale; }
  agg_real focus_y() const { return agg_real(m_focus_y) / gradient_subpixel_scale; }

  //---------------------------------------------------------------------
  int calculate(int x, int y, int) const
  {
    agg_real solution_x;
    agg_real solution_y;

    // Special case to avoid divide by zero or very near zero
    //---------------------------------
    if(x == iround(m_focus_x))
    {
      solution_x = m_focus_x;
      solution_y = 0.0;
      solution_y += (y > m_focus_y) ? m_trivial : -m_trivial;
    }
    else
    {
      // Slope of the focus-current line
      //-------------------------------
      agg_real slope = agg_real(y - m_focus_y) / agg_real(x - m_focus_x);

      // y-intercept of that same line
      //--------------------------------
      agg_real yint  = agg_real(y) - (slope * x);

      // Use the classical quadratic formula to calculate
      // the intersection point
      //--------------------------------
      agg_real a = (slope * slope) + 1;
      agg_real b =  2 * slope * yint;
      agg_real c =  yint * yint - m_radius2;
      agg_real det = sqrt((b * b) - (4.0 * a * c));
      solution_x = -b;

      // Choose the positive or negative root depending
      // on where the X coord lies with respect to the focus.
      solution_x += (x < m_focus_x) ? -det : det;
      solution_x /= 2.0 * a;

      // Calculating of Y is trivial
      solution_y  = (slope * solution_x) + yint;
    }

    // Calculate the percentage (0...1) of the current point along the
    // focus-circumference line and return the normalized (0...d) value
    //-------------------------------
    solution_x -= agg_real(m_focus_x);
    solution_y -= agg_real(m_focus_y);
    agg_real int_to_focus = solution_x * solution_x + solution_y * solution_y;
    agg_real cur_to_focus = agg_real(x - m_focus_x) * agg_real(x - m_focus_x) +
        agg_real(y - m_focus_y) * agg_real(y - m_focus_y);

    return iround(sqrt(cur_to_focus / int_to_focus) * m_radius);
  }

 private:
  //---------------------------------------------------------------------
  void update_values()
  {
    // For use in the quadratic equation
    //-------------------------------
    m_radius2 = agg_real(m_radius) * agg_real(m_radius);

    agg_real dist = sqrt(agg_real(m_focus_x) * agg_real(m_focus_x) +
                         agg_real(m_focus_y) * agg_real(m_focus_y));

    // Test if distance from focus to center is greater than the radius
    // For the sake of assurance factor restrict the point to be
    // no further than 99% of the radius.
    //-------------------------------
    agg_real r = m_radius * 0.99;
    if(dist > r)
    {
      // clamp focus to radius
      // x = r cos theta, y = r sin theta
      //------------------------
      agg_real a = atan2(agg_real(m_focus_y), agg_real(m_focus_x));
      m_focus_x = iround(r * cos(a));
      m_focus_y = iround(r * sin(a));
    }

    // Calculate the solution to be used in the case where x == focus_x
    //------------------------------
    m_trivial = sqrt(m_radius2 - (m_focus_x * m_focus_x));
  }

  int m_radius;
  int m_focus_x;
  int m_focus_y;
  agg_real m_radius2;
  agg_real m_trivial;
};



//==============================================================gradient_x
class gradient_x
{
 public:
  static int calculate(int x, int, int) { return x; }
};


//==============================================================gradient_y
class gradient_y
{
 public:
  static int calculate(int, int y, int) { return y; }
};


//========================================================gradient_diamond
class gradient_diamond
{
 public:
  static AGG_INLINE int calculate(int x, int y, int)
  {
    int ax = abs(x);
    int ay = abs(y);
    return ax > ay ? ax : ay;
  }
};


//=============================================================gradient_xy
class gradient_xy
{
 public:
  static AGG_INLINE int calculate(int x, int y, int d)
  {
    return abs(x) * abs(y) / d;
  }
};


//========================================================gradient_sqrt_xy
class gradient_sqrt_xy
{
 public:
  static AGG_INLINE int calculate(int x, int y, int)
  {
    return fast_sqrt(abs(x) * abs(y));
  }
};


//==========================================================gradient_conic
class gradient_conic
{
 public:
  static AGG_INLINE int calculate(int x, int y, int d)
  {
    return uround(fabs(atan2(agg_real(y), agg_real(x))) * agg_real(d) / pi);
  }
};


//=================================================gradient_repeat_adaptor
template<class GradientF> class gradient_repeat_adaptor
{
 public:
  gradient_repeat_adaptor(const GradientF& gradient) :
      m_gradient(&gradient) {}

  AGG_INLINE int calculate(int x, int y, int d) const
  {
    int ret = m_gradient->calculate(x, y, d) % d;
    if(ret < 0) ret += d;
    return ret;
  }

 private:
  const GradientF* m_gradient;
};


//================================================gradient_reflect_adaptor
template<class GradientF> class gradient_reflect_adaptor
{
 public:
  gradient_reflect_adaptor(const GradientF& gradient) :
      m_gradient(&gradient) {}

  AGG_INLINE int calculate(int x, int y, int d) const
  {
    int d2 = d << 1;
    int ret = m_gradient->calculate(x, y, d) % d2;
    if(ret <  0) ret += d2;
    if(ret >= d) ret  = d2 - ret;
    return ret;
  }

 private:
  const GradientF* m_gradient;
};

//================================================gradient_clamp_adaptor
template<class GradientF> class gradient_clamp_adaptor
{
 public:
  gradient_clamp_adaptor(const GradientF& gradient) :
      m_gradient(&gradient) {}

  AGG_INLINE int calculate(int x, int y, int d) const
  {
    int ret = m_gradient->calculate(x, y, d);
    if(ret <  0) {
      ret = 0;
    }
    if(ret >= d) {
      ret = d-1;
    }
    return ret;
  }

 private:
  const GradientF* m_gradient;
};
}

#endif
