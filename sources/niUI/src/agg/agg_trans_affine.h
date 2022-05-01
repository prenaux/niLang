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
// Affine transformation classes.
//
//----------------------------------------------------------------------------
#ifndef AGG_TRANS_AFFINE_INCLUDED
#define AGG_TRANS_AFFINE_INCLUDED

#include <math.h>
#include "agg_basics.h"

namespace agg
{
const agg_real affine_epsilon = 1e-14; // About of precision of doubles

//============================================================trans_affine
//
// See Implementation agg_trans_affine.cpp
//
// Affine transformation are linear transformations in Cartesian coordinates
// (strictly speaking not only in Cartesian, but for the beginning we will
// think so). They are rotation, scaling, translation and skewing.
// After any affine transformation a line segment remains a line segment
// and it will never become a curve.
//
// There will be no math about matrix calculations, since it has been
// described many times. Ask yourself a very simple question:
// "why do we need to understand and use some matrix stuff instead of just
// rotating, scaling and so on". The answers are:
//
// 1. Any combination of transformations can be done by only 4 multiplications
//    and 4 additions in floating point.
// 2. One matrix transformation is equivalent to the number of consecutive
//    discrete transformations, i.e. the matrix "accumulates" all transformations
//    in the order of their settings. Suppose we have 4 transformations:
//       * rotate by 30 degrees,
//       * scale X to 2.0,
//       * scale Y to 1.5,
//       * move to (100, 100).
//    The result will depend on the order of these transformations,
//    and the advantage of matrix is that the sequence of discret calls:
//    rotate(30), scaleX(2.0), scaleY(1.5), move(100,100)
//    will have exactly the same result as the following matrix transformations:
//
//    affine_matrix m;
//    m *= rotate_matrix(30);
//    m *= scaleX_matrix(2.0);
//    m *= scaleY_matrix(1.5);
//    m *= move_matrix(100,100);
//
//    m.transform_my_point_at_last(x, y);
//
// What is the good of it? In real life we will set-up the matrix only once
// and then transform many points, let alone the convenience to set any
// combination of transformations.
//
// So, how to use it? Very easy - literally as it's shown above. Not quite,
// let us write a correct example:
//
// agg::trans_affine m;
// m *= agg::trans_affine_rotation(30.0 * 3.1415926 / 180.0);
// m *= agg::trans_affine_scaling(2.0, 1.5);
// m *= agg::trans_affine_translation(100.0, 100.0);
// m.transform(&x, &y);
//
// The affine matrix is all you need to perform any linear transformation,
// but all transformations have origin point (0,0). It means that we need to
// use 2 translations if we want to rotate someting around (100,100):
//
// m *= agg::trans_affine_translation(-100.0, -100.0);         // move to (0,0)
// m *= agg::trans_affine_rotation(30.0 * 3.1415926 / 180.0);  // rotate
// m *= agg::trans_affine_translation(100.0, 100.0);           // move back to (100,100)
//----------------------------------------------------------------------
class trans_affine
{
 public:
  //------------------------------------------ Construction
  // Construct an identity matrix - it does not transform anything
  trans_affine() :
      m0(1.0), m1(0.0), m2(0.0), m3(1.0), m4(0.0), m5(0.0)
  {}

  // Construct a custom matrix. Usually used in derived classes
  trans_affine(agg_real v0, agg_real v1, agg_real v2, agg_real v3, agg_real v4, agg_real v5) :
      m0(v0), m1(v1), m2(v2), m3(v3), m4(v4), m5(v5)
  {}

  // Construct a matrix to transform a parallelogram to another one.
  trans_affine(const agg_real* rect, const agg_real* parl)
  {
    parl_to_parl(rect, parl);
  }

  // Construct a matrix to transform a rectangle to a parallelogram.
  trans_affine(agg_real x1, agg_real y1, agg_real x2, agg_real y2,
               const agg_real* parl)
  {
    rect_to_parl(x1, y1, x2, y2, parl);
  }

  // Construct a matrix to transform a parallelogram to a rectangle.
  trans_affine(const agg_real* parl,
               agg_real x1, agg_real y1, agg_real x2, agg_real y2)
  {
    parl_to_rect(parl, x1, y1, x2, y2);
  }


