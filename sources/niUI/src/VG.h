#ifndef __VG_317650_H__
#define __VG_317650_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
// #define agg_real ni::tF32
#include "AGG.h"

#if niMinFeatures(20)
// VGTransform.cpp
niExportFuncCPP(agg::trans_affine&) AGGGetTransform(iVGTransform* apTransform);
niExportFuncCPP(agg::trans_affine&) AGGGetTransform(const iVGTransform* apTransform);

// VGPaint.cpp
iAGGGradientPolymorphicWrapperBase* GetNewGradientWrapperBase(eVGGradientType aType, eVGWrapType aWrap);
sAGGGradientTable AGGGetGradientTable(const iVGGradientTable* apTable);
iVGPaint* CreateVGPaintSolid(const sColor4f& avColor);

struct iVGPathRasterizer {
  virtual void __stdcall Ras_BeginAddPath() = 0;
  virtual void __stdcall Ras_EndAddPath(const iVGStyle* apStyle, tBool abStroke) = 0;
};
#endif

#ifdef _DEBUG
void __VGImage_SetDebugName(iVGImage* apImg, const achar* aaszDebugName);
#define VGImage_SetDebugName(I,DBG) __VGImage_SetDebugName(I,DBG)
#else
#define VGImage_SetDebugName(I,DBG)
#endif

namespace ni {
  niExportFunc(iVGTransform*)  CreateVGTransform();
  niExportFunc(iVGPath*)  CreateVGPath();
  niExportFunc(iVGStyle*)  CreateVGStyle();
  niExportFunc(iVGStyle*)  CreateVGStyle();
}


/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __VG_317650_H__
