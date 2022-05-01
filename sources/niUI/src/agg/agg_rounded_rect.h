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
// Rounded rectangle vertex generator
//
//----------------------------------------------------------------------------

#ifndef AGG_ROUNDED_RECT_INCLUDED
#define AGG_ROUNDED_RECT_INCLUDED

#include "agg_basics.h"
#include "agg_arc.h"

namespace agg
{
//------------------------------------------------------------rounded_rect
//
// See Implemantation agg_rounded_rect.cpp
//
class rounded_rect
{
 public:
  rounded_rect() {}
  rounded_rect(agg_real x1, agg_real y1, agg_real x2, agg_real y2, agg_real r);

  void rect(agg_real x1, agg_real y1, agg_real x2, agg_real y2);
  void radius(agg_real r);
  void radius(agg_real rx, agg_real ry);
  void radius(agg_real rx_bottom, agg_real ry_bottom, agg_real rx_top, agg_real ry_top);
  void radius(agg_real rx1, agg_real ry1, agg_real rx2, agg_real ry2,
              agg_real rx3, agg_real ry3, agg_real rx4, agg_real ry4);
  void normalize_radius();

  void approximation_scale(agg_real s) { m_arc.approximation_scale(s); }
  agg_real approximation_scale() const { return m_arc.approximation_scale(); }

  void rewind(unsigned);
  unsigned vertex(agg_real* x, agg_real* y);

 private:
  agg_real m_x1;
  agg_real m_y1;
  agg_real m_x2;
  agg_real m_y2;
  agg_real m_rx1;
  agg_real m_ry1;
  agg_real m_rx2;
  agg_real m_ry2;
  agg_real m_rx3;
  agg_real m_ry3;
  agg_real m_rx4;
  agg_real m_ry4;
  unsigned m_status;
  arc      m_arc;
};

}

#endif

