#ifndef __IVGPATH_35982248_H__
#define __IVGPATH_35982248_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

#if niMinFeatures(20)
struct iFont;
struct iVGStyle;
struct iVGTransform;
struct iVGPolygonTesselator;
#endif // niMinFeatures

/** \addtogroup niUI
 * @{
 */

#if niMinFeatures(20)

//! VGPath commands.
enum eVGPathCommand
{
  //! Stop command.
  eVGPathCommand_Stop   = 0,
  //! MoveTo command.
  eVGPathCommand_MoveTo = 1,
  //! LineTo command.
  eVGPathCommand_LineTo = 2,
  //! Quadric curve command.
  eVGPathCommand_Curve3 = 3,
  //! Cubic curve command.
  eVGPathCommand_Curve4 = 4,
  //! N curve command.
  eVGPathCommand_CurveN = 5,
  //! Catrom command.
  eVGPathCommand_Catrom = 6,
  //! UBSpline command.
  eVGPathCommand_UBSpline = 7,
  //! End poly command.
  eVGPathCommand_EndPoly  = 0x0F,
  //! Command mask.
  eVGPathCommand_Mask   = 0x0F,
  //! \internal
  eVGPathCommand_ForceDWORD = 0xFFFFFFFF
};

//! VGPathTesselatedRenderer interface.
//! {DispatchWrapper}
struct iVGPathTesselatedRenderer : public iUnknown
{
  niDeclareInterfaceUUID(iVGPathTesselatedRenderer,0x4629b016,0x9a7a,0x44bb,0x8f,0x6c,0x24,0xf5,0x88,0x6d,0xe4,0x62)

  //! Called when begining to render a path.
  virtual void __stdcall BeginAddPath(const iVGStyle* apStyle, tBool abStroke) = 0;
  //! Called when end adding a path.
  virtual void __stdcall EndAddPath(const iVGStyle* apStyle, tBool abStroke) = 0;
  //! Called to request the approximation scale.
  virtual tF32 __stdcall GetPathApproximationScale(const iVGStyle* apStyle) const = 0;
  //! Called to add the path's polygons.
  virtual void __stdcall AddPathPolygons(iVGPolygonTesselator* apTess, const iVGStyle* apStyle, tBool abStroke) = 0;
};

//! VGPath interface.
struct iVGPath : public iUnknown
{
  niDeclareInterfaceUUID(iVGPath,0x01302541,0x1712,0x47b1,0x80,0xa9,0xe2,0xb1,0x61,0x53,0x9f,0x96)

  //! Copy the specified path.
  virtual tBool __stdcall Copy(const iVGPath* apPath) = 0;
  //! Clone this path.
  virtual iVGPath* __stdcall Clone() const = 0;
  //! Clear the path.
  virtual void __stdcall Clear() = 0;
  //! Add a vertex to the path.
  virtual tBool __stdcall AddVertex(const sVec2f& avVertex, eVGPathCommand aCommand) = 0;
  //! Add another path to the end of this path.
  virtual tBool __stdcall AddPath(const iVGPath* apPath) = 0;
  //! Get the number of vertices in the path.
  //! {Property}
  virtual tSize __stdcall GetNumVertices() const = 0;
  //! Set the vertex at the specified index.
  //! {Property}
  virtual tBool __stdcall SetVertex(tU32 anIndex, const sVec2f& avVertex) = 0;
  //! Get the vertex at the specified index.
  //! {Property}
  virtual sVec2f __stdcall GetVertex(tU32 anIndex) const = 0;
  //! Set the command at the specified index.
  //! {Property}
  virtual tBool __stdcall SetCommand(tU32 anIndex, eVGPathCommand aCmd) = 0;
  //! Get the command at the specified index.
  //! {Property}
  virtual eVGPathCommand __stdcall GetCommand(tU32 anIndex) const = 0;

  //! Reset the iterator.
  virtual void __stdcall ResetIterator() const = 0;
  //! Get the next vertex of the iterator.
  //! {Property}
  virtual eVGPathCommand __stdcall GetNextIterator() const = 0;
  //! Get the current vertex of the iterator.
  //! {Property}
  virtual sVec2f __stdcall GetVertexIterator() const = 0;

  //! Flip the path along the X-axis.
  virtual void __stdcall FlipX(tF32 x1, tF32 x2) = 0;
  //! Flip the path along the Y-axis.
  virtual void __stdcall FlipY(tF32 y1, tF32 y2) = 0;

