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
// Bilinear 2D transformations
//
//----------------------------------------------------------------------------
#ifndef AGG_TRANS_BILINEAR_INCLUDED
#define AGG_TRANS_BILINEAR_INCLUDED

#include "agg_basics.h"
#include "agg_simul_eq.h"

namespace agg
{

//==========================================================trans_bilinear
class trans_bilinear
{
 public:
  //--------------------------------------------------------------------
  trans_bilinear() : m_valid(false) {}

  //--------------------------------------------------------------------
  // Arbitrary quadrangle transformations
  trans_bilinear(const agg_real* src, const agg_real* dst)
  {
    quad_to_quad(src, dst);
  }


  //--------------------------------------------------------------------
  // Direct transformations
  trans_bilinear(agg_real x1, agg_real y1, agg_real x2, agg_real y2,
                 const agg_real* quad)
  {
    rect_to_quad(x1, y1, x2, y2, quad);
  }


  //--------------------------------------------------------------------
  // Reverse transformations
  trans_bilinear(const agg_real* quad,
                 agg_real x1, agg_real y1, agg_real x2, agg_real y2)
  {
    quad_to_rect(quad, x1, y1, x2, y2);
  }


  //--------------------------------------------------------------------
  // Set the transformations using two arbitrary quadrangles.
  void quad_to_quad(const agg_real* src, const agg_real* dst)
  {
    agg_real left[4][4];
    agg_real right[4][2];

    unsigned i;
    for(i = 0; i < 4; i++)
    {
      unsigned ix = i * 2;
      unsigned iy = ix + 1;
      left[i][0] = 1.0;
      left[i][1] = src[ix] * src[iy];
      left[i][2] = src[ix];
      left[i][3] = src[iy];

      right[i][0] = dst[ix];
      right[i][1] = dst[iy];
    }
    m_valid = simul_eq<4, 2>::solve(left, right, m_mtx);
  }


  //--------------------------------------------------------------------
  // Set the direct transformations, i.e., rectangle -> quadrangle
  void rect_to_quad(agg_real x1, agg_real y1, agg_real x2, agg_real y2,
                    const agg_real* quad)
  {
    agg_real src[8];
    src[0] = src[6] = x1;
    src[2] = src[4] = x2;
    src[1] = src[3] = y1;
    src[5] = src[7] = y2;
    quad_to_quad(src, quad);
  }


  //--------------------------------------------------------------------
  // Set the reverse transformations, i.e., quadrangle -> rectangle
  void quad_to_rect(const agg_real* quad,
                    agg_real x1, agg_real y1, agg_real x2, agg_real y2)
  {
    agg_real dst[8];
    dst[0] = dst[6] = x1;
    dst[2] = dst[4] = x2;
    dst[1] = dst[3] = y1;
    dst[5] = dst[7] = y2;
    quad_to_quad(quad, dst);
  }

  //--------------------------------------------------------------------
  // Check if the equations were solved successfully
  bool is_valid() const { return m_valid; }

  //--------------------------------------------------------------------
  // Transform a point (x, y)
  void transform(agg_real* x, agg_real* y) const
  {
    agg_real tx = *x;
    agg_real ty = *y;
    agg_real xy = tx * ty;
    *x = m_mtx[0][0] + m_mtx[1][0] * xy + m_mtx[2][0] * tx + m_mtx[3][0] * ty;
    *y = m_mtx[0][1] + m_mtx[1][1] * xy + m_mtx[2][1] * tx + m_mtx[3][1] * ty;
  }


  //--------------------------------------------------------------------
  class iterator_x
  {
    agg_real inc_x;
    agg_real inc_y;

   public:
    agg_real x;
    agg_real y;

    iterator_x() {}
    iterator_x(agg_real tx, agg_real ty, agg_real step, const agg_real m[4][2]) :
        inc_x(m[1][0] * step * ty + m[2][0] * step),
        inc_y(m[1][1] * step * ty + m[2][1] * step),
        x(m[0][0] + m[1][0] * tx * ty + m[2][0] * tx + m[3][0] * ty),
        y(m[0][1] + m[1][1] * tx * ty + m[2][1] * tx + m[3][1] * ty)
    {
    }

    void operator ++ ()
    {
      x += inc_x;
      y += inc_y;
    }
  };

  iterator_x begin(agg_real x, agg_real y, agg_real step) const
  {
    return iterator_x(x, y, step, m_mtx);
  }

 private:
  agg_real m_mtx[4][2];
  bool   m_valid;
};

}

#endif
