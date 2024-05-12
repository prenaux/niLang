#ifndef __VGPATH_91BF7F82_B0CE_4FA7_AA0E_36465B00C12C_H__
#define __VGPATH_91BF7F82_B0CE_4FA7_AA0E_36465B00C12C_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Graphics.h"

#if niMinFeatures(20)
#include "API/niUI/IVGPath.h"

#include "AGG.h"
#include "VG.h"
#include "agg/agg_rasterizer_compound_aa.h"
#include "SVGPathTokenizer.h"

// Path container and renderer.
class cVGPath : public ni::ImplRC<iVGPath>
{
 public:
  template<class VertexSource> class conv_count
  {
   public:
    conv_count(VertexSource& vs) : m_source(&vs), m_count(0) {}

    void count(unsigned n) { m_count = n; }
    unsigned count() const { return m_count; }

    void rewind(unsigned path_id) { m_source->rewind(path_id); }
    unsigned vertex(agg_real* x, agg_real* y)
    {
      ++m_count;
      return m_source->vertex(x, y);
    }

   private:
    VertexSource* m_source;
    unsigned m_count;
  };

  typedef agg::conv_curve<agg::path_storage>      curved;
  // curved
  typedef      conv_count<curved>           curved_count;
  // curved
  typedef agg::conv_smooth_poly1_curve<curved_count>  curved_smooth;

  typedef agg::conv_stroke<curved_count>          curved_stroked;
  typedef agg::conv_transform<curved_stroked>       curved_stroked_trans;

  typedef agg::conv_stroke<curved_smooth>         curved_smooth_stroked;
  typedef agg::conv_transform<curved_smooth_stroked>    curved_smooth_stroked_trans;

  typedef agg::conv_dash<curved_count>          curved_dash;
  typedef agg::conv_stroke<curved_dash>         curved_dash_stroked;
  typedef agg::conv_transform<curved_dash_stroked>    curved_dash_stroked_trans;
  typedef agg::conv_dash<curved_smooth>         curved_smooth_dash;
  typedef agg::conv_stroke<curved_smooth_dash>      curved_smooth_dash_stroked;
  typedef agg::conv_transform<curved_smooth_dash_stroked> curved_smooth_dash_stroked_trans;

  typedef agg::conv_transform<curved_count>     curved_trans;
  typedef agg::conv_contour<curved_trans>       curved_trans_contour;
  typedef agg::conv_transform<curved_smooth>      curved_smooth_trans;
  typedef agg::conv_contour<curved_smooth_trans>    curved_smooth_trans_contour;

  typedef agg::conv_stroke<curved_trans>        curved_trans_stroked;
  typedef agg::conv_stroke<curved_smooth_trans>   curved_trans_smooth_stroked;

  typedef agg::conv_dash<curved_trans>        curved_trans_dash;
  typedef agg::conv_stroke<curved_trans_dash>     curved_trans_dash_stroked;

  typedef agg::conv_dash<curved_smooth_trans>     curved_trans_smooth_dash;
  typedef agg::conv_stroke<curved_trans_smooth_dash>  curved_trans_smooth_dash_stroked;

  cVGPath();
  tBool __stdcall Copy(const iVGPath* apPath);
  iVGPath* __stdcall Clone() const;
  void __stdcall Clear();

  //! Add a vertex to the path.
  virtual tBool __stdcall AddVertex(const sVec2f& avVertex, eVGPathCommand aCommand) {
    m_storage.add_vertex(avVertex.x,avVertex.y,(agg::path_commands_e)aCommand);
    return eTrue;
  }

  tBool __stdcall AddPath(const iVGPath* apPath) {
    if (!niIsOK(apPath))
      return eFalse;

    apPath->ResetIterator();
    while (1) {
      eVGPathCommand cmd = apPath->GetNextIterator();
      if (cmd == eVGPathCommand_Stop)
        break;
      AddVertex(apPath->GetVertexIterator(),cmd);
    }

    return eTrue;
  }

  tSize __stdcall GetNumVertices() const {
    return m_storage.total_vertices();
  }

  tBool __stdcall SetVertex(tU32 anIndex, const sVec2f& avVertex) {
    if (anIndex >= m_storage.total_vertices()) return eFalse;
    m_storage.set_vertex(anIndex,avVertex.x,avVertex.y);
    return eTrue;
  }

  sVec2f __stdcall GetVertex(tU32 anIndex) const {
    agg_real x, y;
    m_storage.vertex(anIndex,&x,&y);
    return Vec2f((tF32)x,(tF32)y);
  }

  virtual tBool __stdcall SetCommand(tU32 anIndex, eVGPathCommand aCmd) {
    if (anIndex >= m_storage.total_vertices())
      return eFalse;
    m_storage.set_command(anIndex,(unsigned char)aCmd);
    return eTrue;
  }