  //! Move to the specified position.
  //! \remark Maps to the 'M' path command.
  virtual void __stdcall MoveTo(tF32 x, tF32 y) = 0;
  //! Move to the specified position, relativly to the previous vertex.
  //! \remark Maps to the 'm' path command.
  virtual void __stdcall MoveToRel(tF32 x, tF32 y) = 0;
  //! Add a line to the specified position.
  //! \remark Maps to the 'L' path command.
  virtual void __stdcall LineTo(tF32 x, tF32 y) = 0;
  //! \remark Maps to the 'l' path command, relativly to the previous vertex.
  virtual void __stdcall LineToRel(tF32 x, tF32 y) = 0;
  //! Add an horizontal line to the specified column.
  //! \remark Maps to the 'H' path command.
  virtual void __stdcall HLineTo(tF32 x) = 0;
  //! \remark Maps to the 'h' path command, relativly to the previous vertex.
  virtual void __stdcall HLineToRel(tF32 x) = 0;
  //! Add a vertical line to the specified line.
  //! \remark Maps to the 'V' path command.
  virtual void __stdcall VLineTo(tF32 y) = 0;
  //! Add a vertical line to the specified line, relativly to the previous vertex.
  //! \remark Maps to the 'v' path command.
  virtual void __stdcall VLineToRel(tF32 y) = 0;
  //! Quadratic Bezier curveto.
  //! \remark Maps to the 'Q' path command.
  virtual void __stdcall Curve3(tF32 x1, tF32 y1, tF32 x, tF32 y) = 0;
  //! Quadratic Bezier curveto.
  //! \remark Maps to the 'q' path command, relativly to the previous vertex.
  virtual void __stdcall Curve3Rel(tF32 x1, tF32 y1, tF32 x, tF32 y) = 0;
  //! Shorthand/smooth quadratic Bezier curveto.
  //! \remark Maps to the 'T' path command.
  virtual void __stdcall Curve3Prev(tF32 x, tF32 y) = 0;
  //! Shorthand/smooth quadratic Bezier curveto, relativly to the previous vertex.
  //! \remark Maps to the 't' path command.
  virtual void __stdcall Curve3PrevRel(tF32 x, tF32 y) = 0;
  //! Cubic Bezier curveto.
  //! \remark Maps to the 'C' path command.
  virtual void __stdcall Curve4(tF32 x1, tF32 y1, tF32 x2, tF32 y2, tF32 x,  tF32 y) = 0;
  //! Cubic Bezier curveto, relativly to the previous vertex.
  //! \remark Maps to the 'c' path command.
  virtual void __stdcall Curve4Rel(tF32 x1, tF32 y1, tF32 x2, tF32 y2, tF32 x,  tF32 y) = 0;
  //! Shorthand/smooth cubic Bezier curveto.
  //! \remark Maps to the 'S' path command.
  virtual void __stdcall Curve4Prev(tF32 x2, tF32 y2, tF32 x,  tF32 y) = 0;
  //! Shorthand/smooth cubic Bezier curveto, relativly to the previous vertex.
  //! \remark Maps to the 's' path command.
  virtual void __stdcall Curve4PrevRel(tF32 x2, tF32 y2, tF32 x,  tF32 y) = 0;
  //! Cubic curve with auto control point computation, relativly to the previous vertex.
  virtual void __stdcall Curve4K(tF32 x2, tF32 y2, tF32 k) = 0;
  //! Cubic curve with auto control point computation.
  virtual void __stdcall Curve4KRel(tF32 x2, tF32 y2, tF32 k) = 0;
  // Hermite curve.
  virtual void __stdcall Hermite(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4) = 0;
  // Hermite curve, relativly to the previous vertex.
  virtual void __stdcall HermiteRel(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4) = 0;
  // UBSpline.
  virtual void __stdcall UBSpline(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4) = 0;
  // UBSpline, relativly to the previous vertex.
  virtual void __stdcall UBSplineRel(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4) = 0;
  // Catrom.
  virtual void __stdcall Catrom(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4) = 0;
  // Catrom, relativly to the previous vertex.
  virtual void __stdcall CatromRel(tF32 x2, tF32 y2, tF32 x3, tF32 y3, tF32 x4, tF32 y4) = 0;
  //! Elliptical arc.
  //! \remark Maps to the SVG 'A' path command.
  virtual void __stdcall ArcTo(tF32 rx, tF32 ry, tF32 angle, tBool large_arc_flag, tBool sweep_flag, tF32 x, tF32 y) = 0;
  //! Elliptical arc, relativly to the previous vertex.
  //! \remark Maps to the SVG 'a' path command.
  virtual void __stdcall ArcToRel(tF32 rx, tF32 ry, tF32 angle, tBool large_arc_flag, tBool sweep_flag, tF32 x, tF32 y) = 0;
  //! Close the current path's polygon.
  //! \remark Maps to the SVG 'Z' and 'z' path commands.
  virtual void __stdcall ClosePolygon() = 0;

