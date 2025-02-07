#ifndef __STDAFX_61021643_H__
#define __STDAFX_61021643_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#define niCCNoStrictOverrideWarnings
#include <niCC.h>

// #define IMDRAW
// #define niProfiler

#include "API/niUI.h"
#include "API/niUI/Utils/ColorUtils.h"
#include <niLang/Utils/ThreadImpl.h>
#include <niLang/Utils/ProfGather.h>

using namespace ni;

#define GDRV_DUMMY

#if defined niWindows
#  define GDRV_AUTO "Vulkan"
//#  define GDRV_GL2
#  define GDRV_VULKAN
#elif defined niOSX
#  define GDRV_AUTO "Metal"
//#  define GDRV_GL2
#  define GDRV_METAL
// #  define GDRV_VULKAN
#elif defined niIOSMac
#  define GDRV_AUTO "Metal"
#  define GDRV_METAL
#elif defined niIOS
#  define GDRV_AUTO "Metal"
//#  define GDRV_GL2
#  define GDRV_METAL
#elif defined niAndroid
#  define GDRV_AUTO "GL2"
//#  define GDRV_GL2
#elif defined niJSCC
#  define GDRV_AUTO "GL2"
//#  define GDRV_GL2
#elif defined niQNX
#  define GDRV_AUTO "GL2"
//#  define GDRV_GL2
#elif defined niLinux
#  define GDRV_AUTO "Vulkan"
//#  define GDRV_GL2
#  define GDRV_VULKAN
#else
#  error "No Graphics driver set for this platform."
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __STDAFX_61021643_H__