  virtual eVGPathCommand __stdcall GetCommand(tU32 anIndex) const {
    if (anIndex >= m_storage.total_vertices())
      return eVGPathCommand_Stop;
    return (eVGPathCommand)m_storage.command(anIndex);
  }

  //! Reset the iterator.
  void __stdcall ResetIterator() const {
    niConstCast(cVGPath*,this)->m_storage.rewind(0);
  }

  //! Get the next vertex of the iterator.
  eVGPathCommand __stdcall GetNextIterator() const  {
    agg_real x = 0, y = 0;
    unsigned cmd = niConstCast(cVGPath*,this)->m_storage.vertex(&x,&y);
    niConstCast(cVGPath*,this)->mvItVertex.x = (tF32)x;
    niConstCast(cVGPath*,this)->mvItVertex.y = (tF32)y;
    return (eVGPathCommand)cmd;
  }

  //! Get the current vertex of the iterator.
  sVec2f __stdcall GetVertexIterator() const {
    return mvItVertex;
  }

  void __stdcall FlipX(tF32 x1, tF32 x2) {
    m_storage.flip_x(x1,x2);
  }

  void __stdcall FlipY(tF32 y1, tF32 y2) {
    m_storage.flip_y(y1,y2);
  }

  // The following functions are essentially a "reflection" of
  // the respective SVG path commands.
  // M
  void __stdcall MoveTo(tF32 x, tF32 y) {
    m_storage.move_to(x, y);
  }
  // m
  void __stdcall MoveToRel(tF32 x, tF32 y) {
    m_storage.move_rel(x, y);
  }
  // L
  void __stdcall LineTo(tF32 x, tF32 y) {
    m_storage.line_to(x,y);
  }
  // l
  void __stdcall LineToRel(tF32 x, tF32 y) {
    m_storage.line_rel(x,y);
  }
  // H
  void __stdcall HLineTo(tF32 x) {
    agg_real x2 = 0.0;
    agg_real y2 = 0.0;
    if (m_storage.total_vertices()) {
      m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
      m_storage.line_to(x,y2);
    }
  }
  // h
  void __stdcall HLineToRel(tF32 x) {
    agg_real x2 = 0.0;
    agg_real y2 = 0.0;
    if (m_storage.total_vertices()) {
      m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
      /*if (rel)*/ x += (tF32)x2;
      m_storage.line_to(x,y2);
    }
  }
  // V
  void __stdcall VLineTo(tF32 y) {
    agg_real x2 = 0.0;
    agg_real y2 = 0.0;
    if(m_storage.total_vertices())
    {
      m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
      m_storage.line_to(x2, y);
    }
  }
  // v
  void __stdcall VLineToRel(tF32 y) {
    agg_real x2 = 0.0;
    agg_real y2 = 0.0;
    if(m_storage.total_vertices())
    {
      m_storage.vertex(m_storage.total_vertices() - 1, &x2, &y2);
      /*if (rel)*/ y += (tF32)y2;
      m_storage.line_to(x2, y);
    }
  }
  // Q
  void __stdcall Curve3(tF32 x1, tF32 y1, tF32 x, tF32 y)
  {
    m_storage.curve3(x1, y1, x, y);
  }
  // q
  void __stdcall Curve3Rel(tF32 x1, tF32 y1, tF32 x, tF32 y)
  {
    m_storage.curve3_rel(x1, y1, x, y);
  }
  // T
  void __stdcall Curve3Prev(tF32 x, tF32 y)
  {
    m_storage.curve3(x, y);
  }
  // t
  void __stdcall Curve3PrevRel(tF32 x, tF32 y)
  {
    m_storage.curve3_rel(x, y);
  }
  // C
  void __stdcall Curve4(tF32 x1, tF32 y1, tF32 x2, tF32 y2, tF32 x,  tF32 y)
  {
    m_storage.curve4(x1, y1, x2, y2, x, y);
  }
  // c
  void __stdcall Curve4Rel(tF32 x1, tF32 y1, tF32 x2, tF32 y2, tF32 x,  tF32 y)
  {
    m_storage.curve4_rel(x1, y1, x2, y2, x, y);
  }
  // S
  void __stdcall Curve4Prev(tF32 x2, tF32 y2, tF32 x,  tF32 y)
  {
    m_storage.curve4(x2, y2, x, y);
  }
  // s
  void __stdcall Curve4PrevRel(tF32 x2, tF32 y2, tF32 x,  tF32 y)
  {
    m_storage.curve4_rel(x2, y2, x, y);
  }
  // Curve with auto control point computation.
  void __stdcall Curve4K(tF32 x2, tF32 y2, tF32 k)
  {
    sVec2f prev = GetVertex(GetNumVertices()-1);
    tF32 x1 = prev.x;
    tF32 y1 = prev.y;
    Curve4(x1 - (y2 - y1) * k,
           y1 + (x2 - x1) * k,
           x2 + (y2 - y1) * k,
           y2 - (x2 - x1) * k,
           x2, y2);
  }
  void __stdcall Curve4KRel(tF32 x2, tF32 y2, tF32 k)
  {
    sVec2f prev = GetVertex(GetNumVertices()-1);
    tF32 x1 = prev.x;
    tF32 y1 = prev.y;
    Curve4Rel(x1 - (y2 - y1) * k,
              y1 + (x2 - x1) * k,
              x2 + (y2 - y1) * k,
              y2 - (x2 - x1) * k,
              x2, y2);
  }
  // Hermite curve
  void __stdcall Hermite(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4) {
    // Trans. matrix Hermite to Bezier
    //
    //  1       0       0       0
    //  1       0       1/3     0
    //  0       1       0       -1/3
    //  0       1       0       0
    //
    sVec2f prev = GetVertex(GetNumVertices()-1);
    tF32 x1 = prev.x;
    tF32 y1 = prev.y;
    return Curve4(
      (3*x1 + x3) / 3,
      (3*y1 + y3) / 3,
      (3*x2 - x4) / 3,
      (3*y2 - y4) / 3,
      x2,
      y2);
  }
  void __stdcall HermiteRel(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4) {
    // Trans. matrix Hermite to Bezier
    //
    //  1       0       0       0
    //  1       0       1/3     0
    //  0       1       0       -1/3
    //  0       1       0       0
    //
    sVec2f prev = GetVertex(GetNumVertices()-1);
    tF32 x1 = prev.x;
    tF32 y1 = prev.y;
    return Curve4Rel(
      (3*x1 + x3) / 3,
      (3*y1 + y3) / 3,
      (3*x2 - x4) / 3,
      (3*y2 - y4) / 3,
      x2,
      y2);
  }
  // UBSpline
  void __stdcall UBSpline(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4)
  {
    // Trans. matrix Uniform BSpline to Bezier
    //
    //  1/6     4/6     1/6     0
    //  0       4/6     2/6     0
    //  0       2/6     4/6     0
    //  0       1/6     4/6     1/6
    //
    sVec2f prev = GetVertex(GetNumVertices()-1);
    tF32 x1 = prev.x;
    tF32 y1 = prev.y;
    MoveTo((x1 + 4*x2 + x3) / 6,
           (y1 + 4*y2 + y3) / 6);
    return Curve4((4*x2 + 2*x3) / 6,
                  (4*y2 + 2*y3) / 6,
                  (2*x2 + 4*x3) / 6,
                  (2*y2 + 4*y3) / 6,
                  (x2 + 4*x3 + x4) / 6,
                  (y2 + 4*y3 + y4) / 6);
  }
  void __stdcall UBSplineRel(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4)
  {
    // Trans. matrix Uniform BSpline to Bezier
    //
    //  1/6     4/6     1/6     0
    //  0       4/6     2/6     0
    //  0       2/6     4/6     0
    //  0       1/6     4/6     1/6
    //
    sVec2f prev = GetVertex(GetNumVertices()-1);
    tF32 x1 = prev.x;
    tF32 y1 = prev.y;
    MoveToRel((x1 + 4*x2 + x3) / 6,
              (y1 + 4*y2 + y3) / 6);
    return Curve4Rel((4*x2 + 2*x3) / 6,
                     (4*y2 + 2*y3) / 6,
                     (2*x2 + 4*x3) / 6,
                     (2*y2 + 4*y3) / 6,
                     (x2 + 4*x3 + x4) / 6,
                     (y2 + 4*y3 + y4) / 6);
  }
  // Catrom
  void __stdcall Catrom(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4)
  {
    // Trans. matrix Catmull-Rom to Bezier
    //
    //  0       1       0       0
    //  -1/6    1       1/6     0
    //  0       1/6     1       -1/6
    //  0       0       1       0
    //
    sVec2f prev = GetVertex(GetNumVertices()-1);
    tF32 x1 = prev.x;
    tF32 y1 = prev.y;
    MoveTo(x2,y2);
    return Curve4(
      (-x1 + 6*x2 + x3) / 6,
      (-y1 + 6*y2 + y3) / 6,
      ( x2 + 6*x3 - x4) / 6,
      ( y2 + 6*y3 - y4) / 6,
      x3,
      y3);
  }
  void __stdcall CatromRel(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4)
  {
    // Trans. matrix Catmull-Rom to Bezier
    //
    //  0       1       0       0
    //  -1/6    1       1/6     0
    //  0       1/6     1       -1/6
    //  0       0       1       0
    //
    sVec2f prev = GetVertex(GetNumVertices()-1);
    tF32 x1 = prev.x;
    tF32 y1 = prev.y;
    MoveToRel(x2,y2);
    return Curve4Rel(
      (-x1 + 6*x2 + x3) / 6,
      (-y1 + 6*y2 + y3) / 6,
      ( x2 + 6*x3 - x4) / 6,
      ( y2 + 6*y3 - y4) / 6,
      x3,
      y3);
  }
  // A
  void __stdcall ArcTo(tF32 rx, tF32 ry, tF32 angle, tBool large_arc_flag, tBool sweep_flag, tF32 x, tF32 y)
  {
    m_storage.arc_to(rx,ry,angle,!!large_arc_flag,!!sweep_flag,x,y);
  }
  // a
  void __stdcall ArcToRel(tF32 rx, tF32 ry, tF32 angle, tBool large_arc_flag, tBool sweep_flag, tF32 x, tF32 y)
  {
    m_storage.arc_rel(rx,ry,angle,!!large_arc_flag,!!sweep_flag,x,y);
  }

