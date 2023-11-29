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

#ifndef AGG_WARP_MAGNIFIER_INCLUDED
#define AGG_WARP_MAGNIFIER_INCLUDED

#include "agg_basics.h"

namespace agg
{

//----------------------------------------------------trans_warp_magnifier
//
// See Inmplementation agg_trans_warp_magnifier.cpp
//
class trans_warp_magnifier
{
 public:
  trans_warp_magnifier() : m_xc(0.0), m_yc(0.0), m_magn(1.0), m_radius(1.0) {}

  void center(agg_real x, agg_real y) { m_xc = x; m_yc = y; }
  void magnification(agg_real m)    { m_magn = m;         }
  void radius(agg_real r)           { m_radius = r;       }

  void transform(agg_real* x, agg_real* y) const;
  void inverse_transform(agg_real* x, agg_real* y) const;

 private:
  agg_real m_xc;
  agg_real m_yc;
  agg_real m_magn;
  agg_real m_radius;
};


}


#endif
