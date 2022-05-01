// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(20)
#include "Graphics.h"
#include "AGG.h"
#include "VG.h"
#include "VGPolygonTesselator.h"
#include "agg/agg_rasterizer_compound_aa.h"
#include "SVGPathTokenizer.h"
#include "VGPath.h"

cVGPath::cVGPath()
  : m_curved(m_storage), m_curved_count(m_curved)
{
  m_storage.start_new_path();
}


tBool __stdcall cVGPath::Copy(const iVGPath* apPath) {
  if (!niIsOK(apPath)) return eFalse;
  Clear();
  AddPath(apPath);
  return eTrue;
}

iVGPath* __stdcall cVGPath::Clone() const {
  cVGPath* pNew = niNew cVGPath();
  pNew->Copy(this);
  return pNew;
}

void __stdcall cVGPath::Clear() {
  m_storage.remove_all();
}


// Translate the path.
tBool __stdcall cVGPath::Translate(const sVec2f& avTranslate) {
  agg::path_storage path = m_storage;
  agg::trans_affine trans = agg::trans_affine_translation(avTranslate.x,avTranslate.y);
  agg::conv_transform<agg::path_storage> transformed(path,trans);
  m_storage.remove_all();
  m_storage.add_path(transformed);
  return eTrue;
}

// Rotate the path.
tBool __stdcall cVGPath::Rotate(tF32 afRadians) {
  agg::path_storage path = m_storage;
  agg::trans_affine trans = agg::trans_affine_rotation(afRadians);
  agg::conv_transform<agg::path_storage> transformed(path,trans);
  m_storage.remove_all();
  m_storage.add_path(transformed);
  return eTrue;
}

// Scale the path.
tBool __stdcall cVGPath::Scale(const sVec2f& avScale) {
  agg::path_storage path = m_storage;
  agg::trans_affine trans = agg::trans_affine_scaling(avScale.x,avScale.y);
  agg::conv_transform<agg::path_storage> transformed(path,trans);
  m_storage.remove_all();
  m_storage.add_path(transformed);
  return eTrue;
}

// Transform the path with the specified transform.
tBool __stdcall cVGPath::Transform(const iVGTransform* apTransform) {
  if (!niIsOK(apTransform)) return eFalse;
  agg::path_storage path = m_storage;
  agg::conv_transform<agg::path_storage> transformed(path,AGGGetTransform(apTransform));
  m_storage.remove_all();
  m_storage.add_path(transformed);
  return eTrue;
}

// Transform the path along another path
tBool __stdcall cVGPath::TransformAlongOnePath(const iVGPath* apPath, tBool abPreserveXScale, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale) {
  if (!niIsOK(apPath)) return eFalse;

  agg::path_storage path = m_storage;

  agg::trans_single_path tcurve;
  tcurve.add_path(niConstStaticCast(iVGPath*,cVGPath*,apPath)->m_storage);
  tcurve.preserve_x_scale(abPreserveXScale?true:false);
  if (afBaseLength > 0.0f)
    tcurve.base_length(afBaseLength);

  typedef agg::conv_curve<agg::path_storage>              conv_curve_type;
  typedef agg::conv_segmentator<conv_curve_type>                      conv_segm_type;
  typedef agg::conv_transform<conv_segm_type, agg::trans_single_path> conv_trans_type;

  conv_curve_type fcurves(path);
  conv_segm_type  fsegm(fcurves);
  conv_trans_type ftrans(fsegm, tcurve);
  fsegm.approximation_scale(afSegmentApproxScale);
  fcurves.approximation_scale(afCurvesApproxScale);

  m_storage.remove_all();
  m_storage.add_path(ftrans);
  return eTrue;
}

// Transform the path against two other paths
tBool __stdcall cVGPath::TransformAlongTwoPaths(const iVGPath* apPath1, const iVGPath* apPath2, tBool abPreserveXScale, tF32 afBaseHeight, tF32 afBaseLength, tF32 afSegmentApproxScale, tF32 afCurvesApproxScale) {
  if (!niIsOK(apPath1)) return eFalse;
  if (!niIsOK(apPath2)) return eFalse;

  agg::path_storage path = m_storage;

  agg::trans_double_path tcurve;
  tcurve.preserve_x_scale(abPreserveXScale?true:false);
  if (afBaseLength > 0.0f)
    tcurve.base_length(afBaseLength);
  tcurve.base_height(afBaseHeight);
  tcurve.add_paths(niConstStaticCast(iVGPath*,cVGPath*,apPath1)->m_storage,
                   niConstStaticCast(iVGPath*,cVGPath*,apPath2)->m_storage);

  typedef agg::conv_curve<agg::path_storage>              conv_curve_type;
  typedef agg::conv_segmentator<conv_curve_type>                      conv_segm_type;
  typedef agg::conv_transform<conv_segm_type, agg::trans_double_path> conv_trans_type;

  conv_curve_type fcurves(path);
  conv_segm_type  fsegm(fcurves);
  conv_trans_type ftrans(fsegm, tcurve);
  fsegm.approximation_scale(afSegmentApproxScale);
  fcurves.approximation_scale(afCurvesApproxScale);

  m_storage.remove_all();
  m_storage.add_path(ftrans);
  return eTrue;
}