  // Z, z
  void __stdcall ClosePolygon()
  {
    m_storage.end_poly(agg::path_flags_close);
    m_storage.set_current_to_initial();
  }

  void __stdcall Rect(tF32 x, tF32 y, tF32 width, tF32 height) {
    MoveTo(x,y);
    LineTo(x+width,y);
    LineTo(x+width,y+height);
    LineTo(x,y+height);
    LineTo(x,y);
    ClosePolygon();
  }

  void __stdcall RoundedRect(tF32 x, tF32 y, tF32 width, tF32 height, tF32 rx, tF32 ry) {
    if (rx*2 > width)
      rx = width/2;
    if (ry*2 > height)
      ry = height/2;
    MoveTo(x+rx,y);
    LineTo(x+width-rx,y);
    if (rx > 0 || ry > 0) {
      ArcTo(rx,ry,0,false,true,x+width,y+ry);
    }
    LineTo(x+width,y+height-ry);
    if (rx > 0 || ry > 0) {
      ArcTo(rx,ry,0,false,true,x+width-rx,y+height);
    }
    LineTo(x+rx,y+height);
    if (rx > 0 || ry > 0) {
      ArcTo(rx,ry,0,false,true,x,y+height-ry);
    }
    LineTo(x,y+ry);
    if (rx > 0 || ry > 0) {
      ArcTo(rx,ry,0,false,true,x+rx,y);
    }
    ClosePolygon();
    /*agg::rounded_rect rect;
      rect.rect(x,y,x+width,y+height);
      rect.radius(rx,ry);
      m_storage.add_path(rect);*/
  }

