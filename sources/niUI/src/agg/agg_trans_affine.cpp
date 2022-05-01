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
// Affine transformations
//
//----------------------------------------------------------------------------
#include "agg_trans_affine.h"



namespace agg
{

//------------------------------------------------------------------------
const trans_affine& trans_affine::parl_to_parl(const agg_real* src,
                                               const agg_real* dst)
{
  m0 = src[2] - src[0];
  m1 = src[3] - src[1];
  m2 = src[4] - src[0];
  m3 = src[5] - src[1];
  m4 = src[0];
  m5 = src[1];
  invert();
  multiply(trans_affine(dst[2] - dst[0], dst[3] - dst[1],
                        dst[4] - dst[0], dst[5] - dst[1],
                        dst[0], dst[1]));
  return *this;
}

//------------------------------------------------------------------------
const trans_affine& trans_affine::rect_to_parl(agg_real x1, agg_real y1,
                                               agg_real x2, agg_real y2,
                                               const agg_real* parl)
{
  agg_real src[6];
  src[0] = x1; src[1] = y1;
  src[2] = x2; src[3] = y1;
  src[4] = x2; src[5] = y2;
  parl_to_parl(src, parl);
  return *this;
}

//------------------------------------------------------------------------
const trans_affine& trans_affine::parl_to_rect(const agg_real* parl,
                                               agg_real x1, agg_real y1,
                                               agg_real x2, agg_real y2)
{
  agg_real dst[6];
  dst[0] = x1; dst[1] = y1;
  dst[2] = x2; dst[3] = y1;
  dst[4] = x2; dst[5] = y2;
  parl_to_parl(parl, dst);
  return *this;
}

//------------------------------------------------------------------------
const trans_affine& trans_affine::multiply(const trans_affine& m)
{
  agg_real t0 = m0 * m.m0 + m1 * m.m2;
  agg_real t2 = m2 * m.m0 + m3 * m.m2;
  agg_real t4 = m4 * m.m0 + m5 * m.m2 + m.m4;
  m1 = m0 * m.m1 + m1 * m.m3;
  m3 = m2 * m.m1 + m3 * m.m3;
  m5 = m4 * m.m1 + m5 * m.m3 + m.m5;
  m0 = t0;
  m2 = t2;
  m4 = t4;
  return *this;
}


//------------------------------------------------------------------------
const trans_affine& trans_affine::invert()
{
  agg_real d  = determinant();

  agg_real t0 =  m3 * d;
  m3 =  m0 * d;
  m1 = -m1 * d;
  m2 = -m2 * d;

  agg_real t4 = -m4 * t0 - m5 * m2;
  m5 = -m4 * m1 - m5 * m3;

  m0 = t0;
  m4 = t4;
  return *this;
}


//------------------------------------------------------------------------
const trans_affine& trans_affine::flip_x()
{
  m0 = -m0;
  m1 = -m1;
  m4 = -m4;
  return *this;
}

//------------------------------------------------------------------------
const trans_affine& trans_affine::flip_y()
{
  m2 = -m2;
  m3 = -m3;
  m5 = -m5;
  return *this;
}

//------------------------------------------------------------------------
const trans_affine& trans_affine::reset()
{
  m0 = m3 = 1.0;
  m1 = m2 = m4 = m5 = 0.0;
  return *this;
}

//------------------------------------------------------------------------
inline bool is_equal_eps(agg_real v1, agg_real v2, agg_real epsilon)
{
  return fabs(v1 - v2) < epsilon;
}

//------------------------------------------------------------------------
bool trans_affine::is_identity(agg_real epsilon) const
{
  return is_equal_eps(m0, 1.0, epsilon) &&
      is_equal_eps(m1, 0.0, epsilon) &&
      is_equal_eps(m2, 0.0, epsilon) &&
      is_equal_eps(m3, 1.0, epsilon) &&
      is_equal_eps(m4, 0.0, epsilon) &&
      is_equal_eps(m5, 0.0, epsilon);
}

//------------------------------------------------------------------------
bool trans_affine::is_equal(const trans_affine& m, agg_real epsilon) const
{
  return is_equal_eps(m0, m.m0, epsilon) &&
      is_equal_eps(m1, m.m1, epsilon) &&
      is_equal_eps(m2, m.m2, epsilon) &&
      is_equal_eps(m3, m.m3, epsilon) &&
      is_equal_eps(m4, m.m4, epsilon) &&
      is_equal_eps(m5, m.m5, epsilon);
}

//------------------------------------------------------------------------
agg_real trans_affine::rotation() const
{
  agg_real x1 = 0.0;
  agg_real y1 = 0.0;
  agg_real x2 = 1.0;
  agg_real y2 = 0.0;
  transform(&x1, &y1);
  transform(&x2, &y2);
  return atan2(y2-y1, x2-x1);
}

//------------------------------------------------------------------------
void trans_affine::translation(agg_real* dx, agg_real* dy) const
{
  trans_affine t(*this);
  t *= trans_affine_rotation(-rotation());
  t.transform(dx, dy);
}

//------------------------------------------------------------------------
void trans_affine::scaling(agg_real* sx, agg_real* sy) const
{
  agg_real x1 = 0.0;
  agg_real y1 = 0.0;
  agg_real x2 = 1.0;
  agg_real y2 = 1.0;
  trans_affine t(*this);
  t *= trans_affine_rotation(-rotation());
  t.transform(&x1, &y1);
  t.transform(&x2, &y2);
  *sx = x2 - x1;
  *sy = y2 - y1;
}


}

