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

#include <math.h>
#include "agg_trans_warp_magnifier.h"

namespace agg
{

//------------------------------------------------------------------------
void trans_warp_magnifier::transform(agg_real* x, agg_real* y) const
{
  agg_real dx = *x - m_xc;
  agg_real dy = *y - m_yc;
  agg_real r = sqrt(dx * dx + dy * dy);
  if(r < m_radius)
  {
    *x = m_xc + dx * m_magn;
    *y = m_yc + dy * m_magn;
    return;
  }

  agg_real m = (r + m_radius * (m_magn - 1.0)) / r;
  *x = m_xc + dx * m;
  *y = m_yc + dy * m;
}

//------------------------------------------------------------------------
void trans_warp_magnifier::inverse_transform(agg_real* x, agg_real* y) const
{
  trans_warp_magnifier t(*this);
  t.magnification(1.0 / m_magn);
  t.radius(m_radius * m_magn);
  t.transform(x, y);
}


}