  void __stdcall RectCentered(tF32 cx, tF32 cy, tF32 width, tF32 height) {
    Rect(cx-(width/2),cy-(height/2),width,height);
  }

  void __stdcall RoundedRectCentered(tF32 cx, tF32 cy, tF32 width, tF32 height, tF32 rx, tF32 ry) {
    RoundedRect(cx-(width/2),cy-(height/2),width,height,rx,ry);
  }

  void __stdcall Circle(tF32 cx, tF32 cy, tF32 radius) {
    MoveTo(cx+radius,cy);
    ArcTo(radius,radius,0,true,true,cx-radius,cy);
    ArcTo(radius,radius,0,true,true,cx+radius,cy);
  }

  void __stdcall Ellipse(tF32 cx, tF32 cy, tF32 rx, tF32 ry) {
    MoveTo(cx+rx,cy);
    ArcTo(rx,ry,0,true,true,cx-rx,cy);
    ArcTo(rx,ry,0,true,true,cx+rx,cy);
  }

  //! Add a line.
  virtual tBool __stdcall Line(tF32 x1, tF32 y1, tF32 x2, tF32 y2) {
    MoveTo(x1,y1);
    LineTo(x2,y2);
    return eTrue;
  }

  //! Add a polyline.
  virtual tBool __stdcall Polyline(const tVec2fCVec* apVerts) {
    niCheckSilent(niIsOK(apVerts),eFalse);
    niCheckSilent(apVerts->GetSize() >= 2,eFalse);
    tVec2fCVec::const_iterator it = apVerts->begin();
    const sVec2f& vFirst = *it++;
    m_storage.move_to(vFirst.x,vFirst.y);
    for (; it != apVerts->end(); ++it) {
      m_storage.line_to(it->x,it->y);
    }
    return eTrue;
  }

  //! Add a polygon.
  virtual tBool __stdcall Polygon(const tVec2fCVec* apVerts) {
    niCheckSilent(niIsOK(apVerts),eFalse);
    niCheckSilent(apVerts->GetSize() >= 3,eFalse);
    tVec2fCVec::const_iterator it = apVerts->begin();
    const sVec2f& vFirst = *it++;
    m_storage.move_to(vFirst.x,vFirst.y);
    for (; it != apVerts->end(); ++it) {
      m_storage.line_to(it->x,it->y);
    }
    ClosePolygon();
    return eTrue;
  }