// Make all polygons CCW-oriented
void __stdcall cVGPath::ArrangeOrientations(tBool abCW)
{
  m_storage.arrange_orientations_all_paths(abCW?(agg::path_flags_cw):(agg::path_flags_ccw));
}

sRectf __stdcall cVGPath::GetBoundingRect(const iVGTransform* apTransform) const
{
  agg::trans_affine transform;
  if (apTransform) transform = AGGGetTransform(apTransform);
  agg::conv_transform<agg::path_storage> trans((agg::path_storage&)m_storage, transform);
  agg_real x1,y1,x2,y2;
  agg::bounding_rect_single(trans,0,&x1,&y1,&x2,&y2);
  return Rectf(Vec2f((tF32)x1,(tF32)y1),Vec2f((tF32)x2,(tF32)y2));
}

//! Render the tesselated path.
struct sRenderTesselated {
  iVGPathTesselatedRenderer*  pPathRenderer;
  sTesselatePath        tesselatePath;

  sRenderTesselated(iVGPathTesselatedRenderer* apPathRenderer)
      : pPathRenderer(apPathRenderer)
      , tesselatePath(niNew cVGPolygonTesselator())
  {
  }

  void Ras_BeginAddPath(const iVGStyle* apStyle, tBool abStroke) {
    pPathRenderer->BeginAddPath(apStyle,abStroke);
  }

  void Ras_EndAddPath(const iVGStyle* apStyle, tBool abStroke) {
    pPathRenderer->EndAddPath(apStyle,abStroke);
  }

  tF32 Ras_GetApproximationScale(const iVGStyle* apStyle) const {
    return pPathRenderer->GetPathApproximationScale(apStyle);
  }

  ///////////////////////////////////////////////
  template<class VertexSource>
  void Ras_AddPath(VertexSource& vs, const iVGStyle* apStyle, tBool abStroke)
  {
    tesselatePath.tesselate_path(vs,
                                 abStroke ?
                                 eTrue :
                                 (apStyle->GetFillEvenOdd() ? eTrue : eFalse),
                                 0);
    pPathRenderer->AddPathPolygons(tesselatePath.m_tesselator,apStyle,abStroke);
  }
};

tBool __stdcall cVGPath::RenderTesselated(iVGPathTesselatedRenderer* apPath, const iVGTransform* apTransform, const iVGStyle* apStyle) const {
  if (!niIsOK(apPath))
    return eFalse;
  if (!niIsOK(apStyle))
    return eFalse;
  sRenderTesselated render(apPath);
  niConstCast(cVGPath*,this)->RenderPath(render,apTransform,apStyle,sVec2f::Zero());
  return eTrue;
}

