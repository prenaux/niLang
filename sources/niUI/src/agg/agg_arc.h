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
// Arc vertex generator
//
//----------------------------------------------------------------------------

#ifndef AGG_ARC_INCLUDED
#define AGG_ARC_INCLUDED

#include <math.h>
#include "agg_basics.h"

namespace agg
{

//=====================================================================arc
//
// See Implementation agg_arc.cpp
//
class arc
{
 public:
  arc() : m_scale(1.0), m_initialized(false) {}
  arc(agg_real x,  agg_real y,
      agg_real rx, agg_real ry,
      agg_real a1, agg_real a2,
      bool ccw=true);

  void init(agg_real x,  agg_real y,
            agg_real rx, agg_real ry,
            agg_real a1, agg_real a2,
            bool ccw=true);

  void approximation_scale(agg_real s);
  agg_real approximation_scale() const { return m_scale;  }

  void rewind(unsigned);
  unsigned vertex(agg_real* x, agg_real* y);

 private:
  void normalize(agg_real a1, agg_real a2, bool ccw);

  agg_real   m_x;
  agg_real   m_y;
  agg_real   m_rx;
  agg_real   m_ry;
  agg_real   m_angle;
  agg_real   m_start;
  agg_real   m_end;
  agg_real   m_scale;
  agg_real   m_da;
  bool     m_ccw;
  bool     m_initialized;
  unsigned m_path_cmd;
};


}


#endif