  // xc,yc : start center
  // r1 : even points radius
  // r2 : odd points radius
  // n : number of sides
  // start_angle : initial rotation angle
  void __stdcall Star(tF32 xc, tF32 yc, tF32 r1, tF32 r2, tU32 n, tF32 start_angle = 0.0)
  {
    tU32 i;
    for(i = 0; i < n; i++)
    {
      tF32 a = ni::kPi * 2.0f * i / n - ni::kPi / 2.0f;
      tF32 dx = ni::Cos(a + start_angle);
      tF32 dy = -ni::Sin(a + start_angle);

      if (i & 1) {
        LineTo(xc + dx * r1, yc + dy * r1);
      }
      else {
        if(i) LineTo(xc + dx * r2, yc + dy * r2);
        else  MoveTo(xc + dx * r2, yc + dy * r2);
      }
    }
    ClosePolygon();
  }

  // Transform the path to a bspline
  void __stdcall ToBSpline(tF32 afInterpolationStep) {
    agg::path_storage path = m_storage;
    typedef agg::conv_bspline<agg::path_storage> conv_bspline_type;
    conv_bspline_type bspline(path);
    bspline.interpolation_step(afInterpolationStep);
    m_storage.remove_all();
    m_storage.add_path(bspline);
  }

  // Translate the path.
  tBool __stdcall Translate(const sVec2f& avTranslate);
  // Rotate the path.
  tBool __stdcall Rotate(tF32 afRadians);
  // Scale the path.
  tBool __stdcall Scale(const sVec2f& avScale);
  // Transform the path with the specified transform.
  tBool __stdcall Transform(const iVGTransform* apTransform);
  // Transform the path along another path
  tBool __stdcall TransformAlongOnePath(const iVGPath* apPath, tBool abPreserveXScale, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale);
  // Transform the path against two other paths
  tBool __stdcall TransformAlongTwoPaths(const iVGPath* apPath1, const iVGPath* apPath2, tBool abPreserveXScale, tF32 afBaseHeight, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale);

  // Make all polygons CCW-oriented
  void __stdcall ArrangeOrientations(tBool abCW);
  sRectf __stdcall GetBoundingRect(const iVGTransform* apTransform) const;

  virtual tBool __stdcall RenderTesselated(iVGPathTesselatedRenderer* apPath, const iVGTransform* apTransform, const iVGStyle* apStyle) const;

  virtual tBool __stdcall AddSVGPath(const achar* aaszPath);
  virtual tBool __stdcall AddPolylinePath(const achar* aaszPath);
  virtual tBool __stdcall AddPolygonPath(const achar* aaszPath);
  cString __stdcall GetSVGPath() const;