  //! Add a rectangle.
  virtual void __stdcall Rect(tF32 x, tF32 y, tF32 width, tF32 height) = 0;
  //! Add a rounded rectangle.
  virtual void __stdcall RoundedRect(tF32 x, tF32 y, tF32 width, tF32 height, tF32 rx, tF32 ry) = 0;
  //! Add a centered rectangle.
  virtual void __stdcall RectCentered(tF32 cx, tF32 cy, tF32 width, tF32 height) = 0;
  //! Add a rounded centered rectangle.
  virtual void __stdcall RoundedRectCentered(tF32 cx, tF32 cy, tF32 width, tF32 height, tF32 rx, tF32 ry) = 0;
  //! Add a speech bubble rectangle.
  virtual void __stdcall SpeechBubbleRect(
    const tF32 x, const tF32 y, const tF32 w, const tF32 h,
    const tF32 rx, const tF32 ry,
    const tF32 tx, const tF32 ty,
    const tF32 arrowGap, const tF32 arrowLen) = 0;

  //! Add a circle.
  virtual void __stdcall Circle(tF32 cx, tF32 cy, tF32 radius) = 0;
  //! Add an ellipse.
  virtual void __stdcall Ellipse(tF32 cx, tF32 cy, tF32 rx, tF32 ry) = 0;

  //! Add a line.
  virtual tBool __stdcall Line(tF32 x1, tF32 y1, tF32 x2, tF32 y2) = 0;
  //! Add a polyline.
  virtual tBool __stdcall Polyline(const tVec2fCVec* apVerts) = 0;
  //! Add a polygon.
  virtual tBool __stdcall Polygon(const tVec2fCVec* apVerts) = 0;

  //! Add a star shape.
  // xc,yc : start center
  // r1 : even points radius
  // r2 : odd points radius
  // n : number of sides
  // start_angle : initial rotation angle
  virtual void __stdcall Star(tF32 xc, tF32 yc, tF32 r1, tF32 r2, tU32 n, tF32 start_angle = 0.0) = 0;

  //! Add a text.
  virtual tBool __stdcall Text(iFont* apFont, const sVec2f& avPos, const achar* aaszText) = 0;
  //! Add a text along a path.
  virtual tBool __stdcall TextAlongOnePath(iFont* apFont, const achar* aaszText, const iVGPath* apPath, tBool abPreserveXScale, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale) = 0;
  //! Add a text between two paths.
  virtual tBool __stdcall TextAlongTwoPaths(iFont* apFont, const achar* aaszText, const iVGPath* apPath1, const iVGPath* apPath2, tBool abPreserveXScale, tF32 afBaseHeight, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale) = 0;

  //! Transform the path to a bspline
  virtual void __stdcall ToBSpline(tF32 afInterpolationStep) = 0;
  //! Translate the path.
  virtual tBool __stdcall Translate(const sVec2f& avTranslate) = 0;
  //! Rotate the path.
  virtual tBool __stdcall Rotate(tF32 afRadians) = 0;
  //! Scale the path.
  virtual tBool __stdcall Scale(const sVec2f& avScale) = 0;
  //! Transform the path with the specified transform.
  virtual tBool __stdcall Transform(const iVGTransform* apTransform) = 0;
  //! Transform the path along another path
  virtual tBool __stdcall TransformAlongOnePath(const iVGPath* apPath, tBool abPreserveXScale, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale) = 0;
  //! Transform the path against two other paths
  virtual tBool __stdcall TransformAlongTwoPaths(const iVGPath* apPath1, const iVGPath* apPath2, tBool abPreserveXScale, tF32 afBaseHeight, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale) = 0;
  //! Make all polygons follow the same orientation.
  virtual void __stdcall ArrangeOrientations(tBool abCW) = 0;
  //! Get the path's bounding box.
  virtual sRectf __stdcall GetBoundingRect(const iVGTransform* apTransform) const = 0;

  //! Render the tesselated path.
  virtual tBool __stdcall RenderTesselated(iVGPathTesselatedRenderer* apPath, const iVGTransform* apTransform, const iVGStyle* apStyle) const = 0;

  //! Add a SVG path.
  //! \remark The syntax of the path is the same as SVG paths.
  virtual tBool __stdcall AddSVGPath(const achar* aaszPath) = 0;
  //! Add a polyline path.
  //! \remark The syntax of the path is the same as SVG polyline element.
  virtual tBool __stdcall AddPolylinePath(const achar* aaszPath) = 0;
  //! Add a polygon path.
  //! \remark The syntax of the path is the same as SVG polygon element.
  virtual tBool __stdcall AddPolygonPath(const achar* aaszPath) = 0;
  //! Get the path in a SVG string form.
  //! {Property}
  virtual cString __stdcall GetSVGPath() const = 0;
};

#endif // niMinFeatures

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IVGPATH_35982248_H__