tBool __stdcall cVGPath::AddSVGPath(const achar* aaszPath)
{
  cString strPath(aaszPath);
  cSVGPathTokenizer tok;
  tok.SetPathString(strPath.Chars());
  while (tok.Next())
  {
    tF32 arg[10];
    char cmd = tok.LastCommand();
    unsigned i;
    switch(cmd)
    {
      case 'M': case 'm':
        arg[0] = tok.LastNumber();
        arg[1] = tok.Next(cmd);
        if (tok.HasError())
          break;
        if (cmd == 'm') {
          MoveToRel(arg[0], arg[1]);
        }
        else {
          MoveTo(arg[0], arg[1]);
        }
        break;

      case 'L': case 'l':
        arg[0] = tok.LastNumber();
        arg[1] = tok.Next(cmd);
        if (tok.HasError())
          break;
        if (cmd == 'l') {
          LineToRel(arg[0], arg[1]);
        }
        else {
          LineTo(arg[0], arg[1]);
        }
        break;

      case 'V': case 'v':
        if (cmd == 'v') {
          VLineToRel(tok.LastNumber());
        }
        else {
          VLineTo(tok.LastNumber());
        }
        break;

      case 'H': case 'h':
        if (cmd == 'h') {
          HLineToRel(tok.LastNumber());
        }
        else {
          HLineTo(tok.LastNumber());
        }
        break;

      case 'Q': case 'q':
        arg[0] = tok.LastNumber();
        for (i = 1; i < 4; i++) {
          arg[i] = tok.Next(cmd);
        }
        if (tok.HasError())
          break;
        if (cmd == 'q') {
          Curve3Rel(arg[0], arg[1], arg[2], arg[3]);
        }
        else {
          Curve3(arg[0], arg[1], arg[2], arg[3]);
        }
        break;

      case 'T': case 't':
        arg[0] = tok.LastNumber();
        arg[1] = tok.Next(cmd);
        if (tok.HasError())
          break;
        if (cmd == 't') {
          Curve3PrevRel(arg[0], arg[1]);
        }
        else {
          Curve3Prev(arg[0], arg[1]);
        }
        break;

      case 'C': case 'c':
        arg[0] = tok.LastNumber();
        for (i = 1; i < 6; i++) {
          arg[i] = tok.Next(cmd);
        }
        if (tok.HasError())
          break;
        if (cmd == 'c') {
          Curve4Rel(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
        }
        else {
          Curve4(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
        }
        break;

      case 'S': case 's':
        arg[0] = tok.LastNumber();
        for (i = 1; i < 4; i++) {
          arg[i] = tok.Next(cmd);
        }
        if (tok.HasError())
          break;
        if (cmd == 's') {
          Curve4PrevRel(arg[0], arg[1], arg[2], arg[3]);
        }
        else {
          Curve4Prev(arg[0], arg[1], arg[2], arg[3]);
        }
        break;

      case 'A': case 'a':
        arg[0] = tok.LastNumber();
        for (i = 1; i < 7; i++) {
          arg[i] = tok.Next(cmd);
        }
        if (tok.HasError())
          break;
        if (cmd == 'a') {
          ArcToRel(arg[0], arg[1], niRadf(arg[2]), arg[3]==0?false:true, arg[4]==0?false:true, arg[5], arg[6]);
        }
        else {
          ArcTo(arg[0], arg[1], niRadf(arg[2]), arg[3]==0?false:true, arg[4]==0?false:true, arg[5], arg[6]);
        }
        break;

      case 'Z': case 'z':
        ClosePolygon();
        break;

      default: {
        niError(niFmt(_A("Invalid Command %c !"),cmd));
        return eFalse;
      }
    }
  }
  if (tok.HasError()) {
    niError(_A("Parsing error !"));
    return eFalse;
  }

  return eTrue;
}

tBool __stdcall cVGPath::AddPolylinePath(const achar* aaszPath) {
  tBool bFirst = eTrue;
  tF32 x, y;
  cString strPath(aaszPath);
  cSVGPathTokenizer tok;
  tok.SetPathString(strPath.Chars());
  while (tok.Next()) {
    x = tok.LastNumber();
    y = tok.Next(0);
    if (bFirst) {
      m_storage.move_to(x,y);
      bFirst = eFalse;
    }
    else {
      m_storage.line_to(x,y);
    }
    if (tok.HasError()) {
      niError(_A("Parsing error !"));
      return eFalse;
    }
  }

  return eTrue;
}

tBool __stdcall cVGPath::AddPolygonPath(const achar* aaszPath) {
  tBool bFirst = eTrue;
  tF32 x, y;
  cString strPath(aaszPath);
  cSVGPathTokenizer tok;
  tok.SetPathString(strPath.Chars());
  while (tok.Next()) {
    x = tok.LastNumber();
    y = tok.Next(0);
    if (bFirst) {
      m_storage.move_to(x,y);
      bFirst = eFalse;
    }
    else {
      m_storage.line_to(x,y);
    }
    if (tok.HasError()) {
      niError(_A("Parsing error !"));
      return eFalse;
    }
  }

  ClosePolygon();
  return eTrue;
}

cString __stdcall cVGPath::GetSVGPath() const
{
  // TODO: Test properly, right now its been used mostly to debug glyph icons extraction
  cString str;
  eVGPathCommand prevCmd = eVGPathCommand_Stop;
  const tU32 numVerts = this->GetNumVertices();
  tU32 i = 0;
  while (i < numVerts) {
    if (i != 0) str.append(" ");
    eVGPathCommand cmd = this->GetCommand(i);
    switch (cmd) {
    case eVGPathCommand_Stop: {
      break;
    }
    case eVGPathCommand_EndPoly: {
      str.append("Z");
      break;
    }
    case eVGPathCommand_MoveTo: {
      if (prevCmd != cmd) str.append("M");
      sVec2f v = this->GetVertex(i);
      str.CatFormat("%g,%g", v.x, v.y);
      break;
    }
    case eVGPathCommand_LineTo: {
      if (prevCmd != cmd) str.append("L");
      sVec2f v = this->GetVertex(i);
      str.CatFormat("%g,%g", v.x, v.y);
      break;
    }
    case eVGPathCommand_Curve3: {
      if (prevCmd != cmd) str.append("Q");
      sVec2f v0 = this->GetVertex(i);
      sVec2f v1 = this->GetVertex(++i);
      str.CatFormat("%g,%g,%g,%g", v0.x, v0.y, v1.x, v1.y);
      break;
    }
    case eVGPathCommand_Curve4: {
      if (prevCmd != cmd) str.append("C");
      sVec2f v0 = this->GetVertex(i);
      sVec2f v1 = this->GetVertex(++i);
      sVec2f v2 = this->GetVertex(++i);
      str.CatFormat("%g,%g,%g,%g", v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);
      break;
    }
    default: {
      // No SVG path equivalent - and we're not going to do curve conversion...
      sVec2f v = this->GetVertex(i);
      if (prevCmd != cmd) str.CatFormat("[%d]",(tU32)cmd);
      str.CatFormat("%g,%g", (tU32)cmd, v.x, v.y);
      break;
    }
    }
    prevCmd = cmd;
    ++i;
  }
  return str;
}

Ptr<iVGPath> __stdcall cGraphics::CreateVGPath() {
  return niNew cVGPath();
}

///////////////////////////////////////////////
niExportFunc(iVGPath*) CreateVGPath() {
  return niNew cVGPath();
}

#endif