  template <class AGGRas>
  void RenderPath(AGGRas& aRas, const iVGTransform* apPathTransform, const iVGStyle* apStyle, const sVec2f& avOffset)
  {
    m_curved_count.count(0);

    agg::trans_affine trans = agg::trans_affine_translation(avOffset.x,avOffset.y);
    if (apPathTransform) trans = AGGGetTransform(apPathTransform);
    agg_real scl = trans.scale()*aRas.Ras_GetApproximationScale(apStyle);
    //m_curved.approximation_method(curve_inc);
    m_curved.approximation_scale(scl);
    m_curved.angle_tolerance(0.0);

    curved_smooth smooth(m_curved_count);
    smooth.smooth_value(apStyle->GetSmooth());

    if (apStyle->GetFill())
    {
      aRas.Ras_BeginAddPath(apStyle,eFalse);

      if (apStyle->GetSmooth() > 0.0f) {
        curved_smooth_trans     smooth_trans(smooth, trans);
        curved_smooth_trans_contour smooth_trans_contour(smooth_trans);
        smooth_trans_contour.auto_detect_orientation(false);
        smooth_trans_contour.width(apStyle->GetFillExpand());

        if (smooth_trans_contour.width() < 0.001f) {
          aRas.Ras_AddPath(smooth_trans,apStyle,eFalse);
        }
        else {
          smooth_trans_contour.miter_limit(apStyle->GetMiterLimit());
          aRas.Ras_AddPath(smooth_trans_contour,apStyle,eFalse);
        }
      }
      else {
        curved_trans      trans_curve(m_curved_count, trans);
        curved_trans_contour  trans_contour(trans_curve);
        trans_contour.auto_detect_orientation(false);
        trans_contour.width(apStyle->GetFillExpand());

        if (trans_contour.width() < 0.001f) {
          aRas.Ras_AddPath(trans_curve,apStyle,eFalse);
        }
        else {
          trans_contour.miter_limit(apStyle->GetMiterLimit());
          aRas.Ras_AddPath(trans_contour,apStyle,eFalse);
        }
      }

      aRas.Ras_EndAddPath(apStyle,eFalse);
    }

    if (apStyle->GetStroke())
    {
      aRas.Ras_BeginAddPath(apStyle,eTrue);

      if (apStyle->GetNumDashes()) {

        if (apStyle->GetSmooth() > 0.0f) {
          if (apStyle->GetStrokeTransformed()) {
            curved_smooth_trans         trans_smooth(smooth,trans);
            curved_trans_smooth_dash      trans_smooth_dash(trans_smooth);
            curved_trans_smooth_dash_stroked  trans_smooth_dash_stroked(trans_smooth_dash);

            for (tU32 i = 0; i < apStyle->GetNumDashes(); ++i) {
              sVec2f d = apStyle->GetDash(i);
              trans_smooth_dash.add_dash(d.x,d.y);
            }
            trans_smooth_dash.dash_start(apStyle->GetDashStart());

            trans_smooth_dash_stroked.width(apStyle->GetStrokeWidth());
            trans_smooth_dash_stroked.line_join((agg::line_join_e)apStyle->GetLineJoin());
            trans_smooth_dash_stroked.line_cap((agg::line_cap_e)apStyle->GetLineCap());
            trans_smooth_dash_stroked.miter_limit(apStyle->GetMiterLimit());
            trans_smooth_dash_stroked.inner_join((agg::inner_join_e)apStyle->GetInnerJoin());
            trans_smooth_dash_stroked.approximation_scale(scl);

            // If the *visual* line width is considerable we
            // turn on processing of curve cusps.
            //---------------------
            if (apStyle->GetStrokeWidth() /** scl*/ > 1.0) {
              m_curved.angle_tolerance(agg_real(0.2));
            }
            aRas.Ras_AddPath(trans_smooth_dash_stroked,apStyle,eTrue);
          }
          else {
            curved_smooth_dash          smooth_dash(smooth);
            curved_smooth_dash_stroked      smooth_dash_stroked(smooth_dash);
            curved_smooth_dash_stroked_trans  smooth_dash_stroked_trans(smooth_dash_stroked,trans);

            for (tU32 i = 0; i < apStyle->GetNumDashes(); ++i) {
              sVec2f d = apStyle->GetDash(i);
              smooth_dash.add_dash(d.x,d.y);
            }
            smooth_dash.dash_start(apStyle->GetDashStart());

            smooth_dash_stroked.width(apStyle->GetStrokeWidth());
            smooth_dash_stroked.line_join((agg::line_join_e)apStyle->GetLineJoin());
            smooth_dash_stroked.line_cap((agg::line_cap_e)apStyle->GetLineCap());
            smooth_dash_stroked.miter_limit(apStyle->GetMiterLimit());
            smooth_dash_stroked.inner_join((agg::inner_join_e)apStyle->GetInnerJoin());
            smooth_dash_stroked.approximation_scale(scl);

            // If the *visual* line width is considerable we
            // turn on processing of curve cusps.
            //---------------------
            if (apStyle->GetStrokeWidth() * scl > 1.0) {
              m_curved.angle_tolerance(agg_real(0.2));
            }
            aRas.Ras_AddPath(smooth_dash_stroked_trans,apStyle,eTrue);
          }
        }
        else {
          if (apStyle->GetStrokeTransformed()) {
            curved_trans        trans_curve(m_curved_count,trans);
            curved_trans_dash     trans_dash(trans_curve);
            curved_trans_dash_stroked trans_dash_stroked(trans_dash);

            for (tU32 i = 0; i < apStyle->GetNumDashes(); ++i) {
              sVec2f d = apStyle->GetDash(i);
              trans_dash.add_dash(d.x,d.y);
            }
            trans_dash.dash_start(apStyle->GetDashStart());

            trans_dash_stroked.width(apStyle->GetStrokeWidth());
            trans_dash_stroked.line_join((agg::line_join_e)apStyle->GetLineJoin());
            trans_dash_stroked.line_cap((agg::line_cap_e)apStyle->GetLineCap());
            trans_dash_stroked.miter_limit(apStyle->GetMiterLimit());
            trans_dash_stroked.inner_join((agg::inner_join_e)apStyle->GetInnerJoin());
            trans_dash_stroked.approximation_scale(scl);

            // If the *visual* line width is considerable we
            // turn on processing of curve cusps.
            //---------------------
            if (apStyle->GetStrokeWidth() * scl > 1.0) {
              m_curved.angle_tolerance(agg_real(0.2));
            }
            aRas.Ras_AddPath(trans_dash_stroked,apStyle,eTrue);
          }
          else {
            curved_dash         dash(m_curved_count);
            curved_dash_stroked     dash_stroked(dash);
            curved_dash_stroked_trans dash_stroked_trans(dash_stroked,trans);

            for (tU32 i = 0; i < apStyle->GetNumDashes(); ++i) {
              sVec2f d = apStyle->GetDash(i);
              dash.add_dash(d.x,d.y);
            }
            dash.dash_start(apStyle->GetDashStart());

            dash_stroked.width(apStyle->GetStrokeWidth());
            dash_stroked.line_join((agg::line_join_e)apStyle->GetLineJoin());
            dash_stroked.line_cap((agg::line_cap_e)apStyle->GetLineCap());
            dash_stroked.miter_limit(apStyle->GetMiterLimit());
            dash_stroked.inner_join((agg::inner_join_e)apStyle->GetInnerJoin());
            dash_stroked.approximation_scale(scl);

            // If the *visual* line width is considerable we
            // turn on processing of curve cusps.
            //---------------------
            if (apStyle->GetStrokeWidth() * scl > 1.0) {
              m_curved.angle_tolerance(agg_real(0.2));
            }
            aRas.Ras_AddPath(dash_stroked_trans,apStyle,eTrue);
          }
        }
      }
      else {
        if (apStyle->GetSmooth() > 0.0f) {
          if (apStyle->GetStrokeTransformed()) {
            curved_smooth_trans       smooth_trans(smooth,trans);
            curved_trans_smooth_stroked   trans_smooth_stroked(smooth_trans);

            trans_smooth_stroked.width(apStyle->GetStrokeWidth());
            trans_smooth_stroked.line_join((agg::line_join_e)apStyle->GetLineJoin());
            trans_smooth_stroked.line_cap((agg::line_cap_e)apStyle->GetLineCap());
            trans_smooth_stroked.miter_limit(apStyle->GetMiterLimit());
            trans_smooth_stroked.inner_join((agg::inner_join_e)apStyle->GetInnerJoin());
            trans_smooth_stroked.approximation_scale(scl);
            aRas.Ras_AddPath(trans_smooth_stroked,apStyle,eTrue);
          }
          else {
            curved_smooth_stroked     smooth_stroked(smooth);
            curved_smooth_stroked_trans smooth_stroked_trans(smooth_stroked,trans);
            smooth_stroked.width(apStyle->GetStrokeWidth());
            smooth_stroked.line_join((agg::line_join_e)apStyle->GetLineJoin());
            smooth_stroked.line_cap((agg::line_cap_e)apStyle->GetLineCap());
            smooth_stroked.miter_limit(apStyle->GetMiterLimit());
            smooth_stroked.inner_join((agg::inner_join_e)apStyle->GetInnerJoin());
            smooth_stroked.approximation_scale(scl);
            aRas.Ras_AddPath(smooth_stroked_trans,apStyle,eTrue);
          }
        }
        else {
          if (apStyle->GetStrokeTransformed()) {
            curved_trans      trans_curve(m_curved_count,trans);
            curved_trans_stroked  trans_stroked(trans_curve);
            trans_stroked.width(apStyle->GetStrokeWidth());
            trans_stroked.line_join((agg::line_join_e)apStyle->GetLineJoin());
            trans_stroked.line_cap((agg::line_cap_e)apStyle->GetLineCap());
            trans_stroked.miter_limit(apStyle->GetMiterLimit());
            trans_stroked.inner_join((agg::inner_join_e)apStyle->GetInnerJoin());
            trans_stroked.approximation_scale(scl);
            aRas.Ras_AddPath(trans_stroked,apStyle,eTrue);
          }
          else {
            curved_stroked      stroked(m_curved_count);
            curved_stroked_trans  stroked_trans(stroked,trans);
            stroked.width(apStyle->GetStrokeWidth());
            stroked.line_join((agg::line_join_e)apStyle->GetLineJoin());
            stroked.line_cap((agg::line_cap_e)apStyle->GetLineCap());
            stroked.miter_limit(apStyle->GetMiterLimit());
            stroked.inner_join((agg::inner_join_e)apStyle->GetInnerJoin());
            stroked.approximation_scale(scl);
            aRas.Ras_AddPath(stroked_trans,apStyle,eTrue);
          }
        }
      }

      aRas.Ras_EndAddPath(apStyle,eTrue);
    }
  }

