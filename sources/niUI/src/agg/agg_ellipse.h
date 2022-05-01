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
// class ellipse
//
//----------------------------------------------------------------------------

#ifndef AGG_ELLIPSE_INCLUDED
#define AGG_ELLIPSE_INCLUDED

#include "agg_basics.h"
#include <math.h>

namespace agg
{

//----------------------------------------------------------------ellipse
class ellipse
{
 public:
  ellipse() :
      m_x(0.0), m_y(0.0), m_rx(1.0), m_ry(1.0), m_scale(1.0),
      m_num(4), m_step(0), m_cw(false) {}

  ellipse(agg_real x, agg_real y, agg_real rx, agg_real ry,
          unsigned num_steps=0, bool cw=false) :
      m_x(x), m_y(y), m_rx(rx), m_ry(ry), m_scale(1.0),
      m_num(num_steps), m_step(0), m_cw(cw)
  {
    if(m_num == 0) calc_num_steps();
  }

  void init(agg_real x, agg_real y, agg_real rx, agg_real ry,
            unsigned num_steps=0, bool cw=false);

  void approximation_scale(agg_real scale);
  void rewind(unsigned path_id);
  unsigned vertex(agg_real* x, agg_real* y);

 private:
  void calc_num_steps();

  agg_real m_x;
  agg_real m_y;
  agg_real m_rx;
  agg_real m_ry;
  agg_real m_scale;
  unsigned m_num;
  unsigned m_step;
  bool m_cw;
};

//------------------------------------------------------------------------
inline void ellipse::init(agg_real x, agg_real y, agg_real rx, agg_real ry,
                          unsigned num_steps, bool cw)
{
  m_x = x;
  m_y = y;
  m_rx = rx;
  m_ry = ry;
  m_num = num_steps;
  m_step = 0;
  m_cw = cw;
  if(m_num == 0) calc_num_steps();
}

//------------------------------------------------------------------------
inline void ellipse::approximation_scale(agg_real scale)
{
  m_scale = scale;
  calc_num_steps();
}

//------------------------------------------------------------------------
inline void ellipse::calc_num_steps()
{
  agg_real ra = (fabs(m_rx) + fabs(m_ry)) / 2;
  agg_real da = acos(ra / (ra + 0.125 / m_scale)) * 2;
  m_num = uround(2*pi / da);
}

//------------------------------------------------------------------------
inline void ellipse::rewind(unsigned)
{
  m_step = 0;
}

//------------------------------------------------------------------------
inline unsigned ellipse::vertex(agg_real* x, agg_real* y)
{
  if(m_step == m_num)
  {
    ++m_step;
    return path_cmd_end_poly | path_flags_close | path_flags_ccw;
  }
  if(m_step > m_num) return path_cmd_stop;
  agg_real angle = agg_real(m_step) / agg_real(m_num) * 2.0 * pi;
  if(m_cw) angle = 2.0 * pi - angle;
  *x = m_x + cos(angle) * m_rx;
  *y = m_y + sin(angle) * m_ry;
  m_step++;
  return ((m_step == 1) ? path_cmd_move_to : path_cmd_line_to);
}

}



#endif