  //---------------------------------- Parellelogram transformations
  // Calculate a matrix to transform a parallelogram to another one.
  // src and dst are pointers to arrays of three points
  // (agg_real[6], x,y,...) that identify three corners of the
  // parallelograms assuming implicit fourth points.
  // There are also transformations rectangtle to parallelogram and
  // parellelogram to rectangle
  const trans_affine& parl_to_parl(const agg_real* src,
                                   const agg_real* dst);

  const trans_affine& rect_to_parl(agg_real x1, agg_real y1,
                                   agg_real x2, agg_real y2,
                                   const agg_real* parl);

  const trans_affine& parl_to_rect(const agg_real* parl,
                                   agg_real x1, agg_real y1,
                                   agg_real x2, agg_real y2);


  //------------------------------------------ Operations
  // Reset - actually load an identity matrix
  const trans_affine& reset();

  // Multiply matrix to another one
  const trans_affine& multiply(const trans_affine& m);

  // Multiply "m" to "this" and assign the result to "this"
  const trans_affine& premultiply(const trans_affine& m);

  // Multiply matrix to inverse of another one
  const trans_affine& multiply_inv(const trans_affine& m);

  // Multiply inverse of "m" to "this" and assign the result to "this"
  const trans_affine& premultiply_inv(const trans_affine& m);

  // Invert matrix. Do not try to invert degenerate matrices,
  // there's no check for validity. If you set scale to 0 and
  // then try to invert matrix, expect unpredictable result.
  const trans_affine& invert();

  // Mirroring around X
  const trans_affine& flip_x();

  // Mirroring around Y
  const trans_affine& flip_y();

  //------------------------------------------- Load/Store
  // Store matrix to an array [6] of agg_real
  void store_to(agg_real* m) const
  {
    *m++ = m0; *m++ = m1; *m++ = m2; *m++ = m3; *m++ = m4; *m++ = m5;
  }

  // Load matrix from an array [6] of agg_real
  const trans_affine& load_from(const agg_real* m)
  {
    m0 = *m++; m1 = *m++; m2 = *m++; m3 = *m++; m4 = *m++;  m5 = *m++;
    return *this;
  }

  //------------------------------------------- Operators

  // Multiply current matrix to another one
  const trans_affine& operator *= (const trans_affine& m)
  {
    return multiply(m);
  }

  // Multiply current matrix to inverse of another one
  const trans_affine& operator /= (const trans_affine& m)
  {
    return multiply_inv(m);
  }

  // Multiply current matrix to another one and return
  // the result in a separete matrix.
  trans_affine operator * (const trans_affine& m)
  {
    return trans_affine(*this).multiply(m);
  }

  // Multiply current matrix to inverse of another one
  // and return the result in a separete matrix.
  trans_affine operator / (const trans_affine& m)
  {
    return trans_affine(*this).multiply_inv(m);
  }

  // Calculate and return the inverse matrix
  trans_affine operator ~ () const
  {
    trans_affine ret = *this;
    return ret.invert();
  }

  // Equal operator with default epsilon
  bool operator == (const trans_affine& m) const
  {
    return is_equal(m, affine_epsilon);
  }

  // Not Equal operator with default epsilon
  bool operator != (const trans_affine& m) const
  {
    return !is_equal(m, affine_epsilon);
  }

  //-------------------------------------------- Transformations
  // Direct transformation x and y
  void transform(agg_real* x, agg_real* y) const;

  // Direct transformation x and y, 2x2 matrix only, no translation
  void transform_2x2(agg_real* x, agg_real* y) const;

  // Inverse transformation x and y. It works slower than the
  // direct transformation, so if the performance is critical
  // it's better to invert() the matrix and then use transform()
  void inverse_transform(agg_real* x, agg_real* y) const;

  //-------------------------------------------- Auxiliary
  // Calculate the determinant of matrix
  agg_real determinant() const
  {
    return 1.0 / (m0 * m3 - m1 * m2);
  }

  // Get the average scale (by X and Y).
  // Basically used to calculate the approximation_scale when
  // decomposinting curves into line segments.
  agg_real scale() const;

  // Check to see if it's an identity matrix
  bool is_identity(agg_real epsilon = affine_epsilon) const;

  // Check to see if two matrices are equal
  bool is_equal(const trans_affine& m, agg_real epsilon = affine_epsilon) const;

  // Determine the major parameters. Use carefully considering degenerate matrices
  agg_real rotation() const;
  void   translation(agg_real* dx, agg_real* dy) const;
  void   scaling(agg_real* sx, agg_real* sy) const;
  void   scaling_abs(agg_real* sx, agg_real* sy) const
  {
    *sx = sqrt(m0*m0 + m2*m2);
    *sy = sqrt(m1*m1 + m3*m3);
  }

