#ifndef __VGPOLYGONTESSELATOR_69007832_H__
#define __VGPOLYGONTESSELATOR_69007832_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#if niMinFeatures(20)

#include "AGG.h"
#include "API/niUI/IVGPolygonTesselator.h"
#include "libtess2/tesselator.h"

// Outputs less triangles, probably slower, but since we didnt really profile
// and also we could aggressively cache its left on by default for now.
#define USE_LIBTESS_OPTIMIZE_OUTPUT_TRIANGLES

// Left here for reference only. We can't use it in prod since it requires a
// commercial license that we dont have, and more importantly it generates
// holes at overlapping line joins. Libtess generates less triangles and so
// far perfect shapes.
// #define USE_GPC

class cVGPolygonTesselator : public cIUnknownImpl<iVGPolygonTesselator>
{
 public:
  //! ctor
  cVGPolygonTesselator();
  //! dtor
  ~cVGPolygonTesselator();

  tBool __stdcall AddVertexF32(tF32 x, tF32 y);
  tBool __stdcall AddVertexF64(tF64 x, tF64 y);
  tBool __stdcall AddVertex(const sVec2f& vertex);

  tSize __stdcall GetNumVertices() const;
  tBool __stdcall SubmitContour();
  tBool __stdcall BeginPolygon(tBool abEvenOdd);
  tBool __stdcall EndPolygon();
  const tVec2fCVec* __stdcall GetTesselatedVertices() const;

 public:
  tU32 mnNumContour;
  tBool mbBegan;
  tBool mbEvenOdd;
  //! list of polygon vertices; only point 0 of triangle is valid
  astl::vector<sVec2f> mvPolyVerts;
  //! list with new triangles
  Nonnull<tVec2fCVec> mvFinalVerts;

 private:
#ifdef USE_GPC
  gpc_polygon mGPCPoly;
#else
  TESStesselator* mTess;
#endif
};

struct sTesselatePath
{
  enum status
  {
    status_initial,
    status_line_to,
    status_closed
  };

  sTesselatePath(cVGPolygonTesselator* apTesselator) :
      m_tesselator(apTesselator) {
  }

  unsigned  m_status;
  agg_real    m_prev_x, m_prev_y;
  agg_real    m_start_x, m_start_y;
  Ptr<cVGPolygonTesselator> m_tesselator;

  void begin(tBool abEvenOdd) {
    m_tesselator->BeginPolygon(abEvenOdd);
    m_status = status_closed;
    move_to(100,100);
  }
  void end() {
    close_polygon();
    m_tesselator->EndPolygon();
  }
  void submit_poly() {
    m_tesselator->SubmitContour();
  }

  void move_to(agg_real x, agg_real y)
  {
    if (m_status == status_line_to) {
      close_polygon();
    }
    m_prev_x = m_start_x = x;
    m_prev_y = m_start_y = y;
    m_status = status_initial;
  }

  void line_to(agg_real x, agg_real y)
  {
    if (m_status == status_initial) {
      m_tesselator->mvPolyVerts.push_back(Vec2f((tF32)m_prev_x,(tF32)m_prev_y));
    }
    m_tesselator->mvPolyVerts.push_back(Vec2f((tF32)x,(tF32)y));
    m_prev_x = x;
    m_prev_y = y;
    m_status = status_line_to;
  }

  void close_polygon()
  {
    if (m_status == status_line_to)
    {
      if (m_prev_x != m_start_x || m_prev_y != m_start_y) {
        m_tesselator->mvPolyVerts.push_back(Vec2f((tF32)m_start_x,(tF32)m_start_y));
      }
      submit_poly();
      m_status = status_closed;
    }
  }

  void add_vertex(agg_real x, agg_real y, unsigned cmd)
  {
    if(agg::is_close(cmd)) {
      close_polygon();
    }
    else if(agg::is_move_to(cmd)) {
      move_to(x, y);
    }
    else if(agg::is_vertex(cmd)) {
      line_to(x, y);
    }
  }

  template<class VertexSource>
  void tesselate_path(VertexSource& vs, tBool abEvenOdd, tU32 aPathID = 0)
  {
    agg_real x, y;
    unsigned int cmd;
    vs.rewind(aPathID);
    begin(abEvenOdd);
    while (1)
    {
      cmd = vs.vertex(&x, &y);
      if (agg::is_stop(cmd))
        break;
      add_vertex(x, y, cmd);
    }
    end();
  }
};

#endif // niMinFeatures
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __VGPOLYGONTESSELATOR_69007832_H__