  tBool __stdcall Text(iFont* apFont, const sVec2f& avPos, const achar* aaszText) {
    if (!niIsOK(apFont))
      return eFalse;

    sVec2f pos = avPos;
    StrCharIt itText(aaszText);
    while (!itText.is_end()) {
      const tU32 c = itText.next();
      const tU32 glyphIndex = apFont->GetGlyphIndexFromCodepoint(c);
      if (glyphIndex) {
        const sVec2f sz = apFont->GetGlyphPath(this,glyphIndex,pos,1.0f);
        pos.x += sz.x;
     }
    }

    return eTrue;
  }

  tBool __stdcall TextAlongOnePath(iFont* apFont, const achar* aaszText, const iVGPath* apPath, tBool abPreserveXScale, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale) {
    if (!niIsOK(apFont))
      return eFalse;

    sVec2f pos = {0,0};

    Ptr<iVGPath> ptrGlyphPath = niNew cVGPath();
    StrCharIt itText(aaszText);
    while (!itText.is_end()) {
      ptrGlyphPath->Clear();
      const tU32 glyphIndex = apFont->GetGlyphIndexFromCodepoint(itText.next());
      pos += apFont->GetGlyphPath(this,glyphIndex,pos,1.0f);
      ptrGlyphPath->TransformAlongOnePath(
        apPath,abPreserveXScale,afBaseLength,afSegmentApproxScale,afCurvesApproxScale);
      this->AddPath(ptrGlyphPath);
    }

    return eTrue;
  }