  agg_real& value(unsigned idx)       { return ((agg_real*)(&m0))[idx]; }
  const agg_real& value(unsigned idx) const { return ((agg_real*)(&m0))[idx]; }


 private:
  agg_real m0;
  agg_real m1;
  agg_real m2;
  agg_real m3;
  agg_real m4;
  agg_real m5;
};

//------------------------------------------------------------------------
inline void trans_affine::transform(agg_real* x, agg_real* y) const
{
  agg_real tx = *x;
  *x = tx * m0 + *y * m2 + m4;
  *y = tx * m1 + *y * m3 + m5;
}

//------------------------------------------------------------------------
inline void trans_affine::transform_2x2(agg_real* x, agg_real* y) const
{
  agg_real tx = *x;
  *x = tx * m0 + *y * m2;
  *y = tx * m1 + *y * m3;
}

//------------------------------------------------------------------------
inline void trans_affine::inverse_transform(agg_real* x, agg_real* y) const
{
  agg_real d = determinant();
  agg_real a = (*x - m4) * d;
  agg_real b = (*y - m5) * d;
  *x = a * m3 - b * m2;
  *y = b * m0 - a * m1;
}

//------------------------------------------------------------------------
inline agg_real trans_affine::scale() const
{
  agg_real x = 0.707106781 * m0 + 0.707106781 * m2;
  agg_real y = 0.707106781 * m1 + 0.707106781 * m3;
  return sqrt(x*x + y*y);
}

//------------------------------------------------------------------------
inline const trans_affine& trans_affine::premultiply(const trans_affine& m)
{
  trans_affine t = m;
  return *this = t.multiply(*this);
}

//------------------------------------------------------------------------
inline const trans_affine& trans_affine::multiply_inv(const trans_affine& m)
{
  trans_affine t = m;
  t.invert();
  multiply(t);
  return *this;
}

//------------------------------------------------------------------------
inline const trans_affine& trans_affine::premultiply_inv(const trans_affine& m)
{
  trans_affine t = m;
  t.invert();
  return *this = t.multiply(*this);
}

//====================================================trans_affine_rotation
// Rotation matrix. sin() and cos() are calculated twice for the same angle.
// There's no harm because the performance of sin()/cos() is very good on all
// modern processors. Besides, this operation is not going to be invoked too
// often.
class trans_affine_rotation : public trans_affine
{
 public:
  trans_affine_rotation(agg_real a) :
      trans_affine(cos(a), sin(a), -sin(a), cos(a), 0.0, 0.0)
  {}
};

//====================================================trans_affine_scaling
// Scaling matrix. sx, sy - scale coefficients by X and Y respectively
class trans_affine_scaling : public trans_affine
{
 public:
  trans_affine_scaling(agg_real sx, agg_real sy) :
      trans_affine(sx, 0.0, 0.0, sy, 0.0, 0.0)
  {}

  trans_affine_scaling(agg_real s) :
      trans_affine(s, 0.0, 0.0, s, 0.0, 0.0)
  {}
};

//================================================trans_affine_translation
// Translation matrix
class trans_affine_translation : public trans_affine
{
 public:
  trans_affine_translation(agg_real tx, agg_real ty) :
      trans_affine(1.0, 0.0, 0.0, 1.0, tx, ty)
  {}
};

//====================================================trans_affine_skewing
// Sckewing (shear) matrix
class trans_affine_skewing : public trans_affine
{
 public:
  trans_affine_skewing(agg_real sx, agg_real sy) :
      trans_affine(1.0, tan(sy), tan(sx), 1.0, 0.0, 0.0)
  {}
};


//===============================================trans_affine_line_segment
// Rotate, Scale and Translate, associating 0...dist with line segment
// x1,y1,x2,y2
class trans_affine_line_segment : public trans_affine
{
 public:
  trans_affine_line_segment(agg_real x1, agg_real y1, agg_real x2, agg_real y2,
                            agg_real dist)
  {
    agg_real dx = x2 - x1;
    agg_real dy = y2 - y1;
    if (dist > 0.0) {
      multiply(trans_affine_scaling(sqrt(dx * dx + dy * dy) / dist));
    }
    multiply(trans_affine_rotation(atan2(dy, dx)));
    multiply(trans_affine_translation(x1, y1));
  }
};


}


#endif
