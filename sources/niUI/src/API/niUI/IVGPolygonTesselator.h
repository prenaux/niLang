#ifndef __IVGPOLYGONTESSELATOR_11036409_H__
#define __IVGPOLYGONTESSELATOR_11036409_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {
/** \addtogroup niUI
 * @{
 */

#if niMinFeatures(20)

//! Polygon tessellator interface.
// Usage Flow:
//
//   BeginPolygon(EvenOdd(true)/NonZero(false))
//    for each contour
//      for each contour vertex : AddVertex(vertex.x,vertex.y)
//      SubmitCountour
//   EndPolygon()
//   GetTesselatedVertices()
//
struct iVGPolygonTesselator : public iUnknown
{
  niDeclareInterfaceUUID(iVGPolygonTesselator,0xbb3a2ec0,0x8f76,0x4888,0x8f,0x1e,0x4a,0x4e,0xd0,0xe8,0x00,0xee)

  //! Add a F32 vertex to the current polygon contour.
  virtual tBool __stdcall AddVertexF32(tF32 x, tF32 y) = 0;
  //! Add a F64 vertex to the current polygon contour.
  virtual tBool __stdcall AddVertexF64(tF64 x, tF64 y) = 0;
  //! Add a Vec2f vertex to the current polygon contour.
  virtual tBool __stdcall AddVertex(const sVec2f& vertex) = 0;
  //! Get the number of vertices in the current contour.
  virtual tSize __stdcall GetNumVertices() const = 0;
  //! Submit the current contour.
  virtual tBool __stdcall SubmitContour() = 0;
  //! Begin tesselating a polygon.
  virtual tBool __stdcall BeginPolygon(tBool abEvenOdd) = 0;
  //! Finish tesselating a polygon.
  virtual tBool __stdcall EndPolygon() = 0;
  //! Get the tesselated polygon's triangles vertices.
  virtual const tVec2fCVec* __stdcall GetTesselatedVertices() const = 0;
};

#endif // niMinFeatures

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __IVGPOLYGONTESSELATOR_11036409_H__
