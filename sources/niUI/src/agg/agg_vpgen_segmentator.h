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

#ifndef AGG_VPGEN_SEGMENTATOR_INCLUDED
#define AGG_VPGEN_SEGMENTATOR_INCLUDED

#include <math.h>
#include "agg_basics.h"

namespace agg
{

//=======================================================vpgen_segmentator
//
// See Implementation agg_vpgen_segmentator.cpp
//
class vpgen_segmentator
{
 public:
  vpgen_segmentator() : m_approximation_scale(1.0) {}

  void approximation_scale(agg_real s) { m_approximation_scale = s;     }
  agg_real approximation_scale() const { return m_approximation_scale;  }

  static bool auto_close()   { return false; }
  static bool auto_unclose() { return false; }

  void reset() { m_cmd = path_cmd_stop; }
  void move_to(agg_real x, agg_real y);
  void line_to(agg_real x, agg_real y);
  unsigned vertex(agg_real* x, agg_real* y);

 private:
  agg_real   m_approximation_scale;
  agg_real   m_x1;
  agg_real   m_y1;
  agg_real   m_dx;
  agg_real   m_dy;
  agg_real   m_dl;
  agg_real   m_ddl;
  unsigned m_cmd;
};



}

#endif