  tBool __stdcall TextAlongTwoPaths(iFont* apFont, const achar* aaszText, const iVGPath* apPath1, const iVGPath* apPath2, tBool abPreserveXScale, tF32 afBaseHeight, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale) {
    if (!niIsOK(apFont))
      return eFalse;

    sVec2f pos = {0,0};

    Ptr<iVGPath> ptrGlyphPath = niNew cVGPath();
    StrCharIt itText(aaszText);
    while (!itText.is_end()) {
      ptrGlyphPath->Clear();
      const tU32 glyphIndex = apFont->GetGlyphIndexFromCodepoint(itText.next());
      pos += apFont->GetGlyphPath(this,glyphIndex,pos,1.0f);
      ptrGlyphPath->TransformAlongTwoPaths(
        apPath1,apPath2,eTrue,afBaseHeight,afBaseLength,afSegmentApproxScale,afCurvesApproxScale);
      this->AddPath(ptrGlyphPath);
    }

    return eTrue;
  }

  void __stdcall SpeechBubbleRect(
    const tF32 x, const tF32 y, const tF32 w, const tF32 h,
    const tF32 rx, const tF32 ry,
    const tF32 tx, const tF32 ty,
    const tF32 arrowGap, const tF32 arrowLen) niImpl
  {
    const tF32 ag2 = arrowGap/2.0f;
    const tF32 x2 = x+w;
    const tF32 y2 = y+h;
    const tF32 cx = x+(w/2.0f);
    const tF32 cy = y+(w/2.0f);
    MoveTo(x+rx,y);

    tF32 testArrowLen = arrowLen / 2.0f;
    int edge = -1;
    // bottom edge?
    if (
      (tx >= x && tx <= x2 && ty >= y2) ||
      (ty >= (y2+testArrowLen))
    ) {
      edge = 2;
    }
    // top edge?
    else if (
      (tx >= x && tx <= x2 && ty <= y) ||
      (ty <= (y-testArrowLen))
    ) {
      edge = 0;
    }
    // left edge?
    else if (
      (ty >= y && ty <= y2 && tx <= x) ||
      (tx <= (x-testArrowLen))
    ) {
      edge = 3;
    }
    // right edge?
    else if (
      (ty >= y && ty <= y2 && tx >= x2) ||
      (tx >= (x2+testArrowLen))
    ) {
      edge = 1;
    }

    // top edge
    if (edge == 0) {
      auto bx = ni::Clamp(tx,x+rx+ag2,x+w-rx-ag2);
      LineTo(bx-ag2,y);
      LineTo(ni::Clamp(tx,x-arrowGap,x+w+arrowGap),y-arrowLen);
      LineTo(bx+ag2,y);
    }
    LineTo(x2-rx,y);

    // top-right corner
    ArcTo(rx,ry,0,false,true,x2,y+ry);

    // right edge
    if (edge == 1) {
      auto by = ni::Clamp(ty,y+ry+ag2,y+h-ry-ag2);
      LineTo(x2,by-ag2);
      LineTo(x2+arrowLen,ni::Clamp(ty,y-arrowGap,y+h+arrowGap));
      LineTo(x2,by+ag2);
    }
    LineTo(x2,y2-ry);

    // bottom-right corner
    ArcTo(rx,ry,0,false,true,x2-rx,y2);

    // bottom edge
    if (edge == 2) {
      auto bx = ni::Clamp(tx,x+rx+ag2,x+w-rx-ag2);
      LineTo(bx+ag2,y2);
      LineTo(ni::Clamp(tx,x-arrowGap,x+w+arrowGap),y2+arrowLen);
      LineTo(bx-ag2,y2);
    }
    LineTo(x+rx,y2);

    // bottom-left corner
    ArcTo(rx,ry,0,false,true,x,y2-ry);

    // left edge
    if (edge == 3) {
      auto by = ni::Clamp(ty,y+ry+ag2,y+h-ry-ag2);
      LineTo(x,by+ag2);
      LineTo(x-arrowLen,ni::Clamp(ty,y-arrowGap,y+h+arrowGap));
      LineTo(x,by-ag2);
    }
    LineTo(x,y+ry);

    // top-left corner
    ArcTo(rx,ry,0,false,true,x+rx,y);

    ClosePolygon();
  }

 private:
  agg::path_storage m_storage;
  curved            m_curved;
  curved_count      m_curved_count;
  sVec2f         mvItVertex;
};
#endif                                  // niMinFeatures

#endif // __VGPATH_91BF7F82_B0CE_4FA7_AA0E_36465B00C12C_H__
