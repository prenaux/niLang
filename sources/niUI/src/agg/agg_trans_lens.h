//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.1
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

#ifndef AGG_WARP_MAGNIFIER_INCLUDED
#define AGG_WARP_MAGNIFIER_INCLUDED

#include <math.h>
#include "agg_basics.h"


namespace agg
{

class trans_warp_magnifier
{
 public:
  trans_warp_magnifier() : m_xc(0.0), m_yc(0.0), m_magn(1.0), m_radius(1.0), m_warp(false) {}

  void center(agg_real x, agg_real y) { m_xc = x; m_yc = y; }
  void magnification(agg_real m)    { m_magn = m;         }
  void radius(agg_real r)           { m_radius = r;       }
  void warp(bool w)               { m_warp = w;         }

  void transform(agg_real* x, agg_real* y) const
  {
    agg_real dx = *x - m_xc;
    agg_real dy = *y - m_yc;
    agg_real r = sqrt(dx * dx + dy * dy);
    agg_real rm = m_radius / m_magn;
    if(r < rm)
    {
      *x = m_xc + dx * m_magn;
      *y = m_yc + dy * m_magn;
      return;
    }

    if(m_warp)
    {
      agg_real m = (r + rm * (m_magn - 1.0)) / r;
      *x = m_xc + dx * m;
      *y = m_yc + dy * m;
      return;
    }

    if(r < m_radius)
    {
      agg_real m = m_radius / r;
      *x = m_xc + dx * m;
      *y = m_yc + dy * m;
    }
  }

 private:
  agg_real m_xc;
  agg_real m_yc;
  agg_real m_magn;
  agg_real m_radius;
  bool   m_warp;
};



}


#endif

