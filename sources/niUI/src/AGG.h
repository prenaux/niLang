#ifndef __AGG_22860808_H__
#define __AGG_22860808_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#define AGG_PIXELFORMAT _A("R8G8B8A8")

#ifndef agg_real
#error agg_real not defined ! should be defined to float or double.
#endif

#include "agg/gpc.h"
#include "agg/agg_basics.h"
#include "agg/agg_rendering_buffer.h"
#include "agg/agg_rasterizer_scanline_aa.h"
#include "agg/agg_span_allocator.h"
#include "agg/agg_scanline_p.h"
#include "agg/agg_renderer_scanline.h"
#include "agg/agg_pixfmt_rgb.h"
#include "agg/agg_gamma_lut.h"
#include "agg/agg_ellipse.h"
#include "agg/agg_rounded_rect.h"
#include "agg/agg_conv_stroke.h"
#include "agg/agg_conv_transform.h"
#include "agg/agg_conv_close_polygon.h"
#include "agg/agg_conv_segmentator.h"
#include "agg/agg_conv_smooth_poly1.h"
#include "agg/agg_conv_contour.h"
#include "agg/agg_conv_curve.h"
#include "agg/agg_conv_dash.h"
#include "agg/agg_path_storage.h"
#include "agg/agg_color_rgba.h"
#include "agg/agg_array.h"
#include "agg/agg_bounding_rect.h"
#include "agg/agg_pixfmt_rgba.h"
#include "agg/agg_renderer_markers.h"
#include "agg/agg_span_image_filter_rgba.h"
#include "agg/agg_span_interpolator_linear.h"
#include "agg/agg_span_interpolator_persp.h"
#include "agg/agg_span_gradient.h"
#include "agg/agg_conv_bspline.h"
#include "agg/agg_conv_segmentator.h"
#include "agg/agg_trans_single_path.h"
#include "agg/agg_trans_double_path.h"
#include "agg/agg_image_accessors.h"
#include "agg/agg_scanline_storage_aa.h"
#include "agg/agg_scanline_storage_bin.h"
#include "agg/agg_scanline_u.h"
#include "agg/agg_scanline_bin.h"
#include "agg/agg_path_storage_integer.h"
#include "agg/agg_trans_affine.h"

namespace ni {

typedef agg::pixfmt_rgba32                  tAGGPixFmt;
typedef tAGGPixFmt::color_type                tAGGColorType;
typedef tAGGPixFmt::order_type                tAGGOrderType;
typedef agg::renderer_base<tAGGPixFmt>            tAGGRendererBase;
typedef agg::renderer_scanline_aa_solid<tAGGRendererBase> tAGGRendererAASolid;
typedef agg::renderer_scanline_bin_solid<tAGGRendererBase>  tAGGRendererBinSolid;
typedef agg::renderer_markers<tAGGRendererBase>       tAGGRendererPrimitives;

typedef agg::rasterizer_scanline_aa<> tAGGRasterizerScanline;
typedef agg::scanline_u8        tAGGScanline;

typedef agg::span_interpolator_linear<>                   tAGGLinearInterpolator;
typedef agg::span_interpolator_persp_exact<>                tAGGPerspInterpolator;

struct iAGGGradientPolymorphicWrapperBase : public iUnknown
{
  virtual int calculate(int x, int y, int) const = 0;
};

struct sAGGGradientTable
{
  sAGGGradientTable(tU32 anTableSize, const tAGGColorType* apTable)
      : mTable(apTable), mnTableSize(anTableSize)
  {
  }
  unsigned int size() const           { return mnTableSize; }
  tAGGColorType operator [] (unsigned v) const  { return mTable[v]; }

