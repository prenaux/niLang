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

#ifndef AGG_GAMMA_FUNCTIONS_INCLUDED
#define AGG_GAMMA_FUNCTIONS_INCLUDED

#include <math.h>
#include "agg_basics.h"

namespace agg
{
//===============================================================gamma_none
struct gamma_none
{
  agg_real operator()(agg_real x) const { return x; }
};


//==============================================================gamma_power
class gamma_power
{
 public:
  gamma_power() : m_gamma(1.0) {}
  gamma_power(agg_real g) : m_gamma(g) {}

  void gamma(agg_real g) { m_gamma = g; }
  agg_real gamma() const { return m_gamma; }

  agg_real operator() (agg_real x) const
  {
    return pow(x, m_gamma);
  }

 private:
  agg_real m_gamma;
};


//==========================================================gamma_threshold
class gamma_threshold
{
 public:
  gamma_threshold() : m_threshold(0.5) {}
  gamma_threshold(agg_real t) : m_threshold(t) {}

  void threshold(agg_real t) { m_threshold = t; }
  agg_real threshold() const { return m_threshold; }

  agg_real operator() (agg_real x) const
  {
    return (x < m_threshold) ? 0.0 : 1.0;
  }

 private:
  agg_real m_threshold;
};


//============================================================gamma_linear
class gamma_linear
{
 public:
  gamma_linear() : m_start(0.0), m_end(1.0) {}
  gamma_linear(agg_real s, agg_real e) : m_start(s), m_end(e) {}

  void set(agg_real s, agg_real e) { m_start = s; m_end = e; }
  void start(agg_real s) { m_start = s; }
  void end(agg_real e) { m_end = e; }
  agg_real start() const { return m_start; }
  agg_real end() const { return m_end; }

  agg_real operator() (agg_real x) const
  {
    if(x < m_start) return 0.0;
    if(x > m_end) return 1.0;
    return (x - m_start) / (m_end - m_start);
  }

 private:
  agg_real m_start;
  agg_real m_end;
};


//==========================================================gamma_multiply
class gamma_multiply
{
 public:
  gamma_multiply() : m_mul(1.0) {}
  gamma_multiply(agg_real v) : m_mul(v) {}

  void value(agg_real v) { m_mul = v; }
  agg_real value() const { return m_mul; }

  agg_real operator() (agg_real x) const
  {
    agg_real y = x * m_mul;
    if(y > 1.0) y = 1.0;
    return y;
  }

 private:
  agg_real m_mul;
};

}

#endif