  const tU32        mnTableSize;
  const tAGGColorType*  mTable;
};

typedef agg::span_gradient<tAGGColorType,tAGGLinearInterpolator,iAGGGradientPolymorphicWrapperBase,sAGGGradientTable> tAGGGradientSpanGen;
typedef agg::span_allocator<tAGGGradientSpanGen::color_type> tAGGGradientSpanAlloc;
typedef agg::renderer_scanline_aa<tAGGRendererBase, tAGGGradientSpanAlloc, tAGGGradientSpanGen>  tAGGRendererAAGradient;
typedef agg::renderer_scanline_bin<tAGGRendererBase, tAGGGradientSpanAlloc, tAGGGradientSpanGen> tAGGRendererBinGradient;

typedef agg::wrap_mode_repeat tAGGImageWrapModePad;
typedef agg::wrap_mode_repeat tAGGImageWrapModeClamp;
typedef agg::wrap_mode_reflect  tAGGImageWrapModeReflect;
typedef agg::wrap_mode_repeat tAGGImageWrapModeRepeat;

typedef agg::image_accessor_wrap<tAGGPixFmt,tAGGImageWrapModePad,tAGGImageWrapModePad> tAGGImageSourcePad;
typedef agg::image_accessor_wrap<tAGGPixFmt,tAGGImageWrapModeClamp,tAGGImageWrapModeClamp> tAGGImageSourceClamp;
typedef agg::image_accessor_wrap<tAGGPixFmt,tAGGImageWrapModeReflect,tAGGImageWrapModeReflect> tAGGImageSourceReflect;
typedef agg::image_accessor_wrap<tAGGPixFmt,tAGGImageWrapModeRepeat,tAGGImageWrapModeRepeat> tAGGImageSourceRepeat;

typedef agg::span_image_filter_rgba<tAGGImageSourcePad, tAGGLinearInterpolator>     tAGGSpanGenTypeImageFilterWrapPad;
typedef agg::span_image_filter_rgba_nn<tAGGImageSourcePad, tAGGLinearInterpolator>    tAGGSpanGenTypeImageFilterNoneWrapPad;
typedef agg::span_image_filter_rgba_bilinear<tAGGImageSourcePad, tAGGLinearInterpolator>  tAGGSpanGenTypeImageFilterBilinearWrapPad;
typedef agg::span_image_filter_rgba_2x2<tAGGImageSourcePad, tAGGLinearInterpolator>   tAGGSpanGenTypeImageFilter2x2WrapPad;

typedef agg::span_image_filter_rgba<tAGGImageSourceClamp, tAGGLinearInterpolator>     tAGGSpanGenTypeImageFilterWrapClamp;
typedef agg::span_image_filter_rgba_nn<tAGGImageSourceClamp, tAGGLinearInterpolator>    tAGGSpanGenTypeImageFilterNoneWrapClamp;
typedef agg::span_image_filter_rgba_bilinear<tAGGImageSourceClamp, tAGGLinearInterpolator>  tAGGSpanGenTypeImageFilterBilinearWrapClamp;
typedef agg::span_image_filter_rgba_2x2<tAGGImageSourceClamp, tAGGLinearInterpolator>   tAGGSpanGenTypeImageFilter2x2WrapClamp;

typedef agg::span_image_filter_rgba<tAGGImageSourceRepeat, tAGGLinearInterpolator>      tAGGSpanGenTypeImageFilterWrapRepeat;
typedef agg::span_image_filter_rgba_nn<tAGGImageSourceRepeat, tAGGLinearInterpolator>   tAGGSpanGenTypeImageFilterNoneWrapRepeat;
typedef agg::span_image_filter_rgba_bilinear<tAGGImageSourceRepeat, tAGGLinearInterpolator> tAGGSpanGenTypeImageFilterBilinearWrapRepeat;
typedef agg::span_image_filter_rgba_2x2<tAGGImageSourceRepeat, tAGGLinearInterpolator>    tAGGSpanGenTypeImageFilter2x2WrapRepeat;

typedef agg::span_image_filter_rgba<tAGGImageSourceReflect, tAGGLinearInterpolator>     tAGGSpanGenTypeImageFilterWrapReflect;
typedef agg::span_image_filter_rgba_nn<tAGGImageSourceReflect, tAGGLinearInterpolator>    tAGGSpanGenTypeImageFilterNoneWrapReflect;
typedef agg::span_image_filter_rgba_bilinear<tAGGImageSourceReflect, tAGGLinearInterpolator>  tAGGSpanGenTypeImageFilterBilinearWrapReflect;
typedef agg::span_image_filter_rgba_2x2<tAGGImageSourceReflect, tAGGLinearInterpolator>   tAGGSpanGenTypeImageFilter2x2WrapReflect;

inline static tAGGColorType AGGColorFromColor4f(const sColor4f& aColor) {
  return agg::rgba8((tU8)(aColor.x*255.0f),
                    (tU8)(aColor.y*255.0f),
                    (tU8)(aColor.z*255.0f),
                    (tU8)(aColor.w*255.0f));
  //return agg::rgba(aColor.r,aColor.g,aColor.b,aColor.a);
}
inline static sColor4f AGGColorToColor4f(const tAGGColorType& aColor) {
  sColor4f col;
  col.x = tF32(aColor.r)/255.0f;
  col.y = tF32(aColor.g)/255.0f;
  col.z = tF32(aColor.b)/255.0f;
  col.w = tF32(aColor.a)/255.0f;
  return col;
}

}
/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __AGG_22860808_H__
